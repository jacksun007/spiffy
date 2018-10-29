/*
 * crext3.cpp
 *
 * contains main() for bootstraping to libext3
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2018, University of Toronto
 *
 */

#include <libext3.h>
#include <iostream>
#include "corruptor.h"
#include "blockio.h"

using namespace std;

int main(int argc, char * argv[]) 
{
    BlockIO io;
    Ext3 ext3(io);
    Corruptor corruptor(ext3);
    Ext3::Ext3SuperBlock * super;
    int ret;

    if ((ret = corruptor.process_arguments(argc, argv)) < 0)
        return EXIT_FAILURE;
    else if (io.open(argv[ret]) < 0) {
        cout << argv[0] << ": could not open " << argv[ret] << endl;
        return -EINVAL;
    }
    
    if ((super = (Ext3::Ext3SuperBlock *)ext3.fetch_super())) {
        io.set_block_size(1024 << super->s_log_block_size);
        super->destroy();
    }
    else {
        cout << argv[0] << ": io error or super block is corrupted" << endl;
        return EXIT_FAILURE;
    }
    
    return corruptor.run();
}

