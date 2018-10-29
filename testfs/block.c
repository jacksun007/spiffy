/*
 * block.c
 *
 * defines block-level operations
 *
 */

#include "common.h"
#include "block.h"
#include "testfs.h"
#ifdef __SNOOP__
#include <snoop.h>
#include "snoop.h"
#endif

static char zero[BLOCK_SIZE] = {0};

void
write_blocks(struct super_block *sb, const char *blocks, off_t start, size_t nr)
{
        long pos;
       
        if ((pos = ftell(sb->dev)) < 0) {
                EXIT("ftell");
        }
        
        if (fseek(sb->dev, start * BLOCK_SIZE, SEEK_SET) < 0) {
                EXIT("fseek");
        }

        if (fwrite(blocks, BLOCK_SIZE, nr, sb->dev) != nr) {
                EXIT("fwrite");
        }
        
#ifdef __SNOOP__      
        for ( unsigned i = 0; i < nr; i++ )
        {
            int ret = snoop_write(snp, start * BLOCK_SIZE, blocks, BLOCK_SIZE);
            assert(ret == 0);
            start  += 1;
            blocks += BLOCK_SIZE;
        }
#endif
        
        if (fseek(sb->dev, pos, SEEK_SET) < 0) {
                EXIT("fseek");
        }
}

void
zero_blocks(struct super_block *sb, off_t start, size_t nr)
{
        size_t i;

        for (i = 0; i < nr; i++) {
                write_blocks(sb, zero, start + i, 1);
        }
}

void
read_blocks(struct super_block *sb, char *blocks, off_t start, size_t nr)
{
        long pos;

        if ((pos = ftell(sb->dev)) < 0) {
                EXIT("ftell");
        }
        
        if (fseek(sb->dev, start * BLOCK_SIZE, SEEK_SET) < 0) {
                EXIT("fseek");
        }
        
        if (fread(blocks, BLOCK_SIZE, nr, sb->dev) != nr) {
                EXIT("freed");
        }
        
#ifdef __SNOOP__      
        for ( unsigned i = 0; i < nr; i++ )
        {
            int ret = snoop_read(snp, start * BLOCK_SIZE, blocks, BLOCK_SIZE);
            assert(ret == 0);
            start  += 1;
            blocks += BLOCK_SIZE;
        }
#endif
        
        if (fseek(sb->dev, pos, SEEK_SET) < 0) {
                EXIT("fseek");
        }       
}

