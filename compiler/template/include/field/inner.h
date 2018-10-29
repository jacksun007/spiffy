class @(field.classname) : public FS::Entity
{
    unsigned size;  

public:
    @[ for child in field.fields ]
    @[ with field=child ]
        @[ include "field.h" with context ]
    @[ endwith ]
    @[ endfor ]

    virtual unsigned get_size() const override { return this->size; }
    virtual int parse(const char * buf, unsigned len) override;
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0); 

    virtual int accept_fields(FS::Visitor & visitor) override;
    virtual int accept_pointers(FS::Visitor & visitor) override;

    /* resolves all pointer types post-parse */
    virtual void resolve(void) override;    
    
    @(field.classname)(@(field.object.classname) & s, int index=0);

    int compare(const @(field.classname)& other, const Entity& p, FS::Visitor& v);

} @(field.name);

