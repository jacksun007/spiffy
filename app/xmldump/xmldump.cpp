/*
 * xmldump.cpp
 *
 * Implements dumping of file system metadata graph into XML format
 *
 * Kuei (Jack) Sun
 * kuei.sun@mail.utoronto.ca
 *
 * University of Toronto
 * 2014
 */

#include <libfs.h>
#include <iostream>
#include <utility>
#include <cstdarg>
#include <string>
#include "xmldump.h"

using namespace std;

XDFormat::XDFormat() {} 

XDFormat::~XDFormat() {}
    
bool XDFormat::add_ignore(ignore_t ig, const char * arg, long value)
{
    switch (ig) {
    case IGNORE_TYPE:
    case IGNORE_POINTER_BY_TYPE:
    case IGNORE_POINTERS:
        ignore[ig].emplace_back(arg);
        break;
    case IGNORE_FIELD_BY_VALUE:
        ignore[ig].emplace_back(nullptr, arg, value);
        break;
    case IGNORE_FIELD_BY_NAME:
    case IGNORE_POINTER_BY_NAME:
    case IGNORE_EMPTY_STRING:
        ignore[ig].emplace_back(nullptr, arg);
        break;
    case IGNORE_TYPE_BY_VALUE:
        ignore[ig].emplace_back(arg, nullptr, value);
        break;
    case IGNORE_OBJECT_BY_FIELD:
    case IGNORE_POINTER_BY_ASPC:
    default:
        return false;
    }
    
    return true;
}

bool XDFormat::add_ignore(ignore_t ig, const char * t, const char * n, long v)
{
    if (ig != IGNORE_OBJECT_BY_FIELD)
        return false;
        
    ignore[ig].emplace_back(t, n, v);
    return true;
}

bool XDFormat::add_ignore(ignore_t ig, long aspc)
{
    if (ig != IGNORE_POINTER_BY_ASPC)
        return false;
  
    ignore[ig].emplace_back(nullptr, nullptr, aspc);
    return true;
}

template<typename F>
static bool try_ignore(FS::Entity * ent, std::vector<Ignore> & igvec, F functor)
{
    std::vector<Ignore>::iterator it = igvec.begin();
    
    for ( ; it != igvec.end(); it++)
    {
        if (functor(ent, *it))
            return true;
    }
    
    return false;
}

bool XDFormat::can_ignore(ignore_t ig, FS::Entity * ent)
{
switch (ig) {
    case IGNORE_TYPE:
    case IGNORE_POINTERS:
    case IGNORE_POINTER_BY_TYPE:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            return (!strcmp(ent->get_type(), ign.type));
        });
    case IGNORE_FIELD_BY_VALUE:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            FS::Field * field = ent->to_field();
            if (field == nullptr) return false;
            
            if (strcmp(ent->get_name(), ign.name))
                return false;
                
            return ((long)field->to_integer() == ign.value);
        });     
    case IGNORE_FIELD_BY_NAME:
    case IGNORE_POINTER_BY_NAME:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            return (!strcmp(ent->get_name(), ign.name));
        });
    case IGNORE_EMPTY_STRING:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            FS::Field * field = ent->to_field();
            
            if (field == nullptr) return false;
            if (strcmp(ent->get_name(), ign.name)) return false;
            
            return strlen(field->to_string()) == 0;
        });
    case IGNORE_TYPE_BY_VALUE:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            FS::Field * field = ent->to_field();
            if (field == nullptr) return false;
            
            if (strcmp(ent->get_type(), ign.type))
                return false;
                
            return ((long)field->to_integer() == ign.value);
        });
    case IGNORE_OBJECT_BY_FIELD:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            FS::Entity * child;
            FS::Field * field;
            
            if (strcmp(ent->get_type(), ign.type))
                return false;
                
            child = ent->get_field_by_name(ign.name);
            if (child == nullptr) return false;
            
            field = child->to_field();
            if (field == nullptr) return false;
            return ((long)field->to_integer() == ign.value);
        });
        return false;
    case IGNORE_POINTER_BY_ASPC:
        return try_ignore(ent, ignore[ig], [](FS::Entity * ent, Ignore & ign) {
            FS::Pointer * ptr = ent->to_pointer();
            if (ptr == nullptr) return false;
            
            return ((long)ptr->pointer_location().aspc == ign.value);
        });
    default:
        break;
    }
    
    return false;
}

class Element
{
    /* make a copy of value since it may be constructed on temporary buffer */
    struct Pair : public std::pair<const char *, char *>
    {
        Pair(const char * a, const char * b) :
            std::pair<const char *, char *>(a, strdup(b)) {}
        Pair(Pair && rhs) : std::pair<const char *, char *>(std::move(rhs))
            { rhs.second = nullptr; }
        ~Pair() { free(this->second); }
    };

    static int level;
    static const char * indent;
    
    const char * tag;
    bool linebreak;  
    std::vector<Pair> attr;

public:
    Element(const char * tag, ...) : tag(tag), linebreak(true) {
        va_list args;
        const char * key, * value;
        
        va_start(args, tag);
        while ((key = va_arg(args, const char *)) != nullptr) {
            if ((value = va_arg(args, const char *)) == nullptr)
                break;
                
            attr.emplace_back(key, value);
        }
        
        va_end(args);
    }
    
    ~Element() {
        
    }
    
    void add_attr(const char * first, const char * second) {
        attr.emplace_back(first, second);
    }
    
    void add_break() {
        linebreak = true;
        cout << endl;
    }

    void inline_tag() {
        start_tag(false);
    }

    void start_tag(bool br=true) {
        std::vector<Pair>::iterator it = attr.begin();
        linebreak = br;
        
        for (int i = 0; i < level; ++i) 
            cout << indent;
            
        cout << "<" << tag;
        for ( ; it != attr.end(); ++it)
            cout << " " << it->first << "=\"" << it->second << "\"";
        cout << (linebreak ? ">\n" : ">");
        
        level++;
    }
    
    void end_tag() {
        level--;
        
        if (linebreak)
            for (int i = 0; i < level; ++i) 
                cout << indent;
    
        cout << "</" << tag << ">\n";
        delete this;
    }
};

int Element::level = 0;
const char * Element::indent = "  ";

#define new_element(n, ...) new Element((n), ##__VA_ARGS__, nullptr)  
#define to_cstring(v) std::to_string(v).c_str()

class XMLVisitor;
class PtrVisitor : public FS::Visitor
{
    XMLVisitor * xml_visitor;
    XDFormat & fmt;

public:    
    PtrVisitor(XMLVisitor * v, XDFormat & fmt) : xml_visitor(v), fmt(fmt) {}
    virtual int visit(FS::Entity & ent) override;
};

class XMLVisitor : public FS::Visitor
{
    FS::FileSystem & fs;
    XDFormat & fmt;
    PtrVisitor ptr_visitor;

    static void print_hex(const char * buf, int num)
    {
        static char table[] = "0123456789abcdef";
    
        for  (int i = 0; i < num; i++ )
        {
            unsigned char v = (unsigned char)buf[i];
            putc(table[v >> 4], stdout);
            putc(table[v & 0x0F], stdout);
        }
    }

    static void print_uuid(const char * buf)
    {
        print_hex(buf, 4);
	    cout << "-";
	    print_hex(buf+4, 2);
	    cout << "-";
	    print_hex(buf+6, 2);
	    cout << "-";
	    print_hex(buf+8, 2);
	    cout << "-";
	    print_hex(buf+10, 6);
    }
    
    static void print_timestamp(time_t ts)
    {
        char buf[128];
        struct tm * timeinfo;
        timeinfo = localtime(&ts);
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", timeinfo);
        cout << buf;
    }

    int visit_container(FS::Container * ctn)
    {
        const char * rank = ctn->is_extent() ? "extent" : "container";
        const FS::Location & loc = ctn->get_location();
        int ret = 0;
        bool ignored;
        Element * elem = new_element(rank, 
            "type", ctn->get_type(),
            "aspc", fs.address_space_to_name(loc.aspc),
            "addr", to_cstring(loc.addr), 
            "size", to_cstring(loc.size));
        
        if (loc.offset > 0)
            elem->add_attr("offset", to_cstring(loc.offset));
            
        if (ctn->is_element())
            elem->add_attr("index", to_cstring(ctn->get_index()));
           
        ignored = fmt.can_ignore(IGNORE_TYPE, ctn);   
        ignored = ignored || fmt.can_ignore(IGNORE_OBJECT_BY_FIELD, ctn);
        
        if (ignored) {
            elem->add_attr("ignored", "true");    
            elem->inline_tag();
        }
        else {    
            elem->start_tag();
            ret = ctn->accept_fields(*this);
        }
        
        elem->end_tag(); 
        if (ret < 0) {
            cerr << "error while traversing " << ctn->get_type() << endl;
            return ret;   
        }
        /* if the container does not exist inside an extent */
        else if (!ctn->is_element() && !fmt.can_ignore(IGNORE_POINTERS, ctn)) {
            ret = ctn->accept_pointers(ptr_visitor);
        }
            
        return ret;
    }
    
    int visit_field(FS::Field * field)
    {
        int ret = 0;
        Element * elem;
        bool ignored;
        
        ignored = fmt.can_ignore(IGNORE_TYPE, field);
        ignored = ignored || fmt.can_ignore(IGNORE_TYPE_BY_VALUE, field);
        ignored = ignored || fmt.can_ignore(IGNORE_FIELD_BY_NAME, field);
        ignored = ignored || fmt.can_ignore(IGNORE_FIELD_BY_VALUE, field);
        ignored = ignored || fmt.can_ignore(IGNORE_EMPTY_STRING, field);
        
        /* don't print ignored fields */
        if (ignored && !field->is_aggregate())
            return 0;
        
        elem = new_element("field",
            "type", field->get_type(),
            "size", to_cstring(field->get_size()));
        
        if (field->is_element() || strlen(field->get_name()) == 0)
            elem->add_attr("index", to_cstring(field->get_index()));
        else
            elem->add_attr("name", field->get_name());           
        
        if (field->is_implicit())
            elem->add_attr("implicit", "true");
        
        if (field->is_offset() || field->is_bitfield())
		        elem->add_attr("value", to_cstring(field->to_integer()));
        
        if (ignored)
            elem->add_attr("ignored", "true");
            
        elem->inline_tag();
        if ( ignored ) { /* do nothing */ }
        else if ( field->is_aggregate() ) 
		{	    
			elem->add_break();
			ret = field->accept_fields(*this);
		}
		else if ( field->is_uuid() )
		{
		    print_uuid(field->to_string()); 
		}
		else if ( field->is_timestamp() )
		{
		    unsigned long myval = field->to_integer();
		    print_timestamp((time_t)myval);
		}
		else if ( field->is_enum() )
		{
		    const char * name = field->to_string();
		    if (name == nullptr)
		        cout << field->to_integer();
		    else
		        cout << name << "(" << field->to_integer() << ")";
		}
		else /* integer and cstring */
		{
		    cout << field->to_string();
		}
		
        elem->end_tag();
        return ret;
    }
    
    /* we also visit objects in this function */
    int visit_entity(FS::Entity * ent)
    {
        int ret = 0;
        Element * elem;
        const char * rank = "entity";
        FS::Object * obj = ent->to_object();
        bool ignored;
        
        if (obj != nullptr) rank = "object";
        else if (ent->is_array()) rank = "array";
        else if (ent->is_struct()) rank = "struct";
        
        elem = new_element(rank,
            "type", ent->get_type(),
            "size", to_cstring(ent->get_size()));
        
        if (ent->is_element() ||   strlen(ent->get_name()) == 0)
            elem->add_attr("index", to_cstring(ent->get_index()));
        else
            elem->add_attr("name", ent->get_name());       
        
        ignored = fmt.can_ignore(IGNORE_TYPE, ent);
        ignored = ignored || fmt.can_ignore(IGNORE_FIELD_BY_NAME, ent);
        if (obj != nullptr && !ignored) {
            ignored = fmt.can_ignore(IGNORE_OBJECT_BY_FIELD, ent);
        }
        
        if (ignored) {
            elem->add_attr("ignored", "true");    
            elem->inline_tag();
        } 
        else {
            elem->start_tag();
            ret = ent->accept_fields(*this);
        }
             
        elem->end_tag();
        if (ret < 0)
            cerr << "error while traversing " << ent->get_type() << endl;
            
        return ret;
    }

public:
    XMLVisitor(FS::FileSystem & fs, XDFormat & fmt) : fs(fs), fmt(fmt),
        ptr_visitor(this, fmt) {}

    virtual int visit(FS::Entity & ent) override
    {
        if (ent.to_container())
            return visit_container(ent.to_container());
        else if (ent.to_field())
            return visit_field(ent.to_field());        
        return visit_entity(&ent);  
    }
};

int PtrVisitor::visit(FS::Entity & ent)
{
    FS::Pointer * ptr = ent.to_pointer();
    FS::Container * ctn;
    int ret = 0;
    bool ignored;
    
    if (ptr == nullptr) {
        cerr << "error visiting non-pointer type " << ent.get_type()
             << " during accept_pointers\n";
        return FS::ERR_CORRUPT;
    }
    
    /* pointer does not point to anything valid */
    if (ptr->pointer_type() == FS::INVALID_TYPE_ID)
        return 0;

    ignored = fmt.can_ignore(IGNORE_POINTER_BY_TYPE, &ent);
    ignored = ignored || fmt.can_ignore(IGNORE_POINTER_BY_NAME, &ent);
    ignored = ignored || fmt.can_ignore(IGNORE_POINTER_BY_ASPC, &ent);
    if (ignored)
        return 0;

    if ((ctn = ptr->fetch()) != nullptr) {
        ret = xml_visitor->visit(*ctn);
        ctn->destroy();
    }
    else if (ptr->to_integer() > 0) {
        cerr << "error while fetching from pointer type " 
             << ent.get_type() << endl;
        return FS::ERR_CORRUPT;
    }
    
    return ret;
}

int xd_dump_filesystem(FS::FileSystem & fs, XDFormat & fmt)
{
    XMLVisitor xml_visitor(fs, fmt);
    FS::Container * super;
    int ret = 0;
    Element * elem = new_element("filesystem", 
        "name", fs.get_name(),
        "src",  fs.io.get_name());
        
    elem->start_tag();
    
    if ((super = fs.fetch_super()) != nullptr) {
        xml_visitor.visit(*super);
        super->destroy();
    }
    else {
        ret = FS::ERR_CORRUPT;
    }
    
    elem->end_tag();
    return ret;
}	

