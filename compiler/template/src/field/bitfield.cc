@[ from "macro/integer.cc" import bitfield_ctor, integer_assign ]

@( bitfield_ctor(fs, field) )

int @(fs.name)::@(field.namespace)::accept_fields(FS::Visitor & visitor)
{
    FS::Bit bit;
    unsigned long val = this->to_integer();
    int ret = 0;

    @[ for element in field.category.elements ]
    bit = FS::Bit(@(element.name) & val, "@(element.name)");
    if( (ret = visitor.visit(bit)) != 0 )
        return ret;
    @[ endfor ]
    
    return ret;
}

@( integer_assign(fs, field) )
