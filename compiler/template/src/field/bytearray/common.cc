@[ from "macro/bytearray.cc" import bytearray_parse, bytearray_serialize ]

int @(fs.name)::@(field.namespace)::parse(const char * inbuf, unsigned len)
{  
    @( bytearray_parse(field.size) );
}

int @(fs.name)::@(field.namespace)::serialize(FileSystem * fs, char * outbuf, unsigned len, int options)
{
    @( bytearray_serialize(field.size) );
}

