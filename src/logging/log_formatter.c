#include "log_formatter.h"

#include "string_tool.h"
#include "timestamp.h"

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

static char* mLevelStr[] = { "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

void build_location(char location_info[], const char* source_file, int line_num,
        const char* func)
{
    int index = simple_string_last_index_of(source_file, "/");
    if (index != -1) {
        const char* tmp = source_file + index + 1;
        sprintf(location_info, "(%s:%d)", tmp, line_num);
    } else {
        sprintf(location_info, "(%s:%d)", source_file, line_num);
    }
}

void build_base(char base_info[], int level)
{
    char buf[64] = { 0 };
    simple_now_iso8601(buf);
    //build pid ,time and log info
    sprintf(base_info, "[%d] %s [%s]", (int) getpid(), buf, mLevelStr[level]);
}

void build_message( char log_message[], const char* fmt, va_list ap)
{
    vsprintf(log_message, fmt, ap);
}

void build_line(
        char line[],
        Logger* logger,
        int level,
        int line_num,
        const char* func,
        const char* source_file,
        const char* fmt,
        va_list ap)
{
    char base_info[256] = { 0 };

    char log_message[512] = { 0 };

    build_base(base_info, level);
    build_message(log_message, fmt, ap);

    char location_info[256] = { 0 };
    build_location(location_info, source_file, line_num, func);
    sprintf(line, "%s %s %s\n", base_info, log_message, location_info);
}
