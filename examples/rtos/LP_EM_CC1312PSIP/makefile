COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../..)
include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

# Find all subdirectories
TIRTOS7-IAR-SUBDIRS = $(wildcard */*/tirtos7/iar/.)
TIRTOS7-TICLANG-SUBDIRS = $(wildcard */*/tirtos7/ticlang/.)

TIRTOS7-SUBDIRS =
ifneq ($(IAR_ARMCOMPILER),)
	TIRTOS7-SUBDIRS += $(TIRTOS7-IAR-SUBDIRS)
endif
ifneq ($(TICLANG_ARMCOMPILER),)
	TIRTOS7-SUBDIRS += $(TIRTOS7-TICLANG-SUBDIRS)
endif

.PHONY: tirtos7 $(TIRTOS7-SUBDIRS)

all: tirtos7

tirtos7: $(TIRTOS7-SUBDIRS)

$(TIRTOS7-SUBDIRS):
	@ $(MAKE) -C $@

CLEAN-TIRTOS7-SUBDIRS = $(addprefix clean-,$(TIRTOS7-SUBDIRS))

.PHONY: clean-tirtos7 $(CLEAN-TIRTOS7-SUBDIRS)

clean: clean-tirtos7

clean-tirtos7: $(CLEAN-TIRTOS7-SUBDIRS)

$(CLEAN-TIRTOS7-SUBDIRS):
	@ $(MAKE) -C $(subst clean-,,$@) clean
