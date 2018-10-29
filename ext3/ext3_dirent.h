/*
 * Copyright (C) 2014
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Annotated ext3 directory entries
 */
 
#ifndef EXT3_DIRENT_H
#define EXT3_DIRENT_H
 
#include <jd.h>

#define EXT3_NAME_LEN 255

VECTOR( name = dir_block, type = struct ext3_dir_entry,
        size = BLOCK_SIZE );

FSCONST(type=enum) ext3_file_type {
        EXT3_FT_UNKNOWN  = 0,        /* unknown file type */
        EXT3_FT_REG_FILE = 1,        /* regular file */
        EXT3_FT_DIR      = 2,        /* directory */
        EXT3_FT_CHRDEV   = 3,        /* character device */
        EXT3_FT_BLKDEV   = 4,        /* block device */
        EXT3_FT_FIFO     = 5,        /* FIFO / buffer file */
        EXT3_FT_SOCK     = 6,        /* socket */
        EXT3_FT_SYMLINK  = 7,        /* symbolic link */
};

// the actual size of this struct is rec_len
FSSTRUCT(size=self.rec_len) ext3_dir_entry {
        __le32  inode;                                  /* Inode number */
        __le16  rec_len;                      /* Directory entry length */
        __u8    name_len;                                /* Name length */
        
        FIELD(type=enum ext3_file_type)
        __u8    file_type;
        
        VECTOR(name=name, type=char, size=self.name_len);  /* File name */
        //__u8  name[0];
};

// indexed directory implementation
// 
// see how dx_show_entries works
// 
// http://lxr.free-electrons.com/source/fs/ext3/namei.c?v=3.14#L288
//

/*
FSSTRUCT() fake_dirent
{
        __le32 inode;
        __le16 rec_len;
        u8     name_len;
        u8     file_type;
};

FSSTRUCT() dx_entry
{
        __le32 hash;
        __le32 block;
};

FSSTRUCT() dx_countlimit
{
        __le16 limit;
        __le16 count;
        
        // (jsun): added since this struct is same size is dx_entry
        __le32 padding;     
};

// (jsun): TODO: still need to support file block addrspace for this to work
FSSTRUCT() dx_indirect_entry
{
        __le32 hash;
        
        POINTER(repr=file_block, type=struct dx_node)
        __le32 block;
};

FSSTRUCT(size=BLOCK_SIZE) dx_node
{
        struct fake_dirent fake;
        struct dx_countlimit index;
        
        VECTOR(name=entries, type=struct dx_entry, size=self.index.count-1);
};

FSSTRUCT(size=BLOCK_SIZE, rank=container) dx_root
{
        struct fake_dirent dot;
        char dot_name[4];
        struct fake_dirent dotdot;
        char dotdot_name[4]; 
             
        struct dx_root_info
        {
                __le32 reserved_zero;
                u8 hash_version;
                u8 info_length;         // 8
                u8 indirect_levels;     // max indirect level supported is 1
                u8 unused_flags;
        } info;  
             
        // (jsun): added because the first entry is always actually this 
        struct dx_countlimit index;     
        
        VECTOR(name=indirect_entries, type=struct dx_indirect_entry, 
               size=self.index.count-1, when=self.info.indirect_levels > 0);
        VECTOR(name=entries, type=struct dx_entry, size=self.index.count-1, 
               when=self.info.indirect_levels == 0);
        
        // dx_countlimit is itself an entry, so must subtract 1
        FIELD(count=self.index.count-1)
        struct dx_entry entries[];
};
*/

#endif /* EXT3_DIRENT_H */

