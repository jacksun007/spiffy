/*
 * libfs.cpp
 *
 * Source code for file system agnostic interface
 *
 * Kuei (Jack) Sun
 * kuei.sun@mail.utoronto.ca
 *
 * University of Toronto
 * 2016
 */

#include <libfs.h>
#include <cstdlib>
#include <iostream>

#ifndef __KERNEL__
#include <string.h>
#endif
 
using namespace FS;

template<> const char * 
Location::get_address<const char *>() const { return this->addrptr; }

template<> void 
Location::set_address(const char * val, unsigned len) {
    if (dynamic)
        delete this->addrptr;
    
    this->len = (unsigned short)len;
    this->addrptr = new char[len];
    memcpy(this->addrptr, val, len);
    this->dynamic = 1;
}

const char * FileSystem::type_to_name(unsigned type) const
{
    switch ( type ) 
    {
    case INVALID_TYPE_ID:
        return "INVALID_TYPE_ID";
    case UNKNOWN_TYPE_ID:
        return "UNKNOWN_TYPE_ID";
    case PRIORITY_TYPE_ID:
        return "PRIORITY_TYPE_ID";
    default:
        break;
    }
    
    return nullptr;    
}

const char * FileSystem::address_space_to_name(int aspc) const
{
    switch (aspc)
    {
    case AS_NONE:
        return "none";
    case AS_BYTE:    
        return "byte";
    default:
        break;
    }
    
    return nullptr;
}

int FileSystem::post_process(Entity & ent, char * buf, unsigned len)
{
    int ret = 0;

    if (serializer)
        ret = serializer->post_process(ent, buf, len);
    
    return ret;
}

/* null parent for default ctor */
Container Object::nullpt(nullptr, "null", 0);

/* default null io object */
IO FileSystem::nio;

Bit & Bit::operator=(Bit && rhs)
{
    this->bit = rhs.bit; 
    set_index(rhs.get_index());
    set_name(rhs.get_name());
    return *this;
}

const char * Bit::to_string(char * buf, unsigned len) const
{
    (void)buf;
    (void)len;
    return (bit) ? "1" : "0";
}

Buffer::Buffer(Buffer && rhs) : Field(std::move(rhs)), buf(rhs.buf), size(rhs.size)
{
    rhs.buf = nullptr;
}

Buffer::~Buffer()
{
    if (buf)
        delete [] buf;
}

unsigned long Buffer::to_integer() const
{
    if (buf == nullptr)
        return 0;

    return strtoul(buf, nullptr, 10);
}

Container * Pointer::fetch() const 
{ 
    assert(resolved);

    if (fetch_func != nullptr)
    {
        return (this->*fetch_func)();
    }
    
    return nullptr;
}

int Container::save(int options)
{
    FileSystem * filsys;
    char * buf;
    int ret;
    
    if (location.size == 0) return -EINVAL;
    if (path == nullptr) return ERR_UNINIT;
    if ((filsys = path->get_file_system()) == nullptr) return ERR_UNINIT;
    
    buf = new char[location.size];
    if (buf == nullptr) return -ENOMEM;

    path->buffer = buf;
    path->length = location.size;
    
    if ((ret = serialize(buf, location.size, options)) < 0)
        goto fail;

    path->buffer = nullptr;
    path->length = 0;
    
    if (!(options | SO_NO_ALLOC)) {
        if ((ret = filsys->io.alloc(location, this->type_id)) < 0)
            goto fail;
    }
    
    ret = filsys->io.write(location, buf);
fail:
    delete [] buf;
    return ret;    
}

void Container::incref() const 
{   
    ++this->refcount;
    //std::cout << "incref " << get_type() << ":" << refcount << std::endl;
}
void Container::decref() const 
{ 
    --this->refcount; 
    if (this->refcount <= 0) {
        //std::cout << "decref " << get_type() << ":" << refcount
        //          << ", deleting" << std::endl;
        delete this;
    }
    //else {
    //    std::cout << "decref " << get_type() << ":" << refcount << std::endl;
    //}
}

