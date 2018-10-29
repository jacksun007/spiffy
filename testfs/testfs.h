#ifndef _TESTFS_H
#define _TESTFS_H

struct super_block;
struct inode;

#define MAX_ARGS 6

struct context {
        int nargs;
        const char *cmd[MAX_ARGS+1];          // +1 to keep the overflows
        struct inode *cur_dir;
};

// dir.c
int cmd_cd(struct super_block *, struct context *c);
int cmd_pwd(struct super_block *, struct context *c);
int cmd_ls(struct super_block *, struct context *c);
int cmd_lsr(struct super_block *, struct context *c);
int cmd_create(struct super_block *, struct context *c);
int cmd_stat(struct super_block *, struct context *c);
int cmd_rm(struct super_block *, struct context *c);
int cmd_mkdir(struct super_block *, struct context *c);

// file.c
int cmd_read(struct super_block *, struct context *c);
int cmd_write(struct super_block *, struct context *c);
int cmd_debug(struct super_block * sb, struct context *c);

#endif /* _TESTFS_H */

