@(fs.name)::@(field.namespace)::Element & 
@(fs.name)::@(field.namespace)::operator[](int idx)
{
    static Element bad(self, FS::INVALID_ENTITY);
    
    if (idx < 0 || idx >= (int)element.size()) {
        return bad;
    }

    return element[idx];
}

