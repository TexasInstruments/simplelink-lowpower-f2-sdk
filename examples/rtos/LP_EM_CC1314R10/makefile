COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../..)
include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

# Find all subdirectories
TIRTOS7-IAR-SUBDIRS = $(wildcard */*/tirtos7/iar/.)
TIRTOS7-TICLANG-SUBDIRS = $(wildcard */*/tirtos7/ticlang/.)
FREERTOS-IAR-SUBDIRS = $(wildcard */*/freertos/iar/.)
FREERTOS-TICLANG-SUBDIRS = $(wildcard */*/freertos/ticlang/.)

TIRTOS7-SUBDIRS =
FREERTOS-SUBDIRS =
ifneq ($(IAR_ARMCOMPILER),)
	TIRTOS7-SUBDIRS += $(TIRTOS7-IAR-SUBDIRS)
	FREERTOS-SUBDIRS += $(FREERTOS-IAR-SUBDIRS)
endif
ifneq ($(TICLANG_ARMCOMPILER),)
	TIRTOS7-SUBDIRS += $(TIRTOS7-TICLANG-SUBDIRS)
	FREERTOS-SUBDIRS += $(FREERTOS-TICLANG-SUBDIRS)
endif

.PHONY: tirtos7 freertos $(TIRTOS7-SUBDIRS) $(FREERTOS-SUBDIRS)

all: tirtos7 freertos

tirtos7: $(TIRTOS7-SUBDIRS)

freertos: $(FREERTOS-SUBDIRS)

$(TIRTOS7-SUBDIRS):
	@ $(MAKE) -C $@

$(FREERTOS-SUBDIRS):
	@ $(MAKE) -C $@

CLEAN-TIRTOS7-SUBDIRS = $(addprefix clean-,$(TIRTOS7-SUBDIRS))
CLEAN-FREERTOS-SUBDIRS = $(addprefix clean-,$(FREERTOS-SUBDIRS))

.PHONY: clean-tirtos7 clean-freertos $(CLEAN-TIRTOS7-SUBDIRS) $(CLEAN-FREERTOS-SUBDIRS)

clean: clean-tirtos7 clean-freertos

clean-tirtos7: $(CLEAN-TIRTOS7-SUBDIRS)
clean-freertos: $(CLEAN-FREERTOS-SUBDIRS)

$(CLEAN-TIRTOS7-SUBDIRS):
	@ $(MAKE) -C $(subst clean-,,$@) clean

$(CLEAN-FREERTOS-SUBDIRS):
	@ $(MAKE) -C $(subst clean-,,$@) clean
