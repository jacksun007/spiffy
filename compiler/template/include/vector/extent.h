class @(obj.classname) : public FS::Extent<@(obj.element.classname)>
{
    std::vector<FS::Location> element_loc;
    @(fs.xrefname) camino;

    @(obj.classname)(const FS::Location & lc, const FS::Path * xr, 
        int idx=0, const char * name="");
public:

    int initialize(void);
    virtual @(obj.element.classname) * create_element(int idx) const override;
    
    static @(obj.classname) * factory(const FS::Location & lc, 
        const FS::Path * xr, int idx=0, const char * name="");
};

