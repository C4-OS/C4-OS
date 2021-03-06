#ifndef _C4OS_NAMESERVER_H
#define _C4OS_NAMESERVER_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
	NAME_LOOKUP_FAILED,
};

static inline unsigned nameserver_hash( const char *str ){
	unsigned hash = 757;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 7) + hash + c);
	}

	return hash;
}

static inline void nameserver_bind( unsigned server,
                                    const char *name,
                                    int32_t endpoint )
{
	int32_t temp = c4_send_temp_endpoint(server);

	c4_debug_printf("--- thread %u: binding %s\n", c4_get_id(), name);
	C4_ASSERT(endpoint > 0);

	message_t msg = {
		.type = NAME_BIND,
		.data = { nameserver_hash(name) },
	};

	c4_msg_send( &msg, temp );

	int ret = c4_cspace_grant(endpoint, temp,
	                          CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE );
	C4_ASSERT(ret >= 0);
	c4_cspace_remove(C4_CURRENT_CSPACE, temp);
}

static inline unsigned nameserver_lookup( unsigned server, const char *name ){
	c4_debug_printf("--- thread %u: trying to find %s\n", c4_get_id(), name);

	int responseq = c4_send_temp_endpoint(server);

	message_t msg = {
		.type = NAME_LOOKUP,
		.data = { nameserver_hash(name) },
	};

	c4_msg_send(&msg, responseq);
	c4_msg_recieve(&msg, responseq);
	c4_cspace_remove(C4_CURRENT_CSPACE, responseq);
	C4_ASSERT(msg.type == MESSAGE_TYPE_GRANT_OBJECT);

	if (msg.type == MESSAGE_TYPE_GRANT_OBJECT){
		c4_debug_printf("--- thread %u: got %u\n", c4_get_id(), msg.data[5]);
		return msg.data[5];
	}

	else {
		return 0;
	}
}

static inline unsigned nameserver_lookup_hash(unsigned server, unsigned hash){
	c4_debug_printf("--- thread %u: trying to find 0x%x\n", c4_get_id(), hash);

	int responseq = c4_send_temp_endpoint(server);

	message_t msg = {
		.type = NAME_LOOKUP,
		.data = { hash },
	};

	c4_msg_send(&msg, responseq);
	c4_msg_recieve(&msg, responseq);
	c4_cspace_remove(C4_CURRENT_CSPACE, responseq);
	C4_ASSERT(msg.type == MESSAGE_TYPE_GRANT_OBJECT);

	if (msg.type == MESSAGE_TYPE_GRANT_OBJECT){
		c4_debug_printf("--- thread %u: got %u\n", c4_get_id(), msg.data[5]);
		return msg.data[5];
	}

	else {
		return 0;
	}
}

#endif
