#ifndef SIMPLE_LOGGING_H_
#define SIMPLE_LOGGING_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

enum LoggerLevel
{
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

typedef struct Logger Logger;

typedef struct Logging Logging;

// API

// 日志系统的初始化
// 参数：
//      conf_path 日志的配置文件，可以传入NULL，表示不加载任何配置，只是初始化日志系统
void    LOGGING_INIT(const char* conf_path);

// 获取日志对象
// 参数：
//      mapping 获取对应的日志实例
Logger* LOGGING_GET_LOGGER(const char* mapping);

// 向日志系统中添加日志对象
void    LOGGING_ADD_LOGGER(const Logger* logger);

// Dump日志系统中的所有数据
void    LOGGING_FLUSH();

// 关闭日志系统，并回收资源
void    LOGGING_CLOSE();

// Dump日志对象中的数据
void    LOGGER_FLUSH(Logger* log);

#define LOG_DEBUG(obj, ...)\
        __logger__(obj, LOG_DEBUG, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__);

#define LOG_INFO(obj, ...)\
        __logger__(obj, LOG_INFO, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__);

#define LOG_WARN(obj, ...)\
        __logger__(obj, LOG_WARN, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__);

#define LOG_ERROR(obj, ...)\
        __logger__(obj, LOG_ERROR, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__);

#define LOG_FATAL(obj, ...)\
        __logger__(obj, LOG_FATAL, __LINE__, __FUNCTION__, __FILE__, __VA_ARGS__);

void __logger__(
        Logger* log,
        enum LoggerLevel level,
        int line_num,
        const char* func,
        const char* file,
        const char* fmt, ...);

#endif /* SIMPLE_LOGGING_H_ */
