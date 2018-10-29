/*
 * f2fsreader.h
 *
 * Reader class for f2fs file system for node translation
 *
 * Author: Kuei (Jack) Sun
 * E-mail: kuei.sun@mail.utoronto.ca
 *
 * 2015, University of Toronto
 */

#ifndef F2FSREADER_H
#define F2FSREADER_H

#include <libf2fs.h>
#include "filereader.h"

class F2FSReader : public FileReader
{
private:
    F2FS::F2fsSuperBlock *sb;
    F2FS::F2fsNatJournal *nat_journal;
    unsigned char *bitmap;
    ssize_t bitmap_size;
    
    ssize_t logical_read(long id, size_t size, off_t offset, char **bufptr);
    ssize_t nid_read(long id, size_t size, off_t offset, char **bufptr);
    
public:
    F2FSReader();
    virtual ~F2FSReader();

    virtual ssize_t read(unsigned int addrsp, long id, size_t size, 
        off_t offset, char ** bufptr) override;
   
    long get_nat_pack(long nid);
    bool check_journal(long nid, const char **buf);

    /* Setter Methods */
    void set_super_block(F2FS::F2fsSuperBlock * super);
    void set_nat_bitmap(const char *bm, size_t bm_size);
    void set_journal(F2FS::F2fsNatJournal *jrl);
};

#endif /* F2FSREADER_H */
