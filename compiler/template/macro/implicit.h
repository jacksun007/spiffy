/* remember to add semicolon to end of these macros */
@[ macro implicit_methods() ]
virtual unsigned get_size() const override { return 0; }
virtual int parse(const char * buf, unsigned len) override; 
int serialize(FS::FileSystem * fs, char * buf, unsigned len, int options=0)
@[ endmacro ]

@[ macro implicit_ctor(field) ]
    @(field.object.classname) & self;

public:
    @(field.classname)(@(field.object.classname) & s, int idx=0)
@[ endmacro ]
