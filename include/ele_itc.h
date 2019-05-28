#pragma once
// Inter-Thread communication

#include "ele_error.h"

#include <stddef.h>
#include <sys/eventfd.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ele_queue_desc ele_queue_desc_t;
struct ele_queue_desc {
	void *attribute;
};

typedef struct ele_queue_item ele_queue_item_t;
struct ele_queue_item {
	size_t data_length;
	char data[0];
};

ele_queue_desc_t *
ele_queue_create(void);

void
ele_queue_destroy(ele_queue_desc_t * qdes);

int
ele_queue_push(ele_queue_desc_t * qdes, const ele_queue_item_t * item);

int
ele_queue_pop(ele_queue_desc_t * qdes, ele_queue_item_t * item);

eventfd_t
ele_queue_get_desc(ele_queue_desc_t * qdes);

#ifdef __cplusplus
}
#endif
