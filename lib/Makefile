#
# Makefile for building library (.a files) out of generated library source
#
# Kuei (Jack) Sun
# kuei.sun@mail.utoronto.ca
#
# University of Toronto
# Copyright Apr 2016
#

CONF=debug

BUILDROOT := ../build/lib
BUILDDIR  := $(BUILDROOT)/$(CONF)
SRCDIR    := ../src
COMPDIR	  := ../compiler

SOURCES   := $(wildcard $(SRCDIR)/*.cpp)
INSTALL   := $(notdir $(SOURCES:.cpp=.a))
MAKETGT   := $(notdir $(SOURCES))
LIBRARY   := $(addprefix $(BUILDDIR)/,$(INSTALL))
OBJECTS   := $(LIBRARY:.a=.o)
DEPENDS   := $(OBJECTS:.o=.d)
INCLUDE   := -I../include/

CFLAGS    := -Wall $(INCLUDE) -Werror -Wextra -Wno-unused-parameter \
             -Wfatal-errors -fno-exceptions -fno-rtti
ifeq ($(CONF),debug)
CFLAGS += -O0 -ggdb3
else ifeq ($(CONF),release)
CFLAGS += -O3
else
$(error CONF must be either debug or release)
endif
CXXFLAGS := $(CFLAGS) -std=gnu++11

all: $(LIBRARY)

-include $(DEPENDS)

# the %.cpp will force re-build of the source file from the compiler if needed
$(INSTALL): %.a: %.cpp $(BUILDDIR)/%.a

$(BUILDDIR)/%.a: $(BUILDDIR)/%.o
	$(AR) rcs $@ $^

# avoid deleting the intermediate object files
.SECONDARY:	
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CC) $(CXXFLAGS) -MMD -c $< -o $@

.PHONY: $(MAKETGT)
$(MAKETGT):
	$(MAKE) -C $(COMPDIR) install-$(basename $@)

.PHONY: clean
clean:
	rm -rf *.a *.o *~
	rm -rf $(BUILDROOT)
	
.SUFFIXES:

