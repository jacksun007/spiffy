#ifndef _TESTFS_SNOOP_H_
#define _TESTFS_SNOOP_H_

#ifdef __cplusplus
extern "C" {
#endif

struct snoop;

// block.c
extern struct snoop * snp;

void snoop_testfs_init(void);
void snoop_testfs_shutdown(void);

struct super_block;
struct context;

int cmd_snoop(struct super_block *sb, struct context *c);

#ifdef __cplusplus
}
#endif

#endif /* _TESTFS_SNOOP_H_ */
