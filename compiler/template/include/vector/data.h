class @(obj.classname) : public FS::Data
{
public:
    using Data::Data;
    static @(obj.classname) * factory(const FS::Location & lc, const FS::Path * p, 
        const char * buf, unsigned len, int idx=0, const char * name="");
};

