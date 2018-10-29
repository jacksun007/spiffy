/*
 * testfs.c
 *
 * program for accessing a testfs filesystem
 *
 */

#define _GNU_SOURCE
#include <stdbool.h>
#include <getopt.h>
#include "common.h"
#include "testfs.h"
#include "super.h"
#include "inode.h"
#include "dir.h"

#ifdef __SNOOP__
#include "snoop.h"
#endif

static bool can_quit = false;

static int cmd_help(struct super_block *, struct context *c);
static int cmd_quit(struct super_block *, struct context *c);
static int cmd_fsstat(struct super_block *sb, struct context *c);

static const struct cmd {
	const char *name;
	int (*func)(struct super_block *sb, struct context *c);
        int max_args;
        const char * help;
} cmdtable[] = {
        /* menus */
        { "?",       cmd_help,    1, "prints this help message. usage: ?" },
        { "cd",      cmd_cd,      2, "changes to directory DIR. usage: cd DIR"},
        { "pwd",     cmd_pwd,     1, "prints current directory. usage: pwd" },
        { "ls",      cmd_ls,      2, "lists files and directories in directory"
                                     " DIR. usage: ls [DIR=.]" },
        { "lsr",     cmd_lsr,     2, "recursively lists files and directories "
                                     "starting from directory DIR. usage: lsr "
                                     "[DIR=.]" },
        { "create",  cmd_create,  2, "creates a new file named FILE in the "
                                     "current directory. usage: create FILE" },
        { "stat",    cmd_stat,    2, "prints information on file named FILE. "
                                     "usage: stat FILE" },
        { "rm",      cmd_rm,      2, "removes a file or directory with name "
                                     "NAME. usage: rm NAME" },
        { "mkdir",   cmd_mkdir,   2, "creates a new directory named DIR in the"
                                     " current directory. usage: mkdir DIR" },
        { "read",    cmd_read,    6, "for file FILE, prints SIZE bytes of "
                                     "ascii data at offset OFFSET. usage: "
                                     "read [-o OFFSET=0][-s SIZE] FILE" },
        { "write",   cmd_write,   1, "write ascii data to file named FILE at "
                                     "offset OFFSET. usage: write "
                                     "[-o OFFSET=0][-t, --truncate=false] "
                                     "FILE [DATA]" },
        { "fsstat",  cmd_fsstat,  1, "print inode and block usage. "
                                     "usage: fsstat" },                             
        { "debug",   cmd_debug,   1, ">> implement your own debug function "
                                     "here <<" },
#ifdef __SNOOP__
        { "snoop",   cmd_snoop,   1, "print all known metadata block types. "
                                     "usage: snoop" },
#endif
        { "quit",    cmd_quit,    1, "quits this program. usage: quit" },
        { NULL, NULL, 0, NULL },
        
        /* when adding new commands, you must make sure MAX_ARGS >= max_args 
           for your new command                                              */
};

static int
cmd_help(struct super_block *sb, struct context *c)
{
        int i = 0;
        
        (void)sb;
        (void)c;
        
        printf("%s", "Commands:\n");
        for (; cmdtable[i].name; i++) {
                printf("%8s - %s\n", cmdtable[i].name, cmdtable[i].help);
        }
        printf("\n%s", "* arguments within square brackets are optional. "
               "The default value is stated after the equal sign. "
               "e.g. invoking 'ls' will assume the current directory "
               "(i.e., '.')\n");
        return 0;
}

static int
cmd_fsstat(struct super_block *sb, struct context *c)
{
        if ( c->nargs != 1 ) {
                return -EINVAL;
        }
        
        printf("nr of allocated inodes = %d\n", sb->sb.used_inode_count);
        printf("nr of allocated blocks = %d\n", sb->sb.used_block_count);
        return 0;
}

static int
cmd_quit(struct super_block *sb, struct context *c)
{
        (void)sb;
        (void)c;
        
        printf("Bye!\n");
        can_quit = true;
        
        return 0;
}

static void
parse_command(const struct cmd * cmd, struct context * c, const char * name, 
        char * args)
{
        char * save = NULL;
        int j = 0;
        
        assert(cmd->func); 
        
        c->cmd[j++] = name;
        if (args == NULL) {
            c->cmd[j] = NULL;
            goto done;
        }
        
        while (j < cmd->max_args &&
               (c->cmd[j] = strtok_r(args, " \t\r\n", &save)) != NULL) {
                j++; 
                args = NULL;  
        }
        
        if ((c->cmd[j] = strtok_r(args, "\r\n", &save)) != NULL) {
                j++;
        }
        
done:                  
        for ( c->nargs = j++; j <= MAX_ARGS; j++ ) {
                c->cmd[j] = NULL;
        }        
}
	
static void
handle_command(struct super_block *sb, struct context *c, const char * name,
        char * args)
{
        int i;
        if (name == NULL)
                return;

        for (i=0; cmdtable[i].name; i++) {
                if (strcmp(name, cmdtable[i].name) == 0) {
                        parse_command(&cmdtable[i], c, name, args);
                        errno = cmdtable[i].func(sb, c);
                        if (errno < 0) {
                                errno = -errno;
                                WARN(c->cmd[0]);
                        }
                        return;
                }
        }
        printf("%s: command not found. type ?\n", name);
}

static void 
usage(const char * progname)
{
    fprintf(stderr, "Usage: %s [-c][-n, --no-prompt][-h, --help] "
        "rawfile\n", progname);
    exit(1);
}

struct args
{
    const char * disk;  // name of disk
    int corrupt;        // to corrupt or not
    int prompt;         // whether to print the prompt
};

static struct args *
parse_arguments(int argc, char * const argv[])
{
        static struct args args = { NULL, 0, 1 };
        static struct option long_options[] =
        {
                {"corrupt", no_argument,       0, 'c'},
                {"help",    no_argument,       0, 'h'},
                {"no-prompt", no_argument,     0, 'n'},
                {0, 0, 0, 0},
        };
        int running = 1;
    
        while (running)
        {
                int option_index = 0;
                int c = getopt_long(argc, argv, "chn", long_options, 
                                    &option_index);
                switch (c)
                {
                case -1:
                        running = 0;
                        break;
                case 0:
                        break;
                case 'c':
                        args.corrupt = 1;
                        break;
                case 'n':
                        args.prompt = 0;
                        break;
                case 'h':
                case '?':
                        usage(argv[0]);
                        break;
                default:
                    abort();
                }
        }
    
        if ( argc - optind != 1 ) {
                int i;
                for ( i = optind + 1; i < argc; i++ ) {
                        fprintf(stderr, "Unknown argument: %s\n", argv[i] );
                }
                usage(argv[0]);
        }
        
        args.disk = argv[optind];
        return &args;
}

int
main(int argc, char * const argv[])
{
        struct super_block *sb;
        char *line = NULL;
        size_t line_size = 0;
        ssize_t nr;
        int ret;
        struct context c;
        struct args * args = parse_arguments(argc, argv);
        
#ifdef __SNOOP__
        snoop_testfs_init();
#endif        
        
        ret = testfs_init_super_block(args->disk, args->corrupt, &sb);
        if (ret) {
                EXIT("testfs_init_super_block");
        }
        c.cur_dir = testfs_get_inode(sb, 0); /* root dir */
        do {
                char * cname; 
                char * pargs;
                char * save = NULL;

                if ( args->prompt )
                {
                        printf("%s", "% ");
                        fflush(stdout);
                }
                
                if (( nr = getline(&line, &line_size, stdin)) == EOF ) {
                        free(line);
                        break;
                }

                if ( line_size > 0 && line[0] == '#' )
                {
                        // this is a comment, which can be added to input 
                        // files. we do not want to parse this as command. 
                }
                else
                {
                        cname = strtok_r(line, " \t\r\n", &save);
                        pargs = strtok_r(NULL, "\r\n", &save);
                        handle_command(sb, &c, cname, pargs);
                        fflush(stdout);
                        fflush(stderr);
                }

                free(line);
                line = NULL;
                line_size = 0;
        } while ( can_quit == false );
               
        testfs_put_inode(c.cur_dir);
        testfs_close_super_block(sb);
        
#ifdef __SNOOP__
        snoop_testfs_shutdown();
#endif         
        
        return 0;
}

