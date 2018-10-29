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

enum CorruptType
{
    CT_SET,     /* set value to n */
    CT_RANDOM,  /* set a random value */
    CT_ADD,     /* add n to current value (may be negative) */
};

struct Victim
{
    const char * name;  /* mandatory: corrupt specific field name */
    CorruptType type;   /* optional: corrupt specific type */
    int index;          /* optional: particular index in case of array */
    int skip;           /* optional: skip X instances before corruption */
    int repeat;         /* optional: repeat the corruption X times */
    long value;         /* optional: set or add this value */
   
    Victim(const char * n) : name(n), type(CT_SET), index(0), skip(0), 
        repeat(1), value(0) {}
};

class CorruptSerializer : public FS::Serializer
{
    CorruptType type;
    long value;
    
    static int fill_random(char * buf, unsigned size);
    
    int try_add(FS::Entity & ent, char * buf, unsigned len);
    int try_set(char * buf, unsigned len);
    
public:
    CorruptSerializer() : type(CT_SET), value(0) {}
    virtual ~CorruptSerializer() {}
    
    virtual int post_process(FS::Entity & ent, char * buf, unsigned len) override;
    void set_victim(Victim & v) { type = v.type; value = v.value; }
};

class PtrVisitor;
class Corruptor : public FS::Visitor
{
    PtrVisitor * ptr_visitor;
    FS::FileSystem & fs;
    std::vector<Victim> victim;
    int num_corrupted;
    
    int visit_container(FS::Container * ctn);
    int visit_field(FS::Field * field);
    
public:
    CorruptSerializer serializer;

    Corruptor(FS::FileSystem & fs);
    ~Corruptor();

    virtual int visit(FS::Entity & ent) override;
    int process_arguments(int argc, char * argv[]);
    Victim * add_victim(const char * n);
    int run();
    size_t size() { return victim.size(); } 
};

#endif

