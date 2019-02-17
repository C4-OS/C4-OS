#ifndef _C4OS_LIBC_SETJMP_H
#define _C4OS_LIBC_SETJMP_H 1

typedef struct {
	;
} jmp_buf;

typedef jmp_buf sigjmp_buf;

int setjmp(jmp_buf env);
int sigsetjmp(sigjmp_buf env, int savesigs);

int longjmp(jmp_buf env, int val);
void siglongjmp(sigjmp_buf env, int val);

#endif
