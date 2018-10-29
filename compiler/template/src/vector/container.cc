@[ from "macro/vector.h" import vector_class ]

@(fs.name)::@(obj.classname)::@(obj.classname)(const FS::Location & lc, 
    const FS::Path * xr, int idx, const char * name) 
    : @(vector_class(obj))(lc, const_cast<FS::Path*>(xr), "@(obj.typename)",
        FS::TF_ARRAY, name, idx) {}

@(fs.name)::@(obj.classname)::@(obj.classname)() 
    : @(vector_class(obj))(nullptr, "@(obj.typename)", FS::TF_ARRAY) {}

@[ if obj.sentinel ]
bool @(fs.name)::@(obj.classname)::is_sentinel(@(obj.element.classname) & self) const
{
    return ( @( obj.sentinel ) ); 
}   
@[ endif ]

