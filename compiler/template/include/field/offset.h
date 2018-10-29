@[ from "macro/offset.h" import offset_class, offset_ctor, offset_methods ]

class @(field.classname) : public @(offset_class(field))
{
    @( offset_ctor(fs, field, field.classname) )

    @( offset_methods(field) )
    
} @(field.name);
