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
SUBDIRS = src gluem graphics/floor_tiles graphics/obstacles $(extradirs)
endif

man_MANS = freedroidRPG.6

freedroid_data = map graphics sound dialogs

EXTRA_DIST = m4 $(man_MANS) $(freedroid_data) HELP_WANTED INSTALL.generic lua

check:
	xvfb-run -a ./src/freedroidRPG -nb text			|| exit 1
	xvfb-run -a ./src/freedroidRPG -nb dialog		|| exit 2
	xvfb-run -a ./src/freedroidRPG -nb loadship		|| exit 3
	xvfb-run -a ./src/freedroidRPG -nb dynarray		|| exit 4
	xvfb-run -a ./src/freedroidRPG -nb mapgen		|| exit 5
	xvfb-run -a ./src/freedroidRPG -nb leveltest	|| exit 6

dist-hook:
	make -C lua clean
	find $(distdir) -name ".git"              | xargs rm -rf
	find $(distdir) -name "*~"                | xargs rm -f
	find $(distdir) -name ".#*"               | xargs rm -f

install-data-local:
	@echo "Installing the data-files ..."
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	find $(srcdir)/$(freedroid_data) -type d ! -path "*.git*" -and ! -name "Makefile*" -and ! -name "*~" -and ! -name ".#*" -exec echo $(DESTDIR)$(pkgdatadir)/{} \; | xargs $(mkinstalldirs)
	find $(srcdir)/$(freedroid_data) -type f ! -path "*.git*" -and ! -name "Makefile*" -and ! -name "*~" -and ! -name ".#*" -exec $(INSTALL_DATA) {} $(DESTDIR)$(pkgdatadir)/{} \;
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
	rsync -ar --exclude='.git' --exclude='Makefile*' --exclude='*~' --exclude='.#*' dialogs graphics map sound $(win_dir)
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
	rsync -ar --exclude='.git' --exclude='Makefile*' --exclude='*~' --exclude='.#*' dialogs graphics map sound $(macosx_dir)/Contents
	perl macosx/pkgApp.pl $(macosx_dir)/Contents MacOS/freedroidRPG Frameworks

doc:
	doxygen Doxyfile

cppcheckflags = -I.                                                            \
				-I..                                                           \
				-I/usr/include/SDL                                             \
				--enable=performance,portability,unusedFunction,missingInclude \
				--force                                                        \
				--quiet

cppcheck:
	@echo "`cppcheck --version`"
	cppcheck gluem croppy src $(cppcheckflags)

gourceflags =	-c 0.8                           \
				--seconds-per-day 0.001          \
				--logo ./win32/w32icon_64x64.png \
				--auto-skip-seconds 0.0001       \
				--title "FreedroidRPG"           \
				--key                            \
				--camera-mode overview           \
				--highlight-all-users            \
				--file-idle-time 0               \
				--hide progress,mouse,filenames  \
				--stop-at-end                    \
				--max-files 99999999999          \
				--max-file-lag 0.000001          \
				--bloom-multiplier 1.3           \
				-1280x720

ffmpegflags =	-f image2pipe                    \
				-vcodec ppm                      \
				-i -                             \
				-y                               \
				-vcodec libx264                  \
				-preset medium                   \
				-crf 22                          \
				-pix_fmt yuv420p                 \
				-threads 2                       \
				-b:v 3000k                       \
				-maxrate 8000k                   \
				-bufsize 10000k FreedroidRPG.mp4


gource:
	gource $(gourceflags)

gource_ffmpeg:
	gource $(gourceflags) --output-ppm-stream - | ffmpeg $(ffmpegflags)


help:
	@echo "Makefile targets:"
	@echo " -----"
	@echo " * check          - Runs certain benchmarks."
	@echo " * doxygen        - Creates doxygen documentation."
	@echo " * cppcheck       - Checks the code using cppcheck (static code analysation)"
	@echo " * clean          - Remove files modified/generated by build"
	@echo " * gource         - Generate gource video from git log (requires git repository)"
	@echo " * gource_ffmpeg  - Generate gource video from git log and record using ffmpeg (requires git repository, ffmpeg and x264)"
