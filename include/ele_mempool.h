#pragma once

#include "ele_error.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

ele_result_t
ele_mempool_create(size_t max_size);

void
ele_mempool_destroy(void);

void *
ele_mempool_alloc(size_t size);

void *
ele_mempool_calloc(size_t size);

void
ele_mempool_free(void * pool);

#ifndef NDEBUG
void mempool_print_pool(void);
size_t mempool_get_max_size();
#endif

#ifdef __cplusplus
}
#endif
