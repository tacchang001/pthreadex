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

static void test_example01(void) {
    struct threadqueue q;
    int actual = thread_queue_init(&q);
    PCU_ASSERT_EQUAL(0, actual);
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
        int actual = thread_queue_add(&q, p_data, 1);;
        PCU_ASSERT_EQUAL(ELE_SUCCESS, actual);
    }
    printf("end queuing\n");
    printf("start dequeuing\n");
    for (int i = 0; i < 3; i++) {
        struct threadmsg msg;
        struct timespec timeout = {
                .tv_sec = 0,
                .tv_nsec = 5000
        };
        int actual = thread_queue_get(&q, &timeout, &msg);
        PCU_ASSERT_EQUAL(0, actual);
        printf("%s\n", (char*)msg.data);
        free(msg.data);
    }
    printf("end dequeuing\n");

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
