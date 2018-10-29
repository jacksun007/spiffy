/*
 * ext3reader.cpp
 *
 * implementation of FS::Reader for the ext3 file system (specifically for the
 * file addrspace)
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#include "ext3reader.h"
#include <libext3.h>
#include <cerrno>
#include <iostream>
#include <cstring>

using namespace std;

Ext3Reader::Ext3Reader() : FileReader(), sb(nullptr) {}

Ext3Reader::~Ext3Reader()
{
    if ( sb != nullptr )
    {
        delete sb;
    }
}

void Ext3Reader::set_super_block(Ext3::Ext3SuperBlock * super)
{
    if ( sb != nullptr )
    {
        delete sb;
    }
    
    sb = super;
    set_block_size(1024 << sb->s_log_block_size); 
}

ssize_t Ext3Reader::get_inode(long id, struct ext3_inode & inode)
{
    char * buf = nullptr;
    ssize_t ret;

    if ( sb == nullptr )
    {
        return -EINVAL;
    }
    
    if ( id > sb->s_inodes_count || id <= 0 )
    {
        return -EINVAL;
    }
    
    long block_group_index = (id - 1) / sb->s_inodes_per_group;
    long inode_table_index = (id - 1) % sb->s_inodes_per_group;
    long inode_per_block   = block_size / sb->s_inode_size;
    long inode_block_index = inode_table_index / inode_per_block;
    long inode_index       = inode_table_index % inode_per_block;
    long bgd_per_block     = block_size / sizeof(struct ext3_group_desc);
    long bgd_block_index   = block_group_index / bgd_per_block;
    long bgd_index         = block_group_index % bgd_per_block;
    long blk_grp_dsc_blknr = sb->s_block_group_desc + bgd_block_index;

    if ( (ret = block_read(blk_grp_dsc_blknr, block_size, 0, &buf)) < 0 )
    {
        return ret;
    }
    
    struct ext3_group_desc * desc = (struct ext3_group_desc *)buf;
    long inode_block_blknr = desc[bgd_index].bg_inode_table + inode_block_index;
    
    delete [] buf;
    if ( (ret = block_read(inode_block_blknr, block_size, 0, &buf)) < 0 )
    {
        return ret;
    }
   
    inode = ((struct ext3_inode *)buf)[inode_index];
    delete [] buf;
    return 0;
}

static long ipow(long val, long exp)
{
    long ret = 1;
    
    while ( exp > 0 )
    {
        ret *= val;
    }
    
    return ret;
}

long Ext3Reader::get_phy_block_nr(unsigned int * indptr, long blknr, int level)
{
    char * block;
    const long indirect_per_block = block_size / sizeof(__le32);
    const long divisor = ipow(indirect_per_block, level-1);
    const long top_index = blknr / divisor;
    const long bot_index = blknr % divisor;
    long ret;
    
    if ( level == 0 )
    {
        return *indptr;
    }
    
    if ( (ret = read(Ext3::AddrSpace::BLOCK, *indptr, 
                     block_size, 0, &block)) < 0 )
        return ret;
    
    unsigned int temp = ((unsigned int *)block)[top_index];
    delete [] block;
    return get_phy_block_nr(&temp, bot_index, level-1);
}

ssize_t Ext3Reader::logical_block_read(struct ext3_inode & inode, long blknr, 
                                       char **bufptr)
{
    ssize_t ret = 0;
    long phy_blknr;
    const long indirect_per_block = block_size / sizeof(__le32);

    if ( blknr < EXT3_NDIR_BLOCKS )
    {
        phy_blknr = inode.i_block[blknr];
        goto out;
    }
    
    blknr -= EXT3_NDIR_BLOCKS;
    // single indirect
    if ( blknr < indirect_per_block )
    {
        phy_blknr = get_phy_block_nr(&inode.i_ind_block, blknr, 1);
        goto out;
    }
    
    blknr -= indirect_per_block;
    // double indirect
    if ( blknr < indirect_per_block*indirect_per_block )
    {
        phy_blknr = get_phy_block_nr(&inode.i_dind_block, blknr, 2);
        goto out;
    }
    
    blknr -= indirect_per_block*indirect_per_block;
    // triple indirect
    if ( blknr < indirect_per_block*indirect_per_block*indirect_per_block )
    {
        phy_blknr = get_phy_block_nr(&inode.i_tind_block, blknr, 3);
        goto out;
    }
    
    return -EFBIG;
out:
    if ( phy_blknr <= 0 )
    {
        *bufptr = nullptr;
        return ret;
    }
    
    return read(Ext3::AddrSpace::BLOCK, phy_blknr, block_size, 0, bufptr);
}

ssize_t Ext3Reader::file_read(long id, size_t size, off_t offset, char **bufptr)
{
    ssize_t ret;
    struct ext3_inode inode;
    char * buf;
    char * block;
    long cur_block = offset / block_size;
    long cur_index = offset % block_size;
    off_t pos = 0;
    
    if ( (ret = get_inode(id, inode)) < 0 )
    {
        return ret;
    }
    
    if ( (offset + size) >= inode.i_size )
    {
        return -EINVAL;
    }

    buf = new char[size];

    while ( size > 0 )
    {
        if ( (ret = logical_block_read(inode, cur_block, &block)) < 0 )
        {
            delete [] buf;
            *bufptr = nullptr;
            return ret;
        }
        
        size_t copy_size = block_size - cur_index;
        copy_size = (size < copy_size) ? size : copy_size;
        memcpy(buf + pos, block + cur_index, copy_size);
        
        pos  += copy_size;
        size -= copy_size; 
        cur_block += 1;
        cur_index = 0;
    }
    
    *bufptr = buf;
    return pos;
}
    
ssize_t Ext3Reader::read(unsigned int addrsp, long id, size_t size, 
                         off_t offset, char ** bufptr) 
{
    if ( addrsp == Ext3::AddrSpace::FILE )
    {
        return file_read(id, size, offset, bufptr);
    }
    
    return FileReader::read(addrsp, id, size, offset, bufptr);
}

