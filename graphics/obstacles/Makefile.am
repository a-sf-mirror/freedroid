## Process this file with automake to produce Makefile.in
noinst_DATA = atlas.txt shadow_atlas.txt

atlas.txt: iso_*.png iso_*.offset */iso_*.png */iso_*.offset
		./../../gluem/make_atlas obstacles_atlas 2048 2048 atlas.txt iso_*.png */iso_*.png

shadow_atlas.txt: shadow_*.png shadow_*.offset # */shadow_*.png */shadow_*.offset
		./../../gluem/make_atlas obstacles_shadows_atlas 2048 2048 shadow_atlas.txt shadow_*.png # */shadow_*.png

clean:
	rm -f atlas.txt
	rm -f shadow_atlas.txt
