#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 */
typedef enum {
    ELE_TASK_NO_WAIT,
    ELE_TASK_WAIT,
    ELE_TASK_INVALID
} ele_task_wait_for_start_t;

/**
 *
 */
typedef void *(*ele_task_entry_t)(void *);

/**
 *
 */
typedef struct {
    int id;
    int schedpolicy;
    int schedparam;
    int cpu; // -1: disable
    ele_task_entry_t start_routine_entry;
    void *start_routine_arg;
} ele_task_init_attr_t;

/**
 *
 */
int ele_task_create(
        const ele_task_init_attr_t attr,
        const ele_task_wait_for_start_t wait);

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


/**
 *
 */
void ele_task_display_pthread_attr(
        const int id
);

/**
 *
 */
void ele_task_wait_to_start(void);

/**
 *
 */
void ele_task_start_all(void);



/**
 *
 */
void display_pthread_attr(
        const pthread_attr_t *const attr,
        const char *const prefix);

#ifdef __cplusplus
}
#endif
