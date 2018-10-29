/*
 * common.c
 *
 * contains common functionalities shared across files
 *
 */
 
 #include "common.h"
 
int
str_to_long(long * lptr, const char * str)
{
    char * end; 
    size_t val = strtol(str, &end, 10);
    
    if ( end != NULL && strlen(end) > 0 ) {
            return -EINVAL;
    }
    
    *lptr = val;
    return 0;
}
 
int
str_to_ulong(unsigned long * ulptr, const char * str)
{
    char * end; 
    size_t val = strtoul(str, &end, 10);
    
    if ( end != NULL && strlen(end) > 0 ) {
            return -EINVAL;
    }
    
    *ulptr = val;
    return 0;
}
 
