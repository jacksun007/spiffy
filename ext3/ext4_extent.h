/*
 * ext4_extent.h
 *
 * Annotated ext4 extent tree
 *
 * Copyright (C) 2017
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 */

#ifndef EXT4_EXTENT_H
#define EXT4_EXTENT_H

/*
VECTOR(name=data_extent, type=data_block);
VECTOR(name=dir_extent, type=dir_block);
*/

/*
 * this is extent on-disk structure
 * it's used at the bottom of the tree
 */
FSSTRUCT() ext4_extent {
	__u32	ee_block;	/* first logical block extent covers */
	__u16	ee_len;		/* number of blocks covered by extent */
	__u16	ee_start_hi;	/* high 16 bits of physical block */
	
	// TODO: (jsun) not currently supported
	/*
	POINTER(repr=block, type=data_extent, count=self.ee_len,
	        when=in.i_mode & EXT3_S_IFREG)
	POINTER(repr=block, type=dir_extent, count=self.ee_len,
	        when=in.i_mode & EXT3_S_IFDIR) 
	*/     
	__u32	ee_start;	/* low 32 bigs of physical block */
};

/*
 * this is index on-disk structure
 * it's used at all the levels, but the bottom
 */
FSSTRUCT() ext4_extent_idx {
	__u32	ei_block;	/* index covers logical blocks from 'block' */
	
	POINTER(repr=block, type=struct ext4_extent_node,
	        when=$ctn.eb_hdr.eh_depth > 1)
	POINTER(repr=block, type=struct ext4_extent_leaf,
	        when=$ctn.eb_hdr.eh_depth == 1)        
	__u32	ei_leaf;	/* pointer to the physical block of the next *
				         * level. leaf or next index could be there */
	__u16	ei_leaf_hi;	/* high 16 bits of physical block */
	__u16	ei_unused;
};

/*
 * each block (leaves and indexes), even inode-stored has header
 */
FSSTRUCT() ext4_extent_header {
	__u16	eh_magic;	/* probably will support different formats */
	__u16	eh_entries;	/* number of valid entries */
	__u16	eh_max;		/* capacity of store in entries */
	__u16	eh_depth;	/* has tree real underlaying blocks? */
	__u32	eh_generation;	/* generation of the tree */
	
	CHECK(expr=self.eh_magic == EXT4_EXT_MAGIC);
};

FSSTRUCT(size=BLOCK_SIZE, rank=container) ext4_extent_node {
    struct ext4_extent_header eb_hdr;
    
    FIELD(count=self.eb_hdr.eh_entries)
    struct ext4_extent_idx eb_idx[];
};

FSSTRUCT(size=BLOCK_SIZE, rank=container) ext4_extent_leaf {
    struct ext4_extent_header eb_hdr;
    
    FIELD(count=self.eb_hdr.eh_entries)
    struct ext4_extent eb_extent[];
};

#define EXT4_EXT_MAGIC		0xf30a

FSSTRUCT() ext4_inline_extent {
    __u16	eh_magic;	/* probably will support different formats */
	__u16	eh_entries;	/* number of valid entries */
	__u16	eh_max;		/* capacity of store in entries */
	__u16	eh_depth;	/* has tree real underlaying blocks? */
	__u32	eh_generation;	/* generation of the tree */
	
	/* copied from ext4_extent_idx */
    __u32	ei_block;	/* index covers logical blocks from 'block' */
	
	POINTER(repr=block, type=struct ext4_extent_node,
	        when=self.eh_entries > 0 && self.eh_depth > 1)
	POINTER(repr=block, type=struct ext4_extent_leaf,
	        when=self.eh_entries > 0 && self.eh_depth == 1)        
	__u32	ei_leaf;	
				      
    /* don't care from this point forward, custom processed for now */
    /* TODO: fix for future */
};

#endif /* EXT4_EXTENT_H */

