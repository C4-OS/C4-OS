#ifndef _C4RT_H
#define _C4RT_H 1
#include <c4/syscall.h>
#include <c4/message.h>
#include <c4rt/compiler.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// capability addresses of default objects added to processes in the C4 runtime
enum {
	C4_CURRENT_CSPACE = 0,
	C4_SERV_PORT = 1,
	C4_CURRENT_ADDRSPACE = 2,
	C4_BOOT_INFO = 3,
	C4_PAGER = 4,
	C4_NAMESERVER = 5,
	C4_DEFAULT_OBJECT_END = 6,
};

// magic value which is passed by the elf loader to signal that it's using
// the c4rt parameter convention
#define C4RT_INIT_MAGIC 0x10adab1e

#define DO_SYSCALL(N, A, B, C, D, RET) \
	asm volatile ( " \
		int $0x60;   \
	" : "=a"(RET)    \
	  : "a"(N), "D"(A), "S"(B), "d"(C), "b"(D) \
	  : "memory" );

void c4_exit( void );

// ipc functions
int c4_msg_create_sync( void );
int c4_msg_create_async( void );
int c4_msg_send( message_t *buffer, unsigned target );
int c4_msg_recieve( message_t *buffer, unsigned whom );
int c4_msg_send_async( message_t *buffer, unsigned target );
int c4_msg_recieve_async( message_t *buffer, unsigned from, unsigned flags );
int c4_send_temp_endpoint( uint32_t server );

// thread functions
int c4_create_thread( void *entry, void *stack, unsigned flags );
int c4_continue_thread( unsigned thread );
int c4_set_pager( unsigned thread, unsigned pager );
DEPRECATED int c4_get_pager( void );
int c4_get_id( void );
int c4_set_addrspace( unsigned thread, unsigned space );
int c4_set_capspace( unsigned thread, unsigned space );

// memory functions
int c4_addrspace_create( void );
int c4_addrspace_map( uint32_t addrspace,
                      uint32_t frame,
                      uintptr_t address,
                      unsigned permissions );
int c4_addrspace_unmap( uint32_t addrspace, uintptr_t address );

int c4_phys_frame_create( uintptr_t physical, size_t size, uint32_t context );
int c4_phys_frame_split( uint32_t frame, size_t offset );

// capability functions
int c4_cspace_create( void );
int c4_cspace_move( uint32_t src_space, uint32_t src_object,
                    uint32_t dest_space, uint32_t dest_object );
int c4_cspace_copy( uint32_t src_space, uint32_t src_object,
                    uint32_t dest_space, uint32_t dest_object );
int c4_cspace_remove( uint32_t cspace, uint32_t object );
int c4_cspace_restrict( uint32_t cspace, uint32_t object, unsigned perms );
int c4_cspace_grant( uint32_t object, uint32_t queue, unsigned perms );

DEPRECATED int c4_mem_unmap( unsigned thread_id, void *addr );
DEPRECATED int c4_mem_map_to( unsigned thread_id, void *from, void *to,
                   unsigned size, unsigned permissions );
DEPRECATED int c4_mem_grant_to( unsigned thread_id, void *from, void *to,
                     unsigned size, unsigned permissions );

// TODO: note in docs that size is in pages
void *c4_request_physical( uintptr_t virt,
                           uintptr_t physical,
                           unsigned size,
                           unsigned permissions );

DEPRECATED void  c4_dump_maps( unsigned thread );

int c4_interrupt_subscribe( unsigned num, uint32_t endpoint );
int c4_interrupt_unsubscribe( uint32_t endpoint );

uint8_t  c4_in_byte( unsigned port );
uint16_t c4_in_word( unsigned port );
uint32_t c4_in_dword( unsigned port );
void     c4_out_byte( unsigned port, uint8_t value );
void     c4_out_word( unsigned port, uint16_t value );
void     c4_out_dword( unsigned port, uint32_t value );

// debugging tools
#ifndef NDEBUG
#define C4_ASSERT(CONDITION) { \
		if ( !(CONDITION) ){ \
			c4_debug_printf( "%s:%u: assertion \"" #CONDITION "\" failed\n", \
				__FILE__, __LINE__ ); \
		} \
	}

#else
#define C4_ASSERT(CONDITION) /* CONDITION */
#endif

void c4_debug_putchar( char c );
void c4_debug_puts( const char *str );
void c4_debug_printf( const char *format, ... );

#endif
