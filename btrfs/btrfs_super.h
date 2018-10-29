/*
 * btrfs_super.h
 *
 * annotated btrfs structures for the super block and its embedded structures
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_SUPER_H
#define BTRFS_SUPER_H

#include "btrfs_chunk.h"

/* 32 bytes in various csum fields */
#define BTRFS_CSUM_SIZE 32

/*
 * this is a very generous portion of the super block, giving us
 * room to translate 14 chunks with 3 stripes each.
 */
#define BTRFS_SYSTEM_CHUNK_ARRAY_SIZE 2048
#define BTRFS_LABEL_SIZE 256

/*
 * just in case we somehow lose the roots and are not able to mount,
 * we store an array of the roots from previous transactions
 * in the super.
 */
#define BTRFS_NUM_BACKUP_ROOTS 4

FSSTRUCT() btrfs_root_backup {
	__le64 tree_root;
	__le64 tree_root_gen;

	__le64 chunk_root;
	__le64 chunk_root_gen;

	__le64 extent_root;
	__le64 extent_root_gen;

	__le64 fs_root;
	__le64 fs_root_gen;

	__le64 dev_root;
	__le64 dev_root_gen;

	__le64 csum_root;
	__le64 csum_root_gen;

	__le64 total_bytes;
	__le64 bytes_used;
	__le64 num_devices;
	/* future */
	__le64 unused_64[4];

	u8 tree_root_level;
	u8 chunk_root_level;
	u8 extent_root_level;
	u8 fs_root_level;
	u8 dev_root_level;
	u8 csum_root_level;
	/* future and to align */
	u8 unused_8[10];
} __attribute__ ((__packed__));

FSCONST(type=flag) btrfs_feature_compat_ro
{
        BTRFS_FEATURE_COMPAT_RO_FREE_SPACE_TREE	= (1ULL << 0),
};

FSCONST(type=flag) btrfs_feature_incompat
{
        BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF = (1ULL << 0),
        BTRFS_FEATURE_INCOMPAT_DEFAULT_SUBVOL = (1ULL << 1),
        BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS = (1ULL << 2),
        BTRFS_FEATURE_INCOMPAT_COMPRESS_LZO = (1ULL << 3),

/*
 * some patches floated around with a second compression method
 * lets save that incompat here for when they do get in
 * Note we don't actually support it, we're just reserving the
 * number
 */
        BTRFS_FEATURE_INCOMPAT_COMPRESS_LZOv2 = (1ULL << 4),

/*
 * older kernels tried to do bigger metadata blocks, but the
 * code was pretty buggy.  Lets not let them try anymore.
 */
        BTRFS_FEATURE_INCOMPAT_BIG_METADATA = (1ULL << 5),
        BTRFS_FEATURE_INCOMPAT_EXTENDED_IREF = (1ULL << 6),
        BTRFS_FEATURE_INCOMPAT_RAID56 = (1ULL << 7),
        BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA = (1ULL << 8),
        BTRFS_FEATURE_INCOMPAT_NO_HOLES = (1ULL << 9),
};

ADDRSPACE(name=raid, size=sb.total_bytes);

FSCONST(type=enum) btrfs_magic {
    BTRFS_MAGIC = 0x4D5F53665248425FULL,
};

/*
 * We don't want to overwrite 1M at the beginning of device, even though
 * there is our 1st superblock at 64k. Some possible reasons:
 *  - the first 64k blank is useful for some boot loader/manager
 *  - the first 1M could be scratched by buggy partitioner or somesuch
 */
#define BTRFS_BLOCK_RESERVED_1M_FOR_SUPER	((u64)1024 * 1024)

/* Mirror copies of the superblock are located at physical addresses 0x4000000, 
 * 0x4000000000  and 0x4000000000000, if these locations are valid.
 *
 * the super block basically lists the main trees of the FS
 * it currently lacks any block count etc etc
 */
FSSUPER(name=sb, location=0x10000) btrfs_super_block {
	u8 csum[BTRFS_CSUM_SIZE];
	
	/* the first 3 fields must match struct btrfs_header */
	FIELD(type=uuid)
	u8 fsid[BTRFS_FSID_SIZE];    /* FS specific uuid */
	
	__le64 bytenr; 				 /* this block number */
	__le64 flags;

	/* allowed to be different from the btrfs_header from here own down */
	FIELD(type=enum btrfs_magic)
	__le64 magic;
	__le64 generation;
	
    POINTER(repr=byte, type=struct btrfs_node, when=self.root_level > 0)
    POINTER(repr=byte, type=struct btrfs_leaf, when=self.root_level == 0)
	__le64 root;
	
	POINTER(repr=byte, type=struct btrfs_node, when=self.chunk_root_level > 0)
    POINTER(repr=byte, type=struct btrfs_leaf, when=self.chunk_root_level == 0)
	__le64 chunk_root;
	
	POINTER(repr=byte, type=struct btrfs_node, when=self.log_root_level > 0)
    POINTER(repr=byte, type=struct btrfs_leaf, when=self.log_root_level == 0)
	__le64 log_root;

	/* this will help find the new super based on the log root */
	__le64 log_root_transid;
	__le64 total_bytes;
	__le64 bytes_used;
	__le64 root_dir_objectid;
	__le64 num_devices;
	__le32 sectorsize;
	__le32 nodesize;
	__le32 leafsize;
	__le32 stripesize;
	__le32 sys_chunk_array_size;
	__le64 chunk_root_generation;
	__le64 compat_flags;
	
	FIELD(type=enum btrfs_feature_compat_ro)
	__le64 compat_ro_flags;
	
	FIELD(type=enum btrfs_feature_incompat)
	__le64 incompat_flags;
	
	__le16 csum_type;
	u8 root_level;
	u8 chunk_root_level;
	u8 log_root_level;
	struct btrfs_dev_item dev_item;

	char label[BTRFS_LABEL_SIZE];

	__le64 cache_generation;
	__le64 uuid_tree_generation;

	/* future expansion */
	__le64 reserved[30];
	u8 sys_chunk_array[BTRFS_SYSTEM_CHUNK_ARRAY_SIZE];
	struct btrfs_root_backup super_roots[BTRFS_NUM_BACKUP_ROOTS];
} __attribute__ ((__packed__));

#endif
