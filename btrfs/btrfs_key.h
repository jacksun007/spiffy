/*
 * btrfs_key.h
 *
 * annotated btrfs key structures and its key types
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_KEY_H
#define BTRFS_KEY_H

#include "btrfs_def.h"

#if BTRFS_UUID_SIZE != 16
#error "UUID items require BTRFS_UUID_SIZE == 16!"
#endif

FSCONST(type=enum) btrfs_key_type 
{
/*
 * (jsun): this was added to handle empty btrfs_disk_key
 */
        BTRFS_INVALID_KEY = 0,

/*
 * inode items have the data typically returned from stat and store other
 * info about object characteristics.  There is one for every file and dir in
 * the FS
 */
        BTRFS_INODE_ITEM_KEY = 1,
        BTRFS_INODE_REF_KEY = 12,
        BTRFS_INODE_EXTREF_KEY = 13,
        BTRFS_XATTR_ITEM_KEY = 24,
        BTRFS_ORPHAN_ITEM_KEY = 48,

        BTRFS_DIR_LOG_ITEM_KEY = 60,
        BTRFS_DIR_LOG_INDEX_KEY = 72,
/*
 * dir items are the name -> inode pointers in a directory.  There is one
 * for every name in a directory.
 */
        BTRFS_DIR_ITEM_KEY = 84,
        BTRFS_DIR_INDEX_KEY = 96,

/*
 * extent data is for file data
 */
        BTRFS_EXTENT_DATA_KEY = 108,

/*
 * csum items have the checksums for data in the extents
 */
        BTRFS_CSUM_ITEM_KEY = 120,
/*
 * extent csums are stored in a separate tree and hold csums for
 * an entire extent on disk.
 */
        BTRFS_EXTENT_CSUM_KEY = 128,

/*
 * root items point to tree roots.  There are typically in the root
 * tree used by the super block to find all the other trees
 */
        BTRFS_ROOT_ITEM_KEY = 132,

/*
 * root backrefs tie subvols and snapshots to the directory entries that
 * reference them
 */
        BTRFS_ROOT_BACKREF_KEY = 144,

/*
 * root refs make a fast index for listing all of the snapshots and
 * subvolumes referenced by a given root.  They point directly to the
 * directory item in the root that references the subvol
 */
        BTRFS_ROOT_REF_KEY = 156,

/*
 * extent items are in the extent map tree.  These record which blocks
 * are used, and how many references there are to each block
 */
        BTRFS_EXTENT_ITEM_KEY = 168,

/*
 * The same as the BTRFS_EXTENT_ITEM_KEY, except it's metadata we already know
 * the length, so we save the level in key->offset instead of the length.
 */
        BTRFS_METADATA_ITEM_KEY = 169,

        BTRFS_TREE_BLOCK_REF_KEY = 176,

        BTRFS_EXTENT_DATA_REF_KEY = 178,

/* old style extent backrefs */
        BTRFS_EXTENT_REF_V0_KEY = 180,

        BTRFS_SHARED_BLOCK_REF_KEY = 182,

        BTRFS_SHARED_DATA_REF_KEY = 184,


/*
 * block groups give us hints into the extent allocation trees.  Which
 * blocks are free etc etc
 */
        BTRFS_BLOCK_GROUP_ITEM_KEY = 192,

/*
 * Every block group is represented in the free space tree by a free space info
 * item, which stores some accounting information. It is keyed on
 * (block_group_start, FREE_SPACE_INFO, block_group_length).
 */
        BTRFS_FREE_SPACE_INFO_KEY = 198,

/*
 * A free space extent tracks an extent of space that is free in a block group.
 * It is keyed on (start, FREE_SPACE_EXTENT, length).
 */
        BTRFS_FREE_SPACE_EXTENT_KEY = 199,

/*
 * When a block group becomes very fragmented, we convert it to use bitmaps
 * instead of extents. A free space bitmap is keyed on
 * (start, FREE_SPACE_BITMAP, length); the corresponding item is a bitmap with
 * (length / sectorsize) bits.
 */
        BTRFS_FREE_SPACE_BITMAP_KEY = 200,

        BTRFS_DEV_EXTENT_KEY = 204,
        BTRFS_DEV_ITEM_KEY = 216,
        BTRFS_CHUNK_ITEM_KEY = 228,

        BTRFS_BALANCE_ITEM_KEY = 248,

/*
 * quota groups
 */
        BTRFS_QGROUP_STATUS_KEY = 240,
        BTRFS_QGROUP_INFO_KEY = 242,
        BTRFS_QGROUP_LIMIT_KEY = 244,
        BTRFS_QGROUP_RELATION_KEY = 246,

/*
 * Persistently stores the io stats in the device tree.
 * One key for all stats, (0, BTRFS_DEV_STATS_KEY, devid).
 */
        BTRFS_DEV_STATS_KEY = 249,

/*
 * Persistently stores the device replace state in the device tree.
 * The key is built like this: (0, BTRFS_DEV_REPLACE_KEY, 0).
 */
        BTRFS_DEV_REPLACE_KEY = 250,

/*
 * Stores items that allow to quickly map UUIDs to something else.
 * These items are part of the filesystem UUID tree.
 * The key is built like this:
 * (UUID_upper_64_bits, BTRFS_UUID_KEY*, UUID_lower_64_bits).
 */
        BTRFS_UUID_KEY_SUBVOL = 251,	/* for UUIDs assigned to subvols */
        BTRFS_UUID_KEY_RECEIVED_SUBVOL = 252,	/* for UUIDs assigned to
						                         * received subvols */

/*
 * string items are for debugging.  They just store a short string of
 * data in the FS
 */
        BTRFS_STRING_ITEM_KEY = 253,
};

FSSTRUCT() btrfs_disk_key {
	__le64 objectid;
	
	FIELD(type=enum btrfs_key_type)
	u8 type;
	
	__le64 offset;
} __attribute__ ((__packed__));

#endif /* BTRFS_KEY_H */

