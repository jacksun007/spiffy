# 
# Makefile for testfs
#
# Currently two build setting and two target setting are allows:
#
# CONF:
# debug - build in debug mode (gdb enabled
# release - build in release mode (with optimization)
#
# TARGET:
# base - the default
# snoop - with the snoop module enabled
#
# e.g. `make CONF=release TARGET=snoop` will build with snoop module in release
#
# Author: Kuei Sun, Ashvin Goel
# E-mail: kuei.sun@mail.utoronto.ca
#
# University of Toronto, 2015
#

# debug or release
CONF=debug

# valid target are snoop and base
#TARGET=base
TARGET=snoop

SOURCES  := testfs.c mktestfs.c cktestfs.c $(COMMON_SOURCES)
INCLUDES := -I../include
CFLAGS   := -Wall -Wextra -Werror -Wno-unused-parameter $(INCLUDES)

BUILDROOT := ../build/testfs
BUILDDIR  := $(BUILDROOT)/$(TARGET)/$(CONF)

ifeq ($(CONF),debug)
CFLAGS   += -g -O0 -ggdb
else ifeq ($(CONF),release)
CFLAGS   += -O3
else
$(error CONF must be either debug or release)
endif

MAIN    := testfs
TOOLS   := mktestfs cktestfs
PROGS   := $(MAIN) $(TOOLS)
# (jsun): only .c files are considered common source, .cpp is used by snoop only
SOURCES := $(filter-out $(addsuffix .c,$(PROGS)),$(wildcard *.c))
OBJECTS :=  $(addprefix $(BUILDDIR)/,$(SOURCES:.c=.o))

ifeq ($(TARGET),snoop)
FSLIB       := libtestfs.a libfs.a
FSLIB_BUILD := ../build/lib/$(CONF)
FSLIB_MAKE  := ../lib

SNOOPLIB       := libsnoop.a
SNOOPLIB_BUILD := ../build/snoop/$(CONF)
SNOOPLIB_MAKE  := ../app/snoop

LIBS    := $(FSLIB) $(SNOOPLIB)
LIBRARY := $(addprefix $(FSLIB_BUILD)/,$(FSLIB)) \
           $(addprefix $(SNOOPLIB_BUILD)/,$(SNOOPLIB))

CFLAGS   += -D__SNOOP__ -I$(SNOOPLIB_MAKE)
else ifneq ($(TARGET),base)
$(error CONF must be either snoop or base)
endif

CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti -std=gnu++11
CFLAGS   += -std=gnu99

all: $(PROGS)

ifneq ($(MAKECMDGOALS),clean)
-include $(wildcard $(BUILDDIR)/*.d)
endif

ifeq ($(TARGET),snoop)
$(FSLIB):
	cd $(FSLIB_MAKE) && $(MAKE) $@ TARGET=user CONF=$(CONF)

$(SNOOPLIB):
	cd $(SNOOPLIB_MAKE) && $(MAKE) TARGET=user CONF=$(CONF)

$(BUILDDIR)/$(MAIN): % : %.o $(BUILDDIR)/snoop.o $(OBJECTS)
	$(CXX) -o $@ $(CXXFLAGS) $^ $(LIBRARY)
else
$(BUILDDIR)/$(MAIN): % : %.o $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $^
endif

# (jsun): the LIBS prerequisite will force all libraries to be re-built if needed
.PHONY: $(PROGS)
$(PROGS) : % : $(LIBS) $(BUILDDIR)/%
	cp $(BUILDDIR)/$@ $@

$(addprefix $(BUILDDIR)/,$(TOOLS)) : % : %.o $(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $^

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -MMD -c $< -o $@
	
$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@	

.PHONY: tar clean realclean

clean:
	rm -rf *.o $(BUILDROOT) $(PROGS) *.exe *.stackdump *~

realclean: clean
	rm -f *.img *.log *.out

tar:
	tar cvzf testfs.tar.gz *.c *.cpp *.h Makefile
	
ifeq (depend.mk,$(wildcard depend.mk))
include depend.mk
endif

