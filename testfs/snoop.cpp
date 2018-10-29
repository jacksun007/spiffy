#include <libtestfs.h>
#include <interpret.h>
#include <assert.h>
#include "snoop.h"      // the local .h header file
#include "testfs.h"

struct snoop * snp = nullptr;

struct TestFSInfo : public SnoopInfo
{
    virtual int is_block_dynamic(unsigned type) const override
    {
        switch (type)
        {
        case FS::UNKNOWN_TYPE_ID:
        case TestFS::DATA_BLOCK:
        case TestFS::DIR_BLOCK:
        case TestFS::DIR_INDIRECT_BLOCK:
        case TestFS::DATA_INDIRECT_BLOCK:
            return 1;
        default:
            break;
        }
        
        return 0;
    }
    
    virtual int is_block_likely_metadata(off_t start, const char * buf, 
        size_t len) const override
    {
        int ret = 0;
        unsigned block_id = start / BLOCK_SIZE;
        FS::Location loc(TestFS::AS_BLOCK, BLOCK_SIZE, 0, block_id);
        FS::Container * ctn = filesystem->parse_by_type(TestFS::DIR_BLOCK, loc,
                              super->get_path(), buf, len);

        if (ctn != nullptr)
        {
            TestFS::DirBlock * dir_block = static_cast<TestFS::DirBlock *>(ctn);
            
            /* there needs to be at least one directory entry */
            if (dir_block->get_count() > 0)
                ret = 1;
                
            ctn->destroy();
        }
        
        return ret;
    }
    
    virtual int get_block_size(FS::Entity * sup) const override
    {
        /* testfs has a fixed block size */
        (void)sup;
        return BLOCK_SIZE;
    }
    
    virtual int visit(FS::Entity & obj) override
    {
        return 0;
    }

    TestFSInfo(struct snoop * snp, TestFS * fs) : 
        SnoopInfo(TestFS::AS_BLOCK, TestFS::DATA_BLOCK, fs) {}
};

void snoop_testfs_init(void)
{
    const int num_ht_buckets = 4096;
    snp = new snoop(num_ht_buckets);
    assert(snp != nullptr);
    snp->set_snoop_info(new TestFSInfo(snp, new TestFS()));
}

void snoop_testfs_shutdown(void)
{
    snoop_shutdown(snp);     
}

int cmd_snoop(struct super_block *sb, struct context *c)
{
        long ret = 0;
        int flags = SNOOP_PRINT_BEGIN;

        if ( c->nargs != 1 ) {
                return -EINVAL;
        }
        
        do {
            char buf[4096];
            ret = snoop_print(snp, buf, sizeof(buf), flags);
            fputs(buf, stdout);
            flags = 0;
        } while ( ret > 0 );
        return 0;
}

