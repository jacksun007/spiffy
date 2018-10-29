/*
 * xdtestfs.cpp
 *
 * contains main() for bootstraping to libtestfs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#include <libtestfs.h>
#include <iostream>
#include "xmldump.h"
#include "blockio.h"

using namespace std;

int main(int argc, const char * argv[]) 
{
    XDFormat fmt;
    BlockIO io;
    TestFS testfs(io);
    const char * filename = "disk.img";
    int ret;

    if (argc != 2) {
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
    
    io.set_block_size(BLOCK_SIZE);
    
    // general ignore
    fmt.add_ignore(IGNORE_TYPE_BY_VALUE, "bit", 0);
    
    // ignore data blocks
    fmt.add_ignore(IGNORE_POINTER_BY_TYPE, "data_block *");
    
    // ignore indirect block pointers
    fmt.add_ignore(IGNORE_TYPE_BY_VALUE, "u32", 0);
    
    // ignore free inode
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct dinode", "i_type", 0);
 
    if ( xd_dump_filesystem(testfs, fmt) < 0 )
    {
        return EXIT_FAILURE;
    }
     
    return EXIT_SUCCESS;
}

