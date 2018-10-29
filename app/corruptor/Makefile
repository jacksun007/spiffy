#
# Makefile for FS Type-specific corruptor
#
# Kuei (Jack) Sun
# kuei.sun@mail.utoronto.ca
#
# University of Toronto
# 2018

CONF := debug

SOURCES   := $(wildcard *.cpp)
PROGS     := $(basename $(wildcard cr*.cpp))
DEPENDS   := $(SOURCES:.cpp=.d)
INCLUDE   := -I../../include
BUILDROOT := ../../build/corruptor
DEPEND    := depend.mk

CFLAGS    := -Wall $(INCLUDE) -Werror -Wextra -Wno-unused-parameter 
CFLAGS    += -Wfatal-errors -fno-exceptions -fno-rtti
ifeq ($(CONF),release)
CFLAGS += -O3
else ifeq ($(CONF),debug)
CFLAGS += -ggdb3
else
$(error CONF must be either debug or release)
endif
CXXFLAGS  := $(CFLAGS) -std=gnu++11

export BUILDDIR   := $(BUILDROOT)/$(CONF)
export LIBPATH    := ../../build/lib/$(CONF)
export OBJECTS    := $(addprefix $(BUILDDIR)/,corruptor.o blockio.o)
EXECUTABLE        := $(addprefix $(BUILDDIR)/,$(PROGS))
# e.g. build-crext3, used to trigger library remake before actual build
BUILDER           := $(addprefix build-,$(PROGS))
LIBRARY           := $(patsubst cr%,lib%,$(PROGS))

# ext3 has a special reader for its file address space
export EXT3_EXTRA := 

# f2fs has a special reader for its file address space
export F2FS_EXTRA := 

all: $(BUILDER)

# this forces install to happen so that you can switch between CONF
.PHONY: $(PROGS)
-include $(DEPEND)
install: all $(PROGS)

# - means we don't care if we can't include it
-include $(DEPENDS)

.PHONY: $(LIBRARY)
$(LIBRARY):
	cd ../../lib && $(MAKE) CONF=$(CONF) $@.a

$(LIBPATH)/libfs.a:
	cd ../../lib && $(MAKE) CONF=$(CONF) $(notdir $@)

$(BUILDER): build-cr% : lib% $(BUILDDIR)/cr%

$(EXECUTABLE):
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $@

$(DEPEND):
	python depend.py $@
	
.PHONY: clean
clean:
	rm -rf $(PROGS) *.exe *.stackdump *.o *~ $(DEPEND)
	rm -rf $(BUILDROOT)
	

