@[ from "macro/vector.h" import vector_class ]

@(fs.name)::@(obj.classname)::@(obj.classname)(
    const FS::Location & lc, const FS::Path * xr, int idx, const char * name)
	: @(vector_class(obj))(lc, nullptr, "@(obj.typename)", FS::TF_ARRAY, 
	                       name, idx) { (void)xr; }

