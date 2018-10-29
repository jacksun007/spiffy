# !/bin/python
"""
lex.py

Lexer for the Jack Daniels annotation language for file systems. 

Please read for detail on PLY's lexer.

http://www.dabeaz.com/ply/ply.html#ply_nn3

Known restrictions that can cause lexer malfunction:

1. Parsing is triggered on jd reserved words with no assumption on context.
   Therefore, enclosing jd reserved words within strings will fail: e.g.,
   
   #include <jd.h>
   const char * fail_string = "FSSTRUCT";
   FSSTRUCT() foo { /* ... */ };
   // ...

Copyright 2013, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

import ply.lex
import re

states = (
    # there is an INITIAL state which ignores everything until it sees a 
    # jd keyword
    ( 'jdstruct', 'exclusive' ),	# parsing annotated C struct 
    ( 'jdkwargs', 'exclusive' ),	# parsing keyword arguments inside annotations
    ( 'jdconst', 'exclusive' ),     # parsing annotated C enum
    ( 'jdcexpr', 'exclusive' ), 	# copying arbitrary C expression
    ( 'jdaexpr', 'exclusive' ), 	# copying arbitrary C expression for array
    ( 'jdvexpr', 'exclusive' ), 	# copying arbitrary C expression for enum value
    ( 'jdattr',  'exclusive' ), 	# copying arbitrary C expression from attribute
)

# add to this for any new global annotations
jd_decls = [
    'ADDRSPACE',
    'VECTOR',
    'DEFINE',
]

jd_structs = [
    'FSSTRUCT',
    'FSSUPER',
    'FSCONST',
]

# jd reserved words 
jd_reserved = [
    'FIELD',
    'POINTER',
    'CHECK',
] + jd_structs + jd_decls

# reserved words during keyword argument
jd_keywords = [
    'name',
    'repr',
    'type',
    'when',
    'size',         # the size (in bytes) of the vector or object
    'count',        # the number of elements within the vector
    'expr',
    'null',
    'sentinel',
    'location',
    'rank',         # the rank of the struct in file system hierarchy
    'base',         # specifies base struct for derived structs
]

reserved = {
    # original C reserved words
    'struct' : 'STRUCT',
    'union' : 'UNION',
    'int' : 'INT',
    'float' : 'FLOAT',
    'double' : 'DOUBLE',
    'long' : 'LONG',
    'char' : 'CHAR',
    'unsigned' : 'UNSIGNED',
    'short' : 'SHORT',
    'signed' : 'SIGNED',
    'enum' : 'ENUM',
    'const' : 'CONST',
    'typedef' : 'TYPEDEF',
    'volatile' : 'VOLATILE',
    '__attribute__' : "ATTRIBUTE",      
}

# adds all jd reserved words to the dict of reserved words
reserved.update(dict((rw, rw) for rw in jd_reserved))

tokens = [
    'IDENT',  
    'EXPR',     # hack to extract arbitrary C expression
    'KEYWORD',
] + list(reserved.values())

literals = "=|+-*/[]{};!?%&^<>\"\',~()"

# -----------------------------------
#  common rules (used by all states)
# -----------------------------------

# Define a rule so we can track line numbers
def t_INITIAL_jdstruct_jdkwargs_jdconst_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)
    
# Skip all errors during raw parsing
def t_INITIAL_jdaexpr_jdcexpr_jdvexpr_jdattr_error(t):
    t.lexer.skip(1)

# ignore spaces during regular parsing
t_jdstruct_jdkwargs_jdconst_ignore  = ' \t\r'

# Print out error during regular parsing
def t_jdstruct_jdkwargs_jdconst_error(t):
    print "Illegal character '%s'" % t.value[0]
    t.lexer.skip(1)
    
# ignore everything except round brackets (required to count scope), comma,
# and quotation marks (single and double)
t_jdcexpr_jdattr_ignore  = ' \t\r\n' + re.sub(r"""[(),"']""", "", literals)

# -----------------------------------------------------------------
# initial lexer (skips everything until it encounters JD keywords)
# -----------------------------------------------------------------

def t_IDENT(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    triggers = jd_structs + jd_decls
    if t.value == "typedef":
        # look ahead to see if it is followed by an annotated struct
        if t_IDENT.re_obj.match(t.lexer.lexdata[t.lexer.lexpos:]):
            t.type = reserved[t.value]
            return t
    elif t.value in triggers:
        t.type = reserved[t.value]
        if t.value in jd_structs:
            if t.value == "FSCONST":
                t.lexer.push_state('jdconst')
            else:
                t.lexer.level = 0
                t.lexer.push_state('jdstruct') 
        t.lexer.push_state('jdkwargs')
        return t
t_IDENT.re_obj = re.compile(r"\s+(" + r"|".join(jd_structs) + r")\W")

# A string containing ignored characters (spaces and tabs)
t_ignore  = ' \t\r' + literals

# -----------------------------------------------------------
# code used to keep track of scopes of arbitrary expressions
# -----------------------------------------------------------

class Stack(object):
    """
    Simple stack implementation with python list
    """
    def __init__(self):
        self.data = list()
        
    def push(self, item):
        """
        push something (except for None) onto the stack
        """
        if item is not None:
            self.data.append(item)
        
    def peek(self):
        """
        Looks at top of the stack, returns None is stack is empty
        """
        if len(self.data) == 0:
            return None
        else:
            return self.data[-1]
            
    def pop(self):
        if len(self.data) == 0:
            return None
        else:
            return self.data.pop()
            
    def __len__(self):
        return len(self.data)

def expr_init(t, state, first_item=None):
    t.lexer.stack = Stack()
    if first_item is not None:
        t.lexer.stack.push(first_item)
    t.lexer.start = t.lexer.lexpos
    t.lexer.push_state(state)

def expr_exit(t):
    t.lexer.lexpos -= 1
    expr = t.lexer.lexdata[t.lexer.start:t.lexer.lexpos]
    t.value = expr.strip()
    t.type = 'EXPR'
    del t.lexer.stack
    del t.lexer.start
    t.lexer.lineno += expr.count('\n')
    t.lexer.pop_state()
    return t

def expr_is_in_str(t):
    return t.lexer.stack.peek() in [ "'", '"' ]

def expr_try_exit(t):
    if t.lexer.stack.peek() is None:
        return expr_exit(t)
    elif not expr_is_in_str(t):
        if t.value == ')':
            assert(t.lexer.stack.peek() == '(')
            t.lexer.stack.pop()
    
# ------------------------------------
# jdstruct lexer (main struct parser)
# ------------------------------------

def t_jdstruct_IDENT(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value, 'IDENT')
    if t.type in jd_reserved:
        t.lexer.push_state('jdkwargs')
    if t.type == "ATTRIBUTE":
        expr_init(t, 'jdattr')
    return t

# keep track of level of braces so we can end jdstruct state
def t_jdstruct_lbrace(t):
    r'\{'
    t.lexer.level += 1
    t.type = '{'
    return t             

def t_jdstruct_rbrace(t):
    r'\}'
    t.lexer.level -= 1
    t.type = '}'
    return t
  
# hack for c expressions inside array definition
def t_jdstruct_lsbrack(t):     
    r'\['
    expr_init(t, "jdaexpr", t.value)
    t.type = t.value
    return t      
    
def t_jdstruct_scolon(t):
    r'\;'
    t.type = ';'
    if t.lexer.level == 0:
        del t.lexer.level
        t.lexer.pop_state()
    return t

# --------------
# jdconst lexer
# --------------

def t_jdconst_IDENT(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value, 'IDENT')
    return t

def t_jdconst_equal(t):
    r'\='
    expr_init(t, 'jdvexpr')
    t.type = t.value
    return t

def t_jdconst_scolon(t):
    r'\;'
    t.type = ';'
    t.lexer.pop_state()
    return t

# ---------------
# jdkwargs lexer
# ---------------

def t_jdkwargs_IDENT(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    if t.value in jd_keywords:
        t.type = "KEYWORD"
    return t

def t_jdkwargs_equal(t):
    r'\='
    expr_init(t, 'jdcexpr')
    t.type = t.value
    return t

def t_jdkwargs_rbrack(t):
    r'\)'
    t.type = ')'
    t.lexer.pop_state()
    return t
    
t_jdkwargs_ignore  = ' \t\r'

def t_jdkwargs_error(t):
    t.lexer.skip(1)

# ---------------------------------------------------------------
# quotation handling rule for all arbitrary expression extractor
# ---------------------------------------------------------------
    
def t_jdcexpr_jdvexpr_jdaexpr_jdattr_quote(t):
    r"""(\\|)["']"""
    if expr_is_in_str(t):
        if t.lexer.stack.peek() == t.value:
            t.lexer.stack.pop()
    else:
        t.lexer.stack.push(t.value[-1])
    
# ----------------------
# jdcexpr/jdvexpr lexer
# ----------------------

def t_jdcexpr_comma_rbrack(t):
    r'\,|\)'
    return expr_try_exit(t)

def t_jdcexpr_jdvexpr_lbrack(t):
    r'\('
    if not expr_is_in_str(t):
        t.lexer.stack.push(t.value)

def t_jdvexpr_comma_rbrack(t):
    r'\,|\)|\}'
    return expr_try_exit(t)
    
t_jdvexpr_ignore = ' \t\r\n' + re.sub(r"""["'(),}]""", "", literals)

# ---------------
# jdaexpr lexer
# --------------

def expr_exit_if_empty(t, c):
    if not expr_is_in_str(t):
        assert(t.lexer.stack.peek() == c)
        t.lexer.stack.pop()
    if t.lexer.stack.peek() is None:
        return expr_exit(t)    

def t_jdaexpr_lbrack(t):
    r'\['
    if not expr_is_in_str(t):
        t.lexer.stack.push(t.value)
    
def t_jdaexpr_rbrack(t):
    r'\]'
    return expr_exit_if_empty(t, '[')
   
# ignore everything except square brackets
t_jdaexpr_ignore  = ' \t\r\n' + re.sub(r"""[[\]'"]""", "", literals)

# ---------------
# jdattr lexer
# --------------

def t_jdattr_lbrack(t):
    r'\('
    if not expr_is_in_str(t):
        t.lexer.stack.push(t.value)
    if len(t.lexer.stack) == 1:
        t.lexer.start = t.lexer.lexpos
        t.type = '('
        return t
    
def t_jdattr_rbrack(t):
    r'\)'
    return expr_exit_if_empty(t, '(')

# Build the lexer
lexer = ply.lex.lex()
lexer.lextokens.update({ 
    '{' : 1, 
    '}' : 1,
    ';' : 1,
    '=' : 1,
    '[' : 1,
    ')' : 1,
    '(' : 1,
})

# http://stackoverflow.com/questions/241327/python-snippet-to-remove-c-and-c-comments
# 
# handles nested C/C++ comments
#
def remove_comment(text):
    def replacer(match):
        s = match.group(0)
        if s.startswith('/'):
            return " " # note: a space and not an empty string
        else:
            return s
    pattern = re.compile(
        r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
        re.DOTALL | re.MULTILINE
    )
    return re.sub(pattern, replacer, text)

def main():
    import os, sys
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        print("usage: %s FILE..."%sys.argv[0])
        sys.exit(1)
    for fname in files:
        with open(fname) as f:
            print("%s:"%fname)
            lexer.lineno = 1
            cleaned_text = remove_comment(f.read())
            lexer.input(cleaned_text)
            for tok in lexer:
                print(tok)

if __name__ == "__main__":
    main()
    
