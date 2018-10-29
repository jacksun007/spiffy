/**
 *
 * f2fs_nat.h
 *
 * Annotation for F2FS Node Address Table (NAT).
 *
 * Author: Joseph Chu, Kuei (Jack) Sun
 * E-mail: joseph.chu@mail.utoronto.ca, kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 *
 **/

#ifndef F2FS_NAT_H
#define F2FS_NAT_H

#include "f2fs_def.h"
#include "f2fs_inode.h"

#define NAT_ENTRY_PER_BLOCK (PAGE_CACHE_SIZE / sizeof(struct f2fs_nat_entry))

FSSTRUCT() f2fs_nat_entry {
	__u8 version;	/* latest version of cached nat entry */
	__le32 ino;		/* inode number */

    POINTER(repr=block, type=f2fs_block)
	__le32 block_addr;	/* block address */
} __attribute__((packed));

FSSTRUCT(rank=container, size=BLOCK_SIZE) f2fs_nat_block {
	struct f2fs_nat_entry entries[NAT_ENTRY_PER_BLOCK];
} __attribute__((packed));

/* Note: Currently, we just view as one contiguous extent with homogeneous structures. */
VECTOR(name=f2fs_nat_extent, type=struct f2fs_nat_block, size=(SEGMENT_SIZE*sb.segment_count_nat)/2);

#endif /* F2FS_NAT_H */

