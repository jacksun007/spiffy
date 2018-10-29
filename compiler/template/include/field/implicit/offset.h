@[ from "macro/offset.h" import offset_class, offset_ctor, offset_methods ]
@[ from "macro/implicit.h" import implicit_methods ]

class @(field.classname) : public @(offset_class(field))
{
    @( offset_ctor(fs, field, field.classname) )
    @( implicit_methods() );
    @( offset_methods(field) )
    
} @(field.name);
