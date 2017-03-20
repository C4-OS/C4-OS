#include <ext2fs/ext2fs.h>
#include <interfaces/filesystem.h>
#include <c4rt/ringbuffer.h>
#include <c4rt/stublibc.h>
#include <stdbool.h>

// TODO: move these into per-thread datastructures once worker threads are
//       implemented
static fs_connection_t connection;
static ext2_inode_t    current_inode;

static void handle_connect( ext2fs_t *fs, message_t *request );
static void handle_disconnect( ext2fs_t *fs, message_t *request );
static void handle_restore_state( ext2fs_t *fs, message_t *request );
static void handle_set_node( ext2fs_t *fs, message_t *request );
static void handle_list_dir( ext2fs_t *fs, message_t *request );
static void handle_find_name( ext2fs_t *fs, message_t *request );
static void handle_get_rootdir( ext2fs_t *fs, message_t *request );
static void handle_get_node_info( ext2fs_t *fs, message_t *request );
static void handle_read_block( ext2fs_t *fs, message_t *request );

void ext2_handle_request( ext2fs_t *fs, message_t *request ){
	switch ( request->type ){
		case FS_MSG_CONNECT:       handle_connect( fs, request );       break;
		case FS_MSG_DISCONNECT:    handle_disconnect( fs, request );    break;
		case FS_MSG_RESTORE_STATE: handle_restore_state( fs, request ); break;
		case FS_MSG_SET_NODE:      handle_set_node( fs, request );      break;
		case FS_MSG_FIND_NAME:     handle_find_name( fs, request );     break;
		case FS_MSG_GET_ROOT_DIR:  handle_get_rootdir( fs, request );   break;
		case FS_MSG_GET_NODE_INFO: handle_get_node_info( fs, request ); break;
		case FS_MSG_LIST_DIR:      handle_list_dir( fs, request );      break;
		case FS_MSG_READ_BLOCK:    handle_read_block( fs, request );    break;
		default: break;
	}
}

static inline void send_error( message_t *request, unsigned error ){
	message_t msg = {
		.type = FS_MSG_ERROR,
		.data = { error },
	};

	c4_msg_send( &msg, request->sender );
}

static inline bool is_connected( message_t *request ){
	return connection.state == FS_STATE_CONNECTED
	    && connection.client == request->sender;
}

static inline void copy_name( fs_dirent_t *fs_ent, ext2_dirent_t *ext_ent ){
	unsigned k = 0;

	for ( ; k < ext_ent->name_length && k < FS_MAX_NAME_LEN - 1; k++ ){
		fs_ent->name[k] = ext_ent->name[k];
	}

	fs_ent->name[k] = '\0';
}

static inline unsigned translate_type( unsigned ext_type ){
	switch ( ext_type ){
		case EXT2_DIRENT_TYPE_FILE:    return FILE_TYPE_FILE;
		case EXT2_DIRENT_TYPE_DIR:     return FILE_TYPE_DIRECTORY;
		case EXT2_DIRENT_TYPE_SYMLINK: return FILE_TYPE_SYMLINK;
		default:                       return FILE_TYPE_UNKNOWN;
	}
}

static void handle_connect( ext2fs_t *fs, message_t *request ){
	if ( connection.state != FS_STATE_DISCONNECTED ){
		send_error( request, FS_ERROR_SERVER_BUSY );
		return;
	}

	connection.state  = FS_STATE_CONNECTED;
	connection.client = request->sender;
	connection.buffer = (void *)0xd0000000;

	message_t msg = {
		.type = FS_MSG_BUFFER,
		.data = { 0xd0000000 },
	};

	c4_msg_send( &msg, connection.client );
	c4_msg_recieve( &msg, connection.client );
	C4_ASSERT( msg.type == MESSAGE_TYPE_MAP_TO );
}

static void handle_disconnect( ext2fs_t *fs, message_t *request ){
	if ( request->sender != connection.client ){
		// sender isn't connected to this server, just ignore
		return;
	}

	connection.state  = FS_STATE_DISCONNECTED;
	connection.client = 0;
	connection.buffer = 0;
	connection.index  = 0;

	message_t msg;

	c4_msg_recieve( &msg, connection.client );
	C4_ASSERT( msg.type == MESSAGE_TYPE_UNMAP );
}

static void handle_restore_state( ext2fs_t *fs, message_t *request ){
	if ( !is_connected( request )){
		send_error( request, FS_ERROR_NOT_CONNECTED );
		return;
	}

	connection.state = request->data[0];
	connection.index = request->data[1];

	message_t msg = { .type = FS_MSG_COMPLETED, };
	c4_msg_send( &msg, request->sender );
}

static void handle_set_node( ext2fs_t *fs, message_t *request ){
	if ( request->sender != connection.client ){
		send_error( request, FS_ERROR_NOT_CONNECTED );
		return;
	}

	connection.current_node = (fs_node_t){
		.inode = request->data[0],
		.type  = FILE_TYPE_UNKNOWN,
		.size  = 0,
	};

	connection.index = 0;

	ext2_get_inode( fs, &current_inode, request->data[0] );

	message_t msg = { .type = FS_MSG_COMPLETED, };
	c4_msg_send( &msg, request->sender );
}

static inline bool name_matches( ext2_dirent_t *dir, char *name, size_t len ){
	if ( dir->name_length != len ){
		return false;
	}

	for ( size_t i = 0; i < len; i++ ){
		if ( name[i] != dir->name[i] )
			return false;
	}

	return true;
}

static void handle_find_name( ext2fs_t *fs, message_t *request ){
	char name[FS_MAX_NAME_LEN];
	size_t namelen = request->data[0];

	// request validation
	if ( !is_connected( request )){
		send_error( request, FS_ERROR_NOT_CONNECTED );
		return;
	}

	if ( !ext2_is_directory( &current_inode )){
		send_error( request, FS_ERROR_NOT_DIRECTORY );
		return;
	}

	if ( !c4_ringbuf_can_read( connection.buffer, namelen )
	  || namelen >= FS_MAX_NAME_LEN )
	{
		send_error( request, FS_ERROR_BAD_REQUEST );
		return;
	}

	c4_ringbuf_read( connection.buffer, name, namelen );

	for ( size_t block = 0;
	      block * ext2_block_size(fs) < current_inode.lower_size;
	      block++ )
	{
		uint8_t *dirbuf = ext2_inode_read_block( fs, &current_inode, block );
		ext2_dirent_t *dirent = (void *)dirbuf;

		for ( unsigned i = 0; i < ext2_block_size(fs); ){
			if ( name_matches( dirent, name, namelen )){
				message_t msg = {
					.type = FS_MSG_COMPLETED,
					.data = {
						dirent->inode,
						translate_type(dirent->type),
						0,
					}
				};

				c4_msg_send( &msg, request->sender );
				return;
			}

			i += dirent->size;
			dirent = (void *)(dirbuf + i);
		}
	}

	send_error( request, FS_ERROR_NOT_FOUND );
}

static void handle_get_rootdir( ext2fs_t *fs, message_t *request ){
	ext2_inode_t inode;

	ext2_get_inode( fs, &inode, 2 );

	message_t msg = {
		.type = FS_MSG_COMPLETED,
		.data = {
			2,
			FILE_TYPE_DIRECTORY,
			inode.lower_size,
		}
	};

	c4_msg_send( &msg, request->sender );
}

static void handle_get_node_info( ext2fs_t *fs, message_t *request ){
	ext2_inode_t inode;

	if ( ! ext2_get_inode( fs, &inode, request->data[0] )){
		send_error( request, FS_ERROR_NOT_FOUND );
		return;
	}

	message_t msg = {
		.type = FS_MSG_COMPLETED,
		.data = {
			inode.lower_size,
			translate_type( ext2_inode_type( &inode )),
		},
	};

	c4_msg_send( &msg, request->sender );
}

static void handle_list_dir( ext2fs_t *fs, message_t *request ){
	message_t msg;
	unsigned sent = 0;

	if ( !is_connected( request )){
		c4_debug_printf( "--- ext2: got error\n" );
		send_error( request, FS_ERROR_NOT_CONNECTED );
		return;
	}

	if ( connection.index * ext2_block_size(fs) >= current_inode.lower_size ){
		goto done;
	}

	uint8_t *dirbuf = ext2_inode_read_block( fs, &current_inode, connection.index );
	ext2_dirent_t *dirent = (void *)dirbuf;

	for ( unsigned i = 0; i < ext2_block_size(fs); ){
		fs_dirent_t ent;

		copy_name( &ent, dirent );
		ent.type = translate_type( dirent->type );
		ent.name_len = dirent->name_length;
		ent.inode = dirent->inode;

		if ( c4_ringbuf_can_write( connection.buffer, sizeof(ent) )){
			c4_ringbuf_write( connection.buffer, &ent, sizeof(fs_dirent_t) );
			sent++;
		}

		i += dirent->size;
		dirent = (void *)(dirbuf + i);
	}

	connection.index++;

done:
	msg.type    = FS_MSG_COMPLETED;
	msg.data[0] = sent;
	c4_msg_send( &msg, request->sender );
}

static void handle_read_block( ext2fs_t *fs, message_t *request ){
	size_t sent = 0;

	if ( !is_connected( request )){
		send_error( request, FS_ERROR_NOT_CONNECTED );
		return;
	}

	if ( connection.index >= current_inode.lower_size ){
		goto done;
	}

	size_t blocksize = ext2_block_size(fs);
	size_t size      = current_inode.lower_size;
	size_t index     = connection.index;
	size_t diff      = size - index;
	size_t writesize = (diff > blocksize)? blocksize : diff;

	if ( !c4_ringbuf_can_write( connection.buffer, writesize )){
		send_error( request, FS_ERROR_QUEUE_FULL );
		return;
	}

	unsigned block = connection.index / blocksize;
	void *foo = ext2_inode_read_block( fs, &current_inode, block );
	size_t wrote = c4_ringbuf_write( connection.buffer, foo, writesize );

	sent += writesize;
	connection.index += writesize;

done: ;
	message_t msg = {
		.type = FS_MSG_COMPLETED,
		.data = { sent },
	};

	c4_msg_send( &msg, request->sender );
}
