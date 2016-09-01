/* A simple event-driven programming library. Originally I wrote this code
 * for the Jim's event-loop (Jim is a Tcl interpreter) but later translated
 * it in form of a library for easy reuse.
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LEMON_EVENT_LOOP_H_
#define LEMON_EVENT_LOOP_H_

#include <time.h>
#include <stdint.h>

#define AE_OK           0
#define AE_ERR         -1

#define AE_NOMORE      -2  // 表示不再执行
#define AE_AGAIN       -3  // 表示继续执行

#define AE_NONE         0
#define AE_READABLE     1
#define AE_WRITABLE     2

typedef struct EventLoop EventLoop;

/**
 * 文件事件回调函数
 * 参数：
 *     loop 事件循环
 *     fd 需要监听的fd
 *     user_data 用户传入的参数，详见event_loop_add_file_event的user_data字段
 *     mask 监听的事件，AE_READABLE，AE_WRITABLE
 *
 * 返回值
 *    AE_NOMORE 表示不再监听当前事件
 *    AE_AGAIN  表示需要继续监控
 *    AE_ERR    表示底层发生了错误
 */
typedef int FileFunc (EventLoop* loop, int fd, void* user_data, int mask);

/**
 * 时间事件回调函数
 * 参数：
 *     loop 事件循环
 *     id 当前时间事件在loop中的id
 *     user_data 在创建时间对象时的输入参数，详见event_loop_add_time_event方法的user_data字段
 *
 * 返回值
 *     TimeFunc下一次执行的时间间隔
 *     AE_NOMORE表示不再执行，如果不再执行，loop会删除该时间事件，不用显示删除
 */
typedef int64_t TimeFunc (EventLoop* loop, int64_t id, void* user_data);

/**
 * 时间事件完成回调函数
 * 参数：
 *     loop 事件循环
 *     user_data 在创建时间对象时的输入参数，详见event_loop_add_time_event方法的user_data字段
 *
 * 返回值：
 *     空
 */
typedef void FinalizeFunc (EventLoop* loop, void* user_data);

/**
 * 事件循环每轮开始执行之前的回调
 */
typedef void BeforeFunc (EventLoop* loop);

/**
 * 事件循环每轮结束时的回调
 */
typedef void AfterFunc (EventLoop* loop);

/* File event structure */
typedef struct file_event_t {
    int                     mask;
    FileFunc*               read;
    FileFunc*               write;
    void*                   user_data;
} FileEvent;

/* Time event structure */
typedef struct time_event_t {
    int64_t                 id;
    int32_t                 when_sec;
    int32_t                 when_ms;
    TimeFunc*               func;
    FinalizeFunc*           finalize;
    void*                   user_data;
    struct time_event_t*    next;
} TimeEvent;

/* Fired event structure */
typedef struct fired_event_t {
    int                     fd;
    int                     mask;
} FiredEvent;

struct EventLoop {
    int                     max_fd; // 目前已经注册的最大文件描述符
    int                     set_size; // 目前追踪到的最大文件描述符
    long                    time_event_next_id;
    time_t                  last_time;
    FileEvent*              events; // 存储所有的事件
    FiredEvent*             fired; // 存储所有正在发生的事件
    TimeEvent*              time_event_head;
    int                     stop;
    void*                   api_data;
    BeforeFunc*             before;
    BeforeFunc*             after;
};

/* API */

/**
 * 创建循环事件，并指定能监听的最大事件个数
 */
EventLoop* event_loop_create(int set_size);

/**
 * 事件循环中，每次循环开始之前调用before
 */
void event_loop_set_before(EventLoop* loop, BeforeFunc* before);

/**
 * 事件循环中，每次循环结束之后调用after
 */

void event_loop_set_after(EventLoop* loop, AfterFunc* after);

/**
 * 添加文件事件
 *
 * 参数
 *     loop 事件循环
 *     fd 需要监听的fd
 *     mask 监听的具体事件AE_READABLE，AE_WRITABLE
 *     func 事件发生后的回调函数
 *     user_data 用户注入的自定义的参数
 *
 * 返回值
 *     AE_OK 表示成功
 *     AE_ERR 表示失败
 */
int event_loop_add_file_event(
        EventLoop* loop,
        int fd,
        int mask,
        FileFunc* func,
        void* user_data);

/**
 *
 */
void event_loop_del_file_event(
        EventLoop* loop,
        int fd,
        int mask);

int event_loop_get_file_event(EventLoop* loop, int fd);

/**
 * 向事件循环中添加一个时间事件，待milliseconds时间之后运行
 *
 * 注意：时间事件运行的时间点是当前时间 + milliseconds
 *
 * 参数：
 *     loop ：事件循环
 *     milliseconds ： 等待时间
 *     func ：时间事件的具体执行回调函数
 *     uesr_data ： func中uesr_data参数
 *     finalize ：时间事件执行之后析构函数
 *
 * 返回值：
 *     时间事件的Id
 */
int64_t event_loop_add_time_event(
        EventLoop* loop,
        int64_t milliseconds,
        TimeFunc* func,
        void* uesr_data,
        FinalizeFunc* final);

/**
 * 根据时间事件Id，删除事件循环中的时间事件
 */
int event_loop_del_time_event(EventLoop* loop, int64_t id);

void event_loop_run(EventLoop* loop);

void event_loop_stop(EventLoop* loop);

#endif /* EVENT_LOOP_H_ */
