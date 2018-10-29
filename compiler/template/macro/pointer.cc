@[ from "macro/pointer.h" import intptr_class ]
@[ from "macro/container.cc" import container_fetch ]
@[ from "macro/path_test.cc" import set_path ]

@[ macro pointer_namespace(field, _classname) ]
@[ if field.classname != _classname ]
@(field.namespace)::@(_classname)
@[ else ]
@(field.namespace)
@[ endif ]
@[ endmacro ]

@[ macro pointer_ctor(fs, field, _classname) ]

@(fs.name)::@(pointer_namespace(field, _classname))::@(_classname)
(@(field.object.classname) & s, int idx, const char * n)
    : @( intptr_class(field) )("@(field.type)", 0, 
    @[ if field.name|length > 0 ]"@(field.name)" @[ else ] n @[ endif ], idx)
    , self(s)
{ (void)n; }

@[ endmacro ]

@[ macro pointer_fetch(pointer, index) ]
fetch_@(pointer.target.classname)_@(index)@[ endmacro ]

@[ macro pointer_resolve(fs, field, _classname) ]
@[ for pointer in field.pointers ]

@(fs.name)::@(pointer.target.classname) * 
@(fs.name)::@(pointer_namespace(field, _classname))::@(pointer_fetch(pointer, loop.index))
() const
{
    @[ if pointer.target.is_extent() ]

    /* nested container reads from disk on-demand (during accept) */
    return @(fs.name)::@(pointer.target.classname)::factory(location, self.get_path());

    @[ elif pointer.addrspace.name == "offset" ]      
#error "(jsun) offset pointer in @(field.object.typename)::@(field.name) should inherit FS::Offset!"    
    @[ else ]
    @( container_fetch(fs, pointer.target.classname, "location", "self.get_path()", "0") );
    @[ endif ]
}
    
@[ endfor ]

void @(fs.name)::@(pointer_namespace(field, _classname))::resolve()
{
    @(set_path(fs, "self.get_path()", " ", field))
    
    @[ for pointer in field.pointers ]
    if ( location.addr != (unsigned long)(@(pointer.addrspace.null))
        @[ if pointer.when ] && ( @(pointer.when) ) @[ endif ] )
    {
        /* for typeid */
        this->ptr_type = @(pointer.container.typeid);
        
        /* for size */
        @[ if pointer.size ]
        location.size = (size_t)(@(pointer.size));
        @[ elif pointer.count ]
        location.size = (size_t)(@(pointer.count))*(@(pointer.target.element.size));
        @[ elif pointer.target.size ]
        location.size = (size_t)(@(pointer.target.size));
        @[ elif pointer.target.count ]
        location.size = (size_t)(@(pointer.target.count))*(@(pointer.target.element.size));
        @[ else ]
        location.size = sizeof(@(pointer.target.typename));
        @[ endif ]
        
        location.aspc = @(pointer.addrspace.enumname);    
          
        /* for type object */
        set_type("@(pointer.type) *");
        @[ if field.is_implicit() ]
        set_flags(FS::TF_IMPLICIT);
        @[ endif ]
        
        /* fetch function */
        fetch_func = (fetch_f)&@(_classname)::@(pointer_fetch(pointer, loop.index));
    }
    else
    @[ endfor ]
    {}
    
    /* for debugging purpose */
    resolved = true;
}

@[ endmacro ]

@[ macro pointer_methods(fs, field, _classname) ]

@( pointer_ctor(fs, field, _classname) )
  
@( pointer_resolve(fs, field, _classname) )

@[ endmacro ]

