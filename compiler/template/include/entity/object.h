class @(obj.classname) : @[ if obj.base ] public @(obj.base.classname) { 
@[ elif obj.is_container() ] public FS::Container {
@[ else ] public FS::Object {
@[ endif ]
@[ if obj.is_referenced() and not obj.base ]
    @(fs.xrefname) camino;
@[ endif ]

protected:
    const @(obj.classname) & self;
    int parse_internal(const char * buf, unsigned len);
    
public: 
@[if obj.is_container() ]
    @(obj.classname)(const FS::Location & lc, const FS::Path * xr, int idx=0, const char * name="");
    @(obj.classname)();
    
    static @(obj.classname) * factory(const FS::Location & lc, const FS::Path * xr, 
        const char * buf, unsigned size, int idx=0, const char * name="");
@[ else ]
    @(obj.classname)(FS::Container & p, const FS::Path * xr, int idx=0, const char * name="");
    @(obj.classname)();
    static @(obj.classname) * factory(FS::Container & p, const FS::Path * xr, const char * buf,
    unsigned size, int idx=0, const char * name="");
@[ endif ]
    @(obj.classname)(const @(obj.classname) & rhs) = delete;
    @(obj.classname)(@(obj.classname) && rhs) = delete;
    virtual ~@(obj.classname)() override;
          
    virtual int parse(const char * buf, unsigned len) override;
@[ if obj.is_container() ]    
    virtual int serialize(char * buf, unsigned len, int options=0) override;
@[ else ]
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0);
@[ endif ]
    
    int compare(@(obj.classname) & other, FS::Visitor & v);

    @[ for field in obj.fields ]
        @[ include "field.h" with context ] 
    @[ endfor ]    

    virtual int accept_fields(FS::Visitor & visitor) override;
    virtual int accept_pointers(FS::Visitor & visitor) override;

    virtual FS::Entity * get_field_by_name(const char * name) override;
    
    virtual void resolve(void) override;
     
}; /* @(obj.classname) */

