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

static int is_end(const char * const word) {
    static char* endian = "END";
    return strncmp(word, endian, strlen(endian));
}

static void test_example01(void) {
    size_t page_size = 256;
    ele_queue_desc_t* q = ele_queue_create(page_size * 4);
    ele_queue_item_t *d = malloc(page_size);
    size_t data_length = page_size - sizeof(size_t);
    d->data_length = data_length;
    ele_result_t actual = ele_queue_push(q, d);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    actual = ele_queue_push(q, d);
    PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    ele_queue_destroy(q);
    free(d);
}

static void * sender(void * arg) {
    ele_queue_desc_t* q = (ele_queue_desc_t*)arg;

    static char *data[] = {
            "To be or",
            "not to be;",
            "that is the question.",
            "END"
    };
    size_t total_size = 256;
    ele_queue_item_t *d = malloc(total_size);
    size_t data_length = total_size - sizeof(size_t);
    d->data_length = data_length;

    srand(10);
    int i=0;
    while (is_end(data[i]) != 0) {
        const char* w = data[i++];
        strncpy(d->data, w, strlen(w));
        if (ele_queue_push(q, d) != ELE_SUCCESS) {
            break;
        }

        usleep(rand() % 200);
    }

    free(d);

	return NULL;
}

static void * receiver(void * arg) {
    ele_queue_desc_t* q = (ele_queue_desc_t*)arg;

    ele_queue_item_t d;
    while(ele_queue_pop(q, &d) == ELE_SUCCESS) {
        const char* w = d.data;
        if (is_end(w) == 0) {
            break;
        } else {
            printf("%s ", w);
        }

        usleep(rand() % 200);
    }
    printf("\n");

    return arg;
}

static void test_example02(void)
{
    static const int SENDER_ID =    100;
    static const int RECEIVER_ID =  101;

    ele_queue_desc_t* q = ele_queue_create(1024);

	ele_task_init_attr_t attr = {
		.id = SENDER_ID,
		.schedpolicy = SCHED_OTHER,
		.schedparam = 0,
		.start_routine_entry = sender,
		.start_routine_arg = q
	};
	if (ele_task_create(attr, ELE_TASK_WAIT) != ELE_SUCCESS) {
		fprintf(stderr, "sender creation error\n");
	}
	attr.id = RECEIVER_ID;
    attr.start_routine_entry = receiver;
    if (ele_task_create(attr, ELE_TASK_WAIT) != ELE_SUCCESS) {
        fprintf(stderr, "receive creation error\n");
    }

    printf("all threads start\n");
    ele_task_start_all();

    ele_task_join(SENDER_ID);
    ele_task_join(RECEIVER_ID);
    ele_queue_destroy(q);
}

PCU_Suite *ItcTest_suite(void)
{
	static PCU_Test tests[] = {
	        { "itc 01", test_example01 },
		    { "itc 02", test_example02 },
	};
	static PCU_Suite suite = {
		"Inter Thread Communication Test", tests, sizeof tests / sizeof tests[0] };
	return &suite;
}
