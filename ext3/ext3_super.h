/*
 * Copyright (C) 2014
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Annotated ext3 super block
 */

#ifndef EXT3_SUPER_H
#define EXT3_SUPER_H

#ifdef __KERNEL__
#include <usercompat.h>
#else
#include <kerncompat.h>
#endif
#include <jd.h>

/* define address spaces in this file system */
ADDRSPACE(name=block, null=0);

// file address space
ADDRSPACE(name=file, null=0);

FSCONST(type=flag) ext3_feature_compat {
	EXT3_FEATURE_COMPAT_DIR_PREALLOC = 0x0001,	/* block prealloc */
	EXT3_FEATURE_COMPAT_MAGIC_INODES = 0x0002,
	EXT3_FEATURE_COMPAT_HAS_JOURNAL  = 0x0004,	/* ext3/4 journal */
	EXT3_FEATURE_COMPAT_EXT_ATTR     = 0x0008,	/* extended inode attr */
	EXT3_FEATURE_COMPAT_RESIZE_INO   = 0x0010,	/* non-standard ino size */
	EXT3_FEATURE_COMPAT_DIR_INDEX    = 0x0020,	/* h-tree dir indexing */
};

FSCONST(type=flag) ext3_feature_incompat {
	EXT3_FEATURE_INCOMPAT_COMPRESSION = 0x0001,	/* disk compression */
	EXT3_FEATURE_INCOMPAT_FILETYPE    = 0x0002,
	EXT3_FEATURE_INCOMPAT_RECOVER     = 0x0004,
	EXT3_FEATURE_INCOMPAT_JOURNAL_DEV = 0x0008,
	EXT3_FEATURE_INCOMPAT_META_BG     = 0x0010,
};

FSCONST(type=flag) ext3_feature_ro_compat {
	EXT3_FEATURE_RO_COMPAT_SPARSE_SUPER = 0x0001,	/* sparse superblock */
	EXT3_FEATURE_RO_COMPAT_LARGE_FILE   = 0x0002,	/* 64-bit filesize */
	EXT3_FEATURE_RO_COMPAT_BTREE_DIR    = 0x0004,	/* binary tree sorted dir */
};

FSCONST(type=enum) ext3_os {
    EXT3_OS_LINUX	= 0,
    EXT3_OS_HURD	= 1,
    EXT3_OS_MASIX	= 2,
    EXT3_OS_FREEBSD	= 3,
    EXT3_OS_LITES	= 4,
};

FSCONST(type=enum) ext3_super_magic {
    EXT3_SUPER_MAGIC = 0xef53,
};

FSSUPER( name=sb, location=1024 ) ext3_super_block {
        
/*00*/  __le32	s_inodes_count;		/* Inodes count */
        __le32	s_blocks_count;		/* Blocks count */
        __le32	s_r_blocks_count;	/* Reserved blocks count */
        __le32	s_free_blocks_count;	/* Free blocks count */
/*10*/	__le32	s_free_inodes_count;	/* Free inodes count */
        __le32	s_first_data_block;	/* First Data Block */
        __le32	s_log_block_size;	        /* Block size */
        __le32	s_log_frag_size;	        /* Fragment size */
/*20*/  __le32	s_blocks_per_group;	/* # Blocks per group */
        __le32	s_frags_per_group;	/* # Fragments per group */
        __le32	s_inodes_per_group;	/* # Inodes per group */
        
        FIELD(type=timestamp)
        __le32	s_mtime;		        /* Mount time */
        
        FIELD(type=timestamp)
/*30*/  __le32	s_wtime;		        /* Write time */
        __le16	s_mnt_count;		/* Mount count */
        __le16	s_max_mnt_count;	/* Maximal mount count */
        
        FIELD(type=enum ext3_super_magic)
        __le16	s_magic;		        /* Magic signature */
        __le16	s_state;		        /* File system state */
        __le16	s_errors;		        /* Behaviour when detecting errors */
        __le16	s_minor_rev_level;	/* minor revision level */
        
        FIELD(type=timestamp)
/*40*/  __le32	s_lastcheck;		/* time of last check */
        __le32	s_checkinterval;	        /* max. time between checks */
        
        FIELD(type=enum ext3_os)
        __le32	s_creator_os;		/* OS */
        __le32	s_rev_level;		/* Revision level */
/*50*/  __le16	s_def_resuid;		/* Default uid for reserved blocks */
        __le16	s_def_resgid;		/* Default gid for reserved blocks */
        
        /*
         * These fields are for EXT3_DYNAMIC_REV superblocks only.
         *
         * Note: the difference between the compatible feature set and
         * the incompatible feature set is that if there is a bit set
         * in the incompatible feature set that the kernel doesn't
         * know about, it should refuse to mount the filesystem.
         *
         * e2fsck's requirements are more strict; if it doesn't know
         * about a feature in either the compatible or incompatible
         * feature set, it must abort and not try to meddle with
         * things it doesn't understand...
         */
        __le32	s_first_ino;		      /* First non-reserved inode */
        __le16  s_inode_size;		      /* size of inode structure */
        __le16	s_block_group_nr;	      /* block group # of this superblock */
        
        FIELD(type=enum ext3_feature_compat)
        __le32	s_feature_compat;	      /* compatible feature set */
        
        FIELD(type=enum ext3_feature_incompat)
/*60*/  __le32	s_feature_incompat;	      /* incompatible feature set */

        FIELD(type=enum ext3_feature_ro_compat)
        __le32	s_feature_ro_compat;	  /* readonly-compatible feature set */
        
        FIELD(type=uuid)
/*68*/  __u8	s_uuid[16];		          /* 128-bit uuid for volume */
/*78*/  char	s_volume_name[16];	      /* volume name */
/*88*/  char	s_last_mounted[64];	      /* directory where last mounted */
/*C8*/  __le32	s_algorithm_usage_bitmap; /* For compression */
        
        /*
         * Performance hints.  Directory preallocation should only
         * happen if the EXT3_FEATURE_COMPAT_DIR_PREALLOC flag is on.
         */
        __u8	s_prealloc_blocks;	      /* Nr of blocks to try to preallocate*/
        __u8	s_prealloc_dir_blocks;	  /* Nr to preallocate for dirs */
        __le16	s_reserved_gdt_blocks;	  /* Per group desc for online growth */
      
        /*
         * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
         */
        FIELD(type=uuid)
/*D0*/  __u8	s_journal_uuid[16];	  /* uuid of journal superblock */

        // TODO: only when s_journal_uuid is null
        POINTER(repr=file, type=ext3_journal)
/*E0*/  __le32	s_journal_inum;		  /* inode number of journal file */
        __le32	s_journal_dev;		  /* device number of journal file */
        
        //POINTER(repr=file, type=struct ext3_inode, alias=true)
        __le32	s_last_orphan;		  /* start of list of inodes to delete */
        
        __le32	s_hash_seed[4];		  /* HTREE hash seed */
        __u8	s_def_hash_version;	  /* Default hash version to use */
        __u8	s_reserved_char_pad;
        __u16	s_reserved_word_pad;
        __le32	s_default_mount_opts;
        
        // TODO: this field causes kernel to run out of 64-byte memory chunks
        //       during parsing...
        // __u32	s_reserved[190];	  /* Padding to the end of the block */
        
        /* implicit pointers */
        POINTER(name=s_block_group_desc, repr=block, 
                type=ext3_group_desc_table, 
                expr=(self.s_log_block_size == 0) ? 2 : 1);
        
        /* maximum supported block size is 64KB */
        CHECK(expr=self.s_log_block_size <= 6);
};


#endif /* EXT3_SUPER_H */

