@[ from "macro/array.h" import array_class ]

@[ if field.sentinel ]
bool @(fs.name)::@(field.namespace)::is_sentinel(@(field.type.classname) & self) const
{
    return ( @(field.sentinel) ) ;
}
@[ endif ]
     
@(fs.name)::@(field.namespace)::@(field.classname)(@(field.object.classname) & s, int idx) 
    : @(array_class(field))("@(field.type) []", FS::TF_ARRAY, "@(field.name)", idx)
    , self(s)   
{}

@(fs.name)::@(field.type.classname) *
@(fs.name)::@(field.namespace)::create_element(int idx, const char * name)
{
    return new @(field.type.classname)(self.get_parent(), self.get_path(), idx, name);
}

@[ if field.size ]
int @(fs.name)::@(field.namespace)::get_count() const
{
    return ( @(field.size) );
}
@[ endif ]

