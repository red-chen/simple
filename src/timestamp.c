#include "timestamp.h"
#include "assert.h"

#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

TIME_IN_MICRO simple_monotonic_time_now()
{
    struct timespec now;
    ASSERT((clock_gettime(CLOCK_MONOTONIC, &now) == 0), "clock_gettime run fail ! errno:%d, error message:%s", errno, strerror(errno));
    return now.tv_sec * MICRO_PER_SEC + now.tv_nsec / NANO_PER_MICRO;
}

TIME_IN_MICRO simple_real_time_now()
{
    struct timeval now;
    ASSERT((gettimeofday(&now, NULL) == 0), "gettimeofday run fail ! errno:%d, error message:%s", errno, strerror(errno));
    return now.tv_sec * MICRO_PER_SEC + now.tv_usec;
}

void simple_timestamp_to_iso8601(TIME_IN_MICRO ts, /*out*/char out[])
{
    time_t now = ts / MICRO_PER_SEC;
    TIME_IN_MICRO micro = (ts - now * MICRO_PER_SEC);
    char time_str[65] = {0};
    char zone_str[10] = {0};
    strftime(time_str, 64, "%Y-%m-%dT%H:%M:%S", localtime(&now));
    strftime(zone_str, 10, "%z", localtime(&now));
    char short_zone[4] = {0};
    strncpy(short_zone, zone_str, 3);
    sprintf(out, "%s.%06d%s", time_str, (int)micro, short_zone);    
}

void simple_now_iso8601(/*out*/char out[])
{
    simple_timestamp_to_iso8601(simple_real_time_now(), out);
}


