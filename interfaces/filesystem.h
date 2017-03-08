#ifndef _C4OS_FILESYSTEM_INTERFACE_H
#define _C4OS_FILESYSTEM_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4rt/ringbuffer.h>
#include <c4/message.h>
#include <c4/paging.h>
#include <stddef.h>

enum {
	FS_MSG_PING = MESSAGE_TYPE_END_RESERVED,
	FS_MSG_PONG,

	FS_MSG_BUFFER,
	FS_MSG_CONNECT,
	FS_MSG_DISCONNECT,

	FS_MSG_FIND_NAME,
	FS_MSG_GET_ROOT_DIR,
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
	unsigned inode;
	unsigned type;
} fs_dirent_t;

typedef struct fs_connection {
	c4_ringbuf_t *buffer;
	fs_node_t current_node;
	unsigned  state;
	unsigned  index;

	union {
		unsigned server;
		unsigned client;
	};
} fs_connection_t;

static inline void fs_dirent_to_node( fs_dirent_t *dirent, fs_node_t *node ){
	node->inode = dirent->inode;
	node->type  = dirent->type;
	node->size  = 0;
}

static inline int fs_connect( unsigned id, void *page, fs_connection_t *conn ){
	message_t msg = {
		.type = FS_MSG_CONNECT,
		.data = { 0 },
	};

	c4_msg_send( &msg, id );
	c4_msg_recieve( &msg, id );

	if ( msg.type == FS_MSG_ERROR ){
		return -msg.data[0];
	}

	C4_ASSERT( msg.type == FS_MSG_BUFFER );

	void *mapaddr = (void *)msg.data[0];
	c4_ringbuf_init( page, PAGE_SIZE );
	c4_mem_map_to( id, page, mapaddr, 1, PAGE_WRITE | PAGE_READ );

	conn->buffer = page;
	conn->server = id;
	conn->state  = FS_STATE_CONNECTED;

	return 0;
}

static inline void fs_disconnect( fs_connection_t *conn ){
	message_t msg = {
		.type = FS_MSG_DISCONNECT,
		.data = { 0 },
	};

	c4_msg_send( &msg, conn->server );
	c4_mem_unmap( conn->server, conn->buffer );

	conn->state  = FS_STATE_DISCONNECTED;
	conn->server = 0;
	conn->buffer = NULL;
}

static inline int fs_find_name( fs_connection_t *conn,
                                fs_node_t *nodebuf,
                                char *name,
                                size_t namelen )
{
	if ( conn->state != FS_STATE_CONNECTED ){
		return -FS_ERROR_NOT_CONNECTED;
	}

	if ( !c4_ringbuf_can_write( conn->buffer, namelen )){
		return -FS_ERROR_SERVER_BUSY;
	}

	message_t msg = {
		.type = FS_MSG_FIND_NAME,
		.data = { namelen },
	};

	c4_ringbuf_write( conn->buffer, name, namelen );
	c4_msg_send( &msg, conn->server );
	c4_msg_recieve( &msg, conn->server );

	if ( msg.type == FS_MSG_ERROR ){
		return -msg.data[0];
	}

	nodebuf->inode = msg.data[0];
	nodebuf->type  = msg.data[1];
	nodebuf->size  = msg.data[2];

	return 1;
}

static inline int fs_get_root_dir( unsigned id, fs_node_t *node ){
	message_t msg = { .type = FS_MSG_GET_ROOT_DIR, };

	c4_msg_send( &msg, id );
	c4_msg_recieve( &msg, id );

	C4_ASSERT( msg.type == FS_MSG_COMPLETED );

	node->inode = msg.data[0];
	node->type  = msg.data[1];
	node->size  = msg.data[2];

	return 1;
}

static inline void fs_set_node( fs_connection_t *conn, fs_node_t *node ){
	message_t msg = {
		.type = FS_MSG_SET_NODE,
		.data = { node->inode, },
	};

	conn->current_node = *node;

	c4_msg_send( &msg, conn->server );
	c4_msg_recieve( &msg, conn->server );
}

static inline int fs_list_dir( fs_connection_t *conn ){
	message_t msg = { .type = FS_MSG_LIST_DIR, };

	c4_msg_send( &msg, conn->server );
	c4_msg_recieve( &msg, conn->server );

	return msg.data[0];
}

static inline int fs_next_dirent( fs_connection_t *conn, fs_dirent_t *dirent ){
	if ( !c4_ringbuf_can_read( conn->buffer, sizeof( fs_dirent_t ))){
		int n = fs_list_dir( conn );

		if ( n <= 0 )
			return n;
	}

	c4_ringbuf_read( conn->buffer, dirent, sizeof( fs_dirent_t ));

	return 1;
}

#endif
