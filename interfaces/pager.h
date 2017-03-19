#ifndef _C4OS_PAGER_INTERFACE_H
#define _C4OS_PAGER_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	// pagers also must handle MESSAGE_TYPE_PAGE_FAULT messages
	PAGER_MSG_REQUEST_PAGES = 0xbeef10af,
	PAGER_MSG_AVAILABLE_MEM,
};

void *pager_request_pages( unsigned  pager,
                           uintptr_t virt,
                           unsigned  permissions,
                           unsigned  n_pages )
{
	message_t msg = {
		.type = PAGER_MSG_REQUEST_PAGES,
		.data = {
			virt,
			permissions,
			n_pages,
		}
	};

	c4_msg_send( &msg, pager );
	c4_msg_recieve( &msg, pager );

	return (void *)virt;
}

unsigned long pager_available_mem( unsigned pager ){
	message_t msg = { .type = PAGER_MSG_AVAILABLE_MEM, };

	c4_msg_send( &msg, pager );
	c4_msg_recieve( &msg, pager );

	return msg.data[0];
}

#endif
