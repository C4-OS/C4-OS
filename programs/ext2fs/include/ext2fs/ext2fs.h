#ifndef _C4OS_EXT2FS_H
#define _C4OS_EXT2FS_H 1
#include <c4/message.h>
#include <c4rt/connman.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum {
	EXT2_SIGNATURE = 0xef53,
};

enum {
	EXT2_FS_CLEAN      = 1,
	EXT2_FS_HAS_ERRORS = 2,
};

enum {
	EXT2_ON_ERROR_IGNORE  = 1,
	EXT2_ON_ERROR_REMOUNT = 2,
	EXT2_ON_ERROR_PANIC   = 3,
};

enum {
	EXT2_CREATOR_LINUX,
	EXT2_CREATOR_HURD,
	EXT2_CREATOR_MASIX,
	EXT2_CREATOR_FREEBSD,
	EXT2_CREATOR_OTHER,
};

// Feature flags
enum {
	EXT2_OPTIONAL_PREALLOC      = 1 << 0,
	EXT2_OPTIONAL_AFS_INODES    = 1 << 1,
	EXT2_OPTIONAL_JOURNAL       = 1 << 2,
	EXT2_OPTIONAL_EXTENDED_ATTR = 1 << 3,
	EXT2_OPTIONAL_RESIZE        = 1 << 4,
	EXT2_OPTIONAL_HASH_INDEX    = 1 << 5,
};

enum {
	EXT2_REQUIRED_COMPRESSION    = 1 << 0,
	EXT2_REQUIRED_DIRECTORY_TYPE = 1 << 1,
	EXT2_REQUIRED_JOURNAL_REPLAY = 1 << 2,
	EXT2_REQUIRED_JOURNAL        = 1 << 3,
};

enum {
	EXT2_READONLY_SPARSE_SUPERBLOCKS  = 1 << 0,
	EXT2_READONLY_64BIT_SIZE          = 1 << 1,
	EXT2_READONLY_BINTREE_DIRECTORIES = 1 << 2,
};

// inode field flags
enum {
	EXT2_INODE_TYPE_FIFO         = 0x1,
	EXT2_INODE_TYPE_CHAR_DEVICE  = 0x2,
	EXT2_INODE_TYPE_DIRECTORY    = 0x4,
	EXT2_INODE_TYPE_BLOCK_DEVICE = 0x6,
	EXT2_INODE_TYPE_REGULAR_FILE = 0x8,
	EXT2_INODE_TYPE_SYMLINK      = 0xa,
	EXT2_INODE_TYPE_UNIX_SOCKET  = 0xc,
};

enum {
	EXT2_INODE_FLAG_SECURE_DELETE    = 1 << 0,
	EXT2_INODE_FLAG_KEEP_ON_DELETE   = 1 << 1,
	EXT2_INODE_FLAG_FILE_COMPRESSION = 1 << 2,
	EXT2_INODE_FLAG_SYNC_UPDATES     = 1 << 3,
	EXT2_INODE_FLAG_IMMUTABLE        = 1 << 4,
	EXT2_INODE_FLAG_APPEND_ONLY      = 1 << 5,
	EXT2_INODE_FLAG_NO_DUMP          = 1 << 6,
	EXT2_INODE_FLAG_NO_ATIME         = 1 << 7,
	EXT2_INODE_FLAG_HASH_INDEXED     = 1 << 16,
	EXT2_INODE_FLAG_AFS_DIRECTORY    = 1 << 17,
	EXT2_INODE_FLAG_JOURNAL_DATA     = 1 << 18,
};

// directory type flags
enum {
	EXT2_DIRENT_TYPE_UKNOWN,
	EXT2_DIRENT_TYPE_FILE,
	EXT2_DIRENT_TYPE_DIR,
	EXT2_DIRENT_TYPE_CHARDEV,
	EXT2_DIRENT_TYPE_BLOCKDEV,
	EXT2_DIRENT_TYPE_FIFO,
	EXT2_DIRENT_TYPE_SOCKET,
	EXT2_DIRENT_TYPE_SYMLINK,
};

typedef struct ext2_base_superblock {
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t superuser_reserved;
	uint32_t unalloced_blocks;
	uint32_t unalloced_inodes;
	uint32_t superblock_block;
	uint32_t block_size;
	uint32_t fragment_size;
	uint32_t blocks_per_group;
	uint32_t fragments_per_group;
	uint32_t inodes_per_group;
	uint32_t last_mount_time;
	uint32_t last_write_time;
	uint16_t mounts_since_fsck;
	uint16_t max_mounts_before_fsck;
	uint16_t signature;
	uint16_t fs_state;
	uint16_t error_hint;
	uint16_t minor_version;
	uint32_t last_fsck_time;
	uint32_t fsck_interval;
	uint32_t os_id;
	uint32_t major_version;
	uint16_t uid_for_reserved;
	uint16_t gid_for_reserved;
} ext2_base_superblock_t;

typedef struct ext2_ext_superblock {
	uint32_t first_free_inode;
	uint16_t inode_size;
	uint16_t superblock_group;
	uint32_t optional_features;
	uint32_t required_features;
	uint32_t read_only_features;
	uint8_t  fs_id[16];
	uint8_t  volume_name[16];
	uint8_t  volume_path[64];
	uint32_t compression;
	uint8_t  file_blocks_prealloc;
	uint8_t  directory_blocks_prealloc;
	uint16_t unused;
	uint8_t  journal_id[16];
	uint32_t journal_inode;
	uint32_t journal_device;
	uint32_t orphan_list;
} ext2_ext_superblock_t;

typedef struct ext2_superblock {
	ext2_base_superblock_t base;
	ext2_ext_superblock_t  ext;
} ext2_superblock_t;

typedef struct ext2_block_group_desc {
	uint32_t block_map;
	uint32_t inode_map;
	uint32_t inode_table;
	uint16_t unalloced_blocks;
	uint16_t unalloced_inodes;
	uint16_t total_directories;
	uint8_t  unused[14];
} ext2_block_group_desc_t;

typedef struct ext2_inode {
	uint16_t modes;
	uint16_t uid;
	uint32_t lower_size;
	uint32_t last_access_time;
	uint32_t creation_time;
	uint32_t last_mod_time;
	uint32_t deletion_time;
	uint16_t gid;
	uint16_t total_hardlinks;
	uint32_t total_disksectors;
	uint32_t flags;
	uint32_t os_value_1;
	uint32_t direct_ptr[12];
	uint32_t single_indirect_ptr;
	uint32_t double_indirect_ptr;
	uint32_t triple_indirect_ptr;
	uint32_t generation;
	uint32_t ext_attributes;
	uint32_t upper_size;
	uint32_t fragment_block;
	uint32_t os_value_2;
} ext2_inode_t;

typedef struct ext2_dirent {
	uint32_t inode;
	uint16_t size;

	union {
		uint16_t name_length_long;
		struct {
			uint8_t name_length;
			uint8_t type;
		};
	};

	char name[];
} ext2_dirent_t;

typedef struct ext2fs {
	ext2_superblock_t superblock;
	size_t block_device_start;

	unsigned block_device;
	c4rt_conn_server_t server;
} ext2fs_t;

//void ext2_handle_request( ext2fs_t *fs, message_t *request );
void ext2_server(ext2fs_t *fs);
ext2_superblock_t *ext2_get_superblock( unsigned device, unsigned drive );
void *ext2_read_block( ext2fs_t *fs, unsigned block );
ext2_block_group_desc_t *ext2_get_block_descs( ext2fs_t *ext2 );
ext2_inode_t *ext2_get_inode( ext2fs_t *ext2,
							  ext2_inode_t *buffer,
                              unsigned inode );
void *ext2_inode_read_block( ext2fs_t *fs, ext2_inode_t *inode, unsigned block );

static inline bool is_valid_ext2fs( ext2fs_t *ext2 ){
	return ext2->superblock.base.signature == EXT2_SIGNATURE;
}

static inline unsigned ext2_block_size( ext2fs_t *ext2 ){
	return 1 << (ext2->superblock.base.block_size + 10);
}

static inline unsigned ext2_inode_size( ext2fs_t *ext2 ){
	return ext2->superblock.ext.inode_size;
}

static inline unsigned ext2_block_to_sector( ext2fs_t *fs, unsigned block ){
	return block * ext2_block_size(fs) / 512 + fs->block_device_start;
}

static inline unsigned ext2_block_size_to_sectors( ext2fs_t *ext2 ){
	return ext2_block_size(ext2) / 512;
}

static inline unsigned ext2_total_block_descs( ext2fs_t *ext2 ){
	return ext2->superblock.base.total_blocks /
	       ext2->superblock.base.blocks_per_group;
}

static inline unsigned ext2_inodes_per_group( ext2fs_t *ext2 ){
	return ext2->superblock.base.inodes_per_group;
}

static inline unsigned ext2_inodes_per_block( ext2fs_t *ext2 ){
	return ext2_block_size(ext2) / ext2_inode_size(ext2);
}

static inline unsigned ext2_max_inode( ext2fs_t *ext2 ){
	return ext2->superblock.base.total_inodes;
}

static inline unsigned ext2_max_block( ext2fs_t *ext2 ){
	return ext2->superblock.base.total_blocks;
}

static inline unsigned ext2_inode_type( ext2_inode_t *inode ){
	return (inode->modes & 0xf000) >> 12;
}

static inline unsigned ext2_block_group( ext2fs_t *ext2, unsigned inode ){
	return (inode - 1) / ext2_inodes_per_group( ext2 );
}

static inline unsigned ext2_group_index( ext2fs_t *ext2, unsigned inode ){
	return (inode - 1) % ext2_inodes_per_group( ext2 );
}

static inline uint32_t ext2_pointers_per_block(ext2fs_t *fs){
	return ext2_block_size(fs) / sizeof(uint32_t);
}

static inline bool ext2_is_directory( ext2_inode_t *inode ){
	return ext2_inode_type(inode) == EXT2_INODE_TYPE_DIRECTORY;
}

#endif
