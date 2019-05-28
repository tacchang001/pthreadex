#include "ele_itc.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <sys/queue.h>

#include "ele_mempool.h"

#define QUEUE_ATTRIBUTE(ptr)((itc_queue_attribute_t*)(ptr->attribute))

typedef struct entry entry_t;
struct entry {
	unsigned int id;
	TAILQ_ENTRY(entry) entries;
	void * item;
};

typedef struct {
	eventfd_t efd;
	char name[64];
	TAILQ_HEAD(tailhead, entry) head;
} itc_queue_attribute_t;

ele_queue_desc_t *
ele_queue_create(void) {
	eventfd_t efd = eventfd(0, 0);
	if (efd == -1) {
		perror("eventfd");
		return NULL;
	}

	itc_queue_attribute_t * qattr = calloc(1, sizeof(itc_queue_attribute_t));
	if (qattr == NULL) {
		perror("calloc");
		close(efd);
		return NULL;
	}
	ele_queue_desc_t * qdes = malloc(sizeof(ele_queue_desc_t));
	if (qdes == NULL) {
		perror("malloc");
		close(efd);
		free(qattr);
		return NULL;
	}

	TAILQ_INIT(&qattr->head);
	qattr->efd = efd;
	qdes->attribute = qattr;

	return qdes;
}

void ele_queue_destroy(ele_queue_desc_t * qdes) {
	assert(qdes != NULL);
	assert(qdes->attribute != NULL);
	assert(QUEUE_ATTRIBUTE(qdes)->efd != -1);

	close(QUEUE_ATTRIBUTE(qdes)->efd);
	free(qdes->attribute);
	free(qdes);
	return;
}

int ele_queue_push(ele_queue_desc_t * qdes,
		const ele_queue_item_t * const item) {
	assert(qdes != NULL);
	assert(qdes->attribute != NULL);

	assert(QUEUE_ATTRIBUTE(qdes)->efd != -1);
	assert(item != NULL);
	assert(item->data_length > 0);

	static unsigned int no = 0;

	size_t data_length = item->data_length + sizeof(ele_queue_item_t);
	ele_queue_item_t * p = ele_mempool_alloc(data_length);
	assert(p != NULL);
	memcpy(p, item, item->data_length);

	entry_t * t = ele_mempool_alloc(sizeof(entry_t));
	assert(t != NULL);
	t->id = no;
	t->item = p;
	TAILQ_INSERT_TAIL(&QUEUE_ATTRIBUTE(qdes)->head, t, entries);

	u_int64_t value = 1;
	int write_result = eventfd_write(QUEUE_ATTRIBUTE(qdes)->efd, value);
	if (write_result == -1) {
		perror("eventfd_write");
		return ELE_FAILURE;
	}

	return ELE_SUCCESS;
}

int ele_queue_pop(ele_queue_desc_t * qdes,
		ele_queue_item_t * const item) {
	assert(qdes != NULL);
	assert(qdes->attribute != NULL);
	assert(QUEUE_ATTRIBUTE(qdes)->efd != -1);
	assert(item != NULL);
	assert(item->data_length > 0);

	entry_t * t = QUEUE_ATTRIBUTE(qdes)->head.tqh_first;
	assert(t != NULL);
	ele_queue_item_t * p = t->item;
	memcpy(item, p, p->data_length);
	TAILQ_REMOVE(&QUEUE_ATTRIBUTE(qdes)->head,
			QUEUE_ATTRIBUTE(qdes)->head.tqh_first, entries);

	u_int64_t value;
	int read_result = eventfd_read(QUEUE_ATTRIBUTE(qdes)->efd, &value);
	if (read_result == -1) {
		perror("eventfd_read");
		return ELE_FAILURE;
	}

	return ELE_SUCCESS;
}

eventfd_t ele_queue_get_desc(ele_queue_desc_t * qdes) {
	assert(qdes != NULL);
	assert(qdes->attribute != NULL);

	return QUEUE_ATTRIBUTE(qdes)->efd;
}
