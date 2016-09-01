#include "file/async_file.h"

#include "assert.h"
#include "collection/blocking_queue.h"

#include <error.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

struct FileBuffer{
    int size;
    void* data;
};

struct asyncfile_t {
    enum AsyncMode async_mode;
    enum WriteMode write_mode;
    FILE* fp;
    char path[512];

    BlockingQueue* queue;
    int buf_num;
    int buf_size;
    pthread_t flush_tid; // 背景flush线程

    struct FileBuffer* buf;
    pthread_mutex_t mutex;
};

static void* flushing_data(void* param) {
    ASSERT_NOT_NULL(param);

    AsyncFile* self = (AsyncFile*)param;

    struct FileBuffer* out = NULL;
    while ((out = blockingqueue_pop(self->queue))->data != NULL) {
        fwrite(out->data, out->size, 1, self->fp);
        fflush(self->fp);

        if (self->write_mode == NO_CACHE) {
            fsync(self->fp->_fileno);
        }
        free(out->data);
        free(out);
    }
    free(out);
    return NULL;
}

static struct FileBuffer* filebuffer_new(int size) {
    struct FileBuffer* buf = malloc(sizeof(struct FileBuffer));
    ASSERT_NOT_NULL(buf);
    buf->data = malloc(size);
    ASSERT_NOT_NULL(buf->data);
    bzero(buf->data, size);
    buf->size = 0;
    return buf;
}

AsyncFile* asyncfile_new(
        const char* path,
        enum AsyncMode async_mode,
        enum WriteMode wirte_mode,
        int buf_num,
        int buf_size) {
    ASSERT(path != NULL && strlen(path) > 0, "Path is null or empty.");
    AsyncFile* self = malloc(sizeof(AsyncFile));
    bzero(self, sizeof(AsyncFile));

    self->async_mode = async_mode;
    self->write_mode = wirte_mode;
    strcpy(self->path, path);
    self->fp = fopen(self->path, "a");

    ASSERT(self->fp != NULL, "Open file failed, errno:%d, errmsg:%s, path ; %s", errno, strerror(errno), path);

    self->buf_size = buf_size;
    self->buf_num = buf_num;
    self->queue = blockingqueue_new(self->buf_num);
    self->buf = filebuffer_new(self->buf_size);

    pthread_mutex_init(&self->mutex, NULL);

    pthread_create(&self->flush_tid, NULL, flushing_data, self);

    return self;
}

void asyncfile_close(AsyncFile* self) {
    ASSERT_NOT_NULL(self);

    struct FileBuffer* terminate = malloc(sizeof(struct FileBuffer));
    terminate->data = NULL;

    blockingqueue_push(self->queue, self->buf);
    blockingqueue_push(self->queue, terminate);

    pthread_join(self->flush_tid, NULL);

    fflush(self->fp);
    fclose(self->fp);
    blockingqueue_free(self->queue);
    free(self);

    pthread_mutex_destroy(&self->mutex);
}

void asyncfile_flush(AsyncFile* self) {
    pthread_mutex_lock(&self->mutex);
    if (self->buf_size > 0) {
        blockingqueue_push(self->queue, self->buf);
        self->buf = filebuffer_new(self->buf_size);
    }
    pthread_mutex_unlock(&self->mutex);
}

// 这里会拷贝用户的数据
void asyncfile_write(AsyncFile* self,const void* data, int size) {
    ASSERT_NOT_NULL(self);
    ASSERT_NOT_NULL(data);
    ASSERT(size > 0, "Input size(%d) <= 0", size);

    if (blockingqueue_size(self->queue) >= self->buf_num  && self->async_mode == ASYNC_MODE_DISCARDED) { // busy
        fprintf(stderr, "discard data when flush is full. data:%s", (char*)data);
        return;
    }

    pthread_mutex_lock(&self->mutex);

    const void* tmp_data = data;
    struct FileBuffer* buf = self->buf;

    while (size > 0) {
        int remain_size = self->buf_size - buf->size;
        if (remain_size == 0) {
            blockingqueue_push(self->queue, buf);
            self->buf = filebuffer_new(self->buf_size);
            buf = self->buf;
        }

        if (size > remain_size) {
            memcpy(buf->data + buf->size, tmp_data, remain_size);
            buf->size += remain_size;
            tmp_data += remain_size; // move point
        } else {
            memcpy(buf->data + buf->size, tmp_data, size);
            buf->size += size;
        }
        size -= remain_size;
    }
    pthread_mutex_unlock(&self->mutex);
}

// 这里会拷贝用户的数据
void asyncfile_write_line(AsyncFile* self, const char* line) {
    ASSERT_NOT_NULL(self);
    ASSERT_NOT_NULL(line);
    asyncfile_write(self, line, strlen(line));
}
