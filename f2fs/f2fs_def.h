/*
 * f2fs_def.h
 *
 * commonly used macro definitions in f2fs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */
 
#ifndef F2FS_DEF_H
#define F2FS_DEF_H

#include <jd.h>

#ifdef __KERNEL__
#include <usercompat.h>
#else
#include <kerncompat.h>
#endif

#define F2FS_BLKSIZE		4096	/* support only 4KB block */
#define PAGE_SIZE           4096
#define PAGE_CACHE_SIZE		4096
#define BITS_PER_BYTE		8

DEFINE(name=BLOCK_SIZE, expr=1 << sb.log_blocksize);
DEFINE(name=SEGMENT_SIZE, 
       expr=(1 << sb.log_blocks_per_seg)*(1 << sb.log_blocksize));
DEFINE(name=NUM_SEGMENTS, 
       expr=sb.block_count/(1 << sb.log_blocks_per_seg));

#ifndef ceil
#define ceil(x, y) (((y) != 0) ? (((x) + (y) - 1) / (y)) : 0)
#endif

/*
typedef FSCONST(type=enum) {
	NAT_JOURNAL = 0,
	SIT_JOURNAL
} journal_type;
*/

/*
 * For segment summary
 *
 * One summary block contains exactly 512 summary entries, which represents
 * exactly 2MB segment by default. Not allow to change the basic units.
 *
 * NOTE: For initializing fields, you must use set_summary
 *
 * - If data page, nid represents dnode's nid
 * - If node page, nid represents the node page's nid.
 *
 * The ofs_in_node is used by only data page. It represents offset
 * from node's page's beginning to get a data block address.
 * ex) data_blkaddr = (block_t)(nodepage_start_address + ofs_in_node)
 */

#define ENTRIES_IN_SUM		512
#define	SUMMARY_SIZE		(7)	/* sizeof(struct summary) */
#define	SUM_FOOTER_SIZE		(5)	/* sizeof(struct summary_footer) */
#define SUM_ENTRIES_SIZE	(SUMMARY_SIZE * ENTRIES_IN_SUM)
#define SUM_JOURNAL_SIZE	(F2FS_BLKSIZE - SUM_FOOTER_SIZE -\
            SUM_ENTRIES_SIZE)

#endif /* F2FS_DEF_H */
 
