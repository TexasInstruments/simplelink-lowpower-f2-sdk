SDK_INSTALL_DIR = $(abspath ../../..)
include $(SDK_INSTALL_DIR)/imports.mak

# Project name and device family
DEVICE_FAMILY = cc26x4
PROJECT_NAME = production_full

PROJECT_PATH = $(SDK_INSTALL_DIR)/tfm_s/$(DEVICE_FAMILY)/$(PROJECT_NAME)

# Use mbedTLS conan package
MBEDCRYPTO_PATH = $(MBEDTLS_INSTALL_DIR)/source/third_party/mbedtls

# Add extra manifest list files and partition paths
TFM_EXTRA_OPTIONS = \
-DTFM_EXTRA_MANIFEST_LIST_FILES=$(PROJECT_PATH)/tfm_manifest_list.yaml \
-DTFM_EXTRA_PARTITION_PATHS=$(PROJECT_PATH)/

# Override default options with custom options and pass additional options
CONFIGURABLE_OPTIONS = \
-DTFM_MBEDCRYPTO_CONFIG_PATH=$(MBEDCRYPTO_PATH)/ti/configs/config-tidrivers.h \
-DTFM_MBEDCRYPTO_PSA_CRYPTO_CONFIG_PATH=$(MBEDCRYPTO_PATH)/ti/configs/config-psa-crypto-cc13x4.h \
-DPROJECT_CONFIG_HEADER_FILE=$(PROJECT_PATH)/config/config_profile.h \
-DTFM_PARTITION_INITIAL_ATTESTATION=False # BQ - change to True once attestation is completed

all: generate configure build

# Include the common makefile, which defines the build targets
include $(SDK_INSTALL_DIR)/tfm_s/util/makefile_common.mak
