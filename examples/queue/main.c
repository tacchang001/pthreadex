#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ele_task.h"
#include "ele_error.h"
#include "queue.h"

#include <stdio.h>
#include <unistd.h>


#define MAX_VALUE   10000
#define DISPLAY_STEP    (MAX_VALUE / 10)


void * sender(void * arg) {
    struct threadqueue *q = arg;

    srand(10);
    int n = MAX_VALUE;
    const int DIGIT = 10;
    for (int i=0; i<=n; i++) {
        char *p_data = malloc(DIGIT + 1);
        sprintf(p_data, "%0*d", DIGIT, i);
        thread_queue_add(q, p_data, 1);

        int interval = rand() % 200;
        if ((i%DISPLAY_STEP) == 0) printf("[%d]sender wait %dus ...\n", i, interval);
        usleep(interval);
    }

    printf("sender completed\n");

    return NULL;
}

void * receiver(void * arg) {
    struct threadqueue *q = arg;

    struct threadmsg msg;
    struct timespec timeout = {
            .tv_sec = 1,
            .tv_nsec = 000
    };
    int result = 0;
    while((result = thread_queue_get(q, &timeout, &msg)) == 0) {
        char word[32];
        strcpy(word, msg.data);
        free(msg.data);
        int a = atoi(word);
        if (a >= MAX_VALUE) {
            printf("%d >= MAX_VALUE\n", a);
            break;
        }

        if ((a%DISPLAY_STEP) == 0) {
            printf("[%d][%s]receiver wait ...\n", a, word);
        }

        int interval = rand() % 200;
        usleep(interval);
    }

    printf("failure:%d\n", result);
    printf("receiver completed\n");

    return NULL;
}

static void test_example02(void)
{
    static const int SENDER_ID =    100;
    static const int RECEIVER_ID =  101;

    struct threadqueue q;
    int actual = thread_queue_init(&q);

    ele_task_init_attr_t attr = {
            .id = SENDER_ID,
            .schedpolicy = SCHED_OTHER,
            .schedparam = 0,
            .start_routine_entry = sender,
            .start_routine_arg = &q
    };
    attr.id = RECEIVER_ID;
    attr.start_routine_entry = receiver;
    if (ele_task_create(attr, ELE_TASK_WAIT) != ELE_SUCCESS) {
        fprintf(stderr, "receive creation error\n");
    } else {
        ele_task_display_pthread_attr(attr.id);
    }
    attr.id = SENDER_ID;
    attr.start_routine_entry = sender;
    if (ele_task_create(attr, ELE_TASK_WAIT) != ELE_SUCCESS) {
        fprintf(stderr, "sender creation error\n");
    } else {
        ele_task_display_pthread_attr(attr.id);
    }

    printf("all threads start\n");
    ele_task_start_all();

    ele_task_join(SENDER_ID);
    ele_task_join(RECEIVER_ID);
    thread_queue_cleanup(&q, 1);
}

int main(int argc, char* argv[]) {
    test_example02();
}
