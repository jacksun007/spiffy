/*
 * f2fsreader.cpp
 *
 * Implementation of FS::Reader for the F2FS file system.
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#include "f2fsreader.h"
#include <libf2fs.h>
#include <cerrno>
#include <iostream>
#include <cstring>
#include <assert.h>

using namespace std;

class JournalVisitor : public FS::Visitor
{
    long nid;
    const char *buf;
public:
    JournalVisitor(long id) : nid(id), buf(nullptr) {}
    const char * get_buf() { return buf; }
    virtual int visit(FS::Entity & obj) override;
};

int JournalVisitor::visit(FS::Entity & ent) 
{
    F2FS::NatJournalEntry & entry = (F2FS::NatJournalEntry &)ent;
    if (entry.nid == nid) {
        F2FS::F2fsNatEntry & ne = entry.ne;
        buf = ne.get_value<const char *>();
    }
    return 0;
}

F2FSReader::F2FSReader() : FileReader(), sb(nullptr) {}

F2FSReader::~F2FSReader()
{
    if ( sb != nullptr )
    {
        delete sb;
    }

    if ( bitmap != nullptr )
    {
        delete bitmap;
    }
}

void F2FSReader::set_super_block(F2FS::F2fsSuperBlock * super)
{
    if ( sb != nullptr )
    {
        delete sb;
    }
    
    sb = super;
    set_block_size(1 << sb->log_blocksize);
}

void F2FSReader::set_nat_bitmap(const char *bm, size_t bm_size)
{
    if ( bitmap != nullptr ) {
        delete bitmap;
    }
    bitmap_size = bm_size;
    bitmap = new unsigned char[bm_size+1];
    memcpy(bitmap, bm, bm_size+1);
    assert(bitmap != nullptr);
}

void F2FSReader::set_journal(F2FS::F2fsNatJournal *jrl)
{
    nat_journal = jrl; 
}

long F2FSReader::get_nat_pack(long nid) {
    int mask;
    char *addr = (char *)bitmap;

    addr += (nid >> 3);
    mask = 1 << (7 - (nid & 0x07));
    return (mask & *addr) != 0;
}
    
bool F2FSReader::check_journal(long nid, const char **buf)
{
    assert(nat_journal != nullptr); 
    JournalVisitor jrl_visitor(nid);

    /* Iterate through journal entries */
    F2FS::NatJournal & n_jrl = nat_journal->nat_j;
    F2FS::NatJournal::Entries & entries = n_jrl.entries;
    entries.accept_fields(jrl_visitor);

    if (jrl_visitor.get_buf() == nullptr) {
        return false;
    }
    const char *buffer = jrl_visitor.get_buf();

    *buf = buffer;
    return true;
}

/* 
 * The logical nat block will be composed of the most updated physical nat entries
 * which is derived from the journal or one of the NAT packs.
 */
ssize_t F2FSReader::logical_read(long id, size_t size, off_t offset, char **bufptr)
{
    ssize_t ret;
    off_t buffer_offset;
    char *phy_block;
    const char *jrl_buf;
    char *logical_block;
    long phy_blknr;
    off_t pos = 0;

    /* Sanity Checks */
    //assert( id == 0); where id !=0, coming from nidspace
    assert( sb != nullptr);
    assert( bitmap != nullptr);
    assert( block_size > 0 );
    assert(offset % block_size == 0);

    /* Size Information */
    size_t nat_entry_sz = sizeof(struct f2fs_nat_entry);
    int nats_per_block = block_size / nat_entry_sz;

    /* Addressing Information */
    unsigned nat_block_offset = offset / block_size;
    unsigned nat_blocknr = id + nat_block_offset;
    long nid_start = nat_blocknr * nats_per_block;

    /* Addrspace boundaries for nids. */
    if ( nat_blocknr >= ( sb->segment_count_nat << (sb->log_blocks_per_seg - 1)) ) {
        return -EINVAL;
    }

    /* Buffer to piece together nat entries */
    logical_block = new char[block_size];
    phy_blknr = sb->nat_blkaddr + nat_blocknr;

    /* Read from the most valid NAT Block */
    long pack = get_nat_pack(nat_block_offset);
    if ( pack != 0 ) {
        phy_blknr += (1 << sb->log_blocks_per_seg);
    }

    if ( (ret = block_read(phy_blknr, block_size, 0, &phy_block)) < 0 ) {
        delete [] logical_block;
        *bufptr = nullptr;
        return ret;
    }

    /* Piece together valid entries for current block */
    for (int i=0; i < nats_per_block; ++i) {
        /* Perform a journal lookup for nid, else look in valid NAT pack */
        if ( check_journal(nid_start+i, &jrl_buf)) {
            //Check Valid
            //struct f2fs_nat_entry ne;
            //memcpy(&ne, jrl_buf, nat_entry_sz);
            //cout << "After cast: " << ne.block_addr << endl;
            memcpy(logical_block + pos, jrl_buf, nat_entry_sz);
        }
        else {
            buffer_offset = i*nat_entry_sz;
            memcpy(logical_block + pos, phy_block + buffer_offset, nat_entry_sz);
        }
        pos += nat_entry_sz;
    }

    assert(pos == (off_t)(nats_per_block*nat_entry_sz));
    delete[] phy_block;
    *bufptr = logical_block;
    return pos;
}

ssize_t F2FSReader::nid_read(long id, size_t size, off_t offset, char **bufptr)
{
    assert(offset == 0);

    char *logical_block = nullptr;
    size_t nat_entry_sz = sizeof(struct f2fs_nat_entry);
    int nats_per_block = block_size / nat_entry_sz;
    off_t byte_offset = (id % nats_per_block)*nat_entry_sz;
    unsigned nat_blocknr = id / nats_per_block;
    
    ssize_t retval = logical_read(nat_blocknr, block_size, 0, &logical_block);

    if (retval >= 0 && logical_block != nullptr) {
        struct f2fs_nat_entry ne;
        memcpy(&ne, &logical_block[byte_offset], nat_entry_sz);
        delete[] logical_block;
        return block_read(ne.block_addr, size, offset, bufptr);
    }
    return -EINVAL;
}
    
ssize_t F2FSReader::read(unsigned int addrsp, long id, size_t size, 
                         off_t offset, char ** bufptr) 
{
    /* Perform node translation for nid->blk_nr of direct/indirect nodes */
    if ( addrsp == F2FS::AddrSpace::NID )
    {
        return nid_read(id, size, offset, bufptr);
    }
    if ( addrsp == F2FS::AddrSpace::LOGICAL )
    {
        return logical_read(id, size, offset, bufptr);
    }
    return FileReader::read(addrsp, id, size, offset, bufptr);
}

