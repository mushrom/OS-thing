#ifndef _kernel_ext2fs_h
#define _kernel_ext2fs_h
#include <fs.h>
#include <stdint.h>
#include <alloc.h>

// filesystem status in ext2_sprblk_t->fs_state
enum {
	EXT_FS_CLEAN = 1,
	EXT_FS_ERROR = 2
};

// What to do in case of an error in sprblk->error_detect
enum {
	EXT_ERROR_CONTINUE = 1,
	EXT_ERROR_READONLY = 2,
	EXT_ERROR_KPANIC   = 3
};

// Features required if set in esprblk->req_features
enum {
	REQ_COMPRESSION 	= 1,
	REQ_DIR_HAS_TYPE	= 2,
	REQ_FS_REPLAY_JOURN	= 4,
	REQ_FS_HAS_JOURN_DEV	= 8
};

// Can be read-only if these are set in esprblk->ro_features;
enum {
	RO_SPARSE	= 1,
	RO_FS_64B_SIZE	= 2,
	RO_DIR_IS_TREE	= 4
};

// Optional features in esprblk->op_features
enum {
	OPT_PREALLOC_BLOCKS	= 1,
	OPT_HAS_AFS_NODES	= 2,
	OPT_HAS_FS_JOURN	= 4,
	OPT_INODES_EXT_ATTR	= 8,
	OPT_FS_CAN_RESIZE	= 16,
	OPT_DIRS_HASH_INDEX	= 32
};

// Inode types
enum {
	INODE_TYPE_FIFO		= 0x1000,
	INODE_TYPE_CHAR_D	= 0x2000,
	INODE_TYPE_DIR		= 0x4000,
	INODE_TYPE_BLOCK_D	= 0x6000,
	INODE_TYPE_FILE		= 0x8000,
	INODE_TYPE_SYMLINK	= 0xa000,
	INODE_TYPE_SOCKET	= 0xc000
};

enum {	
	INODE_FLAG_SEC_DELETE		= 0x1,
	INODE_FLAG_KEEP_COPY		= 0x2,
	INODE_FLAG_FILE_COMPED		= 0x4,
	INODE_FLAG_SYNC_UPDATES 	= 0x8,
	INODE_FLAG_NO_CHANGE		= 0x10,
	INODE_FLAG_APPEND_ONLY		= 0x20,
	INODE_FLAG_NO_DUMP		= 0x40,
	INODE_FLAG_NO_TIME_UPDATE 	= 0x80,
	INODE_FLAG_HASHED_DIR		= 0x10000,
	INODE_FLAG_AFS_DIR		= 0x20000,
	INODE_FLAG_JOURN_DATA		= 0x40000
};

enum {
	DIR_TYPE_UNKNOWN	= 0,
	DIR_TYPE_FILE		= 1,
	DIR_TYPE_DIR		= 2,
	DIR_TYPE_CHAR_D		= 3,
	DIR_TYPE_BLOCK_D	= 4,
	DIR_TYPE_FIFO		= 5,
	DIR_TYPE_SOCKET		= 6,
	DIR_TYPE_SYMLINK	= 7
};
	

// ext2 superblock structure
typedef struct ext2_sprblk {
	uint32_t n_inodes;
	uint32_t n_blocks;
	uint32_t su_reserved;
	uint32_t free_blocks;
	uint32_t free_inodes;
	uint32_t sprblk_block;
	uint32_t block_size;
	uint32_t frag_size;
	uint32_t n_block_block_grp;
	uint32_t n_frag_block_grp;
	uint32_t n_inode_block_grp;
	uint32_t last_mount;
	uint32_t last_write;

	uint16_t n_mount_since_check;
	uint16_t n_mount_allowed_before_check;
	uint16_t ext2_sig;
	uint16_t fs_state;
	uint16_t error_detect;
	uint16_t min_version;

	uint32_t last_check;
	uint32_t check_int;
	uint32_t os_id;
	uint32_t maj_version;

	uint16_t user_id;
	uint16_t group_id;
} ext2_sprblk_t;

// ext2 extended superblock structure
typedef struct ext2_ex_sprblk {
	uint32_t first_free_inode;
	uint16_t inode_size;
	uint16_t self_block_grp;
	uint32_t op_features;
	uint32_t req_features;
	uint32_t ro_features;
	uint32_t fs_id[4];
	char	 vol_name[16];
	char	 last_path[64];
	uint32_t comp_used;
	uint8_t	 prealloc_bfiles;
	uint8_t  prealloc_bdirs;
	uint16_t allow_world_domination;
	uint32_t journ_id;
	uint32_t journ_inode;
	uint32_t journ_dev;
	uint32_t h_orphan_list;
} ext2_ex_sprblk_t;

// ext2 block descriptor
typedef struct ext2_block_desc {
	uint32_t block_usage_addr;
	uint32_t inode_usage_addr;
	uint32_t inode_table_addr;
	uint32_t free_blocks;
	uint32_t free_inodes;
} ext2_block_desc_t;

// ext2 inode structure
typedef struct ext2_inode {
	uint16_t type_perms;
	uint16_t uid;
	uint32_t low_size;
	uint32_t last_access;
	uint32_t creation_time;
	uint32_t last_mod;
	uint32_t deletion_time;
	uint16_t gid;
	uint16_t hard_links;
	uint32_t sectors_used;
	uint32_t flags;
	uint32_t os_flag_1;
	uint32_t d_ptr[12];
	uint32_t si_ptr;
	uint32_t di_ptr;
	uint32_t ti_ptr;
	uint32_t gen_num;
	uint32_t attr_block;
	uint32_t high_size;
	uint32_t block_addr_frag;
	uint32_t os_flag_2[3];
} ext2_inode_t;

typedef struct ext2_inode_os2 { 
	uint8_t frag_no;
	uint8_t frag_size;
} ext2_inode_os2;

typedef struct ext2_dirent {
	uint32_t inode;
	uint16_t size;
	uint8_t  low_name_len;
	uint8_t  type_ind;
	char     name;
} ext2_dirent_t;

file_node_t *ext2fs_create( char *path );
void ext2fs_mount( char *path, char *image );
void ext2fs_dump_info( char *path );

#endif


















































