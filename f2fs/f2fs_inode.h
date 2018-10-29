/**
 * f2fs_inode.h
 *
 * Annotations for Node Structures in the Main Area.
 *  F2FS classifies a node as: inode, direct node, indirect nodes
 *  TODO: 
 *      - extend for xattr; gets a bit more complicated because xattr
 *        takes first set of data_blocks/dir_blocks from inode and file size
 *        also a consideration here as well.
 *      - extend for inline data; currently, disk image mounted without the inline option
 *
 * Author: Joseph Chu, Kuei (Jack) Sun
 * Email: joseph.chu@mail.utoronto.ca, kuei.sun@utoronto.ca
 *
 * Copyright (C) 2016
 * University of Toronto
 *
 **/

#ifndef F2FS_INODE_H
#define F2FS_INODE_H

#include <jd.h>

#include "f2fs_def.h"
#include "f2fs_data.h"

#define ADDRS_PER_BLOCK         1018  /* Address Pointers in a Direct Block */
#define NIDS_PER_BLOCK          1018  /* Node IDs in an Indirect Block */
#define F2FS_NAME_LEN           255
#define F2FS_INLINE_XATTR_ADDRS 50	  /* 200 bytes for inline xattrs */
#define DEF_ADDRS_PER_INODE     923	  /* Address Pointers in an Inode */

typedef FSCONST(type=enum) {
	COLD_BIT_SHIFT = 0,
	FSYNC_BIT_SHIFT = 1,
	DENT_BIT_SHIFT = 2,
	OFFSET_BIT_SHIFT = 3,
} node_footer_flag;

FSCONST(type=flag) f2fs_inline_flag {
    F2FS_INLINE_XATTR  = 0x01,	/* file inline xattr flag */
    F2FS_INLINE_DATA   = 0x02,	/* file inline data flag */
    F2FS_INLINE_DENTRY = 0x04,	/* file inline dentry flag */
    F2FS_DATA_EXIST	   = 0x08,	/* file inline data exist flag */
    F2FS_INLINE_DOTS   = 0x10,	/* file having implicit dot dentries */
};

/* this type should be shared between VFS file systems */
FSCONST(type=flag) f2fs_inode_mode {
    F2FS_S_IFSOCK = 0xC000,	/* socket */
    F2FS_S_IFLNK  = 0xA000,	/* symbolic link */
    F2FS_S_IFREG  = 0x8000,	/* regular file */
    F2FS_S_IFBLK  = 0x6000,	/* block device */
    F2FS_S_IFDIR  = 0x4000,	/* directory */
    F2FS_S_IFCHR  = 0x2000,	/* character device */
    F2FS_S_IFIFO  = 0x1000,	/* fifo */
    F2FS_S_ISUID  = 0x0800,	/* set process user id */
    F2FS_S_ISGID  = 0x0400,	/* set process group id */
    F2FS_S_ISVTX  = 0x0200,	/* sticky bit */
    F2FS_S_IRUSR  = 0x0100,	/* user read */
    F2FS_S_IWUSR  = 0x0080,	/* user write */
    F2FS_S_IXUSR  = 0x0040,	/* user execute */
    F2FS_S_IRGRP  = 0x0020,	/* group read */
    F2FS_S_IWGRP  = 0x0010,	/* group write */
    F2FS_S_IXGRP  = 0x0008,	/* group execute */
    F2FS_S_IROTH  = 0x0004,	/* others read */
    F2FS_S_IWOTH  = 0x0002,	/* others write */
    F2FS_S_IXOTH  = 0x0001,	/* others execute */
};

// Not currently used -- added for completeness
FSCONST(type=flag) f2fs_inode_flag {
    FS_SECRM_FL =        0x00000001,	/* Secure deletion */ 
    FS_UNRM_FL =         0x00000002,	/* Undelete */ 
    FS_COMPR_FL =        0x00000004,	/* Compress file */ 
    FS_SYNC_FL =         0x00000008,	/* Synchronous updates */ 
    FS_IMMUTABLE_FL =    0x00000010,	/* Immutable file */ 
    FS_APPEND_FL =       0x00000020,	/* writes to file may only append */ 
    FS_NODUMP_FL =       0x00000040,	/* do not dump file */ 
    FS_NOATIME_FL =      0x00000080,	/* do not update atime */ 
///* Reserved for compression usage... */
    FS_DIRTY_FL =        0x00000100,	
    FS_COMPRBLK_FL =     0x00000200, /* One or more compressed clusters */ 
    FS_NOCOMP_FL =       0x00000400,	/* Don't compress */ 
    FS_ECOMPR_FL =       0x00000800,	/* Compression error */ 
///* End compression flags --- maybe not all used */
    FS_INDEX_FL =        0x00001000,	/* hash-indexed directory */ 
    FS_IMAGIC_FL =       0x00002000,	/* AFS directory */ 
    FS_JOURNAL_DATA_FL = 0x00004000,	/* Reserved for ext3 */ 
    FS_NOTAIL_FL =       0x00008000,	/* file tail should not be merged */ 
    FS_DIRSYNC_FL =      0x00010000,	/* dirsync behaviour (directories only) */ 
    FS_TOPDIR_FL =       0x00020000,	/* Top of directory hierarchies*/ 
    FS_EXTENT_FL =       0x00080000,	/* Extents */ 
    FS_DIRECTIO_FL =     0x00100000,	/* Use direct i/o */ 
    FS_NOCOW_FL =        0x00800000,	/* Do not cow file */ 
    FS_RESERVED_FL =     0x80000000,	/* reserved for ext2 lib */ 
};

FSSTRUCT() f2fs_extent {
	__le32 fofs;		/* start file offset of the extent */
	__le32 blk_addr;	/* start block address of the extent */
	__le32 len;		/* lengh of the extent */

    //POINTER(repr=block, type=f2fs_data_block, when=in.i_mode & S_IFREG)
    //POINTER(repr=block, type=f2fs_dir_block,  when=in.i_mode & S_IFDIR)
} __attribute__((packed));

// location=$self.id >= sb.main_blkaddr
FSSTRUCT(/* name=in */) f2fs_inode {
    FIELD(type=enum f2fs_inode_mode)
	__le16 i_mode;			/* file mode */
	__u8 i_advise;			/* file hints */

    FIELD(type=enum f2fs_inline_flag)
	__u8 i_inline;			/* file inline flags */
	__le32 i_uid;			/* user ID */
	__le32 i_gid;			/* group ID */
	__le32 i_links;			/* links count */
	__le64 i_size;			/* file size in bytes */
	__le64 i_blocks;		/* file size in blocks */

    FIELD(type=timestamp)
	__le64 i_atime;			/* access time */

    FIELD(type=timestamp)
	__le64 i_ctime;			/* change time */

    FIELD(type=timestamp)
	__le64 i_mtime;			/* modification time */

    /* (jsun): these nsec fields are not valid unix timestamps */
	__le32 i_atime_nsec;		/* access time in nano scale */
	__le32 i_ctime_nsec;		/* change time in nano scale */
	__le32 i_mtime_nsec;		/* modification time in nano scale */

	__le32 i_generation;		/* file version (for NFS) */

	__le32 i_current_depth;		/* only for directory depth */
	__le32 i_xattr_nid;		/* nid to save xattr */

    FIELD(type=enum f2fs_inode_flag)
	__le32 i_flags;			/* file attributes */
	__le32 i_pino;			/* parent inode number */
	__le32 i_namelen;		/* file name length */

    /* __u8 to char */
	char i_name[F2FS_NAME_LEN];	/* file name for SPOR */
	__u8 i_reserved2;		    /* for backward compatibility */
	struct f2fs_extent i_ext;	/* caching a largest extent */

    /* Context Sensitive Pointers to Data vs. Directory Blocks*/
    POINTER(repr=block, type=f2fs_data_block, when=self.i_mode & F2FS_S_IFREG)
    POINTER(repr=block, type=struct f2fs_dentry_block, when=self.i_mode & F2FS_S_IFDIR)
	__le32 i_addr[DEF_ADDRS_PER_INODE];	/* Pointers to data blocks */

    /* (jsun) (jchu) TODO: restore once nid address space is working again */ 
    //POINTER(repr=nid, type=data_direct_node_block, when=((self.i_mode & S_IFMT) == S_IFREG))
    //POINTER(repr=nid, type=dentry_direct_node_block, when=(self.i_mode & S_IFMT) == S_IFDIR)
	__le32 nid_direct[2];

    //POINTER(repr=nid, type=data_indirect_node_block, when=(self.i_mode & S_IFMT) == S_IFREG)
    //POINTER(repr=nid, type=dentry_indirect_node_block, when=(self.i_mode & S_IFMT) == S_IFDIR)
	__le32 nid_indirect[2];

    //POINTER(repr=nid, type=data_dindirect_node_block, when=(self.i_mode & S_IFMT) == S_IFREG)
    //POINTER(repr=nid, type=dentry_dindirect_node_block, when=(self.i_mode & S_IFMT) == S_IFDIR)
	__le32 nid_dindirect;

} __attribute__((packed));

/* 
 * Note: In F2FS, a node structure could contain i-node, direct, indirect block 
 * From the NAT, this is represented generically as an f2fs_node.
 *
 * f2fs_node := BLOCK_SIZE
 * node types := 4072 bytes
 * node footer := 16 bytes
 *
 * Original Structural definition.
    struct f2fs_node {
        union {
            struct f2fs_inode i;
            struct direct_node dn;
            struct indirect_node in;
        };
        struct node_footer footer;
    } __attribute__((packed));
 */

FSSTRUCT() node_footer {
	__le32 nid;		/* node id */
	__le32 ino;		/* inode nunmber */ 

    FIELD(type=node_footer_flag)
	__le32 flag;	/* include cold/fsync/dentry marks and offset */
	__le64 cp_ver;	/* checkpoint version */

	__le32 next_blkaddr;	/* next node page block address */
} __attribute__((packed));

#define F2FS_NODE_NUM_BYTES (F2FS_BLKSIZE - sizeof(struct node_footer))

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    u8 padding[F2FS_NODE_NUM_BYTES];
    struct node_footer footer;
} f2fs_block;

typedef FSSTRUCT(base=f2fs_block, when=(self.footer.ino == self.footer.nid) && (self.footer.nid >= 3)) {
    POINTER(name=i, repr=offset, type=struct f2fs_inode, expr=0);
} inode_block;

/* 
 * We forgo using the builtin struct definition: struct direct_node and opt for
 * a more unambiguous declaration by forking it into a data/dentry direct/indirect node.
 *
 * Note: Though in the f2fs code, we have that direct/indirect nodes are contained
 * within the f2fs_node structure, we do not make this relationship explicit through
 * an inheritance based approach in order to not proceed in a DFS during walking
 * of the NAT table.
 */

/*
typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    POINTER(repr=block, type=f2fs_data_block)
	__le32 addr[ADDRS_PER_BLOCK];	
    struct node_footer footer;
} data_direct_node_block;

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    POINTER(repr=block, type=struct f2fs_dentry_block)
	__le32 addr[ADDRS_PER_BLOCK];	
    struct node_footer footer;
} dentry_direct_node_block;

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    POINTER(repr=nid, type=data_direct_node_block)
	__le32 nid[NIDS_PER_BLOCK];
    struct node_footer footer;
} data_indirect_node_block;

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    POINTER(repr=nid, type=dentry_direct_node_block)
	__le32 nid[NIDS_PER_BLOCK];	
} dentry_indirect_node_block;

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    POINTER(repr=nid, type=data_indirect_node_block)
	__le32 nid[NIDS_PER_BLOCK];
    struct node_footer footer;
} data_dindirect_node_block;

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) {
    POINTER(repr=nid, type=dentry_indirect_node_block)
	__le32 nid[NIDS_PER_BLOCK];
    struct node_footer footer;
} dentry_dindirect_node_block;
*/

#endif /* F2FS_INODE_H */
