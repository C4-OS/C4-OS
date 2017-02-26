#include <c4rt/c4rt.h>
#include <c4/paging.h>
#include <miniforth/stubs.h>
#include <miniforth/miniforth.h>

#include <interfaces/block.h>
#include <interfaces/console.h>
#include <nameserver/nameserver.h>

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

static minift_archive_entry_t c4_words[] = {
	{ "sendmsg",  c4_minift_sendmsg, 0 },
	{ "recvmsg",  c4_minift_recvmsg, 0 },
	{ "block@",   c4_minift_block_read, 0 },
	{ "block!",   c4_minift_block_write, 0 },
	{ "putchar",  c4_minift_console_put_char, 0 },
	{ "setpos",   c4_minift_console_set_pos, 0 },
	{ "clear",    c4_minift_console_clear, 0 },
	{ "consinfo", c4_minift_console_info, 0 },
	{ "lookup",   c4_minift_name_lookup, 0 },
};

static minift_archive_t c4_archive = {
	.name    = "c4",
	.entries = c4_words,
	.size    = sizeof(c4_words) / sizeof(minift_archive_entry_t),
};

void add_c4_archives( minift_vm_t *vm ){
	minift_archive_add( vm, &c4_archive );
}
