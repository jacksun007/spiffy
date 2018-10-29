/*
 * f2fs_sit.h
 *
 * Annotations for Segment Summary Area (SSA).
 *
 * Author: Joseph Chu, Kuei (Jack) Sun
 * Email: joseph.chu@mail.utoronto.ca, kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
*/

#ifndef F2FS_SSA_H
#define F2FS_SSA_H

#include "f2fs_def.h"
#include "f2fs_nat.h"
#include "f2fs_sit.h"

#define SUM_JOURNAL_SIZE	(F2FS_BLKSIZE - SUM_FOOTER_SIZE -\
				SUM_ENTRIES_SIZE)
#define NAT_JOURNAL_ENTRIES	((SUM_JOURNAL_SIZE - 2) /\
				sizeof(struct nat_journal_entry))
#define NAT_JOURNAL_RESERVED	((SUM_JOURNAL_SIZE - 2) %\
				sizeof(struct nat_journal_entry))
#define SIT_JOURNAL_ENTRIES	((SUM_JOURNAL_SIZE - 2) /\
				sizeof(struct sit_journal_entry))
#define SIT_JOURNAL_RESERVED	((SUM_JOURNAL_SIZE - 2) %\
				sizeof(struct sit_journal_entry))

#define EXTRA_INFO_RESERVED	(SUM_JOURNAL_SIZE - 2 - 8)

/* summary block type, node or data, is stored to the summary_footer */
FSCONST(type=enum) block_type {
        SUM_TYPE_NODE = 1,
        SUM_TYPE_DATA = 0,
};

/* NAT Journaling */
FSSTRUCT() nat_journal_entry {
	__le32 nid;
	struct f2fs_nat_entry ne;
} __attribute__((packed));

FSSTRUCT() nat_journal {
	struct nat_journal_entry entries[NAT_JOURNAL_ENTRIES];
	__u8 reserved[NAT_JOURNAL_RESERVED];
} __attribute__((packed));

FSSTRUCT() f2fs_nat_journal {
    __le16 n_nats;
	struct nat_journal nat_j;
} __attribute__((packed));

/* SIT Journaling */
FSSTRUCT() sit_journal_entry {
	__le32 segno;
	struct f2fs_sit_entry se;
} __attribute__((packed));

FSSTRUCT() sit_journal {
	struct sit_journal_entry entries[SIT_JOURNAL_ENTRIES];
	__u8 reserved[SIT_JOURNAL_RESERVED];
} __attribute__((packed));

FSSTRUCT() f2fs_sit_journal {
    __le16 n_sits;
	struct sit_journal sit_j;
} __attribute__((packed));

/* (jsun) TODO: this doesn't seem to actually be used */
/*
FSSTRUCT() f2fs_extra_info {
	__le64 kbytes_written;
	__u8 reserved[EXTRA_INFO_RESERVED];
} __attribute__((packed));
*/

FSSTRUCT() f2fs_node_summary {
    __le32 nid;		    /* parent node id */
	__u8 reserved[3];
} __attribute__((packed));

FSSTRUCT() f2fs_data_summary {
    __le32 nid;		    /* parent node id */
    __u8 version;		/* node version number */
    __le16 ofs_in_node;	/* block index in parent node */
} __attribute__((packed));

//VECTOR(name=f2fs_summaries, type=struct f2fs_summary, count=ENTRIES_IN_SUM);

FSSTRUCT() summary_footer {
    FIELD(type=enum block_type)
	unsigned char entry_type;	/* SUM_TYPE_XXX */
	__u32 check_sum;		/* summary checksum */
} __attribute__((packed));

typedef FSSTRUCT() {
    struct f2fs_node_summary entries[ENTRIES_IN_SUM];   
    struct f2fs_nat_journal  journal;
} f2fs_node_summary_block;

typedef FSSTRUCT() {
    struct f2fs_data_summary entries[ENTRIES_IN_SUM];   
    struct f2fs_sit_journal  journal;
} f2fs_data_summary_block;

/* 4KB-sized summary block structure */
FSSTRUCT(rank=container, size=BLOCK_SIZE) f2fs_summary_block
{
    /* (jsun) NEW: skip fields are used to skip forward during parsing
     * and does not actually cause a visible field to be generated */
    FIELD(type=skip)
    __u8 _rsvd[F2FS_BLKSIZE - SUM_FOOTER_SIZE];
    
	struct summary_footer footer;	
	POINTER(repr=offset, type=f2fs_node_summary_block, name=sm, expr=0, 
            when=self.footer.entry_type == SUM_TYPE_NODE);
    POINTER(repr=offset, type=f2fs_data_summary_block, name=sm, expr=0, 
            when=self.footer.entry_type == SUM_TYPE_DATA);        
} __attribute__((packed));

VECTOR(name=f2fs_ssa_extent, type=struct f2fs_summary_block, 
            size=SEGMENT_SIZE*sb.segment_count_ssa, count=NUM_SEGMENTS);

/* original structure definition */
/*
struct f2fs_journal {
	union {
		__le16 n_nats;
		__le16 n_sits;
	};
	
	union {
		struct nat_journal nat_j;
		struct sit_journal sit_j;
		struct f2fs_extra_info info;
	};
} __attribute__((packed));

struct f2fs_summary_block {
	struct f2fs_summary entries[ENTRIES_IN_SUM];
	struct f2fs_journal journal;
	struct summary_footer footer;
} __attribute__((packed));
*/


#endif /* F2FS_SSA_H */
