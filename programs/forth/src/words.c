#include <c4rt/c4rt.h>
#include <c4rt/stublibc.h>
#include <c4/paging.h>
#include <miniforth/stubs.h>
#include <miniforth/miniforth.h>

#include <c4rt/interface/block.h>
#include <c4rt/interface/console.h>
#include <nameserver/nameserver.h>
#include <c4alloc/c4alloc.h>

static inline void *popptr( minift_vm_t *vm ){
	return (void *)minift_pop( vm, &vm->param_stack );
}

static inline unsigned long popnum( minift_vm_t *vm ){
	return minift_pop( vm, &vm->param_stack );
}

static inline void pushptr( minift_vm_t *vm, void *ptr ){
	minift_push( vm, &vm->param_stack, (uintptr_t)ptr );
}

static inline void pushnum( minift_vm_t *vm, unsigned long n ){
	minift_push( vm, &vm->param_stack, n );
}

static bool c4_minift_sendmsg( minift_vm_t *vm ){
	unsigned long target = minift_pop( vm, &vm->param_stack );
	unsigned long temp   = minift_pop( vm, &vm->param_stack );
	message_t *msg = (void *)temp;

	if ( !vm->running ){
		return false;
	}

	c4_msg_send( msg, target );

	return true;
}

static bool c4_minift_recvmsg( minift_vm_t *vm ){
	//  TODO: add 'from' argument, once that's supported
	unsigned long temp   = minift_pop( vm, &vm->param_stack );
	message_t *msg = (void *)temp;

	if ( !vm->running ){
		return false;
	}

	c4_msg_recieve( msg, 0 );

	return true;
}

/* TODO: consider reimplementing this later, just needs to use the
 *       post-capability block interface, but I cba to update this now
 *       since the interpreter here has just about reached the end of it's
 *       useful life
static uint8_t buffer[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));

static bool c4_minift_block_read( minift_vm_t *vm ){
	unsigned size     = minift_pop( vm, &vm->param_stack );
	unsigned location = minift_pop( vm, &vm->param_stack );
	unsigned drive    = minift_pop( vm, &vm->param_stack );
	unsigned id       = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	block_read( id, buffer, drive, location, size );
	minift_push( vm, &vm->param_stack, (uintptr_t)buffer );

	return true;
}

static bool c4_minift_block_write( minift_vm_t *vm ){
	unsigned size     = minift_pop( vm, &vm->param_stack );
	unsigned location = minift_pop( vm, &vm->param_stack );
	unsigned drive    = minift_pop( vm, &vm->param_stack );
	unsigned id       = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	block_write( id, buffer, drive, location, size );
	minift_push( vm, &vm->param_stack, (uintptr_t)buffer );

	return true;
}
*/

static bool c4_minift_console_put_char( minift_vm_t *vm ){
	unsigned c  = minift_pop( vm, &vm->param_stack );
	unsigned id = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	console_put_char( id, c );

	return true;
}

static bool c4_minift_console_set_pos( minift_vm_t *vm ){
	unsigned y  = minift_pop( vm, &vm->param_stack );
	unsigned x  = minift_pop( vm, &vm->param_stack );
	unsigned id = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	console_set_position( id, x, y );

	return true;
}

static bool c4_minift_console_clear( minift_vm_t *vm ){
	unsigned id = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	console_clear( id );

	return true;
}

static bool c4_minift_console_info( minift_vm_t *vm ){
	unsigned id = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	console_info_t info;

	console_get_info( id, &info );
	minift_push( vm, &vm->param_stack, info.x_pos );
	minift_push( vm, &vm->param_stack, info.y_pos );
	minift_push( vm, &vm->param_stack, info.width );
	minift_push( vm, &vm->param_stack, info.height );

	return true;
}

static bool c4_minift_name_lookup( minift_vm_t *vm ){
	char *name  = (char *)minift_pop( vm, &vm->param_stack );
	unsigned id = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	unsigned ret = nameserver_lookup( id, name );
	minift_push( vm, &vm->param_stack, ret );

	return true;
}

static bool c4_minift_key( minift_vm_t *vm ){
	pushnum( vm, minift_get_char( ));

	return true;
}

static c4a_heap_t minift_heap;

static bool c4_minift_allocate( minift_vm_t *vm ){
	unsigned size = minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	void *foo = c4a_alloc( &minift_heap, size );
	minift_push( vm, &vm->param_stack, (uintptr_t)foo );

	return true;
}

static bool c4_minift_free( minift_vm_t *vm ){
	void *ptr = (void *)minift_pop( vm, &vm->param_stack );

	if ( !vm->running ){
		return false;
	}

	c4a_free( &minift_heap, ptr );
	minift_push( vm, &vm->param_stack, 0 );

	return true;
}

static bool c4_minift_open_file( minift_vm_t *vm ){
	char *fam  = popptr( vm );
	char *name = popptr( vm );

	FILE *fp = fopen( name, fam );

	pushptr( vm, fp );
	pushnum( vm, fp? 0 : 1 );

	return true;
}

static bool c4_minift_close_file( minift_vm_t *vm ){
	fclose( popptr( vm ));

	return true;
}

static bool c4_minift_read_file( minift_vm_t *vm ){
	FILE *fp          = popptr( vm );
	unsigned long len = popnum( vm );
	char *buf         = popptr( vm );

	unsigned long n = fread( buf, len, 1, fp );

	pushnum( vm, n );
	pushnum( vm, ferror(fp) );

	return true;
}

static bool c4_minift_read_line( minift_vm_t *vm ){
	FILE *fp          = popptr( vm );
	unsigned long len = popnum( vm );
	char *buf         = popptr( vm );

	fgets( buf, len, fp );

	pushnum( vm, strlen(buf) );
	pushnum( vm, ferror(fp) );

	return true;
}

static bool c4_minift_write_file( minift_vm_t *vm ){
	return true;
}

static bool c4_minift_write_line( minift_vm_t *vm ){
	return true;
}

extern void set_cur_include( FILE *fp );

static bool c4_minift_include_file( minift_vm_t *vm ){
	set_cur_include( popptr( vm ));

	return true;
}

static minift_archive_entry_t c4_words[] = {
	// general interface words
	{ "sendmsg",  c4_minift_sendmsg, 0 },
	{ "recvmsg",  c4_minift_recvmsg, 0 },
	/*
	{ "block@",   c4_minift_block_read, 0 },
	{ "block!",   c4_minift_block_write, 0 },
	*/
	{ "putchar",  c4_minift_console_put_char, 0 },
	{ "setpos",   c4_minift_console_set_pos, 0 },
	{ "clear",    c4_minift_console_clear, 0 },
	{ "consinfo", c4_minift_console_info, 0 },
	{ "lookup",   c4_minift_name_lookup, 0 },
	{ "key",      c4_minift_key, 0 },

	// allocation words
	// TODO: implement 'resize' once realloc is implemented in c4alloc
	{ "allocate", c4_minift_allocate },
	{ "free",     c4_minift_free },

	// file words
	{ "open-file",    c4_minift_open_file },
	{ "close-file",   c4_minift_close_file },
	{ "read-file",    c4_minift_read_file },
	{ "read-line",    c4_minift_read_line },
	{ "write-file",   c4_minift_write_file },
	{ "write-line",   c4_minift_write_line },
	{ "include-file", c4_minift_include_file },
};

static minift_archive_t c4_archive = {
	.name    = "c4",
	.entries = c4_words,
	.size    = sizeof(c4_words) / sizeof(minift_archive_entry_t),
};

void add_c4_archives( minift_vm_t *vm ){
	minift_archive_add( vm, &c4_archive );
}

void init_c4_allocator( minift_vm_t *vm ){
	c4a_heap_init( &minift_heap, 0xd0000000 );
}
