void @(fs.name)::@(field.namespace)::resolve(void)
{
    std::vector<Element>::iterator it = element.begin();

    for ( ; it != element.end(); ++it )
    {
        it->resolve();
    }    
}

