@[ from "macro/path_test.cc" import set_path ]

@[ macro skip_amount(field) ]
sizeof(@(field.type)) @[if field.size] * @(field.size) @[ endif ]
@[ endmacro ]

int @(fs.name)::@(obj.classname)::parse_internal(const char * buf, unsigned len)
{
    int bytes_parsed;
    int total_bytes = 0;
    
    @(set_path(fs, "get_path()", "FS::ERR_UNINIT", obj))
    
    @[ if obj.base ]    
    bytes_parsed = @(obj.base.classname)::parse_internal(buf, len);
    if ( bytes_parsed < 0 ) return bytes_parsed;
    
    @[ if obj.when ]
    if ( @(obj.when) )
    {
        buf += bytes_parsed;
        len -= bytes_parsed;
        total_bytes += bytes_parsed;
    }
    else return -EINVAL;    /* not the correct sub-class, end now */
    @[ else ]
    buf += bytes_parsed;
    len -= bytes_parsed;
    total_bytes += bytes_parsed;
    @[ endif ] /* if derived class has a when clause */
    
    @[ endif ] /* if a base class exists */
      
@[ for field in obj.fields ]    
  @[ if field.is_anonymous_union() ]
#error "TODO: parse anonymous union fields"
  @[ else ]
    @[ if field.is_skip() ]
    bytes_parsed = @( skip_amount(field) ); // skip field
    @[ else ]
    bytes_parsed = this->@(field.name).parse(buf, len);
    @[ endif ]
    @[ if field.is_implicit() ]
    if ( bytes_parsed != 0 ) return ( bytes_parsed < 0 ) ? bytes_parsed : -EINVAL;
    @[ elif field.is_flexible_array() ]
    if ( bytes_parsed < 0 ) return bytes_parsed;
    @[ else ]
    if ( bytes_parsed <= 0 ) return bytes_parsed;
    @[ endif ]
    buf += bytes_parsed;
    len -= bytes_parsed;
    total_bytes += bytes_parsed;
  @[ endif ]    
@[ endfor ]
    
    @[ for check in obj.checks ]
    if (!( @(check) ))
        return -EINVAL;
    @[ endfor ]
    
    return total_bytes;
}

@[ if obj.is_container() ]
int @(fs.name)::@(obj.classname)::serialize(char * buf, unsigned len, int options)
@[ else ]
int @(fs.name)::@(obj.classname)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
@[ endif ]
{
    int bytes_written;
    int total_bytes = 0;
    
    @(set_path(fs, "get_path()", "FS::ERR_UNINIT", obj))
@[ if obj.is_container() ]
    FS::FileSystem * fs = path->get_file_system();
@[ endif ]
    (void)fs;
    
    @[ for check in obj.checks ]
    if (!( @(check) ))
        return FS::ERR_CORRUPT;
    @[ endfor ]
    
    @[ if obj.base ]
    @[ if obj.base.is_container() ]
    bytes_written = @(obj.base.classname)::serialize(buf, len, options);
    @[ else ]
    bytes_written = @(obj.base.classname)::serialize(fs, buf, len, options);
    @[ endif ]
    if ( bytes_written < 0 ) return bytes_written;
    
    @[ if obj.when ]
    if ( @(obj.when) )
    {
        buf += bytes_written;
        len -= bytes_written;
        total_bytes += bytes_written;
    }
    else return -EINVAL;    /* not the correct sub-class, end now */
    @[ else ]
    buf += bytes_written;
    len -= bytes_written;
    total_bytes += bytes_written;
    @[ endif ] /* if derived class has a when clause */
    
    @[ endif ] /* if a base class exists */
      
@[ for field in obj.fields ]    
  @[ if field.is_anonymous_union() ]
#error "TODO: serialize anonymous union fields"
  @[ else ]
    @[ if field.is_skip() ]
    bytes_written = @( skip_amount(field) ); // skip field
    @[ else ]
    bytes_written = this->@(field.name).serialize(fs, buf, len, options);
    @[ endif ] 
    @[ if field.is_implicit() ]
    if ( bytes_written != 0 ) return ( bytes_written < 0 ) ? bytes_written : -EINVAL;
    @[ elif field.is_flexible_array() ]
    if ( bytes_written < 0 ) return bytes_written;
    @[ else ]
    if ( bytes_written <= 0 ) return bytes_written;
    @[ endif ]
    buf += bytes_written;
    len -= bytes_written;
    total_bytes += bytes_written;
  @[ endif ]
@[ endfor ]
    
    @[ if obj.size ]
    int actual_size = (int)(@( obj.size ));
    if (total_bytes > actual_size) {
        return -EINVAL;
    }
    total_bytes = actual_size;
    @[ endif ]
    return total_bytes;
}

int @(fs.name)::@(obj.classname)::parse(const char * buf, unsigned len)
{
    int total_bytes;
    @(set_path(fs, "get_path()", "FS::ERR_UNINIT", obj))
    
    @[ if obj.rank == "container" ]
    path->buffer = const_cast<char *>(buf);
    path->length = len;
    @[ endif ]
    
    total_bytes = parse_internal(buf, len);
    if ( total_bytes < 0 ) return total_bytes;
    
    @[ if obj.size ]
    int actual_size = (int)(@( obj.size ));
    if (total_bytes > actual_size) {
        /* 
         * this is technically impossible -- we read more than the actual size
         * some bytes read in the end would have been "garbage"
         */
        return -EINVAL;
    } else if (actual_size > (int)len) {
        return FS::ERR_BUF2SM;
    } else
        set_size(actual_size);
    @[ else ]
    set_size(total_bytes);
    @[ endif ]
    
    /* path already set-up by"set_path() */
    @[ if obj.xref ]
    const @(obj.classname) * temp = path->@(obj.xref);
    path->@(obj.xref) = this;
    temp->decref();
    temp = nullptr;
    @[ endif ]

    @[ if obj.rank == "container" ]
    /* resolve all pointer types now that everything is fully parsed - offset
     * address space pointers *MUST* resolve its pointer prior to fetch */
    resolve();
    path->buffer = nullptr;
    path->length = 0;
    @[ endif ]
    
    return get_size();
}

void @(fs.name)::@(obj.classname)::resolve(void)
{
@[ if obj.base ]
    @( obj.base.classname)::resolve();
@[ endif ]
@[ for field in obj.fields ]
@[ if field.is_pointer() or field.is_object() or field.is_aggregate() ]
    this->@(field.name).resolve();
@[ endif ]
@[ endfor ]
}

