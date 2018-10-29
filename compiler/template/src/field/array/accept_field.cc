int @(fs.name)::@(field.namespace)::accept_fields(FS::Visitor & visitor)
{
    std::vector<Element>::iterator it = element.begin();
    int ret = 0;
    
    for ( ; it != element.end(); ++it )
    {
        if ( (ret = visitor.visit(*it)) != 0 )
            break;
    }
    
    return ret;
}

