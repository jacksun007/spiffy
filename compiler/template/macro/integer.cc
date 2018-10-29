@[ from "macro/integer.h" import integer_class, bitfield_class ]

@[ macro generic_ctor(fs, field, cls) ]
@(fs.name)::@(field.namespace)::@(field.classname)(
    @(field.object.classname) & s, int index) : @(cls)("@(field.type)", 
    FS::TF_INTEGRAL @[ if flags ] | @(flags) @[ endif ]
    @[ if field.is_big_endian() ]     | FS::TF_BIGENDIAN @[ endif ]
    @[ if field.is_signed() ]         | FS::TF_SIGNED    @[ endif ]
    @[ if field.enum == "timestamp" ] | FS::TF_TIMESTAMP 
    @[ elif field.is_enum() ] @[ if field.category.type == "flag" ] | FS::TF_BITFIELD
    @[ else ] | FS::TF_ENUM @[ endif ] @[ endif ]
@[ if field.is_implicit() ] | FS::TF_IMPLICIT, "@(field.name)", index)
    , self(s) {} 
@[ else ]
    , "@(field.name)", index)
{
    (void)s;
}
@[ endif ]
@[ endmacro ]

@[ macro integer_ctor(fs, field) ]
@( generic_ctor(fs, field, integer_class(field)) )
@[ endmacro ]

@[ macro bitfield_ctor(fs, field) ]
@( generic_ctor(fs, field, bitfield_class(field)) )
@[ endmacro ]

@[ macro integer_assign(fs, field) ]
@(field.type) & @(fs.name)::@(field.namespace)::operator=(@(field.type) rhs)
{
    return set_value(rhs);
}
@[ endmacro ]
