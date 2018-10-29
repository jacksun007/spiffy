    @(field.object.classname) & self;
    std::vector<Element> element;

public:
    @(field.classname)(@(field.object.classname) & s, int idx=0);
    Element & operator[](int idx);
    virtual unsigned get_size() const override;

