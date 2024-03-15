# Should be the same as the current directory
BUILD_DIR = $(SDK_INSTALL_DIR)/tfm_s/build/$(DEVICE_FAMILY)/$(PROJECT_NAME)
REMOTE_GEN_DIR = $(BUILD_DIR)/generated
GENERATED_FLASH_LAYOUT_DIR = $(REMOTE_GEN_DIR)/platform/ext/target/ti/$(DEVICE_FAMILY)/partition
CMAKE_INSTALL_PREFIX = $(BUILD_DIR)/Release
MBEDCRYPTO_PATH ?= $(SDK_INSTALL_DIR)/source/third_party/mbedtls

# ANSI escape codes for color output
COLOR_BLUE = \033[1;34m
COLOR_RESET = \033[0m

# TFM root is fixed by submodule or SDK layout
TFM_ROOT = $(SDK_INSTALL_DIR)/source/third_party/tfm

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

# Use our own version of mbedCrypto for projects which are not named "regress"
DEFAULT_OPTIONS += $(if $(filter-out "regress","$(PROJECT_NAME)"),-DMBEDCRYPTO_PATH=$(MBEDCRYPTO_PATH))

clean:
	@ echo "$(COLOR_BLUE)Cleaning $(DEVICE_FAMILY)/$(PROJECT_NAME)...$(COLOR_RESET)"
	@ $(RMDIR) $(BUILD_DIR)

generate:
	@ echo "$(COLOR_BLUE)Generating TI TF-M project for $(DEVICE_FAMILY)/$(PROJECT_NAME)...$(COLOR_RESET)"
	@ mkdir -p $(GENERATED_FLASH_LAYOUT_DIR)
	@ $(PYTHON) $(TFM_ROOT)/tools/tfm_gen.py \
		memory_layout.yaml $(TFM_ROOT) \
		platform/ext/target/ti/$(DEVICE_FAMILY)/partition/flash_layout.h \
		$(GENERATED_FLASH_LAYOUT_DIR)/flash_layout.h

configure: generate
	@ echo "$(COLOR_BLUE)Configuring TI TF-M project for $(DEVICE_FAMILY)/$(PROJECT_NAME)...$(COLOR_RESET)"
	cd $(TFM_ROOT) && $(CMAKE) -S . -B $(BUILD_DIR) \
		-G"Unix Makefiles" \
		-DTFM_TOOLCHAIN_FILE=$(TFM_ROOT)/toolchain_GNUARM.cmake \
		-DSYSCONFIG_TOOL=$(SYSCONFIG_TOOL) \
		-DSOURCE_REPO=$(SDK_INSTALL_DIR)/source \
		-DKERNEL_REPO=$(SDK_INSTALL_DIR)/kernel \
		-DGENERATED_FLASH_LAYOUT_DIR=$(GENERATED_FLASH_LAYOUT_DIR) \
		-DTFM_PLATFORM=ti/$(DEVICE_FAMILY) \
		-DCMAKE_INSTALL_PREFIX=$(CMAKE_INSTALL_PREFIX) \
		-DREMOTE_GEN_DIR=$(REMOTE_GEN_DIR) \
		$(TFM_EXTRA_OPTIONS) \
		$(DEFAULT_OPTIONS) \
		$(CONFIGURABLE_OPTIONS)

build: configure
	@ echo "$(COLOR_BLUE)Building TI TF-M project for $(DEVICE_FAMILY)/$(PROJECT_NAME)...$(COLOR_RESET)"
	@ cd $(TFM_ROOT) && $(CMAKE) --build $(BUILD_DIR) -- install

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
	$(CMAKE_INSTALL_PREFIX)/interface

TRIM_FILES = \
	$(BUILD_DIR)/*.txt \
	$(BUILD_DIR)/*.cmake \
	$(BUILD_DIR)/Makefile

trim:
	@ echo "$(COLOR_BLUE)Trimming content in $(BUILD_DIR) for SDK...$(COLOR_RESET)"
	@ $(RMDIR) $(TRIM_DIRS)
	@ $(RM) $(TRIM_FILES)
