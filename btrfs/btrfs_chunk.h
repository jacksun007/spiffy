/*
 * btrfs_chunk.h
 *
 * annotated btrfs structures for all the possible items in the chunk tree
 * and the dev tree
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_CHUNK_H
#define BTRFS_CHUNK_H

#include "btrfs_def.h"

// ---------- part of chunk tree -----------

FSSTRUCT() btrfs_stripe {
	__le64 devid;
	__le64 offset;
	
	FIELD(type=uuid)
	u8 dev_uuid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_chunk {
	/* size of this chunk in bytes */
	__le64 length;

	/* objectid of the root referencing this chunk */
	__le64 owner;

	__le64 stripe_len;
	__le64 type;

	/* optimal io alignment for this chunk */
	__le32 io_align;

	/* optimal io width for this chunk */
	__le32 io_width;

	/* minimal io size for this chunk */
	__le32 sector_size;

	/* 2^16 stripes is quite a lot, a second limit is the size of a single
	 * item in the btree
	 */
	__le16 num_stripes;

	/* sub stripes only matter for raid10 */
	__le16 sub_stripes;
	
	FIELD(count=self.num_stripes)
	struct btrfs_stripe stripe[];
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_dev_item {
	/* the internal btrfs device id */
	__le64 devid;

	/* size of the device */
	__le64 total_bytes;

	/* bytes used */
	__le64 bytes_used;

	/* optimal io alignment for this device */
	__le32 io_align;

	/* optimal io width for this device */
	__le32 io_width;

	/* minimal io size for this device */
	__le32 sector_size;

	/* type and info about this device */
	__le64 type;

	/* expected generation for this device */
	__le64 generation;

	/*
	 * starting byte of this partition on the device,
	 * to allowr for stripe alignment in the future
	 */
	__le64 start_offset;

	/* grouping information for allocation decisions */
	__le32 dev_group;

	/* seek speed 0-100 where 100 is fastest */
	u8 seek_speed;

	/* bandwidth 0-100 where 100 is fastest */
	u8 bandwidth;

	/* btrfs generated uuid for this device */
	FIELD(type=uuid)
	u8 uuid[BTRFS_UUID_SIZE];

	/* uuid of FS who owns this device */
	FIELD(type=uuid)
	u8 fsid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

// ---------- part of dev tree -----------

/* dev extents record free space on individual devices.  The owner
 * field points back to the chunk allocation mapping tree that allocated
 * the extent.  The chunk tree uuid field is a way to double check the owner
 */
FSSTRUCT() btrfs_dev_extent {
	__le64 chunk_tree;
	__le64 chunk_objectid;
	__le64 chunk_offset;
	__le64 length;
	
	FIELD(type=uuid)
	u8 chunk_tree_uuid[BTRFS_UUID_SIZE];
} __attribute__ ((__packed__));

#endif

