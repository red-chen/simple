// 主要是提供常用的宏定义
#ifndef LEMON_ASSERT_H_
#define LEMON_ASSERT_H_

#include "timestamp.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <execinfo.h>

void va_list_print(char log_message[], char* format, ...);

#define DEBUG_STRING(input)\
    fprintf(stdout, "DEBUG_STRING: %s\n", (input));

#define DEBUG_INT(input)\
    fprintf(stdout, "DEBUG_INT: %d\n", (input));

#define DEBUG_LONG(input)\
    fprintf(stdout, "DEBUG_LONG: %ld\n", (input));

#define ASSERT_NOT_NULL(flag) \
        if(!((flag) != NULL)){\
            char __log_time__[64] = { 0 };\
            simple_now_iso8601(__log_time__);\
            char __buffer_stack__[1024];\
            void *__array__[10];\
            char **__strframe__ = NULL;\
            int __i__, __idx__=0, __n__ = backtrace(__array__, 10);\
            __strframe__ = (char **)backtrace_symbols(__array__, __n__);\
            for (__i__ = 0; __i__ < __n__; __i__++) \
            __idx__ += snprintf(__idx__+__buffer_stack__, 256, "%s\n", __strframe__[__i__]);\
            free(__strframe__);\
            fprintf(stderr, "%s, Assert fail ! pointer is NULL.\t[%s:%d]\n%s\n", (__log_time__), __FILE__, __LINE__, __buffer_stack__);\
            abort();\
        }

#define ASSERT(flag, ...)\
        if(!(flag)){\
            char __log_time__[64] = { 0 };\
            simple_now_iso8601(__log_time__);\
            char __buffer_stack__[1024];\
            void *__array__[10];\
            char **__strframe__ = NULL;\
            int __i__, __idx__=0, __n__ = backtrace(__array__, 10);\
            __strframe__ = (char **)backtrace_symbols(__array__, __n__);\
            for (__i__ = 0; __i__ < __n__; __i__++) \
            __idx__ += snprintf(__idx__+__buffer_stack__, 256, "%s\n", __strframe__[__i__]);\
            free(__strframe__);\
            char __log_message__[512] = { 0 };\
            va_list_print(__log_message__, __VA_ARGS__);\
            fprintf(stderr, "%s, Assert fail ! %s\t[%s:%d]\n%s\n", (__log_time__), (__log_message__),  __FILE__, __LINE__, __buffer_stack__);\
            abort();\
        }

#define WARN(...) \
    char __log_time__[64] = { 0 };\
    simple_now_iso8601(__log_time__);\
    char __log_message__[512] = { 0 };\
    va_list_print(__log_message__, __VA_ARGS__);\
    fprintf(stderr, "%s, Warning ! %s\t[%s:%d]\n", (__log_time__), (__log_message__),  __FILE__, __LINE__);\

#endif /* ASSERT_H_ */
