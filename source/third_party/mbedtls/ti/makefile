# This makefile builds all TI libraries it finds.  To build a specific
# library, run the appropriate lib/<toolchain>/<isa>/makefile

.PHONY: all $(SUBDIRS) clean $(SUBCLEAN)

# Locate all subdirectories to build
SUBDIRS := $(wildcard lib/*/*)

# phony '<subdir>.clean' build goals (used during clean)
SUBCLEAN := $(addsuffix .clean,$(SUBDIRS))

all: $(addsuffix /mbedtls.a,$(SUBDIRS))

%/mbedtls.a: %/makefile
	$(MAKE) -C $*

clean: $(SUBCLEAN)

$(SUBCLEAN): %.clean:
	$(MAKE) -C $* clean
