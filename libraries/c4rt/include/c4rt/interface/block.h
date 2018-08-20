// block device interface stubs
#ifndef _C4OS_BLOCK_INTERFACE_H
#define _C4OS_BLOCK_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4rt/mem.h>
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

static inline bool block_access( uint32_t id,
                                 message_t *msg,
                                 c4_mem_object_t *buffer )
{
	bool error = BLOCK_MSG_NO_ERROR;
	uint32_t temppoint = c4_send_temp_endpoint(id);

	// send read request
	c4_msg_send(msg, temppoint);
	c4_cspace_grant(buffer->page_obj, temppoint,
	                CAP_MODIFY | CAP_ACCESS | CAP_SHARE);
	// wait for block device to process the request
	c4_msg_recieve(msg, temppoint);

	C4_ASSERT(msg->type == BLOCK_MSG_COMPLETED);
	if ( msg->type != BLOCK_MSG_COMPLETED ){
		error = BLOCK_MSG_HAD_ERROR;
		goto done;
	}

done:
	c4_cspace_remove( C4_CURRENT_CSPACE, temppoint );

	return error;
}

static inline bool block_read( unsigned id,
                               c4_mem_object_t *buffer,
                               unsigned drive,
                               unsigned location,
                               unsigned size )
{
	message_t msg = {
		.type = BLOCK_MSG_READ,
		.data = {drive, location, size},
	};

	return block_access(id, &msg, buffer);
}

static inline bool block_write( unsigned id,
                                c4_mem_object_t *buffer,
                                unsigned drive,
                                unsigned location,
                                unsigned size )
{
	message_t msg = {
		.type = BLOCK_MSG_WRITE,
		.data = { drive, location, size },
	};

	return block_access(id, &msg, buffer);
}

#endif
