SUBDIRS = po-src po-dialogs po-data

# Special constructs to enable recursive call of
# update-po and update-gmo.
# Should be replaced by AM_EXTRA_RECURSIVE_TARGETS in
# top configure.ac, but this needs automake 1.13
# Inspired by http://lackof.org/taggart/hacking/make-example/

# disables -j , hack to fix some race condition
.NOTPARALLEL: update-po update-gmo

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

# TX synchronization rules
# We call subdirs in sequence, to avoid to flood the Transifex server
# too much

TXTARGETS = tx-update tx-pull tx-push
TXPROJ    = freedroid-rpg
TXPROPS   = '{"category":"autoTM"}'
TXCRED    = $(shell $(top_srcdir)/po/tx-credentials)
# Create a list of potfiles:
#  foreach dir in $SUBDIRS - $(foreach ...)
#    - extract from Makevars the value of the DOMAIN parameter - $(shell ...)
#    - strip leading/trailing whitespaces - $(strip ...)
#    - add the dirname as a prefix - $(addprefix ...)
#    - add a .pot suffix - $(addsuffix ...)
TXPOTS  = $(foreach dir, $(SUBDIRS), $(addsuffix .pot, $(addprefix $(dir)/, $(strip $(shell gawk -F'=' -e '/^DOMAIN/{ print $$2; }' $(dir)/Makevars)))))

$(TXTARGETS):
	@test " $(TXCRED)" = " " \
	 && { \
	      echo "You do not have a Transifex account"; \
	      exit 1; \
	    }; \
	 for dir in $(SUBDIRS) ; do \
	     $(MAKE) -C $$dir $@; \
	 done; \
	 case $@ in \
	   tx-update) \
	     $(mkdir_p) tx; \
	     msgcat --width=80 --no-wrap --sort-by-file --use-first -o tx/all_in_one.pot $(TXPOTS); \
	     if test -r tx/all_in_one.pot; then \
	       if ! tx -q status -r $(TXPROJ).all_in_one 2&>/dev/null; then \
	         echo "Creating all_in_one"; \
	         tx -q set --auto-local -r $(TXPROJ).all_in_one "tx/all_in_one-<lang>.po" -t PO --source-lang en --source-file tx/all_in_one.pot --execute; \
	         if tx -q push -r $(TXPROJ).all_in_one -s; then \
	           curl_output=`curl -s -L $(TXCRED) -X PUT -d $(TXPROPS) -H "Content-Type: application/json" https://www.transifex.com/api/2/project/$(TXPROJ)/resource/all_in_one;` \
	           rtn=$$?; \
	           if test $$rtn -ne 0 -o " $$curl_output" != " OK"; then echo "Warning. Failed to set properties: $$curl_output (error code $$rtn)"; fi; \
	         fi; \
	       else \
	         echo "Updating all_in_one"; \
	         tx -q push -r $(TXPROJ).all_in_one -s; \
	       fi; \
	     fi; \
	     rm -rf tx; \
	     ;; \
	 esac;

.PHONY: $(TXTARGETS)

clean:
	rm -f ./po-data/*.gmo
	rm -f ./po-dialogs/*.gmo
	rm -f ./po-src/*.gmo
