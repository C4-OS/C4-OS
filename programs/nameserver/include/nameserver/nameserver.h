#ifndef _C4OS_NAMESERVER_H
#define _C4OS_NAMESERVER_H 1
#include <c4rt/c4rt.h>
#include <c4/message.h>

enum {
	NAME_BIND = 0x1024,
	NAME_UNBIND,
	NAME_LOOKUP,
	NAME_RESULT,
};

static inline unsigned nameserver_hash( const char *str ){
	unsigned hash = 757;
	int c;

	while (( c = *str++ )){
		hash = ((hash << 7) + hash + c);
	}

	return hash;
}

static inline void nameserver_bind( unsigned server, const char *name ){
	message_t msg = {
		.type = NAME_BIND,
		.data = { nameserver_hash(name) },
	};

	c4_msg_send( &msg, server );
}

static inline unsigned nameserver_lookup( unsigned server, const char *name ){
	message_t msg = {
		.type = NAME_LOOKUP,
		.data = { nameserver_hash(name) },
	};

	c4_msg_send( &msg, server );
	c4_msg_recieve( &msg, server );

	return msg.data[0];
}

#endif
