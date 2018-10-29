@[ if obj.is_extent() ]

@(fs.name)::@(obj.classname) * @(fs.name)::@(obj.classname)::factory(
    const FS::Location & lc, const FS::Path * xr, int idx, const char * name)
{
    @(obj.classname) * inst = new @(obj.classname)(lc, xr, idx, name);
    
    if (inst != nullptr) {
        if (inst->initialize() < 0) {
            inst->destroy();
            return nullptr;
        }
    }
              
    return inst;
}

@[ elif obj.is_container() ]

@(fs.name)::@(obj.classname) * @(fs.name)::@(obj.classname)::factory(
    const FS::Location & lc, const FS::Path * xr, const char * buf, unsigned size, 
    int idx, const char * name)
{
    @(obj.classname) * tmp;

    @[ for derived in obj.derived ]
    /* 
     * (jsun): we do not allow polymorphism if derived class inherits
     * base class without a when clause
     */
    @[ if derived.when ]
    tmp = @(derived.classname)::factory(lc, xr, buf, size, idx, name);
    if ( tmp != nullptr ) { return tmp; }
    @[ endif ]
    @[ endfor ]
    
    tmp = new @(obj.classname)(lc, xr, idx, name);
    int bytes_parsed = tmp->parse(buf, size);
    
    if ( bytes_parsed > 0 ) {
        return tmp;
    }
    
    tmp->destroy();
    return nullptr;
}

@[ else ]

@(fs.name)::@(obj.classname) * @(fs.name)::@(obj.classname)::factory(
    FS::Container & p, const FS::Path * xr, const char * buf, unsigned size, int idx,
    const char * name)
{
    @(obj.classname) * tmp;

    @[ for derived in obj.derived ]
    @[ if derived.when ]
    tmp = @(derived.classname)::factory(p, xr, buf, size, idx, name);
    if ( tmp != nullptr ) { return tmp; }
    @[ endif ]
    @[ endfor ]
    
    tmp = new @(obj.classname)(p, xr, idx, name);
    int bytes_parsed = tmp->parse(buf, size);
    
    if ( bytes_parsed > 0 ) {
        return tmp;
    }
    
    delete tmp;
    return nullptr;
}

@[ endif ]
