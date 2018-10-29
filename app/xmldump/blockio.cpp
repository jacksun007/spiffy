/*
 * filereader.cpp
 *
 * implementation of FS::IO for reading/writing from/to block or byte address space
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#include "blockio.h"
#include <cstdio>
#include <cerrno>

int BlockIO::read_internal(off_t pos, size_t size, char * & buf)
{
    int ret = 0;
 
    if ((ret = fseek(fsimg, pos, SEEK_SET)) < 0) {
        buf = nullptr;
        return ret;
    }
    
    if ((buf = new char[size]) == nullptr) {
        return -ENOMEM;
    }
    
    if (fread(buf, size, 1, fsimg) != 1) {
        delete [] buf;
        buf = nullptr;
        return -EIO;
    }

    return size;
}

int BlockIO::write_internal(off_t pos, size_t size, const char * buf)
{
    int ret = 0;
 
    if ((ret = fseek(fsimg, pos, SEEK_SET)) < 0) {
        return ret;
    }
    
    if (fwrite(buf, size, 1, fsimg) != 1) {
        return -EIO;
    }

    return size;
}

int BlockIO::byte_read(const FS::Location & loc, char * & buf)
{
    off_t pos = loc.addr + loc.offset;
    return read_internal(pos, loc.size, buf);
}

int BlockIO::block_read(const FS::Location & loc, char * & buf)
{
    off_t pos;
    if (block_size == 0)
        return FS::ERR_UNINIT;
    pos = (off_t)(loc.addr * block_size) + loc.offset;
    return read_internal(pos, loc.size, buf);
}

int BlockIO::byte_write(const FS::Location & loc, const char * buf)
{
    off_t pos = loc.addr + loc.offset;
    return write_internal(pos, loc.size, buf);
}

int BlockIO::block_write(const FS::Location & loc, const char * buf)
{
    off_t pos;
    if (block_size == 0)
        return FS::ERR_UNINIT;
    pos = (off_t)(loc.addr * block_size) + loc.offset;
    return write_internal(pos, loc.size, buf);
}

BlockIO::BlockIO() : IO(""), fsimg(nullptr), block_size(0) {}
    
BlockIO::~BlockIO() 
{ 
    close();
}

int BlockIO::open(const char * filename)
{
    if (fsimg != nullptr)
        return -EINVAL;
        
    if ((fsimg = fopen(filename, "rb+")) == nullptr)
        return -errno;
    
    set_name(filename);    
    return 0;
}

int BlockIO::close()
{
    int ret = -EINVAL;

    if (fsimg) {
        ret = fclose(fsimg);
        fsimg = nullptr;
    }
    
    return ret;
}

int BlockIO::read(const FS::Location & loc, char * & buf)
{
    switch (loc.aspc)
    {
    case FS::AS_BYTE:
        return byte_read(loc, buf);
    // TODO: this is a nasty assumption...
    case FS::NUM_ADDRSPACES:
        return block_read(loc, buf);
    default:
        break;
    }
    
    buf = nullptr;
    return -EINVAL;
}

int BlockIO::write(const FS::Location & loc, const char * buf)
{
    switch (loc.aspc)
    {
    case FS::AS_BYTE:
        return byte_write(loc, buf);
    // TODO: this is a nasty assumption...
    case FS::NUM_ADDRSPACES:
        return block_write(loc, buf);
    default:
        break;
    }
    
    return -EINVAL;
}


