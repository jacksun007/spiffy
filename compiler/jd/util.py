#
# util.py
#
# utility functions used in jd compiler
#
# Author: Kuei (Jack) Sun
# E-mail: kuei.sun@utoronto.ca
#
# Copyright (C) 2016
# University of Toronto
#

import re, sys

def error(msg):
    """
    prints the error message and quits immediately
    """
    print(msg)
    sys.exit(1)

def is_integral_type(type_name):
    """
    TODO: this is very heuristics based and can (will) break easily
    """
    tokens = type_name.split(" ")
    for prim in [ "int", "char", "short", "long" ]:
        if prim in tokens:
            return True
    m = is_integral_type.prog.match(type_name)
    if m is not None:
        return True
    return False
# this is a static variable of is_integral_type
is_integral_type.prog = re.compile(r"_{,2}((l|b)e|u|s|U|S)(8|16|32|64)")

def is_big_endian(type_name):
    return is_big_endian.prog.match(type_name)
is_big_endian.prog = re.compile(r"(__|)be")


def get_boolean(item, karg):
    """
    Converts from C++ boolean keyword to Python boolean
    """
    val = item.get(karg, "false")
    try:
        return { "true" : True, "false" : False }[val] 
    except KeyError:
        error("Line %d: Error, %s argument must be either 'true' or "
            "'false'"%(item.lineno, karg))

