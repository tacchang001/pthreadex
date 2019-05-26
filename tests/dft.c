#include "misc.h"
#include <math.h>
#define ENABLE_TASK_CANCELATION
#ifdef ENABLE_TASK_CANCELATION
#include <pthread.h>
#endif

int dft(const int max) {
    int k, n, i;
    double f[max + 1];
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    double result_re[max + 1], result_im[max + 1];
#pragma GCC diagnostic warning "-Wunused-but-set-variable"

    for (i = 0; i < max; i++) {
        f[i] = sin(i);
    }
    for (n = 0; n < i; n++) {
        double re = 0.0;
        double im = 0.0;
        for (k = 0; k < i; k++) {
            re += f[k] * cos(2 * M_PI * k * n / i);
            im += -f[k] * sin(2 * M_PI * k * n / i);
        }
#ifdef ENABLE_TASK_CANCELATION
        pthread_testcancel(); /* A cancellation point */
#endif
        result_re[n] = re;
        result_im[n] = im;
    }

    return 0;
}
