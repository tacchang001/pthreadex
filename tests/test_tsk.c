#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <PCUnit/PCUnit.h>

#include "pcunit_unittests.h"

#include "ele_task.h"
#include "ele_error.h"

#include <stdio.h>
#include <unistd.h>

#include "misc.h"

// #define EXECUTE_WITH_ROOT

static const useconds_t us1000 = 1000;

static void
ele_cleanup_tsk1(void *arg) {
    printf("task clean end\n");
    return;
}

void *example01(void *arg) {
    int deadline = *(int *) arg;

    pthread_cleanup_push(ele_cleanup_tsk1, NULL);

    int cnt = 0;
    while (cnt < deadline) {
        pthread_testcancel(); /* A cancellation point */
        cnt++;
        usleep(us1000);
    }

    pthread_cleanup_pop(0);
    PCU_FAIL("forced termination");

    return NULL;
}

void *example02(void *arg) {
    printf("thread:%lx started\n", pthread_self());

    int times = *(int *) arg;
    dft(times);

    printf("thread:%lx ended\n", pthread_self());

    return NULL;
}

static void test_example01(void) {
    const int TASK_ID = 100;

    int a = 10;
    ele_task_init_attr_t attr = {
            .id = TASK_ID,
#ifdef EXECUTE_WITH_ROOT
    .schedpolicy = SCHED_FIFO,
    .schedparam = 50,
#else
            .schedpolicy = SCHED_OTHER,
            .schedparam = 0,
#endif
            .start_routine_entry = example01,
            .start_routine_arg = &a
    };

    ele_result_t actual = ele_task_create(attr, ELE_TASK_NO_WAIT);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
//    ele_task_display_pthread_attr(TASK_ID);

    useconds_t working_time = 5 * us1000;
    usleep(working_time);

    actual = ele_task_destroy(TASK_ID);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);

}

static void test_example02(void) {
    for (int i = 1; i < 100; i++) {
        int deadline = 1;
        ele_task_init_attr_t attr = {
    #ifdef EXECUTE_WITH_ROOT
        .schedpolicy = SCHED_FIFO,
        .schedparam = 50,
    #else
                .schedpolicy = SCHED_OTHER,
                .schedparam = 0,
    #endif
                .start_routine_entry = example02,
                .start_routine_arg = &deadline
        };

        attr.id = i;
        ele_result_t actual = ele_task_create(attr, ELE_TASK_NO_WAIT);
        PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);

        usleep(20);

        actual = ele_task_join(i);
        PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    }
}

static void test_example03(void) {
    const int TASK_ID1 = 100;
    const int TASK_ID2 = 101;
    const int TASK_ID3 = 102;

    int times = 1000;
    ele_task_init_attr_t attr = {
            .id = TASK_ID1,
#ifdef EXECUTE_WITH_ROOT
    .schedpolicy = SCHED_FIFO,
    .schedparam = 50,
#else
            .schedpolicy = SCHED_OTHER,
            .schedparam = 0,
#endif
            .start_routine_entry = example02,
            .start_routine_arg = &times
    };

    ele_result_t actual = ele_task_create(attr, ELE_TASK_WAIT);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    attr.id = TASK_ID2;
    actual = ele_task_create(attr, ELE_TASK_WAIT);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    attr.id = TASK_ID3;
    actual = ele_task_create(attr, ELE_TASK_WAIT);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);

    printf("all threads start\n");
    ele_task_start_all();

    actual = ele_task_join(TASK_ID1);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    actual = ele_task_join(TASK_ID2);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    actual = ele_task_join(TASK_ID3);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
}

PCU_Suite *TaskTest_suite(void) {
    static PCU_Test tests[] = {
            {"single task", test_example01},
            {"many threads", test_example02},
            {"pthread_cond_broadcast", test_example03},
    };
    static PCU_Suite suite = {
            "ExampleTest", tests, sizeof(tests) / sizeof(tests[0])};
    return &suite;
}
