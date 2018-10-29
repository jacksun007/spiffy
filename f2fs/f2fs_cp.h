/**
 * f2fs_cp.h
 *
 * Annotations for Checkpoint Region & Orphan Inode Management.
 * For simplicity:
 *      Assuming 3 data logs and 3 node logs. This is default setting.
 *
 * Author: Joseph Chu, Kuei (Jack) Sun
 * Email: joseph.chu@mail.utoronto.ca, kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 *
 **/

#ifndef F2FS_CP_H
#define F2FS_CP_H

#include "f2fs_ssa.h"
#include "f2fs_super.h"
#include "f2fs_inode.h"

#define MAX_ACTIVE_LOGS	        16
#define MAX_ACTIVE_NODE_LOGS	8
#define MAX_ACTIVE_DATA_LOGS	8

#define	NR_CURSEG_DATA_TYPE	(3)
#define NR_CURSEG_NODE_TYPE	(3)
#define NR_CURSEG_TYPE	(NR_CURSEG_DATA_TYPE + NR_CURSEG_NODE_TYPE)

/* For Compact Summary: TODO: change casting. */
#ifndef MIN
#define MIN(a,b) \
    ({ __typeof__ (b) _a = (a); \
       __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#endif

/* Macros for cp state */
DEFINE(name=HAS_ORPHANS, expr=self.ckpt_flags & CP_ORPHAN_PRESENT_FLAG);
DEFINE(name=IS_COMPACT_SUMM, expr=self.ckpt_flags & CP_COMPACT_SUM_FLAG);
// fastboot is mount-time opt
DEFINE(name=HAS_NODE_SUMM, expr=( (self.ckpt_flags & CP_UMOUNT_FLAG) || 
            (self.ckpt_flags & CP_FASTBOOT_FLAG) ));

DEFINE(name=NODE_SUMM_START_ADDR, expr=$self.addr + self.cp_pack_total_block_count - 1 - NR_CURSEG_NODE_TYPE);
DEFINE(name=COMPACT_FULL_START, expr=$self.addr + self.cp_pack_start_sum + 1);

#define REMAINDER_ENTRIES_SZ \
    ( (F2FS_BLKSIZE - SUM_FOOTER_SIZE - 2*SUM_JOURNAL_SIZE) )

DEFINE(name=CURSEG_ENTRIES_SZ, expr=(cp.cur_data_blkoffh + cp.cur_data_blkoffw + cp.cur_data_blkoffc)*sizeof(struct f2fs_data_summary));

/* Sagar: substituted below define directive with spiffy DEFINE annotation
 *
 * #define CURSEG_ENTRIES_SZ \
    ( (cp.cur_data_blkoffh + cp.cur_data_blkoffw + cp.cur_data_blkoffc)*sizeof(struct f2fs_data_summary) ) */

FSCONST(type=flag) checkpoint_flag {
    CP_FASTBOOT_FLAG	= 0x00000020,
    CP_FSCK_FLAG		= 0x00000010,
    CP_ERROR_FLAG		= 0x00000008,
    CP_COMPACT_SUM_FLAG	= 0x00000004,
    CP_ORPHAN_PRESENT_FLAG	= 0x00000002,
    CP_UMOUNT_FLAG		= 0x00000001,
};

/*
 * In the victim_sel_policy->alloc_mode, there are two block allocation modes.
 * LFS writes data sequentially with cleaning operations.
 * SSR (Slack Space Recycle) reuses obsolete space without cleaning operations.
 */
//typedef FSCONST(type=enum) {
//	LFS = 0,
//	SSR
//} policy;
//
// Notes: log position, check w/ dump
// Disable heap-style segment allocation which finds free
// segments for data from the beginning of main area, while
// for node from the end of main area.
//
// Default Settings: Mount option set to config.heap = 1
// if (config.heap) {
//     config.cur_seg[CURSEG_HOT_NODE] = last_section(last_zone(total_zones));
//     config.cur_seg[CURSEG_WARM_NODE] = prev_zone(CURSEG_HOT_NODE);
//     config.cur_seg[CURSEG_COLD_NODE] = prev_zone(CURSEG_WARM_NODE);
//     config.cur_seg[CURSEG_HOT_DATA] = prev_zone(CURSEG_COLD_NODE);
//     config.cur_seg[CURSEG_COLD_DATA] = 0;
//     config.cur_seg[CURSEG_WARM_DATA] = next_zone(CURSEG_COLD_DATA);
// } 

FSSTRUCT() f2fs_crc
{
    __le32 crc;
};

FSSTRUCT(size=BLOCK_SIZE, rank=container) f2fs_checkpoint 
{
	__le64 checkpoint_ver;		/* checkpoint block version number */
	__le64 user_block_count;	/* # of user blocks */
	__le64 valid_block_count;	/* # of valid blocks in main area */
	__le32 rsvd_segment_count;	/* # of reserved segments for gc */
	__le32 overprov_segment_count;	/* # of overprovision segments */
	__le32 free_segment_count;	/* # of free segments in main area */

    /* 
     * This information provides the current position of each of the log.
     * This is recorded as segno, blkoffset. This will be used in the 
     * address translation code.
     */

	/* information of current node segments */
	__le32 cur_node_segno[MAX_ACTIVE_NODE_LOGS];
	__le16 cur_node_blkoff[MAX_ACTIVE_NODE_LOGS];

	/* information of current data segments */
	__le32 cur_data_segno[MAX_ACTIVE_DATA_LOGS];

    // Repurpose cur_data_blkoff
	__le16 cur_data_blkoffh;
	__le16 cur_data_blkoffw;
	__le16 cur_data_blkoffc;
	__le16 cur_data_blkoff[MAX_ACTIVE_DATA_LOGS - 3];
	
	FIELD(type=enum checkpoint_flag)
	__le32 ckpt_flags;		/* Flags : umount and journal_present */
	
	__le32 cp_pack_total_block_count;	/* total # of one cp pack */
	__le32 cp_pack_start_sum;	/* start block number of data summary */
	__le32 valid_node_count;	/* Total number of valid nodes */
	__le32 valid_inode_count;	/* Total number of valid inodes */
	__le32 next_free_nid;		/* Next free node number */
	__le32 sit_ver_bitmap_bytesize;	/* Default value 64 */
	__le32 nat_ver_bitmap_bytesize; /* Default value 256 */
	
	POINTER(repr=offset, type=struct f2fs_crc)
	__le32 checksum_offset;		/* checksum offset inside cp block */
	__le64 elapsed_time;		/* mounted time */
	
	/* allocation type of current segment */
    // FIELD(type=enum policy) based on alloc_type the number of blocks for the data summary could vary
    // within the checkpoint pack
	unsigned char alloc_type[MAX_ACTIVE_LOGS];
	//unsigned char sit_nat_version_bitmap[];
	
	/* sit bitmap is outside of f2fs_checkpoint block when cp_payload > 0 */
    VECTOR(name=sit_version_bitmap, type=bitmap, 
        size=(sb.cp_payload > 0) ?  0 : self.sit_ver_bitmap_bytesize);
    VECTOR(name=nat_version_bitmap, type=bitmap, size=self.nat_ver_bitmap_bytesize);
} __attribute__((packed));



//VECTOR(name=nat_bitmap, type=bitmap, size=BLOC);
/*
 * For orphan inode management
 */
#define F2FS_ORPHANS_PER_BLOCK	1020

FSSTRUCT(size=BLOCK_SIZE, rank=container) f2fs_orphan_block {
	__le32 ino[F2FS_ORPHANS_PER_BLOCK];	/* inode numbers */
	__le32 reserved;	/* reserved */
	__le16 blk_addr;	/* block index in current CP */
	__le16 blk_count;	/* Number of orphan inode blocks in CP */
	__le32 entry_count;	/* Total number of orphan nodes in current CP */
	__le32 check_sum;	/* CRC32 for orphan inode block */
} __attribute__((packed));

VECTOR(name=f2fs_orphan_blocks, type=struct f2fs_orphan_block,
       count=cp.cp_pack_start_sum-1);

/*
 * CheckPoint Region:
 *
 * After a checkpoint is written. We are guaranteed a consistent set of NAT mappings
 * if we check the summary block within the checkpoint region (HOT_DATA) or the NAT
 * blocks themselves. Depending on whether the CP_COMPACT_SUM_FLAG is set, we will have different
 * metadata on-disk layout.
 *
 * 1) Normal Summaries:
 *      - Vector of SSA Blocks = NR_CURSEG_DATA_TYPE + NR_CURSEG_NODE_TYPE
 *
 * 2) Compacted Summaries: CP_COMPACT_SUM_FLAG set when n < 3.
 *      - n := {1,2} Number of blocks required to fit curseg summary entries ONLY!.
 *      - Blocks = n + NR_CURSEG_NODE_TYPE
 *      - Block #1: nat_journal(sz=SUM_JOURNAL_SIZE), sit_journal(sz=SUM_JOURNAL_SIZE)
 *          where the nat_cache and sit_cache obtained from CURSEG_HOT/COLD_DATA respectively.
 *          remainder is filled with struct f2fs_summary entries.
 *      - Optionally block 2 and block 3 filled with summary entries.
 */

// TODO (jsun): leftover_summaries must be an object and not container
//VECTOR(name=leftover_summaries, type=struct f2fs_data_summary, 
//       size=MIN(CURSEG_ENTRIES_SZ, REMAINDER_ENTRIES_SZ));

FSSTRUCT(size=BLOCK_SIZE, rank=container) compact_block_header {
    struct f2fs_nat_journal nat_jrl;
    struct f2fs_sit_journal sit_jrl;
    __u8 padding[REMAINDER_ENTRIES_SZ];
    struct summary_footer footer;

	//FIELD(type=leftover_summaries, name=entries, expr=2*SUM_JOURNAL_SIZE);
} __attribute__((packed));

// Requires: alloc_type for all curseg == LFS rather than SSR (default)
VECTOR(name=compact_block_full, type=struct f2fs_data_summary,
        size=BLOCK_SIZE);
VECTOR(name=compact_block_partial, type=struct f2fs_data_summary,
        size=CURSEG_ENTRIES_SZ - REMAINDER_ENTRIES_SZ - BLOCK_SIZE);

/* For Normal Data Summary */
VECTOR(name=normal_data_summary_blocks, type=struct f2fs_summary_block, count=NR_CURSEG_DATA_TYPE);
/* General Node Summary */
VECTOR(name=node_summary_blocks, type=struct f2fs_summary_block, count=NR_CURSEG_NODE_TYPE);

VECTOR(name=sit_version_bitmap, type=bitmap, size=F2FS_BLKSIZE);

typedef FSSTRUCT(name=cp, base=struct f2fs_checkpoint, rank=container) {
        POINTER(repr=block, name=sit_ver_bitmap_blkaddr, when=sb.cp_payload > 0,
                expr=$self.addr + 1, type=sit_version_bitmap);

        /* Orphan Inode Blocks */
        POINTER(name=orphan, repr=block, type=f2fs_orphan_blocks,
                expr=$self.addr + 1 + sb.cp_payload, when=HAS_ORPHANS);

        /* Compact Data Summary Blocks */
        POINTER(name=compact_header, repr=block, type=struct compact_block_header, 
                expr=$self.addr + self.cp_pack_start_sum, when=IS_COMPACT_SUMM, size=BLOCK_SIZE);

        POINTER(name=compact2, repr=block, type=compact_block_full, 
                expr= COMPACT_FULL_START, when=CURSEG_ENTRIES_SZ > REMAINDER_ENTRIES_SZ)

        POINTER(name=compact3, repr=block, type=compact_block_partial, 
                expr= COMPACT_FULL_START + 1, 
                when=(CURSEG_ENTRIES_SZ > REMAINDER_ENTRIES_SZ) & (CURSEG_ENTRIES_SZ - REMAINDER_ENTRIES_SZ - BLOCK_SIZE > 0))

        /* Normal Data Summary Blocks */
        POINTER(name=normal_data_summaries, repr=block, type=normal_data_summary_blocks, 
                expr=$self.addr + self.cp_pack_start_sum, when=!(IS_COMPACT_SUMM));

        /* Node Summary Blocks */
        POINTER(name=node_summaries, repr=block, type=node_summary_blocks, 
                expr=NODE_SUMM_START_ADDR, when=HAS_NODE_SUMM);

        POINTER(name=footer, repr=block, type=struct f2fs_checkpoint, 
                expr=$self.addr + self.cp_pack_total_block_count - 1);
} f2fs_checkpoint_header;

#endif /* F2FS_CP_H */
