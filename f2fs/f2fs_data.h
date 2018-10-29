/**
 *
 * f2fs_data.h
 *
 * Annotation for F2FS Data blocks:
 *  This is further subdivided into file vs. directory data blocks (direntry blocks).
 *  TODO: Inline dentry currently not supported.
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

#ifndef F2FS_DATA_H
#define F2FS_DATA_H

#include "f2fs_def.h"

/* One directory entry slot covers 8bytes-long file name */
#define F2FS_SLOT_LEN		8
#define F2FS_SLOT_LEN_BITS	3

/* the number of dentry in a block */
#define NR_DENTRY_IN_BLOCK	214
#define SIZE_OF_DENTRY_BITMAP	((NR_DENTRY_IN_BLOCK + BITS_PER_BYTE - 1) / \
					BITS_PER_BYTE)
#define SIZE_OF_RESERVED	(PAGE_SIZE - ((SIZE_OF_DIR_ENTRY + \
				F2FS_SLOT_LEN) * \
				NR_DENTRY_IN_BLOCK + SIZE_OF_DENTRY_BITMAP))

/* MAX level for dir lookup */
#define MAX_DIR_HASH_DEPTH	63
#define SIZE_OF_DIR_ENTRY	11	/* by byte */

/* For use in: f2fs_dir_entry->file_type */
//FSCONST(type=flag) ftype { 
//	F2FS_FT_UNKNOWN	= 0,
//	F2FS_FT_FIFO	= 1,
//	F2FS_FT_CHRDEV	= 2,
//	F2FS_FT_DIR		= 4,
//	F2FS_FT_BLKDEV	= 6,
//	F2FS_FT_REG_FILE= 8,
//	F2FS_FT_SYMLINK	= 10,
//	F2FS_FT_SOCK	= 12,
//};

/* Regular File Data Block */
VECTOR(name=f2fs_data_block, type=data, size=BLOCK_SIZE);

/* Directory Data Entry */
FSSTRUCT() f2fs_dir_entry {
	__le32 hash_code;	/* hash code of file name */
	__le32 ino;		/* inode number */
	__le16 name_len;	/* lengh of file name */
    //FIELD(type=enum ftype)
	__u8 file_type;		/* file type */
} __attribute__((packed));

/* 4KB-sized directory entry block */
FSSTRUCT(rank=container, size=BLOCK_SIZE) f2fs_dentry_block {
	/* validity bitmap for directory entries in each block */
	FIELD(type=bitmap)
	__u8 dentry_bitmap[SIZE_OF_DENTRY_BITMAP];
	__u8 reserved[SIZE_OF_RESERVED];
	struct f2fs_dir_entry dentry[NR_DENTRY_IN_BLOCK];
    char filename[NR_DENTRY_IN_BLOCK][F2FS_SLOT_LEN];
} __attribute__((packed));

#endif /* F2FS_DATA_H */
