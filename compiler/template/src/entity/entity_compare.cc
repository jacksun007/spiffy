int
@(fs.name)::@(field.namespace)::compare(const @(field.classname)& other, const Entity& p, FS::Visitor& v)
{
std::vector<Element>::iterator it_this = element.begin();
std::vector<Element>::const_iterator it_other = other.element.begin();

int ret = 0;

for ( ; (it_this != element.end() && it_other != other.element.end()); ++it_this, ++it_other ) {

if ((ret = ((*it_this).compare(*it_other, p, v))) != 0)
break;

}

return ret;
}