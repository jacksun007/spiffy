@[ from "macro/path_test.cc" import set_path ]

unsigned @(fs.name)::@(field.namespace)::get_size() const
{
@[ if field.size|length == 1 ]
    return element.size() * sizeof(@(field.type));
@[ elif field.size|length == 2 ]
    return element.size() * sizeof(@(field.type))*@(field.size[1]);
@[ else ]
#error "we do not currently support 3-dimensional arrays (or higher)"
@[ endif ]
}
    

int @(fs.name)::@(field.namespace)::parse(const char * buf, unsigned len)
{
    int max_count;
    @(set_path(fs, "self.get_path()", "FS::ERR_UNINIT", field))

    max_count = (int)(@(field.size[0]));
    element.clear();
    element.reserve(max_count);
    
    for ( int i = 0; i < max_count; i++ )
    {
@[ if field.size|length == 1 ]
        if ( len >= sizeof(@(field.type)) ) 
@[ elif field.size|length == 2 ]
        if ( len >= sizeof(@(field.type))*@(field.size[1]) ) 
@[ else ]
#error "we do not currently support 3-dimensional arrays (or higher)"
@[ endif ]
        {
            element.emplace_back(self, i, get_name());
            Element & elem = element.back();
            int byte_parsed = elem.parse(buf, len);
            if (byte_parsed <= 0) return byte_parsed;
            elem.set_element();
            buf += byte_parsed;
            len -= byte_parsed;
        }
        else
            return FS::ERR_BUF2SM;
    }
    
    return get_size();
}

int @(fs.name)::@(field.namespace)::serialize(FS::FileSystem * fs, char * buf, unsigned len, int options)
{
    @(set_path(fs, "self.get_path()", "FS::ERR_UNINIT", field))

    for (int i = 0; i < (int)(@(field.size[0])); i++)
    {
@[ if field.size|length == 1 ]
        if ( len < sizeof(@(field.type)) ) 
@[ elif field.size|length == 2 ]
        if ( len < sizeof(@(field.type))*@(field.size[1]) ) 
@[ else ]
#error "we do not currently support 3-dimensional arrays (or higher)"
@[ endif ]
            return FS::ERR_BUF2SM;
        
        if (i < (int)element.size()) {
            Element & elem = element[i];
            int byte_written = elem.serialize(fs, buf, len, options);
            if (byte_written <= 0) return byte_written;
            buf += byte_written;
            len -= byte_written;
        }
        else 
            return -EINVAL;
    }
    
    return get_size();
}

