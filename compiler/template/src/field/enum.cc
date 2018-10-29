@[ from "macro/integer.h" import integer_class ]
@[ from "macro/integer.cc" import integer_ctor, integer_assign ]
@[ from "macro/enum.cc" import enum_check ]

@( integer_ctor(fs, field) )

int @(fs.name)::@(field.namespace)::parse(const char * buf, unsigned len)
{
    int bytes_parsed = @( integer_class(field) )::parse(buf, len);
    if ( bytes_parsed <= 0 ) return -EINVAL;
 
    @( enum_check(field) )
    return bytes_parsed;
}

int @(fs.name)::@(field.namespace)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
{
    @( enum_check(field) )
    return @( integer_class(field) )::serialize(fs, buf, len, options);
}
    
const char * @(fs.name)::@(field.namespace)::to_string(char * buf, unsigned len) const
{ 
    (void)buf;
    (void)len;
    return @(field.category.classname)::enum_to_name(to_integer());
}

@( integer_assign(fs, field) )
