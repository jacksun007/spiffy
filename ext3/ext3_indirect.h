/*
 * Copyright (C) 2014
 * University of Toronto
 * 
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@utoronto.ca
 *
 * Annotated ext3 indirect blocks
 */

#ifndef EXT3_INDIRECT_H
#define EXT3_INDIRECT_H

#include <jd.h>
#include "ext3_dirent.h"
 
VECTOR(name=data_block, type=data, size=BLOCK_SIZE);
 
// Note: BLOCK_SIZE is defined in ext3_blkgrp.h

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) 
{
        FIELD(count=BLOCK_SIZE/sizeof(__le32))
        POINTER(repr=block, type=dir_x2_indirect_block)
        __le32 ind_block_nr[];
} dir_x3_indirect_block;

VECTOR( name=dir_x2_indirect_block, type=struct dir_x2_indirect, 
        size=BLOCK_SIZE );

FSSTRUCT() dir_x2_indirect {
        POINTER ( repr=block, type=dir_indirect_block )
        __le32 ind_block_nr ;
};

VECTOR( name=dir_indirect_block, type=struct dir_indirect, 
        size=BLOCK_SIZE );

FSSTRUCT() dir_indirect {
        POINTER ( repr=block, type=dir_block )
        __le32 ind_block_nr ;
};

VECTOR( name=data_x3_indirect_block, type=struct data_x3_indirect, 
        size=BLOCK_SIZE );

FSSTRUCT() data_x3_indirect {
        POINTER ( repr=block, type=data_x2_indirect_block )
        __le32 ind_block_nr ;
};

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) 
{
        FIELD(count=BLOCK_SIZE/sizeof(__le32))
        POINTER (repr=block, type=data_indirect_block)
        __le32 ind_block_nr[];
} data_x2_indirect_block;

typedef FSSTRUCT(rank=container, size=BLOCK_SIZE) 
{
        FIELD(count=BLOCK_SIZE/sizeof(__le32))
        POINTER(repr=block, type=data_block)
        __le32 ind_block_nr[];
} data_indirect_block;

//VECTOR(name=data_indirect_block, type=__le32, size=BLOCK_SIZE);
        
#endif /* EXT3_INDIRECT_H */
        
