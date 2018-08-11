#include <ext2fs/ext2fs.h>
#include <c4rt/interface/block.h>
#include <nameserver/nameserver.h>
#include <c4rt/c4rt.h>
#include <c4rt/mem.h>
#include <c4/paging.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef NDEBUG
#define DEBUGF( FORMAT, ... ) \
	c4_debug_printf( "--- ext2fs: " FORMAT, __VA_ARGS__ )
#endif

// XXX: This is a global buffer used for communicating with the disk driver.
//      higher-level functions use it in a self-contained way so that there
//      should be no worry about accidentally overwriting previously retrieved
//      data, but please be mindful of it if you happen to be modifying
//      the code.
//static uint8_t disk_buffer[PAGE_SIZE] __attribute__((aligned(PAGE_SIZE)));
static c4_mem_object_t block_buffer;

ext2_superblock_t *ext2_get_superblock( unsigned device, unsigned drive ){
	void *disk_buffer = (void *)block_buffer.vaddr;
	bool error = block_read( device, disk_buffer, drive, 2050, 2 );

	return error? NULL : (void *)disk_buffer;
}

void *ext2_read_block( ext2fs_t *fs, unsigned block ){
	void *disk_buffer = (void *)block_buffer.vaddr;

	if ( block > ext2_max_block(fs) ){
		DEBUGF( "have request for block not in filesystem, %u\n", block );
		return NULL;
	}

	unsigned sector = ext2_block_to_sector( fs, block );
	unsigned size   = ext2_block_size_to_sectors( fs );
	bool error = block_read( fs->block_device, disk_buffer, 0, sector, size );

	return error? NULL : (void *)disk_buffer;
}

ext2_block_group_desc_t *ext2_get_block_descs( ext2fs_t *ext2 ){
	unsigned start = (ext2_block_size(ext2) == 1024)? 2 : 1;

	ext2_block_group_desc_t *desctab = ext2_read_block( ext2, start );

	return desctab;
}

ext2_inode_t *ext2_get_inode( ext2fs_t *ext2,
							  ext2_inode_t *buffer,
                              unsigned inode )
{
	ext2_block_group_desc_t *descs = ext2_get_block_descs( ext2 );

	if ( !buffer || !descs ) return NULL;

	if ( inode > ext2_max_inode(ext2) ){
		DEBUGF( "have request for inode not in filesystem, %u\n", inode );
		return NULL;
	}

	unsigned group = ext2_block_group( ext2, inode );
	unsigned table = descs[group].inode_table;
	unsigned group_index = ext2_group_index( ext2, inode );

	unsigned offset = group_index / ext2_inodes_per_block( ext2 );
	unsigned index  = group_index % ext2_inodes_per_block( ext2 );

	DEBUGF( "looking for inode %u at %u[%u] (offset %u[%u]) @ %u\n",
		inode, table, group_index,
		offset, index,
		ext2_block_to_sector( ext2, table ));

	uint8_t *inode_table = ext2_read_block( ext2, table + offset );

	if ( !inode_table ){
		DEBUGF( "inode table not in filesystem, %u\n", inode );
		return NULL;
	}

	ext2_inode_t *inode_ptr = (void *)(inode_table + index * ext2_inode_size(ext2));

	*buffer = *inode_ptr;

	return buffer;
}

static unsigned iexp( unsigned i, unsigned n ){
	unsigned result = 1;

	for ( unsigned k = 0; k < n; k++ ){
		result *= i;
	}

	return result;
}

void *ext2_inode_read_block( ext2fs_t *fs, ext2_inode_t *inode, unsigned block )
{
	if ( block < 12 ){
		return ext2_read_block( fs, inode->direct_ptr[block] );
	}

	unsigned ptr_per_blk = ext2_block_size(fs) / sizeof(uint32_t);

	if ( block < ptr_per_blk + 12 ){
		unsigned index = block - 12;
		uint32_t *ents = ext2_read_block( fs, inode->single_indirect_ptr );
		return ext2_read_block( fs, ents[index] );

	} else if ( block < iexp( ptr_per_blk, 2 ) + 12 ){
		DEBUGF( "doing doubly-indirect block read\n", 0 );

	} else if ( block < iexp( ptr_per_blk, 3 ) + 12 ){
		DEBUGF( "doing triply-indirect block read\n", 0 );
	}

	return NULL;
}

//void _start( uintptr_t nameserver ){
int main(int argc, char *argv[]) {
	int disk = 0;
	int serv_port = c4_msg_create_sync();
	int nameserver = getnameserv();

	C4_ASSERT(c4_memobj_alloc(&block_buffer, PAGE_SIZE, PAGE_READ|PAGE_WRITE));
	DEBUGF("have block buffer at %p\n", block_buffer.vaddr);

	// TODO: find another way to handle this, this limits the number of
	//       ext2 filesystems to one per nameserver
	nameserver_bind( nameserver, "/dev/ext2fs", serv_port );
	
	while ( !disk ){
		disk = nameserver_lookup( nameserver, "/dev/ata" );
	}

	ext2_superblock_t *temp = ext2_get_superblock( disk, 0 );
	DEBUGF( "hello, world! thread %u\n", c4_get_id());

	if ( !temp ){
		DEBUGF( "error while reading block device %u...\n", disk );
		c4_exit();
	}

	ext2fs_t ext2 = {
		.superblock = *temp,
		.block_device_start = 2048,
		.block_device = disk,
	};

	if ( !is_valid_ext2fs( &ext2 )){
		DEBUGF( "invalid signature, %x\n", ext2.superblock.base.signature );
		c4_exit();
	}

	c4_debug_printf( "--- ext2fs: \n" );
	DEBUGF( "version: %u.%u\n",
		ext2.superblock.base.major_version,
		ext2.superblock.base.minor_version );
	DEBUGF( "have superblock, sig: %x\n", ext2.superblock.base.signature );
	DEBUGF( "total blocks: %u\n", ext2.superblock.base.total_blocks );
	DEBUGF( "block size: %u bytes\n", ext2_block_size( &ext2 ));
	DEBUGF( "blocks per group: %u\n", ext2.superblock.base.blocks_per_group );
	DEBUGF( "inodes per group: %u\n", ext2.superblock.base.inodes_per_group );
	DEBUGF( "block descriptors: %u\n", ext2_total_block_descs( &ext2 ));
	DEBUGF( "inode size: %u\n", ext2.superblock.ext.inode_size );

	if ( ext2.superblock.base.major_version >= 1 ){
		DEBUGF( "required features: %x\n",
			ext2.superblock.ext.required_features );
		DEBUGF( "readonly features: %x\n",
			ext2.superblock.ext.read_only_features );
	}
	DEBUGF( "got here, %u\n", 0 );

	ext2_block_group_desc_t *descs = ext2_get_block_descs( &ext2 );
	unsigned n = ext2_total_block_descs( &ext2 );
	DEBUGF( "got here, %u\n", n );

	for ( unsigned i = 0; i < n; i++ ){
		DEBUGF( "block descriptor %u:\n", i );
		DEBUGF( "         block map: %u\n", descs[i].block_map );
		DEBUGF( "         inode map: %u\n", descs[i].inode_map );
		DEBUGF( "       inode table: %u\n", descs[i].inode_table );
		DEBUGF( "  unalloced blocks: %u\n", descs[i].unalloced_blocks );
		DEBUGF( "  unalloced inodes: %u\n", descs[i].unalloced_inodes );
		DEBUGF( "        total dirs: %u\n", descs[i].total_directories );
	}

	DEBUGF( "rootdir block group: %u\n", ext2_block_group( &ext2, 2 ));
	DEBUGF( "rootdir group index: %u\n", ext2_group_index( &ext2, 2 ));

	ext2_inode_t inode;
	unsigned in = 2;

	ext2_get_inode( &ext2, &inode, in );

	DEBUGF( "inode %u:\n", in );
	DEBUGF( "  modes: %x\n", inode.modes );
	DEBUGF( "   size:  %u\n", inode.lower_size );
	DEBUGF( "    uid:  %u\n", inode.uid );

	for ( unsigned i = 0; i < 12; i++ ){
		DEBUGF( "  direct_ptr[%u]: %u\n", i, inode.direct_ptr[i] );
	}

	C4_ASSERT( ext2_inode_type(&inode) == EXT2_INODE_TYPE_DIRECTORY );

	uint8_t *dirents = ext2_inode_read_block( &ext2, &inode, 0 );
	ext2_dirent_t *foo = (void *)dirents;

	for ( unsigned i = 0; i < ext2_block_size(&ext2); ){
		DEBUGF( "> dirent name: %s", "" );
		for ( unsigned j = 0; j < foo->name_length; j++ ){
			c4_debug_putchar( foo->name[j] );
		}

		c4_debug_putchar( '\n' );

		DEBUGF( "  dirent inode: %u\n", foo->inode );
		DEBUGF( "  dirent size:  %u\n", foo->size );
		DEBUGF( "  dirent name length: %u\n", foo->name_length );
		DEBUGF( "  dirent type: %u\n", foo->type );

		C4_ASSERT( foo->size != 0 );
		if ( foo->size == 0 ) break;

		i += foo->size;
		foo = (void *)(dirents + i);
	}

	while ( true ){
		message_t msg;

		//c4_msg_recieve( &msg, 0 );
		c4_msg_recieve( &msg, serv_port );
		ext2_handle_request( &ext2, &msg );
	}

	return 0;
	//c4_exit();
}
