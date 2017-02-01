#include <simple/io_thread.h>
#include <simple/timestamp.h>

#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static bool RUNNING = true;

static void handle_sig(int sig) {
    printf("Get signal: %d\n", sig);
    RUNNING = false;
}

static int64_t mye_time_func(EventLoop* loop, int64_t id, void* user_data) {
    char now[64] = {0};
    simple_now_iso8601(now);
    printf("%s, id:%ld, %s\n", now, id, (char*)user_data);
    return 1000;
}

int main() {

    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    SimpleIOThread* t = simple_io_thread_create(NULL);

    // 每1000毫秒定时打印一次hello world
    simple_io_thread_add_time_event(t, 1000, mye_time_func, "hello world", NULL);

    simple_io_thread_start(t);

    while(RUNNING) {
        sleep(2);
    }

    simple_io_thread_stop(t);
    simple_io_thread_join(t);
    simple_io_thread_destroy(t);
    return 0;
}
