## Process this file with automake to produce Makefile.in
noinst_DATA = $(srcdir)/atlas.txt $(srcdir)/shadow_atlas.txt

ATLASTOOLDIR = $(top_builddir)/tools/atlas

PNGS = $(shell find $(srcdir) -name "iso_*.png" -printf "%P\n" | sort)
OFFSETS = $(shell find $(srcdir) -name "iso_*.offset")

$(srcdir)/atlas.txt: $(PNGS) $(OFFSETS)
	$(MAKE) --directory=$(ATLASTOOLDIR)
	$(ATLASTOOLDIR)/make_atlas $(srcdir)/obstacles_atlas 2048 2048 $(srcdir)/atlas.txt $(PNGS)

SHADOW_PNGS = $(shell find $(srcdir) -name "shadow_*.png" -printf "%P\n" | sort)

$(srcdir)/shadow_atlas.txt: $(SHADOW_PNGS)
		$(MAKE) --directory=$(ATLASTOOLDIR)
		$(ATLASTOOLDIR)/make_atlas $(srcdir)/obstacles_shadows_atlas 2048 2048 $(srcdir)/shadow_atlas.txt $(SHADOW_PNGS)

clean:
	rm -f $(srcdir)/atlas.txt
	rm -f $(srcdir)/shadow_atlas.txt
