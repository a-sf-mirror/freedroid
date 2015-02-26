## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CPPFLAGS =
freedroidRPG_LDADD =

if BUILD_LUA
freedroidRPG_LDADD += $(top_builddir)/lua/liblua.a
AM_CPPFLAGS += -I$(top_srcdir)/lua
endif

if WIN32
freedroidRPG_LDADD += $(top_builddir)/win32/freedroidRPG.coff
endif

if USE_NLS
freedroidRPG_LDADD += @LIBINTL@ @LIBICONV@
AM_CPPFLAGS += -DLOCALEDIR=\"$(localedir)\"
endif

bin_PROGRAMS = freedroidRPG

EXTRA_DIST =

freedroidRPG_SOURCES = version.c \
	menu.c shop.c items.c character.c skills.c saveloadgame.c \
	event.c dynarray.c item_upgrades.c item_upgrades_ui.c addon_crafting_ui.c \
	lua.c luaconfig.c \
	armor.c \
	blocks.c \
	floor_tiles.c \
	graphics.c map.c colldet.c init.c \
	image.c \
	takeover.c bullet.c influ.c action.c misc.c text.c text_public.c sound.c \
	enemy.c input.c keyboard.c main.c hud.c view.c automap.c BFont.c font.c \
	defs.h map.h vars.h takeover.h global.h \
	proto.h struct.h system.h BFont.h \
	open_gl.c open_gl_atlas.c open_gl_debug.c mission.c chat.c light.c \
	getopt.c getopt1.c getopt.h scandir.c scandir.h sound_effects.c \
	lang.c lang.h lists.c lists.h \
	gen_savestruct.py savestruct_internal.c savestruct_internal.h \
	string.c pathfinder.c \
	benchmark.c rtprof.c \
	npc.c \
	obstacle_extension.c \
	quest_browser_ui.c \
	map_label.c \
	faction.c \
	waypoint.c \
	obstacle.c \
	widgets/widgets.c widgets/widgets.h widgets/widget_button.c widgets/widget_button.h \
	widgets/widget_group.c widgets/widget_group.h \
	widgets/widget_text.c widgets/widget_text.h \
	widgets/widget_background.c widgets/widget_background.h \
	widgets/widget_text_list.c widgets/widget_text_list.h \
	game_ui.c \
	pngfuncs.c pngfuncs.h \
	lvledit/lvledit.c lvledit/lvledit.h \
	lvledit/lvledit_actions.c lvledit/lvledit_actions.h \
	lvledit/lvledit_display.c lvledit/lvledit_display.h \
	lvledit/lvledit_beautify_actions.c lvledit/lvledit_beautify_actions.h \
	lvledit/lvledit_input.c \
	lvledit/lvledit_enemy.c \
	lvledit/lvledit_map.c lvledit/lvledit_map.h \
	lvledit/lvledit_menu.c lvledit/lvledit_menu.h \
	lvledit/lvledit_validator.c lvledit/lvledit_validator.h \
	lvledit/lvledit_object_lists.c lvledit/lvledit_object_lists.h \
	lvledit/lvledit_tools.c lvledit/lvledit_tools.h \
	lvledit/lvledit_tool_move.c lvledit/lvledit_tool_move.h \
	lvledit/lvledit_tool_place.c lvledit/lvledit_tool_place.h \
	lvledit/lvledit_tool_select.c lvledit/lvledit_tool_select.h \
	lvledit/lvledit_widget_map.c lvledit/lvledit_widget_map.h \
	lvledit/lvledit_widget_toolbar.c lvledit/lvledit_widget_toolbar.h \
	lvledit/lvledit_widget_categoryselect.c lvledit/lvledit_widget_categoryselect.h \
	lvledit/lvledit_widget_minimap.c lvledit/lvledit_widget_minimap.h \
	lvledit/lvledit_widgets.c lvledit/lvledit_widgets.h \
	mapgen/mapgen.c mapgen/mapgen.h mapgen/gram_simple.c animate.c \
	mapgen/themes.c mapgen/themes.h \
	lua/luaFD.c lua/luaFD_bindings.h lua/luaFD_tux.c lua/luaFD_npc.c

nodist_freedroidRPG_SOURCES = savestruct.c savestruct.h build_flags.h
BUILT_SOURCES = savestruct.h build_flags.h

savestruct.h: struct.h savestruct_internal.h gen_savestruct.py
	$(PYTHON) $(srcdir)/gen_savestruct.py $(srcdir)/struct.h savestruct

version.c: dummy
	version_string=`echo "char *freedroid_version = \"$$(git describe --tags 2>/dev/null || echo "@PACKAGE_VERSION@")$$(git diff --exit-code >& /dev/null || echo "_mod")\";"` \
	&& (echo $$version_string | cmp -s - $@) || echo $$version_string > $@

build_flags.h: Makefile
	@echo '#define BUILD_CFLAGS "$(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)"' >$@
	@echo '#define BUILD_LDFLAGS "$(AM_LDFLAGS) $(LDFLAGS) $(freedroidRPG_LDADD) $(LIBS)"' >>$@

dummy:

CLEANFILES = savestruct.c savestruct.h build_flags.h version.c

.PHONY: dummy
