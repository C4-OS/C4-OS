#ifndef _C4OS_LIBC_MATH_H
#define _C4OS_LIBC_MATH_H 1
#include <stdint.h>

static inline double c4_make_inf(void) {
	union { double d; uint64_t u; } un;
	un.u = 0x7ff0000000000000;
	return un.d;
}

#define INFINITY (c4_make_inf())
#define HUGE_VAL INFINITY

double pow(double x, double y);
double floor(double x);
double fmod(double x, double y);
double frexp(double x, int *exp);

#endif
