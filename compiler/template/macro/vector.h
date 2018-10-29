@[ macro vector_class(obj) ]
FS::Vector<@(obj.element.classname), @(obj.element.typename)>
@[ endmacro ]

@[ macro vector_declare(fs, obj) ]
class @(obj.classname) : public @( vector_class(obj) )
{
    
public:
    @[ if obj.sentinel ]
    virtual bool is_sentinel(@(obj.element.classname) & self) const override;
    @[ endif ]
    
    @(obj.classname)();
    @(obj.classname)(const FS::Location & lc, const FS::Path * xr, int idx=0, const char * name="");
    static @(obj.classname) * factory(const FS::Location & lc, const FS::Path * xr, 
        const char * buf, unsigned len, int idx=0, const char * name="");
}
@[ endmacro ]
