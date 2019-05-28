#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <PCUnit/PCUnit.h>

#include "pcunit_unittests.h"

#include "ele_task.h"
#include "ele_error.h"
#include "queue.h"

#include <stdio.h>
#include <unistd.h>

#define N_ENQUEUE_THREADS 10
static void test_example01(void) {
    queue_t *q = create_queue(N_ENQUEUE_THREADS);
    static char *data[] = {
            "To be or",
            "not to be;",
            "that is the question.",
            "END",
            ""
    };
    printf("start queuing\n");
    for (int i = 0; i < 3; i++) {
        const char *w = data[i];
        char *p_data = malloc(strlen(w) + 1);
        strncpy(p_data, w, strlen(w)+1);
        enqueue(q, p_data);
        //PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    }
    printf("end queuing\n");
    printf("start dequeuing\n");
    for (int i = 0; i < 3; i++) {
        char* p_data = dequeue(q);
        //PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
        printf("%s\n", p_data);
        free(p_data);
    }
    printf("end dequeuing\n");

}

#define MAX_VALUE   10
static int is_end(const char * const word) {
    int value = atoi(word);
    return (value >= MAX_VALUE) ? 0 : 1;
}

static void * sender(void * arg) {
    queue_t *q = arg;

    srand(10);
    int n = MAX_VALUE;
    const int DIGIT = 10;
    for (int i=0; i<=n; i++) {
        char *p_data = malloc(DIGIT + 1);
        sprintf(p_data, "%*d", DIGIT, i);
        enqueue(q, p_data);

        usleep(rand() % 200);
    }

    printf("sender completed\n");
    fflush(stdout);

    return NULL;
}

static void * receiver(void * arg) {
    queue_t *q = arg;

    char* p_data = NULL;
    while((p_data = dequeue(q)) != NULL) {
        char word[32];
        strcpy(word, p_data);
        free(p_data);
        if (atoi(word) >= MAX_VALUE) {
            break;
        } else {
            printf("%s ", word);
            fflush(stdout);
        }

        usleep(rand() % 200);
    }
    printf("receiver completed\n");
    fflush(stdout);

    return NULL;
}

static void test_example02(void)
{
    static const int SENDER_ID =    100;
    static const int RECEIVER_ID =  101;

    queue_t *q = create_queue(N_ENQUEUE_THREADS);

	ele_task_init_attr_t attr = {
		.id = SENDER_ID,
		.schedpolicy = SCHED_OTHER,
		.schedparam = 0,
		.start_routine_entry = sender,
		.start_routine_arg = q
	};
    attr.id = RECEIVER_ID;
    attr.start_routine_entry = receiver;
    if (ele_task_create(attr, ELE_TASK_NO_WAIT) != ELE_SUCCESS) {
        fprintf(stderr, "receive creation error\n");
    }
    attr.id = SENDER_ID;
    attr.start_routine_entry = sender;
	if (ele_task_create(attr, ELE_TASK_NO_WAIT) != ELE_SUCCESS) {
		fprintf(stderr, "sender creation error\n");
	}

//    printf("all threads start\n");
//    ele_task_start_all();

    ele_task_join(SENDER_ID);
    ele_task_join(RECEIVER_ID);
    destroy_queue(q);
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
