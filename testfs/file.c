/*
 * file.c
 *
 * operations for regular files
 *
 */
 
#include "common.h"
#include "testfs.h"
#include "block.h"
#include "inode.h"
#include "dir.h"

struct catargs {
        size_t size;
        off_t offset;
        const char * filename;
};

struct writeargs {
        const char * filename;
        const char * content;
        off_t offset;
        int truncate;
};

static int cmd_read_internal(struct super_block * sb, struct context *c, 
                            struct catargs * cp);
static int cmd_write_internal(struct super_block *sb, struct context *c, 
                              struct writeargs * wp);            

static int
testfs_get_file_inode(struct inode ** inptr, struct super_block * sb,
        struct inode *dir, const char * name)
{
        int inode_nr = testfs_dir_name_to_inode_nr(dir, name);
        struct inode *in;
        
        if (inode_nr < 0)
                return inode_nr;
        in = testfs_get_inode(sb, inode_nr);
        if (testfs_inode_get_type(in) == I_DIR) {
                testfs_put_inode(in);
                return -EISDIR;
        }
        
        *inptr = in;
        return 0;
}

int
cmd_read(struct super_block *sb, struct context *c)
{
        struct catargs args;
        int ret, i;
        
        if ( c->nargs < 2 ) {
                return -EINVAL;
        }
        
        args.size = 0;
        args.offset = 0;
        args.filename = NULL;
        
        for ( i = 1; i < c->nargs - 1; i++ ) {
                if ( strcmp(c->cmd[i], "-s") == 0 ) {
                        ret = str_to_ulong(&args.size, c->cmd[++i]);
                } else if ( strcmp(c->cmd[i], "-o") == 0 ) {
                        ret = str_to_long(&args.offset, c->cmd[++i]);
                } else {
                        ret = -EINVAL;
                }
                
                if ( ret < 0 ) {
                        return ret;
                }
        }
        
        if ( i >= c->nargs || args.offset < 0 ) {
                return -EINVAL;
        }
        
        args.filename = c->cmd[c->nargs - 1];
        return cmd_read_internal(sb, c, &args);
}

/*
 * cmd_write:
 *
 * performs special parsing of arguments before passing context off to
 * cmd_write_internal().
 */
int
cmd_write(struct super_block *sb, struct context *c)
{
        char * line = (char *)c->cmd[1];
        char * save = NULL;
        char * token;
        struct writeargs wargs = { 0 };
        
        if ( c->nargs != 2 ) {
                return -EINVAL;
        }

        while ( (token = strtok_r(line, " \t\r\n", &save)) != NULL ) {
                line = NULL;
                if ( strcmp(token, "-o") == 0 ) {
                        token = strtok_r(NULL, " \t\r\n", &save);
                        if ( token == NULL ) 
                                return -EINVAL;
                        if ( str_to_long(&wargs.offset, token) < 0 ) 
                                return -EINVAL;
                } else if ( strcmp(token, "-t") == 0 ||
                            strcmp(token, "--truncate") == 0 ) {
                        wargs.truncate = 1;
                } else { 
                        break;
                }
        }
        
        wargs.filename = token;                             // FILE
        wargs.content  = strtok_r(NULL, "\r\n", &save);     // DATA
        
        // write no data (does nothing unless -t option is on)
        if ( wargs.content == NULL ) {
                wargs.content = "";
        }
        
        if ( wargs.filename == NULL || wargs.offset < 0 ) 
                return -EINVAL;
        
        return cmd_write_internal(sb, c, &wargs);
}

static void
array_pretty_print(u32 * array, int max)
{
        const int per_row = 5;
        int i;
        
        for ( i = 0; i < max; i++ ) {
                char name[32];
                sprintf(name, "[%d]", i); 
                printf("%6s = %-5u ", name, array[i]);
                if ( (i + 1)%per_row == 0 ) {
                        printf("\n");
                }
        }

        if ( i%per_row > 0 ) {
                printf("\n");
        }
}

int
cmd_debug(struct super_block * sb, struct context *c)
{
        char block[BLOCK_SIZE];
        char * save = NULL;
        char * filename = strtok_r((char *)c->cmd[1], " \t\r\n", &save);
        struct inode * in;
        int inode_nr;
        struct dinode * iptr;
        
        if ( filename == NULL )
                return -EINVAL;
        
        inode_nr = testfs_dir_name_to_inode_nr(c->cur_dir, filename);
        if (inode_nr < 0)
                return inode_nr;
        in = testfs_get_inode(sb, inode_nr);
        assert(in);
        
        printf("filename: %s\n", filename);
        printf("inode number: %d\n", testfs_inode_get_nr(in));
        printf("file size: %zu bytes\n", testfs_inode_get_size(in));
        printf("direct blocks:\n");
        iptr = (struct dinode *)in;
        array_pretty_print(iptr->i_block_nr, NR_DIRECT_BLOCKS);
        printf("indirect block: %u\n", iptr->i_indirect);
        if ( iptr->i_indirect > 0 ) {
                read_blocks(sb, block, iptr->i_indirect, 1);
                array_pretty_print((u32 *)block, NR_INDIRECT_BLOCKS);
        }
        
        if (testfs_inode_get_type(in) == I_DIR) {
                struct dirent * d;
                long offset = 0;
                long p_offset = 0;
                
                while ((d = testfs_next_dirent(in, &offset)) != NULL) {
                        printf("%08lx: rec_len = %d, name_len = %d, "
                               "inode_nr = %d", p_offset,
                               d->d_rec_len, d->d_name_len, d->d_inode_nr);
                        if ( d->d_name_len > 0 )
                            printf(", name = %s\n", D_NAME(d));
                        else
                            printf("\n");
                        free(d);
                        p_offset = offset;
                }
        }
        
        testfs_put_inode(in);
        return 0;
}

static int
cmd_read_internal(struct super_block * sb, struct context *c, 
                 struct catargs * cp)
{
        char *buf;
        struct inode *in;
        int ret = 0;
        long file_size;
        size_t read_size, max_size;
                
        ret = testfs_get_file_inode(&in, sb, c->cur_dir, cp->filename);
        if ( ret < 0 )
                return ret;

        file_size = testfs_inode_get_size(in);
        if ( cp->offset > file_size ) {
                ret = -EINVAL;  
        } else if (file_size > 0) {
                max_size = file_size - cp->offset;
                if ( cp->size == 0 ) 
                        read_size = max_size;
                else
                        read_size = MIN(max_size, cp->size);
                
                buf = malloc(read_size + 1);
                if (!buf) {
                        ret = -ENOMEM;
                        goto out;
                }
                testfs_read_data(in, cp->offset, buf, read_size);
                buf[read_size] = 0;
                printf("%s\n", buf);
                free(buf);
        }
out:
        testfs_put_inode(in);                
        return ret;
}

static int
cmd_write_internal(struct super_block *sb, struct context *c,
                   struct writeargs * wp)
{
        struct inode *in;
        size_t size = strlen(wp->content);
        int ret = 0;
        
        if ( size == 0 && !wp->truncate ) {
                return ret;
        }
        
        ret = testfs_get_file_inode(&in, sb, c->cur_dir, wp->filename);
        if ( ret < 0 )
                return ret;
        
        /* offset outside of file boundary, no truncate can occur */
        if ( size == 0 && testfs_inode_get_size(in) <= wp->offset ) {
                goto out;
        }
        
        if ( size > 0 ) {
                ret = testfs_write_data(in, wp->offset, wp->content, size);
        }
        
        if (ret >= 0 && wp->truncate) {
                testfs_truncate_data(in, wp->offset + size);
        }
        
        testfs_sync_inode(in);
out:
        testfs_put_inode(in);
        return ret;       
}

