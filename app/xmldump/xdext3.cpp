/*
 * xdext3.cpp
 *
 * contains main() for bootstraping to libext3
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 *
 */

#include <libext3.h>
#include <iostream>
#include "xmldump.h"
#include "blockio.h"

using namespace std;

int main(int argc, const char * argv[]) 
{
    XDFormat fmt;
    BlockIO io;
    Ext3 ext3(io);
    const char * filename = "disk.img";
    Ext3::Ext3SuperBlock * super;
    int ret;

    if ( argc != 2 ) {
        cout << "usage: " << argv[0] << " device" << endl;
        return EXIT_FAILURE;
    }
    else {
        filename = argv[1];
    }
    
    if ((ret = io.open(filename)) < 0) {
        cout << argv[0] << ": could not open " << filename << endl;
        return EXIT_FAILURE;
    }
    
    if ((super = (Ext3::Ext3SuperBlock *)ext3.fetch_super())) {
        io.set_block_size(1024 << super->s_log_block_size);
        super->destroy();
    }
    else {
        cout << argv[0] << ": io error or super block is corrupted" << endl;
        return EXIT_FAILURE;
    }
    
     // indirect block ignores
    fmt.add_ignore(IGNORE_TYPE_BY_VALUE, "bit", 0);
    fmt.add_ignore(IGNORE_POINTER_BY_TYPE, "data_block *");
    
    // inode ignore
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "osd1");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "osd2");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "data");
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct ext3_inode", "i_mode", 0);
    
    // block group ignore
    fmt.add_ignore(IGNORE_POINTER_BY_NAME, "bg_block_bitmap");
    fmt.add_ignore(IGNORE_POINTER_BY_NAME, "bg_inode_bitmap");
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct ext3_group_desc", "bg_inode_table", 0);
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "bg_reserved");
                
    // super block ignore
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "s_reserved");
    fmt.add_ignore(IGNORE_POINTER_BY_ASPC, Ext3::AS_FILE);
          
    // journal super block ignore
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "s_padding");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "s_users");
                    
    // indirect block ignores
    fmt.add_ignore(IGNORE_FIELD_BY_VALUE, "ind_block_nr", 0);
                   
    if ((ret = xd_dump_filesystem(ext3, fmt)) < 0)
    {
        return ret;
    }
     
    return EXIT_SUCCESS;
}

