#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mempool {
	struct mempool * prev;
	struct mempool * next;
	size_t size;
	char data[1];
} mempool_t;

#ifdef __cplusplus
}
#endif
