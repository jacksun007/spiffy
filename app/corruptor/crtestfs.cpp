/*
 * crtestfs.cpp
 *
 * contains main() for bootstraping to libext3
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2018, University of Toronto
 *
 */

#include <libtestfs.h>
#include <iostream>
#include "corruptor.h"
#include "blockio.h"

using namespace std;

int main(int argc, char * argv[]) 
{
    BlockIO io;
    TestFS testfs(io);
    Corruptor corruptor(testfs);
    int ret;

    if ((ret = corruptor.process_arguments(argc, argv)) < 0)
        return EXIT_FAILURE;
    else if (io.open(argv[ret]) < 0) {
        cout << argv[0] << ": could not open " << argv[ret] << endl;
        return -EINVAL;
    }
    
    io.set_block_size(BLOCK_SIZE);
    return corruptor.run();
}

