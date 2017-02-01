#include <simple/io_thread.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

struct User {
    int size;
    int sock;
};

static bool RUNNING = true;

static char buffer[1024] = {0};

static int count = 0;

static int limit = 10;

static void handle_sig(int sig) {
    printf("Get signal: %d\n", sig);
    RUNNING = false;
}

static int handle_read(EventLoop* loop, int fd, void* user_data, int mask);
static int handle_write(EventLoop* loop, int fd, void* user_data, int mask);

// Implement file func
int handle_write(EventLoop* loop, int fd, void* user_data, int mask) {
    struct User* u = (struct User*)user_data;
    if (count++ < limit) {
        printf("fd:%d, sock:%d\n", fd, u->sock);
        write(fd, buffer, strlen(buffer) + 1);
        event_loop_add_file_event(loop, fd, AE_READABLE, handle_read, buffer);    
    } else {
        close(fd);
        count++;
    }
    return AE_NOMORE;
}

// Implement file func
int handle_read(EventLoop* loop, int fd, void* user_data, int mask) {
    bzero(buffer, 1024);
    read(fd, buffer, 1024);
    printf("%s\n", buffer);
    event_loop_add_file_event(loop, fd, AE_WRITABLE, handle_write, buffer);    
    return AE_NOMORE;
}

int client_connect() {
    struct sockaddr_in server_addr, client_addr;
    bzero(&server_addr, sizeof(server_addr));
    bzero(&client_addr, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(11233);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    return fd;
}

int main() {
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    SimpleIOThread* t = simple_io_thread_create(NULL);

    strcpy(buffer, "hello world !");

    for (int i = 0; i < limit; i++){
        // 将fd注册到io thread中
        int fd = client_connect();
        struct User* u = malloc(sizeof(struct User));
        u->sock = fd;
        u->size = 1024;

        simple_io_thread_add_file_event(
                t, 
                fd, 
                AE_WRITABLE, 
                handle_write, 
                u);
    }

    simple_io_thread_start(t);

    while(RUNNING && count <= limit) {
        sleep(1);
    }

    simple_io_thread_stop(t);
    simple_io_thread_join(t);
    simple_io_thread_destroy(t);
    return 0;
   
}
