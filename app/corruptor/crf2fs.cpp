/*
 * crf2fs.cpp
 *
 * contains main() for bootstraping to libf2fs
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2018, University of Toronto
 *
 */

#include <libf2fs.h>
#include <iostream>
#include "corruptor.h"
#include "blockio.h"

using namespace std;

int main(int argc, char * argv[]) 
{
    BlockIO io;
    F2FS f2fs(io);
    Corruptor corruptor(f2fs);
    int ret;

    if ((ret = corruptor.process_arguments(argc, argv)) < 0)
        return EXIT_FAILURE;
    else if (io.open(argv[ret]) < 0) {
        cout << argv[0] << ": could not open " << argv[ret] << endl;
        return -EINVAL;
    }

    /* f2fs always uses this block size */
    io.set_block_size(F2FS_BLKSIZE);
    return corruptor.run();
}

