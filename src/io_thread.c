#include "io_thread.h"

#include "event_loop.h"
#include "assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_RUNNING_INTERVAL_IN_MILLI 1000

typedef void* ThreadWorker(void*);

struct simple_io_thread_t {
    char name[64];
    pthread_t thread_id;
    bool running;
    ThreadWorker* worker;
    EventLoop* loop;
};

static int64_t check_thread_running(EventLoop* loop, int64_t id, void* user_data) {
    SimpleIOThread* self = (SimpleIOThread*)user_data;
    if (!self->running) { // 当线程被停止掉，表示不再check，那么返回-1
        return AE_NOMORE;
    }

    // 返回下一次要检测的时间间隔
    return CHECK_RUNNING_INTERVAL_IN_MILLI;
}

static void finalize(EventLoop* loop, void* user_data) {
    event_loop_stop(loop);
}

static void* run_loop(void* input) {
    SimpleIOThread* self = (SimpleIOThread*)input;

    // 注入一个定时检测线程是否在运行的函数
    event_loop_add_time_event(
            self->loop,
            CHECK_RUNNING_INTERVAL_IN_MILLI,
            check_thread_running,
            input,
            finalize
            );

    // 启动loop
    event_loop_run(self->loop);
    return NULL;
}

SimpleIOThread* simple_io_thread_create(const char* name) {
    SimpleIOThread* self = malloc(sizeof(SimpleIOThread));
    bzero(self, sizeof(SimpleIOThread));

    self->worker = run_loop;
    self->loop = event_loop_create(1024);
    self->running = false;

    if (name != NULL) {
        strcpy(self->name, name);
    }
    return self;
}

const char* simple_io_thread_get_name(SimpleIOThread* self) {
    return self->name;
}

void simple_io_thread_destroy(SimpleIOThread* self) {
    free(self);
}

void simple_io_thread_start(SimpleIOThread* self) {
    self->running = true;
    pthread_create(&(self->thread_id), NULL, self->worker, (void*)self);
}

void simple_io_thread_stop(SimpleIOThread* self) {
    self->running = false;
}

void simple_io_thread_join(SimpleIOThread* self) {
    pthread_join(self->thread_id, NULL);
}

EventLoop* simple_io_thread_get_loop(SimpleIOThread* self) {
    return self->loop;
}

int simple_io_thread_add_file_event(
        SimpleIOThread* self,
        int fd,
        int mask,
        FileFunc* func,
        void* user_data) {
    //FIXME check is running
    return event_loop_add_file_event(self->loop, fd, mask, func, user_data);
}

void simple_io_thread_del_file_event(SimpleIOThread* self, int fd, int mask) {
    event_loop_del_file_event(self->loop, fd, mask);
}

int64_t simple_io_thread_add_time_event(
        SimpleIOThread* self,
        int64_t millisecond,
        TimeFunc* func,
        void* user_data,
        FinalizeFunc* finalize) {
    return event_loop_add_time_event(self->loop, millisecond, func, user_data, finalize);
}

void simple_io_thread_del_time_event(SimpleIOThread* self, int64_t timeEventId) {
    event_loop_del_time_event(self->loop, timeEventId);
}
