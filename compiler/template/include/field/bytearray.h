class @(field.classname) : public FS::Buffer
{
    @(field.object.classname) & self;

public:
    @(field.classname)(@(field.object.classname) & s, int index=0);

    virtual int parse(const char * buf, unsigned len) override;
    int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0);

} @(field.name);


