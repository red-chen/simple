#include "log_record.h"
#include "log_formatter.h"

#include "assert.h"

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

LogRecord* logrecord_new() {
    LogRecord* self = malloc(sizeof(LogRecord));

    ASSERT_NOT_NULL(self);

    self->pid = getpid();
    self->tid = pthread_self();

    return self;
}

void logrecord_message(
        Logger* logger,
        int level,
        int line_num,
        const char* func,
        const char* source_file,
        const char* fmt,
        va_list ap,
        char line[]) {
    build_line(line, logger, level, line_num, func, source_file, fmt, ap);
}


