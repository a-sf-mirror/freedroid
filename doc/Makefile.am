## Process this file with automake to produce Makefile.in
# $Id: Makefile.am,v 1.26 2007-09-15 18:02:36 ahuillet Exp $

SUBDIRS = manual

# Special construct to enable recursive call of doc.
# Should be replaced by AM_EXTRA_RECURSIVE_TARGETS in top
# configure.ac, but this needs automake 1.13
# Inspired by http://lackof.org/taggart/hacking/make-example/

DOCSUBDIRS = $(SUBDIRS:%=%-doc)

doc: local-doc $(DOCSUBDIRS)

local-doc:
	@rm -f doxygen.log
	doxygen Doxyfile

$(DOCSUBDIRS): 
	$(MAKE) -C $(@:%-doc=%) $(AM_MAKEFLAGS) doc

.PHONY: local-doc $(DOCSUBDIRS)
