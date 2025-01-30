BUILD_DIR := $(SDK_INSTALL_DIR)/tfm_s/build/$(DEVICE_FAMILY)/$(PROJECT_NAME)
REMOTE_GEN_DIR := $(BUILD_DIR)/generated
GENERATED_FLASH_LAYOUT_DIR := $(REMOTE_GEN_DIR)/platform/ext/target/ti/$(DEVICE_FAMILY)/partition
CMAKE_INSTALL_PREFIX := $(BUILD_DIR)/Release
MBEDCRYPTO_PATH ?= $(SDK_INSTALL_DIR)/source/third_party/mbedtls

# Define component paths for SDK builds which do not use conan packages
DRIVERLIB_CC13XX_CC26XX_INSTALL_DIR ?= $(SDK_INSTALL_DIR)
TIDRIVERS_CC13XX_CC26XX_INSTALL_DIR ?= $(SDK_INSTALL_DIR)
DRIVERLIB_LOKI_INSTALL_DIR ?= $(SDK_INSTALL_DIR)
TIDRIVERS_LPF3_INSTALL_DIR ?= $(SDK_INSTALL_DIR)
TIUTILS_INSTALL_DIR ?= $(SDK_INSTALL_DIR)

ifeq ($(DEVICE_FAMILY), cc26x4)
	DRIVERLIB_SOURCE_DIR := $(DRIVERLIB_CC13XX_CC26XX_INSTALL_DIR)/source
	TIDRIVERS_INSTALL_DIR := $(TIDRIVERS_CC13XX_CC26XX_INSTALL_DIR)
endif

ifeq ($(DEVICE_FAMILY), cc27xx)
	DRIVERLIB_SOURCE_DIR := $(DRIVERLIB_LOKI_INSTALL_DIR)/source
	TIDRIVERS_INSTALL_DIR := $(TIDRIVERS_LPF3_INSTALL_DIR)
endif

NS_BUILD_DIR := $(SDK_INSTALL_DIR)/tests/$(PROJECT_NAME)/build/$(DEVICE_FAMILY)
TFM_TEST_REPO_VERSION := $(shell grep -Po "version=\K[^ ]+" $(SDK_INSTALL_DIR)/source/third_party/tfm/lib/ext/tf-m-tests/version.txt)

# ANSI escape codes for color output
COLOR_BLUE := \033[1;34m
COLOR_RESET := \033[0m

# TFM root is fixed by submodule or SDK layout
TFM_ROOT := $(SDK_INSTALL_DIR)/source/third_party/tfm

# Add GCC_ARMCOMPILER to path
# GCC_ARMCOMPILER must be set in imports.mak
PATH := $(GCC_ARMCOMPILER)/bin:$(PATH)

# CMAKE_BUILD_TYPE is normally 'Release' which will enable -03 optimization.
# Change CMAKE_BUILD_TYPE to 'Debug' for testing which will disable optimization.
DEFAULT_OPTIONS ?= \
-DTFM_PROFILE=profile_medium \
-DTFM_ISOLATION_LEVEL=1 \
-DCMAKE_BUILD_TYPE=Release \
-DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=False \
-DTFM_PARTITION_INITIAL_ATTESTATION=False \
-DTFM_PARTITION_PROTECTED_STORAGE=False \
-DTFM_PARTITION_PLATFORM=False

# Specify mbedCrypto path and SysConfig tool for projects which are not named "regress"
DEFAULT_OPTIONS += $(if $(filter-out "regress","$(PROJECT_NAME)"),-DMBEDCRYPTO_PATH=$(MBEDCRYPTO_PATH) -DSYSCONFIG_TOOL=$(SYSCONFIG_TOOL))

clean:
	@ echo "$(COLOR_BLUE)Cleaning $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	@ $(RMDIR) $(BUILD_DIR)
	@ $(RMDIR) $(NS_BUILD_DIR)

check_environment:
ifndef DRIVERLIB_SOURCE_DIR
	@ $(error $(DEVICE_FAMILY) device family is not supported)
endif

generate: check_environment
	@ echo "$(COLOR_BLUE)Generating $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	@ mkdir -p $(GENERATED_FLASH_LAYOUT_DIR)
	@ $(PYTHON) $(TFM_ROOT)/tools/tfm_gen.py \
		memory_layout.yaml $(TFM_ROOT) \
		platform/ext/target/ti/$(DEVICE_FAMILY)/partition/flash_layout.h \
		$(GENERATED_FLASH_LAYOUT_DIR)/flash_layout.h

configure: generate
	@ echo "$(COLOR_BLUE)Configuring $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	cd $(TFM_ROOT) && $(CMAKE) -S . -B $(BUILD_DIR) \
		-G"Unix Makefiles" \
		-DTFM_TOOLCHAIN_FILE=$(TFM_ROOT)/toolchain_GNUARM.cmake \
		-DDRIVERLIB_SOURCE_DIR=$(DRIVERLIB_SOURCE_DIR) \
		-DTIDRIVERS_INSTALL_DIR=$(TIDRIVERS_INSTALL_DIR) \
		-DTIUTILS_INSTALL_DIR=$(TIUTILS_INSTALL_DIR) \
		-DGENERATED_FLASH_LAYOUT_DIR=$(GENERATED_FLASH_LAYOUT_DIR) \
		-DTFM_PLATFORM=ti/$(DEVICE_FAMILY) \
		-DCMAKE_INSTALL_PREFIX=$(CMAKE_INSTALL_PREFIX) \
		$(TFM_EXTRA_OPTIONS) \
		$(DEFAULT_OPTIONS) \
		$(CONFIGURABLE_OPTIONS)

build: configure
	@ echo "$(COLOR_BLUE)Building $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	@ cd $(TFM_ROOT) && $(CMAKE) --build $(BUILD_DIR) -- install

clone_regress:
	@ echo "$(COLOR_BLUE)Cloning TF-M tests repo for $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	@ $(RMDIR) $(SDK_INSTALL_DIR)/tf-m-tests
	@ cd $(SDK_INSTALL_DIR) && git clone https://git.trustedfirmware.org/TF-M/tf-m-tests.git
	cd $(SDK_INSTALL_DIR)/tf-m-tests && git checkout $(TFM_TEST_REPO_VERSION)

configure_regress: generate clone_regress
	@ echo "$(COLOR_BLUE)Configuring $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	cd $(SDK_INSTALL_DIR)/tf-m-tests/tests_reg && $(CMAKE) -S spe -B $(BUILD_DIR) \
		-G"Unix Makefiles" \
		-DCMAKE_BUILD_TYPE=Debug \
		-DTFM_TOOLCHAIN_FILE=$(TFM_ROOT)/toolchain_GNUARM.cmake \
		-DDRIVERLIB_SOURCE_DIR=$(DRIVERLIB_SOURCE_DIR) \
		-DTIDRIVERS_INSTALL_DIR=$(TIDRIVERS_INSTALL_DIR) \
		-DGENERATED_FLASH_LAYOUT_DIR=$(GENERATED_FLASH_LAYOUT_DIR) \
		-DTFM_PLATFORM=ti/$(DEVICE_FAMILY) \
		-DCONFIG_TFM_SOURCE_PATH=$(TFM_ROOT) \
		$(DEFAULT_OPTIONS) \
		$(CONFIGURABLE_OPTIONS)

build_regress: configure_regress
	@ echo "$(COLOR_BLUE)Building $(DEVICE_FAMILY) $(PROJECT_NAME) project...$(COLOR_RESET)"
	@ cd $(SDK_INSTALL_DIR)/tf-m-tests/tests_reg && $(CMAKE) --build $(BUILD_DIR) -- install

configure_regress_ns:
	@ echo "$(COLOR_BLUE)Configuring $(DEVICE_FAMILY) $(PROJECT_NAME) Non-secure project...$(COLOR_RESET)"
	cd $(SDK_INSTALL_DIR)/tf-m-tests/tests_reg && $(CMAKE) -S . -B $(NS_BUILD_DIR) \
		-DDRIVERLIB_SOURCE_DIR=$(DRIVERLIB_SOURCE_DIR) \
		-DCONFIG_SPE_PATH=$(BUILD_DIR)/api_ns

build_regress_ns: configure_regress_ns
	@ echo "$(COLOR_BLUE)Building $(DEVICE_FAMILY) $(PROJECT_NAME) Non-secure project...$(COLOR_RESET)"
	@ cd $(SDK_INSTALL_DIR)/tf-m-tests/tests_reg && $(CMAKE) --build $(BUILD_DIR) -- install && $(CMAKE) --build $(NS_BUILD_DIR)

TRIM_DIRS = \
	$(BUILD_DIR)/bin \
	$(BUILD_DIR)/CMakeFiles \
	$(BUILD_DIR)/generated/interface \
	$(BUILD_DIR)/generated/platform \
	$(BUILD_DIR)/generated/secure_fw \
	$(BUILD_DIR)/generated/tools \
	$(BUILD_DIR)/interface \
	$(BUILD_DIR)/lib \
	$(BUILD_DIR)/platform \
	$(BUILD_DIR)/secure_fw \
	$(BUILD_DIR)/tools \
	$(CMAKE_INSTALL_PREFIX)/cmake \
	$(CMAKE_INSTALL_PREFIX)/config \
	$(CMAKE_INSTALL_PREFIX)/interface \
	$(CMAKE_INSTALL_PREFIX)/platform

TRIM_FILES = \
	$(BUILD_DIR)/*.txt \
	$(BUILD_DIR)/*.cmake \
	$(BUILD_DIR)/Makefile \
	$(CMAKE_INSTALL_PREFIX)/*.txt

trim:
	@ echo "$(COLOR_BLUE)Trimming content in $(BUILD_DIR) for SDK...$(COLOR_RESET)"
	@ $(RMDIR) $(TRIM_DIRS)
	@ $(RM) $(TRIM_FILES)

# These rules do not correspond to a specific file
.PHONY: generate configure configure_regress configure_regress_ns build build_regress build_regress_ns clean
