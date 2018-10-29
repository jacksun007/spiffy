#!/usr/bin/python
#
# buildfs.py
#
# this program is intended to randomly generate a set of commands to 
# populate a file system image.
#
# usage:
# python buildfs.py NUM_COMMANDS > input.txt
# ./testfs disk.img < input.txt
#
# Author: Kuei (Jack) Sun 
# Email: kuei.sun@utoronto.ca
#
# University of Toronto, 2015
#

import re
import random
import string

# file system parameters
BLOCK_SIZE  = 256
NR_DIRECT   = 4
NR_INDIRECT = BLOCK_SIZE / 4

# generator parameters
MAX_NAME_LEN  = BLOCK_SIZE / 3
MEAN_NAME_LEN = 16 
MAX_FILE_LEN  = BLOCK_SIZE * (NR_DIRECT + NR_INDIRECT)
OUTPUT_FILE   = "script.txt"

# directory entry structure
DNAME = 0
DOBJ  = 1

# ==================
#  helper functions
# ==================

def get_word_list():
    """
    get word list from system, and remove any non alpha-numeric characters
    
    returns a list
    """   
    f = open("/usr/share/dict/words")
    prog = re.compile(r"[^a-zA-Z0-9_]")
    text = f.read()
    f.close()
    return [ prog.sub("", a) for a in text.splitlines() ]

WORDS = get_word_list()

def get_random_slug(min_size):
    """
    get a slug string of at least min_size bytes long
    """
    slug = random.choice(WORDS)
    while len(slug) < min_size:
        slug += "-" + random.choice(WORDS)
    return slug

def get_random_name():
    """
    get a random name for generator
    
    """
    size = int(random.expovariate(1./MEAN_NAME_LEN)) % MAX_NAME_LEN
    return get_random_slug(size)

def random_text(num_bytes):
    """
    returns a string of random text of length num_bytes
    
    """
    data = [ random.choice(random_text.charset) for _ in xrange(num_bytes) ]
    return "".join(data)
random_text.charset = string.ascii_letters + string.digits


# ========================
#  file system operations
# ========================

actions_in_curdir = 0

def chdir(f, curdir):
    """
    change directory
    
    * will perform random other action if we have not taken any action within 
    the current directory.
    
    * it is possible to chdir to parent (i.e., ..)   
    """
    global actions_in_curdir
    assert(len(curdir['dir']) > 0)
    dirname, dirent = random.choice(curdir['dir'])
    if actions_in_curdir > 0:
        f.write("cd %s\n"%dirname)
        actions_in_curdir = -1
        return dirent
    return randopt(f, curdir)


def mkdir(f, curdir):
    """
    make a new directory in the current directory
    
    """
    name = get_random_name()
    newdir = { 'file':[], 'dir':[("..", curdir)] }
    curdir['dir'].append((name, newdir))
    f.write("mkdir %s\n"%name)
    return curdir

    
def touch(f, curdir):
    """
    create a new regular file
    
    """
    while True:
        name = get_random_name()
        if name not in curdir['file']:
            break
    curdir['file'].append(name)
    f.write("create %s\n"%name)
    return curdir    


def write(f, curdir):
    """
    write to a regular file
    
    """
    if len(curdir['file']) > 0:
        which = random.choice(curdir['file'])
        # average offset will be 1 (we don't want too much crazy offset)
        offset = int(random.expovariate(1.)) % (MAX_FILE_LEN - 1)
        length = random.randint(1, MAX_FILE_LEN - offset)
        data = random_text(length)
        f.write("write -t -o %d %s %s\n"%(offset, which, data))
    else:
        return touch(f, curdir)
    return curdir


def rmdir(f, curdir):
    """
    try to remove a random directory
    
    if that directory is not empty, change into it and remove something
    """
    if len(curdir['dir']) == 1:
        if len(curdir['file']) > 0:
            return remove(f, curdir)
        else:
            return randopt(f, curdir)
    dirinfo = random.choice(curdir['dir'][1:])
    dirname, dirent = dirinfo
    assert(dirname != "..")
    if len(dirent['dir']) > 1:
        # cd into that dir and remove a dir
        f.write("cd %s\n"%dirname)
        return rmdir(f, dirent)
    elif len(dirent['file']) > 0:
        f.write("cd %s\n"%dirname)
        return remove(f, dirent)
    curdir['dir'].remove(dirinfo)
    return curdir 
     

def remove(f, curdir):
    """
    remove a regular file from current directory
    
    otherwise, remove a directory
    """
    if len(curdir['file']) > 0:
        which = random.choice(curdir['file'])
        curdir['file'].remove(which)
        f.write("rm %s\n"%which)
    elif len(curdir['dir']) > 1:
        return rmdir(f, curdir)
    else:
        return randopt(f, curdir)
    return curdir
    
CMDS = [ chdir, touch, remove, mkdir, rmdir ] + [ write ] * 8 

def randopt(f, curdir):
    """
    perform a random other operation
    
    """
    return random.choice(CMDS)(f, curdir)

def main(num_cmds, output_file):
    """
    output_file: name of output file
    num_cmds:    number of commands to be generated
    
    """
    global actions_in_curdir
    root = { 'file':[], 'dir':[] }
    root['dir'].append(("..", root))
    pwd = root
    print "Generating %d commands to '%s'"%(num_cmds, output_file)
    f = open(output_file, "wb")
    for n in xrange(num_cmds):
        pwd = randopt(f, pwd)
        actions_in_curdir = actions_in_curdir + 1
    f.close()
    
if __name__ == "__main__":
    import sys
    num_cmds = 5000
    if len(sys.argv) == 2:
        num_cmds = int(sys.argv[1])
    main(num_cmds, OUTPUT_FILE)
    
