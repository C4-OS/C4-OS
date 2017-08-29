#ifndef _C4OS_PAGER_INTERFACE_H
#define _C4OS_PAGER_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	// pagers also must handle MESSAGE_TYPE_PAGE_FAULT messages
	PAGER_MSG_REQUEST_PAGES = 0xbeef10af,
	PAGER_MSG_AVAILABLE_MEM,
};

static inline int32_t pager_request_pages( unsigned  pager,
                                           uintptr_t virt,
                                           unsigned  permissions,
                                           unsigned  n_pages )
{
	/*
	int temppoint = c4_msg_create_sync();
	C4_ASSERT( temppoint > 0 );

	c4_cspace_grant( temppoint, pager, CAP_ACCESS | CAP_MODIFY
	                                   | CAP_MULTI_USE | CAP_SHARE );
	 */

	int temppoint = c4_send_temp_endpoint( pager );

	message_t msg = {
		.type = PAGER_MSG_REQUEST_PAGES,
		.data = {
			//virt,
			permissions,
			n_pages,
		}
	};

	c4_msg_send( &msg, temppoint );
	c4_msg_recieve( &msg, temppoint );
	c4_cspace_remove( C4_CURRENT_CSPACE, temppoint );

	c4_addrspace_map( C4_CURRENT_ADDRSPACE, msg.data[5], (uintptr_t)virt, permissions );

	//return (void *)virt;
	return msg.data[5];
}

static inline unsigned long pager_available_mem( unsigned pager ){
	message_t msg = { .type = PAGER_MSG_AVAILABLE_MEM, };

	c4_msg_send( &msg, pager );
	c4_msg_recieve( &msg, pager );

	return msg.data[0];
}

#endif
