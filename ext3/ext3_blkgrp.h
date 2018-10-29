/*
 * Copyright (C) 2014
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Annotated ext3 block groups and block group descriptors
 */

#ifndef EXT3_BLKGRP_H
#define EXT3_BLKGRP_H

#include <jd.h>
#include "ext3_super.h"

#ifndef ceil
#define ceil(x, y) (((y) != 0) ? (((x) + (y) - 1) / (y)) : 0)
#endif

// the block size of the file system
DEFINE(name=BLOCK_SIZE, expr=1024 << sb.s_log_block_size);

// number of block groups in the file system
#define NUM_BLOCK_GROUP ceil(sb.s_blocks_count, sb.s_blocks_per_group)

// there is always exactly 1 block of each type of bitmap
VECTOR( name=ext3_block_bitmap, type=bitmap, size=BLOCK_SIZE );
VECTOR( name=ext3_inode_bitmap, type=bitmap, size=BLOCK_SIZE ); 

VECTOR( name=ext3_inode_table, 
        type=ext3_inode_block, 
        size=sb.s_inodes_per_group*sb.s_inode_size
);

VECTOR(name=ext3_group_desc_table,
       type=ext3_group_desc_block,
       size=ceil(NUM_BLOCK_GROUP*sizeof(struct ext3_group_desc), BLOCK_SIZE)*BLOCK_SIZE
);

VECTOR(name=ext3_group_desc_block,
       type=struct ext3_group_desc,
       size=BLOCK_SIZE
);

FSCONST(type=flag) ext3_bg {
	EXT3_BG_INODE_UNINIT = 0x0001, /* Inode table/bitmap not initialized */
    EXT3_BG_BLOCK_UNINIT = 0x0002, /* Block bitmap not initialized */
    EXT3_BG_INODE_ZEROED = 0x0004, /* On-disk itable initialized to zero */
};

FSSTRUCT() ext3_group_desc
{
        POINTER(repr=block, type=ext3_block_bitmap, when=!(self.bg_flags & EXT3_BG_BLOCK_UNINIT))
        __le32	bg_block_bitmap;	    /* Blocks bitmap block */
        
        POINTER(repr=block, type=ext3_inode_bitmap, when=!(self.bg_flags & EXT3_BG_INODE_UNINIT))
        __le32	bg_inode_bitmap;	    /* Inodes bitmap block */
        
        POINTER(repr=block, type=ext3_inode_table, when=!(self.bg_flags & EXT3_BG_INODE_UNINIT))
        __le32	bg_inode_table;		    /* Inodes table block */
        
        __le16	bg_free_blocks_count;	/* Free blocks count */
        __le16	bg_free_inodes_count;	/* Free inodes count */
        __le16	bg_used_dirs_count;	    /* Directories count */
        
        FIELD(type=enum ext3_bg)
        __u16	bg_flags;
        
	    __u32	bg_exclude_bitmap_lo;	/* Exclude bitmap for snapshots */
	    __u16	bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
	    __u16	bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bitmap) LSB */
	    __u16	bg_itable_unused;	/* Unused inodes count */
	    __u16	bg_checksum;		/* crc16(s_uuid+group_num+group_desc)*/
};

#endif /* EXT3_BLKGRP_H */


 
