@(fs.name)::@(obj.classname)::@(obj.classname)(const FS::Location & lc, 
    const FS::Path * xr, int idx, const char * name) 
    : FS::Extent<@(obj.element.classname)>(lc, &camino, "@(obj.typename)", 
        FS::TF_ARRAY, name, idx), camino(xr)
{}        

@[ from "macro/path_test.cc" import set_path ]

int @(fs.name)::@(obj.classname)::initialize(void)
{
    @(set_path(fs, "get_path()", "FS::ERR_UNINIT", obj))

    int element_size = (@(obj.element.size));
    int count;
    unsigned offset = 0;

    assert(this->location.size > 0);
@[ if obj.count ]
    count = @(obj.count);
@[ else ]
    count = this->location.size / element_size;
@[ endif ]
    
    for (int i = 0; i < count; i++) {
        element.emplace_back(nullptr);
        element_loc.emplace_back(location.aspc, element_size, offset,
            location.addr);
        offset += element_size;
    }
    
    assert((int)element.size() == count);
    assert((int)element_loc.size() == count);
    return 0;
}

@[ from "macro/container.cc" import container_fetch ]

@(fs.name)::@(obj.element.classname) * 
    @(fs.name)::@(obj.classname)::create_element(int idx) const
{
    if (idx < 0 || idx >= (int)element_loc.size())
        return nullptr;
    
    const FS::Location & lc = element_loc[idx];

@[ if obj.is_nested() ]
    return @(obj.element.classname)::factory(lc, &camino, idx);
@[ else ]
    @( container_fetch(fs, obj.element.classname, "lc", "&camino", "idx") );
@[ endif ]    
}

