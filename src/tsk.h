#pragma once

#include "ele_task.h"

#ifndef ELE_TSK_GLOBAL
#define EXTERN extern
#else
#define EXTERN /* nop */
#endif

#include <pthread.h>

#ifndef ELE_TSK_CREATE_LIMIT
#define ELE_TSK_CREATE_LIMIT 32
#endif

typedef struct {
	ele_task_init_attr_t init_attr;
	pthread_t thread_id;
} ele_task_attr_t;

typedef struct {
	ele_task_entry_t entry;
	void* arg;
} ele_task_func_call_t;

EXTERN void * ele_task_container(void* arg);
