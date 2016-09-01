#ifndef LOG_RECORD_H_
#define LOG_RECORD_H_

#include "logging.h"

#include <stdio.h>

typedef struct logrecord_t {
    int pid;
    int tid;
    char* fmt;
    va_list ap;
}LogRecord;

LogRecord* logrecord_new();

void logrecord_message(
        Logger* logger,
        int level,
        int line_num,
        const char* func,
        const char* source_file,
        const char* fmt,
        va_list ap,
        char line[]);

#endif /* LOG_RECORD_H_ */
