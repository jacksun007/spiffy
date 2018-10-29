/*
 * f2fs_sit.h
 *
 * annotated segment information table (sit) in f2fs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef F2FS_SIT_H
#define F2FS_SIT_H

#include "f2fs_def.h"

/*
 * Note that f2fs_sit_entry->vblocks has the following bit-field information.
 * [15:10] : allocation type such as CURSEG_XXXX_TYPE
 * [9:0] : valid block count
 */
#define SIT_VBLOCKS_SHIFT	10
#define SIT_VBLOCKS_MASK	((1 << SIT_VBLOCKS_SHIFT) - 1)
#define GET_SIT_VBLOCKS(raw_sit)				\
	(le16_to_cpu((raw_sit)->vblocks) & SIT_VBLOCKS_MASK)
#define GET_SIT_TYPE(raw_sit)					\
	((le16_to_cpu((raw_sit)->vblocks) & ~SIT_VBLOCKS_MASK)	\
	 >> SIT_VBLOCKS_SHIFT)

#define SIT_VBLOCK_MAP_SIZE 64
#define SIT_ENTRY_PER_BLOCK (PAGE_CACHE_SIZE / sizeof(struct f2fs_sit_entry))

FSCONST(type=enum) curseg_type {
	CURSEG_HOT_DATA	= 0,	/* directory entry blocks */
	CURSEG_WARM_DATA,	    /* data blocks */
	CURSEG_COLD_DATA,	    /* multimedia or GCed data blocks */
	CURSEG_HOT_NODE,	    /* direct node blocks of directory files */
	CURSEG_WARM_NODE,	    /* direct node blocks of normal files */
	CURSEG_COLD_NODE,	    /* indirect node blocks */
};

FSSTRUCT() f2fs_sit_entry {
	__le16 vblocks;				            /* reference above */
	
	FIELD(type=bitmap)
	__u8 valid_map[SIT_VBLOCK_MAP_SIZE];	/* bitmap for valid blocks */
	__le64 mtime;				            /* segment age for cleaning */
	
	FIELD(name=curseg, type=enum curseg_type, 
	    expr=(self.vblocks & ~SIT_VBLOCKS_MASK) >> SIT_VBLOCKS_SHIFT);
    FIELD(name=valid_cnt, type=int, expr=self.vblocks & SIT_VBLOCKS_MASK);	    
} __attribute__((packed));

FSSTRUCT(rank=container, size=BLOCK_SIZE) f2fs_sit_block {
	struct f2fs_sit_entry entries[SIT_ENTRY_PER_BLOCK];
} __attribute__((packed));

/* (jchu): view as one contiguous extent with homogeneous structures. */
VECTOR(name=f2fs_sit_extent, type=struct f2fs_sit_block, 
       count=ceil(NUM_SEGMENTS*sizeof(f2fs_sit_entry), F2FS_BLKSIZE));

#endif /* F2FS_SIT_H */
