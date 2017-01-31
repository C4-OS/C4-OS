#ifndef _C4RT_H
#define _C4RT_H 1
#include <c4/syscall.h>
#include <c4/message.h>
#include <stdbool.h>
#include <stdint.h>

#define NULL ((void *)0)

#define DO_SYSCALL(N, A, B, C, D, RET) \
	asm volatile ( " \
		int $0x60;   \
	" : "=a"(RET)    \
	  : "a"(N), "D"(A), "S"(B), "d"(C), "b"(D) \
	  : "memory" );

void c4_exit( void );

int c4_msg_send( message_t *buffer, unsigned target );
int c4_msg_recieve( message_t *buffer, unsigned whom );
int c4_msg_send_async( message_t *buffer, unsigned target );
int c4_msg_recieve_async( message_t *buffer, unsigned flags );
int c4_create_thread( void *entry, void *stack, unsigned flags );
int c4_continue_thread( unsigned thread );
int c4_set_pager( unsigned thread, unsigned pager );
int c4_get_pager( void );
int c4_get_id( void );

int c4_mem_map_to( unsigned thread_id, void *from, void *to,
                   unsigned size, unsigned permissions );
int c4_mem_grant_to( unsigned thread_id, void *from, void *to,
                     unsigned size, unsigned permissions );

void *c4_request_physical( uintptr_t virt,
                           uintptr_t physical,
                           unsigned size,
                           unsigned permissions );

uint8_t  c4_in_byte( unsigned port );
uint16_t c4_in_word( unsigned port );
uint32_t c4_in_dword( unsigned port );
void     c4_out_byte( unsigned port, uint8_t value );
void     c4_out_word( unsigned port, uint16_t value );
void     c4_out_dword( unsigned port, uint32_t value );

#endif
