SUBDIRS = po-src po-dialogs

# Special constructs to enable recursive call of
# update-po and update-gmo.
# Should be replaced by AM_EXTRA_RECURSIVE_TARGETS in
# top configure.ac, but this needs automake 1.13
# Inspired by http://lackof.org/taggart/hacking/make-example/

UPDATEPODIRS = $(SUBDIRS:%=updatepo-%)

update-po: update-po-@USE_NLS@
update-po-no:
	@echo "NLS is disabled. Nothing to do."
update-po-yes: $(UPDATEPODIRS)

$(UPDATEPODIRS):
	$(MAKE) -C $(@:updatepo-%=%) $(AM_MAKEFLAGS) update-po

.PHONY: update-po-no update-po-yes $(UPDATEPODIRS)


UPDATEGMODIRS = $(SUBDIRS:%=updategmo-%)

update-gmo: update-gmo-@USE_NLS@
update-gmo-no:
	@echo "NLS is disabled. Nothing to do."
update-gmo-yes: $(UPDATEGMODIRS)

$(UPDATEGMODIRS):
	$(MAKE) -C $(@:updategmo-%=%) $(AM_MAKEFLAGS) update-gmo

.PHONY: update-gmo-no update-gmo-yes $(UPDATEGMODIRS)
