@[ from "macro/pointer.cc" import pointer_fetch ]

@[ macro offset_class(f) ]
FS::Offset<@(f.type) @[ if f.is_big_endian() ], FS::TF_BIGENDIAN @[ endif ]>
@[ endmacro ]

@[ macro offset_save(offset, index) ]
save_@(offset.target.classname)_@(index)@[ endmacro ]

@[ macro offset_ctor(fs, field, _classname) ]
    @(field.object.classname) & self;

public:
    @(_classname)(@(field.object.classname) & s, int idx=0);
  
    virtual int create_target() override;
    virtual int save_target(FS::FileSystem * filsys, int options) override;
@[ endmacro ]

@[ macro offset_methods(field) ]
@[ for pointer in field.pointers ]
    int @(pointer_fetch(pointer, loop.index))(); 
    int @(offset_save(pointer, loop.index))(FS::FileSystem * filsys, int options); 
@[ endfor ]
@[ endmacro ]    
