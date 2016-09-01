#include "memory_pool.h"
#include "unittest.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void test_simple_memory_pool_base();

void UNITTEST_SET_ALL_SUITE()
{
    UNITTEST_SUITE_INIT();
    UNITTEST_SUITE_SETUP(NULL);
    UNITTEST_SUITE_ADD_TESTCASE("test_simple_memory_pool_base", test_simple_memory_pool_base);
}

void test_simple_memory_pool_base()
{
    SimplePool* pool = simple_pool_create(1024);
    void* data = simple_pool_malloc(pool, 200);
    strcpy(data, "hello world");
    printf("%s\n", (char*)data);
    simple_pool_destory(pool);
}

UNITTEST_REGISTER();
