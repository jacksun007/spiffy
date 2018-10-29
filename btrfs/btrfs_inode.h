/*
 * btrfs_inode.h
 *
 * annotated btrfs structures for inode-related items and all items in the root
 * tree (e.g., root_ref)
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_INODE_H
#define BTRFS_INODE_H

#include <jd.h>

FSSTRUCT() btrfs_timespec {
    FIELD(type=timestamp)
	__le64 sec;
	__le32 nsec;
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_inode_item {
	/* nfs style generation number */
	__le64 generation;
	/* transid that last touched this inode */
	__le64 transid;
	__le64 size;
	__le64 nbytes;
	__le64 block_group;
	__le32 nlink;
	__le32 uid;
	__le32 gid;
	__le32 mode;
	__le64 rdev;
	__le64 flags;

	/* modification sequence number for NFS */
	__le64 sequence;

	/*
	 * a little future expansion, for more than this we can
	 * just grow the inode item and version it
	 */
	__le64 reserved[4];
	struct btrfs_timespec atime;
	struct btrfs_timespec ctime;
	struct btrfs_timespec mtime;
	struct btrfs_timespec otime;
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_inode_ref {
	__le64 index;
	__le16 name_len;
	
	VECTOR(name=name, type=char, size=self.name_len);
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_inode_extref {
	__le64 parent_objectid;
	__le64 index;
	__le16 name_len;
	
	VECTOR(name=name, type=char, size=self.name_len);
} __attribute__ ((__packed__));

#endif /* BTRFS_INODE_H */

