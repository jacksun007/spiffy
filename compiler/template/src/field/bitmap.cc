@[ from "macro/path_test.cc" import set_path ]

@(fs.name)::@(field.namespace)::@(field.classname)(@(field.object.classname) & s, 
    int index, const char * n) : FS::Bitmap<FS::Entity>("bitmap", 
    FS::TF_ARRAY, "@(field.name)", index), self(s)
{
    (void)n;
}

unsigned @(fs.name)::@(field.namespace)::get_size() const
{
    @(set_path(fs, "self.get_path()", "0", field))

    return sizeof(@(field.type)) * @(field.size);
}

