@(fs.name)::@(obj.classname)::@(obj.classname)(const FS::Location & lc, 
    const FS::Path * p, int idx, const char * name) : FS::Bitmap<FS::Container>(
    lc, nullptr, "@(obj.typename)", FS::TF_ARRAY, name, idx) { (void)p; }

@(fs.name)::@(obj.classname)::@(obj.classname)() : FS::Bitmap<FS::Container>(
    nullptr, "@(obj.typename)", FS::TF_ARRAY) {}
	
