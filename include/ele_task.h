#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
typedef void * (*ele_task_entry_t)(void*);

/**
 *
 */
typedef struct {
	int id;
	int schedpolicy;
	int schedparam;
	int cpu; // -1: disable
	ele_task_entry_t entry;
	void* arg;
} ele_task_init_attr_t;

/**
 *
 */
int ele_task_create(
	const ele_task_init_attr_t attr);

/**
 *
 */
int ele_task_destroy(int id);

/**
 *
 */
int ele_task_join(int id);

/**
 *
 */
pthread_t ele_task_get_thread_id(
	const int id
);

#ifdef __cplusplus
}
#endif
