@[ from "macro/offset.h" import offset_class, offset_save ]
@[ from "macro/pointer.cc" import pointer_namespace, pointer_fetch ]
@[ from "macro/container.cc" import container_fetch ]
@[ from "macro/path_test.cc" import set_path ]

@[ macro offset_ctor(fs, field, _classname) ]

@(fs.name)::@(pointer_namespace(field, _classname))::@(_classname)
(@(field.object.classname) & s, int idx)
    : @( offset_class(field) )(s.get_path(), "@(field.type)", FS::TF_OFFSET, "@(field.name)", idx)
    , self(s) {}

@[ endmacro ]

@[ macro offset_methods(fs, field, _classname) ]

int @(fs.name)::@(pointer_namespace(field, _classname))::create_target() {
    int ret = 0;

@[ for pointer in field.pointers ]
    if ((ret = @(pointer_fetch(pointer, loop.index))()) < 0)
        return ret;
    else if (ret > 0) return ret;
@[ endfor ]
    
    /* can only happen if ret == 0 at some point... */
    assert(target == nullptr);
    return 0;
}

int @(fs.name)::@(pointer_namespace(field, _classname))::save_target(FS::FileSystem * filsys, int options) {
    int ret = 0;

    if (target == nullptr)
        return 0;

@[ for pointer in field.pointers ]
    if ((ret = @(offset_save(pointer, loop.index))(filsys, options)) < 0)
        return ret;
    else if (ret > 0) return ret;
@[ endfor ]
    
    /* cannot happen if target is not null */
    assert(0);
    return -EINVAL;
}

@[ for pointer in field.pointers ]

/* returns 0 when not successful
 * returns size when successful
 */
int @(fs.name)::@(pointer_namespace(field, _classname))::@(pointer_fetch(pointer, loop.index))()
{
    const char * buf = nullptr;
    long len = 0;
    size_t size;
    long value;
    int ret;

    @(set_path(fs, "this->path", "FS::ERR_UNINIT", field))

    @[ if pointer.when ] 
    if ( !(@(pointer.when)) ) 
        return 0;
    @[ endif ]

    @[ if pointer.size ]
    size = (size_t)(@(pointer.size));
    @[ elif pointer.count ]
    size = (size_t)(@(pointer.count))*(@(pointer.target.element.size));
    @[ elif pointer.target.size ]
    size = (size_t)(@(pointer.target.size));
    @[ elif pointer.target.count ]
    size = (size_t)(@(pointer.target.count))*(@(pointer.target.element.size));
    @[ else ]
    size = sizeof(@(pointer.target.typename));
    @[ endif ]
    
    @[ if pointer.expr ]
    value = (@(pointer.expr));
    @[ else ]
    value = *this;
    @[ endif ]
    
    if ((ret = seek_to_position(value, buf, len)) < 0)
        return ret;

    if (len >= (long)size) {
        target = @(pointer.target.classname)::factory(self.get_parent(), self.get_path(), buf, (unsigned)len);
        if (target == nullptr)
            return -EINVAL;
        @[ if pointer.expr ]
        set_value(value);
        @[ endif ]    
        return size;
    }
   
    return 0;
}

int @(fs.name)::@(pointer_namespace(field, _classname))::@(offset_save(pointer, loop.index))(
    FS::FileSystem * filsys, int options)
{
    const char * buf = nullptr;
    long len = 0;
    size_t size;
    long value;
    int ret;

    @(set_path(fs, "this->path", "FS::ERR_UNINIT", field))

    @[ if pointer.when ] 
    if ( !(@(pointer.when)) ) 
        return 0;
    @[ endif ]

    @[ if pointer.size ]
    size = (size_t)(@(pointer.size));
    @[ elif pointer.count ]
    size = (size_t)(@(pointer.count))*(@(pointer.target.element.size));
    @[ elif pointer.target.size ]
    size = (size_t)(@(pointer.target.size));
    @[ elif pointer.target.count ]
    size = (size_t)(@(pointer.target.count))*(@(pointer.target.element.size));
    @[ else ]
    size = sizeof(@(pointer.target.typename));
    @[ endif ]
    
    @[ if pointer.expr ]
    value = (@(pointer.expr));
    @[ else ]
    value = *this;
    @[ endif ]
    
    if ((ret = seek_to_position(value, buf, len)) < 0)
        return ret;

    if (len >= (long)size) {
        return static_cast<@(pointer.target.classname) *>(target)->serialize(
            filsys, const_cast<char *>(buf), len, options);
    }
   
    return FS::ERR_BUF2SM;
}

@[ endfor ]

@[ endmacro ]

