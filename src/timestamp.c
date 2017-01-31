#include "timestamp.h"
#include "assert.h"

#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

TIME_IN_MICRO simple_monotonic_time_now()
{
    struct timespec now;
    ASSERT(
            (clock_gettime(CLOCK_MONOTONIC, &now) == 0), 
            "clock_gettime run fail ! errno:%d, error message:%s", errno, strerror(errno)
    );
    return now.tv_sec * MICRO_PER_SEC + now.tv_nsec / NANO_PER_MICRO;
}

TIME_IN_MICRO simple_real_time_now()
{
    struct timeval now;
    ASSERT(
            (gettimeofday(&now, NULL) == 0), 
            "gettimeofday run fail ! errno:%d, error message:%s", errno, strerror(errno)
    );
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

static TIME_IN_MICRO simple_get_zone_diff(const char* input_zone) {
    TIME_IN_MICRO ts = 0;
    char zone_str[10] = {0};

    if (NULL == input_zone) {
        time_t now = 0;
        strftime(zone_str, 10, "%z", localtime(&now));
        zone_str[3] = 0;
    } else {
        strncpy(zone_str, input_zone, 3);
    }

    char flag_buf[2] = {0};
    strncpy(flag_buf, zone_str, 1);

    char zone_buf[3] = {0};
    strncpy(zone_buf, zone_str + 1, 2);

    int64_t zone = atoi(zone_buf);

    if (strcmp("+", flag_buf) == 0) {
        ts += (zone * HOUR_IN_SEC * MICRO_PER_SEC);
    } else {
        ts -= (zone * HOUR_IN_SEC * MICRO_PER_SEC);
    }

    return ts;
}

TIME_IN_MICRO simple_parse_iso8601(const char* str_utc_time) 
{
    // len (1970-01-01T08:00:00.000000+08) == 29
    int len = strlen(str_utc_time);
    ASSERT(
            (len == 29), 
            "input str time len(%d) != 29, input: %s", strlen(str_utc_time), str_utc_time
    );

    struct tm tm_;
    strptime(str_utc_time, "%Y-%m-%dT%H:%M:%S%Z", &tm_);
    tm_.tm_isdst = -1;
    // 
    time_t t_ = mktime(&tm_);
    ASSERT((t_ != -1), "mktime time fail, input time str: %s", str_utc_time);
    
    TIME_IN_MICRO ts = t_ * MICRO_PER_SEC;

    ts += simple_get_zone_diff(NULL);
    
    char mills_buf[7] = {0};
    strncpy(mills_buf, str_utc_time + 20, 6);

    int32_t mills = atoi(mills_buf);
    ts += mills;

    char zone_buf[4] = {0};
    strncpy(zone_buf, str_utc_time + 26, 3);

    ts -= simple_get_zone_diff(zone_buf);

    return ts;
}

