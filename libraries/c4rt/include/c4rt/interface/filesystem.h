#ifndef _C4OS_FILESYSTEM_INTERFACE_H
#define _C4OS_FILESYSTEM_INTERFACE_H 1

#include <stddef.h>
#include <c4/message.h>
#include <c4/paging.h>
#include <c4rt/c4rt.h>
#include <c4rt/ringbuffer.h>
#include <c4rt/connman.h>
#include <c4rt/mem.h>
#include <c4rt/interface/pager.h>

enum {
	FS_MSG_PING = MESSAGE_TYPE_END_RESERVED,
	FS_MSG_PONG,

	FS_MSG_BUFFER,
	FS_MSG_CONNECT,
	FS_MSG_DISCONNECT,
	FS_MSG_RESTORE_STATE,

	FS_MSG_FIND_NAME,
	FS_MSG_GET_ROOT_DIR,
	FS_MSG_GET_NODE_INFO,
	FS_MSG_LIST_DIR,
	FS_MSG_SET_NODE,
	FS_MSG_CREATE_NODE,
	FS_MSG_READ_BLOCK,
	FS_MSG_WRITE_BLOCK,

	FS_MSG_NEXT_DATA,
	FS_MSG_COMPLETED,
	FS_MSG_ERROR,
	FS_MSG_END,
};

enum {
	FS_ERROR_NONE,
	FS_ERROR_NOT_DIRECTORY,
	FS_ERROR_NOT_CONNECTED,
	FS_ERROR_NOT_FOUND,
	FS_ERROR_SERVER_BUSY,
	FS_ERROR_QUEUE_FULL,
	FS_ERROR_BAD_REQUEST,
};

enum {
	FS_STATE_DISCONNECTED,
	FS_STATE_CONNECTED,
	FS_STATE_READING,
	FS_STATE_WRITING,
	FS_STATE_ERROR,
};

enum {
	FILE_TYPE_UNKNOWN,
	FILE_TYPE_FILE,
	FILE_TYPE_DIRECTORY,
	FILE_TYPE_SYMLINK,
};

enum {
	FS_MAX_NAME_LEN = 256,
};

typedef struct fs_node {
	unsigned long inode;

	unsigned type;
	size_t   size;
} fs_node_t;

typedef struct fs_dirent {
	char   name[FS_MAX_NAME_LEN];
	size_t name_len;
	unsigned long inode;
	unsigned type;
} fs_dirent_t;

typedef struct fs_connection {
	c4rt_conn_t server;

	fs_node_t current_node;
	unsigned  index;
} fs_connection_t;

static inline int fs_get_node_info( fs_connection_t *conn,
                                    unsigned long inode,
                                    fs_node_t *node )
{
	message_t msg = {
		.type = FS_MSG_GET_NODE_INFO,
		.data = { inode, },
	};

	c4rt_connman_call(&conn->server, &msg);

	if ( msg.type == FS_MSG_ERROR )
		return -msg.data[0];

	node->inode = inode;
	node->size  = msg.data[0];
	node->type  = msg.data[1];

	return 0;
}

static inline int fs_dirent_to_node( fs_connection_t *conn,
                                     fs_dirent_t *dirent,
                                     fs_node_t *node )
{
	return fs_get_node_info(conn, dirent->inode, node);
}

static inline int fs_connect(uint32_t serv_endpoint, fs_connection_t *conn){
	// TODO: handle error if we can't connect to the server
	c4rt_connman_connect(&conn->server, serv_endpoint);
	return 0;
}

static inline void fs_disconnect( fs_connection_t *conn ){
	c4rt_connman_disconnect(&conn->server);
}

static inline int fs_find_name( fs_connection_t *conn,
                                fs_node_t *nodebuf,
                                char *name,
                                size_t namelen )
{
	message_t msg = {
		.type = FS_MSG_FIND_NAME,
		.data = { namelen },
	};

	c4_ringbuf_write(conn->server.ringbuf, name, namelen);
	c4rt_connman_call(&conn->server, &msg);

	if (msg.type == FS_MSG_ERROR) {
		return -msg.data[0];
	}

	nodebuf->inode = msg.data[0];
	nodebuf->type  = msg.data[1];
	nodebuf->size  = msg.data[2];

	return 1;
}

static inline int fs_get_root_dir(fs_connection_t *conn, fs_node_t *node){
	message_t msg = { .type = FS_MSG_GET_ROOT_DIR, };

	c4rt_connman_call(&conn->server, &msg);
	C4_ASSERT(msg.type == FS_MSG_COMPLETED);
	C4_ASSERT(msg.type != FS_MSG_GET_ROOT_DIR);

	node->inode = msg.data[0];
	node->type  = msg.data[1];
	node->size  = msg.data[2];

	return 1;
}

static inline int fs_set_node( fs_connection_t *conn, fs_node_t *node ){
	message_t msg = {
		.type = FS_MSG_SET_NODE,
		.data = { node->inode, },
	};

	conn->current_node = *node;

	c4rt_connman_call(&conn->server, &msg);
	C4_ASSERT(msg.type == FS_MSG_COMPLETED);

	return msg.type == FS_MSG_ERROR? -msg.data[0] : 1;
}

static inline int fs_list_dir( fs_connection_t *conn ){
	message_t msg = { .type = FS_MSG_LIST_DIR, };

	c4rt_connman_call(&conn->server, &msg);

	return msg.data[0];
}

static inline int fs_next_dirent( fs_connection_t *conn, fs_dirent_t *dirent ){
	if (!c4_ringbuf_can_read(conn->server.ringbuf, sizeof(fs_dirent_t))) {
		int n = fs_list_dir(conn);

		if (n <= 0)
			return n;
	}

	c4_ringbuf_read(conn->server.ringbuf, dirent, sizeof(fs_dirent_t));

	return 1;
}

static inline int fs_read_block( fs_connection_t *conn,
                                 void *buffer,
                                 size_t maxlen )
{
	if (c4_ringbuf_empty(conn->server.ringbuf)) {
		message_t msg = {
			.type = FS_MSG_READ_BLOCK,
			.data = { 0, },
		};

		c4rt_connman_call(&conn->server, &msg);

		if (msg.type == FS_MSG_ERROR) {
			c4_debug_printf("--- error: %u\n", msg.data[0]);
			return -msg.data[0];
		}
	}

	int n = c4_ringbuf_read(conn->server.ringbuf, buffer, maxlen);
	conn->index += n;

	return n;
}

#endif
