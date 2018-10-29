@[ from "macro/pointer.h" import pointer_ctor, pointer_resolve, intptr_class ]
@[ from "macro/implicit.h" import implicit_methods ]

class @(field.classname) final : public @(intptr_class(field))
{
    @( pointer_ctor(fs, field, field.classname) )
    @( implicit_methods() );
    @( pointer_resolve(fs, field, field.classname) )
    
} @(field.name);
