#
#  ======== makefile ========
#

SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

BIOS_INSTALL_DIR ?= $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)
TIPOSIX_INSTALL_DIR ?= $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)
DRIVERLIB_CC13XX_CC26XX_INSTALL_DIR ?= $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)

XDCPATH = $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source;$(BIOS_INSTALL_DIR)/kernel/tirtos/packages;$(TIPOSIX_INSTALL_DIR)/source;$(DRIVERLIB_CC13XX_CC26XX_INSTALL_DIR)/source

#
# TI-RTOS application configuration file
#
CFG_FILE   ?= ../debug.cfg

#
# Platform to build
#
PLATFORM   ?= ti.platforms.simplelink:CC13X2_CC26X2

#
# Compiler options for the build
#
CFLAGS = -DDeviceFamily_CC13X2X7_CC26X2X7

#
# Set XDCOPTIONS.  Use -v for a verbose build.
#
#XDCOPTIONS=v
export XDCOPTIONS

#######################################################
## Shouldn't have to modify anything below this line ##
#######################################################
XS = $(XDC_INSTALL_DIR)/xs
XDC = $(XDC_INSTALL_DIR)/xdc

all:
	@ $(XS) --xdcpath="$(XDCPATH)" xdc.tools.configuro -o ../ticlang \
            -t ti.targets.arm.clang.M4F -p $(PLATFORM) -c "$(TICLANG_ARMCOMPILER)" \
            --compileOptions "$(CFLAGS)" $(CFG_FILE)

clean:
	@ echo cleaning ...
ifeq ($(wildcard package.xdc), package.xdc)
	@ $(XDC) clean > $(DEVNULL) 2>&1
endif
	@ $(RM) compiler.opt* > $(DEVNULL) 2>&1
	@ $(RM) config.bld > $(DEVNULL) 2>&1
	@ $(RM) custom.mak > $(DEVNULL) 2>&1
	@ $(RM) package.* > $(DEVNULL) 2>&1
	@ $(RM) linker.cmd > $(DEVNULL) 2>&1
	@ $(RM) .xdcenv.mak > $(DEVNULL) 2>&1