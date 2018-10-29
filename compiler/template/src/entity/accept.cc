int @(fs.name)::@(obj.classname)::accept_pointers(FS::Visitor & visitor)
{
    int ret = 0;
    
    @[ if obj.base ]
    if ( (ret = @(obj.base.classname)::accept_pointers(visitor) ) != 0 )
        return ret;
    @[ endif ]
    
    @[ for field in obj.fields ]
    @[ if field.is_object() or field.is_aggregate() ]
    if ( (ret = @(field.name).accept_pointers(visitor) ) != 0 )
        return ret;
    @[ elif field.pointers|length > 0 ]
    @[ if field.size > 1 ]
    if ( (ret = @(field.name).accept_pointers(visitor) ) != 0 )
        return ret;
    @[ elif field.pointers[0].repr == "offset" ]
    if ( (ret = @(field.name).accept_pointers(visitor) ) != 0 )
        return ret;
    @[ else ]
    if ( (ret = visitor.visit(@(field.name))) != 0 )
        return ret;
    @[ endif ]
    @[ endif ]
    @[ endfor ]
    
    (void)visitor;
    return ret;
}

int @(fs.name)::@(obj.classname)::accept_fields(FS::Visitor & visitor)
{
    int ret = 0;
    
    @[ if obj.base ]
    if ( (ret = @(obj.base.classname)::accept_fields(visitor) ) != 0 )
        return ret;
    @[ endif ]
    
    @[ for field in obj.fields ]
    @[ if field.is_skip() ] /* nothing */ @[ else ]
    if ( (ret = visitor.visit(@(field.name))) != 0 )
        return ret;
    @[ endif ]
    @[ endfor ]
    
    (void)visitor;
    return ret;
}

