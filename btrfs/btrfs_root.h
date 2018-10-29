/*
 * btrfs_inode.h
 *
 * annotated btrfs structures for all items in the root tree
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_ROOT_H
#define BTRFS_ROOT_H

#include "btrfs_key.h"
#include "btrfs_inode.h"

/*
 * (jsun): btrfs_root_item_v0
 * we will ignore any older versions of the structs for sanity purposes...
 */

FSSTRUCT() btrfs_root_item {
	struct btrfs_inode_item inode;
	__le64 generation;
	__le64 root_dirid;
	
	POINTER(repr=byte, type=struct btrfs_node, when=self.level > 0)
    POINTER(repr=byte, type=struct btrfs_leaf, when=self.level == 0)
	__le64 bytenr;
	
	__le64 byte_limit;
	__le64 bytes_used;
	__le64 last_snapshot;
	__le64 flags;
	__le32 refs;
	struct btrfs_disk_key drop_progress;
	u8 drop_level;
	u8 level;

	/*
	 * The following fields appear after subvol_uuids+subvol_times
	 * were introduced.
	 */

	/*
	 * This generation number is used to test if the new fields are valid
	 * and up to date while reading the root item. Everytime the root item
	 * is written out, the "generation" field is copied into this field. If
	 * anyone ever mounted the fs with an older kernel, we will have
	 * mismatching generation values here and thus must invalidate the
	 * new fields. See btrfs_update_root and btrfs_find_last_root for
	 * details.
	 * the offset of generation_v2 is also used as the start for the memset
	 * when invalidating the fields.
	 */
	__le64 generation_v2;
	
	FIELD(type=uuid)
	u8 uuid[BTRFS_UUID_SIZE];
	
	FIELD(type=uuid)
	u8 parent_uuid[BTRFS_UUID_SIZE];
	
	FIELD(type=uuid)
	u8 received_uuid[BTRFS_UUID_SIZE];
	
	__le64 ctransid; /* updated when an inode changes */
	__le64 otransid; /* trans when created */
	__le64 stransid; /* trans when sent. non-zero for received subvol */
	__le64 rtransid; /* trans when received. non-zero for received subvol */
	struct btrfs_timespec ctime;
	struct btrfs_timespec otime;
	struct btrfs_timespec stime;
	struct btrfs_timespec rtime;
    __le64 reserved[8]; /* for future */
} __attribute__ ((__packed__));

/*
 * this is used for both forward and backward root refs
 */
FSSTRUCT() btrfs_root_ref {
	__le64 dirid;
	__le64 sequence;
	__le16 name_len;
	
	VECTOR(name=name, type=char, size=self.name_len);
} __attribute__ ((__packed__));

#endif /* BTRFS_ROOT_H */

