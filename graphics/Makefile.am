## Process this file with automake to produce Makefile.in
noinst_DATA = obstacles/atlas.txt obstacles/shadow_atlas.txt floor_tiles/atlas.txt

obstacles/atlas.txt: obstacles/iso_*.png obstacles/iso_*.offset
	cd obstacles; ./../../gluem/make_atlas obstacles_atlas 1024 512 atlas.txt iso_*.png

obstacles/shadow_atlas.txt: obstacles/shadow_*.png obstacles/shadow_*.offset
	cd obstacles; ./../../gluem/make_atlas obstacles_shadows_atlas 1024 256 shadow_atlas.txt shadow_*.png

floor_tiles/atlas.txt: floor_tiles/iso_*.png floor_tiles/iso_*.offset
	cd floor_tiles; ./../../gluem/make_atlas floor_tiles_atlas 1024 1024 atlas.txt iso_*.png
