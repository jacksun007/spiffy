/*
 * cktestfs.c
 *
 * checks a testfs image for consistency
 */

#include "common.h" 
#include "testfs.h" 
#include "bitmap.h"
#include "super.h" 
#include "block.h"
#include "inode.h"
#include "dir.h"

#ifdef __SNOOP__
#include <nosnoop.c>    // super hack
#endif

// defined in inode.c (solution only)
extern int ipow(int val, int exp);

/* incomplete definition hack */
struct inode {
        struct dinode in;
        int i_flags;
        int i_nr;
        /* more goes here, but don't care */
};

/* used to mark as seen in a dirent */
#define I_FLAGS_SEEN      0x4

static void
testfs_check_block(struct super_block *sb, struct bitmap *b_freemap, 
                   struct inode * in, int block_nr)
{
        if (block_nr > 0) {
                int phy_block_nr = block_nr - sb->sb.data_blocks_start;
                
                if ( block_nr < (int)sb->sb.data_blocks_start ||
                     phy_block_nr >= bitmap_getsize(b_freemap) ) {
                        printf("error: inode %d refers to invalid block "
                               "number %d.\n", in->i_nr, block_nr);
                        return;
                }
                else if ( bitmap_isset(b_freemap, phy_block_nr) ) {
                        printf("error: block %d is already allocated"
                                " by inode %d.\n", block_nr, in->i_nr);
                } else 
                        bitmap_mark(b_freemap, phy_block_nr);
                
                if ( !bitmap_isset(sb->block_freemap, phy_block_nr) ) {
                        printf("error: block %d is not allocated"
                               " but inode %d refers to it.\n", 
                               block_nr, in->i_nr);
                }
        } else {
                /* directory inode should not be sparse */
                assert(in->in.i_type == I_FILE);
        }        
}

static void
testfs_check_inode_indirect(struct super_block *sb, struct bitmap *b_freemap,
                   struct inode * in, size_t * szptr, int ind_block_nr, 
                   int level)
{
        const size_t i_size = in->in.i_size;
        
        if ( level == 0 ) {
                testfs_check_block(sb, b_freemap, in, ind_block_nr);
                *szptr += BLOCK_SIZE;
        } else if ( ind_block_nr <= 0 ) {
                *szptr += BLOCK_SIZE * ipow(NR_INDIRECT_BLOCKS, level);
        } else if ( *szptr < i_size ) {
                char block[BLOCK_SIZE];
                int i;
                
                testfs_check_block(sb, b_freemap, in, ind_block_nr);
                read_blocks(sb, block, ind_block_nr, 1);
                
                for (i = 0; *szptr < i_size && i < NR_INDIRECT_BLOCKS; i++) {
                        testfs_check_inode_indirect(sb, b_freemap, in, szptr,
                        ((int *)block)[i], level - 1);
                }
        }              
}

static void
testfs_check_inode(struct super_block *sb, struct bitmap *b_freemap,
                   struct inode *in)
{ 
        size_t size = 0;
        int i;       

        for (i = 0; size < in->in.i_size && i < NR_DIRECT_BLOCKS; i++) {
                size += BLOCK_SIZE;
                testfs_check_block(sb, b_freemap, in, in->in.i_block_nr[i]);   
        }
        
        testfs_check_inode_indirect(sb, b_freemap, in, &size, 
                in->in.i_indirect, 1);
}

 
static void
testfs_check_dirent(struct super_block *sb, struct inode *in)
{
        long offset = 0;
        struct dirent *d;
            
        assert(in);
        assert(testfs_inode_get_type(in) == I_DIR);
        
        for (; (d = testfs_next_dirent(in, &offset)); free(d)) {
                if ((d->d_inode_nr < 0) || 
                    (strcmp(D_NAME(d), ".") == 0) || 
                    (strcmp(D_NAME(d), "..") == 0))
                        continue;
                if ( d->d_inode_nr > 0 ) {
                        struct inode * inc;
                        inc = testfs_get_inode(sb, d->d_inode_nr);
                        assert(inc);
                        inc->i_flags |= I_FLAGS_SEEN;
                        testfs_put_inode(inc);
                }
        }
}

static void
testfs_check_block_freemap(struct super_block *sb, struct bitmap *b_freemap)
{
        long nbits = bitmap_getsize(sb->block_freemap);
        long i, j, maxix = DIVROUNDUP(nbits, BITS_PER_WORD);
        WORD_TYPE * a = (WORD_TYPE *)bitmap_getdata(sb->block_freemap);
        WORD_TYPE * b = (WORD_TYPE *)bitmap_getdata(b_freemap);
        
        assert(nbits == BLOCK_SIZE * BLOCK_FREEMAP_SIZE * BITS_PER_WORD);
        for ( i = 0; i < maxix; i++ ) {
                long maxbix = nbits - BITS_PER_WORD * i;
                maxbix = MIN(maxbix, BITS_PER_WORD);
                for ( j = 0; j < maxbix; j++ ) {
                        WORD_TYPE abit = a[i] & (1 << j);
                        WORD_TYPE bbit = b[i] & (1 << j);
                        
                        if ( abit && !bbit ) {
                                long block_nr = i * BITS_PER_WORD + j + 
                                                sb->sb.data_blocks_start;
                                printf("error: block %ld is allocated but not "
                                       "referenced by any inode.\n", block_nr);
                        }
                }
        }
        
}

static void
testfs_checkfs(struct super_block *sb)
{
        const int nr_inodes = NR_INODE_BLOCKS * INODES_PER_BLOCK;
        struct inode ** in_array = malloc(sizeof(struct inode *)*nr_inodes);
        struct bitmap * b_freemap;
        int i;
        int ret;
        
        assert(in_array);
        ret = bitmap_create(BLOCK_SIZE * BLOCK_FREEMAP_SIZE * BITS_PER_WORD,
                            &b_freemap);
        assert(ret == 0);
        
        /* load all the inodes into memory first */
        for ( i = 0; i < nr_inodes; i++ ) {
                struct inode *in = testfs_get_inode(sb, i);
                in_array[i] = in;
        }
        
        in_array[0]->i_flags |= I_FLAGS_SEEN;
        for ( i = 0; i < nr_inodes; i++ ) {
                struct inode *in = in_array[i];
                int allocated = ((testfs_inode_get_type(in) == I_FILE) || 
                                 (testfs_inode_get_type(in) == I_DIR)) ? 1 : 0;
                int isset = bitmap_isset(sb->inode_freemap, i);
                
                if ( !isset && allocated ) {
                        printf("error: inode %d is not allocated but its "
                               "type is not none.\n", i);
                } else if ( isset && !allocated ) {
                        printf("error: inode %d is allocated but its "
                               "type is none.\n", i);
                }
                
                if (testfs_inode_get_type(in) == I_DIR) {
                        testfs_check_dirent(sb, in);
                        testfs_check_inode(sb, b_freemap, in);
                } else if (testfs_inode_get_type(in) == I_FILE) {
                        testfs_check_inode(sb, b_freemap, in);
                }
        }
        
        /* check block_freemap to see if there are leaked blocks */
        testfs_check_block_freemap(sb, b_freemap);
        
        /* check all allocated inodes are referenced and then free */
        for ( i = 0; i < nr_inodes; i++ ) {
                struct inode *in = in_array[i];
                
                if ( in->in.i_type != I_NONE && 
                   !(in->i_flags & I_FLAGS_SEEN) ) {
                        printf("error: inode %d is not referenced by any "
                               "directory entry.\n", i);
                }
                
                testfs_put_inode(in);
        }
        
        bitmap_destroy(b_freemap);
        free(in_array);
}
 
int main(int argc, const char * argv[])
{
        struct super_block *sb;
        u32 used_inode_count;
        u32 used_block_count;
        int ret;

        if (argc != 2) {
                printf("usage: %s rawfile\n", argv[0]);
                return -EINVAL;
        }
        
        ret = testfs_init_super_block(argv[1], 0, &sb);
        if (ret) {
                EXIT("testfs_init_super_block");
        }

        testfs_checkfs(sb);
        
        used_inode_count = bitmap_nr_allocated(sb->inode_freemap);
        if ( sb->sb.used_inode_count != used_inode_count ) {
                printf("error: super block's record (%d) does not match the "
                       "actual number of allocated inodes (%d).\n",
                       sb->sb.used_inode_count, used_inode_count);
        } else
                printf("nr of allocated inodes = %d\n", used_inode_count);
        
        used_block_count = bitmap_nr_allocated(sb->block_freemap);
        if ( sb->sb.used_block_count != used_block_count ) {
                printf("error: super block's record (%d) does not match the "
                       "actual number of allocated blocks (%d).\n",
                       sb->sb.used_block_count, used_block_count);
        } else 
                printf("nr of allocated blocks = %d\n", used_block_count);
               
        testfs_close_super_block(sb);       
        return 0;
}
         
         
         
