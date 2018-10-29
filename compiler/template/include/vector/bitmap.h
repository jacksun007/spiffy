class @(obj.classname) : public FS::Bitmap<FS::Container>
{
	@(obj.classname)(const FS::Location & lc, const FS::Path * p, int idx=0, 
	    const char * name="");

public:	
    @(obj.classname)();
    virtual ~@(obj.classname)() {}

	static @(obj.classname) *
    factory(const FS::Location & lc, const FS::Path * p, const char * buf, 
        unsigned len, int idx=0, const char * name="");
};

