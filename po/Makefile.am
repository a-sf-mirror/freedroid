POFILES = fr_FR/LC_MESSAGES/freedroidrpg.po

MOFILES = $(POFILES:.po=.mo)

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
		../src/view.c

DOMAIN = $(PACKAGE)

COPYRIGHT_HOLDER = Freedroid RPG Development Team

MSGID_BUGS_ADDRESS = freedroid-discussion@lists.sourceforge.net

LOCALEDIR=$(pkgdatadir)/locale

EXTRA_DIST = $(DOMAIN).pot $(POFILES)

MSGMERGE_OPTIONS = --quiet --update
XGETTEXT_OPTIONS = --keyword=_ --keyword=N_ --force-po
SUFFIXES = .po .mo

.po.mo:
	$(MSGFMT) -o $@ $<

$(POFILES): $(DOMAIN).pot
	@lang=`echo $@ | sed 's/\.po$$//'`; \
	test -f $${lang}.po && $(MSGMERGE) $(MSGMERGE_OPTIONS) $${lang}.po $(DOMAIN).pot

$(DOMAIN).pot: $(SRCFILES) 
	$(XGETTEXT) $(XGETTEXT_OPTIONS) --default-domain=$(DOMAIN) \
	--copyright-holder='$(COPYRIGHT_HOLDER)' \
	--msgid-bugs-address='$(MSGID_BUGS_ADDRESS)' \
	-o $(DOMAIN).pot $(SRCFILES)

all-local: all-local-@USE_NLS@
all-local-no:
all-local-yes: $(DOMAIN).pot $(MOFILES)

install-data-local: install-data-local-@USE_NLS@
install-data-local-no: all-local
install-data-local-yes: all-local
	for file in $(MOFILES); do \
		$(INSTALL_DATA) -D $$file $(LOCALEDIR)/$$file; \
	done;

uninstall-local: uninstall-local-@USE_NLS@
uninstall-local-no:
uninstall-local-yes:
	for file in $(MOFILES); do \
		rm -f $(LOCALEDIR)/$$file; \
	done;
