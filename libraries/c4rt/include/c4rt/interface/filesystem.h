#ifndef _C4OS_FILESYSTEM_INTERFACE_H
#define _C4OS_FILESYSTEM_INTERFACE_H 1
#include <c4rt/c4rt.h>
#include <c4rt/ringbuffer.h>
#include <c4rt/interface/pager.h>
#include <c4/message.h>
#include <c4/paging.h>
#include <stddef.h>

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
	c4_ringbuf_t *buffer;
	fs_node_t current_node;
	unsigned  state;
	unsigned  index;
	int32_t   page_obj;
	int32_t   temp_point;

	union {
		unsigned server;
		unsigned client;
	};
} fs_connection_t;

static inline int fs_get_node_info( unsigned id,
                                    unsigned long inode,
                                    fs_node_t *node )
{
	message_t msg = {
		.type = FS_MSG_GET_NODE_INFO,
		.data = { inode, },
	};

	c4_msg_send( &msg, id );
	c4_msg_recieve( &msg, id );

	if ( msg.type == FS_MSG_ERROR )
		return -msg.data[0];

	node->inode = inode;
	node->size  = msg.data[0];
	node->type  = msg.data[1];

	return 0;
}

static inline int fs_dirent_to_node( unsigned id,
                                     fs_dirent_t *dirent,
                                     fs_node_t *node )
{
	return fs_get_node_info( id, dirent->inode, node );
}

//static inline int fs_connect( unsigned id, void *page, fs_connection_t *conn ){
static inline int fs_connect( unsigned serv_endpoint, fs_connection_t *conn ){
	conn->temp_point = c4_send_temp_endpoint( serv_endpoint );
	C4_ASSERT( conn->temp_point > 0 );

	message_t msg = {
		.type = FS_MSG_CONNECT,
		.data = { 0 },
	};

	c4_msg_send( &msg, conn->temp_point );
	c4_msg_recieve( &msg, conn->temp_point );

	if ( msg.type == FS_MSG_ERROR ){
		return -msg.data[0];
	}

	C4_ASSERT( msg.type == FS_MSG_BUFFER );

	// TODO: dynamically allocate an address once a virtual memory manager is
	//       implemented in the c4rt
	conn->buffer = (void *)0xf11e0000;
	conn->page_obj = pager_request_pages( C4_PAGER, 0xf11e0000,
	                                      PAGE_READ | PAGE_WRITE, 1 );
	C4_ASSERT( conn->page_obj > 0 );
	c4_cspace_grant( conn->page_obj, conn->temp_point,
	                 CAP_ACCESS | CAP_MODIFY | CAP_SHARE );

	//void *mapaddr = (void *)msg.data[0];
	//c4_ringbuf_init( page, PAGE_SIZE );
	//c4_mem_map_to( id, page, mapaddr, 1, PAGE_WRITE | PAGE_READ );

	//conn->buffer = page;
	//conn->server = id;
	conn->server = serv_endpoint;
	conn->state  = FS_STATE_CONNECTED;

	return 0;
}

static inline void fs_disconnect( fs_connection_t *conn ){
	message_t msg = {
		.type = FS_MSG_DISCONNECT,
		.data = { 0 },
	};

	c4_msg_send( &msg, conn->server );
	//c4_mem_unmap( conn->server, conn->buffer );
	c4_addrspace_unmap( C4_CURRENT_ADDRSPACE, (uintptr_t)conn->buffer );
	c4_cspace_remove( C4_CURRENT_CSPACE, conn->page_obj );
	c4_cspace_remove( C4_CURRENT_CSPACE, conn->server );

	conn->state  = FS_STATE_DISCONNECTED;
}

static inline int fs_set_node( fs_connection_t *conn, fs_node_t *node );

static inline int fs_restore_state( fs_connection_t *old_conn ){
	message_t msg = {
		.type = FS_MSG_RESTORE_STATE,
		.data = {
			old_conn->state,
			old_conn->index,
		},
	};

	int temp;
	if (( temp = fs_set_node( old_conn, &old_conn->current_node )) <= 0 ){
		return temp;
	}

	//c4_msg_send( &msg, old_conn->server );
	//c4_msg_recieve( &msg, old_conn->server );
	c4_msg_send( &msg, old_conn->temp_point );
	c4_msg_recieve( &msg, old_conn->temp_point );

	return msg.type == FS_MSG_ERROR? -msg.data[0] : 1;
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
	c4_msg_send( &msg, conn->temp_point );
	c4_msg_recieve( &msg, conn->temp_point );

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

static inline int fs_set_node( fs_connection_t *conn, fs_node_t *node ){
	message_t msg = {
		.type = FS_MSG_SET_NODE,
		.data = { node->inode, },
	};

	conn->current_node = *node;

	c4_msg_send( &msg, conn->temp_point );
	c4_msg_recieve( &msg, conn->temp_point );

	return msg.type == FS_MSG_ERROR? -msg.data[0] : 1;
}

static inline int fs_list_dir( fs_connection_t *conn ){
	message_t msg = { .type = FS_MSG_LIST_DIR, };

	c4_msg_send( &msg, conn->temp_point );
	c4_msg_recieve( &msg, conn->temp_point );

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

static inline int fs_read_block( fs_connection_t *conn,
                                 void *buffer,
                                 size_t maxlen )
{
	if ( c4_ringbuf_empty( conn->buffer )){
		message_t msg = {
			.type = FS_MSG_READ_BLOCK,
			.data = { 0, },
		};

		c4_msg_send( &msg, conn->temp_point );
		c4_msg_recieve( &msg, conn->temp_point );

		if ( msg.type == FS_MSG_ERROR ){
			c4_debug_printf( "--- error: %u\n", msg.data[0] );
			return -msg.data[0];
		}
	}

	int n = c4_ringbuf_read( conn->buffer, buffer, maxlen );
	conn->index += n;

	return n;
}

static inline void fs_set_connection_info( fs_connection_t *conn,
                                           fs_node_t *node,
                                           unsigned server )
{
    C4_ASSERT( conn );
    C4_ASSERT( conn->state == FS_STATE_DISCONNECTED );

    c4_ringbuf_init( page, PAGE_SIZE );

    conn->server       = server;
    conn->buffer       = page;
    conn->current_node = *node;
}

static inline int fs_read_block_autoconn( fs_connection_t *conn,
                                          void *databuf,
                                          size_t maxlen )
{
	if ( c4_ringbuf_empty( conn->buffer )){
		bool should_disconnect = false;
		message_t msg = {
			.type = FS_MSG_READ_BLOCK,
			.data = { 0, },
		};

		if ( conn->state == FS_STATE_DISCONNECTED ){
			fs_connect( conn->server, conn );
			fs_restore_state( conn );
			should_disconnect = true;
		}

		c4_msg_send( &msg, conn->server );
		c4_msg_recieve( &msg, conn->server );

		if ( should_disconnect ){
			fs_disconnect( conn );
		}

		if ( msg.type == FS_MSG_ERROR ){
			c4_debug_printf( "--- error: %u\n", msg.data[0] );
			return -msg.data[0];
		}
	}

	int n = c4_ringbuf_read( conn->buffer, databuf, maxlen );
	conn->index += n;

	return n;
}

#endif
