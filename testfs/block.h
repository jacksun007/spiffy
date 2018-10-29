#ifndef _BLOCK_H
#define _BLOCK_H

#include "super.h"
#include <sys/types.h>

void write_blocks(struct super_block *sb, const char *blocks, off_t start, 
                  size_t nr);
void zero_blocks(struct super_block *sb, off_t start, size_t nr);
void read_blocks(struct super_block *sb, char *blocks, off_t start, size_t nr);

#endif /* _BLOCK_H */

