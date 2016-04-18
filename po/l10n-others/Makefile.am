# The localized files handled here are not based on .po/.mo getext files.
# We generate one whole file per language, in a <lang> subdir.
# However, this Makefile implements the standard gettext rules, to be
# compatible with the main po/ Makefile. 

.NOTPARALLEL:

update-pot:
tx-push:

EXTRA_DIST = LINGUAS

.PHONY: update-pot update-po update-gmo
.PHONY: l10n-stats
.PHONY: tx-update tx-pull tx-push

#== Output statistics

l10n-stats:
	@echo "    Not yet implemented."

#== Transifex synchronization - Shared functions definition

TX       = tx
TXPROJ   = freedroid-rpg
TXPROPS  = '{"category":"other"}'
TXCREDS  = $(shell $(top_srcdir)/po/tx-credentials)

LANGS = $(shell tail -n +2 $(srcdir)/LINGUAS)

#-- Update tx rsrc
# $(1) = rsrc name
# $(2) = rsrc file

define rsrc_update =
	if ! $(TX) -q status -r $(TXPROJ).$(1) 2&>/dev/null; then \
	  echo "Creating $(1)"; \
	  $(TX) -q set --auto-local -r $(TXPROJ).$(1) "tx/$(1)-<lang>.txt" -t TXT --source-lang en --source-file $(2) --execute; \
	  if $(TX) -q push -r $(TXPROJ).$(1) -s; then \
	    curl_output=`curl -s -L $(TXCREDS) -X PUT -d $(TXPROPS) -H "Content-Type: application/json" https://www.transifex.com/api/2/project/$(TXPROJ)/resource/$(1);` \
	    rtn=$$?; \
	    if test $$rtn -ne 0 -o " $$curl_output" != " OK"; then echo "Warning. Failed to set properties: $$curl_output (error code $$rtn)"; fi; \
	  fi; \
	else \
	  echo "Updating $(1)"; \
	  $(TX) -q push -r $(TXPROJ).$(1) -s; \
	fi
endef

#-- Pull a translated rsrc
#-- If the translated rsrc does not contain any translated string, we remove it
#-- Note: TX does not correctly handle last blank lines and the last newline.
#-- So we add the possibly missing newline with a 'sed' script and do not
#-- take blank lines into account to compare files.
# $(1) = rsrc name
# $(2) = lang

define rsrc_pull =
	echo "Pulling $(1)-$(2) from transifex"; \
	$(TX) -q pull -r $(TXPROJ).$(1) -l $(2); \
	sed -i -e '$$a\' tx/$(1)-$(2).txt; \
	if diff -q --ignore-blank-lines tx/$(1).txt tx/$(1)-$(2).txt >/dev/null; then \
	  echo "    no translated strings: removing"; \
	  rm tx/$(1)-$(2).txt; \
	fi
endef

#-- Copy src file to dest file if src and dest are differents
#-- (this avoid to change dest file timestamp if src and dest are identicals) 
# $(1) = src file
# $(2) = dest file

define copy_on_diff =
	if ! cmp -s $(1) $(2) ; then \
	  rm -f $(2) && mv $(1) $(2); \
	  echo "$(2) updated"; \
	else \
	  rm $(1); \
	fi
endef

#== l10n of fdrpg 'base' title files

BASE_TITLES_DIR  = $(top_srcdir)/data/base/titles
BASE_TITLES_RSRC = $(basename $(notdir $(wildcard $(BASE_TITLES_DIR)/*.lua)))
BASE_TITLES_TX   = $(BASE_TITLES_RSRC:%=tx/title-%.txt)
BASE_TITLES_L10N = $(foreach lang,$(LANGS),$(BASE_TITLES_RSRC:%=$(BASE_TITLES_DIR)/$(lang)/%.lua))

#-- Create a TX rsrc by extracting the text part of a title
# $(1) = title name

define update-po-base-title =
 tx/title-$(1).txt: $(BASE_TITLES_DIR)/$(1).lua
	@$(mkdir_p) tx; \
	 $(AWK) "BEGIN { isintext=0; } /^text[[:space:]]*=[[:space:]]*[\[]+/ { isintext=1; next; } /^[\]]+/ { exit; } isintext == 1 { print; }" $$< > $$@
endef

$(foreach rsrc,$(BASE_TITLES_RSRC),$(eval $(call update-po-base-title,$(rsrc))))

#-- Merge a l10n version of a title's text with the title's header,
#-- and copy the result to map/titles
#-- Note: $(wilcard) if used so that an empty prerequisite list is generated if
#-- the translation file does not exist
# $(1) = base name
# $(2) = lang

define update-gmo-base-title =
 $(BASE_TITLES_DIR)/$(2)/$(1).lua: $(wildcard tx/title-$(1)-$(2).txt)
	@if test -r tx/title-$(1)-$(2).txt ; then \
	   $(mkdir_p) $(BASE_TITLES_DIR)/$(2); \
	   $(AWK) "/^text[[:space:]]*=[[:space:]]*[\[]+/ { print; nextfile; } { print; } END { print \"]]\"; print \"}\"; }" \
	        $(BASE_TITLES_DIR)/$(1).lua $$< > $$<.tmp; \
	   $(call copy_on_diff,$$<.tmp,$$@); \
	 fi
endef

$(foreach lang,$(LANGS),\
    $(foreach rsrc,$(BASE_TITLES_RSRC),\
        $(eval $(call update-gmo-base-title,$(rsrc),$(lang)))))

#-- update-po creates the new tx/title-<title name>.txt files

update-po:: $(BASE_TITLES_TX)

#-- Push all title rsrcs to TX

tx-update:: $(BASE_TITLES_TX)
	@if [ " $(TXCREDS)" = " " ] ; then \
		echo "You do not have a Transifex account"; \
		exit 1; \
	 fi
	@for rsrc in $(BASE_TITLES_RSRC); do \
	   $(call rsrc_update,title-$$rsrc,tx/title-$$rsrc.txt); \
	 done

#-- Pull all l10n versions of the titles' text from TX

tx-pull:: $(BASE_TITLES_TX)
	@if [ " $(TXCREDS)" = " " ] ; then \
		echo "You do not have a Transifex account"; \
		exit 1; \
	 fi
	@for lang in $(LANGS); do \
	   for rsrc in $(BASE_TITLES_RSRC); do \
	     $(call rsrc_pull,title-$$rsrc,$$lang); \
	   done; \
	 done

#-- update-gmo creates the new l10n version of the titles

update-gmo:: $(BASE_TITLES_L10N)

#== l10n of fdrpg 'story' title files

STORY_TITLES_DIR  = $(top_srcdir)/data/map/titles
STORY_TITLES_RSRC = $(basename $(notdir $(wildcard $(STORY_TITLES_DIR)/*.lua)))
STORY_TITLES_TX   = $(STORY_TITLES_RSRC:%=tx/title-%.txt)
STORY_TITLES_L10N = $(foreach lang,$(LANGS),$(STORY_TITLES_RSRC:%=$(STORY_TITLES_DIR)/$(lang)/%.lua))

#-- Create a TX rsrc by extracting the text part of a title
# $(1) = title name

define update-po-story-title =
 tx/title-$(1).txt: $(STORY_TITLES_DIR)/$(1).lua
	@$(mkdir_p) tx; \
	 $(AWK) "BEGIN { isintext=0; } /^text[[:space:]]*=[[:space:]]*[\[]+/ { isintext=1; next; } /^[\]]+/ { exit; } isintext == 1 { print; }" $$< > $$@
endef

$(foreach rsrc,$(STORY_TITLES_RSRC),$(eval $(call update-po-story-title,$(rsrc))))

#-- Merge a l10n version of a title's text with the title's header,
#-- and copy the result to map/titles
#-- Note: $(wilcard) if used so that an empty prerequisite list is generated if
#-- the translation file does not exist
# $(1) = base name
# $(2) = lang

define update-gmo-story-title =
 $(STORY_TITLES_DIR)/$(2)/$(1).lua: $(wildcard tx/title-$(1)-$(2).txt)
	@if test -r tx/title-$(1)-$(2).txt ; then \
	   $(mkdir_p) $(STORY_TITLES_DIR)/$(2); \
	   $(AWK) "/^text[[:space:]]*=[[:space:]]*[\[]+/ { print; nextfile; } { print; } END { print \"]]\"; print \"}\"; }" \
	        $(STORY_TITLES_DIR)/$(1).lua $$< > $$<.tmp; \
	   $(call copy_on_diff,$$<.tmp,$$@); \
	 fi
endef

$(foreach lang,$(LANGS),\
    $(foreach rsrc,$(STORY_TITLES_RSRC),\
        $(eval $(call update-gmo-story-title,$(rsrc),$(lang)))))

#-- update-po creates the new tx/title-<title name>.txt files

update-po:: $(STORY_TITLES_TX)

#-- Push all title rsrcs to TX

tx-update:: $(STORY_TITLES_TX)
	@if [ " $(TXCREDS)" = " " ] ; then \
		echo "You do not have a Transifex account"; \
		exit 1; \
	 fi
	@for rsrc in $(STORY_TITLES_RSRC); do \
	   $(call rsrc_update,title-$$rsrc,tx/title-$$rsrc.txt); \
	 done

#-- Pull all l10n versions of the titles' text from TX

tx-pull:: $(STORY_TITLES_TX)
	@if [ " $(TXCREDS)" = " " ] ; then \
		echo "You do not have a Transifex account"; \
		exit 1; \
	 fi
	@for lang in $(LANGS); do \
	   for rsrc in $(STORY_TITLES_RSRC); do \
	     $(call rsrc_pull,title-$$rsrc,$$lang); \
	   done; \
	 done

#-- update-gmo creates the new l10n version of the titles

update-gmo:: $(STORY_TITLES_L10N)

#== l10n of the main README

README_DIR  = $(top_srcdir)
README_RSRC = README
README_TX   = tx/README.txt
README_L10N = $(foreach lang,$(LANGS),$(README_RSRC:%=$(README_DIR)/README-$(lang)))

#-- Copy the README into the tx/ subdir

tx/README.txt: $(README_DIR)/README
	@$(mkdir_p) tx; \
	 cp $< $@


#-- Put localized versions in place
# $(1) = lang

define update-gmo-readme =
 $(README_DIR)/README-$(1): $(wildcard tx/README-$(1).txt)
	@if test -r tx/README-$(1).txt ; then \
	   cp $$< $$<.tmp; \
	   $(call copy_on_diff,$$<.tmp,$$@); \
	 fi
endef

$(foreach lang,$(LANGS),\
    $(eval $(call update-gmo-readme,$(lang))))

#-- update-po creates the new tx/README.txt files

update-po:: $(README_TX)

#-- Push the README rsrc to TX

tx-update:: $(README_TX)
	@if [ " $(TXCREDS)" = " " ] ; then \
		echo "You do not have a Transifex account"; \
		exit 1; \
	 fi
	@$(call rsrc_update,README,tx/README.txt)

#-- Pull all l10n versions of the README from TX

tx-pull:: $(README_TX)
	@if [ " $(TXCREDS)" = " " ] ; then \
		echo "You do not have a Transifex account"; \
		exit 1; \
	 fi
	@for lang in $(LANGS); do \
	   $(call rsrc_pull,README,$$lang); \
	 done

#-- update-gmo creates the new l10n version of the README

update-gmo:: $(README_L10N)

