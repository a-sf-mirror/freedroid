## Process this file with automake to produce Makefile.in
noinst_DATA = atlas.txt

atlas.txt: iso_*.png iso_*.offset */iso_*.png */iso_*.offset
	make --directory=../../gluem/
	./../../gluem/make_atlas floor_tiles_atlas 2048 1024 atlas.txt iso_*.png */iso_*.png

clean:
	rm -f atlas.txt
