/*
 * btrfs_btree.h
 *
 * annotated btree-related data structures in btrfs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 */

#ifndef BTRFS_BTREE_H
#define BTRFS_BTREE_H

#include "btrfs_key.h"

/*
 * every tree block (leaf or node) starts with this header.
 */
FSSTRUCT() btrfs_header {
	/* these first four must match the super block */
	u8 csum[BTRFS_CSUM_SIZE];
	
	FIELD(type=uuid)
	u8 fsid[BTRFS_FSID_SIZE]; /* FS specific uuid */
	
	__le64 bytenr; /* which block this node is supposed to live in */
	__le64 flags;

	/* allowed to be different from the super from here on down */
	FIELD(type=uuid)
	u8 chunk_tree_uuid[BTRFS_UUID_SIZE];
	__le64 generation;
	__le64 owner;
	__le32 nritems;
	u8 level;
} __attribute__ ((__packed__));

/* 
 * (jsun): the rest of these items are either unsure or unnecessary
 *
        BTRFS_ORPHAN_ITEM_KEY has no associated object      
        BTRFS_CSUM_ITEM_KEY
	    BTRFS_EXTENT_CSUM_KEY          	  
        BTRFS_FREE_SPACE_EXTENT_KEY
        BTRFS_FREE_SPACE_BITMAP_KEY
        BTRFS_BALANCE_ITEM_KEY
        BTRFS_QGROUP_RELATION_KEY
        BTRFS_DEV_STATS_KEY
        BTRFS_DEV_REPLACE_KEY
        BTRFS_UUID_KEY_SUBVOL
        BTRFS_UUID_KEY_RECEIVED_SUBVOL
        BTRFS_STRING_ITEM_KEY
*/

FSSTRUCT() btrfs_item {
	struct btrfs_disk_key key;
	__le32 offset;
	__le32 size;
	
	// btrfs_extent.h
	POINTER(name=item, repr=offset, type=struct btrfs_block_group_item,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_BLOCK_GROUP_ITEM_KEY)
	POINTER(name=item, repr=offset, type=struct btrfs_file_extent_item,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_EXTENT_DATA_KEY)
	POINTER(name=item, repr=offset, type=struct btrfs_extent_item,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_EXTENT_ITEM_KEY ||
	             self.key.type == BTRFS_METADATA_ITEM_KEY)
	        
	// btrfs_inode.h
	POINTER(name=item, repr=offset, type=struct btrfs_inode_item, 
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
            when=self.key.type == BTRFS_INODE_ITEM_KEY);
    POINTER(name=item, repr=offset, type=struct btrfs_inode_ref,
            size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_INODE_REF_KEY);
	POINTER(name=item, repr=offset, type=struct btrfs_inode_extref,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_INODE_EXTREF_KEY); 
	        
	// btrfs_root.h        
	POINTER(name=item, repr=offset, type=struct btrfs_root_item,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_ROOT_ITEM_KEY);
	POINTER(name=item, repr=offset, type=struct btrfs_root_ref,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_ROOT_BACKREF_KEY ||
	             self.key.type == BTRFS_ROOT_REF_KEY)   
	       
	// btrfs_chunk.h
	POINTER(name=item, repr=offset, type=struct btrfs_dev_item,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_DEV_ITEM_KEY)
	POINTER(name=item, repr=offset, type=struct btrfs_chunk,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_CHUNK_ITEM_KEY)
	POINTER(name=item, repr=offset, type=struct btrfs_dev_extent, 
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_DEV_EXTENT_KEY)        
	
	// btrfs_dir.h        
	POINTER(name=item, repr=offset, type=struct btrfs_dir_log_item,
	        size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_DIR_LOG_ITEM_KEY ||
                 self.key.type == BTRFS_DIR_LOG_INDEX_KEY)
    POINTER(name=item, repr=offset, type=struct btrfs_dir_item,
            size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_DIR_ITEM_KEY ||
                 self.key.type == BTRFS_DIR_INDEX_KEY ||
                 self.key.type == BTRFS_XATTR_ITEM_KEY )
               
    // btrfs_item.h
    POINTER(name=item, repr=offset, type=struct btrfs_qgroup_status_item,
            size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
            when=self.key.type == BTRFS_QGROUP_STATUS_KEY)
    POINTER(name=item, repr=offset, type=struct btrfs_qgroup_info_item,
            size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
            when=self.key.type == BTRFS_QGROUP_INFO_KEY)
    POINTER(name=item, repr=offset, type=struct btrfs_qgroup_limit_item,
            size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
            when=self.key.type == BTRFS_QGROUP_LIMIT_KEY)
    POINTER(name=item, repr=offset, type=struct btrfs_free_space_info,
            size=self.size, expr=sizeof(struct btrfs_header)+self.offset,
	        when=self.key.type == BTRFS_FREE_SPACE_INFO_KEY)       
} __attribute__ ((__packed__));

FSSTRUCT(size=sb.leafsize, rank=container) btrfs_leaf {
    struct btrfs_header header;
    FIELD(count=self.header.nritems)
	struct btrfs_item items[];
	
} __attribute__ ((__packed__));

/*
 * all non-leaf blocks are nodes, they hold only keys and pointers to
 * other blocks
 */
FSSTRUCT() btrfs_key_ptr {
	struct btrfs_disk_key key;
	POINTER(repr=byte, type=struct btrfs_leaf, when=node.header.level == 1)
    POINTER(repr=byte, type=struct btrfs_node, when=node.header.level > 1) 
	__le64 blockptr;
	__le64 generation;
} __attribute__ ((__packed__));

FSSTRUCT(name=node, size=sb.nodesize, rank=container) btrfs_node {
    struct btrfs_header header;
    FIELD(count=self.header.nritems)
	struct btrfs_key_ptr ptrs[];
	
} __attribute__ ((__packed__));

#endif /* BTRFS_BTREE_H */

