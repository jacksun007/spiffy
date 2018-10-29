#ifndef _DIR_H
#define _DIR_H

struct inode;
struct super_block;

struct dirent *testfs_next_dirent(struct inode *dir, long *offset);
int testfs_dir_name_to_inode_nr(struct inode *dir, const char *name);
int testfs_create_file_or_dir(struct super_block *sb, struct inode *dir,
                          inode_type type, const char *name);
int testfs_make_root_dir(struct super_block *sb);

#endif /* _DIR_H */
