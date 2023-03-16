SDK_INSTALL_DIR = $(abspath ../../..)
include $(SDK_INSTALL_DIR)/imports.mak

# Project name and device family
DEVICE_FAMILY = cc26x4
PROJECT_NAME = production_full

# Override default options with custom options
DEFAULT_OPTIONS = \
-DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=True \
-DTFM_PARTITION_INITIAL_ATTESTATION=True \
-DTFM_PARTITION_PROTECTED_STORAGE=False \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DTFM_PARTITION_CRYPTO=False \
-DTFM_PARTITION_PLATFORM=False

all: generate configure build deploy

# Include the common makefile, which defines the build targets
include $(SDK_INSTALL_DIR)/tfm_s/util/makefile_$(DEVICE_FAMILY).mak
