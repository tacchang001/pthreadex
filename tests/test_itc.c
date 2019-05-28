#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <PCUnit/PCUnit.h>

#include "pcunit_unittests.h"

#include "ele_task.h"
#include "ele_error.h"
#include "ele_itc.h"

#include <stdio.h>
#include <unistd.h>

static const int TASK_ID = 100;

static void * anything(void * arg) {
	int value = *(int*)arg;

	printf("value = %d\n", value);

	return arg;
}

static void test_example01(void)
{
	int a = 10;
	ele_task_init_attr_t attr = {
		.id = TASK_ID,
		.schedpolicy = SCHED_OTHER,
		.schedparam = 0,
		.start_routine_entry = anything,
		.start_routine_arg = &a
	};

	if (ele_task_create(attr, ELE_TASK_NO_WAIT) != ELE_SUCCESS) {
		fprintf(stderr, "ele_tsk_create error\n");
	}

	ele_task_join(TASK_ID);
}

PCU_Suite *ItcTest_suite(void)
{
	static PCU_Test tests[] = {
		{ "itc 01", test_example01 },
	};
	static PCU_Suite suite = {
		"Inter Thread Communication Test", tests, sizeof tests / sizeof tests[0] };
	return &suite;
}
