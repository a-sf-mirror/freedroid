## Process this file with automake to produce Makefile.in

freedroidRPG_LDADD = ../lua/liblua.a

if WIN32
freedroidRPG_LDADD += ../win32/freedroidRPG.coff
endif

bin_PROGRAMS = freedroidRPG

EXTRA_DIST =

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
	lvledit/lvledit.c lvledit/lvledit.h \
	lvledit/lvledit_actions.c lvledit/lvledit_actions.h \
	lvledit/lvledit_display.c lvledit/lvledit_display.h \
	lvledit/lvledit_grass_actions.c lvledit/lvledit_grass_actions.h \
	lvledit/lvledit_input.c \
	lvledit/lvledit_map.c lvledit/lvledit_map.h \
	lvledit/lvledit_menu.c lvledit/lvledit_menu.h \
	lvledit/lvledit_validator.c lvledit/lvledit_validator.h \
	lvledit/lvledit_tools.c lvledit/lvledit_tools.h \
	lvledit/lvledit_tool_move.c lvledit/lvledit_tool_move.h \
	lvledit/lvledit_tool_place.c lvledit/lvledit_tool_place.h \
	lvledit/lvledit_tool_select.c lvledit/lvledit_tool_select.h \
	lvledit/lvledit_widget_button.c lvledit/lvledit_widget_button.h \
	lvledit/lvledit_widget_map.c lvledit/lvledit_widget_map.h \
	lvledit/lvledit_widget_menu.c lvledit/lvledit_widget_menu.h \
	lvledit/lvledit_widget_toolbar.c lvledit/lvledit_widget_toolbar.h \
	lvledit/lvledit_widget_typeselect.c lvledit/lvledit_widget_typeselect.h \
	lvledit/lvledit_widgets.c lvledit/lvledit_widgets.h

savestruct.c savestruct.h: struct.h
	python2.5 gen_savestruct.py struct.h savestruct

clean-local:
	cd ../lua; make clean

../lua/liblua.a: 
	cd ../lua; make CC=${CC} AR="${AR} rcu" liblua.a
if WIN32
	i686-pc-mingw32-ranlib ../lua/liblua.a
endif
