#
# Makefile for the whole fs template project
#
# Kuei (Jack) Sun
# kuei.sun@mail.utoronto.ca
#
# University of Toronto
# 2015

CONF := release
APP  := $(notdir $(wildcard app/*))
LIB  := lib
PWD  := $(shell pwd)
FS   := testfs
SRC  := compiler

default: $(SRC) $(LIB)

all: default $(APP)

.PHONY: $(SRC) $(LIB)
$(SRC):
	@cd $(SRC) && $(MAKE) install && $(MAKE) install

$(LIB):
	@cd $(LIB) && $(MAKE) CONF=$(CONF)

install:
	@cd $(LIB) && $(MAKE) CONF=$(CONF) install

$(APP):
	@cd app/$@ && $(MAKE) CONF=$(CONF)

# clean in the reverse order as build
.PHONY: clean
clean:
	$(foreach X,$(FS) ,cd $(PWD)/$(X) && $(MAKE) clean &&) :
	$(foreach X,$(APP),cd $(PWD)/app/$(X) && $(MAKE) clean &&) :
	cd $(LIB)   && $(MAKE) clean
	cd compiler && $(MAKE) clean
	find . -name '*~' -delete

.PHONY: tar
tar:
	git ls-tree --full-tree -r HEAD | awk '{ print $$4 }' | xargs tar czvf fs-annotate.tar.gz
	
