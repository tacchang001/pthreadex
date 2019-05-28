#pragma once

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void lock_unlock(pthread_mutex_t **p) {
    pthread_mutex_unlock(*p);
}

static inline void rwlock_unlock(pthread_rwlock_t **p) {
    pthread_rwlock_unlock(*p);
}

#define SCOPED_RDLOCK(m) \
	pthread_rwlock_rdlock(&m); \
	__attribute__((__cleanup__(rwlock_unlock))) pthread_rwlock_t* scoped_lock_ = &m

#define SCOPED_WRLOCK(m) \
	pthread_rwlock_wrlock(&m); \
	__attribute__((__cleanup__(rwlock_unlock))) pthread_rwlock_t* scoped_lock_ = &m

#define SCOPED_LOCK(m) \
  pthread_mutex_lock(&m); \
  __attribute__((__cleanup__(lock_unlock))) pthread_mutex_t* scoped_lock_ = &m

#ifdef __cplusplus
}
#endif
