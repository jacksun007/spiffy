/*
 * xdbtrfs.cpp
 *
 * contains main() for bootstraping to libbtrfs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2016, University of Toronto
 */

#include <libbtrfs.h>
#include <iostream>
#include "xmldump.h"
#include "blockio.h"

using namespace std;

int main(int argc, const char * argv[]) 
{
    XDFormat fmt;
    BlockIO io;
    Btrfs btrfs(io);
    Btrfs::BtrfsSuperBlock * super;
    const char * filename = nullptr;
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
    
    if ((super = (Btrfs::BtrfsSuperBlock *)btrfs.fetch_super())) {
        io.set_block_size(super->sectorsize);
        super->destroy();
    }
    else {
        cout << argv[0] << ": io error or super block is corrupted" << endl;
        return EXIT_FAILURE;
    }

    // general ignore
    fmt.add_ignore(IGNORE_TYPE_BY_VALUE, "bit", 0);
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "csum");
    fmt.add_ignore(IGNORE_POINTER_BY_TYPE, "data_extent *");

    // super block ignore
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "sys_chunk_array");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "reserved");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "unused_64");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "unused_8");

    if ( xd_dump_filesystem(btrfs, fmt) < 0 )
    {
        return EXIT_FAILURE;
    }
     
    return EXIT_SUCCESS;
}

