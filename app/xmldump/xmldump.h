/*
 * xmldump.h
 *
 * Copyright (C) 2015
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 */
 
#ifndef XMLDUMP_H
#define XMLDUMP_H

#include <libfs.h>
#include <vector>

/* (jsun) TODO: IGNORE_POINTERS if container in extent */
enum ignore_t
{
    IGNORE_TYPE,            /* ignore all instances of this type */
    IGNORE_FIELD_BY_NAME,   /* ignore field with name */
    IGNORE_FIELD_BY_VALUE,  /* ignore field with name and value */
    IGNORE_TYPE_BY_VALUE,   /* ignore type with field of value */
    IGNORE_OBJECT_BY_FIELD, /* ignore object with field of value */
    IGNORE_POINTER_BY_TYPE, /* ignore pointer with type */
    IGNORE_POINTER_BY_NAME, /* ignore pointer with name */
    IGNORE_POINTERS,        /* ignore all pointers in type */
    IGNORE_POINTER_BY_ASPC, /* ignore pointer with aspc */
    IGNORE_EMPTY_STRING,    /* ignore empty string field by name */
    
    NUM_IGNORES,
};

struct Ignore
{
    const char * type;  /* can also mean aspc */   
    const char * name;
    long value;
    
    Ignore(const char * t, const char * n=nullptr, long v=0) :
        type(t), name(n), value(v) {}
};

// output format class
class XDFormat 
{   
private:
    std::vector<Ignore> ignore[NUM_IGNORES];
    
public:
    XDFormat();
    ~XDFormat();

    bool add_ignore(ignore_t ig, const char * arg, long value=0);
    bool add_ignore(ignore_t ig, const char *, const char *, long); 
    bool add_ignore(ignore_t ig, long aspc);
    bool can_ignore(ignore_t ig, FS::Entity * ent);
};

// reader: the object responsible for reading from the raw image of the 
// filesystem. The image must be loaded before this function is called
// fs: a file system object that represents the file system that we want to 
// dump
//
// returns 0 on success, negative value on failure
//
int xd_dump_filesystem(FS::FileSystem &, XDFormat &);

#endif

