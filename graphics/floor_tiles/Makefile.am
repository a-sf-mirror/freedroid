## Process this file with automake to produce Makefile.in
noinst_DATA = $(srcdir)/atlas.txt

ATLASTOOLDIR = $(top_builddir)/tools/atlas

PNGS = $(shell find $(srcdir) -name "iso_*.png" -printf "%P\n")
OFFSETS = $(shell find $(srcdir) -name "iso_*.offset" -printf "%P\n")

$(srcdir)/atlas.txt: $(PNGS) $(OFFSETS)
	$(MAKE) --directory=$(ATLASTOOLDIR)
	$(ATLASTOOLDIR)/make_atlas $(srcdir)/floor_tiles_atlas 2048 1024 $(srcdir)/atlas.txt $(PNGS)

clean:
	rm -f $(srcdir)/atlas.txt
