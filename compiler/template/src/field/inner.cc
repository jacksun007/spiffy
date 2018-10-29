@(fs.name)::@(field.namespace)::@(field.classname)(
    @(field.object.classname) & s, int index) : 
    Entity("@(field.type)", FS::TF_STRUCT, "@(field.name)", index)
    @[ for child in field.fields ]
    , @(child.name)(s)
    @[ endfor ]    
{
}

int @(fs.name)::@(field.namespace)::accept_fields(FS::Visitor & visitor)
{
    int ret = 0;
    @[ for child in field.fields ]
    if ( (ret = visitor.visit(@(child.name)) ) < 0 )
        return ret;
    @[ endfor ]
    return ret;
}

int @(fs.name)::@(field.namespace)::accept_pointers(FS::Visitor & visitor)
{
    int ret = 0;
    @[ for child in field.fields ]
    @[ if child.is_object() or child.is_aggregate() ]
    if ( (ret = @(child.name).accept_pointers(visitor) ) < 0 )
        return ret;
    @[ elif child.pointers|length > 0 ]
    @[ if child.size > 1 ]
    if ( (ret = @(child.name).accept_pointers(visitor) ) < 0 )
        return ret;
    @[ elif child.pointers[0].repr != "offset" ]
    if ( (ret = visitor.visit(@(child.name))) < 0 )
        return ret;
    @[ endif ]
    @[ endif ]
    @[ endfor ]
    return ret;         
}

/* TODO: this is the exact same code as src/obj/parse.cc's resolve() */
void @(fs.name)::@(field.namespace)::resolve(void)
{
    @[ for child in field.fields ]
    @[ if child.is_pointer() or child.is_object() or child.is_aggregate() ]
        this->@(child.name).resolve();
    @[ endif ]
    @[ endfor ]
}

int @(fs.name)::@(field.namespace)::parse(const char * buf, unsigned len)
{
@[ if field.is_struct() ]

    int bytes_parsed;
    
    this->size = 0;
    @[ for child in field.fields ]
    bytes_parsed = @(child.name).parse(buf, len);
    if ( bytes_parsed <= 0 ) return bytes_parsed;
    buf += bytes_parsed;
    this->size += bytes_parsed;
    len -= bytes_parsed;
    @[ endfor ]
    
    return this->size;

@[ elif field.is_union() ]

    int bytes_parsed;
    int max_bytes = 0;

    @[ for child in field.fields ]
    bytes_parsed = @(child.name).parse(buf, len);
    if ( bytes_parsed <= 0 ) return bytes_parsed;
    if ( max_bytes < bytes_parsed ) max_bytes = bytes_parsed;
    @[ endfor ]
    
    this->size = max_bytes;
    return max_bytes; 
    
@[ else ]

#error "inner aggregate type @(field.name) is neither a struct nor union"  
  
@[ endif ]    
}

int @(fs.name)::@(field.namespace)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
{
    int bytes_written;

@[ if field.is_struct() ]
    unsigned total_bytes = 0;
    
    @[ for child in field.fields ]
    bytes_written = @(child.name).serialize(fs, buf, len, options);
    if (bytes_written <= 0) return bytes_written;
    buf += bytes_written;
    len -= bytes_written;
    total_bytes += bytes_written;
    @[ endfor ]
    
    if (total_bytes != this->size)
        return -EINVAL;
    
@[ elif field.is_union() ]
    @[ for child in field.fields ]
    @[ if loop.first ]
    bytes_written = @(child.name).serialize(fs, buf, len, options);
    if (bytes_written <= 0) return bytes_written;
    @[ endif ]
    @[ endfor ]
    
    // TODO: currently serializes first union member, should actually
    //       serialize the only "valid" one OR the biggest one
    if (bytes_written != (int)this->size)
        return FS::ERR_UNIMP;
    
@[ else ]
#error "inner aggregate type @(field.name) is neither a struct nor union"  
  
@[ endif ]  
    return this->size;  
}

@[ for child in field.fields ]
    @[ with field=child ]
        @[ include "field.cc" with context ]
    @[ endwith ]
@[ endfor ]

int @(fs.name)::@(field.namespace)::compare(const @(field.classname)& other, const Entity& p, FS::Visitor& v) {

    int ret;

    @[ for child in field.fields ]

        @[ if not child.is_skip() and not child.is_object() and not (child.enum == "bitmap") ]
        ret = this->@(child.name).compare(other.@(child.name), *this, v);
        if (ret < 0)
        return ret;
        @[ elif (not child.is_skip() and child.is_object()) or (child.enum == "bitmap") ]
        ret = this->@(child.name).compare(other.@(child.name), v);
        if (ret < 0)
        return ret;
        @[ endif ]

    @[ endfor ]

    (void)ret;
    return 0;
}
