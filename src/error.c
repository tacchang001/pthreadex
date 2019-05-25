#include <ele_error.h>

#include <stdio.h>
#include <pthread.h>

static void no_error_handler(
    const char *reason,
    const char *file,
    int line,
    int ele_errno)
{
	printf("%s(%d): [%d]%s\n", file, line, ele_errno, reason);
	return;
}

static __thread ele_error_handler_t *
ele_error_handler = no_error_handler;

void ele_error(const char * reason, const char * file, int line, int ele_errno)
{
	if (ele_error_handler) {
		(*ele_error_handler)(reason, file, line, ele_errno);
		return;
	}
}

ele_error_handler_t *
ele_set_error_handler(ele_error_handler_t * new_handler)
{
	ele_error_handler_t * previous_handler = ele_error_handler;
	ele_error_handler = new_handler;
	return previous_handler;
}

ele_error_handler_t *
ele_set_error_handler_off(void)
{
	ele_error_handler_t * previous_handler = ele_error_handler;
	ele_error_handler = no_error_handler;
	return previous_handler;
}
