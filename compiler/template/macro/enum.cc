@[ macro enum_check(field) ]
@[ if field.category.type == "enum" ]
unsigned long _val = this->to_integer();  
if (
@[ for enumval in field.category.elements ]
    _val != @(enumval.name) && 
@[ endfor ]
1 ) 
{
    /* failed type-safety check */
    return -EINVAL;
}
@[ endif ] /* partial enum doesn't need to be checked */
@[ endmacro ]
