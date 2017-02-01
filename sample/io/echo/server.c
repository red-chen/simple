/*
 *  server
 *  1. make -B
 *  2. ./server
 *
 *  client
 *  telnet 127.0.0.1 11233
 *
 */
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

static bool RUNNING = true;

static char buffer[1024] = {0};

static void handle_sig(int sig) {
    printf("Get signal: %d\n", sig);
    RUNNING = false;
}

static int handle_connect(EventLoop* loop, int listen_fd, void* user_data, int mask);
static int handle_read(EventLoop* loop, int fd, void* user_data, int mask);
static int handle_write(EventLoop* loop, int fd, void* user_data, int mask);

// Implement file func
int handle_write(EventLoop* loop, int fd, void* user_data, int mask) {
    int n = write(fd, buffer, strlen(buffer) + 1);
    if (n > 0) {
        printf("write:%s\n", buffer);
        event_loop_add_file_event(loop, fd, AE_READABLE, handle_read, buffer);    
    } else {
        close(fd);
    }
    return AE_NOMORE;
}

// Implement file func
int handle_read(EventLoop* loop, int fd, void* user_data, int mask) {
    bzero(buffer, 1024);
    int n = read(fd, buffer, 1024);
    if (n > 0) {
        printf("read:%s\n", buffer);
        event_loop_add_file_event(loop, fd, AE_WRITABLE, handle_write, buffer);    
    } else {
        close(fd);
    }
    // 思考：如果Read的数据不完整怎么办？
    // loop提供了AE_AGAIN选项，所以可以将这个选项返回。对于真实的应用场景，这里应该在封装一层，因为
    // 数据还没有收集完整，buffer也不能交给下游，所以应该封装一个‘Session’和fd绑定，生存周期也和fd一致。
    // Session内部创建Request和Response，当单次的读数据完整之后，拼装一个Request，交给下游。
    return AE_NOMORE;
}

// Implement file func
int handle_connect(EventLoop* loop, int listen_fd, void* user_data, int mask) {
    struct sockaddr_in client_addr;
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    int conn_fd = accept(listen_fd, (struct sockaddr*) &client_addr, (socklen_t*) &sockaddr_len);
    printf("new connection from [%s:%d] accept socket fd:%d.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), conn_fd);
    // 思考：如果我们要实现多线程这里应该怎么处理？
    // 如例子所示，一个IO线程专门用户新连接的接收，可以吧线程池放入user_data中，这里可以顺序选择线程池中的
    // 线程。比如，线程池中维护一个IO线程的列表，每个连接被建立，每次都选取一个连接，并将连接绑定到IO线程上。
    event_loop_add_file_event(loop, conn_fd, AE_READABLE, handle_read, user_data);    
    return AE_AGAIN;
}

int main() {
    // 设置信号处理
    signal(SIGINT, handle_sig);
    signal(SIGTERM, handle_sig);

    // 初始化IO线程
    SimpleIOThread* t = simple_io_thread_create(NULL);

    // 设置Tcp
    int fd = socket(AF_INET, SOCK_STREAM, 0); 
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(11233);
    puts("start listen the port(11233)");

    bind(fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(fd, 1024);

    // 将fd注册到io thread中
    simple_io_thread_add_file_event(t, fd, AE_READABLE, handle_connect, "handle connect");

    simple_io_thread_start(t);

    while(RUNNING) {
        sleep(1);
    }

    // 注销IO线程
    simple_io_thread_stop(t);
    simple_io_thread_join(t);
    simple_io_thread_destroy(t);
    return 0;
   
}
