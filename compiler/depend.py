#!bin/python
#
# depend.py
#
# emulate the behaviour of the -MM option of gnu compilers
#
# Kuei (Jack) Sun
# kuei.sun@mail.utoronto.ca
#
# University of Toronto
# Mar 2016
#


import sys, os
import fnmatch

EXTRA_DEPENDS = [ "../include/libfs.h" ]

def glob_recursive(folder, pattern):
    """
    implements recursive glob (of Python 3.5)
    """
    matches = []
    for root, dirnames, filenames in os.walk(folder):
        for filename in fnmatch.filter(filenames, pattern):
            matches.append(os.path.join(root, filename))
    return matches

# {1} is lower case, {2} is capitalized
RULES = """lib{1}.h lib{1}.cpp: {0}.d
{1}: {0}.d
$(SRCDIR)/lib{1}.cpp: lib{1}.cpp
\tcp $< $(SRCDIR)
$(INCDIR)/lib{1}.h: lib{1}.h
\tcp $< $(INCDIR)
.PHONY: install-lib{1} remove-lib{1}
install-lib{1}: $(SRCDIR)/lib{1}.cpp $(INCDIR)/lib{1}.h
remove-lib{1}:
\trm -f $(SRCDIR)/lib{1}.cpp $(INCDIR)/lib{1}.h
"""

def make_depend(filename):
    """
    build a Makefile compatible list of dependencies for each file system 
    """
    output = open(filename, "w")
    fslist = list()
    temps = " ".join(glob_recursive("template", "*.cc") + 
                     glob_recursive("template", "*.h") +
                     glob_recursive("jd", "*.py") + 
                     EXTRA_DEPENDS )
    for fsname in os.listdir("fs"):
        dfile = open(os.path.join("fs", fsname))
        deps = " ".join(dfile.read().split("\n"))
        dfile.close()
        output.write("%s.d: fs/%s %s %s\n"%(fsname, fsname, deps, temps))
        # create a per-filesystem make command
        output.write(RULES.format(fsname, fsname.lower()))
        fslist.append(fsname)
    output.write("install: %s\n"%(" ".join(["install-lib%s"%f.lower() for f in fslist])))
    output.write("remove: %s\n"%(" ".join(["remove-lib%s"%f.lower() for f in fslist])))  
    output.close()

if __name__ == "__main__":
    if len(sys.argv) == 2:
        make_depend(sys.argv[1])
    else:
        print "usage: %s FILE"%sys.argv[0]
