# !/bin/python
"""
jdc.py

The Jack Daniels annotation language compiler

Copyright 2013, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

import sys, optparse, os
from jd.yacc import parser as ply_parser
from jd.lex import lexer as ply_lexer, remove_comment
from jd.semantic import create_file_system
from jd.backend  import Backend

def compile_file(fname):
    """
    Compiles a single file
    """
    with open(fname) as f:
        print("%s: parsing %s ..."%(os.path.basename(sys.argv[0]), 
            os.path.basename(fname))),
        cleaned_text = remove_comment(f.read())
        p = ply_parser.parse(cleaned_text, debug=0)
        ply_lexer.lineno = 1    # reset line number for each file
        print("done!")
        if p is not None:
            return p
        return list()
    sys.exit(1)

def compile_files(files):
    """
    Compiles an array of files (or directories). In the case of directory,
    it will compile all of the .h files in that directory (not recursively)
    """
    structs = list()
    headers = list()
    for fname in files:
        if os.path.isdir(fname):
            found = False
            for dirent in os.listdir(fname):
                if dirent.endswith(".h"):
                    if not found:
                        found = True
                        print("%s: entered directory %s"%(
                            os.path.basename(sys.argv[0]), fname))
                    path = os.path.join(fname, dirent)
                    structs += compile_file(path)
                    headers.append(path)
        elif os.path.exists(fname):
            structs += compile_file(fname)
            headers.append(fname)
        else:
            print("%s: error: %s does not exist."%(
                os.path.basename(sys.argv[0]), fname))
            sys.exit(1)
    return (structs, headers)



def main():
    parser = optparse.OptionParser(
        description='Compiles JD-annotated C header files',
        usage="Usage: %prog [-hd][-n NAME] DIR|FILE ... ",
        version="0.2",
        epilog="Report bugs to kuei.sun@utoronto.ca")
    parser.add_option('-n', '--name', metavar='NAME', dest='name', 
        default="TestFS", help='specify the name of the file system')
    parser.add_option('-d', '--debug', dest='debug', action="store_true",
        default=False, help='prints additional debug information')    
    (options, args) = parser.parse_args()
    if len(args) == 0:
        parser.error("must specify at least one file or directory")
    (structs, headers) = compile_files(args)
    if options.debug:
        for s in structs:
            s.debug()
    fs = create_file_system(options.name, structs)
    ctxt = dict(headers=headers, 
                fs=fs, 
                libheader="lib%s.h"%(options.name.lower()),
                libsrc="lib%s.cpp"%(options.name.lower()))
    backend = Backend(ctxt)
    backend.emit()

        
if __name__ == "__main__":
    main()
