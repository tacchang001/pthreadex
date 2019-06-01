/*
 * https://l1o0.hatenadiary.org/entry/20100123/1264242629
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#define TRUE  1
#define FALSE 0

#define E_OK       0    /* success */
#define E_NOEXS    (-2) /* no objcet  */
#define E_SYS      (-6) /* system error */
#define E_PAR      (-7) /* parameter error */

#define NUM_OF_TIMER   3
#define TIMER_INTERVAL 10 /* msec */

/* macro */
#define NELEMS(array) (sizeof(array) / sizeof(array[0]))

typedef struct TimerId {
    u_int index;
    u_int sequence_number;
} TimerId;

typedef void  (*timer_callback)(void *, int);

typedef struct TimerInfo {
    int             use;
    int             timeout;
    u_int           sequence_number;
    timer_callback  callback;
    void           *data;
    u_int           len;
} TimerInfo;

typedef struct TimerTable {
    pthread_mutex_t mutex;
    u_int           sequence_number;
    TimerInfo       timer_info[NUM_OF_TIMER];
} TimerTable;


int do_timer_callback(TimerInfo *timer_info, u_int index);
int calc_elapsed(struct timeval *tv_before, struct timeval *tv_after);
int init_timer(pthread_t *thread);
int create_timer(TimerId *timer_id);
int start_timer(TimerId *timer_id,
                int timeout, timer_callback callback, void *data, u_int len);
int stop_timer_no_mutex(TimerId *timer_id);
int stop_timer(TimerId *timer_id);
int dump_timer(FILE *fp);
char *test_print_msec(char *buf);
void test_callback(void *data, int len);

/* global */
TimerTable g_timer_table;

int do_timer_callback(TimerInfo *timer_info, u_int index)
{
    TimerId timer_id;
    int     ercd;

    timer_info->callback(timer_info->data, timer_info->len);

    timer_id.index           = index;
    timer_id.sequence_number = timer_info->sequence_number;
    ercd                     = stop_timer_no_mutex(&timer_id);

    return ercd;
}

/* milli sec */
int calc_elapsed(struct timeval *tv_before, struct timeval *tv_after)
{
    int sec;
    int usec;

    sec  = tv_after->tv_sec - tv_before->tv_sec;
    usec = tv_after->tv_usec - tv_before->tv_usec;

    return (sec * 1000) + (usec / 1000);
}

void *timer_thread(void *arg)
{
    struct timespec  ts;
    struct timespec  rem;
    struct timeval   tv_before;
    struct timeval   tv_after;
    int              i;
    TimerInfo       *timer_info;
    int              elapsed; /* milli sec */
/*     char             buf[256]; */

    elapsed    = 0;
    ts.tv_sec  = 0;
    ts.tv_nsec = TIMER_INTERVAL * 1000 * 1000;

    while(1) {
        while (EINTR == nanosleep(&ts, &rem)) {
            ts.tv_sec  = rem.tv_sec;
            ts.tv_nsec = rem.tv_nsec;
        }

        gettimeofday(&tv_before, NULL);
        pthread_mutex_lock(&(g_timer_table.mutex));

        for (i = 0; i < NELEMS(g_timer_table.timer_info); i++) {
            timer_info = &(g_timer_table.timer_info[i]);
            if (TRUE == timer_info->use) {
                timer_info->timeout -= TIMER_INTERVAL - elapsed;
                if (timer_info->timeout <= 0) {
                    do_timer_callback(timer_info, i);
                }
            }
        }

        pthread_mutex_unlock(&(g_timer_table.mutex));

        gettimeofday(&tv_after, NULL);
        elapsed = calc_elapsed(&tv_before, &tv_after);
        if (elapsed > TIMER_INTERVAL) {
            fprintf(stderr, "!!! timer handler elapsed %d over %d !!!\n",
                    elapsed, TIMER_INTERVAL);
            elapsed = TIMER_INTERVAL;
        }
        ts.tv_sec  = 0;
        ts.tv_nsec = (TIMER_INTERVAL - elapsed) * 1000 * 1000;
/*         printf("timer tick end   %s elapsed = %d\n", */
/*                test_print_msec(buf), elapsed); */
    }
}

int init_timer(pthread_t *thread)
{
    int            i;
    pthread_attr_t attr;

#ifdef __CYGWIN32__
    g_timer_table.mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
    pthread_mutexattr_t mutex_attr;

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&(g_timer_table.mutex), &mutex_attr);
#endif

    g_timer_table.sequence_number = 0;

    for (i = 0; i < NELEMS(g_timer_table.timer_info); i++) {
        g_timer_table.timer_info[i].use             = FALSE;
        g_timer_table.timer_info[i].sequence_number = 0;
        g_timer_table.timer_info[i].callback        = NULL;
        g_timer_table.timer_info[i].data            = NULL;
        g_timer_table.timer_info[i].len             = 0;
    }

    pthread_attr_init(&attr);
    pthread_create(thread, &attr, &timer_thread, NULL);
    return E_OK;
}

int create_timer(TimerId *timer_id)
{
    int i;
    int ercd;

    ercd = E_NOEXS;
    for (i = 0; i < NELEMS(g_timer_table.timer_info); i++) {
        if (FALSE == g_timer_table.timer_info[i].use) {
            timer_id->index          = i;
            timer_id->sequence_number = g_timer_table.sequence_number;

            g_timer_table.timer_info[i].use = TRUE;
            g_timer_table.timer_info[i].sequence_number =
                    g_timer_table.sequence_number;
            g_timer_table.sequence_number++;
            ercd = E_OK;
            break;
        }
    }

    return ercd;
}

int start_timer(TimerId *timer_id,
                int timeout, timer_callback callback, void *data, u_int len)
{
    int        ercd;
    TimerInfo *timer_info;

    if (timeout <= TIMER_INTERVAL) {
        fprintf(stderr, "timeout too small : %d %d\n", timeout, TIMER_INTERVAL);
        return E_PAR;
    }

    pthread_mutex_lock(&(g_timer_table.mutex));

    ercd = create_timer(timer_id);
    if (E_OK == ercd) {
        timer_info           = &(g_timer_table.timer_info[timer_id->index]);
        timer_info->callback = callback;
        timer_info->len      = len;
        timer_info->timeout  = timeout;
        if ((len > 0) && (NULL != data)) {
            timer_info->data = malloc(len);
            if (NULL == timer_info->data) {
                stop_timer_no_mutex(timer_id);
                ercd = E_SYS;
            }
            else {
                memcpy(timer_info->data, data, len);
                ercd = E_OK;
            }
        }
    }

    pthread_mutex_unlock(&(g_timer_table.mutex));

    return ercd;
}

int stop_timer_no_mutex(TimerId *timer_id)
{
    int        ercd;
    TimerInfo *timer_info;

    if (NUM_OF_TIMER <= timer_id->index) {
        return E_PAR;
    }

    timer_info = &(g_timer_table.timer_info[timer_id->index]);
    if (FALSE == timer_info->use) {
        ercd = E_NOEXS;
    }
    else if (timer_id->sequence_number != timer_info->sequence_number) {
        ercd = E_NOEXS;
    }
    else {
        if ((timer_info->len > 0) && (NULL != timer_info->data)) {
            free(timer_info->data);
            timer_info->data = NULL;
        }
        timer_info->use      = FALSE;
        timer_info->timeout  = 0;
        timer_info->callback = NULL;
        timer_info->len      = 0;

        ercd= E_OK;
    }

    return ercd;
}

int stop_timer(TimerId *timer_id)
{
    int ercd;

    pthread_mutex_lock(&(g_timer_table.mutex));
    ercd = stop_timer_no_mutex(timer_id);
    pthread_mutex_unlock(&(g_timer_table.mutex));

    return ercd;
}

int dump_timer(FILE *fp)
{
    int        i;
    TimerInfo *timer_info;

    if (NULL == fp) {
        fp = stdout;
    }

    fprintf(fp, "sequence_number = %d\n", g_timer_table.sequence_number);
    for (i = 0; i < NELEMS(g_timer_table.timer_info); i++) {
        fprintf(fp, "----\n");
        timer_info = &(g_timer_table.timer_info[i]);
        fprintf(fp, "use             = %d\n", timer_info->use);
        fprintf(fp, "timeout         = %d\n", timer_info->timeout);
        fprintf(fp, "sequence_number = %d\n", timer_info->sequence_number);
        fprintf(fp, "callback        = %p\n", timer_info->callback);
        fprintf(fp, "data            = %p\n", timer_info->data);
        fprintf(fp, "len             = %d\n", timer_info->len);
    }

    return E_OK;
}

char *test_print_msec(char *buf)
{
    struct timeval tv;
    struct tm      tm;

    gettimeofday(&tv, NULL);
    localtime_r(&(tv.tv_sec), &tm);

    sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d.%03ld",
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            tv.tv_usec / 1000);

    return buf;
}

void test_callback(void *data, int len)
{
    char buf[256];
    test_print_msec(buf);
    printf("\nexpired = %s, data = %d, len = %d\n", buf, *(int *)data, len);
    dump_timer(NULL);
}

int main(int argc, char *argv[])
{
    TimerId   timer_id[NUM_OF_TIMER + 1];
    int       data;
    int       ret;
    pthread_t thread;
    char      buf[256];
    int       i;

    printf("#### init_timer\n");
    init_timer(&thread);
    dump_timer(NULL);

    printf("#### start timer (NUM_OF_TIMER + 1)\n");
    for (i = 0; i < NUM_OF_TIMER + 1; i++) {
        data = i + 100;
        test_print_msec(buf);
        ret = start_timer(&(timer_id[i]),
                          100, test_callback, (void * )&data, sizeof(int));
        printf("start = %s, ret = %d\n", buf, ret);
    }
    dump_timer(NULL);

    printf("#### wait to expire\n");
    sleep(2);

    printf("##### start timer (NUM_OF_TIMER)\n");
    for (i = 0; i < NELEMS(g_timer_table.timer_info); i++) {
        data = i + 200;
        test_print_msec(buf);
        ret = start_timer(&(timer_id[i]),
                          100, test_callback, (void * )&data, sizeof(int));
        printf("start = %s, ret = %d\n", buf, ret);
    }
    dump_timer(NULL);

    printf("#### stop timer\n");
    ret = stop_timer(&(timer_id[1]));
    printf("stop_timer ret = %d\n", ret);
    dump_timer(NULL);

    printf("#### wait to expire\n");
    sleep(2);
    dump_timer(NULL);

    data = 123;
    ret = start_timer(&(timer_id[0]),
                      10, test_callback, (void * )&data, sizeof(int));
    printf("start = %s, ret = %d\n", buf, ret);

    return 0;
}
