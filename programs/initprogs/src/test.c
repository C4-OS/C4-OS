#include <c4/syscall.h>
#include <c4/message.h>
#include <stdint.h>
#include <stdbool.h>

#define NULL ((void *)0)

#define DO_SYSCALL(N, A, B, C, D, RET) \
	asm volatile ( " \
		mov %1, %%eax; \
		mov %2, %%edi; \
		mov %3, %%esi; \
		mov %4, %%edx; \
		mov %5, %%ebx; \
		int $0x60;     \
		mov %%eax, %0  \
	" : "=r"(RET) \
	  : "g"(N), "g"(A), "g"(B), "g"(C), "g"(D) \
	  : "eax", "edi", "esi", "edx", "ebx" );

// TODO: remove this program
int main(int argc, char *argv[]){
	int ret;
	uintptr_t display = (uintptr_t)0;

	volatile message_t msg = {
		.type = MESSAGE_TYPE_DEBUG_PRINT,
		.data = { 'A', },
	};

	// XXX: kernel hangs if this recieve call isn't made before the send call,
	//      will need to figure out what is causing that
	DO_SYSCALL( SYSCALL_SYNC_RECIEVE, &msg, 0, 0, 0, ret );

	msg.data[0] = 'A';
	msg.type = 0xabcd;

	while ( true ){
		for ( volatile unsigned k = 0; k < 10000000; k++ );
		DO_SYSCALL( SYSCALL_SYNC_SEND, &msg, display, 0, 0, ret );
	}
}
