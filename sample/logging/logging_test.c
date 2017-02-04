#include "simple/logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static Logger* log_a0 = NULL;
static Logger* log_a1 = NULL;

void test(int i) {
    LOG_DEBUG(log_a0, "%d", i);
    LOG_INFO(log_a0, "%d", i);
    LOG_ERROR(log_a0, "%d", i);
    LOG_FATAL(log_a0, "%d", i);
    LOG_DEBUG(log_a1, "%d", i);
    LOG_INFO(log_a1, "%d", i);
    LOG_ERROR(log_a1, "%d", i);
    LOG_FATAL(log_a1, "%d", i);
}

int main() {
    puts("LOGGING_INIT !");

    LOGGING_INIT("logging.conf");
    //LOGGING_INIT(NULL);

    log_a0 = LOGGING_GET_LOGGER("/test/A0/example");
    log_a1 = LOGGING_GET_LOGGER("/test/A1/example");

    for (int i = 0; i < 100; i++) {
        test(i);
    }

    LOGGER_FLUSH(log_a0);
    LOGGER_FLUSH(log_a1);
    puts("end !");
    sleep(1);
    return 0;
}

