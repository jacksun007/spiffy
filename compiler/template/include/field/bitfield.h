@[ from "macro/integer.h" import bitfield_class ]

struct @(field.classname) : public @( bitfield_class(field) )
{
    virtual int accept_fields(FS::Visitor & visitor) override;
    
    @(field.classname)(@(field.object.classname) & s, int index=0);
    @(field.type) & operator=(@(field.type) rhs);
    
} @(field.name);
