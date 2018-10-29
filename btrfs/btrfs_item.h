/*
 * btrfs_item.h
 *
 * annotated btrfs structures for everything else that doesn't belong...
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_ITEM_H
#define BTRFS_ITEM_H

#include "btrfs_def.h"

FSCONST(type=flag) btrfs_free_space {
    BTRFS_FREE_SPACE_USING_BITMAPS = 0x01,
};

FSSTRUCT() btrfs_free_space_info {
	__le32 extent_count;
	
	FIELD(type=enum btrfs_free_space)
	__le32 flags;
} __attribute__ ((__packed__));

FSCONST(type=flag) btrfs_qgroup_status {
    BTRFS_QGROUP_STATUS_FLAG_ON		      = (1ULL << 0),
    BTRFS_QGROUP_STATUS_FLAG_RESCAN		  = (1ULL << 1),
    BTRFS_QGROUP_STATUS_FLAG_INCONSISTENT = (1ULL << 2),
};

FSSTRUCT() btrfs_qgroup_status_item {
	__le64 version;
	__le64 generation;
	
	FIELD(type=enum btrfs_qgroup_status)
	__le64 flags;
	
	__le64 scan;		/* progress during scanning */
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_qgroup_info_item {
	__le64 generation;
	__le64 referenced;
	__le64 referenced_compressed;
	__le64 exclusive;
	__le64 exclusive_compressed;
} __attribute__ ((__packed__));

FSCONST(type=flag) btrfs_qgroup_limit {
	BTRFS_QGROUP_LIMIT_MAX_RFER  = (1ULL << 0),
	BTRFS_QGROUP_LIMIT_MAX_EXCL  = (1ULL << 1),
	BTRFS_QGROUP_LIMIT_RSV_RFER  = (1ULL << 2),
	BTRFS_QGROUP_LIMIT_RSV_EXCL  = (1ULL << 3),
	BTRFS_QGROUP_LIMIT_RFER_CMPR = (1ULL << 4),
	BTRFS_QGROUP_LIMIT_EXCL_CMPR = (1ULL << 5),
};

FSSTRUCT() btrfs_qgroup_limit_item {

    FIELD(type=enum btrfs_qgroup_limit)
	__le64 flags;
	
	__le64 max_referenced;
	__le64 max_exclusive;
	__le64 rsv_referenced;
	__le64 rsv_exclusive;
} __attribute__ ((__packed__));

#endif /* BTRFS_ITEM_H */

