"""
jdtempl.py

library using jinja2's template language to generate C source code.

Copyright 2015, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

from jinja2 import Environment, FileSystemLoader
from subprocess import Popen, PIPE
import shlex, os, re

CURRFILE_DIR = os.path.dirname(os.path.realpath(__file__))
TEMPLATE_DIR = "%s/template/"%os.path.dirname(CURRFILE_DIR)


class Generator(object):
    """
    wrapper class for code generation using multiple jinja2 templates
    """
    def __init__(self, ctxt, filename, rootdir=""):
        self.context = ctxt
        self.filename = filename
        self.output = open(filename, "w")
        # now create the environment for template generation
        path = [TEMPLATE_DIR, os.path.join(TEMPLATE_DIR, rootdir)]
        self.env = Environment(block_start_string='@[', block_end_string=']', 
                   variable_start_string='@(', variable_end_string=')',
                   comment_start_string='/*', comment_end_string='*/',
                   trim_blocks=True, lstrip_blocks=True,
                   loader=FileSystemLoader(path),
                   extensions=['jinja2.ext.with_'])
    
    @staticmethod
    def beautify(text):
        """
        removes any excessively long line breaks
        """
        prog = re.compile(r"[ \t\r\f\v]*\n\s*\n", re.MULTILINE)
        return prog.sub("\n\n", text)
                         
    def render(self, fname, ctxt=dict()):
        """
        render a jinja2 template and append output to file
        """
        temp = self.env.get_template(fname)
        ctxt.update(self.context)
        self.output.write(Generator.beautify(temp.render(ctxt)))
        self.output.write("\n")
       
    def write(self, data):
        """
        append arbitrary output to file
        """
        self.output.write(data)
        
    def export(self):
        """
        finalize the writing to this file
        """
        self.output.close()
        command_line = "astyle --style=kr --suffix=none %s"%self.filename 
        command = shlex.split(command_line)
        try:
            process = Popen(command)
            process.wait()
        except OSError:
            print("Error while running astyle (did you install it?)")  
        
