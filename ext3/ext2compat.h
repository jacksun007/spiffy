/* 
 * ext2compat.h
 *
 * ext2 defines used by jd
 *
 */

#pragma once

#define EXT2_SUPER_MAGIC		0xef53

/* State settings */
#define EXT2_VALID_FS			1		/* Cleanly unmounted */
#define EXT2_ERROR_FS			2		/* Currently mounted / not unmounted */

/* s_error options */
#define EXT2_ERRORS_CONTINUE	1		/* continue on error */
#define EXT2_ERRORS_RO			2		/* remount read-only */
#define EXT2_ERRORS_PANIC		3		/* panic on error */

/* Creator OS options */
#define EXT2_OS_LINUX			0
#define EXT2_OS_HURD			1
#define EXT2_OS_MASIX			2
#define EXT2_OS_FREEBSD			3
#define EXT2_OS_LITES			4
#define EXT2_OS_ROS				666		/* got dibs on the mark of the beast */

/* Revision Levels */
#define EXT2_GOOD_OLD_REV		0		/* Revision 0 */
#define EXT2_DYNAMIC_REV		1		/* Revision 1, extra crazies, etc */

/* FS Compatibile Features.  We can support them or now, without risk of
 * damaging meta-data. */
#define EXT2_FEATURE_COMPAT_DIR_PREALLOC	0x0001	/* block prealloc */
#define EXT2_FEATURE_COMPAT_MAGIC_INODES	0x0002
#define EXT2_FEATURE_COMPAT_HAS_JOURNAL		0x0004	/* ext3/4 journal */
#define EXT2_FEATURE_COMPAT_EXT_ATTR		0x0008	/* extended inode attr */
#define EXT2_FEATURE_COMPAT_RESIZE_INO		0x0010	/* non-standard ino size */
#define EXT2_FEATURE_COMPAT_DIR_INDEX		0x0020	/* h-tree dir indexing */

/* FS Incompatibile Features.  We should refuse to mount if we don't support
 * any of these. */
#define EXT2_FEATURE_INCOMPAT_COMPRESSION	0x0001	/* disk compression */
#define EXT2_FEATURE_INCOMPAT_FILETYPE		0x0002
#define EXT2_FEATURE_INCOMPAT_RECOVER		0x0004
#define EXT2_FEATURE_INCOMPAT_JOURNAL_DEV	0x0008
#define EXT2_FEATURE_INCOMPAT_META_BG		0x0010

/* FS read-only features: We should mount read-only if we don't support these */
#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER	0x0001	/* sparse superblock */
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE	0x0002	/* 64-bit filesize */
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR	0x0004	/* binary tree sorted dir */

/* Compression types (s_algo_bitmap) */
#define EXT2_LZV1_ALG			0x0001
#define EXT2_LZRW3A_ALG			0x0002
#define EXT2_GZIP_ALG			0x0004
#define EXT2_BZIP2_ALG			0x0008
#define EXT2_LZO_ALG			0x0010

/* Special inode numbers */
#define EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT4_USR_QUOTA_INO	 3	/* User quota inode */
#define EXT4_GRP_QUOTA_INO	 4	/* Group quota inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */
#define EXT2_RESIZE_INO		 7	/* Reserved group descriptors inode */
#define EXT2_JOURNAL_INO	 8	/* Journal inode */
#define EXT2_EXCLUDE_INO	 9	/* The "exclude" inode, for snapshots */
#define EXT4_REPLICA_INO	10	/* Used by non-upstream feature */

/* First non-reserved inode for old ext2 filesystems */
#define EXT2_GOOD_OLD_FIRST_INO	11

/* Inode/File access mode and type (i_mode).  Note how they use hex here, but
the crap we keep around for glibc/posix is (probably still) in octal. */
#define EXT2_S_IFSOCK			0xC000	/* socket */
#define EXT2_S_IFLNK			0xA000	/* symbolic link */
#define EXT2_S_IFREG			0x8000	/* regular file */
#define EXT2_S_IFBLK			0x6000	/* block device */
#define EXT2_S_IFDIR			0x4000	/* directory */
#define EXT2_S_IFCHR			0x2000	/* character device */
#define EXT2_S_IFIFO			0x1000	/* fifo */
#define EXT2_S_ISUID			0x0800	/* set process user id */
#define EXT2_S_ISGID			0x0400	/* set process group id */
#define EXT2_S_ISVTX			0x0200	/* sticky bit */
#define EXT2_S_IRUSR			0x0100	/* user read */
#define EXT2_S_IWUSR			0x0080	/* user write */
#define EXT2_S_IXUSR			0x0040	/* user execute */
#define EXT2_S_IRGRP			0x0020	/* group read */
#define EXT2_S_IWGRP			0x0010	/* group write */
#define EXT2_S_IXGRP			0x0008	/* group execute */
#define EXT2_S_IROTH			0x0004	/* others read */
#define EXT2_S_IWOTH			0x0002	/* others write */
#define EXT2_S_IXOTH			0x0001	/* others execute */

/* Inode flags, for how to access data for an inode/file/object */
#define EXT2_SECRM_FL			0x00000001	/* secure deletion */
#define EXT2_UNRM_FL			0x00000002	/* record for undelete */
#define EXT2_COMPR_FL			0x00000004	/* compressed file */
#define EXT2_SYNC_FL			0x00000008	/* synchronous updates */
#define EXT2_IMMUTABLE_FL		0x00000010	/* immutable file */
#define EXT2_APPEND_FL			0x00000020	/* append only */
#define EXT2_NODUMP_FL			0x00000040	/* do not dump/delete file */
#define EXT2_NOATIME_FL			0x00000080	/* do not update i_atime */
/* Compression Flags */
#define EXT2_DIRTY_FL			0x00000100	/* dirty (modified) */
#define EXT2_COMPRBLK_FL		0x00000200	/* compressed blocks */
#define EXT2_NOCOMPR_FL			0x00000400	/* access raw compressed data */
#define EXT2_ECOMPR_FL			0x00000800	/* compression error */
/* End of compression flags */
#define EXT2_BTREE_FL			0x00010000	/* b-tree format directory */
#define EXT2_INDEX_FL			0x00010000	/* hash indexed directory */
#define EXT2_IMAGIC_FL			0x00020000	/* AFS directory */
#define EXT3_JOURNAL_DATA_FL	0x00040000	/* journal file data */
#define EXT2_RESERVED_FL		0x80000000	/* reserved for ext2 library */

/* Directory entry file types */
#define EXT2_FT_UNKNOWN			0	/* unknown file type */
#define EXT2_FT_REG_FILE		1	/* regular file */
#define EXT2_FT_DIR				2	/* directory */
#define EXT2_FT_CHRDEV			3	/* character device */
#define EXT2_FT_BLKDEV			4	/* block device */
#define EXT2_FT_FIFO			5	/* FIFO / buffer file */
#define EXT2_FT_SOCK			6	/* socket */
#define EXT2_FT_SYMLINK			7	/* symbolic link */

