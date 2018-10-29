@(fs.name)::@(field.namespace)::@(field.classname)(
    @(field.object.classname) & s, int index) : 
    Entity("@(field.type) []", FS::TF_ARRAY, "@(field.name)", index), self(s)  
{
/* TODO: array elements should be filled here IF size is known */
}

@[ include "field/array/index.cc" with context ]    
 
@[ include "field/array/parse.cc" with context ]
    
@[ include "field/array/accept_field.cc" with context ]

@[ include "entity/entity_compare.cc" with context ]