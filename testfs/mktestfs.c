/*
 * mktestfs.c
 *
 * program for making a new testfs file system
 *
 */

#include "testfs.h"
#include "super.h"
#include "inode.h"
#include "dir.h"
#include "common.h"
#include <stdint.h>

#ifdef __SNOOP__
#include <nosnoop.c>    // super hack
#endif

static void
usage(char *progname)
{
        fprintf(stderr, "Usage: %s rawfile\n", progname);
        exit(1);
}

int
main(int argc, char *argv[])
{
        struct super_block *sb;
        int ret;

        if (argc != 2) {
                usage(argv[0]);
        }
        
        const size_t num_bits_in_freemap = BLOCK_SIZE * INODE_FREEMAP_SIZE * 8;
        if ( num_bits_in_freemap < NR_INODE_BLOCKS * INODES_PER_BLOCK ) {
                errno = EINVAL;
                EXIT("not enough inode freemap to support " 
                     STR(NR_INODE_BLOCKS) " inode blocks");
        }
        
        sb = testfs_make_super_block(argv[1]);
        testfs_make_inode_freemap(sb);
        testfs_make_block_freemap(sb);
        testfs_make_inode_blocks(sb);
        testfs_close_super_block(sb);

        ret = testfs_init_super_block(argv[1], 0, &sb);
        if (ret) {
                EXIT("testfs_init_super_block");
        }
        testfs_make_root_dir(sb);
        
        const uint32_t ind_per_block = BLOCK_SIZE / sizeof(u32);
        const uint64_t max_blocks = NR_DIRECT_BLOCKS +             // direct
                                    ind_per_block +                // single
                                    ind_per_block * ind_per_block; // double
        const uint64_t num_data_blocks = BLOCK_FREEMAP_SIZE * BLOCK_SIZE * 8;
        printf("%s", "\n");                   
        printf("   number of inode(s): %lu\n", NR_INODE_BLOCKS * INODES_PER_BLOCK);
        printf("number of data blocks: %lu\n", num_data_blocks);
        printf("    maximum file size: %lu bytes\n", BLOCK_SIZE * max_blocks);
        printf("\n");
        printf("0x%08x: super block\n", 0);
        printf("0x%08x: inode freemap\n", sb->sb.inode_freemap_start * BLOCK_SIZE);
        printf("0x%08x: block freemap\n", sb->sb.block_freemap_start * BLOCK_SIZE);
        printf("0x%08x: inode blocks\n", sb->sb.inode_blocks_start * BLOCK_SIZE);
        printf("0x%08x: data blocks\n", sb->sb.data_blocks_start * BLOCK_SIZE);
        printf("\n");      
        testfs_close_super_block(sb);       
        return 0;
}

