#ifndef _C4RT_COMPILER_H
#define _C4RT_COMPILER_H 1

#if __GNUC__
#define WEAK           __attribute__((weak))
#define NO_RETURN      __attribute__((noreturn))
#define likely(EXPR)   (__builtin_expect(!!(EXPR), 1))
#define unlikely(EXPR) (__builtin_expect(!!(EXPR), 0))
#define ALIGN_TO(N)    __attribute__((aligned(N)))

#else
#define WEAK           /* weak */
#define NO_RETURN      /* no return */
#define likely(EXPR)   (EXPR)
#define unlikely(EXPR) (EXPR)
#define ALIGN_TO(N)    /* aligned to N */

#endif
#endif
