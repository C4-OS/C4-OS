// block device interface stubs
#ifndef _C4OS_BLOCK_INTERFACE_H
#define _C4OS_BLOCK_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>
#include <c4/paging.h>

enum {
	BLOCK_MSG_PING  = MESSAGE_TYPE_END_RESERVED,
	BLOCK_MSG_PONG,
	BLOCK_MSG_READ,
	BLOCK_MSG_WRITE,
	BLOCK_MSG_BUFFER,

	BLOCK_MSG_ERROR,
	BLOCK_MSG_COMPLETED,
};

enum {
	BLOCK_MSG_NO_ERROR  = false,
	BLOCK_MSG_HAD_ERROR = true,
};

static inline bool block_access( unsigned id,
                                 message_t *msg,
                                 void *page )
{
	// send read request
	c4_msg_send( msg, id );
	// wait for the address where a read buffer should be mapped to
	c4_msg_recieve( msg, id );
	void *mapaddr = (void *)msg->data[0];

	if ( msg->type != BLOCK_MSG_BUFFER )
		return BLOCK_MSG_HAD_ERROR;

	// map the buffer to the given address
	c4_mem_map_to( id, page, mapaddr, 1, PAGE_WRITE | PAGE_READ );
	// wait for the ata driver to finish and notify the sender
	c4_msg_recieve( msg, id );
	C4_ASSERT( msg->type == BLOCK_MSG_COMPLETED );

	// finally unmap the buffer
	c4_mem_unmap( id, mapaddr );

	return BLOCK_MSG_NO_ERROR;
}

static inline bool block_read( unsigned id,
                               void *page,
                               unsigned drive,
                               unsigned location,
                               unsigned size )
{
	message_t msg = {
		.type = BLOCK_MSG_READ,
		.data = { drive, location, size / 512 },
	};

	return block_access( id, &msg, page );
}

static inline bool block_write( unsigned id,
                                void *page,
                                unsigned drive,
                                unsigned location,
                                unsigned size )
{
	message_t msg = {
		.type = BLOCK_MSG_WRITE,
		.data = { drive, location, size / 512 },
	};

	return block_access( id, &msg, page );
}

#endif
