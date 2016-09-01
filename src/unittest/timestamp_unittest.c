#include "timestamp.h"
#include "unittest.h"

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
    TIME_IN_MICRO ts = 1448776474450749;
    char buffer[128] = {0};
    simple_timestamp_to_iso8601(ts, buffer);
    TEST_EQUAL_STRING("2015-11-29T13:54:34.450749+08", (char*)buffer);
}

UNITTEST_REGISTER();
