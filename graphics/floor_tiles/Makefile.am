## Process this file with automake to produce Makefile.in
SUBDIRS = ../../gluem

noinst_DATA = atlas.txt

atlas.txt: iso_*.png iso_*.offset
	./../../gluem/make_atlas floor_tiles_atlas 2048 1024 atlas.txt iso_*.png

clean:
	rm -f atlas.txt
