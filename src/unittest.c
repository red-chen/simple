#include <unittest.h>
#include <timestamp.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>

static Test* testInst = NULL;

static char error[512] = { 0 };

static int ut_flag = 0;

static int simple_unittest_suite_run(Suite* suite);

#define ADD_SUITE_TO_LIST_TAIL(suite_list, suite)\
        if (suite_list == NULL){\
            suite_list = suite;\
        }else{\
            Suite* suite_index = suite_list; \
            while (suite_index->next != NULL) { \
                suite_index = suite_index->next; \
            } \
            suite_index->next = suite;\
        }

void simple_unittest_init()
{
    testInst = malloc(sizeof(Test));
    bzero(testInst, sizeof(Test));
    testInst->total_case = 0;
    testInst->fail_case = 0;
    testInst->start_time = simple_monotonic_time_now();
    testInst->cur_time = testInst->start_time;
    testInst->suites = NULL;

    printf("\n");
    printf("====================================================\n");
    printf("=               Unit Test Framework                =\n");
    printf("====================================================\n\n");
}

int simple_unittest_suite_run_by_name(const char* suite_name)
{
    int ret = 0;
    Suite* index = testInst->suites;
    while (index != NULL)
    {
        if (strcmp(index->name, suite_name) == 0)
        {
            ret = simple_unittest_suite_run(index);
            break;
        }
        index = index->next;
    }
    return ret;
}

int simple_unittest_suite_run_all()
{
    int ret = 0;
    Suite* index = testInst->suites;
    while (index != NULL)
    {
        if (simple_unittest_suite_run(index) != 0)
        {
            ret = -1;
        }
        index = index->next;
    }
    return ret;
}

void simple_unittest_suite_init(const char* suite_name)
{
    Suite* suite = malloc(sizeof(Suite));
    bzero(suite, sizeof(Suite));
    strcpy(suite->name, suite_name);
    suite->setup_func = NULL;
    suite->cleanup_func = NULL;
    suite->cases = NULL;

    ADD_SUITE_TO_LIST_TAIL(testInst->suites, suite);
}

int simple_unittest_suite_setup(const char* suite_name, SetupFunc func)
{
    int ret = 0;
    Suite* index = testInst->suites;
    while (index != NULL)
    {
        if (strcmp(index->name, suite_name) == 0)
        {
            index->setup_func = func;
            ret = 1;
            break;
        }
        index = index->next;
    }
    return ret;
}

int simple_unittest_suite_cleanup(const char* suite_name, CleanupFunc func)
{
    int ret = 0;
    Suite* index = testInst->suites;
    while (index != NULL)
    {
        if (strcmp(index->name, suite_name) == 0)
        {
            index->cleanup_func = func;
            ret = 1;
            break;
        }
        index = index->next;
    }
    return ret;
}

#define ADD_TESTCAAE_TO_SUITE(test_case_list, test_case) \
    if(test_case_list == NULL) {\
        test_case_list = test_case;\
    }else{\
        TestCase* test_case_index = test_case_list;\
        while(test_case_index->next != NULL){\
            test_case_index = test_case_index->next;\
        } \
        test_case_index->next = test_case; \
    }


int simple_unittest_suite_add_testcase(const char* suite_name, const char* case_name,
        TestCaseFunc func)
{
    int ret = 0;
    Suite* index = testInst->suites;
    while (index != NULL)
    {
        if (strcmp(index->name, suite_name) == 0)
        {
            TestCase* tc = malloc(sizeof(TestCase));
            bzero(tc, sizeof(TestCase));
            strcpy(tc->name, case_name);
            tc->func = func;
            tc->status = 0;

            ADD_TESTCAAE_TO_SUITE(index->cases, tc);

            ret = 1;
            testInst->total_case++;
            index->total_case++;
            break;
        }
        index = index->next;
    }
    return ret;
}

void simple_unittest_free()
{
    int total_time = testInst->cur_time - testInst->start_time;
    int pass = testInst->total_case - testInst->fail_case;
    printf(" Total case passed : %d/%d\n", pass, testInst->total_case);
    printf(" Total time spend  : %d micro-seconds\n\n", total_time);
}

void simple_unittest_set_flag(int flag)
{
    ut_flag = flag;
}

void simple_unittest_set_error(char* msg)
{
    bzero(error, 256);
    strcpy(error, msg);
}

void simple_unittest_set_total_size(int num)
{
    testInst->total_case = num;
}

static int simple_unittest_suite_run(Suite* suite)
{
    printf(" Unit test for Suite : %s \n", suite->name);
    int fail_count = 0;
    TestCase* index = suite->cases;
    while (index != NULL)
    {
        if (suite->setup_func)
        {
            suite->setup_func();
        }
        simple_unittest_set_flag(1);
        testInst->cur_time = simple_monotonic_time_now();
        index->func();
        int64_t tmp = simple_monotonic_time_now();
        if (ut_flag == 1)
        {
            if(isatty(stdout->_fileno) == 1){
                printf("\e[32m PASS \e[37m\x1b[0m , %s , %"PRId64" micro-seconds\n",
                    index->name, (tmp - testInst->cur_time));
            }
            else{
                printf(" PASS , %s , %"PRId64" micro-seconds\n",
                                    index->name, (tmp - testInst->cur_time));
            }
            testInst->cur_time = tmp;
        }
        else
        {
            if(isatty(stdout->_fileno) == 1){
                printf("\e[31m FAIL \e[37m\x1b[0m , %s , \"%s\"\n", index->name,
                    error);
            }else{
                printf(" FAIL  , %s , \"%s\"\n", index->name, error);
            }
            fail_count++;
            testInst->fail_case++;
        }
        if (suite->cleanup_func)
        {
            suite->cleanup_func();
        }
        index = index->next;
    }

    suite->fail_case = fail_count;

    printf(" Suite case passed: %d/%d\n\n",
            (suite->total_case - suite->fail_case), suite->total_case);

    return (suite->fail_case > 0) ? -1 : 0;
}
