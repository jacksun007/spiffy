@[ from "macro/integer.cc" import integer_ctor ]
@[ from "macro/path_test.cc" import set_path ]

@( integer_ctor(fs, field) )

int @(fs.name)::@(field.namespace)::parse(const char * buf, unsigned len)
{

    @(set_path(fs, "self.get_path()", "FS::ERR_UNINIT", field))

    this->set_value(@(field.expr));
    return get_size();

}

int @(fs.name)::@(field.namespace)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
{
    (void)fs;
    (void)buf;
    (void)len;
    (void)options;
    return 0;
}
 
