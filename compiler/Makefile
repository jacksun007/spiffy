#
# Makefile for generating library code of each file system
#
# New file systems should be added to the fs/ folder. The name is the file 
# should be the class name of the file system, and the content should be a 
# newline separated list of files which contains annotated file system data
# structures
#
# Kuei (Jack) Sun
# kuei.sun@mail.utoronto.ca
#
# University of Toronto
# Mar 2016

DEPEND := depend.mk
FSYS := $(addsuffix .d,$(notdir $(wildcard fs/*)))
export SRCDIR := ../src
export INCDIR := ../include

# we will force the depend file to be remade if anything else needs to be
# remade so that we can catch any new template files being added
all: $(FSYS)

# make install is inside depend.mk
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPEND)
endif

clean:
	rm -f *.pyc $(DEPEND) *.d lib*.cpp lib*.h
	cd jd && rm -f *.pyc parser.out parsetab.py
	find . -name '*~' -delete

$(FSYS):
	xargs python jdc.py -n $(basename $@) < fs/$(basename $@)
	touch $@

$(DEPEND): depend.py
	python depend.py $@
	
# dummy target to avoid error when called as submake
install-libfs:

.PHONY: clean all install remove
.SUFFIXES:


