#include "simple/logging.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static Logger* log = NULL;

void test(int i) {
    LOG_DEBUG(log, "%d", i);
    LOG_INFO(log, "%d", i);
    LOG_ERROR(log, "%d", i);
    LOG_FATAL(log, "%d", i);
}

int main() {
    puts("LOGGING_INIT !");

    LOGGING_INIT("logging.conf");
    //LOGGING_INIT(NULL);

    log = LOGGING_GET_LOGGER("/test/A0/example");

    for (int i = 0; i < 100; i++) {
        test(i);
    }

    LOGGER_FLUSH(log);
    puts("end !");
    sleep(1);
    return 0;
}

