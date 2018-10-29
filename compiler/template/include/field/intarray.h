@[ from "macro/integer.h" import integer_class ]

class @(field.classname) : public FS::Entity
{
    struct Element : public @( integer_class(field) ) {
        Element(@(field.object.classname) & s, int idx, const char * n="") : 
        @( integer_class(field) )("@(field.type)", FS::TF_ELEMENT, n, idx)
        { (void)s; }
    };

    @[ include "field/array/ctor.h" with context ]
    
    virtual int parse(const char * buf, unsigned len) override;
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0); 
    
    virtual int accept_fields(FS::Visitor & visitor) override;

    int compare(const @(field.classname)& other, const Entity& p, FS::Visitor& v);

} @(field.name);
