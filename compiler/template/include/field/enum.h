@[ from "macro/integer.h" import integer_class ]

struct @(field.classname) : public @( integer_class(field) )
{  
    virtual int parse(const char * buf, unsigned len) override;
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0); 
    virtual const char * to_string(char * buf=nullptr, unsigned len=0) const override;
    
    @(field.classname)(@(field.object.classname) & s, int index=0);
    @(field.type) & operator=(@(field.type) rhs);
} @(field.name);

