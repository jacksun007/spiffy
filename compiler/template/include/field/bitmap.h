class @(field.classname) : public FS::Bitmap<FS::Entity>
{
    @(field.object.classname) & self;

public:
    @(field.classname)(@(field.object.classname) & s, int idx=0, const char * n="");
    
    virtual unsigned get_size() const override;
} @(field.name);

