#include "timestamp.h"
#include "unittest.h"
#include "assert.h"

#include <stdio.h>
#include <stdbool.h>

void test_simple_timestamp_to_iso8601();

void UNITTEST_SET_ALL_SUITE()
{
    UNITTEST_SUITE_INIT();
    UNITTEST_SUITE_SETUP(NULL);
    UNITTEST_SUITE_ADD_TESTCASE("test_simple_timestamp_to_iso8601", test_simple_timestamp_to_iso8601);
}

void test_simple_timestamp_to_iso8601()
{
    // Local
    TIME_IN_MICRO ts = 1485958251050749;
    char buffer[128] = {0};
    simple_timestamp_to_iso8601(ts, buffer);

    TIME_IN_MICRO new_ts = simple_parse_iso8601(buffer);
    char new_buffer[128] = {0};
    simple_timestamp_to_iso8601(new_ts, new_buffer);
    
    TEST_EQUAL_INT(ts, new_ts);
    TEST_EQUAL_STRING((char*)buffer, (char*)new_buffer);

    // UTC
    // 2017-02-01T00:00:00.000000+00 = 1485907200000000
    TIME_IN_MICRO utc_ = 1485907200000000;
    const char* utc_str = "2017-02-01T00:00:00.000000+00";
    TIME_IN_MICRO utc_ts = simple_parse_iso8601(utc_str);
    char out_utc_str[128] = {0};
    simple_timestamp_to_iso8601(utc_, out_utc_str);
    TEST_EQUAL_INT(utc_, utc_ts);
}

UNITTEST_REGISTER();
