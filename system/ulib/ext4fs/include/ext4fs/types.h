// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <bitmap/raw-bitmap.h>
#include <bitmap/storage.h>
#include <fbl/limits.h>
#include <fbl/macros.h>
#include <fbl/type_support.h>

#include <zircon/types.h>

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>



namespace ext4fs {

constexpr uint32_t kExt4fsSuperblockMagic = 0xEF53;
constexpr uint16_t kExt4fsSuperblockSize  = 1024;
constexpr uint16_t kExt4fsSuperblockOffset  = 1024;
constexpr uint8_t  kExt4fsSuperblockOsLinux = 0;
constexpr uint8_t  kExt4fsSuperblockOsHurd  = 1;

/*
 * Misc. filesystem flags
 */
constexpr uint16_t kExt4fsSuperblockFlagSignedHash = 0x0001;   /* Signed dirhash in use */
constexpr uint16_t kExt4fsSuperblockFlagUnSignedHash = 0x0002; /* Unsigned dirhash in use */
constexpr uint16_t kExt4fsSuperblockFlagTestFilesys = 0x0004;  /* to test development code */

/*
 * Filesystem states
 */
#define EXT4_SUPERBLOCK_STATE_VALID_FS   0x0001  /* Unmounted cleanly */
#define EXT4_SUPERBLOCK_STATE_ERROR_FS   0x0002  /* Errors detected */
#define EXT4_SUPERBLOCK_STATE_ORPHAN_FS  0x0004  /* Orphans being recovered */

/*
 * Behaviour when errors detected
 */
#define EXT4_SUPERBLOCK_ERRORS_CONTINUE  1  /* Continue execution */
#define EXT4_SUPERBLOCK_ERRORS_RO        2  /* Remount fs read-only */
#define EXT4_SUPERBLOCK_ERRORS_PANIC     3  /* Panic */
#define EXT4_SUPERBLOCK_ERRORS_DEFAULT   EXT4_ERRORS_CONTINUE

/*
 * Compatible features
 */
#define EXT4_FEATURE_COMPAT_DIR_PREALLOC   0x0001
#define EXT4_FEATURE_COMPAT_IMAGIC_INODES  0x0002
#define EXT4_FEATURE_COMPAT_HAS_JOURNAL    0x0004
#define EXT4_FEATURE_COMPAT_EXT_ATTR       0x0008
#define EXT4_FEATURE_COMPAT_RESIZE_INODE   0x0010
#define EXT4_FEATURE_COMPAT_DIR_INDEX      0x0020
#define EXT4_FEATURE_COMPAT_SPARSE_SUPER2  0x0200

/*
 * Read-only compatible features
 */
#define EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER  0x0001
#define EXT4_FEATURE_RO_COMPAT_LARGE_FILE    0x0002
#define EXT4_FEATURE_RO_COMPAT_BTREE_DIR     0x0004
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE     0x0008
#define EXT4_FEATURE_RO_COMPAT_GDT_CSUM      0x0010
#define EXT4_FEATURE_RO_COMPAT_DIR_NLINK     0x0020
#define EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE   0x0040

/*
 * Incompatible features
 */
#define EXT4_FEATURE_INCOMPAT_COMPRESSION  0x0001
#define EXT4_FEATURE_INCOMPAT_FILETYPE     0x0002
#define EXT4_FEATURE_INCOMPAT_RECOVER      0x0004  /* Needs recovery */
#define EXT4_FEATURE_INCOMPAT_JOURNAL_DEV  0x0008  /* Journal device */
#define EXT4_FEATURE_INCOMPAT_META_BG      0x0010
#define EXT4_FEATURE_INCOMPAT_EXTENTS      0x0040  /* extents support */
#define EXT4_FEATURE_INCOMPAT_64BIT        0x0080
#define EXT4_FEATURE_INCOMPAT_MMP          0x0100
#define EXT4_FEATURE_INCOMPAT_FLEX_BG      0x0200
#define EXT4_FEATURE_INCOMPAT_EA_INODE     0x0400  /* EA in inode */
#define EXT4_FEATURE_INCOMPAT_DIRDATA      0x1000  /* data in dirent */

#define EXT4_FEATURE_COMPAT_SUPP  (EXT4_FEATURE_COMPAT_DIR_INDEX)

#define EXT4_FEATURE_INCOMPAT_SUPP \
	(EXT4_FEATURE_INCOMPAT_FILETYPE | \
	EXT4_FEATURE_INCOMPAT_EXTENTS | \
	EXT4_FEATURE_INCOMPAT_64BIT | \
	EXT4_FEATURE_INCOMPAT_FLEX_BG)

#define EXT4_FEATURE_RO_COMPAT_SUPP \
	(EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER | \
	EXT4_FEATURE_RO_COMPAT_DIR_NLINK | \
	EXT4_FEATURE_RO_COMPAT_HUGE_FILE | \
	EXT4_FEATURE_RO_COMPAT_LARGE_FILE | \
	EXT4_FEATURE_RO_COMPAT_GDT_CSUM | \
	EXT4_FEATURE_RO_COMPAT_EXTRA_ISIZE)

constexpr uint32_t kExt4fsMinBlockSize = 1024U;   /* 1 KiB */
constexpr uint32_t kExt4fsMaxBlockSize = 65536U;  /* 64 KiB */
#define EXT4_REV0_INODE_SIZE  128

/* Added by Manu */
constexpr uint8_t kExt4fsMaxNameSize       = 255U;
constexpr uint8_t kExt4fsInodeRootIndex    = 2U;

typedef uint32_t blk_t;

typedef struct {
    uint64_t magic0;
    uint64_t magic1;
    uint32_t version;
    uint32_t flags;
    uint32_t block_size;    // 8K typical
    uint32_t inode_size;    // 256
    uint32_t block_count;   // total number of data blocks
    uint32_t inode_count;   // total number of inodes
    uint32_t alloc_block_count; // total number of allocated data blocks
    uint32_t alloc_inode_count; // total number of allocated inodes
    blk_t ibm_block;     // first blockno of inode allocation bitmap
    blk_t abm_block;     // first blockno of block allocation bitmap
    blk_t ino_block;     // first blockno of inode table
    blk_t dat_block;     // first blockno available for file data
    // The following flags are only valid with (flags & kMinfsFlagFVM):
    uint64_t slice_size;    // Underlying slice size
    uint64_t vslice_count;  // Number of allocated underlying slices
    uint32_t ibm_slices;    // Slices allocated to inode bitmap
    uint32_t abm_slices;    // Slices allocated to block bitmap
    uint32_t ino_slices;    // Slices allocated to inode table
    uint32_t dat_slices;    // Slices allocated to file data section
} ext4fs_info_t;

/*
 * Structure of the super block
 */
typedef struct ext4_superblock {
	uint32_t inodes_count;              /* I-nodes count */
	uint32_t blocks_count_lo;           /* Blocks count */
	uint32_t reserved_blocks_count_lo;  /* Reserved blocks count */
	uint32_t free_blocks_count_lo;      /* Free blocks count */
	uint32_t free_inodes_count;         /* Free inodes count */
	uint32_t first_data_block;          /* First Data Block */
	uint32_t log_block_size;            /* Block size */
	uint32_t log_frag_size;             /* Obsoleted fragment size */
	uint32_t blocks_per_group;          /* Number of blocks per group */
	uint32_t frags_per_group;           /* Obsoleted fragments per group */
	uint32_t inodes_per_group;          /* Number of inodes per group */
	uint32_t mount_time;                /* Mount time */
	uint32_t write_time;                /* Write time */
	uint16_t mount_count;               /* Mount count */
	uint16_t max_mount_count;           /* Maximal mount count */
	uint16_t magic;                     /* Magic signature */
	uint16_t state;                     /* File system state */
	uint16_t errors;                    /* Behaviour when detecting errors */
	uint16_t minor_rev_level;           /* Minor revision level */
	uint32_t last_check_time;           /* Time of last check */
	uint32_t check_interval;            /* Maximum time between checks */
	uint32_t creator_os;                /* Creator OS */
	uint32_t rev_level;                 /* Revision level */
	uint16_t def_resuid;                /* Default uid for reserved blocks */
	uint16_t def_resgid;                /* Default gid for reserved blocks */

	/* Fields for EXT4_DYNAMIC_REV superblocks only. */
	uint32_t first_inode;             /* First non-reserved inode */
	uint16_t inode_size;              /* Size of inode structure */
	uint16_t block_group_index;       /* Block group index of this superblock */
	uint32_t features_compatible;     /* Compatible feature set */
	uint32_t features_incompatible;   /* Incompatible feature set */
	uint32_t features_read_only;      /* Readonly-compatible feature set */
	uint8_t uuid[16];                 /* 128-bit uuid for volume */
	char volume_name[16];             /* Volume name */
	char last_mounted[64];            /* Directory where last mounted */
	uint32_t algorithm_usage_bitmap;  /* For compression */

	/*
	 * Performance hints. Directory preallocation should only
	 * happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
	 */
	uint8_t prealloc_blocks;        /* Number of blocks to try to preallocate */
	uint8_t prealloc_dir_blocks;    /* Number to preallocate for dirs */
	uint16_t reserved_gdt_blocks;   /* Per group desc for online growth */

	/*
	 * Journaling support valid if EXT4_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	uint8_t journal_uuid[16];       /* UUID of journal superblock */
	uint32_t journal_inode_number;  /* Inode number of journal file */
	uint32_t journal_dev;           /* Device number of journal file */
	uint32_t last_orphan;           /* Head of list of inodes to delete */
	uint32_t hash_seed[4];          /* HTREE hash seed */
	uint8_t default_hash_version;   /* Default hash version to use */
	uint8_t journal_backup_type;
	uint16_t desc_size;             /* Size of group descriptor */
	uint32_t default_mount_opts;    /* Default mount options */
	uint32_t first_meta_bg;         /* First metablock block group */
	uint32_t mkfs_time;             /* When the filesystem was created */
	uint32_t journal_blocks[17];    /* Backup of the journal inode */

	/* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
	uint32_t blocks_count_hi;           /* Blocks count */
	uint32_t reserved_blocks_count_hi;  /* Reserved blocks count */
	uint32_t free_blocks_count_hi;      /* Free blocks count */
	uint16_t min_extra_isize;           /* All inodes have at least # bytes */
	uint16_t want_extra_isize;          /* New inodes should reserve # bytes */
	uint32_t flags;                     /* Miscellaneous flags */
	uint16_t raid_stride;               /* RAID stride */
	uint16_t mmp_interval;              /* # seconds to wait in MMP checking */
	uint64_t mmp_block;                 /* Block for multi-mount protection */
	uint32_t raid_stripe_width;         /* Blocks on all data disks (N * stride) */
	uint8_t log_groups_per_flex;        /* FLEX_BG group size */
	uint8_t reserved_char_pad;
	uint16_t reserved_pad;
	uint64_t kbytes_written;            /* Number of lifetime kilobytes written */
	uint32_t snapshot_inum;             /* I-node number of active snapshot */
	uint32_t snapshot_id;               /* Sequential ID of active snapshot */
	uint64_t snapshot_r_blocks_count;   /* Reserved blocks for active snapshot's future use */
	uint32_t snapshot_list;             /* I-node number of the head of the on-disk snapshot list */
	uint32_t error_count;               /* Number of file system errors */
	uint32_t first_error_time;          /* First time an error happened */
	uint32_t first_error_ino;           /* I-node involved in first error */
	uint64_t first_error_block;         /* Block involved of first error */
	uint8_t first_error_func[32];       /* Function where the error happened */
	uint32_t first_error_line;          /* Line number where error happened */
	uint32_t last_error_time;           /* Most recent time of an error */
	uint32_t last_error_ino;            /* I-node involved in last error */
	uint32_t last_error_line;           /* Line number where error happened */
	uint64_t last_error_block;          /* Block involved of last error */
	uint8_t last_error_func[32];        /* Function where the error happened */
	uint8_t mount_opts[64];             /* String containing the mount options */
	uint32_t usr_quota_inum;            /* Inode number of user quota file */
	uint32_t grp_quota_inum;            /* Inode number of group quota file */
	uint32_t overhead_blocks;           /* Overhead blocks/clusters */
	uint32_t backup_bgs[2];             /* Block groups containing superblock backups (if SPARSE_SUPER2) */
	uint32_t encrypt_algos;             /* Encrypt algorithm in use */
	uint32_t padding[105];              /* Padding to the end of the block */
} __attribute__((packed)) ext4_superblock_t;




}
