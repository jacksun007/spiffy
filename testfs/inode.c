/*
 * inode.c
 *
 * defines operations on the inode structure
 *
 */

#include "common.h"
#include "testfs.h"
#include "super.h"
#include "block.h"
#include "inode.h"
#include "list.h"

/* inode is dirty */
#define I_FLAGS_DIRTY     0x1   

/* used by external testfs checker program. (i.e. do not use) */
#define I_FLAGS_RESERVED  0x4

struct inode {
        struct dinode in;
        int i_flags;
        int i_nr;
        int i_count;
        struct hlist_node hnode; /* keep these structures in a hash table */
        struct super_block *sb;
};

static struct hlist_head *inode_hash_table = NULL;

#define INODE_HASH_SHIFT 8

#define inode_hashfn(nr)	\
	hash_int((unsigned int)nr, INODE_HASH_SHIFT)

static const int inode_hash_size = (1 << INODE_HASH_SHIFT);

void
inode_hash_init(void)
{
        int i;

        inode_hash_table = malloc(inode_hash_size * sizeof(struct hlist_head));
        if (!inode_hash_table) {
                EXIT("malloc");
        }
        for (i = 0; i < inode_hash_size; i++) {
                INIT_HLIST_HEAD(&inode_hash_table[i]);
        }
}

void
inode_hash_destroy(void)
{
        int i;
        assert(inode_hash_table);
        for (i = 0; i < inode_hash_size; i++) {
                assert(hlist_empty(&inode_hash_table[i]));
        }
        free(inode_hash_table);
}

static struct inode *
inode_hash_find(struct super_block *sb, int inode_nr)
{
        struct hlist_node *elem;
        struct inode *in;

        hlist_for_each_entry(in, elem, 
                             &inode_hash_table[inode_hashfn(inode_nr)], hnode) {
                if ((in->sb == sb) && (in->i_nr == inode_nr)) {
                        return in;
                }
        }
	return NULL;
}

static void
inode_hash_insert(struct inode *in)
{
        INIT_HLIST_NODE(&in->hnode);
        hlist_add_head(&in->hnode, 
                       &inode_hash_table[inode_hashfn(in->i_nr)]);
}

static void
inode_hash_remove(struct inode *in)
{
        hlist_del(&in->hnode);
}

static int
testfs_inode_to_block_nr(struct inode *in)
{
        int block_nr = in->i_nr / INODES_PER_BLOCK;
        assert(block_nr >= 0);
        assert(block_nr < NR_INODE_BLOCKS);
        return block_nr;
}

static int
testfs_inode_to_block_offset(struct inode *in)
{
        int block_offset = (in->i_nr % INODES_PER_BLOCK) * 
                sizeof(struct dinode);
        assert(block_offset >= 0);
        assert(block_offset < BLOCK_SIZE);
        return block_offset;
}

static void
testfs_read_inode_block(struct inode *in, char *block)
{
        int block_nr = testfs_inode_to_block_nr(in);
        read_blocks(in->sb, block, in->sb->sb.inode_blocks_start + block_nr, 1);
}

static void
testfs_write_inode_block(struct inode *in, char *block)
{
        int block_nr = testfs_inode_to_block_nr(in);
        write_blocks(in->sb, block, 
                     in->sb->sb.inode_blocks_start + block_nr, 1);
}

struct inode *
testfs_get_inode(struct super_block *sb, int inode_nr)
{
        char block[BLOCK_SIZE];
        int block_offset;
        struct inode *in;

        in = inode_hash_find(sb, inode_nr);
        if (in) {
                in->i_count++;
                return in;
        }
        if ((in = calloc(1, sizeof(struct inode))) == NULL) {
                EXIT("calloc");
        }
        in->i_flags = 0;
        in->i_nr = inode_nr;
        in->sb = sb;
        in->i_count = 1;
        testfs_read_inode_block(in, block);
        block_offset = testfs_inode_to_block_offset(in);
        memcpy(&in->in, block + block_offset, sizeof(struct dinode));
        inode_hash_insert(in);
        return in;
}

void
testfs_sync_inode(struct inode *in)
{
        char block[BLOCK_SIZE];
        int block_offset;

        assert(in->i_flags & I_FLAGS_DIRTY);
        testfs_read_inode_block(in, block);
        block_offset = testfs_inode_to_block_offset(in);
        memcpy(block + block_offset, &in->in, sizeof(struct dinode));
        testfs_write_inode_block(in, block);
        in->i_flags &= ~I_FLAGS_DIRTY;
}

void
testfs_put_inode(struct inode *in)
{
        assert((in->i_flags & I_FLAGS_DIRTY) == 0);
        if (--in->i_count == 0) {
                inode_hash_remove(in);
                free(in);
        }
}

long
testfs_inode_get_size(struct inode *in)
{
        return in->in.i_size;
}

inode_type
testfs_inode_get_type(struct inode *in)
{
        return in->in.i_type;
}

int
testfs_inode_get_nr(struct inode *in)
{
        return in->i_nr;
}

/* returns negative value on error */
int
testfs_create_inode(struct super_block *sb, inode_type type, struct inode **inp)
{
        struct inode *in;
        int inode_nr = testfs_get_inode_freemap(sb);

        if (inode_nr < 0) {
                return inode_nr;
        } else if ( inode_nr / INODES_PER_BLOCK >= NR_INODE_BLOCKS ) {
                // undo the freemap and return error
                testfs_put_inode_freemap(sb, inode_nr);
                return -ENOSPC;
        }
        in = testfs_get_inode(sb, inode_nr);
        in->in.i_type = type;
        in->i_flags |= I_FLAGS_DIRTY;
        *inp = in;
        return 0;
}

void
testfs_remove_inode(struct inode *in)
{
        testfs_truncate_data(in, 0);
        /* zero the inode */
        bzero(&in->in, sizeof(struct dinode));
        in->i_flags |= I_FLAGS_DIRTY;
        testfs_put_inode_freemap(in->sb, in->i_nr);
        testfs_sync_inode(in);
        testfs_put_inode(in);
}

int
ipow(int val, int exp)
{
        int i;
        int ret = 1;
        for ( i = 0; i < exp; i++ )
                ret *= val;
        return ret; 
}

/* returns positive value if physical block is found.
 * returns 0 if physical block does not exist.
 * returns negative value if the logical block does not exist within this 
 * range of indirect blocks. */
static int
testfs_get_indirect_block(struct inode *in, char *block, int log_block_nr,
        u32 blockptr, int level)
{
        int max;
        
        if (level == 0) {
                assert(log_block_nr == 0);
                read_blocks(in->sb, block, blockptr, 1);
                return (int)blockptr;
        } 
        
        max = ipow(NR_INDIRECT_BLOCKS, level - 1);
        if ( log_block_nr < max * NR_INDIRECT_BLOCKS ) {
                int log_upper_idx = log_block_nr / max;
                int log_lower_idx = log_block_nr % max;
                u32 phy_block_nr;
                
                if (blockptr == 0)
                        return 0;

                read_blocks(in->sb, block, blockptr, 1);
                phy_block_nr = ((u32 *)block)[log_upper_idx];
                return testfs_get_indirect_block(in, block, 
                                log_lower_idx, phy_block_nr, 
                                level - 1);
        }
        
        return -1;
}

/* given logical block number, read physical block
 * return physical block number.
 * returns 0 if physical block does not exist.
 * returns negative value on other errors. */
static int
testfs_get_block(struct inode *in, char *block, int log_block_nr)
{
        int phy_block_nr;

        assert(log_block_nr >= 0);
        if (log_block_nr < NR_DIRECT_BLOCKS) {
                phy_block_nr = (int)in->in.i_block_nr[log_block_nr];
                if ( phy_block_nr > 0 )
                        read_blocks(in->sb, block, phy_block_nr, 1);
                return phy_block_nr;
        }
        log_block_nr -= NR_DIRECT_BLOCKS;
        
        /* indirect block */
        phy_block_nr = testfs_get_indirect_block(in, block, log_block_nr,
                in->in.i_indirect, 1);
        if ( phy_block_nr >= 0 )
                return phy_block_nr;
        return -EFBIG;
}

static int
testfs_allocate_indirect_block(struct inode *in, char *block, int log_block_nr,
        u32 * indptr, int level)
{
        int phy_block_nr;
        int max;
        
        if ( level == 0 ) {
                assert(log_block_nr == 0);
                assert(*indptr == 0);
                phy_block_nr = testfs_alloc_block(in->sb, block);
                if (phy_block_nr < 0)
                        return phy_block_nr;
                *indptr = phy_block_nr;
                in->i_flags |= I_FLAGS_DIRTY;
                return phy_block_nr;        
        }
        
        max = ipow(NR_INDIRECT_BLOCKS, level - 1);
        if ( log_block_nr < max * NR_INDIRECT_BLOCKS ) {
                char indirect[BLOCK_SIZE];
                int log_upper_idx = log_block_nr / max;
                int log_lower_idx = log_block_nr % max;
                
                if (*indptr == 0) {
                        phy_block_nr = testfs_alloc_block(in->sb, indirect);
                        if (phy_block_nr < 0)
                                return phy_block_nr;
                        *indptr = phy_block_nr;
                        in->i_flags |= I_FLAGS_DIRTY;
                } else {
                        read_blocks(in->sb, indirect, *indptr, 1);
                }
          
                phy_block_nr = testfs_allocate_indirect_block(in,
                                        block, log_lower_idx,
                                        &((u32 *)indirect)[log_upper_idx],
                                        level - 1);
                write_blocks(in->sb, indirect, *indptr, 1);
                return phy_block_nr;        
        }
        
        return 0;
}

static int
testfs_allocate_block(struct inode *in, char *block, int log_block_nr)
{
        int phy_block_nr;

        assert(log_block_nr >= 0);
        phy_block_nr = testfs_get_block(in, block, log_block_nr);
        
        /* block exists or error (phy_block_nr < 0) */
        if ( phy_block_nr != 0 )
                return phy_block_nr;
        
        /* allocate a direct block */
        if (log_block_nr < NR_DIRECT_BLOCKS) {
                phy_block_nr = testfs_alloc_block(in->sb, block);
                if (phy_block_nr < 0)
                        return phy_block_nr;
                in->in.i_block_nr[log_block_nr] = phy_block_nr;
                in->i_flags |= I_FLAGS_DIRTY;
                return phy_block_nr;
        }
        log_block_nr -= NR_DIRECT_BLOCKS;

        /* allocate an indirect block */
        phy_block_nr = testfs_allocate_indirect_block(in, block, log_block_nr,
                &in->in.i_indirect, 1);
                
        /* testfs_get_block should have returned EFBIG already, can't be 0 */
        assert(phy_block_nr != 0);
        return phy_block_nr;    
}

int
testfs_read_data(struct inode *in, off_t start, char *buf, size_t len)
{
        char block[BLOCK_SIZE];
        size_t b_offset = start % BLOCK_SIZE; /* src offset in block for copy */
        size_t buf_offset = 0; /* dst offset in buf for copy */
        int done = 0;
        
        assert(buf);
        if ((start + len) > in->in.i_size) 
                len = in->in.i_size - start;
        do {
                int block_nr = (start + buf_offset)/BLOCK_SIZE;
                int copy_size;

                block_nr = testfs_get_block(in, block, block_nr);
                if (block_nr < 0)
                        return block_nr;
                if ((len - buf_offset) <= (BLOCK_SIZE - b_offset)) {
                        copy_size = len - buf_offset;
                        done = 1;
                } else {
                        copy_size = BLOCK_SIZE - b_offset;
                }
                if ( block_nr == 0 ) {
                        /* sparse section -- just copy zero */
                        memset(buf + buf_offset, '\0', copy_size);
                } else {
                        memcpy(buf + buf_offset, block + b_offset, copy_size);
                }
                buf_offset += copy_size;
                b_offset = 0;
        } while (!done);
        return len;
}

int
testfs_write_data(struct inode *in, off_t start, const char *buf, size_t size)
{
        char block[BLOCK_SIZE];
        size_t b_offset = start % BLOCK_SIZE;  /* dst offset in block for copy */
        size_t buf_offset = 0; /* src offset in buf for copy */
        int done = 0;
        
        assert(buf);

        do {
                int block_nr = (start + buf_offset)/BLOCK_SIZE;
                size_t copy_size;

                block_nr = testfs_allocate_block(in, block, block_nr);
                if (block_nr < 0) {
                        size_t orig_size = in->in.i_size;
                        in->in.i_size = MAX(orig_size, start + buf_offset);
                        in->i_flags |= I_FLAGS_DIRTY;
                        testfs_truncate_data(in, orig_size);
                        return block_nr;
                }
                assert(block_nr > 0);
                if ((size - buf_offset) <= (BLOCK_SIZE - b_offset)) {
                        copy_size = size - buf_offset;
                        done = 1;
                } else {
                        copy_size = BLOCK_SIZE - b_offset;
                }
                memcpy(block + b_offset, buf + buf_offset, copy_size);
                write_blocks(in->sb, block, block_nr, 1);
                buf_offset += copy_size;
                b_offset = 0;
        } while (!done);
        in->in.i_size = MAX(in->in.i_size, start + size);
        in->i_flags |= I_FLAGS_DIRTY;
        return 0;
}

static void
testfs_truncate_indirect_block(struct inode *in, int *sbp, int *ebp, 
        u32 *indptr, int level) 
{
        int s_block_nr = *sbp;
        int e_block_nr = *ebp;
        int max = ipow(NR_INDIRECT_BLOCKS, level);

        /* remove direct block */
        if ( level == 0 ) {
                if ( *indptr > 0 ) {
                        testfs_free_block(in->sb, *indptr);
                        *indptr = 0;
                } else {
                        assert(in->in.i_type != I_DIR);
                }
                
                assert(e_block_nr > 0);
                goto out;            
        }

        /* remove indirect blocks */
        if (e_block_nr > 0) {
                int pow = max / NR_INDIRECT_BLOCKS;
                int s_block_idx = s_block_nr / pow;            // rounds down
                int e_block_idx = DIVROUNDUP(e_block_nr, pow); // rounds up
                int s_inner_idx;
                int e_inner_idx;
                char block[BLOCK_SIZE];
                int i;
        
                /* do not truncate if indptr is null or starting block is
                 * outside the boundary of this indirect block */
                if( *indptr == 0 || s_block_idx >= max ) {
                        goto out;
                }
                
                /* the start inner index (one level down) */
                s_inner_idx =  s_block_nr % pow;
                
                /* the end inner index (one level down) */
                e_inner_idx = (e_block_nr / pow - s_block_idx)*pow + 
                               e_block_nr % pow;

                read_blocks(in->sb, block, *indptr, 1);          
                for (i = s_block_idx; 
                     i < e_block_idx && i < NR_INDIRECT_BLOCKS; i++) {
                        testfs_truncate_indirect_block(in, &s_inner_idx, 
                                &e_inner_idx, &((u32 *)block)[i], 
                                level - 1);   
                }
                if (*sbp == 0) {
                        testfs_free_block(in->sb, *indptr);
                        *indptr = 0;
                        in->i_flags |= I_FLAGS_DIRTY;
                } else {
                        write_blocks(in->sb, block, *indptr, 1);
                }
        } else {
                assert(*indptr == 0);
        }
out:
        *sbp = (*sbp - max > 0) ? *sbp - max : 0;
        *ebp = *ebp - max;
}

void
testfs_truncate_data(struct inode *in, size_t size)
{
        int s_block_nr;
        int e_block_nr;
        int i;

        if (in->in.i_size <= size)
                return;
        s_block_nr = DIVROUNDUP(size, BLOCK_SIZE);
        e_block_nr = DIVROUNDUP(in->in.i_size, BLOCK_SIZE);

        /* remove direct blocks */
        for (i = s_block_nr; i < e_block_nr && i < NR_DIRECT_BLOCKS; i++) {
                /* remove if block actually allocated (not sparse) */
                if ( in->in.i_block_nr[i] > 0 ) {
                        testfs_free_block(in->sb, in->in.i_block_nr[i]);
                        in->in.i_block_nr[i] = 0;
                        in->i_flags |= I_FLAGS_DIRTY;
                }
        }
        s_block_nr -= NR_DIRECT_BLOCKS;
        s_block_nr = MAX(s_block_nr, 0);
        e_block_nr -= NR_DIRECT_BLOCKS;

        testfs_truncate_indirect_block(in, &s_block_nr, &e_block_nr, 
                &in->in.i_indirect, 1);
        assert(s_block_nr == 0);                       

        in->in.i_size = size;
        in->i_flags |= I_FLAGS_DIRTY;
}

