#!/bin/python
"""
irep.py

classes for jd's internal representation (used by the code template)

Copyright 2014, University of Toronto

Report bugs to kuei.sun@utoronto.ca
"""

import collections
import cStringIO, itertools, re
from operator import attrgetter
from util import is_integral_type, is_big_endian

class Entity(object):
    """
    Represents any named 'things'
    """
    next_id = 0
    
    def __init__(self, name):
        self.id = Entity.next_id
        Entity.next_id += 1
        self.name = name.strip()
        
    def __repr__(self):
        return '%s(%s)'%(type(self).__name__, self.name)
    
    def __str__(self):
        """
        By default, casting an entity to a string will print its name
        """
        return self.name
    
    def debug(self):
        """
        method to debug the current entity
        
        """
        print repr(self)
        
    def args_to_str(self, *args):
        """
        turns a list of keyword arguments to comma separated strings
        """
        kwargs = [ (arg, getattr(self, arg, None), ) for arg in args ]
        temp = [ t for t in kwargs if t[1] is not None ]
        if len(temp) == 0:
            return ""
        argstr = ", ".join([ "%s=%s"%(s[0], str(s[1])) for s in temp ])
        return "[%s]"%argstr
    
    @classmethod
    def _name_to_classname(cls, orig):
        toupper = lambda m: m.group(2).upper()
        name = cls.classname_reg_expr.sub(toupper, orig)
        name = name.replace('.', '_')
        if name == orig:
            return name + "Obj"
        else:
            return name
    classname_reg_expr = re.compile(r"(^|_)([a-zA-Z])")
        
    @property
    def classname(self):
        """
        Returns name of class rather than name of C struct
        """
        return Entity._name_to_classname(self.name)
    
    @property
    def typename(self):
        return self.name
        
    def __eq__(self, other):
        """
        by default, we identity ourselves by our name
        """
        return self.name == other


    @property
    def vars(self):
        return list()


class Metadata(Entity):
    """
    A filesystem metadata object
    
    member variables:
    size: size of metadata, in bytes (can be a C expression)
    parents: parent metadata object(s)
    children: child metadata object(s), either by inheritance, pointer, or inclusion
    """
    def __init__(self, name, size=None, alias=[]):
        super(Metadata, self).__init__(name)
        self.size = size
        self.parents = list()
        self.children = list()
        self.alias = alias
    
    def is_vector_type(self):
        """
        is this metadata defined with VECTOR() annotation?
        """
        return isinstance(self, Container)
    
    def is_object(self):
        return not self.is_container() and not self.is_extent()
    
    def is_container(self):
        """
        is the metadata a container block?
        """
        return False
    
    @property
    def container(self):
        """
        returns the innermost container type
        
        (jsun): since extent and container are mutually exclusive types, it is
        easier to say return the first element is not an extent
        """
        ret = self
        while True:
            # (jsun): heterogeneous extents are excluded from this calculation
            if ret.is_extent() and hasattr(ret, 'element'):
                ret = ret.element      
            else:
                break
        return ret
    
    def is_extent(self):
        """
        is this a group containing another group or a container?
        """
        return False
    
    def is_nested(self):
        """
        is this a nested extent?
        """
        return False
    
    def is_embedded(self):
        """
        is the metadata a field of another metadata object?
        """
        return False
    
    def is_integral(self):
        """
        is the metadata of integral type? (never)
        """
        return False
    
    @property
    def typeid(self):
        """
        returns the name used for typeid (part of the fs TypeId enum)
        """
        idname = self.name.upper()
        if idname == self.name:
            return idname + "_ID"
        else:
            return idname
        
    @property
    def refcount(self):
        return len(self.parents)

    @property
    def vars(self):
        return self.size.vars
        
class FileSystem(Entity):
    """
    Represents a file system
    """
    def __init__(self, name, objects, containers, enums, addrspaces, root):
        super(FileSystem, self).__init__(name)
        self.addrspaces = addrspaces
        self.objects = objects
        self.containers = containers
        self.enums = enums
        self.root = root
        self.pointer_table = list()
        self.xrefs = list()
    
    @property
    def xrefname(self):
        return "%sXRef"%self.name
        
    def add_pointer_type(self, pointer):
        """
        This method will add pointer to the list of pointers in the filesystem.
        Duplicates will be removed. Note that we only care about pointer type,
        so the 'when' argument is ignored (only repr and type are considered).
        """
        for ptr in self.pointer_table:
            if ptr.repr == pointer.repr and ptr.type == pointer.type:
                if pointer.parent.is_implicit():
                    return None
                # warning is returned on the assumption all pointers are of the 
                # same storage type (e.g., no int for some and long for others)
                if ptr.parent.type != pointer.parent.type:
                    return ptr
        self.pointer_table.append(pointer)
        return None
    
    def _setup_xrefs(self):
        """
        setup the list of xrefs to make it available to the fs object
        """
        for obj in self.objects:
            if obj.xref is not None:
                self.xrefs.append(obj)
    
    class Deque(collections.deque):
        """
        adding an extra method to the existing deque class
        """
        def push_front(self, value):
                try:
                    self.remove(value)
                except ValueError:
                    pass
                finally:
                    self.appendleft(value)
                
    def _setup_object_table(self):
        """
        Iteratively insert an item from the unsorted list into the table,
        and insert its children into the unsorted list. If the 
        item already exists in the table, move it to the beginning
        of the table (because there are dependencies later in the table).
        The final table should start with the item with least dependency,
        and end with the item with the most dependency (i.e., the super
        block)
        """
        if not hasattr(self, 'object_table'):
            table = FileSystem.Deque()
            unsorted = [ obj for obj in self.objects if len(obj.parents) == 0 ]
            unsorted = collections.deque(unsorted)
            forward = set()    # list of classes to forward declare
            depend = collections.defaultdict(set)
            while len(unsorted) > 0:
                curr = unsorted.popleft()
                table.push_front(curr)
                for child in curr.children:
                    if child in unsorted:
                        forward.add(child)
                    if child not in depend[curr]:
                        depend[child].update(depend[curr])
                        depend[child].add(curr)
                        unsorted.append(child)
                    else:
                        # (jsun): we have a recursion here -- just add the 
                        # child to the back of the table immediately. We do not
                        # move an existing object to the back since the 
                        # recursion can only occur by pointer
                        forward.add(child)
                        if child not in table:
                            table.appendleft(child)
            self.object_table = list(table)
            self.forward_decl = list(forward)
                
    def setup(self):
        """
        calls all the private setup functions
        """
        self._setup_xrefs()
        self._setup_object_table()

    @property
    def type_table(self):
        return itertools.chain(self.objects, self.containers, self.enums)
    
    @property
    def new_address_spaces(self):
        return [ aspc for aspc in self.addrspaces if not aspc.generic ]
    
    def debug(self):
        print("%s:"%self.name)
        for aspc in self.addrspaces:
            aspc.debug()
        for obj in self.objects:
            obj.debug()
        for ctnr in self.containers:
            ctnr.debug()
        for enum in self.enums:
            enum.debug()
            
class Rank(object):
    """
    The rank of a file system metadata. This class allows for comparison
    """
    OBJECT = 1
    CONTAINER = 2
    EXTENT = 3
    
    order = { "object": OBJECT, "container" : CONTAINER, "extent" : EXTENT }
    trans = [ None, "object", "container", "extent" ]
                   
    def __init__(self, rank):
        self.rank = Rank.order[rank]
    
    def __str__(self):
        return Rank.trans[self.rank]
    
    def __repr__(self):
        return "Rank(%s)"%(str(self))
    
    def __eq__(self, rhs):
        if isinstance(rhs, Rank):
            return self.rank == rhs.rank
        elif isinstance(rhs, str):
            return self.rank == Rank.order[rhs]
        else:   # assume integer
            return self.rank == rhs

    def __ne__(self, rhs):
        if isinstance(rhs, Rank):
            return self.rank != rhs.rank
        elif isinstance(rhs, str):
            return self.rank != Rank.order[rhs]
        else:   # assume integer
            return self.rank != rhs
        
    def __lt__(self, rhs):
        if isinstance(rhs, Rank):
            return self.rank < rhs.rank
        elif isinstance(rhs, str):
            return self.rank < Rank.order[rhs]
        else: # assume integer
            return self.rank < rhs

  
class Object(Metadata):
    """
    Represents all concrete file system metadata
    
    name: type name of object
    alias: alias of type name
    fields: all fields of the object
    checks: all the check annotations for the object
    size: size of the object, in bytes, written as a C expression
    xref: cross reference name of the object
    base: base class of this object
    when: when this object is actually of its type
    derived: derived class(es) or this object
    """
    def __init__(self, name, fields, size=None, checks=list(), alias=[], 
        xref=None, base=None, when=None, rank=None):
        super(Object, self).__init__(name, size, alias)
        self.fields = fields
        self.checks = checks
        self.xref = xref 
        self.embedded = False
        self.base = base
        self.when = when 
        self.derived = []
        self.rank = Rank(rank)
    
    def is_object(self):
        return self.rank == Rank.OBJECT
    
    def is_container(self):
        return self.rank == Rank.CONTAINER
    
    def is_extent(self):
        return self.rank == Rank.EXTENT
    
    def is_embedded(self):
        """
        whether this object can be embedded into other objects as a field
        """
        return self.embedded

    def is_referenced(self):
        return (self.xref is not None)
    
    def debug(self):
        print("obj %s%s {"%(self.name, self.args_to_str("size")))
        for field in self.fields:
            print("  ")
            field.debug()
        for check in self.checks:
            check.debug()
        print("}")
        
    @property
    def typename(self):
        if len(self.alias) > 0:
            return self.alias[0]
        else:
            return 'struct ' + super(Object, self).typename
            
    def __str__(self):
        """
        When casted to a string, return its type name
        """
        return self.typename

    @property
    def refcount(self):
        return super(Object, self).refcount + (1 if self.base is not None else 0)

    @property
    def vars(self):

        vars = list()

        for check in self.checks:
            vars = list(set().union(vars, check.vars))

        if (self.when is not None):
            vars = list(set().union(vars, self.when.vars))

        if (self.size is not None):
            vars = list(set().union(vars, self.size.vars))

        return vars

class Super(Object):
    """
    Represents the super block, or the root of the file system tree
    """
    def __init__(self, name, fields, size=None, location=0, checks=list(),
        alias=[], xref=None):
        super(Super, self).__init__(name, fields, size, 
            checks=checks, alias=alias, xref=xref, rank="container")
        self.location = location


class Element(object):
    """
    Placeholder element object for built-in types or integral types
    """
    def __init__(self, _type, prefix):
        self.type = _type
        self.typename = _type
        self.classname = "%sElement"%prefix
    
    def is_big_endian(self):
        return is_big_endian(self.type)
    
    def is_integral(self):
        return is_integral_type(self.type)
    
    def is_container(self):
        return False
    
    def is_extent(self):
        return False
    
    def is_object(self):
        return True
    
    def __str__(self):
        return self.type

        
class Container(Metadata):
    """
    Represents a stretch of contiguous bytes that contains objects and/or
    other containers
    """
    def __init__(self, name, _type, size=None, sentinel=None, count=None):
        super(Container, self).__init__(name, size)
        self.sentinel = sentinel
        self.element = Element(_type, self.classname)
        self.count = count
    
    builtin_container_types = ["bitmap", "data"]
    
    def is_container(self):
        """
        this metadata is a container if it contains objects
        """
        return (not self.element.is_container() and \
                not isinstance(self.element, Container)) or \
               self.element.type in Container.builtin_container_types
    
    # note: is_container and is_extent should be disjoint -- i.e., if you are
    # a container, you cannot be an extent, and vice versa
    def is_extent(self):
        """
        this an extent if it is either a block group (contains containers)
        or a nested extent (contains other extents)
        """
        return not self.element.is_object() or isinstance(self.element, Container)
    
    def is_nested(self):
        """
        this is a nested extent
        """
        return self.element.is_extent()      
    
    def debug(self):
        print("ctnr %s {\n  %s%s;\n}"%(self.name, 
            self.element.typename, self.args_to_str("size", "sentinel")))

    @property
    def vars(self):

        vars = list()

        if (self.element is not None):
            vars = list(set().union(vars, self.element.size.vars))

        if (self.count is not None):
            vars = list(set().union(vars, self.count.vars))

        if (self.size is not None):
            vars = list(set().union(vars, self.size.vars))

        return vars


class Dimension(object):
    """
    Wrapper class for tuple of dimensions
    
    TODO: currently doesn't support proper casting to string if the dimension
    exceeds 1 (e.g., 2-dimensional array does not work)
    """
    def __init__(self, size):
        self.size = size
        
    def __len__(self):
        return len(self.size)
        
    def __getitem__(self, idx):
        return self.size[idx]
    
    def __setitem__(self, idx, val):
        self.size[idx] = val
        
    def __nonzero__(self):
        return len(self.size) != 0 and not \
             ( len(self.size) == 1 and len(self.size[0].expr) == 0 )
        
    def __str__(self):
        return ",".join([ "(%s)"%sz for sz in self.size ])


class BaseField(Entity):
    """
    base class for fields of an object
    """
    def __init__(self, name, _type, when):
        super(BaseField, self).__init__(name)
        self.type = _type
        self.when = when
        self.parent = None  # set later by the parent object or parent field
    
    def set_object(self, obj):
        """
        sets the parent object for which the field belongs
        """
        self.object = obj
    
    @property
    def namespace(self):
        this = self.parent
        name = this.classname
        while isinstance(this, BaseField):
            this = this.parent
            name = "%s::%s"%(this.classname, name)
        return "%s::%s"%(name, self.classname)
    
    @property
    def classname(self):
        """
        Returns name of class rather than name of C struct
        """
        if self.name == "":
            return "Anonymous" + self.type[0].upper() + self.type[1:] + \
                str(self.id)
        return super(BaseField, self).classname
    
    def is_container(self):
        return False
    
    def is_object(self):
        return isinstance(self.type, Metadata)
    
    def is_large(self):
        return isinstance(self.type, Metadata) and not self.type.is_object()
    
    def is_skip(self):
        return False
       
    def is_struct(self):
        return False
        
    def is_union(self):
        return False
    
    def is_aggregate(self):
        return isinstance(self, Nested)
    
    def is_implicit(self):
        return False
    
    def is_array(self):
        return False
    
    def is_flexible_array(self):
        return False
    
    def is_pointer(self):
        return False
    
    def is_offset(self):
        return False
        
    def is_conditional_pointer(self):
        return False
        
    def is_int(self):
        return False
    
    def is_big_endian(self):
        return is_big_endian.prog.match(self.type)
    
    def is_signed(self):
        return BaseField.is_signed.prog.match(self.type)
    is_signed.prog = re.compile(r"(s|int|long|short|char)")
    
    def is_cstr(self):
        return False
    
    def is_string_array(self):
        return False
    
    def is_enum(self):
        return False
        
    def is_anonymous_union(self):
        return self.type == "union" and len(self.name) == 0


class Nested(BaseField):
    """
    Represents an inner struct or union
    """
    def __init__(self, name, _type, fields=list(), when=None):
        super(Nested, self).__init__(name, _type, when)
        self.fields = fields
    
    @classmethod
    def is_struct_type(cls, name):
        return (cls.struct_prog.search(name) is not None)
    struct_prog = re.compile(r"^struct( |$)")
    
    @classmethod
    def is_union_type(cls, name):
        return (cls.union_prog.search(name) is not None)
    union_prog = re.compile(r"^union( |$)")    
    
    def is_struct(self):
        return Nested.is_struct_type(self.type)
        
    def is_union(self):
        return Nested.is_union_type(self.type)

    def set_object(self, obj):
        """
        sets the parent object for which the field belongs
        """
        self.object = obj
        for field in self.fields:
            field.set_object(obj)
            
        
class Field(BaseField):
    """
    Represents a field within an object
    """
    buffer_type = [ "bitmap", "data", "skip" ]
    builtin_type = [ "timestamp", "uuid" ] + buffer_type
    
    def __init__(self, name, _type, enum=None, size=None, pointers=list(),
        sentinel=None, prefix=None, when=None, count=None, expr=None):
        super(Field, self).__init__(name, _type, when)
        self.prefix = prefix
        self.size = size
        self.enum = enum
        self.pointers = pointers
        self.implicit = False
        for ptr in self.pointers:
            if ptr.is_implicit():
                self.implicit = True
                break
        self.sentinel = sentinel
        self.count = count      # (jsun): currently not used
        self.category = None    # this is filled by jdsem.validate_all_fields()
        for ptr in self.pointers:
            ptr.parent = self   # back-reference to the field
        self.object = None      # this is filled by validate_all_fields
        self.expr = expr 
        if self.type in Field.buffer_type:
            self.enum = self.type
            self.type = "unsigned char"
    
    def add_pointer(self, ptr):
        """
        add a new pointer to this field
        """
        self.pointers.append(ptr)
        ptr.parent = self
    
    def is_skip(self):
        return self.enum == "skip"
    
    def is_implicit(self):
        return self.implicit or self.expr is not None
    
    def is_array(self):
        if self.size is not None:
            # this performs __nonzero__ check
            if bool(self.size):
                return True
        return (self.sentinel is not None) or \
               (self.count is not None and len(self.count.expr) > 0)
    
    def is_flexible_array(self):
        if isinstance(self.size, Dimension):
            return 'self' in self.size[0].vars
        elif self.size is not None:
            return 'self' in self.size.vars
        return False
    
    def is_pointer(self):
        return len(self.pointers) > 0
        
    def is_offset(self):
        return (self.is_pointer() and self.pointers[0].repr == "offset")
        
    def is_conditional_pointer(self):
        for ptr in self.pointers:
            if ptr.when:
                return True
        return False
    
    is_int_regex = re.compile(r"\b(char|long|short|int|_{,2}(s|u|le|be|uint)(8|16|32|64))\b")
    
    def is_int(self):
        return Field.is_int_regex.search(str(self.type))
    
    def is_cstr(self):
        """
        Is the type of the field a string
        """
        return "char" == self.type and self.size is not None
    
    def is_string_array(self):
        """
        Is the type an array of strings
        """
        return "char" == self.type and self.size is not None and \
                isinstance(self.size, Dimension) and len(self.size) == 2
    
    def is_enum(self):
        """
        Is the type of the field an enumerated value
        """
        return self.category is not None
        
    @property
    def fullname(self):
        return self.prefix + self.name
    
    def debug(self):
        for pointer in self.pointers:
            pointer.debug()
        print("%s %s%s;"%(self.type, self.name, 
            self.args_to_str("enum", "size")))

    @property
    def vars(self):

        # @[ if pointer.size ]
        # location.size = (size_t)(@(pointer.size));
        # @[ elif pointer.count ]
        # location.size = (size_t)(@(pointer.count))*(@(pointer.target.element.size));
        # @[ elif pointer.target.size ]
        # location.size = (size_t)(@(pointer.target.size));
        # @[ elif pointer.target.count ]
        # location.size = (size_t)(@(pointer.target.count))*(@(pointer.target.element.size));
        # @[ else ]
        # location.size = sizeof(@(pointer.target.typename));
        # @[ endif ]

        vars = list()

        for pointer in self.pointers:

            if (pointer.when is not None):
                vars = list(set().union(vars, pointer.when.vars))

            if (pointer.expr is not None):
                vars = list(set().union(vars, pointer.expr.vars))

            if (pointer.size is not None):
                vars = list(set().union(vars, pointer.size.vars))

            try:
                vars = list(set().union(vars, pointer.target.size.vars))
            except:
                pass

            try:
                vars = list(set().union(vars, pointer.target.count.vars))
            except:
                pass

            try:
                vars = list(set().union(vars, pointer.target.element.size.vars))
                vars = list(set().union(vars, pointer.count.vars))
            except:
                pass

            try:
                vars = list(set().union(vars, pointer.target.count.vars))
                vars = list(set().union(vars, pointer.target.element.size.vars))
            except:
                pass

        # if statement # 1: checks for computed fields
        # if statement # 2: checks if field contains a list for size member

        if (self.expr is not None):
            vars = list(set().union(vars, self.expr.vars))

        if (type(self.size) is not None and type(self.size) is Dimension):
            vars = list(set().union(vars, self.size[0].vars))
        elif (self.size is not None):
            vars = list(set().union(vars, self.size.vars))

        return vars


class Pointer(Entity):
    """
    Represents a pointer
    """
    def __init__(self, _repr, _type, expr=None, when=None, count=None, size=None):
        self.repr = _repr
        self.type = _type
        self.when = when
        # only used by implicit pointers
        if expr is not None:
            self.expr = re.sub(r"\bcontainer\b", "0", expr)
            self.relative = (self.expr != expr)
        else:
            self.expr = None
            self.relative = None
        self.count = count
        self.size = size
        self.target = None          # to be filled by validate_all_fields
        self.addrspace = None       # to be filled by validate_all_fields
    
    @property
    def container(self):
        """
        return the innermost container type in a chain of nested extents
        """
        entity = self.target
        if self.repr == "offset" or not isinstance(entity, Metadata):
            # (jsun): the offset addrspace is the pointer representation that
            # allows pointing to non-containers
            return entity
        return entity.container
    
    def is_implicit(self):
        return self.expr is not None
    
    def __repr__(self):
        return 'Pointer("%s")'%self.type
        
    def debug(self):
        print('ptr %s'%self.args_to_str("repr", "type", "when", "name", 
                                        "expr", 'size'))


class Check(object):
    """
    Represents an arbitrary type-safe check of a file system metadata
    """
    def __init__(self, expr):
        self.expr = expr
    
    def __str__(self):
        return self.expr
        
    def __repr__(self):
        return 'Check(%s)'%self.expr

        
class Category(Entity):
    """
    Represents an enumerated type
    """
        
    def __init__(self, name, elements, _type, simple, alias=[]):
        super(Category, self).__init__(name)
        self.elements = elements
        self.type = _type
        self.simple = simple
        self.refcount = 0
        self.alias = alias
    
    @property
    def classname(self):
        return Entity._name_to_classname(self.name.split(" ")[-1])
        
    def debug(self):
        print("enum %s%s {"%(self.name, 
            self.args_to_str("type", "simple")))
        for element in self.elements:
            print("  ")
            element.debug()
        print("}")
        
    
class Constant(Entity):
    """
    Represents a named constant
    """
    def __init__(self, name, value):
        super(Constant, self).__init__(name)
        self.value = value
        
    def debug(self):
        print("%s = %s"%(self.name, self.value))
        
    
class Addrspace(Entity):
    """
    Represents an address space
    """
    def __init__(self, name, size, null, generic):
        super(Addrspace, self).__init__(name)
        self.size    = size
        self.null    = null     # the null value of the addrspace (usually 0)
        self.generic = generic  # whether the addrspace is default
    
    @property
    def enumname(self):
        return "%sAS_%s"%("FS::" if self.generic else "", self.name.upper())
    
    def debug(self):
        print("aspc %s%s;"%(self.name, 
            self.args_to_str("null", "generic")))


class Expression(object):
    """
    Represents a valid C expression
    """
    var_finder = re.compile(r"(?<=\b)[a-zA-Z_]\w*(?=\.)")
    
    # build regular expression from this expression
    prop_find = re.compile(r"(^\$|(?<=[^$a-zA-Z0-9_])\$)([a-zA-Z_]\w*\.)")
    prop_repl = r"\2get_location()."
    
    def __init__(self, expr):
        self.raw = expr
        self.vars = Expression.var_finder.findall(expr)
        self.expr = Expression.prop_find.sub(Expression.prop_repl, expr)
        
    def is_constant(self):
        return len(self.vars) == 0
        
    def __str__(self):
        return self.expr
        
    def __repr__(self):
        return self.raw
        
        
    
    
        
          
