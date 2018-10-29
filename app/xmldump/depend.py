#!/bin/python
#
# depend.py
#
# generates dependencies for each file system dump tool
#
# Kuei (Jack) Sun
# kuei.sun@mail.utoronto.ca
#
# University of Toronto
# June 2016

# (JSUN):
# TODO: the same result can probably be achieved using static pattern rules

def get_file_systems():
    """
    get a list of file system names that we support
    """
    import re, os
    fsnames = list()
    prog = re.compile("xd(\w+).cpp")
    for filename in os.listdir("."):
        match = prog.match(filename)
        if match is not None:
            fsnames.append(match.group(1))
    return fsnames

MAKE_RULE = """$(BUILDDIR)/xd{0}: $(BUILDDIR)/xd{0}.o $({1}_EXTRA) $(OBJECTS) \
$(LIBPATH)/lib{0}.a $(LIBPATH)/libfs.a  
xd{0}: $(BUILDDIR)/xd{0}
\tcp $< $@
"""

def make_depend(filename):
    output = open(filename, "w")
    for fsname in get_file_systems():
        output.write(MAKE_RULE.format(fsname, fsname.upper()))
    output.close()

if __name__ == "__main__":
    import sys
    if len(sys.argv) == 2:
        make_depend(sys.argv[1])
    else:
        print "usage: %s FILE"%sys.argv[0]


