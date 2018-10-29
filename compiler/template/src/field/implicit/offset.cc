@[ from "macro/offset.cc" import offset_ctor, offset_methods ]
@[ from "macro/path_test.cc" import set_path ]

@( offset_ctor(fs, field, field.classname) )

int @(fs.name)::@(field.namespace)::parse(const char * buf, unsigned len)
{
    int ret;
    (void)buf;
    (void)len;
    
    if ((ret = create_target()) <= 0)
        return ret;
        
    return get_size();
}

/* serialize target but not the implicit pointer itself */
int @(fs.name)::@(field.namespace)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
{
    int ret = save_target(fs, options);
    
    if (ret < 0)
        return ret;
    
    (void)buf;
    (void)len;
    return 0;
}
 
@( offset_methods(fs, field, field.classname) )
 
