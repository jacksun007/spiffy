/* remember to add a semicolon after invoking this macro */

@[ macro container_fetch(fs, classname, locname, pathexpr, index) ]
char * buf = nullptr;
const FS::Path * path = @(pathexpr);
FS::FileSystem * fs;
ssize_t retval;
@(fs.name)::@(classname) * tmp = nullptr;

if (path == nullptr)
    return nullptr;

fs = path->get_file_system();
if (fs == nullptr)
    return nullptr;

retval = fs->io.read(@(locname), buf);                                              
if (buf != nullptr)
{
    if (retval >= @(locname).size)
        tmp = @(fs.name)::@(classname)::factory(@(locname), path, buf, retval, @(index));
        
    delete [] buf;
}

return tmp;
@[ endmacro ]
