#include <c4rt/c4rt.h>
#include <nameserver/nameserver.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct name_entry {
	unsigned hash;
	//unsigned long thread;
	int32_t endpoint;
} name_entry_t;

enum {
	MAX_NAME_ENTRIES = 512,
};

static name_entry_t names[MAX_NAME_ENTRIES];

void bind_name( uint32_t endpoint, unsigned name ){
	unsigned i = 0;

	for ( ; i < MAX_NAME_ENTRIES && names[i].hash; i++ ){
		if ( name == names[i].hash )
			break;
	}

	c4_debug_printf( "--- nameserver: binding for %x at %u\n",
	                 name, i );

	names[i].endpoint = endpoint;
	names[i].hash     = name;
}

uint32_t lookup_name( unsigned name ){
	for ( unsigned i = 0; i < MAX_NAME_ENTRIES && names[i].hash; i++ ){
		if ( name == names[i].hash ){
			return names[i].endpoint;
		}
	}

	return 0;
}

void handle_bind( uint32_t endpoint, unsigned hash ){
	bind_name( endpoint, hash );
	c4_debug_printf( "--- nameserver: bound object %x at %x:%x\n",
	                 endpoint, hash, endpoint );
}

void handle_lookup( uint32_t responseq, unsigned hash ){
	int32_t obj = lookup_name( hash );
	c4_debug_printf("--- nameserver: %x:%x => %x\n", hash, responseq, obj);

	if (obj > 0){
		c4_cspace_grant( obj, responseq, CAP_MODIFY | CAP_SHARE | CAP_MULTI_USE );

	} else {
		message_t msg = { .type = NAME_LOOKUP_FAILED, };
		c4_msg_send(&msg, responseq);
	}

	c4_cspace_remove(C4_CURRENT_CSPACE, responseq);
}

int main(int argc, char *argv[]){
	while ( true ){
		message_t msg;
		int32_t obj;
		int32_t temp;
		int32_t id = 0;
		int ret;

		c4_debug_printf( "--- nameserver: waiting: %u\n", c4_get_id() );
		int k = c4_msg_recieve( &msg, 1 );
		c4_debug_printf( "--- nameserver: got %u\n", k );
		c4_debug_printf( "--- nameserver: recieved message %u\n", msg.type );

		C4_ASSERT(msg.type == MESSAGE_TYPE_GRANT_OBJECT);
		temp = msg.data[5];
		c4_debug_printf("--- nameserver: got temp endpoint...\n");
		c4_msg_recieve(&msg, temp);
		c4_debug_printf("--- nameserver: got real message\n");

		switch ( msg.type ){
			case NAME_LOOKUP:
				handle_lookup(temp, msg.data[0]);
				break;

			case NAME_BIND:
				id = msg.data[0];
				ret = c4_msg_recieve(&msg, temp);
				obj = msg.data[5];

				C4_ASSERT(msg.type == MESSAGE_TYPE_GRANT_OBJECT);
				C4_ASSERT(ret >= 0);
				C4_ASSERT(obj >= 0);

				handle_bind(obj, id);
				break;

			case NAME_UNBIND:
				break;

			default:
				c4_debug_printf( "--- nameserver: unknown message %u\n", msg.type );
				break;
		}

		c4_cspace_remove(C4_CURRENT_CSPACE, temp);
	}

	return 0;
}
