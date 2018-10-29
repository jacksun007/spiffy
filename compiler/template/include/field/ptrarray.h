@[ from "macro/pointer.h" import pointer_declare ]

class @(field.classname) : public FS::Entity
{
    @( pointer_declare(fs, field, "Element") );
    
    @[ include "field/array/ctor.h" with context ]

    virtual int parse(const char * buf, unsigned len) override;
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0);
    
    virtual int accept_fields(FS::Visitor & visitor) override;
    virtual int accept_pointers(FS::Visitor & visitor) override;
    
    virtual void resolve(void) override;

    int compare(const @(field.classname)& other, const Entity& p, FS::Visitor& v);

} @(field.name);
