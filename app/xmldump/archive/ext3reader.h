/*
 * ext3reader.h
 *
 * reader class for ext3 file system within a regular file
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#ifndef EXT3READER_H
#define EXT3READER_H

#include <libext3.h>
#include "filereader.h"

class Ext3Reader : public FileReader
{
private:
    Ext3::Ext3SuperBlock * sb;
    
    ssize_t get_inode(long id, struct ext3_inode & inode);
    ssize_t file_read(long id, size_t size, off_t offset, char **bufptr);
    ssize_t logical_block_read(struct ext3_inode & inode, long blknr, 
                               char **bufptr);
    long get_phy_block_nr(unsigned int * indptr, long blknr, int level);
    
public:
    Ext3Reader();
    virtual ~Ext3Reader();

    virtual ssize_t read(unsigned int addrsp, long id, size_t size, 
        off_t offset, char ** bufptr) override;
        
    // reader will delete this pointer in its destructor
    void set_super_block(Ext3::Ext3SuperBlock * super);
};


#endif /* EXT3READER_H */

