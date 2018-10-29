@[ from "macro/pointer.cc" import pointer_methods ]

@( pointer_methods(fs, field, "Element") )

@[ include "field/array/index.cc" with context ]    

@(fs.name)::@(field.namespace)::@(field.classname)(@(field.object.classname) & s, int idx)
    : FS::Entity("@(field.type) []", FS::TF_ARRAY, "@(field.name)", idx)
    , self(s) 
{}

@[ include "field/array/parse.cc" with context ]
    
@[ include "field/array/accept_field.cc" with context ]

int @(fs.name)::@(field.namespace)::accept_pointers(FS::Visitor & visitor)
{
    std::vector<Element>::iterator it = element.begin();
    int ret = 0;
    
    for ( ; it != element.end(); ++it )
    {
        if ( (ret = visitor.visit(*it)) != 0 )
            break;
    }
    
    return ret;
}

@[ include "field/array/resolve.cc" with context ]

@[ include "entity/entity_compare.cc" with context ]
  
