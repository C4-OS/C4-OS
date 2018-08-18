#ifndef _C4RT_CONNMAN_H
#define _C4RT_CONNMAN_H 1

/* TODO: this could be made entirely asyncronous, right now it only
 *       blocks on one syncronous message to the server port,
 *       which makes things simpler since then only one server port is
 *       needed. But, being fully asyncronous would be pretty useful,
 *       so after everything is working it's definitely a thing
 *       to consider
 */

#include <stdbool.h>
#include <stddef.h>
#include <c4rt/mem.h>
#include <c4rt/ringbuffer.h>
#include <c4rt/andtree.h>

enum {
	C4RT_CONNMAN_CONNECT = 0xdabd00d,
	C4RT_CONNMAN_DISCONNECT,
	C4RT_CONNMAN_CLIENT_ID,
	C4RT_CONNMAN_NEW_MESSAGE,
	C4RT_CONNMAN_WRITE,
	C4RT_CONNMAN_READ,
};

enum {
	C4RT_CONNMAN_NO_BLOCK = false,
	C4RT_CONNMAN_BLOCK = true,
};

// this struct is used both as the client handle, and the internal
// client struct of the server, which keeps things simpler.
typedef struct {
	uint32_t server_port;
	uint32_t async_to;
	uint32_t async_from;
	uint32_t client_id;
	void     *prog_data;

	c4_mem_object_t bufobj;
	c4_ringbuf_t *ringbuf;
} c4rt_conn_t;

typedef struct {
	c4rt_andtree_t tree;
	uint32_t server_port;
} c4rt_conn_server_t;

bool c4rt_connman_connect(c4rt_conn_t *conn, uint32_t server);
void c4rt_connman_disconnect(c4rt_conn_t *conn);

bool c4rt_connman_send(c4rt_conn_t *conn, message_t *msg);
bool c4rt_connman_recv(c4rt_conn_t *conn, message_t *msg, bool block);
bool c4rt_connman_call(c4rt_conn_t *conn, message_t *msg);

void c4rt_connman_server_init(c4rt_conn_server_t *serv, uint32_t port);
void c4rt_connman_server_deinit(c4rt_conn_server_t *serv);
c4rt_conn_t *c4rt_connman_server_listen(c4rt_conn_server_t *serv, message_t *msg);
void c4rt_connman_server_respond(c4rt_conn_t *conn, message_t *msg);

#endif
