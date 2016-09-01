#include "logging.h"

#include "log_file_manager.h"
#include "log_conf.h"
#include "log_record.h"

#include "collection/array_list.h"
#include "collection/order_list.h"

#include "assert.h"
#include "conf.h"
#include "string_tool.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <inttypes.h>
#include <errno.h>

// define

struct Logging {
    LoggingConf* conf;
    OrderList* pool;
};

struct Logger {
    pthread_mutex_t mutex;
    LoggerConf* conf;
    LogFileManager* manager;
};

// implement
static Logging* mLogging = NULL;

static int mapping_compare(void* v1, void* v2) {
    Logger* l1 = (Logger*)v1;
    Logger* l2 = (Logger*)v2;

    return -strcmp(l1->conf->mapping, l2->conf->mapping);// 逆向排序
}

static Logger* logger_create(LoggerConf* conf) {
    Logger* self =  malloc(sizeof(Logger));
    ASSERT_NOT_NULL(self);

    pthread_mutex_init(&self->mutex, NULL);
    self->conf = conf;
    self->manager = logfilemanager_new(
            conf->path,
            conf->mode,
            conf->file_size_in_byte,
            conf->file_limit,
            conf->buffer_size
    );
    return self;
}

static void logging_init(Logging* self, Conf* conf) {
    ASSERT_NOT_NULL(self);
    ASSERT_NOT_NULL(conf);

    LoggingConf* lc = logconf_load(conf);
    self->conf = lc;

    for (int i = 0; i < arraylist_size(self->conf->loggers); i++) {
        orderlist_add(self->pool, logger_create(arraylist_get(self->conf->loggers, i)));
    }
}

static void load_conf(Logging* self, const char* conf_path) {
    ASSERT_NOT_NULL(self);
    ASSERT_NOT_NULL(conf_path);

    // check is exist
    if (access(conf_path, 0) == -1) { // not exist
        ASSERT(false, "Can not find the log conf file : %s", conf_path);
    }

    Conf* conf = conf_create();
    conf_load(conf, conf_path);

    logging_init(self, conf);

    conf_destroy(conf);
}

static Logging* logging_create(const char* conf_path) {
    Logging* self = malloc(sizeof(Logging));
    ASSERT_NOT_NULL(self);
    bzero(self, sizeof(Logging));

    self->pool = orderlist_new(mapping_compare);

    if (conf_path != NULL) {
        load_conf(self, conf_path);
    }
    return self;
}

void LOGGING_INIT(const char* conf_path) {
    ASSERT(mLogging == NULL, "The singleton Logging has been init.");
    mLogging = logging_create(conf_path);
}

Logger* LOGGING_GET_LOGGER(const char* mapping) {
    if (mLogging == NULL) {
        return NULL;
    }
    int size = orderlist_size(mLogging->pool);
    Logger* value = malloc(sizeof(Logger));
    value->conf = malloc(sizeof(LoggerConf));
    strcpy(value->conf->mapping, mapping);

    Logger* r = NULL;

    if (size != 0) {
        int index = orderlist_binary_search(
                mLogging->pool,
                0,
                size - 1,
                value);
        if (index >=0 && index < size) {
            r = orderlist_get(mLogging->pool, index);
        } else if (index < 0) {
            Logger* tmp = orderlist_get(mLogging->pool, (index + 1) * -1);
            if (simple_string_start_with(mapping,tmp->conf->mapping)){
                r = tmp;
            }
        }
        free(value->conf);
        free(value);
    }

    return r;
}

void LOGGER_FLUSH(Logger* log) {
    if (log != NULL) {
        logfilemanager_flush(log->manager);
    }
}

// 向日志系统中添加日志对象
void    LOGGING_ADD_LOGGER(const Logger* logger) {
    orderlist_add(mLogging->pool, (void*)logger);
}

// Dump日志系统中的所有数据
void    LOGGING_FLUSH(){
    ASSERT(false, "Unimplement");
}

// 关闭日志系统，并回收资源
void    LOGGING_CLOSE() {
    ASSERT(false, "Unimplement");
}

//

void __logger__(
        Logger* log,
        enum LoggerLevel level,
        int line_num,
        const char* func,
        const char* file,
        const char* fmt, ...) {
    if (log != NULL && log->conf->level <= level ) {
        pthread_mutex_lock(&log->mutex);
        char line[1024] = {0};
        va_list ap;
        va_start(ap, fmt);
        logrecord_message(log, level, line_num, func, file, fmt, ap, line);
        if (log->conf->stdout) {
            printf("%s", line);
        }
        logfilemanager_write_line(log->manager, line);
        pthread_mutex_unlock(&log->mutex);
    }
}
