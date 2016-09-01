// 主要是转义用户定义的日志格式

#ifndef LOG_FORMATTER_H_
#define LOG_FORMATTER_H_

#include "logging.h"

#include <stdio.h>

void get_localtime_str(char str[]);

void build_location(char location_info[], const char* source_file, int line_num,
        const char* func);

void build_base(char base_info[], int level);

void build_message( char log_message[], const char* fmt, va_list ap);

void build_line(
        char line[],
        Logger* logger,
        int level,
        int line_num,
        const char* func,
        const char* source_file,
        const char* fmt,
        va_list ap);

#endif /* LOG_FORMATTER_H_ */
