/*
 * f2fs_super.h
 *
 * annotated super block in f2fs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef F2FS_SUPER_H
#define F2FS_SUPER_H

#include "f2fs_def.h"
#include "f2fs_cp.h"
#include "f2fs_nat.h"
#include "f2fs_sit.h"
#include "f2fs_ssa.h"

#define F2FS_SUPER_OFFSET   1024    /* byte-size offset */
#define F2FS_MAX_EXTENSION  64      /* # of extension entries */
#define VERSION_LEN	        256

ADDRSPACE(name=block, null=0);
//ADDRSPACE(name=logical, size=1 << sb.log_blocksize, null=-1);
//ADDRSPACE(name=nid, size=1 << sb.log_blocksize, null=-1);

/* (jsun): this expression uses "self", which is only usable within the
 * superblock itself */
DEFINE(name=BLOCKS_PER_SEGMENT, expr=1 << self.log_blocks_per_seg);

/*
 * For superblock
 * Note in default mkfs, we have 512 blocks == 1 seg == 1 section == 1 zone
 */
FSSUPER(name=sb, location=F2FS_SUPER_OFFSET) f2fs_super_block {
	__le32 magic;			        /* Magic Number */
	__le16 major_ver;		        /* Major Version */
	__le16 minor_ver;		        /* Minor Version */
	__le32 log_sectorsize;		    /* log2 sector size in bytes */
	__le32 log_sectors_per_block;	/* log2 # of sectors per block */
	__le32 log_blocksize;		    /* log2 block size in bytes */
	__le32 log_blocks_per_seg;	    /* log2 # of blocks per segment */
	__le32 segs_per_sec;		    /* # of segments per section */
	__le32 secs_per_zone;		    /* # of sections per zone */

	/* (jsun): current not in-use? */
	__le32 checksum_offset;		    /* checksum offset inside super block */
	__le64 block_count;		        /* total # of user blocks */
	__le32 section_count;		    /* total # of sections */
	__le32 segment_count;		    /* total # of segments */
	__le32 segment_count_ckpt;	    /* # of segments for checkpoint */
	__le32 segment_count_sit;	    /* # of segments for SIT */
	__le32 segment_count_nat;	    /* # of segments for NAT */
	__le32 segment_count_ssa;	    /* # of segments for SSA */
	__le32 segment_count_main;	    /* # of segments for main area */

    // The starting position of segment is at CP region: segment0_blkaddr == cp_blkaddr
	__le32 segment0_blkaddr;	    /* start block address of segment 0 */

    POINTER(repr=block, type=f2fs_checkpoint_header)
	__le32 cp_blkaddr;		/* start block address of checkpoint */

    /* (jsun): points to the first sit extent */
    POINTER(repr=block, type=f2fs_sit_extent)
	__le32 sit_blkaddr;		/* start block address of SIT */

    POINTER(repr=block, type=f2fs_nat_extent)
	__le32 nat_blkaddr;		/* start block address of NAT */

    POINTER(repr=block, type=f2fs_ssa_extent)
	__le32 ssa_blkaddr;		/* start block address of SSA */

	__le32 main_blkaddr;	/* start block address of main area */
	__le32 root_ino;		/* root inode number */
	__le32 node_ino;		/* node inode number */
	__le32 meta_ino;		/* meta inode number */

    FIELD(type=uuid)
	__u8 uuid[16];			/* 128-bit uuid for volume */

    /* TODO: Low Priority: FIELD(type=utf16); */
	__le16 volume_name[512];	/* volume name */
	__le32 extension_count;		/* # of extensions below */

    // (jsun): actual type was __u8, but jdc expects char for c-strings
    char extension_list[F2FS_MAX_EXTENSION][8];	/* extension array */

	__le32 cp_payload;
	char version[VERSION_LEN];	    /* the kernel version */
    char init_version[VERSION_LEN];	/* the initial kernel version */
	__le32 feature;			        /* defined features */
	__u8 encryption_level;		    /* versioning level for encryption */
	__u8 encrypt_pw_salt[16];	    /* Salt used for string2key algorithm */
	
	POINTER(name=cp_blkaddr2, repr=block, type=f2fs_checkpoint_header, 
	        expr=self.cp_blkaddr + (1 << self.log_blocks_per_seg));        
	POINTER(name=sit_blkaddr2, repr=block, type=f2fs_sit_extent,
	        expr=self.sit_blkaddr + self.segment_count_sit*BLOCKS_PER_SEGMENT/2);
	POINTER(name=nat_blkaddr2, repr=block, type=f2fs_nat_extent,
	        expr=self.nat_blkaddr + self.segment_count_nat*BLOCKS_PER_SEGMENT/2);	           
} __attribute__((packed));

#endif /* F2FS_SUPER_H */

