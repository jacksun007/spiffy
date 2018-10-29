@(fs.name)::@(field.namespace)::@(field.classname)(
    @(field.object.classname) & s, int index) : 
    Buffer("@(field.type) []", FS::TF_CSTRING
    , "@(field.name)", index), self(s)
{
}

@[ include "field/bytearray/common.cc" with context ]

