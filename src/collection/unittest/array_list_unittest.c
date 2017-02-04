#include "collection/array_list.h"
#include "unittest.h"
#include "assert.h"

#include <stdio.h>
#include <stdbool.h>

void test_arraylist_op();
void test_arraylist_invalid();

void UNITTEST_SET_ALL_SUITE()
{
    UNITTEST_SUITE_INIT();
    UNITTEST_SUITE_SETUP(NULL);
    UNITTEST_SUITE_ADD_TESTCASE("test_arraylist_op", test_arraylist_op);
    UNITTEST_SUITE_ADD_TESTCASE("test_arraylist_invalid", test_arraylist_invalid);
}

void test_arraylist_op()
{
    char* data[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8"};
    ArrayList* list = arraylist_new(2);

    arraylist_add(list, data[0]);
    arraylist_add(list, data[1]);
    arraylist_add(list, data[2]);
    arraylist_add(list, data[3]);

    TEST_EQUAL_STRING(data[0], arraylist_get(list, 0));
    TEST_EQUAL_STRING(data[1], arraylist_get(list, 1));
    TEST_EQUAL_STRING(data[2], arraylist_get(list, 2));
    TEST_EQUAL_STRING(data[3], arraylist_get(list, 3));

    TEST_EQUAL_INT(4, arraylist_size(list));

    arraylist_remove(list, 1);

    TEST_EQUAL_INT(3, arraylist_size(list));

    TEST_EQUAL_STRING(data[0], arraylist_get(list, 0));
    TEST_EQUAL_STRING(data[2], arraylist_get(list, 1));
    TEST_EQUAL_STRING(data[3], arraylist_get(list, 2));

    arraylist_clear(list);
    
    TEST_EQUAL_INT(0, arraylist_size(list));

    arraylist_reset(list, 2);
    arraylist_reset(list, 6);
    
    TEST_EQUAL_INT(0, arraylist_size(list));

    arraylist_add(list, data[4]);
    arraylist_add(list, data[5]);
    arraylist_add(list, data[6]);
    arraylist_add(list, data[7]);

    TEST_EQUAL_INT(4, arraylist_size(list));
    
    TEST_EQUAL_STRING(data[4], arraylist_get(list, 0));
    TEST_EQUAL_STRING(data[5], arraylist_get(list, 1));
    TEST_EQUAL_STRING(data[6], arraylist_get(list, 2));
    TEST_EQUAL_STRING(data[7], arraylist_get(list, 3));
}

void test_arraylist_invalid()
{

}

UNITTEST_REGISTER();
