/* remember to add semicolon to end of the macro invocation! */

@[ macro bytearray_parse(field_size) ]
int temp = @(field_size);

if (temp < 0)
    return -EINVAL;

if (len >= (unsigned)temp)
{
    if (buf)
        delete [] buf;

    this->size = temp;
    buf = new char[temp+1];
    memcpy(buf, inbuf, temp);
    buf[temp] = '\0';   
    return temp;
}

return FS::ERR_BUF2SM
@[ endmacro ]

@[ macro bytearray_serialize(field_size) ]
int temp = @(field_size);

if (temp < 0)
    return -EINVAL;

if (buf == nullptr)
    return -EINVAL;

if ((unsigned)temp != this->size)
    return -EINVAL;

if (len >= (unsigned)temp)
{
    memcpy(outbuf, buf, temp);
    return this->size;
}

(void)fs;
return FS::ERR_BUF2SM
@[ endmacro ]
