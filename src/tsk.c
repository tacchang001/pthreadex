#define _GNU_SOURCE

#include <sched.h>

#include "ele_task.h"

#define ELE_TSK_GLOBAL

#include "tsk.h"

#undef  ELE_TSK_GLOBAL

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "ele_error.h"
#include "mutex_lock.h"

typedef struct {
    ele_task_entry_t start_routine_entry;
    void *start_routine_arg;
    ele_task_wait_for_start_t wait;
} ele_task_start_routine_argument_t;

const int INVALID_TSK_NO = -1;

pthread_mutex_t errchkmutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static ele_task_attr_t tab[ELE_TSK_CREATE_LIMIT];
static const size_t TAB_SIZ = (sizeof(tab) / sizeof(tab[0]));

static pthread_mutex_t mutex_wait_for_start;
static pthread_cond_t cond_wait_for_start;

__attribute__((constructor))
static void ele_task_tab_init(void) {
    unsigned int i;
    for (i = 0; i < TAB_SIZ; i++) {
        tab[i].init_attr.id = INVALID_TSK_NO;
        tab[i].thread_id = 0;
    }

    pthread_mutex_init(&mutex_wait_for_start, NULL);
    pthread_cond_init(&cond_wait_for_start, NULL);
}

static void print_thread_affinity(int tskno, int cpu, const pthread_t id) {
#ifndef NDEBUG
    int cores = sysconf(_SC_NPROCESSORS_ONLN);
    cpu_set_t cpuset;
    int result = pthread_getaffinity_np(id, sizeof(cpuset), &cpuset);
    if (result != 0) {
        ELE_PERROR("print_thread_affinity");
        return;
    }
    int i;
    for (i = 0; i < cores; i++) {
        if (CPU_ISSET(i, &cpuset)) {
            printf("thread tsk:%d cpu:%d core:%d/%d\n", tskno, cpu, i, cores);
        }
    }
#endif
}

/*
 *
 */
pthread_t ele_task_get_thread_id(
        const int id) {
    unsigned int i = 0;
    for (i = 0; i < TAB_SIZ; i++) {
        if (tab[i].init_attr.id == id) {
            return tab[i].thread_id;
        }
    }

    return ELE_FAILURE;
}

/*
 *
 */
static ele_task_attr_t *ele_task_get_tab_space(void) {
    unsigned int i;
    for (i = 0; i < TAB_SIZ; i++) {
        if (tab[i].init_attr.id == INVALID_TSK_NO) {
            return &tab[i];
        }
    }

    return NULL;
}

/*
 *
 */
void ele_task_start_routine(
        ele_task_start_routine_argument_t *thread) {

    if (thread->wait == ELE_TASK_WAIT) {
        pthread_mutex_lock(&mutex_wait_for_start);
        pthread_cond_wait(&cond_wait_for_start, &mutex_wait_for_start);
        pthread_mutex_unlock(&mutex_wait_for_start);
    }

    thread->start_routine_entry(thread->start_routine_arg);
}

/*
 *
 */
int ele_task_create(
        const ele_task_init_attr_t attr,
        const ele_task_wait_for_start_t wait) {
    assert(attr.id > 0);

    do {
        SCOPED_LOCK(errchkmutex);

        ele_task_attr_t *const rec = ele_task_get_tab_space();
        if (rec == NULL) {
            return ELE_FAILURE;
        }

        pthread_attr_t a;
        memset(&a, 0, sizeof(a));
        if (pthread_attr_init(&a) != 0) {
            ELE_PERROR("ele_task_create");
            return ELE_FAILURE;
        }
        switch (attr.schedpolicy) {
            case SCHED_FIFO:
            case SCHED_RR:
                if (pthread_attr_setinheritsched(&a, PTHREAD_EXPLICIT_SCHED) != 0) {
                    ELE_PERROR("ele_task_create");
                    return ELE_FAILURE;
                }
                if (pthread_attr_setschedpolicy(&a, attr.schedpolicy) != 0) {
                    ELE_PERROR("ele_task_create");
                    return ELE_FAILURE;
                }
                const struct sched_param sp = {
                        attr.schedparam
                };
                if (pthread_attr_setschedparam(&a, &sp) != 0) {
                    ELE_ERROR_OUTOFRANGE(
                            sched_get_priority_min(attr.schedpolicy),
                            sp.__sched_priority,
                            sched_get_priority_max(attr.schedpolicy));
                    return ELE_OUTOFRANGE;
                }
                break;
            default:
                break;
        }
        pthread_t id = 0;
        ele_task_start_routine_argument_t t;
        t.start_routine_entry = attr.start_routine_entry;
        t.start_routine_arg = attr.start_routine_arg;
        t.wait = wait;
        if (pthread_create(&id, &a, ele_task_start_routine, &t) != 0) {
            ELE_PERROR("ele_task_create");
            return ELE_FAILURE;
        }

        if (attr.cpu >= 0) {
            const int cores = sysconf(_SC_NPROCESSORS_ONLN);
            if (attr.cpu > cores) {
                fprintf(stderr, "cpu=%d > %d\n", cores, attr.cpu);
                return ELE_FAILURE;
            }
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(attr.cpu, &cpuset);
            int result = pthread_setaffinity_np(id, sizeof(cpu_set_t), &cpuset);
            if (result != 0) {
                perror("ele_task_create");
                fprintf(stderr, "cpu = %d\n", attr.cpu);
                return ELE_FAILURE;
            }
            print_thread_affinity(attr.id, attr.cpu, id);
        }

        rec->init_attr = attr;
        rec->thread_id = id;
    } while (0);

    return ELE_SUCCESS;
}

/*
 *
 */
int ele_task_destroy(int id) {
    assert(id > 0);

    do {
        SCOPED_LOCK(errchkmutex);

        const pthread_t tid = ele_task_get_thread_id(id);

        if (pthread_cancel(tid) != 0) {
            ELE_PERROR("ele_task_destroy");
        }

        void *result = NULL;
        if (pthread_join(tid, &result) != 0) {
            ELE_PERROR("ele_task_destroy");
        }
    } while (0);

    return ELE_SUCCESS;
}

/*
 *
 */
int ele_task_join(int id) {
    assert(id > 0);

    const pthread_t tid = ele_task_get_thread_id(id);

    void *result = NULL;
    if (pthread_join(tid, &result) != 0) {
        ELE_PERROR("ele_task_join");
    }

    return 0;
}

/*
 *
 */
void ele_task_start_all(void) {
    usleep(50);
    pthread_mutex_lock(&mutex_wait_for_start);
    pthread_cond_broadcast(&cond_wait_for_start);
    pthread_mutex_unlock(&mutex_wait_for_start);
}

/*
 *
 */
void ele_task_display_pthread_attr(const int id) {
    pthread_attr_t attr;
    static const char *prefix = "\t";

    pthread_t tid = ele_task_get_thread_id(id);
    if (tid != ELE_FAILURE) {
        int s = pthread_getattr_np(tid, &attr);
        if (s == 0) {
            printf("%sTask id             = %d\n", prefix, id);
            printf("%sThread id           = %lx\n", prefix, tid);
            display_pthread_attr(&attr, prefix);
        } else {
            ELE_PERROR("ele_task_display_pthread_attr");
        }
    } else {
        ELE_PERROR("ele_task_display_pthread_attr");
    }
}




/*
 *
 */
void display_pthread_attr(
        const pthread_attr_t *const attr,
        const char *const prefix) {
    int s, i;
    size_t v;
    void *stkaddr;
    struct sched_param sp;

    s = pthread_attr_getdetachstate(attr, &i);
    if (s == 0) {
        printf("%sDetach state        = %s\n", prefix,
               (i == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
               (i == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
               "???");
    }

    s = pthread_attr_getscope(attr, &i);
    if (s == 0) {
        printf("%sScope               = %s\n", prefix,
               (i == PTHREAD_SCOPE_SYSTEM) ? "PTHREAD_SCOPE_SYSTEM" :
               (i == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
               "???");
    }

    s = pthread_attr_getinheritsched(attr, &i);
    if (s == 0) {
        printf("%sInherit scheduler   = %s\n", prefix,
               (i == PTHREAD_INHERIT_SCHED) ? "PTHREAD_INHERIT_SCHED" :
               (i == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
               "???");
    }

    s = pthread_attr_getschedpolicy(attr, &i);
    if (s == 0) {
        printf("%sScheduling policy   = %s\n", prefix,
               (i == SCHED_OTHER) ? "SCHED_OTHER" :
               (i == SCHED_FIFO) ? "SCHED_FIFO" :
               (i == SCHED_RR) ? "SCHED_RR" :
               "???");
    }

    s = pthread_attr_getschedparam(attr, &sp);
    if (s == 0) {
        printf("%sScheduling priority = %d\n", prefix, sp.sched_priority);
    }

    s = pthread_attr_getguardsize(attr, &v);
    if (s == 0) {
        printf("%sGuard size          = %ld bytes\n", prefix, v);
    }

    s = pthread_attr_getstack(attr, &stkaddr, &v);
    if (s == 0) {
        printf("%sStack address       = %p\n", prefix, stkaddr);
        printf("%sStack size          = 0x%lx bytes\n", prefix, v);
    }
}
