@[ from "macro/integer.h" import integer_class ]
@[ from "macro/implicit.h" import implicit_methods, implicit_ctor ]

class @(field.classname) : public @( integer_class(field) )
{
    @( implicit_ctor(field) );  
    @( implicit_methods() );
} @(field.name);

