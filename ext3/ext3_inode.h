/*
 * Copyright (C) 2014
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Annotated ext3 inode
 */

#ifndef EXT3_INODE_H
#define EXT3_INODE_H

#include <jd.h>
#include "ext4_extent.h"

#define EXT4_GOOD_OLD_INODE_SIZE 128

/*
 * Special inodes numbers
 */
#define EXT3_BAD_INO         1	/* Bad blocks inode */
#define EXT3_ROOT_INO        2	/* Root inode */
#define EXT3_BOOT_LOADER_INO 5	/* Boot loader inode */
#define EXT3_UNDEL_DIR_INO   6	/* Undelete directory inode */
#define EXT3_RESIZE_INO      7	/* Reserved group descriptors inode */
#define EXT3_JOURNAL_INO     8	/* Journal inode */

/* First non-reserved inode for old ext3 filesystems */
#define EXT3_GOOD_OLD_FIRST_INO 11

/*
 * Constants relative to the data blocks
 */
#define EXT3_NDIR_BLOCKS 12
#define EXT3_IND_BLOCK   EXT3_NDIR_BLOCKS
#define	EXT3_DIND_BLOCK	 (EXT3_IND_BLOCK + 1)
#define	EXT3_TIND_BLOCK	 (EXT3_DIND_BLOCK + 1)
#define	EXT3_N_BLOCKS	 (EXT3_TIND_BLOCK + 1)
#define EXT3_MAX_INLINE (EXT3_N_BLOCKS * sizeof(__le32))

FSCONST(type=flag) ext3_inode_mode {
    EXT3_S_IFSOCK = 0xC000,	/* socket */
    EXT3_S_IFLNK  = 0xA000,	/* symbolic link */
    EXT3_S_IFREG  = 0x8000,	/* regular file */
    EXT3_S_IFBLK  = 0x6000,	/* block device */
    EXT3_S_IFDIR  = 0x4000,	/* directory */
    EXT3_S_IFCHR  = 0x2000,	/* character device */
    EXT3_S_IFIFO  = 0x1000,	/* fifo */
    EXT3_S_ISUID  = 0x0800,	/* set process user id */
    EXT3_S_ISGID  = 0x0400,	/* set process group id */
    EXT3_S_ISVTX  = 0x0200,	/* sticky bit */
    EXT3_S_IRUSR  = 0x0100,	/* user read */
    EXT3_S_IWUSR  = 0x0080,	/* user write */
    EXT3_S_IXUSR  = 0x0040,	/* user execute */
    EXT3_S_IRGRP  = 0x0020,	/* group read */
    EXT3_S_IWGRP  = 0x0010,	/* group write */
    EXT3_S_IXGRP  = 0x0008,	/* group execute */
    EXT3_S_IROTH  = 0x0004,	/* others read */
    EXT3_S_IWOTH  = 0x0002,	/* others write */
    EXT3_S_IXOTH  = 0x0001,	/* others execute */
};

FSCONST(type=flag) ext3_inode_flag {
    EXT3_SECRM_FL        = 0x00000001, /* Secure deletion */
    EXT3_UNRM_FL         = 0x00000002, /* Undelete */
    EXT3_COMPR_FL        = 0x00000004, /* Compress file */
    EXT3_SYNC_FL         = 0x00000008, /* Synchronous updates */
    EXT3_IMMUTABLE_FL    = 0x00000010, /* Immutable file */
    EXT3_APPEND_FL       = 0x00000020, /* writes to file may only append */
    EXT3_NODUMP_FL       = 0x00000040, /* do not dump file */
    EXT3_NOATIME_FL      = 0x00000080, /* do not update atime */
    /* Reserved for compression usage... */
    EXT3_DIRTY_FL        = 0x00000100,
    EXT3_COMPRBLK_FL     = 0x00000200, /* One or more compressed clusters */
    EXT3_NOCOMPR_FL      = 0x00000400, /* Don't compress */
    EXT3_ECOMPR_FL       = 0x00000800, /* Compression error */
    /* End compression flags --- maybe not all used */
    EXT3_INDEX_FL        = 0x00001000, /* hash-indexed directory */
    EXT3_IMAGIC_FL       = 0x00002000, /* AFS directory */
    EXT3_JOURNAL_DATA_FL = 0x00004000, /* file data should be journaled */
    EXT3_NOTAIL_FL       = 0x00008000, /* file tail should not be merged */
    EXT3_DIRSYNC_FL      = 0x00010000, /* dirsync behaviour (directories only) */
    EXT3_TOPDIR_FL       = 0x00020000, /* Top of directory hierarchies*/
    EXT4_HUGE_FILE_FL    = 0x00040000, /* Set to each huge file */
    EXT4_EXTENTS_FL 	 = 0x00080000, /* Inode uses extents */
    EXT4_EA_INODE_FL	 = 0x00200000, /* Inode used for large EA */
/* EXT4_EOFBLOCKS_FL 0x00400000 was here */
    FS_NOCOW_FL			 = 0x00800000, /* Do not cow file */
    EXT4_SNAPFILE_FL     = 0x01000000, /* Inode is a snapshot */
    EXT4_SNAPFILE_DELETED_FL = 0x04000000, /* Snapshot is being deleted */
    EXT4_SNAPFILE_SHRUNK_FL = 0x08000000,  /* Snapshot shrink has completed */       
    EXT3_RESERVED_FL     = 0x80000000, /* reserved for ext3 lib */
};

VECTOR( name=ext3_inode_block, type=struct ext3_inode, size=BLOCK_SIZE );

FSSTRUCT( name=in, size=sb.s_inode_size) ext3_inode {
        FIELD(type=enum ext3_inode_mode)
        __le16	i_mode;		/* File mode */
        
        __le16	i_uid;		/* Low 16 bits of Owner Uid */
        __le32	i_size;		/* Size in bytes */
        
        FIELD(type=timestamp)
        __le32	i_atime;	/* Access time */
        
        FIELD(type=timestamp)
        __le32	i_ctime;	/* Creation time */
        
        FIELD(type=timestamp)
        __le32	i_mtime;	/* Modification time */
        
        __le32	i_dtime;	/* Deletion Time */
        __le16	i_gid;		/* Low 16 bits of Group Id */
        __le16	i_links_count;	/* Links count */
        __le32	i_blocks;	/* Blocks count */
        
        FIELD(type=enum ext3_inode_flag)
        __le32	i_flags;	/* File flags */
        
        union {
                struct {
                        __u32  l_i_reserved1;
                } linux1;
                struct {
                        __u32  h_i_translator;
                } hurd1;
                struct {
                        __u32  m_i_reserved1;
                } masix1;
        } osd1;		        /* OS dependent 1 */
        
        union {
            FIELD(when=!(self.i_flags & EXT4_EXTENT_FL))
            struct {
                POINTER(repr=block, type=dir_block, 
                        when=self.i_mode & EXT3_S_IFDIR)
                POINTER(repr=block, type=data_block, 
                        when=self.i_mode & EXT3_S_IFREG)        
                __le32	dir[EXT3_NDIR_BLOCKS];
                
                POINTER(repr=block, type=dir_indirect_block, 
                        when=self.i_mode & EXT3_S_IFDIR)
                POINTER(repr=block, type=data_indirect_block, 
                        when=self.i_mode & EXT3_S_IFREG)
                __le32	ind;
                
                POINTER(repr=block, type=dir_x2_indirect_block, 
                        when=self.i_mode & EXT3_S_IFDIR)
                POINTER(repr=block, type=data_x2_indirect_block,
                        when=self.i_mode & EXT3_S_IFREG)
                __le32	dind;
                
                POINTER(repr=block, type=dir_x3_indirect_block,
                        when=self.i_mode & EXT3_S_IFDIR)
                POINTER(repr=block, type=data_x3_indirect_block,
                        when=self.i_mode & EXT3_S_IFREG)
                __le32	tind;
            } block;
            
            /* fast symlink -- TODO: check incompat flag in super block */
            FIELD(when=(self.i_mode & EXT3_S_IFLNK) && (self.i_size < EXT3_MAX_INLINE))
            char data[EXT3_MAX_INLINE];
        } i;
                
        // original definition
        // __le32  i_block[EXT3_N_BLOCKS]; /* Pointers to blocks */
        
        __le32	i_generation;	/* File version (for NFS) */
        __le32	i_file_acl;	/* File ACL */
        __le32	i_dir_acl;	/* Directory ACL */
        __le32	i_faddr;	/* Fragment address */
        
        union {
                struct {
                        __u8	l_i_frag;	/* Fragment number */
                        __u8	l_i_fsize;	/* Fragment size */
                        __u16	i_pad1;
                        __le16	l_i_uid_high;	/* these 2 fields    */
                        __le16	l_i_gid_high;	/* were reserved2[0] */
                        __u32	l_i_reserved2;
                } linux2;
                struct {
                        __u8	h_i_frag;	/* Fragment number */
                        __u8	h_i_fsize;	/* Fragment size */
                        __u16	h_i_mode_high;
                        __u16	h_i_uid_high;
                        __u16	h_i_gid_high;
                        __u32	h_i_author;
                } hurd2;
                struct {
                        __u8	m_i_frag;	/* Fragment number */
                        __u8	m_i_fsize;	/* Fragment size */
                        __u16	m_pad1;
                        __u32	m_i_reserved2[2];
                } masix2;
        } osd2;				/* OS dependent 2 */
};

/* TODO: (jsun) don't care for now. leave out for performance */
/*
FSSTRUCT(base=struct ext3_inode, size=sb.s_inode_size,
         when=sb.s_inode_size > EXT4_GOOD_OLD_INODE_SIZE) ext3_inode_lg {
        __le16	i_extra_isize;
        __u16	i_checksum_hi;	 
        __u32	i_ctime_extra; 
        __u32	i_mtime_extra;	 
        __u32	i_atime_extra;	 
        __u32	i_crtime;	   
        __u32	i_crtime_extra;	 
        __u32	i_version_hi;	 
};
*/

#endif


