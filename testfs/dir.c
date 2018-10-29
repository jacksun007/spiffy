/*
 * dir.c
 *
 * defines operations on the dirent structure
 *
 */

#include "common.h"
#include "testfs.h"
#include "super.h"
#include "inode.h"
#include "block.h"
#include "dir.h"

#define ROOT_INODE_NR      0
#define INVALID_INODE_NR (-1)

/* reads next dirent, updates offset to next dirent in directory */
/* allocates memory, caller should free */
struct dirent *
testfs_next_dirent(struct inode *dir, long *offset)
{
        char buf[BLOCK_SIZE];
        struct dirent *dp, *d;
        long block_ix = *offset % BLOCK_SIZE;
        long size;
        int ret;
        
        assert(dir);
        assert(testfs_inode_get_type(dir) == I_DIR);
     
        /* if the offset is greater than file size, no more dirent exists */
        if (*offset >= testfs_inode_get_size(dir))
                return NULL;
        
        /* size of the read must fit a dirent or else there is a bug */
        size = BLOCK_SIZE - block_ix;
        assert( size >= (long)sizeof(struct dirent));       
         
	    ret = testfs_read_data(dir, *offset, buf, size);
        if ( ret < 0 )
                return NULL;
        
        /* read_data must have read at least the size of a dirent */            
        assert(ret >= (int)sizeof(struct dirent));
	    d = (struct dirent *)buf;
	
	    /* actual size of the dirent is header + name */
        size = sizeof(struct dirent) + d->d_name_len;
        assert(ret >= size);
        assert(block_ix + size <= BLOCK_SIZE);
        
        /* malloc a new dirent and copy the dirent */
        if ( !(dp = malloc(size)) )
                return NULL;
        memcpy(dp, d, size);
        
        /* the next dirent is rec_len bytes away */
        *offset += d->d_rec_len;
        return dp;
}

/* returns dirent associated with inode_nr in dir.
 * returns NULL on error.
 * allocates memory, caller should free. */
static struct dirent *
testfs_find_dirent(struct inode *dir, int inode_nr)
{
        struct dirent *d;
        long offset = 0;

        assert(dir);
        assert(testfs_inode_get_type(dir) == I_DIR);
        assert(inode_nr >= 0);
        for (; (d = testfs_next_dirent(dir, &offset)); free(d)) {
                if (d->d_inode_nr == inode_nr)
                        return d;
        }
        return NULL;
}

static void 
testfs_fill_dirent(struct dirent * d, const char *name, int len,
                   int inode_nr, int rec_len)
{
        memset(d, 0, rec_len);
        d->d_inode_nr = inode_nr;
        d->d_rec_len = rec_len;
        d->d_name_len = len;
        assert((int)sizeof(struct dirent) + len <= rec_len);
        strcpy(D_NAME(d), name);
}

static struct dirent * 
testfs_create_dirent(const char *name, int inode_nr, int rec_len)
{
        int len = strlen(name) + 1;
        struct dirent * d = malloc(rec_len);
        if ( !d )
                return NULL;
        testfs_fill_dirent(d, name, len, inode_nr, rec_len);
        return d;
}

/* return 0 on success.
 * return negative value on error. */
static int
testfs_write_dir_block(struct inode *dir, const char *name, int len, 
                       int inode_nr, long offset)
{
        struct dirent *d;
        int size = sizeof(struct dirent) + len;
	int ret;
        
        assert(inode_nr >= 0);
        assert(size < BLOCK_SIZE);
        assert(offset % BLOCK_SIZE == 0);
        
        d = testfs_create_dirent(name, inode_nr, BLOCK_SIZE);
        if ( !d )
                return -ENOMEM;

        ret = testfs_write_data(dir, offset, (const char *)d, BLOCK_SIZE);
        free(d);
        return ret;
}

/* return 0 on success.
 * return negative value on error. */
static int
testfs_add_dirent(struct inode *dir, const char *name, int inode_nr)
{
        struct dirent *d;       // current
        long p_offset = 0, offset = 0;
        long len = strlen(name) + 1;
        long size = len + sizeof(struct dirent);
        int found = 0;
        int ret = 0;

        assert(dir);
        assert(testfs_inode_get_type(dir) == I_DIR);
        assert(name); 
        
        /* while no error and space is not found */
        for (; ret == 0 && found == 0; free(d)) {
                p_offset = offset;
                
                // if there is no next dirent, break
                if ((d = testfs_next_dirent(dir, &offset)) == NULL)
                        break;
                        
                // if this particular dirent has the same name
                if ((d->d_inode_nr >= 0) && (strcmp(D_NAME(d), name) == 0)) {
                        ret = -EEXIST;
                        continue;
                }
                
                /* fit inside a valid dirent with extra space */
                if ( d->d_inode_nr >= 0 ) {
                        const long space = d->d_rec_len - sizeof(struct dirent) 
                                                        - d->d_name_len;
                        if ( space < size )
                                continue;
                /* fit inside a deleted dirent */
                } else if ( d->d_rec_len < size )
                        continue; // cannot fit
                                
                found = 1;
        }
        
        /* on error, abort operation */
        if (ret < 0)
                return ret;

        offset = p_offset;
        if ( found )
        {
                struct dirent * p = testfs_next_dirent(dir, &offset);
                assert(p);
                
                // a deleted dirent -- can replace it
                if ( p->d_inode_nr < 0 )
                {
                        /* there must be enough space to fit the new name */
                        assert(p->d_rec_len >= size);
                        
                        d = testfs_create_dirent(name, inode_nr, p->d_rec_len);
                        assert(d);
                        
                        ret = testfs_write_data(dir, p_offset, (const char *)d,
                                                p->d_rec_len);
                        free(d);
                }
                else /* we are placing a new dirent inside this one */
                { 
                        long p_size = sizeof(struct dirent) + p->d_name_len;
                        struct dirent * b;
                        
                        /* create a buffer with p's data and size */
                        b = testfs_create_dirent(D_NAME(p), p->d_inode_nr, 
                                                 p->d_rec_len);
                        assert(b);
                        
                        /* fix the b->d_rec_len field now to shrunk size */
                        b->d_rec_len = p_size;

                        /* d now points to the next free space */
                        d = (struct dirent *)((char *)b+p_size);
                        testfs_fill_dirent(d, name, len, inode_nr, 
                                           p->d_rec_len - p_size);
                        
                        ret = testfs_write_data(dir, p_offset, (const char *)b,
                                                p->d_rec_len);
                        free(b);
                }
                
                free(p);
                return ret;
        }
        else if ( p_offset == testfs_inode_get_size(dir) )
        {
                /* we must be at block boundary */
                return testfs_write_dir_block(dir, name, len, inode_nr, 
                                              p_offset);
        }
           
        assert(0);
        return -EIO;
}


/* returns negative value if name within dir is not empty */
static int
testfs_remove_dirent_allowed(struct super_block *sb, int inode_nr)
{
        struct inode *dir;
        struct dirent *d;
        long offset = 0;
        int ret = 0;

        dir = testfs_get_inode(sb, inode_nr);
        if (testfs_inode_get_type(dir) != I_DIR)
                goto out;
        for (; ret == 0 && (d = testfs_next_dirent(dir, &offset)); free(d)) {
                if ((d->d_inode_nr < 0) || (strcmp(D_NAME(d), ".") == 0) || 
                    (strcmp(D_NAME(d), "..") == 0))
                        continue;
                ret = -ENOTEMPTY;
        }
out:
        testfs_put_inode(dir);
        return ret;
}

/* returns inode_nr of dirent removed.
   returns negative value if name is not found */
static int
testfs_remove_dirent(struct super_block *sb, struct inode *dir, 
                     const char *name)
{
        struct dirent *d;
        long p_offset, offset = 0;
        int inode_nr = INVALID_INODE_NR;
        int ret = -ENOENT;

        assert(dir);
        assert(name);
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
                return -EINVAL;
        }
        for (; inode_nr == INVALID_INODE_NR; free(d)) {
                long size;
                
                p_offset = offset;
                if ((d = testfs_next_dirent(dir, &offset)) == NULL)
                        break;
                if ((d->d_inode_nr < 0) || (strcmp(D_NAME(d), name) != 0))
                        continue;
                /* found the dirent */
                inode_nr = d->d_inode_nr;
                if ((ret = testfs_remove_dirent_allowed(sb, inode_nr)) < 0)
                        continue; /* this will break out of the loop */
                
                /* calculate the size of this dirent (for overwriting) */
                size = sizeof(struct dirent) + d->d_name_len;
                
                /* set inode number to invalid */
                d->d_inode_nr = INVALID_INODE_NR;
                
                /* clear out the old name and set name_len to 0 for reuse */
                memset(d+1, 0, d->d_name_len);
                d->d_name_len = 0;
                
                ret = testfs_write_data(dir, p_offset, (char *)d, size);
                if (ret >= 0)
                        ret = inode_nr;
        }
        return ret;
}

struct dir_root
{
        struct dirent first;
        char dot[2];
        struct dirent second;
        char dotdot[3];
        char padding[BLOCK_SIZE - 2*sizeof(struct dirent) - 5*sizeof(char)];
} __attribute__((packed));

static int
testfs_create_empty_dir(struct super_block *sb, int p_inode_nr,
                        struct inode *cdir)
{
        int ret;
        const u16 dot_size = sizeof(struct dirent) + 2;
        struct dir_root * root = malloc(sizeof(struct dir_root));
        
        assert(sizeof(struct dir_root) == BLOCK_SIZE);
        assert(testfs_inode_get_type(cdir) == I_DIR);
        if ( root == NULL )
                return -ENOMEM;
        
        root->first.d_rec_len = dot_size;
        root->first.d_name_len = 2;
        root->first.d_inode_nr = testfs_inode_get_nr(cdir);
        strcpy(root->dot, ".");
        root->second.d_rec_len = BLOCK_SIZE - dot_size;
        root->second.d_name_len = 3;
        root->second.d_inode_nr = p_inode_nr;
        strcpy(root->dotdot, "..");
        memset(root->padding, 0, sizeof(root->padding));
        
        ret = testfs_write_data(cdir, 0, (const char *)root, BLOCK_SIZE);
        if ( ret != 0 )
                return ret;
        
        (void)sb;
        free(root);
        return 0;
}

int
testfs_create_file_or_dir(struct super_block *sb, struct inode *dir,
                          inode_type type, const char *name)
{
        int ret = 0;
        struct inode *in;
        int inode_nr;

        if (dir) {
                inode_nr = testfs_dir_name_to_inode_nr(dir, name);
                if (inode_nr >= 0)
                        return -EEXIST;
        }
        /* first create inode */
        ret = testfs_create_inode(sb, type, &in);
        if (ret < 0) {
                goto fail;
        }
        inode_nr = testfs_inode_get_nr(in);
        if (type == I_DIR) { /* create directory */
                int p_inode_nr = dir ? testfs_inode_get_nr(dir) : inode_nr;
                ret = testfs_create_empty_dir(sb, p_inode_nr, in);
                if (ret < 0)
                        goto out;
        }
        /* then add directory entry */
        if (dir) {
                if ((ret = testfs_add_dirent(dir, name, inode_nr)) < 0)
                        goto out;
                testfs_sync_inode(dir);
        }
        testfs_sync_inode(in);
        testfs_put_inode(in);
        return 0;
out:
        testfs_remove_inode(in);
fail:
        return ret;
}

static int
testfs_pwd(struct super_block *sb, struct inode *in)
{
        int p_inode_nr;
        struct inode *p_in;
        struct dirent *d;
        int ret;

        assert(in);
        assert(testfs_inode_get_nr(in) >= 0);
        p_inode_nr = testfs_dir_name_to_inode_nr(in, "..");
        assert(p_inode_nr >= 0);
        if (p_inode_nr == testfs_inode_get_nr(in)) {
                printf("/");
                return 1;
        }
        p_in = testfs_get_inode(sb, p_inode_nr);
        d = testfs_find_dirent(p_in, testfs_inode_get_nr(in));
        assert(d);
        ret = testfs_pwd(sb, p_in);
        testfs_put_inode(p_in);
        printf("%s%s", ret == 1 ? "" : "/", D_NAME(d));
        free(d);
        return 0;
}

/* returns negative value if name is not found */
int
testfs_dir_name_to_inode_nr(struct inode *dir, const char *name)
{
        struct dirent *d;
        long offset = 0;
        int ret = -ENOENT;

        assert(dir);
        assert(name);
        assert(testfs_inode_get_type(dir) == I_DIR);
        for (; ret < 0 && (d = testfs_next_dirent(dir, &offset)); free(d)) {
                if ((d->d_inode_nr < 0) || (strcmp(D_NAME(d), name) != 0))
                        continue;
                ret = d->d_inode_nr;
        }
        return ret;
}

int
testfs_make_root_dir(struct super_block *sb)
{
        return testfs_create_file_or_dir(sb, NULL, I_DIR, NULL);
}

int
cmd_cd(struct super_block *sb, struct context *c)
{
        int inode_nr;
        struct inode *dir_inode;

        if (c->nargs != 2) {
                return -EINVAL;
        }
        inode_nr = testfs_dir_name_to_inode_nr(c->cur_dir, c->cmd[1]);
        if (inode_nr < 0)
                return inode_nr;
        dir_inode = testfs_get_inode(sb, inode_nr);
        if (testfs_inode_get_type(dir_inode) != I_DIR) {
                testfs_put_inode(dir_inode);
                return -ENOTDIR;
        }
        testfs_put_inode(c->cur_dir);
        c->cur_dir = dir_inode;
        return 0;
}

int
cmd_pwd(struct super_block *sb, struct context *c)
{
        if (c->nargs != 1) {
                return -EINVAL;
        }
        testfs_pwd(sb, c->cur_dir);
        printf("\n");
        return 0;
}

static int
testfs_ls(struct super_block * sb, struct inode *in, int recursive)
{
        long offset = 0;
        struct dirent *d;

        for (; (d = testfs_next_dirent(in, &offset)); free(d)) {
                struct inode *cin;

                if (d->d_inode_nr < 0)
                        continue;
                cin = testfs_get_inode(sb, d->d_inode_nr);
                printf("%s%s\n", D_NAME(d), 
                       (testfs_inode_get_type(cin) == I_DIR) ? "/":"");
                if (recursive && testfs_inode_get_type(cin) == I_DIR &&
                    (strcmp(D_NAME(d), ".") != 0) && 
                    (strcmp(D_NAME(d), "..") != 0)) {
                        testfs_ls(sb, cin, recursive);
                }
                testfs_put_inode(cin);
        }
        return 0;
}

static int
cmd_ls_internal(struct super_block *sb, struct context *c, int recursive)
{
        int inode_nr;
        struct inode *in;
        const char *cdir = ".";

        if (c->nargs != 1 && c->nargs != 2) {
                return -EINVAL;
        }
        if (c->nargs == 2) {
                cdir = c->cmd[1];
        }
        assert(c->cur_dir);
        inode_nr = testfs_dir_name_to_inode_nr(c->cur_dir, cdir);
        if (inode_nr < 0)
                return inode_nr;
        in = testfs_get_inode(sb, inode_nr);
        if ( testfs_inode_get_type(in) == I_DIR ) {
                testfs_ls(sb, in, recursive);
        } else {
                return -ENOTDIR;
        }
        testfs_put_inode(in);
        return 0;        
}

int
cmd_ls(struct super_block *sb, struct context *c)
{
        return cmd_ls_internal(sb, c, 0);
}

int
cmd_lsr(struct super_block *sb, struct context *c)
{
        return cmd_ls_internal(sb, c, 1);
}

int
cmd_create(struct super_block *sb, struct context *c)
{
        if (c->nargs != 2) {
                return -EINVAL;
        }
        return testfs_create_file_or_dir(sb, c->cur_dir, I_FILE, c->cmd[1]);
}

int
cmd_stat(struct super_block *sb, struct context *c)
{
        int inode_nr;
        struct inode *in;
        int i;

        if (c->nargs < 2) {
                return -EINVAL;
        }   
        for (i = 1; i < c->nargs; i++ )
        {
                inode_nr = testfs_dir_name_to_inode_nr(c->cur_dir, c->cmd[i]);
                if (inode_nr < 0)
                        return inode_nr;
                in = testfs_get_inode(sb, inode_nr);
                printf("%s: i_nr = %d, i_type = %d, i_size = %zu\n", c->cmd[i], 
                       testfs_inode_get_nr(in), testfs_inode_get_type(in), 
                       testfs_inode_get_size(in));
                testfs_put_inode(in);
        }
        return 0;
}

int
cmd_rm(struct super_block *sb, struct context *c)
{
        int inode_nr;
        struct inode *in;

        if (c->nargs != 2) {
                return -EINVAL;
        }
        inode_nr = testfs_remove_dirent(sb, c->cur_dir, c->cmd[1]);
        if (inode_nr < 0) {
                return inode_nr;
        }
        in = testfs_get_inode(sb, inode_nr);
        testfs_remove_inode(in);
        testfs_sync_inode(c->cur_dir);
        return 0;
}

int
cmd_mkdir(struct super_block *sb, struct context *c)
{
        if (c->nargs != 2) {
                return -EINVAL;
        }
        return testfs_create_file_or_dir(sb, c->cur_dir, I_DIR, c->cmd[1]);
}
