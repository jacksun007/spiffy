/*
 * btrfs_dir.h
 *
 * annotated btrfs structures for directory-related items
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_DIR_H
#define BTRFS_DIR_H

#include "btrfs_key.h"

FSCONST(type=enum) btrfs_file_type {
        BTRFS_FT_UNKNOWN  = 0,        /* unknown file type */
        BTRFS_FT_REG_FILE = 1,        /* regular file */
        BTRFS_FT_DIR      = 2,        /* directory */
        BTRFS_FT_CHRDEV   = 3,        /* character device */
        BTRFS_FT_BLKDEV   = 4,        /* block device */
        BTRFS_FT_FIFO     = 5,        /* FIFO / buffer file */
        BTRFS_FT_SOCK     = 6,        /* socket */
        BTRFS_FT_SYMLINK  = 7,        /* symbolic link */
		BTRFS_FT_XATTR    = 8,
};

FSSTRUCT() btrfs_dir_item {
	struct btrfs_disk_key location;
	__le64 transid;
	__le16 data_len;
	__le16 name_len;
	
	FIELD(type=enum btrfs_file_type)
	u8 type;
	
	VECTOR(name=name, type=char, size=self.name_len);
	VECTOR(name=data, type=char, size=self.data_len);
} __attribute__ ((__packed__));

FSSTRUCT() btrfs_dir_log_item {
	__le64 end;
} __attribute__ ((__packed__));


#endif
