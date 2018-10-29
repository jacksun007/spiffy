@[ from "macro/bytearray.cc" import bytearray_parse, bytearray_serialize ]

@(fs.name)::@(field.namespace)::Element::Element(
    @(field.object.classname) & s, int idx, const char * n) 
    : FS::Buffer("@(field.type) [" stringify(@(field.size[1])) "]", 
      FS::TF_CSTRING | FS::TF_ELEMENT, n, idx)
{
    (void)s;
}

int 
@(fs.name)::@(field.namespace)::Element::parse(const char * inbuf, unsigned len)
{
    @( bytearray_parse(field.size[1]) );
}

int 
@(fs.name)::@(field.namespace)::Element::serialize(FS::FileSystem * fs, char * outbuf, unsigned len, int options)
{
    @( bytearray_serialize(field.size[1]) );
}

@[ include "field/array/index.cc" with context ]    

@(fs.name)::@(field.namespace)::@(field.classname)(
    @(field.object.classname) & s, int index) : 
    FS::Entity("@(field.type) [][" stringify(@(field.size[1])) "]", FS::TF_ARRAY
    , "@(field.name)", index), self(s)   
{
    /* TODO: array elements should be filled here IF size is known */
}

@[ include "field/array/parse.cc" with context ]
    
@[ include "field/array/accept_field.cc" with context ]

@[ include "entity/entity_compare.cc" with context ]
