@[ from "macro/pointer.cc" import pointer_ctor, pointer_resolve ]
@[ from "macro/path_test.cc" import set_path ]

@( pointer_ctor(fs, field, field.classname) )

int @(fs.name)::@(field.namespace)::parse(const char * buf, unsigned len)
{
    @(set_path(fs, "self.get_path()", "FS::ERR_UNINIT", field))

    @[ with pointer = field.pointers[0] ]
    @[ if pointer.when ]
    if ( ! ( @(pointer.when) ) )
        this->set_address(0);
    else 
    @[ endif ]
    this->set_address(@(pointer.expr));
    @[ endwith ]
    
    (void)buf;
    (void)len;
    return 0;
}

/* implicit pointers cannot be serialized */
int @(fs.name)::@(field.namespace)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
{
    (void)fs;
    (void)buf;
    (void)len;
    (void)options;
    return 0;
}
    
@( pointer_resolve(fs, field, field.classname) )

