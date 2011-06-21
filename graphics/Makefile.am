## Process this file with automake to produce Makefile.in
atlas_DATA = obstacles/atlas.txt obstacles/shadow_atlas.txt

atlasdir=graphics/obstacles/

obstacles/atlas.txt: obstacles/iso_*.png obstacles/iso_*.offset
	cd obstacles; ./../../gluem/make_atlas obstacles_atlas 2048 2048 atlas.txt iso_*.png

obstacles/shadow_atlas.txt: obstacles/shadow_*.png obstacles/shadow_*.offset
	cd obstacles; ./../../gluem/make_atlas obstacles_shadows_atlas 2048 1024 shadow_atlas.txt shadow_*.png
