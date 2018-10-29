int @(fs.name)::@(obj.classname)::compare(@(obj.classname) & other, FS::Visitor & v)
{
    int ret;
@[ for field in obj.fields ]

@[ if not field.is_skip() and not field.is_object() and not (field.enum == "bitmap") ]
    ret = this->@(field.name).compare(other.@(field.name), *this, v);
    if (ret < 0)
        return ret;
@[ elif (not field.is_skip() and field.is_object()) or (field.enum == "bitmap") ]
    ret = this->@(field.name).compare(other.@(field.name), v);
    if (ret < 0)
        return ret;
@[ endif ]

@[ endfor ]
    (void)ret;
    return 0;
}
