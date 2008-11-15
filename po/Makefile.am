POFILES_SRC =	fr_FR/LC_MESSAGES/freedroidrpg.po \
		de_DE/LC_MESSAGES/freedroidrpg.po \
		sv_SE/LC_MESSAGES/freedroidrpg.po \
		ru_RU/LC_MESSAGES/freedroidrpg.po

POFILES_DAT = 	fr_FR/LC_MESSAGES/freedroidrpg_data.po \
      		de_DE/LC_MESSAGES/freedroidrpg_data.po \
		sv_SE/LC_MESSAGES/freedroidrpg_data.po \
		ru_RU/LC_MESSAGES/freedroidrpg_data.po

POFILES_DIA = 	fr_FR/LC_MESSAGES/freedroidrpg_dialogs.po \
      		de_DE/LC_MESSAGES/freedroidrpg_dialogs.po \
		sv_SE/LC_MESSAGES/freedroidrpg_dialogs.po \
		ru_RU/LC_MESSAGES/freedroidrpg_dialogs.po


MOFILES_SRC = $(POFILES_SRC:.po=.mo)
MOFILES_DAT = $(POFILES_DAT:.po=.mo)
MOFILES_DIA = $(POFILES_DIA:.po=.mo)

DATAFILES = ../map/freedroid.*

DIALOGFILES = ../dialogs/*.dialog

SRCFILES = ../src/BFont.c \
		../src/BFont.h \
		../src/SDL_rotozoom.c \
		../src/SDL_rotozoom.h \
		../src/automap.c \
		../src/blocks.c \
		../src/bullet.c \
		../src/character.c \
		../src/chat.c \
		../src/defs.h \
		../src/enemy.c \
		../src/getopt.c \
		../src/getopt.h \
		../src/getopt1.c \
		../src/global.h \
		../src/graphics.c \
		../src/hud.c \
		../src/influ.c \
		../src/init.c \
		../src/input.c \
		../src/items.c \
		../src/lang.h \
		../src/leveleditor.c \
		../src/light.c \
		../src/main.c \
		../src/colldet.c \
		../src/map.c \
		../src/map.h \
		../src/menu.c \
		../src/misc.c \
		../src/mission.c \
		../src/open_gl.c \
		../src/proto.h \
		../src/saveloadgame.c \
		../src/scandir.c \
		../src/scandir.h \
		../src/ship.c \
		../src/shop.c \
		../src/skills.c \
		../src/sound.c \
		../src/sound.h \
		../src/sound_effects.c \
		../src/struct.h \
		../src/system.h \
		../src/takeover.c \
		../src/takeover.h \
		../src/text.c \
		../src/text_public.c \
		../src/vars.h \
		../src/view.c \
		../src/savestruct.c


COPYRIGHT_HOLDER = Freedroid RPG Development Team

MSGID_BUGS_ADDRESS = freedroid-discussion@lists.sourceforge.net

LOCALEDIR=$(DESTDIR)/$(pkgdatadir)/locale

EXTRA_DIST = freedroidrpg.pot freedroidrpg_data.pot freedroidrpg_dialogs.pot $(POFILES_SRC) $(POFILES_DAT) $(POFILES_DIA) CMakeLists.txt *.py

MSGMERGE_OPTIONS = --quiet --update
XGETTEXT_OPTIONS = --keyword=_ --keyword=N_ --force-po
SUFFIXES = .po .mo

.po.mo:
	$(MSGFMT) -o $@ $<

$(POFILES_SRC): freedroidrpg.pot 
	@lang=`echo $@ | sed 's/\.po$$//'`; \
	test -f $${lang}.po && $(MSGMERGE) $(MSGMERGE_OPTIONS) $${lang}.po freedroidrpg.pot

$(POFILES_DAT): freedroidrpg_data.pot
	@lang=`echo $@ | sed 's/\.po$$//'`; \
	test -f $${lang}.po && $(MSGMERGE) $(MSGMERGE_OPTIONS) $${lang}.po freedroidrpg_data.pot

$(POFILES_DIA): freedroidrpg_dialogs.pot
	@lang=`echo $@ | sed 's/\.po$$//'`; \
	test -f $${lang}.po && $(MSGMERGE) $(MSGMERGE_OPTIONS) $${lang}.po freedroidrpg_dialogs.pot

freedroidrpg.pot: $(SRCFILES) 
	$(XGETTEXT) $(XGETTEXT_OPTIONS) --default-domain=$(DOMAIN) \
	--copyright-holder='$(COPYRIGHT_HOLDER)' \
	--msgid-bugs-address='$(MSGID_BUGS_ADDRESS)' \
	-o freedroidrpg.pot $(SRCFILES)

freedroidrpg_data.pot: $(DATAFILES)
	python extract-strings-from-data.py ../map 'freedroid.*' freedroidrpg_data.pot

freedroidrpg_dialogs.pot: $(DIALOGFILES)
	python extract-strings-from-data.py ../dialogs '*.dialog' freedroidrpg_dialogs.pot

all-local: all-local-@USE_NLS@
all-local-no:
all-local-yes: freedroidrpg.pot freedroidrpg_data.pot freedroidrpg_dialogs.pot $(MOFILES_SRC) $(MOFILES_DAT) $(MOFILES_DIA)

install-data-local: install-data-local-@USE_NLS@
install-data-local-no: all-local
install-data-local-yes: all-local
	for file in $(MOFILES_SRC) $(MOFILES_DAT) $(MOFILES_DIA); do \
		$(INSTALL_DATA) -D $$file $(LOCALEDIR)/$$file; \
	done;

uninstall-local: uninstall-local-@USE_NLS@
uninstall-local-no:
uninstall-local-yes:
	for file in $(MOFILES_SRC) $(MOFILES_DAT) $(MOFILES_DIA); do \
		rm -f $(LOCALEDIR)/$$file; \
	done;
