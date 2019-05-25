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


static const useconds_t us1000 = 1000;

static void
ele_cleanup_tsk1(void *arg)
{
	printf("task clean end\n");
	return ;
}

void * anything(void * arg) {
	pthread_cleanup_push(ele_cleanup_tsk1, NULL);

	int deadline = *(int*)arg;
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

static void test_example01(void) {
    const int TASK_ID = 100;

    int a = 10;
	ele_task_init_attr_t attr = {
		.id = TASK_ID,
		.schedpolicy = SCHED_OTHER,
		.schedparam = 0,
		.entry = anything,
		.arg = &a
	};

    ele_result_t actual = ele_task_create(attr);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);

//    useconds_t working_time = 5 * us1000;
//    usleep(working_time);
    dft(100);

    actual = ele_task_destroy(TASK_ID);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);

}

PCU_Suite *TaskTest_suite(void)
{
    static PCU_Test tests[] = {
            { "single task", test_example01 },
    };
    static PCU_Suite suite = {
            "ExampleTest", tests, sizeof(tests) / sizeof(tests[0]) };
    return &suite;
}
