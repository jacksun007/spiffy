@[ macro pointer_ctor(fs, field, _classname) ]
    @(field.object.classname) & self;

public:
    @(_classname)(@(field.object.classname) & s, int idx=0, const char * n="");
@[ endmacro ]

@[ macro pointer_resolve(fs, field, _classname) ]
    virtual void resolve(void) override;
    
private:
    @[ for pointer in field.pointers ]
    @(fs.name)::@(pointer.target.classname) * 
    fetch_@(pointer.target.classname)_@(loop.index)() const;
    @[ endfor ]

@[ endmacro ]

@[ macro intptr_class(f) ]
FS::IntPtr<@(f.type) @[ if f.is_big_endian() ], FS::TF_BIGENDIAN @[ endif ]>
@[ endmacro ]

@[ macro pointer_declare(fs, field, _classname) ]

class @(_classname) final : public @( intptr_class(field) )
{
    @( pointer_ctor(fs, field, _classname) )

    @(pointer_resolve(fs, field, _classname))   
}

@[ endmacro ]

