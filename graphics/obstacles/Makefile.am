## Process this file with automake to produce Makefile.in
noinst_DATA = $(srcdir)/atlas.txt $(srcdir)/shadow_atlas.txt

ATLASTOOLDIR = $(top_builddir)/tools/atlas

$(ATLASTOOLDIR)/make_atlas:
	$(MAKE) --directory=$(ATLASTOOLDIR)
	
PNGS = $(shell find $(srcdir) -name "iso_*.png" | sort)
OFFSETS = $(shell find $(srcdir) -name "iso_*.offset")

$(srcdir)/atlas.txt: $(ATLASTOOLDIR)/make_atlas $(PNGS) $(OFFSETS)
	@$(ATLASTOOLDIR)/make_atlas -D $(srcdir) $(srcdir)/obstacles_atlas 2048 2048 $(srcdir)/atlas.txt $(PNGS)

SHADOW_PNGS = $(shell find $(srcdir) -name "shadow_*.png" | sort)

$(srcdir)/shadow_atlas.txt: $(ATLASTOOLDIR)/make_atlas $(SHADOW_PNGS)
	@$(ATLASTOOLDIR)/make_atlas -D $(srcdir) $(srcdir)/obstacles_shadows_atlas 2048 2048 $(srcdir)/shadow_atlas.txt $(SHADOW_PNGS)

clean:
	rm -f $(srcdir)/atlas.txt
	rm -f $(srcdir)/shadow_atlas.txt
