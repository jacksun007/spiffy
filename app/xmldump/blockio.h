/*
 * blockio.h
 *
 * supports byte and block address space, which basically every file system
 * uses.
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2017, University of Toronto
 */

#ifndef BLOCKIO_H
#define BLOCKIO_H

#include <libfs.h>
#include <cstdio>

class BlockIO : public FS::IO
{
protected:
    FILE * fsimg;
    unsigned block_size;

    int read_internal(off_t pos, size_t size, char * & buf);
    int write_internal(off_t pos, size_t size, const char * buf);
    
    int byte_read(const FS::Location & loc, char * & buf);
    int block_read(const FS::Location & loc, char * & buf);
    
    int byte_write(const FS::Location & loc, const char * buf);
    int block_write(const FS::Location & loc, const char * buf);
    
public:
    BlockIO();
    virtual ~BlockIO() override;

    int open(const char * filename);
    int close();
    
    void set_block_size(unsigned size) { block_size = size; }
    size_t get_block_size() const { return block_size; }

    virtual int read(const FS::Location & loc, char * & buf) override;
    virtual int write(const FS::Location & loc, const char * buf) override;
    virtual int alloc(FS::Location & loc, int type) override {
        return FS::ERR_UNIMP;
    }
};


#endif /* BLOCKIO_H */

