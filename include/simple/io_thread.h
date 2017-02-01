/**
 * @file io_thread.h
 * @brief IO线程，主要是负责IO事件处理，IO线程中包括了一个独立的事件循环 
 * one loop one thread
 * @author chen wan hong, redchen1255@gmail.com
 * @version 
 * @date 2016-04-07
 */

#ifndef SIMPLE_IO_THREAD_H_
#define SIMPLE_IO_THREAD_H_

#include "event_loop.h"

#include <pthread.h>
#include <stdbool.h>

typedef struct simple_io_thread_t SimpleIOThread;

/**
 * @brief 创建创建一个IO线程，在线程内部启动一个event loop
 *
 * @param name 线程的名字，可以传入一个空，名字的长度不超过64个字节
 *
 * @return 
 *     - 返回创建的IO线程对象
 */
SimpleIOThread* simple_io_thread_create(const char* name);

/**
 * @brief 销毁IO线程
 *
 * @param self IO线程对象
 */
void simple_io_thread_destroy(SimpleIOThread* self);

/**
 * @brief 启动线程，并启动loop
 *
 * @param self IO线程对象
 */
void simple_io_thread_start(SimpleIOThread* self);

/**
 * @brief 停止线程 
 *
 * @param self IO线程对象
 */
void simple_io_thread_stop(SimpleIOThread* self);

/**
 * @brief 等待IO线程结束
 *
 * @param self IO线程对象
 */
void simple_io_thread_join(SimpleIOThread* self);

/**
 * @brief 获得线程的事件循环队列 
 *
 * @param self IO线程对象
 *
 * @return 
 *     - 事件循环对象
 */
EventLoop* simple_io_thread_get_loop(SimpleIOThread* self);

/**
 * @brief 获取IO线程的名称
 *
 * @param self IO线程对象
 *
 * @return 
 *     - IO线程对象的名称
 */
const char* simple_io_thread_get_name(SimpleIOThread* self);

int simple_io_thread_add_file_event(
        SimpleIOThread* self,
        int fd,
        int mask,
        FileFunc* func,
        void* user_data);

void simple_io_thread_del_file_event(
	SimpleIOThread* self, 
	int fd, 
	int mask);

int64_t simple_io_thread_add_time_event(
        SimpleIOThread* self,
        int64_t millisecond,
        TimeFunc* func,
        void* user_data,
        FinalizeFunc* final);

void simple_io_thread_del_time_event(
	SimpleIOThread* self, 
	int64_t timeEventId);

#endif /* SIMPLE_IO_THREAD_H_ */
