#-------------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
# Copyright (c) 2021-2022 Cypress Semiconductor Corporation (an Infineon company)
# or an affiliate of Cypress Semiconductor Corporation. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set (VALID_ISOLATION_LEVELS 1 2 3)

tfm_invalid_config(NOT TFM_ISOLATION_LEVEL IN_LIST VALID_ISOLATION_LEVELS)
tfm_invalid_config(TFM_ISOLATION_LEVEL EQUAL 3 AND NOT PLATFORM_HAS_ISOLATION_L3_SUPPORT)
tfm_invalid_config(TFM_ISOLATION_LEVEL GREATER 1 AND PSA_FRAMEWORK_HAS_MM_IOVEC)

tfm_invalid_config(TFM_MULTI_CORE_TOPOLOGY AND TFM_NS_MANAGE_NSID)
tfm_invalid_config(TFM_PLAT_SPECIFIC_MULTI_CORE_COMM AND NOT TFM_MULTI_CORE_TOPOLOGY)
tfm_invalid_config(TFM_ISOLATION_LEVEL EQUAL 3 AND CONFIG_TFM_STACK_WATERMARKS)

########################## BL2 #################################################

get_property(MCUBOOT_STRATEGY_LIST CACHE MCUBOOT_UPGRADE_STRATEGY PROPERTY STRINGS)
tfm_invalid_config(BL2 AND (NOT MCUBOOT_UPGRADE_STRATEGY IN_LIST MCUBOOT_STRATEGY_LIST) AND NOT USE_KCONFIG_TOOL)

# Maximum number of MCUBoot images supported by TF-M NV counters and ROTPKs
tfm_invalid_config(MCUBOOT_IMAGE_NUMBER GREATER 9)

tfm_invalid_config(MCUBOOT_SIGNATURE_TYPE STREQUAL "EC-P256" AND NOT MCUBOOT_USE_PSA_CRYPTO)
tfm_invalid_config(MCUBOOT_SIGNATURE_TYPE STREQUAL "EC-P384" AND NOT MCUBOOT_USE_PSA_CRYPTO)

tfm_invalid_config((BL2 AND CONFIG_TFM_BOOT_STORE_MEASUREMENTS AND NOT CONFIG_TFM_BOOT_STORE_ENCODED_MEASUREMENTS) AND NOT MCUBOOT_DATA_SHARING)
tfm_invalid_config((NOT (TFM_PARTITION_FIRMWARE_UPDATE OR CONFIG_TFM_BOOT_STORE_MEASUREMENTS)) AND MCUBOOT_DATA_SHARING)

get_property(MCUBOOT_ALIGN_VAL_LIST CACHE MCUBOOT_ALIGN_VAL PROPERTY STRINGS)
tfm_invalid_config(BL2 AND (NOT MCUBOOT_ALIGN_VAL IN_LIST MCUBOOT_ALIGN_VAL_LIST) AND NOT USE_KCONFIG_TOOL)

tfm_invalid_config(TFM_DUMMY_PROVISIONING AND MCUBOOT_GENERATE_SIGNING_KEYPAIR)

tfm_invalid_config((NOT MCUBOOT_HW_KEY) AND (MCUBOOT_GENERATE_SIGNING_KEYPAIR))

####################### Code sharing ###########################################

set(TFM_CODE_SHARING_PLATFORM_LISTS arm/mps2/an521 arm/musca_b1) # Without crypto hw acceleration
tfm_invalid_config(NOT TFM_CODE_SHARING STREQUAL "OFF" AND NOT TFM_PLATFORM IN_LIST TFM_CODE_SHARING_PLATFORM_LISTS)
tfm_invalid_config(NOT TFM_CODE_SHARING STREQUAL "OFF" AND CRYPTO_HW_ACCELERATOR)
tfm_invalid_config(NOT TFM_CODE_SHARING STREQUAL "OFF" AND NOT C_COMPILER_ID:IAR)

########################## Platform ############################################

tfm_invalid_config(OTP_NV_COUNTERS_RAM_EMULATION AND NOT (PLATFORM_DEFAULT_OTP OR PLATFORM_DEFAULT_NV_COUNTERS))
tfm_invalid_config(PLATFORM_DEFAULT_NV_COUNTERS AND  NOT PLATFORM_DEFAULT_OTP_WRITEABLE)
tfm_invalid_config(TFM_DUMMY_PROVISIONING AND (PLATFORM_DEFAULT_OTP AND NOT PLATFORM_DEFAULT_OTP_WRITEABLE))
tfm_invalid_config(TFM_NS_NV_COUNTER_AMOUNT GREATER 3)

####################### Firmware Update Partition ###############################

tfm_invalid_config(NOT PLATFORM_HAS_FIRMWARE_UPDATE_SUPPORT AND TFM_PARTITION_FIRMWARE_UPDATE)
tfm_invalid_config(TFM_PARTITION_FIRMWARE_UPDATE AND NOT TFM_PARTITION_PLATFORM)
tfm_invalid_config((MCUBOOT_UPGRADE_STRATEGY STREQUAL "DIRECT_XIP" OR MCUBOOT_UPGRADE_STRATEGY STREQUAL "RAM_LOAD") AND TFM_PARTITION_FIRMWARE_UPDATE)
tfm_invalid_config(TFM_PARTITION_FIRMWARE_UPDATE AND NOT MCUBOOT_DATA_SHARING)

####################### Protected Storage Partition ###############################

# PS only uses the platform partition when PS_ROLLBACK_PROTECTION is ON, but
# the dependency in the manifest file means the dependency is unconditional
tfm_invalid_config(TFM_PARTITION_PROTECTED_STORAGE AND NOT TFM_PARTITION_PLATFORM)

########################## FIH #################################################

get_property(TFM_FIH_PROFILE_LIST CACHE TFM_FIH_PROFILE PROPERTY STRINGS)
tfm_invalid_config(NOT TFM_FIH_PROFILE IN_LIST TFM_FIH_PROFILE_LIST)

######################## TF-M Profile config check #############################

tfm_invalid_config(TFM_PROFILE STREQUAL "profile_small" AND CONFIG_TFM_SPM_BACKEND_IPC)

######################## TF-M Arch config check ################################

tfm_invalid_config(TFM_PXN_ENABLE AND NOT TFM_SYSTEM_ARCHITECTURE STREQUAL "armv8.1-m.main")

###################### Compiler check for FP support ###########################

include(config/cp_check.cmake)

###################### Platform-specific checks ################################

include(${TARGET_PLATFORM_PATH}/check_config.cmake OPTIONAL)
