#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <math.h>
#include <libext3.h>
#include "blockio.h"

using namespace std;

// in # of blocks
struct Extent {
    unsigned int start_addr;
    unsigned int length;
};

typedef struct extent_list {
    struct Extent item;
    struct extent_list *next;
} extent_list;


int bitmap_bit_free(const char * bm, long bit) {
    long byte = bit / 8;
    long off = bit % 8;
    int free = (bm[byte] & (1 << off)) == 0;
    return free;
}


class MyDescTableVisitor : public FS::Visitor
{
    char * buf;
    unsigned block_size;
    int ctr = 0;

public:
    MyDescTableVisitor(char * buf, unsigned bs) : buf(buf), block_size(bs) {}
    int visit(FS::Entity & obj) override;
};

int MyDescTableVisitor::visit(FS::Entity & obj)
{
    if (strcmp(obj.get_type(), "ext3_group_desc_block") == 0) {
        return obj.accept_fields(*this);
    }

    Ext3::Ext3GroupDesc * xx = (Ext3::Ext3GroupDesc*) &obj;

    if (xx->bg_flags & EXT3_BG_BLOCK_UNINIT) {
        unsigned blocks_per_group = block_size * 8;
        unsigned used_block_count = blocks_per_group - xx->bg_free_blocks_count;

        for (unsigned int ii = 0; ii < used_block_count; ii++) {
            char * bm = buf + ctr*blocks_per_group/8 + ii/8;
            *bm |= 1 << (ii % 8);
        }
    } 
    else if (xx->bg_block_bitmap != 0) {
        Ext3::Ext3BlockBitmap * yy;
        yy = (Ext3::Ext3BlockBitmap*) xx->bg_block_bitmap.fetch();
        assert(yy);
        assert(block_size == yy->get_size());
        memcpy(buf + ctr*block_size, yy->get_buffer(), block_size);
        yy->destroy();
    }

    ctr++;
    return 0;
}

int main(int argc, const char * argv[])
{
    BlockIO io;
    Ext3 ext3(io);
    const char * filename = "/dev/sdb1";
    Ext3::Ext3SuperBlock * super;
    unsigned int blocksize = 0;
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
    
    if ((super = (Ext3::Ext3SuperBlock *)ext3.fetch_super()) != nullptr) {
        blocksize = 1024 << super->s_log_block_size;
        io.set_block_size(blocksize);
    }
    else {
        cout << argv[0] << ": io error or super block is corrupted" << endl;
        return EXIT_FAILURE;
    }

    // Print free space info from superblock
    cout << "Filesystem block size: " << blocksize << endl;
    cout << "Blocks per group: " <<  super->s_blocks_per_group << endl;
    
    unsigned int blockcount = super->s_blocks_count;
    unsigned int freecount = super->s_free_blocks_count;
    cout << freecount << " of " << blockcount << " blocks free" << endl;

    Ext3::Ext3GroupDescTable * table;
    table = (Ext3::Ext3GroupDescTable *)super->s_block_group_desc.fetch();

    char * buf = (char *) malloc(blockcount);
    memset(buf, 0, blockcount);

    MyDescTableVisitor dt_visitor(buf, blocksize);
    table->accept_fields((FS::Visitor&)dt_visitor);

    extent_list* L = nullptr;
    extent_list* last = nullptr;
    unsigned int cb = 0; // First block
    int state = 0;
    while (cb <= blockcount -1) { // Last block
        if (state == 0) {
            if (bitmap_bit_free(buf, cb)) {
                extent_list * temp = new extent_list();
                *temp = (extent_list) {
                    .item = {.start_addr = cb, .length = 1},
                    .next = nullptr
                };
                if (L == nullptr) {
                    L = temp;
                } else {
                    last->next = temp;
                }
                last = temp;
                state = 1;
            }
        } else {
            if (bitmap_bit_free(buf, cb)) {
                last->item.length++;
            } else {
                state = 0;
            }
        }

        cb++;
    }
    std::cout << std::endl;

    int free_hist [20] = {0};

    extent_list* iter = L;
    if (iter == nullptr) {
        std::cout << "No blocks free\n";
    }
    while (iter != nullptr) {
        printf("Extent %d-%d (Length: %d) \n", iter->item.start_addr,
            iter->item.start_addr + iter->item.length - 1,
            iter->item.length);

        free_hist[(int) (log(iter->item.length)/log(2))]++;

        extent_list* temp = iter;
        iter = iter->next;
        delete temp;
    }

    std::cout << "\nFree space histogram:\n";
    for (int i = 0; i < 20; i++) {
        printf("%15lld : %5d \n", ((long long) 1)<<i, free_hist[i]);
    }

	super->destroy();
    delete buf;
    return 0;
}
