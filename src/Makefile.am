## Process this file with automake to produce Makefile.in

if WIN32
freedroidRPG_LDADD = ../win32/freedroidRPG.coff
endif

bin_PROGRAMS = freedroidRPG

EXTRA_DIST = dialed.py CMakeLists.txt

freedroidRPG_SOURCES = menu.c shop.c items.c character.c skills.c saveloadgame.c \
	leveleditor.c leveleditor.h blocks.c graphics.c map.c colldet.c init.c \
	ship.c takeover.c bullet.c influ.c misc.c text.c text_public.c sound.c \
	enemy.c input.c main.c hud.c view.c automap.c BFont.c \
	defs.h map.h vars.h takeover.h global.h \
	proto.h struct.h system.h BFont.h \
	SDL_rotozoom.c SDL_rotozoom.h open_gl.c open_gl_atlas.c mission.c chat.c light.c \
	getopt.c getopt1.c getopt.h scandir.c scandir.h sound.h sound_effects.c \
	lang.h lists.c lists.h gen_savestruct.py savestruct.c savestruct.h

savestruct.c savestruct.h: struct.h
	python gen_savestruct.py struct.h savestruct
