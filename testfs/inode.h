#ifndef _INODE_H
#define _INODE_H

#include "format.h"
#include <sys/types.h>

struct inode;
struct super_block;

/* initialize a hash table to store inodes */
void inode_hash_init(void);
void inode_hash_destroy(void);

/* retrieve inode from disk and store it in hash table */
struct inode *testfs_get_inode(struct super_block *sb, int inode_nr);

/* synchronize updated inode with the disk */
void testfs_sync_inode(struct inode *in);

/* remove inode from hash table */
void testfs_put_inode(struct inode *in);

/* functions to access inode information */
long testfs_inode_get_size(struct inode *in);
inode_type testfs_inode_get_type(struct inode *in);
int testfs_inode_get_nr(struct inode *in);

/* create an inode in the file system */
int testfs_create_inode(struct super_block *sb, inode_type type,
                        struct inode **inp);
                        
/* remove an inode from the file system */
void testfs_remove_inode(struct inode *in);

/* read up to len bytes of data starting at file offset st into buf 
 * on error, returns a negative value
 * on success, returns the number of bytes read */
int testfs_read_data(struct inode *in, off_t st, char *buf, size_t len);

/* write sz bytes of data starting at file offset st from buf */
int testfs_write_data(struct inode *in, off_t st, const char *buf, size_t sz);
                             
/* reduce the size of the inode to 'size', freeing blocks as required */
void testfs_truncate_data(struct inode *in, size_t size);

#endif /* _INODE_H */

