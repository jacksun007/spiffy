@[ include "field/bytearray/common.cc" with context ]

@(fs.name)::@(field.namespace)::@(field.classname)(
    @(field.object.classname) & s, int index) : 
    Buffer("@(field.type) []", FS::TF_UUID, "@(field.name)", index), self(s)
{
}

