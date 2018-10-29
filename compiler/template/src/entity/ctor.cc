@(fs.name)::@(obj.classname)::@(obj.classname)(
@[ if obj.is_container() ]
const FS::Location & a1, const FS::Path * xr, int idx, const char * name)
@[ else ]
FS::Container & a1, const FS::Path * xr, int idx, const char * name)
@[ endif ]
@[ if obj.base ]
    : @(obj.base.classname)(a1, xr, idx, name)
@[ elif obj.is_container() ]
    @[ if obj.is_referenced() ]
    : FS::Container(a1, &camino, "@(obj.typename)", FS::TF_STRUCT
      @[ if fs.root == obj ] | FS::TF_SUPER @[ endif ], name, idx)
    @[ else ]
    : FS::Container(a1, const_cast<FS::Path*>(xr), "@(obj.typename)", FS::TF_STRUCT
    @[ if fs.root == obj ] | FS::TF_SUPER @[ endif ], name, idx)
    @[ endif ]
@[ else ]
    @[ if obj.is_referenced() ]
    : FS::Object(a1, &camino, "@(obj.typename)", FS::TF_STRUCT, name, idx)
    @[ else ]
    : FS::Object(a1, const_cast<FS::Path*>(xr), "@(obj.typename)", FS::TF_STRUCT, name, idx)
    @[ endif ]
@[ endif ]      
@[ if obj.is_referenced() and not obj.base ]
    , camino(xr)
@[ endif ]
    , self(*this)
    @[ for field in obj.fields ]
        @[ if field.is_skip() ] /* nothing */
        @[ elif field.is_object() and not field.is_array() ]
        , @(field.name)(get_parent(), xr, 0, "@(field.name)")
        @[ else ]
        , @(field.name)(*this)
        @[ endif ]
    @[ endfor ]
{
@[ if obj.base ] 
    set_type("@(obj.typename)");
@[ endif ]
@[ if not obj.is_container() ]
    /* this is an embedded object with a name */
    if (name != nullptr && name[0] != '\0') {
        set_flags(FS::TF_FIELD);
    }
@[ endif ]
} 

/* default constructor */
@(fs.name)::@(obj.classname)::@(obj.classname)()
@[ if obj.base ]
    : @(obj.base.classname)()  
@[ elif obj.is_container() ]
    @[ if obj.is_referenced() ]
    : FS::Container(&camino, "@(obj.typename)", FS::TF_STRUCT
    @[ if fs.root == obj ] | FS::TF_SUPER @[ endif ])
    @[ else ]
    : FS::Container(nullptr, "@(obj.typename)", FS::TF_STRUCT
    @[ if fs.root == obj ] | FS::TF_SUPER @[ endif ])
    @[endif ]
@[ else ]
    @[ if obj.is_referenced() ]
    : FS::Object(&camino, "@(obj.typename)", FS::TF_STRUCT)
    @[ else ]
    : FS::Object(nullptr, "@(obj.typename)", FS::TF_STRUCT)
    @[ endif ]
@[ endif ]   
    , self(*this)
    @[ for field in obj.fields ]
        @[ if field.is_skip() ] /* nothing */
        @[ elif field.is_object() and not field.is_array() ]
        , @(field.name)(get_parent(), nullptr, 0, "@(field.name)")
        @[ else ]
        , @(field.name)(*this)
        @[ endif ]
    @[ endfor ]
{
    assert(!is_valid());
} 

@(fs.name)::@(obj.classname)::~@(obj.classname)()
{
@[ if obj.xref ]
    @(fs.xrefname) * path = static_cast<@(fs.xrefname) *>(get_path());
    
    if (path == nullptr)
        return;
        
    /* don't decref again */
    if (path->@(obj.xref) == this)
        path->@(obj.xref) = nullptr;
@[ endif ]
}


