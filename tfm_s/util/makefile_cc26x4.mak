# Should be the same as the current directory
PROJECT_ROOT = $(SDK_INSTALL_DIR)/tfm_s/$(DEVICE_FAMILY)/$(PROJECT_NAME)
BUILD_DIR = $(SDK_INSTALL_DIR)/tfm_s/build/$(DEVICE_FAMILY)/$(PROJECT_NAME)
MBEDCRYPTO_INSTALL_DIR ?= $(SDK_INSTALL_DIR)

# Fixed, by submodule or SDK layout
TFM_ROOT = $(SDK_INSTALL_DIR)/source/third_party/tfm

# Add GCC_ARMCOMPILER to path
# GCC_ARMCOMPILER must be set in imports.mak
PATH := $(GCC_ARMCOMPILER)/bin:$(PATH)

DEFAULT_OPTIONS ?= \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE=False \
-DTFM_PARTITION_INITIAL_ATTESTATION=False \
-DTFM_PARTITION_PROTECTED_STORAGE=False \
-DTFM_PARTITION_CRYPTO=False \
-DTFM_PARTITION_PLATFORM=False

clean:
	@ echo Cleaning $(DEVICE_FAMILY)/$(PROJECT_NAME)
	@ $(RMDIR) $(PROJECT_ROOT)/Release
	@ $(RMDIR) $(BUILD_DIR)

generate:
	@ mkdir -p $(BUILD_DIR)/secure_fw/services

	@ echo Generating secure project for $(DEVICE_FAMILY)/$(PROJECT_NAME)
	@ $(PYTHON) $(TFM_ROOT)/tools/tfm_gen.py \
		memory_layout.yaml $(TFM_ROOT) \
		platform/ext/target/ti/cc26x4/partition/flash_layout.h \
		$(BUILD_DIR)/platform/ext/target/ti/cc26x4/partition/flash_layout.h

	@ echo Parsing manifest
	@ cd $(TFM_ROOT); \
		$(PYTHON) tools/tfm_parse_manifest_list.py \
		-m $(PROJECT_ROOT)/tfm_manifest_list.yaml \
		-f $(SDK_INSTALL_DIR)/tfm_s/util/tfm_generated_file_list.yaml \
		-o $(BUILD_DIR)

configure: generate
	@ echo Configuring secure project for $(DEVICE_FAMILY)/$(PROJECT_NAME)
	@ cd $(BUILD_DIR); \
		$(CMAKE) -G"Unix Makefiles" \
		-DPROJ_CONFIG=$(TFM_ROOT)/configs/ConfigCoreIPC.cmake \
		-DSYSCONFIG_TOOL=$(SYSCONFIG_TOOL) \
		-DSOURCE_REPO=$(SDK_INSTALL_DIR)/source \
		-DKERNEL_REPO=$(SDK_INSTALL_DIR)/kernel \
		-DMBEDCRYPTO_INSTALL_DIR=$(MBEDCRYPTO_INSTALL_DIR) \
		-DREMOTE_GEN_DIR=$(BUILD_DIR) \
		-DCMAKE_INSTALL_PREFIX=$(BUILD_DIR)/Release \
		$(CONFIGURABLE_OPTIONS) \
		$(DEFAULT_OPTIONS) \
		-DTARGET_PLATFORM=CC26X4 \
		-DCOMPILER=GNUARM \
		-DBL2=False \
		$(PROJECT_ROOT)

build: configure
	@ echo Building $(DEVICE_FAMILY)/$(PROJECT_NAME)
	@ cd $(BUILD_DIR); $(CMAKE) --build ./ -- install
	@ cd $(BUILD_DIR)/Release/outputs/CC26X4; arm-none-eabi-objcopy --output-target=ihex tfm_s.axf tfm_s.hex

deploy: build
ifeq ("$(SHELL)","sh.exe")
	@ copy $(SDK_INSTALL_DIR)\source\ti\tfm\syscfg_c.rov.xs $(subst /,\,$(BUILD_DIR)/Release/outputs/CC26X4)
	@ copy $(BUILD_DIR)\tfm_s.map $(subst /,\,$(BUILD_DIR)/Release/outputs/CC26X4)
	@ $(RM) $(BUILD_DIR)/tfm_s*
	@ $(RM) $(BUILD_DIR)/s_veneers.o
	@ $(RMDIR) $(BUILD_DIR)\Release\outputs\fvp
	@ $(RMDIR) $(BUILD_DIR)\Release\export\tfm\src
	@ $(RMDIR) $(BUILD_DIR)\Release\export\tfm\inc
	@ copy $(BUILD_DIR)\platform\ext\target\ti\cc26x4\partition\flash_layout.h $(BUILD_DIR)\Release\export\tfm\inc
	@ mkdir $(BUILD_DIR)\Release\export\tfm\inc\psa_manifest
	@ copy $(BUILD_DIR)\interface\include\psa_manifest\sid.h $(BUILD_DIR)\Release\export\tfm\inc\psa_manifest
	@ copy $(BUILD_DIR)\interface\include\psa_manifest\pid.h $(BUILD_DIR)\Release\export\tfm\inc\psa_manifest
ifeq ("$(PROJECT_NAME)", "production_full")
	@ copy $(BUILD_DIR)\..\..\..\cc26x4\production_full\README $(BUILD_DIR)\Release
endif
else
	@ cp $(SDK_INSTALL_DIR)/source/ti/tfm/syscfg_c.rov.xs $(BUILD_DIR)/Release/outputs/CC26X4
	@ cp $(BUILD_DIR)/tfm_s.map $(BUILD_DIR)/Release/outputs/CC26X4
	@ $(RM) $(BUILD_DIR)/tfm_s*
	@ $(RM) $(BUILD_DIR)/s_veneers.o
	@ $(RMDIR) $(BUILD_DIR)/Release/outputs/fvp
	@ $(RMDIR) $(BUILD_DIR)/Release/export/tfm/src
	@ $(RMDIR) $(BUILD_DIR)/Release/export/tfm/inc/*
	@ cp $(BUILD_DIR)/platform/ext/target/ti/cc26x4/partition/flash_layout.h $(BUILD_DIR)/Release/export/tfm/inc
	@ mkdir $(BUILD_DIR)/Release/export/tfm/inc/psa_manifest
	@ cp $(BUILD_DIR)/interface/include/psa_manifest/sid.h $(BUILD_DIR)/Release/export/tfm/inc/psa_manifest
	@ cp $(BUILD_DIR)/interface/include/psa_manifest/pid.h $(BUILD_DIR)/Release/export/tfm/inc/psa_manifest
ifeq ("$(PROJECT_NAME)", "production_full")
	@ cp $(BUILD_DIR)/../../../cc26x4/production_full/README $(BUILD_DIR)/Release
endif
endif

cleanup:
	@ echo Trimming $(DEVICE_FAMILY)/$(PROJECT_NAME)
ifeq ("$(SHELL)","sh.exe")
	@ $(RMDIR) $(BUILD_DIR)\Release\outputs\fvp
	@ $(RMDIR) $(BUILD_DIR)\CMakeFiles
	@ $(RMDIR) $(BUILD_DIR)\platform
	@ $(RMDIR) $(BUILD_DIR)\*_sp
	@ $(RMDIR) $(BUILD_DIR)\secure_fw
	@ $(RMDIR) $(BUILD_DIR)\mbed-crypto
	@ $(RMDIR) $(BUILD_DIR)\mbedcrypto_lib-prefix
	@ $(RM) $(BUILD_DIR)\Makefile
	@ $(RM) $(BUILD_DIR)\tfm_s.ld.i
	@ $(RM) $(BUILD_DIR)\*.txt
	@ $(RM) $(BUILD_DIR)\*.cmake
	@ $(RM) $(BUILD_DIR)\Makefile
else
	@ $(RMDIR) $(BUILD_DIR)/Release/outputs/fvp
	@ $(RMDIR) $(BUILD_DIR)/CMakeFiles
	@ $(RMDIR) $(BUILD_DIR)/platform
	@ $(RMDIR) $(BUILD_DIR)/*_sp
	@ $(RMDIR) $(BUILD_DIR)/secure_fw
	@ $(RMDIR) $(BUILD_DIR)/mbed-crypto
	@ $(RMDIR) $(BUILD_DIR)/mbedcrypto_lib-prefix
	@ $(RM) $(BUILD_DIR)/Makefile
	@ $(RM) $(BUILD_DIR)/tfm_s.ld.i
	@ $(RM) $(BUILD_DIR)/*.txt
	@ $(RM) $(BUILD_DIR)/*.cmake
	@ $(RM) $(BUILD_DIR)/Makefile
endif
