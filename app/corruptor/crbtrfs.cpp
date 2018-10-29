/*
 * crbtrfs.cpp
 *
 * contains main() for bootstraping to libbtrfs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2018, University of Toronto
 *
 */

#include <libbtrfs.h>
#include <iostream>
#include "corruptor.h"
#include "blockio.h"

using namespace std;

int main(int argc, char * argv[]) 
{
    BlockIO io;
    Btrfs btrfs(io);
    Corruptor corruptor(btrfs);
    Btrfs::BtrfsSuperBlock * super;
    int ret;

    if ((ret = corruptor.process_arguments(argc, argv)) < 0)
        return EXIT_FAILURE;
    else if (io.open(argv[ret]) < 0) {
        cout << argv[0] << ": could not open " << argv[ret] << endl;
        return -EINVAL;
    }
    
    if ((super = (Btrfs::BtrfsSuperBlock *)btrfs.fetch_super())) {
        io.set_block_size(super->leafsize);
        super->destroy();
    }
    else {
        cout << argv[0] << ": io error or super block is corrupted" << endl;
        return EXIT_FAILURE;
    }
    
    return corruptor.run();
}

