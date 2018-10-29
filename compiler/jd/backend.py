# !/bin/python
"""
backend.py

JD's backend - for generating the file system's library

Copyright 2014, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

from irep import Container, Object
from util import is_integral_type
from template import Generator
import re

class Backend(object):
    """
    backend object responsible for generating the library source and header
    files
    """
    
    def _create_generator(self, filename, context, rootdir):
        """
        filename: name of output file
        context:  context used for template generation
        rootdir:  the root directory of the template files
        """
        try:
            gen = Generator(context, filename, rootdir)
        except IOError:
            print "Could not open %s for writing"%(filename)
            self.error = True
        return gen
    
    def __init__(self, ctxt):
        self.fs = ctxt['fs']
        self.fs.setup() 
        self.error = False
        
        # add this function to the context
        ctxt['cross_reference'] = lambda t : [ xr.xref for xr in \
            self.fs.xrefs if re.search(r"(^|\W)%s\."%xr.xref, t) is not None ]
            
        self.context = ctxt 
        self.header = self._create_generator(ctxt["libheader"], ctxt, "include")
        self.source = self._create_generator(ctxt["libsrc"], ctxt, "src")
        
    def emit(self):
        """
        Generate library code
        """
        if self.error == False:
            self.source.render("source.cc")
            self.source.export()
            self.header.render("header.h")
            self.header.export()
        

