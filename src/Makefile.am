## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

# Initialize automake variables
AM_CFLAGS   = @FD_CFLAGS@
AM_CPPFLAGS = @FD_CPPFLAGS@
AM_LDFLAGS  = @FD_LDFLAGS@
LIBS        = @FD_LIBS@ @LIBS@

freedroidRPG_LDADD =
if WIN32
freedroidRPG_LDADD += $(top_builddir)/win32/freedroidRPG.coff
endif

bin_PROGRAMS = freedroidRPG

freedroidRPG_SOURCES = \
	action.c addon_crafting_ui.c animate.c armor.c automap.c \
	benchmark.c BFont.c blocks.c bullet.c \
	character.c chat.c colldet.c \
	dynarray.c \
	enemy.c event.c extint_macros.h \
	faction.c floor_tiles.c font.c \
	game_ui.c getopt.c getopt1.c graphics.c \
	hud.c \
	image.c influ.c init.c input.c items.c item_upgrades.c item_upgrades_ui.c \
	keyboard.c \
	lang.c light.c lists.c lua.c luaconfig.c \
	main.c map.c map_label.c menu.c misc.c mission.c \
	npc.c \
	obstacle.c obstacle_extension.c open_gl.c open_gl_atlas.c open_gl_debug.c open_gl_shaders.c \
	pathfinder.c pngfuncs.c \
	quest_browser_ui.c \
	rtprof.c \
	saveloadgame.c savestruct_internal.c scandir.c shop.c skills.c sound.c sound_effects.c string.c \
	takeover.c text.c text_public.c title.c \
	view.c \
	waypoint.c \
	\
	BFont.h defs.h getopt.h global.h lang.h lists.h map.h pngfuncs.h proto.h savestruct_internal.h scandir.h struct.h system.h takeover.h vars.h \
	\
	gen_savestruct.py \
	\
	lua/luaFD.c lua/luaFD.h lua/luaFD_bindings.h lua/luaFD_tux.c lua/luaFD_npc.c \
	\
	lvledit/lvledit.c lvledit/lvledit.h \
	lvledit/lvledit_actions.c lvledit/lvledit_actions.h \
	lvledit/lvledit_beautify_actions.c lvledit/lvledit_beautify_actions.h \
	lvledit/lvledit_display.c lvledit/lvledit_display.h \
	lvledit/lvledit_enemy.c \
	lvledit/lvledit_input.c \
	lvledit/lvledit_map.c lvledit/lvledit_map.h \
	lvledit/lvledit_menu.c lvledit/lvledit_menu.h \
	lvledit/lvledit_object_lists.c lvledit/lvledit_object_lists.h \
	lvledit/lvledit_tools.c lvledit/lvledit_tools.h \
	lvledit/lvledit_tool_move.c lvledit/lvledit_tool_move.h \
	lvledit/lvledit_tool_place.c lvledit/lvledit_tool_place.h \
	lvledit/lvledit_tool_select.c lvledit/lvledit_tool_select.h \
	lvledit/lvledit_validator.c lvledit/lvledit_validator.h \
	lvledit/lvledit_widgets.c lvledit/lvledit_widgets.h \
	lvledit/lvledit_widget_categoryselect.c lvledit/lvledit_widget_categoryselect.h \
	lvledit/lvledit_widget_map.c lvledit/lvledit_widget_map.h \
	lvledit/lvledit_widget_minimap.c lvledit/lvledit_widget_minimap.h \
	lvledit/lvledit_widget_toolbar.c lvledit/lvledit_widget_toolbar.h \
	\
	mapgen/gram_simple.c \
	mapgen/mapgen.c mapgen/mapgen.h \
	mapgen/themes.c mapgen/themes.h \
	\
	savegame/convert.c savegame/savegame.h \
	savegame/filters_0_16.c \
	savegame/filters_0_16_1.c \
	\
	widgets/widgets.c widgets/widgets.h \
	widgets/widget_autoscroll_text.c widgets/widget_autoscroll_text.h \
	widgets/widget_background.c widgets/widget_background.h \
	widgets/widget_button.c widgets/widget_button.h \
	widgets/widget_group.c widgets/widget_group.h \
	widgets/widget_text.c widgets/widget_text.h \
	widgets/widget_text_list.c widgets/widget_text_list.h

EXTRA_DIST =

# Generated files rules, to be created before sources compilation
nodist_freedroidRPG_SOURCES = build_flags.h savestruct.c savestruct.h version.c
BUILT_SOURCES =  build_flags.h savestruct.h version.c
CLEANFILES = build_flags.h savestruct.c savestruct.h version.c

build_flags.h: Makefile
	@echo '#define BUILD_CFLAGS "$(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)"' >$@
	@echo '#define BUILD_LDFLAGS "$(AM_LDFLAGS) $(LDFLAGS) $(freedroidRPG_LDADD) $(LIBS)"' >>$@

savestruct.h: struct.h savestruct_internal.h gen_savestruct.py
	$(PYTHON) $(srcdir)/gen_savestruct.py $(srcdir)/struct.h savestruct

version.c: dummy
	@if $$(git status >&/dev/null) ; then \
	 	version_string=`echo "char *freedroid_version = \"$$(git describe --tags 2>/dev/null || echo "@PACKAGE_VERSION@")$$(git diff --no-patch --exit-code >& /dev/null || echo "_mod")\";"` ; \
	 else \
	 	version_string=`echo "char *freedroid_version = \"@PACKAGE_VERSION@\";"` ; \
	 fi ; \
	 if ! $$(echo $$version_string | cmp -s - $@) ; then \
	 	echo $$version_string > $@ ; \
	 fi

dummy:

.PHONY: dummy
