class @(field.classname) : public FS::Entity
{
    class Element : public FS::Buffer
    {
    public:
        Element(@(field.object.classname) & s, int idx=0, const char * n="");
        virtual int parse(const char * buf, unsigned size) override;    
        int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0); 
    };

    /* this include flips visibility to public */
    @[ include "field/array/ctor.h" with context ]

    virtual int parse(const char * buf, unsigned size) override;
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0); 
    
    virtual int accept_fields(FS::Visitor & visitor) override;

    int compare(const @(field.classname)& other, const Entity& p, FS::Visitor& v);

} @(field.name);
