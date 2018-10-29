#ifndef FORMAT_H
#define FORMAT_H

#include <jd.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed int s32;

#define BLOCK_SIZE 256

#define SUPER_BLOCK_SIZE    1           
#define INODE_FREEMAP_SIZE  1    /* must be 1 else the annotation must change */      
#define BLOCK_FREEMAP_SIZE  62        
#define NR_INODE_BLOCKS     192
/* size of data blocks will be as many as block freemap supports */

ADDRSPACE(name=block, size=BLOCK_SIZE, null=0);

FSSUPER(location=0) dsuper_block {
        POINTER(repr=block, type=inode_freemap)
        u32 inode_freemap_start;
        
        POINTER(repr=block, type=block_freemap_table)
        u32 block_freemap_start;
        
        POINTER(repr=block, type=inode_table)
        u32 inode_blocks_start;
        
        u32 data_blocks_start;
        u32 used_inode_count;
        u32 used_block_count;
};

VECTOR(name=data_block, type=data, size=BLOCK_SIZE);

typedef FSCONST() {I_NONE, I_FILE, I_DIR} inode_type;

#define NR_DIRECT_BLOCKS 4
#define NR_INDIRECT_BLOCKS (BLOCK_SIZE/(int)sizeof(u32))

FSSTRUCT(name=in) dinode {
        FIELD(type=inode_type)
        u32 i_type;                             /* 0x00 */
        u64 i_size;                             /* 0x04 */
        
        POINTER(repr=block, type=data_block, when=self.i_type == I_FILE)        
        POINTER(repr=block, type=dir_block,  when=self.i_type == I_DIR)
        u32 i_block_nr[NR_DIRECT_BLOCKS];       /* 0x0C */
        
        POINTER(repr=block, type=data_indirect_block, when=self.i_type == I_FILE)
        POINTER(repr=block, type=dir_indirect_block,  when=self.i_type == I_DIR)
        u32 i_indirect;                         /* 0x1C */
} __attribute__((packed));

#define INODES_PER_BLOCK (BLOCK_SIZE/(sizeof(struct dinode)))

VECTOR(name=inode_table, type=inode_block, size=BLOCK_SIZE*NR_INODE_BLOCKS);

VECTOR(name=inode_block, type=struct dinode, size=BLOCK_SIZE);

// array struct declaration for bitmaps
VECTOR(name=inode_freemap, type=bitmap, size=BLOCK_SIZE);

VECTOR(name=block_freemap_table, type=block_freemap, size=BLOCK_FREEMAP_SIZE*BLOCK_SIZE);

VECTOR(name=block_freemap, type=bitmap, size=BLOCK_SIZE);

typedef FSSTRUCT(rank=container) {
    POINTER(repr=block, type=data_block)
    u32 ind_block_nr[BLOCK_SIZE/sizeof(u32)];
} data_indirect_block;

typedef FSSTRUCT(rank=container) {
    POINTER(repr=block, type=dir_block) 
    u32 ind_block_nr[BLOCK_SIZE/sizeof(u32)];
} dir_indirect_block;

VECTOR(name=dir_block, type=struct dirent, size=BLOCK_SIZE);

FSSTRUCT(size=self.d_rec_len) dirent {
        u16 d_rec_len;
        u16 d_name_len;
        s32 d_inode_nr;
        
        VECTOR(name=d_name, type=char, size=self.d_name_len);
        FIELD(name=d_size, type=int, expr=in.i_size);

} __attribute__((packed));

#define D_NAME(d) ((char*)(d) + sizeof(struct dirent))

#endif

