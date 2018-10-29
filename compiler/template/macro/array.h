@[ macro array_class(field) ]
FS::Array<@(field.type.classname), @(field.type.typename)>
@[ endmacro ]

@[ macro array_declare(fs, field) ]
class @(field.classname) : public @( array_class(field) )
{
    @(field.object.classname) & self;

@[ if field.size ]
protected:
    virtual int get_count() const override;
@[ endif ]
    
public:
    @[ if field.sentinel ]
    virtual bool is_sentinel(@(field.type.classname) & self) const override;
    @[ endif ]
    
    virtual @(field.type.classname) * create_element(int i, const char * n) override;
    @(field.classname)(@(field.object.classname) & s, int idx=0);
}
@[ endmacro ]
