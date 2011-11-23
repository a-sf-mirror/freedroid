## Process this file with automake to produce Makefile.in
# $Id: Makefile.am,v 1.26 2007-09-15 18:02:36 ahuillet Exp $

## make sure rebuilding uses the right aclocal-switches
ACLOCAL_AMFLAGS = -I m4

if NONLINUX
extradirs = 
else
extradirs = win32 croppy 
endif 

if CROSS_COMPILING
SUBDIRS = src gluem $(extradirs)
else
SUBDIRS = src gluem graphics $(extradirs)
endif

man_MANS = freedroidRPG.6

freedroid_data = map graphics sound dialogs

EXTRA_DIST = m4 $(man_MANS) $(freedroid_data) HELP_WANTED INSTALL.generic lua

dist-hook:
	make -C lua clean
	find $(distdir) -name ".svn"              | xargs rm -rf
	find $(distdir) -name "*~"                | xargs rm -f
	find $(distdir) -name ".#*"               | xargs rm -f

install-data-local:
	@echo "Installing the data-files ..."
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	find $(srcdir)/$(freedroid_data) -type d ! -path "*.svn*" -and ! -name "Makefile*" -and ! -name "*~" -and ! -name ".#*" -exec echo $(DESTDIR)$(pkgdatadir)/{} \; | xargs $(mkinstalldirs)
	find $(srcdir)/$(freedroid_data) -type f ! -path "*.svn*" -and ! -name "Makefile*" -and ! -name "*~" -and ! -name ".#*" -exec $(INSTALL_DATA) {} $(DESTDIR)$(pkgdatadir)/{} \;
	@echo "..done."

uninstall-local:
	@echo "Removing data-files ..."
	rm -rf $(DESTDIR)$(pkgdatadir)
	@echo "...done."


sign:
	@echo "Generating signatures ..."
	gpg --digest-algo SHA512  -o freedroidrpg-$(VERSION).tar.bz2.sig --detach-sign freedroidrpg-$(VERSION).tar.bz2
	md5sum freedroidrpg-$(VERSION).tar.bz2 > freedroidrpg-$(VERSION).tar.bz2.md5

## Win32 specific stuff follows here

win_txts = COPYING AUTHORS README ChangeLog
win_dir = freedroidrpg-win32
dist-win32: src/freedroidRPG.exe
	rm -rf $(win_dir)
	mkdir -p $(win_dir)
	cp -r graphics map sound dialogs $(win_dir)
	cp src/freedroidRPG.exe $(win_dir)
	for i in $(win_txts); do \
	   cp $$i $(win_dir)/$$i.txt; \
	done
	find $(win_dir) -name ".svn"              | xargs rm -rf
	find $(win_dir) -name "*~"                | xargs rm -f
	find $(win_dir) -name ".#*"               | xargs rm -f
