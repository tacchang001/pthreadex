#pragma once

#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * gsl_error.hを参考にする。
 */

/**
 *
 */
typedef enum {
	ELE_SUCCESS = 0,
	ELE_FAILURE = -1,
	ELE_OUTOFRANGE 			= 0x08000001,  // output range
	ELE_NOTENOUGH           = 0x08000002,  // not enough resources
} ele_result_t;

typedef void ele_error_handler_t(
    const char * reason,
    const char * file,
    int line,
    int ele_errno);

/**
 *
 */
void
ele_error(const char * reason, const char * file, int line, int ele_errno);

/**
 *
 */
const char *
ele_strerror(const int ele_errno);

/**
 *
 */
ele_error_handler_t *
ele_set_error_handler(ele_error_handler_t * new_handler);

/**
 *
 */
ele_error_handler_t *
ele_set_error_handler_off(void);

/**
 *
 */
#define ELE_PERROR(reason) \
   do { \
	   const int error_no = errno; \
	   ele_error (reason, __FILE__, __LINE__, error_no) ; \
   } while (0)

/**
 *
 */
#define ELE_ERROR_OUTOFRANGE(min, value, max) \
	do { \
		char msg[256]; \
		snprintf(msg, sizeof(msg), "%d < %d < %d", min, value, max); \
		ele_error (msg, __FILE__, __LINE__, ELE_OUTOFRANGE) ; \
	} while (0)

#ifdef __cplusplus
}
#endif
