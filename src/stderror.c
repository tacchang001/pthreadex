#include <ele_error.h>

const char * ele_strerror(const int ele_errno)
{
	switch (ele_errno) {
	case ELE_SUCCESS:
		return "success";
	case ELE_FAILURE:
		return "failure";
	case ELE_OUTOFRANGE:
		return "output range error";
	default:
		return "unknown error code";
	}
}
