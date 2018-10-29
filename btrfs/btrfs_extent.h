/*
 * btrfs_dir.h
 *
 * annotated btrfs structures for items inside the extent tree and extent data
 * items (which exist in the fs tree)
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_EXTENT_H
#define BTRFS_EXTENT_H

#include "btrfs_key.h"

typedef FSCONST() {
	BTRFS_COMPRESS_NONE  = 0,
	BTRFS_COMPRESS_ZLIB  = 1,
	BTRFS_COMPRESS_LZO   = 2,
	// (jsun): what is this?? 
	/* BTRFS_COMPRESS_TYPES = 2, */
	BTRFS_COMPRESS_LAST  = 3,
} btrfs_compression_type;

/* we don't understand any encryption methods right now */
typedef FSCONST() {
	BTRFS_ENCRYPTION_NONE = 0,
	BTRFS_ENCRYPTION_LAST = 1,
} btrfs_encryption_type;

FSCONST() btrfs_file_extent {
    BTRFS_FILE_EXTENT_INLINE = 0,
    BTRFS_FILE_EXTENT_REG = 1,
    BTRFS_FILE_EXTENT_PREALLOC = 2,
};

FSSTRUCT() btrfs_file_extent_item {
	/*
	 * transaction id that created this extent
	 */
	__le64 generation;
	/*
	 * max number of bytes to hold this extent in ram
	 * when we split a compressed extent we can't know how big
	 * each of the resulting pieces will be.  So, this is
	 * an upper limit on the size of the extent in ram instead of
	 * an exact limit.
	 */
	__le64 ram_bytes;

	/*
	 * 32 bits for the various ways we might encode the data,
	 * including compression and encryption.  If any of these
	 * are set to something a given disk format doesn't understand
	 * it is treated like an incompat flag for reading and writing,
	 * but not for stat.
	 */
	FIELD(type=btrfs_compression_type)
	u8 compression;
	
	FIELD(type=btrfs_encryption_type)
	u8 encryption;
	
	__le16 other_encoding; /* spare for later use */

	/* are we inline data or a real extent? */
	FIELD(type=enum btrfs_file_extent)
	u8 type;
	
} __attribute__ ((__packed__));

VECTOR(name=data_extent, type=data, size=sb.sectorsize);

FSSTRUCT(base=struct btrfs_file_extent_item, 
         when=self.type != BTRFS_FILE_EXTENT_INLINE) btrfs_real_file_extent {
	/*
	 * disk space consumed by the extent, checksum blocks are included
	 * in these numbers
	 */
	POINTER(repr=byte, type=data_extent, size=self.disk_num_bytes) 
	__le64 disk_bytenr;
	__le64 disk_num_bytes;
	/*
	 * the logical offset in file blocks (no csums)
	 * this extent record is for.  This allows a file extent to point
	 * into the middle of an existing extent on disk, sharing it
	 * between two snapshots (useful if some bytes in the middle of the
	 * extent have changed
	 */
	__le64 offset;
	/*
	 * the logical number of file blocks (no csums included)
	 */
	__le64 num_bytes;	
} __attribute__ ((__packed__));

/*
 * items in the extent btree are used to record the objectid of the
 * owner of the block and the number of references
 */

FSCONST(type=flag) btrfs_extent_flag {
    BTRFS_EXTENT_FLAG_DATA       = 0x01,
    BTRFS_EXTENT_FLAG_TREE_BLOCK = 0x02,
    BTRFS_BLOCK_FLAG_FULL_BACKREF = (1 << 8),
};

// (jsun): we don't want to annotate old stuff
//         struct btrfs_extent_item_v0...

FSSTRUCT() btrfs_extent_item {
        __le64 refs;
        __le64 generation;

        FIELD(type=enum btrfs_extent_flag)
        __le64 flags;
       
} __attribute__ ((__packed__));

DEFINE(name=IS_TREE_BLOCK, expr=self.flags & BTRFS_EXTENT_FLAG_TREE_BLOCK);

FSSTRUCT(base=struct btrfs_extent_item, 
         when=IS_TREE_BLOCK) btrfs_tree_block_info {
        struct btrfs_disk_key key;
        u8 level;
        
        // (jsun): technically only SHARED_BLOCK_REF and TREE_BLOCK_REF are
        // possible here -- but for some reason btrfs designer wants to make 
        // this unreasonably ugly to read.
        FIELD(type=enum btrfs_key_type)
        u8 type;       
        __le64 offset;       
} __attribute__ ((__packed__));

FSSTRUCT(base=struct btrfs_extent_item, 
         when=!IS_TREE_BLOCK) btrfs_extent_inline_ref {
        
        FIELD(type=enum btrfs_key_type)
        u8 type;
        	
} __attribute__ ((__packed__));

FSSTRUCT(base=struct btrfs_extent_inline_ref, 
         when=self.type==BTRFS_EXTENT_DATA_REF_KEY) btrfs_extent_data_ref {
	__le64 root;
	__le64 objectid;
	__le64 offset;
	__le32 count;
} __attribute__ ((__packed__));

FSSTRUCT(base=struct btrfs_extent_inline_ref, 
         when=self.type==BTRFS_SHARED_DATA_REF_KEY) btrfs_shared_data_ref {
    __le64 offset;
	__le32 count;
} __attribute__ ((__packed__));

/* tag for the radix tree of block groups in ram */
FSCONST(type=flag) btrfs_block_group {
	BTRFS_BLOCK_GROUP_DATA	    = (1ULL << 0),
	BTRFS_BLOCK_GROUP_SYSTEM    = (1ULL << 1),
	BTRFS_BLOCK_GROUP_METADATA  = (1ULL << 2),
	BTRFS_BLOCK_GROUP_RAID0	    = (1ULL << 3),
	BTRFS_BLOCK_GROUP_RAID1	    = (1ULL << 4),
	BTRFS_BLOCK_GROUP_DUP	    = (1ULL << 5),
	BTRFS_BLOCK_GROUP_RAID10    = (1ULL << 6),
	BTRFS_BLOCK_GROUP_RAID5     = (1ULL << 7),
	BTRFS_BLOCK_GROUP_RAID6     = (1ULL << 8),
};

FSSTRUCT() btrfs_block_group_item {
	__le64 used;
	__le64 chunk_objectid;
	
	FIELD(type=enum btrfs_block_group)
	__le64 flags;
} __attribute__ ((__packed__));

#endif /* BTRFS_EXTENT_H */

