## Process this file with automake to produce Makefile.in
noinst_DATA = $(srcdir)/atlas.txt

ATLASTOOLDIR = $(top_builddir)/tools/atlas

$(ATLASTOOLDIR)/make_atlas:
	$(MAKE) --directory=$(ATLASTOOLDIR)

PNGS = $(shell find $(srcdir) -name "iso_*.png" | sort)
OFFSETS = $(shell find $(srcdir) -name "iso_*.offset")

$(srcdir)/atlas.txt: $(ATLASTOOLDIR)/make_atlas $(PNGS) $(OFFSETS)
	@$(ATLASTOOLDIR)/make_atlas -D $(srcdir) $(srcdir)/floor_tiles_atlas 2048 1024 $(srcdir)/atlas.txt $(PNGS)

clean:
	rm -f $(srcdir)/atlas.txt
