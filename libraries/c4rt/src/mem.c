#include <c4rt/c4rt.h>

void *c4_request_physical( uintptr_t virt,
                           uintptr_t physical,
                           unsigned size,
                           unsigned permissions )
{
	message_t msg = {
		.type = MESSAGE_TYPE_REQUEST_PHYS,
		.data = {
			virt,
			physical,
			size,
			permissions
		},
	};

	c4_msg_send( &msg, 0 );

	return (void *)virt;
}

void *c4_request_page( unsigned pager, uintptr_t virt, unsigned permissions ){
	message_t msg = {
		.type = 0xbeef10af,
		.data = { virt, permissions, },
	};

	c4_msg_send( &msg, pager );
	// wait for response from pager
	// TODO: check for errors/denied request and error out somehow
	c4_msg_recieve( &msg, pager );

	return (void *)virt;
}

int c4_mem_map_to( unsigned thread_id,
                   void *from,
                   void *to,
                   unsigned size,
                   unsigned permissions )
{
	message_t msg = {
		.type = MESSAGE_TYPE_MAP_TO,
		.data = {
			(uintptr_t)from,
			(uintptr_t)to,
			(uintptr_t)size,
			(uintptr_t)permissions,
		},
	};

	return c4_msg_send( &msg, thread_id );
}

int c4_mem_grant_to( unsigned thread_id,
                     void *from,
                     void *to,
                     unsigned size,
                     unsigned permissions )
{
	message_t msg = {
		.type = MESSAGE_TYPE_GRANT_TO,
		.data = {
			(uintptr_t)from,
			(uintptr_t)to,
			(uintptr_t)size,
			(uintptr_t)permissions,
		},
	};

	return c4_msg_send( &msg, thread_id );
}
