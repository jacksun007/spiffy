#!/bin.python
#
# mkdata.py
#
# generates a set of empty files to test hash entry of ext3
#
# Author: Kuei (Jack) Sun
# E-mail: kuei.sun@mail.utoronto.ca 
#
# University of Toronto, 2015
#

import re, sys, random

def get_word_list():
    """
    get a list of words from linux words file
    
    """
    f = open("/usr/share/dict/words")
    prog = re.compile(r"[^a-zA-Z0-9_]")
    text = f.read()
    f.close()
    return [ prog.sub("", a) for a in text.splitlines() ]

WORDS = get_word_list()

if __name__ == "__main__":
    num_files = 10000
    if len(sys.argv) == 2:
        num_files = int(sys.argv[1])
    for _ in xrange(num_files):
        filename = "%s%05d"%(random.choice(WORDS), random.randint(0, 99999))
        with open(filename, 'a'):
            pass

