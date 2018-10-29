# !/bin/python
"""
semantic.py

A collection of functions which performs semantic analysis on the AST

Copyright 2014, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

from ast import Annotation, Struct, Type, Enum, Inner, Scalar, Array
from irep import Object, Container, Category, Constant, FileSystem, \
    Super, Addrspace, Field, Dimension, Pointer, Entity, Nested, Expression, \
    Check
from util import is_integral_type, error, get_boolean
import sys, re

def invalid_argument(lineno, key, anno):
    error("Line %d: Error, invalid argument '%s' in %s "
        "annotation"%(lineno, key, anno))

def missing_argument(anno, arg):
    if 'name' in anno.args:
        error("Line %d: Error, missing mandatory argument '%s' "
            "for annotation '%s' with name '%s'"%(anno.lineno, arg, 
            anno.name, anno['name'])) 
    else:
        error("Line %d: Error, missing mandatory argument '%s' "
            "for anonymous annotation '%s'"%(anno.lineno, arg, anno.name)) 

def validate_arguments(anno, white_list):
    """
    Make sure all arguments in the annotation 'anno' are in 'white_list'
    """
    for key in anno.args:
        if key not in white_list:
            invalid_argument(anno.lineno, key, anno.name)

def create_vector(item, cls):
    validate_arguments(item, [ 'name', 'type', 'size', 'sentinel', 'count' ])
    try:
        obj = cls(item['name'], item['type'], 
                size=item.get('size'), sentinel=item.get('sentinel'),
                count=item.get('count'))
        # (jsun): could possibly be specified on the parent pointer
        #if obj.size is None and obj.sentinel is None and obj.count is None:
        #    error("Line %d: Error, must specify at least one of 'size', " \
        #        "'count' or 'sentinel' argument (or all) for annotation" \
        #        " '%s'"%(item.lineno, item.name))
        return obj
    except KeyError as err:
        missing_argument(item, err.message)            
            
def create_container(item):
    return create_vector(item, Container)

def create_explicit_pointer(item):
    validate_arguments(item, [ 'type', 'when', 'repr', 'count', 'size' ] )
    try:
        return Pointer(item['repr'], item['type'], when=item.get('when'), 
            count=item.get('count'), size=item.get("size"))
    except KeyError as err:
        missing_argument(item, err.args[0])

def validate_field_annotation(anno, num_prev_seen):
    if num_prev_seen > 0:
        error("Line %d: Error, multiple %s annotations defined for"
            " member variable '%s'"%(anno.lineno, anno.name, item.name))
    assert(anno.name == "FIELD")
    validate_arguments(anno, [ 'type', 'when', 'count', 'sentinel' ])
        
def create_field(item, prefix=""):
    assert(isinstance(item.type, Type))
    pointers = list()
    num_field_annos = 0
    field_enum = None
    field_size = ''
    field_when = None
    field_sentinel = None
    for anno in item.annos:
        if anno.name == "POINTER":
            pointers.append(create_explicit_pointer(anno))
        else:
            validate_field_annotation(anno, num_field_annos)
            num_field_annos += 1
            field_enum = anno.get('type')
            field_size = anno.get('count', '')
            field_sentinel = anno.get('sentinel')
            field_when = anno.get('when')
    if isinstance(item, Array):
        size = item.sizes
        assert(size > 0)
        if len(size) == 1 and len(size[0]) == 0:
            # see if FIELD annotation has size for flexible array member
            if field_size != '':
                size[0] = field_size
            elif field_sentinel is not None:
                pass
            else:
                error("Line %d: Error, missing FIELD annotation with 'count' "
                    "or 'sentinel' argument for flexible array member '%s'"%(
                    item.lineno, item.name))
        elif field_size != '':
            error("Line %d: Error, invalid argument 'count' in %s annotations "
                "for non-flexible array member '%s'"%(anno.lineno, anno.name, 
                item.name))
        elif field_sentinel is not None:
            error("Line %d: Error, invalid argument 'sentinel' in %s annotation"
                " for non-flexible array member '%s'"%(anno.lineno, anno.name, 
                item.name))
        return Field(item.name, item.type.full_name, 
            size=Dimension(size), pointers=pointers, enum=field_enum,
            prefix=prefix, when=field_when, sentinel=field_sentinel)
    else:
        assert(isinstance(item, Scalar))
        if field_size != '':
            error("Line %d: Error, invalid argument 'count' in %s annotations "
                "for scalar member variable '%s'"%(anno.lineno, anno.name, 
                item.name))
        return Field(item.name, item.type.full_name, pointers=pointers,
            enum=field_enum, prefix=prefix, when=field_when)
    
def create_vector_field(anno):
    return create_vector(anno, Field)

def create_inner_field(item, prefix=""):
    num_field_annos = 0
    field_when = None
    for anno in item.annos:
        validate_field_annotation(anno, num_field_annos)
        num_field_annos += 1
        field_when = anno.get('when')
        for errarg in [ 'type', 'size' ]:
            if errarg in anno.args:
                error("Line %d: Error, invalid argument '%s' in %s "
                    "annotations for nested struct/union"%(anno.lineno, 
                    errarg, anno.name))
    inner = Nested(item.name, item.type.full_name, when=field_when)
    fields = list()
    populate_fields(item.type, fields, item.name)
    inner.fields = fields
    for field in fields:
        field.parent = inner
    return inner

    
def populate_fields(item, fields, prefix=""):
    for field in item.fields:
        # (jsun): item may be a VECTOR annotation
        if isinstance(field, Annotation):
            fields.append(create_vector_field(field))
        elif isinstance(field.type, Inner):
            # recursively add anonymous inner struct fields
            if isinstance(field, Scalar):
                fields.append(create_inner_field(field, prefix))
            else:
                raise NotImplementedError("Cannot handle array of structs yet")
        else:
            fields.append(create_field(field, prefix))    

def check_duplicate_pointer(fields, field_name, pointer):
    # see if an existing implicit field of that name exists               
    for field in fields:
        if field.name == field_name:
            if not field.is_implicit():
                error("Line %d: Error, field '%s' already exists and is not " \
                    "implicit"%(item.lineno, field.name))
            first_ptr = field.pointers[0]
            if (first_ptr.when is not None and pointer.when is None) or \
               (first_ptr.when is None and pointer.when is not None):
               error("Line %d: Error, implicit pointers of field '%s' "
                    "must all be conditional"%(item.lineno, field.name))
            if first_ptr.repr == "offset" and pointer.repr != first_ptr.repr:
                error("Line %d: Error, field '%s' mixes standard and offset "
                    "pointers"%(item.lineno, field.name))
            field.add_pointer(pointer)
            return False
    return True
                

def add_implicit_pointer(fields, item):
    """
    creates an implicit pointer, and add it to the list of fields
    """
    fake_data_type = 'long'
    validate_arguments(item, ['type', 'when', 'repr', 'name', 'expr', 'size'] )
    try:
        field_name = item['name']
        pointer = Pointer(item['repr'], item['type'], expr=item['expr'], 
                          when=item.get('when'), size=item.get('size'))
    except KeyError as err:
        missing_argument(item, err.args[0])     
       
    if not check_duplicate_pointer(fields, field_name, pointer):
        return
            
    # create a new one and add it to the list of fields
    fields.append(Field(field_name, fake_data_type, pointers=[ pointer ]))

def add_computed_field(fields, item):
    validate_arguments(item, ['type', 'name', 'expr'] )
    try:             
        if is_builtin_type(item['type']):
            data_type = item['type']
            enum_type = None
        else:
            data_type = 'long'
            enum_type = item['type']  
        fields.append(Field(item['name'], data_type, enum=enum_type, 
            expr=item['expr']))                           
    except KeyError as err:
        missing_argument(item, err.args[0])
            
            
def create_object(item):
    if len(item.name) == 0:
        name = item.alias[0]
    else:
        name = item.name
    fields = list()
    checks = list()
    populate_fields(item, fields)
    first_anno = item.annos[0]
    for anno in item.annos[1:]:
        if anno.name == "VECTOR":
            error("Line %d: Error, VECTOR is not a property annotation. " \
                  "Please report this as a compiler bug."%(anno.lineno))
        elif anno.name == "POINTER":
            add_implicit_pointer(fields, anno)
        elif anno.name == "FIELD":
            add_computed_field(fields, anno)
        else:
            assert(anno.name == "CHECK")
            validate_arguments(anno, [ 'expr' ])
            checks.append(Check(anno['expr']))
    if first_anno.name == "FSSTRUCT":
        validate_arguments(first_anno, [ 'size', 'name', 'base', 'when', 'rank' ])
        rank = first_anno.get('rank', 'object')
        if rank not in ['extent', 'container', 'object']:
            error("Line %d: Error, 'rank' must be one of 'extent', " \
                "'container' or 'object'"%(first_anno.lineno))
        if not first_anno.has('base') and first_anno.has('when'):
            error("Line %d: Error, 'when' argument specified for struct " \
            "without declaring a base struct"%(first_anno.lineno))
        obj = Object(name, fields, first_anno.get('size'), alias=item.alias,
            xref=first_anno.get('name'), base=first_anno.get('base'), 
            when=first_anno.get('when'), rank=rank, checks=checks)
    else:
        assert(first_anno.name == "FSSUPER")
        validate_arguments(first_anno, ['size', 'location', 'name'])
        create_object.num_super += 1
        if create_object.num_super > 1:
            error("Line %d: Error, multiple structs with FSSUPER annotation"%(
                first_anno.lineno))
        try:
            obj = Super(name, fields, first_anno.get('size'), 
                first_anno.get('location', 0), alias=item.alias,
                xref=first_anno.get('name'), checks=checks)
        except KeyError as err:
             missing_argument(first_anno, err.args[0])
    # reference each field with the parent object
    for field in fields:
        # the different between set_object and parent is that the object
        # is passed down to inner fields whereas the parent of a field may
        # be an inner field
        field.set_object(obj) 
        field.parent = obj
    return obj
create_object.num_super = 0

def create_enum(item):
    validate_arguments(item.anno, [ 'type', ])
    if len(item.name) == 0:
        name = item.alias[0]
    else:
        name = "enum %s"%item.name
    try:
        enum_type = item.anno['type']
        if enum_type not in [ 'flag', 'enum', 'partial' ]:
            error("Line %d: Error, 'type' argument of FSCONST must be "
                " 'flag', 'enum', or 'partial'"%(item.anno.lineno))
    except KeyError:
        enum_type = 'enum'
    elements = list()
    simple = True
    val = -1      # 1 is added before the first enumerator is created
    for element in item.elements:
        if element.value is None:
            if isinstance(val, int):
                val += 1
                const = Constant(element.name, str(val))
            else:
                val = "(%s)+1"%val
                const = Constant(element.name, val)
        else:
            try:
                curval = int(element.value)
                if curval < val:
                    simple = False
                val = curval
            except ValueError:
                val = element.value
                simple = False
            const = Constant(element.name, val)
        elements.append(const)
    return Category(name, elements, enum_type, simple, item.alias)


def create_address_space(addrspaces, item):
    """
    create an AddrSpace irep object from an AST item
    """
    validate_arguments(item, [ 'name', 'size', 'null' ])
    try:
        aspc_name = item['name']
    except KeyError as err:
        missing_argument(item, err.message)
    if aspc_name in addrspaces:
        error("Line %d: Error, address space '%s' already exists"%(
                item.lineno, aspc_name))
    addrspaces.append(Addrspace(aspc_name, item.get('size', 0), 
                                item.get('null', '0'), False))
    
    
def create_macro(item):
    """
    returns a tuple of substring and replacement
    """
    validate_arguments(item, [ 'name', 'expr' ])
    try:
        return (item['name'], item['expr'], )
    except KeyError as err:
        missing_argument(item, err.message)

# TODO (jsun): come back to fix this once we make bitmap/data work as either
#              container or field
def is_builtin_type(type_name):
    if type_name in Container.builtin_container_types:
        return True
    if type_name == "skip":
        return True
    return is_integral_type(type_name)
        
def increment_reference_count(type_table, type_name, parent):
    """ 
    Returns True on for built-in types
    Returns obj if found in type table
    Returns False if reference not found
    
    Also sets up pointer to object relationship
    """
    if is_builtin_type(type_name):
        return True
    for obj in type_table:
        all_names = [ obj.typename ] + obj.alias
        if type_name in all_names:
            parent.children.append(obj)
            obj.parents.append(parent)
            return obj
    return False

def validate_field_type(type_table, field, parent):
    """
    Same as increment_reference_count, but switches field.type if to an
    object (was string) if the type is annotated
    """
    if is_builtin_type(field.type):
        return True
    for obj in type_table:
        all_names = [ obj.typename ] + obj.alias
        if field.type in all_names:
            # (jsun): this is a hack to deal with toy file systems that use
            # enums directly as a type. enums are *not* metadata structures
            # so we need to treat it as a builtin type
            if isinstance(obj, Category):
                return True
            parent.children.append(obj)
            obj.parents.append(parent)
            field.type = obj
            obj.embedded = True
            return obj
    return False

def validate_base_struct(type_table, child):
    """
    Similar to above, but base struct must be an annotated struct. Also
    sets base reference of the child
    """
    for obj in type_table:
        all_names = [ obj.typename ] + obj.alias
        if child.base in all_names:
            # the parent/child relationship for derived classes is reversed
            # because derived classes need to appear AFTER the parent class
            # has been fully declared
            obj.parents.append(child)
            child.children.append(obj)
            child.base = obj    # override the string with the actual object
            obj.derived.append(child)
            return obj
    return None
   
def validate_pointer_addrspace(file_system, pointer):
    for aspc in file_system.addrspaces:
        if aspc.name == pointer.repr:
            # we set up the pointer repr to addrspace relationship here
            pointer.addrspace = aspc
            return True
    return False
   
   
def validate_all_fields(obj, fields, file_system):
    """
    validate all fields in object 'obj'.
    Important! on initial call, 'fields' argument must be 'obj.fields' 
              (due to recursion)
    """
    success = True
    for field in fields:
        # set up back-reference for each field
        field.object = obj
        if isinstance(field, Nested):
            # recursive validation
            validate_all_fields(obj, field.fields, file_system)
            continue
        else:
            # non-nested field, increment reference for its type
            retval = validate_field_type(file_system.type_table, field, obj)
            if retval == False:
                print("Warning, user-defined type '%s' of field '%s' in "
                    "object '%s' is not annotated"%(field.type,
                    field.name, obj.name, ))
                success = False
        if obj.rank == "extent":
            error("Error, extent rank for structures is currently unsupported"
                  " (%s)."%(obj.name))
        elif obj.rank == "container":
            if field.is_object() and not field.type.is_object():
                error("Error, container '%s' illegally contains "
                      "%s-ranked field '%s'"%(obj.name, field.type.rank, field.name))          
        conditional_ptr = False
        for pointer in field.pointers:
            errptr = file_system.add_pointer_type(pointer)
            if errptr is not None:
                error("Warning, pointers of the same type '%s' have "
                    "mismatching storage types '%s' and '%s' (field names are"
                    " '%s' and '%s')"%(pointer.type, errptr.parent.type, 
                    pointer.parent.type, errptr.parent.name, 
                    pointer.parent.name))
            retval = increment_reference_count(file_system.type_table, 
                pointer.type, obj) 
            if conditional_ptr and pointer.when is None:
                error("Error, conditional pointer field '%s' of object "
                "'%s' contains non-conditional pointer annotations"%(
                field.name, obj.name))
            if pointer.when is not None:
                conditional_ptr = True
            if retval == False:
                print("Warning, type '%s' referenced by field '%s' of "
                    "object '%s' is not annotated"%(pointer.type,
                    field.name, obj.name, ))
                success = False
            elif retval == True:
                error("Error, '%s' referenced by pointer field '%s' in object "
                "'%s' cannot be primitive or builtin type"%(
                pointer.type, field.name, obj.name))
            elif retval.is_object() and pointer.repr != "offset":
                error("Error, '%s' referenced by pointer field '%s' in object "
                "'%s' is neither a container nor an extent"%(
                retval.name, field.name, obj.name))
            elif pointer.repr == "offset" and not retval.is_object():
                error("Error, '%s' referenced by offset field '%s' in object "
                "'%s' cannot be a container or extent"%(
                retval.name, field.name, obj.name))    
            if not validate_pointer_addrspace(file_system, pointer):
                print("Warning, addrspace '%s' referenced by field '%s'" 
                    " of object '%s' is not declared"%(pointer.repr,
                    field.name, obj.name, ))
                success = False
            if success:
                pointer.target = retval  
        if field.enum is not None and field.enum not in Field.builtin_type:
            enum_found = False
        else:
            enum_found = True        
        # check to see if FSCONST enums are used
        for st in file_system.enums:
            if st.name == field.type or st.name == field.enum:
                field.category = st
                st.refcount += 1
                enum_found = True
        if not enum_found:
            error("Error, '%s' referenced by enum field '%s' in object "
                "'%s' is not defined or annotated"%(
                field.enum, field.name, obj.name))    
    return success

def validate_object(obj, file_system):
    """
    this function will hook up all necessary references and validate
    all fields of the input object
    """
    if obj.base is not None:
        if not validate_base_struct(file_system.objects, obj):
            error("Error, '%s' referenced as base struct of '%s' is not "
                  "annotated"%(obj.base, obj.name, ))
            # error() will end program in error
    if obj.rank == "extent" and obj.size is None:
        error("Error, %s '%s' must explicitly declare its size"%(obj.rank, obj.name, ))          
    return validate_all_fields(obj, obj.fields, file_system)


def validate_inheritance(file_system):
    """
    this function will pass down annotations from the parent to the child
    classes (currently only the size argument)
    """
    for obj in file_system.objects:
        for child in obj.derived:
            if child.size is None:
                child.size = obj.size
            # if the parent is a container, the child must also be one
            if obj.rank < child.rank:
                if child.rank == "extent":
                    error("Error, base struct '%s' of extent '%s' has a " \
                          "lower rank (%s) than itself"%(obj.name, 
                          child.name, obj.rank))
                # otherwise it is ok for child to have a higher rank
            if child.rank < obj.rank:
                # by default, we raise the rank of the child to match
                # the base struct (it makes sense?)
                child.rank = obj.rank
 
           
def validate_file_system(file_system):
    success = True
    for obj in file_system.objects:
        if not validate_object(obj, file_system):
            success = False
    validate_inheritance(file_system)
    for obj in file_system.containers:
        retval = increment_reference_count(file_system.type_table, 
            obj.element.type, obj)
        if retval == False:
            print("Warning, type '%s' referenced by container '%s' is "
                "not annotated"%(obj.element.type, obj.name))
            success = False
        elif isinstance(retval, Entity):
            #print "assign element", retval, "to", obj
            obj.element = retval
        # otherwise retval is a builtin type -- do not override
    return success

def update_expression(callback, obj, names):
    """
    Similar to macro_substitute, except this time we wrap the expression 
    """
    for name in names:
        if hasattr(obj, name):
            member = getattr(obj, name)
            if member is None:
                continue
            callback(obj, name, member)

C_EXPRESSIONS = ['when', 'size', 'expr']
            
def update_expression_fields(callback, obj):
    """
    performs recursion on nested fields
    """
    for field in obj.fields:
        update_expression(callback, field, C_EXPRESSIONS)
        if isinstance(field, Field):
            for ptr in field.pointers:
                update_expression(callback, ptr, C_EXPRESSIONS)
        elif isinstance(field, Nested):
            update_expression_fields(callback, field)
                    
def update_expression_all(objects, containers, enums, addrspaces, callback):
    """
    Wrap each C expression using Expression object, which enables looking
    up variable dependencies
    """
    for container in containers:
        update_expression(callback, container, ['size', 'count', 'sentinel'])
    for obj in objects:
            update_expression(callback, obj, C_EXPRESSIONS)
            update_expression_fields(callback, obj)

def convert_to_expression(exprs):
    """
    using closure trick to pass this list into the function below
    """
    def callback(obj, name, member):
        """
        callback function to convert expressions from string to Expression object
        """
        if isinstance(member, Dimension):
            for i in xrange(len(member)):
                member[i] = Expression(member[i])
                exprs.append(member[i])
        else:
            assert(isinstance(member, str))
            exprs.append(Expression(member))
            setattr(obj, name, exprs[-1])
    return callback

def macro_substitute_all(macros, objects, containers, enums, addrspaces):
    """
    replaces all instances of each macro within all "arbitrary C" expressions
    """
    for macro in macros:
        ident, expr = macro
        expr = "(%s)"%expr  # add parenthesis
        prog = re.compile(r"\b%s\b"%re.escape(ident))
        def macrosub(obj, name, orig):
            """
            callback function to replace original expression with the macro 
            """
            if isinstance(orig, Dimension):
                for i in xrange(len(orig)):
                    repl = prog.sub(expr, orig[i])
                    orig[i] = repl
            else:
                repl = prog.sub(expr, orig)
                setattr(obj, name, repl)
        update_expression_all(objects, containers, enums, addrspaces, macrosub)
              
def create_file_system(name, ast):
    """
    given an abstract syntax tree, create the full internal representation
    of this file system
    """
    addrspaces = [ Addrspace("byte", "0", "0", True), 
                   Addrspace("offset", "0", "-1", True) ]
    objects = list()
    containers = list()
    enums = list()
    macros = list()
    exprs = list()
    root = None
    for item in ast:
        if isinstance(item, Annotation):
            if item.name == "DEFINE":
                macros.append(create_macro(item))
            elif item.name == "VECTOR":
                containers.append(create_container(item))
            else:
                assert(item.name == "ADDRSPACE")
                create_address_space(addrspaces, item)
        elif isinstance(item, Struct):
            objects.append(create_object(item))
        else:
            assert(isinstance(item, Enum))
            enums.append(create_enum(item))
    if create_object.num_super == 0:
        error("Error, missing struct with FSSUPER annotation (must have"
            " exactly one)")
    else:
        temp = [ s for s in objects if isinstance(s, Super) ]
        assert(len(temp) == 1)
    macro_substitute_all(macros, objects, containers, enums, addrspaces)
    update_expression_all(objects, containers, enums, addrspaces, \
        convert_to_expression(exprs))
    fs = FileSystem(name, objects, containers, enums, addrspaces, temp[0])
    success = validate_file_system(fs)
    for obj in fs.type_table:
        if not isinstance(obj, Super) and obj.refcount <= 0:
            print("Warning, annotated type '%s' is not referenced at all."%(
                obj.name))
            success = False
    if not success:
        sys.exit(1)
    return fs
    

    
