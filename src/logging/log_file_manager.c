#include "log_file_manager.h"

#include "file/async_file.h"

#include "assert.h"
#include "timestamp.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

struct logfilemanager_t {
    char path[256];
    enum LogFileManagerMode mode;
    int file_size;
    int file_size_limit;
    int file_count_limit;
    AsyncFile* file;
    pthread_mutex_t mutex;
    int buffer_size;
    int buffer_size_limit;
    void* buffer;

    //
    TIME_IN_MICRO day;
    TIME_IN_MICRO hour;
};

static void open_file(LogFileManager* self) {
    ASSERT(self->file == NULL, "the file still open.");
    self->file_size = 0;
    self->buffer_size = 0;
    self->file = asyncfile_new(self->path, ASYNC_MODE_DISCARDED, CACHE, 10, 128 * 1024);
}

static void reset_file_rolling(LogFileManager* self) {
    char src_file_name[256] = {0};
    char target_file_name[256] = {0};

    sprintf(src_file_name, "%s.%d", self->path, self->file_count_limit - 1);

    unlink(src_file_name);
    // errno == 2 means 'file not exists'
    ASSERT((errno == 0 || errno == 2), "unlink %s fail, error code:%d, error:%s.", src_file_name, errno, strerror(errno));


    for(int i = self->file_count_limit - 1; i > 0; i--) {
        bzero(src_file_name,    256);
        bzero(target_file_name, 256);
        if (i == 1)
        {
            sprintf(src_file_name,    "%s", self->path);
            sprintf(target_file_name, "%s.%d", self->path, i);
        }
        else
        {
            sprintf(src_file_name,    "%s.%d", self->path, i - 1);
            sprintf(target_file_name, "%s.%d", self->path, i);
        }
        rename(src_file_name, target_file_name);
        ASSERT((errno == 0 || errno == 2), "rename %s fail, error code:%d, error:%s.", src_file_name, errno, strerror(errno));
    }
}

static void reset_file_day(LogFileManager* self) {
    ASSERT(false, "Unimplement");
}

static void reset_file_hour(LogFileManager* self) {
    ASSERT(false, "Unimplement");
}

static void reset_file(LogFileManager* self, const char* line) {
    switch (self->mode) {
        case LOG_FILE_MANAGER_ROLLING :
        {
            if (self->file_size + strlen(line) > self->file_size_limit) {
                logfilemanager_flush(self);
                asyncfile_close(self->file);
                self->file = NULL;
                reset_file_rolling(self);
                open_file(self);
            }
            break;
        }
        case LOG_FILE_MANAGER_DAY :
        {
            TIME_IN_MICRO curDay = simple_real_time_now();
            if (self->day + DAY_IN_MICRO < curDay) {
                logfilemanager_flush(self);
                asyncfile_close(self->file);
                self->file = NULL;
                reset_file_day(self);
                open_file(self);
            }
            break;
        }

        case LOG_FILE_MANAGER_HOUR:
        {
            TIME_IN_MICRO curHour = simple_real_time_now();
            if (self->hour + HOUR_IN_MICRO < curHour) {
                logfilemanager_flush(self);
                asyncfile_close(self->file);
                self->file = NULL;
                reset_file_hour(self);
                open_file(self);
            }
            break;
        }

    }
}

LogFileManager* logfilemanager_new(
        const char* path,
        enum LogFileManagerMode mode,
        int file_size,
        int file_num,
        int buffer_size) {
    ASSERT_NOT_NULL(path);
    ASSERT(strlen(path) > 0 && strlen(path) < 255, "Input path len(%d) must > 0 and < 255.", strlen(path));

    LogFileManager* self = malloc(sizeof(LogFileManager));
    ASSERT_NOT_NULL(self);

    memcpy(self->path, path, strlen(path));
    self->file_size_limit = file_size;
    self->file_count_limit = file_num;
    self->mode = mode;

    self->buffer_size_limit = buffer_size;
    self->buffer = malloc(self->buffer_size_limit);
    ASSERT_NOT_NULL(self->buffer);
    bzero(self->buffer, self->buffer_size_limit);

    // reset environment
    if (mode == LOG_FILE_MANAGER_ROLLING) {
        reset_file_rolling(self);
    }

    open_file(self);

    pthread_mutex_init(&self->mutex, NULL);

    return self;
}

void logfilemanager_close(LogFileManager* self) {
    pthread_mutex_destroy(&self->mutex);

    asyncfile_close(self->file);

    free(self->buffer);
    free(self);
}


void logfilemanager_write_line(LogFileManager* self, const char* line) {
    int line_size = strlen(line);
    const char* tmp_line = line;
    reset_file(self, line);

    pthread_mutex_lock(&self->mutex);
    self->file_size += line_size;

    while (line_size > 0) {
        int remain_size = self->buffer_size_limit - self->buffer_size;
        if (remain_size == 0) {
            asyncfile_write(self->file, self->buffer, self->buffer_size);
            bzero(self->buffer, self->buffer_size_limit);
            self->buffer_size = 0;
            continue;
        }
        if (line_size > remain_size) {
            memcpy(self->buffer + self->buffer_size, tmp_line, remain_size);
            self->buffer_size += remain_size;
            tmp_line += remain_size; // move point
        } else {
            memcpy(self->buffer + self->buffer_size, tmp_line, line_size);
            self->buffer_size += line_size;
        }
        line_size -= remain_size;
    }
    pthread_mutex_unlock(&self->mutex);
}

void logfilemanager_flush(LogFileManager* self) {
    pthread_mutex_lock(&self->mutex);
    if (self->buffer_size > 0) {
        asyncfile_write(self->file, self->buffer, self->buffer_size);
        bzero(self->buffer, self->buffer_size_limit);
        self->buffer_size = 0;
    }
    asyncfile_flush(self->file);
    pthread_mutex_unlock(&self->mutex);
}
