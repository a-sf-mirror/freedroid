## Process this file with automake to produce Makefile.in

freedroidRPG_LDADD = ../lua/liblua.a

if WIN32
freedroidRPG_LDADD += ../win32/freedroidRPG.coff
endif

bin_PROGRAMS = freedroidRPG

EXTRA_DIST = dialed.py CMakeLists.txt

freedroidRPG_SOURCES = menu.c shop.c items.c character.c skills.c saveloadgame.c \
	event.c \
	lua.c \
	blocks.c graphics.c map.c colldet.c init.c \
	takeover.c bullet.c influ.c misc.c text.c text_public.c sound.c \
	enemy.c input.c keyboard.c main.c hud.c view.c automap.c BFont.c \
	defs.h map.h vars.h takeover.h global.h \
	proto.h struct.h system.h BFont.h \
	SDL_rotozoom.c SDL_rotozoom.h open_gl.c open_gl_atlas.c mission.c chat.c light.c \
	getopt.c getopt1.c getopt.h scandir.c scandir.h sound.h sound_effects.c \
	lang.h lists.c lists.h gen_savestruct.py savestruct.c savestruct.h \
	leveleditor.c leveleditor.h \
	leveleditor_actions.c leveleditor_actions.h \
	leveleditor_display.c leveleditor_display.h \
	leveleditor_grass_actions.c leveleditor_grass_actions.h \
	leveleditor_input.c \
	leveleditor_map.c leveleditor_map.h \
	leveleditor_menu.c leveleditor_menu.h \
	leveleditor_validator.c leveleditor_validator.h \
	leveleditor_tools.c leveleditor_tools.h \
	leveleditor_tool_move.c leveleditor_tool_move.h \
	leveleditor_tool_place.c leveleditor_tool_place.h \
	leveleditor_tool_select.c leveleditor_tool_select.h \
	leveleditor_widget_button.c leveleditor_widget_button.h \
	leveleditor_widget_map.c leveleditor_widget_map.h \
	leveleditor_widget_menu.c leveleditor_widget_menu.h \
	leveleditor_widget_toolbar.c leveleditor_widget_toolbar.h \
	leveleditor_widget_typeselect.c leveleditor_widget_typeselect.h \
	leveleditor_widgets.c leveleditor_widgets.h

savestruct.c savestruct.h: struct.h
	python2.5 gen_savestruct.py struct.h savestruct

clean-local:
	cd ../lua; make clean

../lua/liblua.a: 
	cd ../lua; make CC=${CC} AR="${AR} rcu" liblua.a
if WIN32
	i686-pc-mingw32-ranlib ../lua/liblua.a
endif
