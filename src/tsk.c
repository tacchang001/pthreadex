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

const int INVALID_TSK_NO = -1;

pthread_mutex_t errchkmutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static ele_task_attr_t tab[ELE_TSK_CREATE_LIMIT];
static const size_t TAB_SIZ = (sizeof(tab)/sizeof(tab[0]));

__attribute__((constructor))
static void ele_task_tab_init(void)
{
	unsigned int i;
	for (i=0; i<TAB_SIZ; i++) {
		tab[i].init_attr.id = INVALID_TSK_NO;
		tab[i].thread_id = 0;
	}
}

static void print_thread_affinity(int tskno, int cpu, const pthread_t id) {
#ifndef NDEBUG
	int cores = sysconf(_SC_NPROCESSORS_ONLN);
	cpu_set_t cpuset;
	int result = pthread_getaffinity_np(id, sizeof(cpuset), &cpuset);
	if (result == -1) {
		perror("pthread_getaffinity_np");
		return;
	}
	int i;
	for (i=0; i<cores; i++) {
		if (CPU_ISSET(i, &cpuset)) {
			printf("thread(%d) on %d -> %d/%d\n", tskno, cpu, i, cores);
		}
	}
#endif
}

/*
 *
 */
pthread_t ele_task_get_thread_id(
	const int id)
{
	unsigned int i=0;
	for (i=0; i<TAB_SIZ; i++) {
		if (tab[i].init_attr.id == id) {
			return tab[i].thread_id;
		}
	}

	return 0;
}

/*
 *
 */
static ele_task_attr_t * ele_task_get_tab_space(void)
{
	unsigned int i;
	for (i=0; i<TAB_SIZ; i++) {
		if (tab[i].init_attr.id == INVALID_TSK_NO) {
			return &tab[i];
		}
	}

	return NULL;
}

/*
 *
 */
int ele_task_create(
	const ele_task_init_attr_t attr)
{
	assert(attr.id > 0);

	do {
		SCOPED_LOCK(errchkmutex);

		ele_task_attr_t * const rec = ele_task_get_tab_space();
		if (rec == NULL) {
			return ELE_FAILURE;
		}

		pthread_attr_t a;
		memset(&a, 0, sizeof(a));
		if (pthread_attr_init(&a) != 0) {
			ELE_PERROR("pthread_attr_init");
			return ELE_FAILURE;
		}
		switch (attr.schedpolicy) {
		case SCHED_FIFO:
		case SCHED_RR:
			if (pthread_attr_setinheritsched(&a, PTHREAD_EXPLICIT_SCHED) != 0) {
				ELE_PERROR("pthread_attr_setinheritsched");
				return ELE_FAILURE;
			}
			if (pthread_attr_setschedpolicy(&a, attr.schedpolicy) != 0) {
				ELE_PERROR("pthread_attr_setschedpolicy");
				return ELE_FAILURE;
			}
			const struct sched_param sp = {
				attr.schedparam
			};
			if (pthread_attr_setschedparam(&a, &sp) != 0) {
				ELE_PERROR("pthread_attr_setschedparam");
				ELE_ERROR_OUTOFRANGE(
					sched_get_priority_min(attr.schedpolicy),
					sp.__sched_priority,
					sched_get_priority_max(attr.schedpolicy));
				return ELE_FAILURE;
			}
			break;
		default:
			break;
		}
		pthread_t id = 0;
		if (pthread_create(&id, &a, attr.entry, attr.arg) != 0) {
			ELE_PERROR("pthread_create");
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
			if (result == -1) {
				perror("pthread_setaffinity_np");
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
int ele_task_destroy(int id)
{
	assert(id > 0);

	do {
		SCOPED_LOCK(errchkmutex);

		const pthread_t tid = ele_task_get_thread_id(id);

		if (pthread_cancel(tid) != 0) {
			ELE_PERROR("pthread_cancel");
		}

		void *result = NULL;
		if (pthread_join(tid, &result) != 0) {
			ELE_PERROR("pthread_join");
		}
	} while (0);

	return ELE_SUCCESS;
}

/*
 *
 */
int ele_task_join(int id)
{
	assert(id > 0);

	const pthread_t tid = ele_task_get_thread_id(id);

	void *result = NULL;
	if (pthread_join(tid, &result) != 0) {
		ELE_PERROR("pthread_join");
	}

	return 0;
}

