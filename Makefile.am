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
	rsync -ar --exclude='.svn' --exclude='Makefile*' --exclude='*~' --exclude='.#*' dialogs graphics map sound $(win_dir)
	cp src/freedroidRPG.exe $(win_dir)
	for i in $(win_txts); do \
	   sed 's/$$/\r/' $$i > $(win_dir)/$$i.txt; \
	done

$(win_dir): dist-win32

package-win32: $(filter-out $(wildcard $(win_dir)), $(win_dir))
	rm -f freedroidRPG-*-win32.exe
	rm -f freedroidRPG.nsi
	win32/install_file_lists.py win32/freedroidRPG.nsi.in $(win_dir) > freedroidRPG.nsi
	makensis -V1 -DPRODUCT_VERSION="$(VERSION)" -DSRC_DIR="${win_dir}" freedroidRPG.nsi

macosx_dir = freedroidRPG.app
dist-macosx: src/freedroidRPG
	rm -rf $(macosx_dir)
	mkdir -p $(macosx_dir)/Contents
	mkdir -p $(macosx_dir)/Contents/MacOS
	mkdir -p $(macosx_dir)/Contents/Resources
	mkdir -p $(macosx_dir)/Contents/Frameworks
	cp macosx/app.icns $(macosx_dir)/Contents/Resources/Icon.icns
	cp src/freedroidRPG $(macosx_dir)/Contents/MacOS
	sed -e "s/__#PROGNAME#__/$(PACKAGE_NAME)/" -e "s/__#VERSION#__/$(PACKAGE_VERSION)/" macosx/Info.plist.in > $(macosx_dir)/Contents/Info.plist
	echo "APPL????" > $(macosx_dir)/Contents/PkgInfo
	rsync -ar --exclude='.svn' --exclude='Makefile*' --exclude='*~' --exclude='.#*' dialogs graphics map sound $(macosx_dir)/Contents
	perl macosx/pkgApp.pl $(macosx_dir)/Contents MacOS/freedroidRPG Frameworks
