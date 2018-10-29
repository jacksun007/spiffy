# !/bin/python
"""
yacc.py

Parser for the Jack Daniels annotation language for file systems 

Known restrictions (subset of C grammar by design):

1. No pointer fields.

2. No anonymous FSCONST, FSSTRUCT, or FSSUPER unless it is typedef'ed - this is
   because the annotation needs to refer to named type (and seriously, how do
   you refer to it yourself if they are anonymous?)
   
3. Correctness of arbitrary C expressions are not checked - and are copy-pasted
   verbatim into the final output (Bison does this too, so too bad).
   
4. Bit fields are currently not supported.

5. implicit pointers and vector fields are NOT supported inside inner unions 
   and structs. (see p_body vs. p_declaration_inner...)
   
Copyright 2013, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

import sys
import ply.yacc
from lex import tokens, lexer, remove_comment
from ast import Struct, Field, Annotation, Scalar, Array, Inner, \
    Type, Enum, Constant, Token

# adds last element to the first element (which is a list)
def add_to_list_nonzero(p, n=2, check=lambda p,n : None):
    if len(p) == 2:
        p[0] = [ p[1], ]
    else:
        p[0] = p[1]
        check(p, n)
        p[0].append(p[n])

def check_user_defined_type_name(name, lineno):
    if len(name) == 0:
        return
    elif name in check_user_defined_type_name.array:
        print("\nLine %d: Error, redeclaration of '%s'"%(lineno, name)),
        raise SyntaxError  
    else:
        check_user_defined_type_name.array.append(name)
check_user_defined_type_name.array = list()

def check_unique(p, n):
    """
    checks to see if the newly added item is unique in the list
    """
    assert(n > 0)
    if p[n] in p[0]:
        if hasattr(p[n], 'lineno'):
            lineno = p[n].lineno
        else:
            lineno = p.lineno(n)
        print("\nLine %d: Error, redeclaration of '%s'"%(lineno, p[n])),
        raise SyntaxError       

def p_jd(p):
    """
    jd : jdstruct
       | decl_annotation
       | vector_annotation
       | jdconst
    """
    p[0] = [ p[1], ]

def p_jd_recursive(p):
    """
    jd : jd jdstruct
       | jd decl_annotation
       | jd vector_annotation
       | jd jdconst
    """
    p[0] = p[1]
    p[0].append(p[2])

def p_jdstruct(p):
    """
    jdstruct : struct_annotation opt_attribute IDENT '{' body '}' \
                                 opt_attribute ';'
    """
    check_user_defined_type_name(p[3], p.lineno(3))
    annos = [ p[1], ] + p[5].annotations
    p[0] = Struct(p[3], p[5].fields, annos, lineno=p.lineno(4))

def p_jdstruct_typedef(p):
    """
    jdstruct : TYPEDEF struct_annotation opt_attribute \
                       opt_ident '{' body '}' \
                       opt_attribute typedef_names ';'
    """
    check_user_defined_type_name(p[4], p.lineno(4))
    annos = [ p[2], ] + p[6].annotations
    p[0] = Struct(p[4], p[6].fields, annos, p[9], lineno=p.lineno(5))

def p_typedef_name(p):
    """
    typedef_name : IDENT
    """
    check_user_defined_type_name(p[1], p.lineno(1))
    p[0] = p[1]
    
def p_typedef_names(p):
    """
    typedef_names : typedef_name
                  | typedef_names ',' typedef_name
    """
    add_to_list_nonzero(p, 3)

def p_opt_comma(p):
    """
    opt_comma : 
              | ','
    """
    pass

def p_enum_annotation(p):
    """
    enum_annotation : FSCONST '(' keyword_arguments opt_comma ')'
    """
    p[0] = Annotation(p[1], p[3], lineno=p.lineno(2))

# (jsun): we do not allow for anonymous enum because we would have no way to 
#         refer to it in our annotation: e.g., FIELD(repr=enum foo)
def p_jdconst(p):
    """
    jdconst : enum_annotation opt_attribute IDENT '{' enumerations \
                              opt_comma '}' opt_attribute semicolons
    """
    check_user_defined_type_name(p[3], p.lineno(3))
    p[0] = Enum(p[3], p[5], p[1])
    
def p_jdconst_typedef(p):
    """
    jdconst : TYPEDEF enum_annotation opt_attribute \
                      opt_ident '{' enumerations \
                      opt_comma '}' opt_attribute typedef_names semicolons
    """
    check_user_defined_type_name(p[4], p.lineno(4))
    p[0] = Enum(p[4], p[6], p[2], p[10]) 

def p_enumerations(p):
    """
    enumerations : enumeration
                 | enumerations ',' enumeration
    """
    add_to_list_nonzero(p, 3, check_unique)

def p_enumeration(p):
    """
    enumeration : IDENT opt_enum_value
    """
    check_user_defined_type_name(p[1], p.lineno(1))
    p[0] = Constant(p[1], p[2], lineno=p.lineno(1))
    
def p_opt_enum_value(p):
    """
    opt_enum_value : 
                   | '=' EXPR
    """
    if len(p) == 1:
        p[0] = None
    else:
        p[0] = p[2]

# we don't care about attributes
def p_opt_attribute(p):
    """
    opt_attribute : 
                  | attribute 
    """
    pass

def p_attribute(p):
    """
    attribute : ATTRIBUTE '(' EXPR ')'
    """
    pass

def p_body(p):
    """
    body : 
         | declarations
         | declarations decl_annotations
    """
    if len(p) == 1:
        # empty metadata
        p[0] = Token(fields=list(), annotations=list() )
    if len(p) == 2:
        p[0] = Token(fields=p[1], annotations=list() )
    elif len(p) == 3:
        for anno in p[2]:
            if anno.name not in Annotation.STRUCT:
                print("\nLine %d: Error, '%s' is not a "
                    "valid struct annotation"%(anno.lineno, anno.name)),
                raise SyntaxError
        p[0] = Token(fields=p[1], annotations=p[2] )

def p_body_annotations_only(p):
    """
    body : decl_annotations
    """
    p[0] = Token(fields=list(), annotations=p[1] )
    
def p_decl_anno_word(p):
    """
    decl_anno_word : ADDRSPACE
                   | CHECK
                   | POINTER
                   | FIELD
                   | DEFINE
    """
    p[0] = p[1]
   
def p_decl_annotation(p):
    """
    decl_annotation : decl_anno_word '(' keyword_arguments opt_comma ')' \
                      opt_semicolons
    """
    p[0] = Annotation(p[1], p[3], p[6], lineno=p.lineno(2))

def p_vector_annotation(p):
    """
    vector_annotation : VECTOR '(' keyword_arguments opt_comma ')' \
                        opt_semicolons
    """
    p[0] = Annotation(p[1], p[3], p[6], lineno=p.lineno(2))
    
def p_semicolons(p):
    """
    semicolons : ';'
               | semicolons ';'
    """
    if len(p) == 2:
        p[0] = 1
    else:
        p[0] = p[1] + 1
    
def p_opt_semicolons(p):
    """
    opt_semicolons : semicolons
                   |
    """
    if len(p) == 1:
        p[0] = 0
    else:
        p[0] = p[1]

def p_decl_annotations(p):
    """
    decl_annotations : decl_annotation
                     | decl_annotations decl_annotation
    """
    add_to_list_nonzero(p)

def p_struct_anno_word(p):
    """
    struct_anno_word : FSSUPER
                     | FSSTRUCT
    """
    p[0] = p[1]
    
def p_struct_annotation(p):
    """
    struct_annotation : struct_anno_word '(' keyword_arguments opt_comma ')'
    """
    p[0] = Annotation(p[1], p[3], lineno=p.lineno(2))
 
def p_opt_field_annotations(p):
    """
    opt_field_annotations : 
                          | decl_annotations
    """
    if len(p) == 1:
        p[0] = list()
    else:
        for anno in p[1]:
            if anno.num_semicolons > 0:
                print("\nLine %d: Syntax Error, semicolon(s) after field "
                    "annotations '%s'"%(anno.lineno, anno.name)),
                raise SyntaxError
            if anno.name not in Annotation.FIELD:
                print("\nLine %d: Error, '%s' is not a "
                    "valid field annotation"%(anno.lineno, anno.name)),
                raise SyntaxError
        p[0] = p[1]
    
def p_declarations(p):
    """
    declarations : declaration
                 | declarations declaration 
    """
    if len(p) == 2:
        p[0] = p[1]
    else:
        for item in p[2]:
            if item in p[1]:
                print("\nLine %d: Error, duplicate member '%s'"%(
                    item.lineno, item.name)),
                raise SyntaxError    
        p[0] = p[1] + p[2]

def create_fields(spec_qual_list, annos, vars):
    """
    Creates fields from vars with the specified type (spec_qual_list) and 
    annotations (annos)
    """
    fields = list()
    for var in vars:
        if hasattr(var, 'size'):
            # see p_variable_array
            field = Array(var.name, spec_qual_list, var.size, annos, var.lineno)
        else:
            field = Scalar(var.name, spec_qual_list, annos, var.lineno)
        fields.append(field)
    return fields    

def p_declaration_named_type_field(p):
    """
    declaration : opt_field_annotations opt_qualifiers primitive_type \
                                        opt_qualifiers variables \
                                        semicolons
                | opt_field_annotations opt_qualifiers IDENT opt_qualifiers \
                                        variables semicolons
    """
    p[0] = create_fields(p[2] + Type(p[3]) + p[4], p[1], p[5])

def p_declaration_only_qualifier_field(p):
    """
    declaration : opt_field_annotations opt_qualifiers variables \
                                        semicolons
    """
    if p[2] is None:
        print("\nLine %d: Syntax error at token '%s', expected "
            "specifier qualifier list"%(p.lineno(3), p[3][0])),
        raise SyntaxError
    p[0] = create_fields(p[2], p[1], p[3])
    
def p_declaration_aggregate_field(p):
    """
    declaration : opt_field_annotations opt_qualifiers user_defined \
                                        opt_attribute IDENT \
                                        opt_qualifiers variables \
                                        semicolons
    """
    p[0] = create_fields(p[2] + Type("%s %s"%(p[3], p[5])) + p[6], 
                         p[1], p[7])

def p_declaration_inline_aggregate_field(p):
    """
    declaration : opt_field_annotations opt_qualifiers user_defined \
                                        opt_attribute opt_ident \
                                        '{' declarations '}' \
                                        opt_qualifiers opt_attribute \
                                        opt_variables semicolons
    """
    if p[3] not in [ "struct", "union" ]:
        print("\nLine %d: Syntax error at token '%s', must be struct or "
            "union"%(p.lineno(3), p[3])),
        raise SyntaxError
    if p[5] == "":
        name = p[3]
    else:
        name = "%s %s"%(p[3], p[5])
    vars = p[11]
    if len(vars) == 0:
        # using '}' as its lineno
        vars.append(Token(name="", lineno=p.lineno(8)))  
    p[0] = create_fields(p[2] + Inner(name, p[7]) + p[9], 
        p[1], vars)

# (jsun): we allow the placement of vector annotations in between fields
def p_declaration_vector_annotation(p):
    """
    declaration : vector_annotation
    """
    # must be placed inside a list (expected by declarations)
    p[0] = [ p[1], ]

def p_opt_variable(p):
    """
    opt_variables :
                  | variables
    """
    if len(p) == 1:
        p[0] = list()
    else:
        p[0] = p[1]
    
def p_opt_ident(p):
    """
    opt_ident : 
              | IDENT
    """
    if len(p) == 1:
        p[0] = ""
    else:
        p[0] = p[1]
        p.set_lineno(0, p.lineno(1))
    
def p_type_qualifier(p):
    """
    type_qualifier : CONST
                   | VOLATILE
                   | SHORT
                   | LONG
                   | UNSIGNED
                   | SIGNED
    """
    p[0] = Type(p[1])

    
    
def p_type_qualifiers(p):
    """
    type_qualifiers : type_qualifier
                    | type_qualifiers type_qualifier
    """
    if len(p) == 2:
        p[0] = p[1]
    else:
        p[0] = p[1]
        p[0].append(p[2])

def p_opt_qualifiers(p):
    """
    opt_qualifiers : 
                   | type_qualifiers
    """
    if len(p) == 1:
        p[0] = None
    else:
        p[0] = p[1]
    
def p_primitive_type_specifier(p):
    """
    primitive_type : INT
                   | CHAR
                   | FLOAT
                   | DOUBLE
    """
    p[0] = Type(p[1])

def p_user_defined_keyword(p):
    """
    user_defined : STRUCT
                 | UNION
                 | ENUM
    """
    p[0] = p[1]
    p.set_lineno(0, p.lineno(1))
    
def p_variables(p):
    """
    variables : variable
              | variables ',' variable
    """
    add_to_list_nonzero(p, 3, check_unique)

def p_variable_scalar(p):
    """
    variable : IDENT
    """
    p[0] = Token(name=p[1], lineno=p.lineno(1))

def p_array_dimension(p):
    """
    dimension : '[' EXPR ']'
    """
    p[0] = p[2]
    
def p_array_dimension_recursive(p):
    """
    dimensions : dimension
               | dimensions dimension
    """
    add_to_list_nonzero(p)
    
def p_variable_array(p):
    """
    variable : IDENT dimensions
    """
    p[0] = Token(name=p[1], size=p[2], lineno=p.lineno(1))
    
def p_keyword_argument(p):
    """
    keyword_argument : KEYWORD '=' EXPR
    """
    p[0] = { p[1] : p[3] }

def p_keyword_arguments(p):
    """
    keyword_arguments : 
                      | keyword_argument
                      | keyword_arguments ',' keyword_argument
    """
    p[0] = dict()
    if len(p) == 2:
        p[0].update(p[1])
    elif len(p) == 4:
        p[0] = p[1]
        p[0].update(p[3])
    
def p_error(p):
    if p is not None:
        print("\nLine %d: Syntax error at token '%s'"%(p.lineno, p.type)),
        sys.exit(1)

parser = ply.yacc.yacc()

def main():
    import os
    if len(sys.argv) > 1:
        files = sys.argv[1:]
    else:
        print("usage: %s FILE..."%sys.argv[0])
        sys.exit(1)
    structs = list()
    for fname in files:
        with open(fname) as f:
            print("Parsing %s..."%fname),
            cleaned_text = remove_comment(f.read())
            p = parser.parse(cleaned_text, debug=1)
            lexer.lineno = 1            # reset line number for each file
            if p is not None:
                print("done!")
                structs += p
            else:
                sys.exit(1)
    for s in structs:
        print str(s)    

if __name__ == "__main__":
    main()
