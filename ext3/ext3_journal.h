/*
 * Copyright (C) 2015
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Annotated ext3 journal metadata
 */

#ifndef EXT3_JOURNAL_H
#define EXT3_JOURNAL_H
 
#include <jd.h>

/*
 * Internal structures used by the logging mechanism:
 */

#define JFS_MAGIC_NUMBER 0xc03b3998U /* The first 4 bytes of /dev/random! */

VECTOR(name=ext3_journal, type=struct journal_block, size=BLOCK_SIZE*100);

/*
 * Base class for all journal blocks:
 */
FSSTRUCT(rank=container, size=BLOCK_SIZE) journal_block
{
    __be32 h_magic;
};

/*
 * Descriptor block types:
 */

FSCONST() journal_block_type
{
	JFS_DESCRIPTOR_BLOCK = 1,
	JFS_COMMIT_BLOCK = 2,
	JFS_SUPERBLOCK_V1 = 3,
	JFS_SUPERBLOCK_V2 = 4,
	JFS_REVOKE_BLOCK = 5,
};

FSSTRUCT(base=struct journal_block, 
         when=self.h_magic == JFS_MAGIC_NUMBER) journal_header
{
    // __be32   h_magic;

    FIELD(type=enum journal_block_type)
	__be32  h_blocktype;
	__be32  h_sequence;
};

/* Definitions for the journal tag flags word: */
enum journal_flag
{
	JFS_FLAG_ESCAPE = 1,	/* on-disk block is escaped */
	JFS_FLAG_SAME_UUID = 2,	/* block has same uuid as previous */
	JFS_FLAG_DELETED = 4,	/* block deleted by this transaction */
	JFS_FLAG_LAST_TAG = 8,	/* last tag in this descriptor block */
};

/*
 * The block tag: used to describe a single buffer in the journal
 */
typedef FSSTRUCT() journal_block_tag_s
{
	__be32  t_blocknr;	/* The on-disk block number */
	__be32  t_flags;	/* See below */
	
} journal_block_tag_t;

/*FSSTRUCT(base=struct journal_header,
         when=self.h_blocktype == JFS_DESCRIPTOR_BLOCK) journal_descriptor_block
{
    FIELD(sentinel=self.t_flags & JFS_FLAG_LAST_TAG)
    journal_block_tag_t  d_blocktag[];
};*/

FSSTRUCT(base=struct journal_header,
         when=self.h_blocktype == JFS_REVOKE_BLOCK) journal_revoke_block
{
    __be32  r_count;	/* Count of bytes used in the block */

    FIELD(count=self.r_count)
    __be32  r_blocknr[];
};

FSSTRUCT(base=struct journal_header,
         when=self.h_blocktype == JFS_COMMIT_BLOCK) journal_commit_block
{
    /* no extra fields inside a commit block */
};

typedef FSSTRUCT(base=struct journal_header,
         when=self.h_blocktype == JFS_SUPERBLOCK_V2) journal_superblock_s
{
/* 0x0000 */
	// journal_header_t s_header;

/* 0x000C */
	/* Static information describing the journal */
	__be32	s_blocksize;		/* journal device blocksize */
	__be32	s_maxlen;		/* total blocks in journal file */
	__be32	s_first;		/* first block of log information */

/* 0x0018 */
	/* Dynamic information describing the current state of the log */
	__be32	s_sequence;		/* first commit ID expected in log */
	__be32	s_start;		/* blocknr of start of log */

/* 0x0020 */
	/* Error value, as set by journal_abort(). */
	__be32	s_errno;

/* 0x0024 */
	/* Remaining fields are only valid in a version-2 superblock */
	__be32	s_feature_compat;	/* compatible feature set */
	__be32	s_feature_incompat;	/* incompatible feature set */
	__be32	s_feature_ro_compat;	/* readonly-compatible feature set */
/* 0x0030 */
    FIELD(type=uuid)
	__u8	s_uuid[16];		/* 128-bit uuid for journal */

/* 0x0040 */
	__be32	s_nr_users;		/* Nr of filesystems sharing log */

	__be32	s_dynsuper;		/* Blocknr of dynamic superblock copy*/

/* 0x0048 */
	__be32	s_max_transaction;	/* Limit of journal blocks per trans.*/
	__be32	s_max_trans_data;	/* Limit of data blocks per trans. */

/* 0x0050 */
	__u32	s_padding[44];

/* 0x0100 */
	__u8	s_users[16*48];		/* ids of all fs'es sharing the log */
/* 0x0400 */
} journal_superblock_t;


FSSTRUCT(base=journal_block_tag_t, when=!(self.t_flags & JFS_FLAG_SAME_UUID))
journal_block_tag_uuid
{
    FIELD(type=uuid)
    __u8	t_uuid[16];
};

#endif /* EXT3_JOURNAL_H */

