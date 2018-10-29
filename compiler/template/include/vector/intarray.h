@[ from "macro/vector.h" import vector_class ]
@[ from "macro/integer.h" import integer_class ]

struct @(obj.element.classname) : public @(integer_class(obj.element))
{
    @(obj.element.classname)(FS::Container & s, int idx=0) : 
        @(integer_class(obj.element))("@(obj.element.type)", FS::TF_ELEMENT, 
        "", idx) { (void)s; }
};
    
class @(obj.classname) : public @(vector_class(obj))
{
	@(obj.classname)(const FS::Location & lc, const FS::Path * xr, int idx=0,
	    const char * name="");
public:

	static @(obj.classname) *
    factory(const FS::Location & lc, const FS::Path * xr, const char * buf, 
        unsigned len, int idx=0, const char * name="");
};
