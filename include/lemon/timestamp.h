#ifndef LEMON_TIMESTAMP_H_
#define LEMON_TIMESTAMP_H_

#include <stdint.h>

typedef int64_t TIME_IN_SEC;      // seconds
typedef int64_t TIME_IN_MILLI;    // millisecond
typedef int64_t TIME_IN_MICRO;    // microsecond

#define MICRO_PER_SEC  1000000
#define MILLI_PER_SEC  1000
#define NANO_PER_MICRO 1000

#define DAY_IN_SEC     86400
#define DAY_IN_MILLI   86400000
#define DAY_IN_MICRO   86400000000

#define HOUR_IN_SEC    3600
#define HOUR_IN_MILLI  3600000
#define HOUR_IN_MICRO  3600000000

TIME_IN_MICRO simple_monotonic_time_now();

TIME_IN_MICRO simple_real_time_now();

void simple_now_iso8601(/*out*/char out[]);

void simple_timestamp_to_iso8601(TIME_IN_MICRO ts,/*out*/char out[]);

// ISO8601: 2015-11-29T13:12:25.494+08
TIME_IN_MICRO simple_parse_iso8601(const char* iso8601Time);

// UTC: 2015-12-12T00:00:00.000
//TIME_IN_MICRO simple_parse(const char* utcTime);

#endif /* TIMESTAMP_H_ */
