$(BUILDDIR)/fspext3: $(BUILDDIR)/fspext3.o $(EXT3_EXTRA) $(OBJECTS) $(LIBPATH)/libext3.a $(LIBPATH)/libfs.a  
fspext3: $(BUILDDIR)/fspext3
	cp $< $@
