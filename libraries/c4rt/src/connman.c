#include <c4/capability.h>
#include <c4rt/connman.h>
#include <c4rt/mem.h>
#include <c4rt/c4rt.h>

#include <c4rt/prng.h>
#include <c4rt/ringbuffer.h>
#include <c4rt/andtree.h>

// TODO: this is included for malloc() and stuff, it would probably be a good
//       idea to have c4 prefixes for the c4rt allocator, both for code
//       cleanliness and to prevent having problems with conflicting malloc
//       implementations once a full libc is ported, or people implementing
//       their own, etc
#include <c4rt/stublibc.h>

bool c4rt_connman_connect(c4rt_conn_t *conn, uint32_t server){
	conn->async_port = c4_msg_create_async();
	C4_ASSERT(c4_memobj_alloc(&conn->bufobj, PAGE_SIZE, PAGE_READ|PAGE_WRITE));

	// TODO: add size field to memobj struct and add syscall to get the size
	//       of page objects
	c4_ringbuf_init(conn->bufobj.vaddrptr, PAGE_SIZE);

	int32_t temppoint = c4_send_temp_endpoint(server);
	C4_ASSERT(temppoint > 0);

	// send connect request
	message_t msg = { .type = C4RT_CONNMAN_CONNECT, };
	c4_msg_send(&msg, temppoint);

	// get client ID number
	c4_msg_recieve(&msg, temppoint);
	conn->client_id = msg.data[0];

	// send objects for communication
	c4_cspace_grant(conn->async_port, temppoint,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);
	c4_cspace_grant(conn->bufobj.page_obj, temppoint,
	                CAP_ACCESS | CAP_MODIFY | CAP_MULTI_USE | CAP_SHARE);

	c4_cspace_remove(C4_CURRENT_CSPACE, temppoint);
	return true;
}

void c4rt_connman_disconnect(c4rt_conn_t *conn){
	message_t msg = { .type = C4RT_CONNMAN_DISCONNECT, };

	c4_msg_send(&msg, conn->server_port);
	c4_cspace_remove(C4_CURRENT_CSPACE, conn->async_port);
	c4_memobj_free(&conn->bufobj);
}

bool c4rt_connman_send(c4rt_conn_t *conn, message_t *msg){
	message_t temp = {
		.type = C4RT_CONNMAN_NEW_MESSAGE,
		.data = { conn->client_id, },
	};

	c4_msg_send_async(msg, conn->async_port);
	c4_msg_send(&temp, conn->server_port);

	return true;
}

bool c4rt_connman_recv(c4rt_conn_t *conn, message_t *msg, bool block){
	return c4_msg_recieve_async(msg, conn->async_port,
	                            block? MESSAGE_ASYNC_BLOCK : 0);
}

static int connman_get_client_id(void *ptr){
	c4rt_conn_t *conn = ptr;

	return (int)conn->client_id;
}

void c4rt_connman_server_init(c4rt_conn_server_t *serv, uint32_t port){
	andtree_init(&serv->tree, connman_get_client_id);
	serv->server_port = port;
}

void c4rt_connman_server_deinit(c4rt_conn_server_t *serv){
	andtree_deinit(&serv->tree);
	// TODO: deinit any clients still connected
}

static c4rt_conn_t *handle_connect(c4rt_conn_server_t *serv, uint32_t temp){
	message_t msgbuf;
	c4rt_conn_t *new_client = NULL;

	// have a connect request, allocate a new client
	new_client = calloc(1, sizeof(*new_client));
	new_client->client_id = c4rt_prng_u32();
	andtree_insert(&serv->tree, new_client);
	c4_debug_printf("--- connman: new client %x\n", new_client->client_id);

	msgbuf.type = C4RT_CONNMAN_CLIENT_ID;
	msgbuf.data[0] = new_client->client_id;
	c4_msg_send(&msgbuf, temp);

	c4_msg_recieve(&msgbuf, temp);
	if (msgbuf.type != MESSAGE_TYPE_GRANT_OBJECT) {
		goto error_out;
	}

	new_client->async_port = msgbuf.data[5];

	c4_msg_recieve(&msgbuf, temp);
	if (msgbuf.type != MESSAGE_TYPE_GRANT_OBJECT) {
		goto error_out;
	}

	int obj = msgbuf.data[5];
	C4_ASSERT(
	c4_memobj_region_map(obj, &new_client->bufobj,
	                     PAGE_SIZE, PAGE_READ|PAGE_WRITE));
	new_client->ringbuf = new_client->bufobj.vaddrptr;
	return new_client;

error_out:
	c4_debug_printf("--- connman: error connecting client %x...",
	                new_client->client_id);
	andtree_remove_key(&serv->tree, new_client->client_id);
	free(new_client);
	return NULL;
}

static c4rt_conn_t *handle_new_message( c4rt_conn_server_t *serv,
                                        message_t *buf,
                                        uint32_t client )
{
	c4rt_andnode_t *node = andtree_find_key(&serv->tree, client);

	if (!node) {
		c4_debug_printf("--- connman: invalid client %x!\n", client);
		return NULL;
	}

	c4rt_conn_t *conn = node->data;

	if (!c4_msg_recieve_async(buf, conn->async_port, 0)) {
		c4_debug_printf("--- connman: request with no message (%x)!\n", client);
		return NULL;
	}

	return conn;
}

c4rt_conn_t *c4rt_connman_server_listen( c4rt_conn_server_t *serv,
                                         message_t *msg )
{
	c4rt_conn_t *conn = NULL;

	while (!conn) {
		message_t msgbuf;

		// wait for temporary sync endpoint
		c4_msg_recieve(&msgbuf, serv->server_port);
		if (msgbuf.type != MESSAGE_TYPE_GRANT_OBJECT) {
			continue;
		}

		// wait for request on temp endpoint
		int temp = msgbuf.data[5];
		c4_msg_recieve(&msgbuf, serv->server_port);

		if (msgbuf.type != C4RT_CONNMAN_CONNECT) {
			goto cleanup;
		}

		switch (msgbuf.type) {
			case C4RT_CONNMAN_CONNECT:
				handle_connect(serv, temp);
				break;

			case C4RT_CONNMAN_DISCONNECT:
				//handle_disconnect(serv, temp);
				break;

			case C4RT_CONNMAN_NEW_MESSAGE:
				conn = handle_new_message(serv, msg, msgbuf.data[0]);
				break;

			default: goto cleanup;
		}

cleanup:
		c4_cspace_remove(C4_CURRENT_CSPACE, temp);
	}

	return conn;
}

void c4rt_connman_server_respond(c4rt_conn_t *conn, message_t *msg){
	c4_msg_send_async(msg, conn->async_port);
}
