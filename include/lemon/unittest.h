// ----------------------------------------------------------------------------
// Lemon Unit Test FrameWork
//
// == 描述 ==
// 测试框架中分为3个模块，依次是Test、Suit和TestCase。
// Test
//     表示一个测试项目，如对公共模块的测试，建立一个公共模块的测试项目。
// Suit
//     表示一组测试组件，一般指代一个文件。比如公共模块中的链表，这是就可以建立一个链表的
// 测试组件。
// 
// TestCase
//     表示测试用例，一般所对具体方法的测试，如链表的add方法，可以建立一个对该方法的.
//
// 测试用例
//
// 注意：
//     系统不会检查Suite的名称，需要用户保证。如果Test中存在同名的Suite，在运行.
//
// unittest_suite_run_by_name方法时，系统只保证运行第一个Suite，并不会运行之后的Suite。
// ----------------------------------------------------------------------------

#ifndef LEMON_UNITTEST_H_
#define LEMON_UNITTEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef int (*SetupFunc)(void);
typedef int (*CleanupFunc)(void);
typedef void (*TestCaseFunc)(void);

#define UNITTEST_NAME_LEN (256)

typedef struct testcase_t
{
    char name[UNITTEST_NAME_LEN];
    int status; // 1 means succ, 0 means fail
    TestCaseFunc func;
    struct testcase_t* next;
} TestCase;

typedef struct suite_t
{
    char name[UNITTEST_NAME_LEN];
    SetupFunc setup_func;
    CleanupFunc cleanup_func;
    TestCase* cases;
    int total_case;
    int fail_case;
    struct suite_t* next;
} Suite;

typedef struct test_t
{
    int64_t start_time;
    int64_t cur_time;
    int total_case;
    int fail_case;
    Suite* suites;
} Test;

void simple_unittest_init();

int  simple_unittest_suite_run_by_name(const char* suite_name);

int  simple_unittest_suite_run_all();

void simple_unittest_suite_init(const char* suite_name);

int  simple_unittest_suite_setup(const char* suite_name, SetupFunc func);

int  simple_unittest_suite_cleanup(const char* suite_name, CleanupFunc func);

int  simple_unittest_suite_add_testcase(
         const char* suite_name,
         const char* case_name,
         TestCaseFunc func);

void simple_unittest_free();

void simple_unittest_set_total_size(int num);

void simple_unittest_set_flag(int flag);

void simple_unittest_set_error(char* msg);

#define UNITTEST_SUITE_INIT() { \
    simple_unittest_suite_init(__FILE__); \
}

#define UNITTEST_SUITE_SETUP(setup_func) { \
    simple_unittest_suite_setup(__FILE__, setup_func); \
}

#define UNITTEST_SUITE_CLEANUP(cleanup_func) { \
    simple_unittest_suite_cleanup(__FILE__, cleanup_func); \
}

#define UNITTEST_SUITE_ADD_TESTCASE(case_name, func) { \
    simple_unittest_suite_add_testcase(__FILE__, case_name, func); \
}

#define TEST_TRUE(value) { \
    if (!value) {\
        char simple_test_equal_string_buf[256] = {0};\
        sprintf(simple_test_equal_string_buf, "expect True but False [%d,%s]",__LINE__, __FUNCTION__);\
        simple_unittest_set_flag(0);\
        simple_unittest_set_error(simple_test_equal_string_buf);\
        return;\
    } \
}

#define TEST_FALSE(value) { \
    if (value) {\
            char simple_test_equal_string_buf[256] = {0};\
            sprintf(simple_test_equal_string_buf, "expect False but True [%d,%s]",__LINE__, __FUNCTION__);\
            simple_unittest_set_flag(0);\
            simple_unittest_set_error(simple_test_equal_string_buf);\
            return;\
    } \
}

#define TEST_EQUAL_INT(expect,target) { \
    int64_t unitttest_target_int = target; \
    int64_t unitttest_expect_int = expect; \
    if (unitttest_expect_int != unitttest_target_int) {\
        char simple_test_equal_string_buf[256] = {0};\
        sprintf(simple_test_equal_string_buf, "Not equal,  expect = %ld, target = %ld [%d,%s]", unitttest_expect_int, unitttest_target_int, __LINE__, __FUNCTION__);\
        simple_unittest_set_flag(0);\
        simple_unittest_set_error(simple_test_equal_string_buf);\
        return;\
    } \
}

#define TEST_EQUAL_DOUBLE(expect,target) { \
    double unitttest_target_double = target; \
    double unitttest_expect_double = expect; \
    if (unitttest_expect_double != (unitttest_target_double)) {\
        char simple_test_equal_string_buf[256] = {0};\
        sprintf(simple_test_equal_string_buf, "Not equal,  expect = %f, target = %f [%d,%s]", unitttest_expect_double, unitttest_target_double, __LINE__, __FUNCTION__);\
        simple_unittest_set_flag(0);\
        simple_unittest_set_error(simple_test_equal_string_buf);\
        return;\
    } \
}

#define TEST_EQUAL_STRING(expect,target) {\
    const char* unitttest_target_void = target; \
    if ((void*)expect == NULL || (void*)unitttest_target_void == NULL){\
        if((void*)expect != (void*)target){\
            char simple_test_equal_string_buf[256] = {0};\
            if((void*)expect != NULL){\
                sprintf(simple_test_equal_string_buf, "Not equal,  expect = %s, target = NULL [%d,%s]", expect, __LINE__, __FUNCTION__);\
                simple_unittest_set_flag(0);\
                simple_unittest_set_error(simple_test_equal_string_buf);\
            }else{\
                sprintf(simple_test_equal_string_buf, "Not equal,  expect = NULL, target = %s [%d,%s]", unitttest_target_void, __LINE__, __FUNCTION__);\
                simple_unittest_set_flag(0);\
                simple_unittest_set_error(simple_test_equal_string_buf);\
            }\
            return;\
        }\
    }else if (strcmp((expect),(target)) != 0) {\
        char simple_test_equal_string_buf[256] = {0};\
        sprintf(simple_test_equal_string_buf, "Not equal,  expect = %s, target = %s [%d,%s]", expect, unitttest_target_void, __LINE__, __FUNCTION__);\
        simple_unittest_set_flag(0);\
        simple_unittest_set_error(simple_test_equal_string_buf);\
        return;\
    } \
}

#define UNITTEST_REGISTER() \
    int main(int argc, char* argv[])  \
    {  \
        int ret = 0;  \
        simple_unittest_init();  \
        UNITTEST_SET_ALL_SUITE();  \
        if (argc > 1)  \
        {  \
            simple_unittest_set_total_size(argc - 1);  \
            for (int i = 1; i < argc; i++)  \
            {  \
                ret = simple_unittest_suite_run_by_name(argv[i]);  \
            }  \
        }  \
        else  \
        {  \
            ret = simple_unittest_suite_run_all();  \
        }  \
        simple_unittest_free();  \
        return ret;  \
    }


#endif
