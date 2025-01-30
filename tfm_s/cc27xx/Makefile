# ======== Makefile ========
SDK_INSTALL_DIR = $(abspath ../..)
include $(SDK_INSTALL_DIR)/imports.mak

# Shortcut makefile for rebuilding the production_full image that
# ships with the SDK. Builds both images and then trims generated content
# (for example CMakefiles) to simplify the results.

# Note that CMAKE must be configured to CMAKE's bin/ folder in the SDK's
# imports.mak to run these goals.

all:
	@ $(MAKE) -C production_full all
	@ $(MAKE) -C production_full trim

clean:
	@ $(MAKE) -C production_full clean
