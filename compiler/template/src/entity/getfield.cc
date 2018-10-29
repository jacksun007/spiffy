FS::Entity * @(fs.name)::@(obj.classname)::get_field_by_name(const char * name)
{
    FS::Entity * ret = nullptr;

    @[ if obj.base ]
    ret = @(obj.base.classname)::get_field_by_name(name);
    if ( ret != nullptr) return ret;
    @[ endif ]

    @[ for field in obj.fields ]
    @[ if field.is_anonymous_union() or field.is_skip() ]
    /* TODO: get anonymous union fields (currently not available) */
    @[ else ]
    if ( strcmp(name, "@(field.name)") == 0 ) {
        ret = &this->@(field.name);
    }
    else
    @[ endif ]
    @[ endfor ]
    { /* else return nothing */ }
    
    return ret;
}

