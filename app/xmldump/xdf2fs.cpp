/*
 * xdf2fs.cpp
 *
 * contains main() for bootstraping to libf2fs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2016, University of Toronto
 */

#include <libf2fs.h>
#include <iostream>
#include "xmldump.h"
#include "blockio.h"

using namespace std;

int main(int argc, const char * argv[]) 
{
    XDFormat fmt;
    BlockIO io;
    F2FS f2fs(io);
    const char * filename = "disk.img";
    int ret;

    if (argc != 2)
    {
        cout << "usage: " << argv[0] << " device" << endl;
        return EXIT_FAILURE;
    }
    else
    {
        filename = argv[1];
    }
    
    if ((ret = io.open(filename)) < 0) {
        cout << argv[0] << ": could not open " << filename << endl;
        return EXIT_FAILURE;
    }
    
    /* f2fs always uses this block size */
    io.set_block_size(F2FS_BLKSIZE);
    
    // general ignore
    fmt.add_ignore(IGNORE_TYPE_BY_VALUE, "bit", 0);
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "padding");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "reserved");
    fmt.add_ignore(IGNORE_POINTER_BY_TYPE, "f2fs_data_block *");
    
    // super block ignores
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "volume_name");
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "encrypt_pw_salt");
    fmt.add_ignore(IGNORE_EMPTY_STRING, "extension_list");
    
    // sit entry ignores
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct f2fs_sit_entry", "vblocks", 0);
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "valid_map");

    // nat extent ignore (suppresses huge output)
    fmt.add_ignore(IGNORE_TYPE, "f2fs_nat_extent");
    
    // nat entry ignores
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct f2fs_nat_entry", "ino", 0);
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct nat_journal_entry", "nid", 0);
    
    // inode ignores 
    fmt.add_ignore(IGNORE_FIELD_BY_VALUE, "i_addr", 0);
    fmt.add_ignore(IGNORE_FIELD_BY_VALUE, "nid_direct", 0);
    fmt.add_ignore(IGNORE_FIELD_BY_VALUE, "nid_indirect", 0);

    // summary ignore
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct f2fs_data_summary", "nid", 0);
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct f2fs_node_summary", "nid", 0);

    // dir block ignore
    fmt.add_ignore(IGNORE_OBJECT_BY_FIELD, "struct f2fs_dir_entry", "ino", 0);
    fmt.add_ignore(IGNORE_EMPTY_STRING, "filename");

    // checkpoint ignore
    fmt.add_ignore(IGNORE_FIELD_BY_NAME, "alloc_type");
    
    if ((ret = xd_dump_filesystem(f2fs, fmt)) < 0) {
        return ret;
    }
     
    return EXIT_SUCCESS;
}

