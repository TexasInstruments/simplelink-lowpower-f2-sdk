#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
# Copyright (c) 2023-2024, Texas Instruments Incorporated. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.15)

# Allow the target_link_libraries() command to be called from any directory to
# add link dependencies and link interface libraries to targets created in other
# directories
cmake_policy(SET CMP0079 NEW)

# Set to "ON" to enable a subset of PSA crypto APIs to be accessed by other
# secure partitions via PSA calls (IPC model). Required to support attestation.
set (ENABLE_PSA_CRYPTO_INTEGRATION OFF)

# Key storage is required - setting this variable to "OFF" is for debug purposes only.
set (ENABLE_KEY_STORAGE ON)

# Set to "ON" to enable building mbedTLS alternative crypto implementation with
# HW acceleration. MBEDTLS_<crypto-module>_ALT defines must be uncommented in
# the mbedTLS config file to enable this feature.
set (ENABLE_MBEDTLS_ALT_HW_CRYPTO OFF)

if (ENABLE_KEY_STORAGE)
    # Internal Trusted Storage (ITS) local integration is required to support
    # Trusted Direct Call (TDC) crypto functions which utilize secure key
    # storage since the SPM is bypassed and partition boundaries cannot be
    # crossed.
    set (TI_CRYPTO_ITS_INTEGRATION    ON    CACHE BOOL  "Enable ITS integration into TI Crypto partition")
endif()

if (TI_CRYPTO_ITS_INTEGRATION)
    # Set to "ON" to enable ITS functions to be accessed by NS code or other secure
    # partitions via PSA calls (IPC model). Requires TI_CRYPTO_ITS_INTEGRATION.
    set (ENABLE_ITS_IPC_INTEGRATION ON)
endif()

add_library(ti_tfm_psa_rot_partition_crypto STATIC)

add_dependencies(ti_tfm_psa_rot_partition_crypto manifest_tool)

# TODO: Update this when attestation is implemented.
# if (ENABLE_PSA_CRYPTO_INTEGRATION)
#     target_sources(ti_tfm_psa_rot_partition_crypto
#         PRIVATE
#             ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/ecc/ECCParamsCC26X2.c
#             psa/PSA_s.c
#             psa/ti_psa_crypto.c
#             psa/ti_psa_crypto_secure_api.c
#     )
# endif()

if (TI_CRYPTO_ITS_INTEGRATION)
    add_library(ti_tfm_crypto_its STATIC)

    # Add defines to tfm_config allow integration of ITS into TI Crypto partition
    target_compile_definitions(tfm_config
        INTERFACE
            TI_CRYPTO_ITS_INTEGRATION
            TFM_PARTITION_INTERNAL_TRUSTED_STORAGE
    )

    # Add ITS source files to TI Crypto partition target
    set(ITS_PARTITION_SOURCE_DIR ${CMAKE_SOURCE_DIR}/secure_fw/partitions/internal_trusted_storage)
    target_sources(ti_tfm_crypto_its
        PRIVATE
            tfm_its_local_api.c
            ${ITS_PARTITION_SOURCE_DIR}/tfm_internal_trusted_storage.c
            ${ITS_PARTITION_SOURCE_DIR}/its_utils.c
            $<$<BOOL:${ITS_ENCRYPTION}>:${ITS_PARTITION_SOURCE_DIR}/its_crypto_interface.c>
            ${ITS_PARTITION_SOURCE_DIR}/flash/its_flash.c
            ${ITS_PARTITION_SOURCE_DIR}/flash/its_flash_nand.c
            ${ITS_PARTITION_SOURCE_DIR}/flash/its_flash_nor.c
            ${ITS_PARTITION_SOURCE_DIR}/flash/its_flash_ram.c
            ${ITS_PARTITION_SOURCE_DIR}/flash_fs/its_flash_fs.c
            ${ITS_PARTITION_SOURCE_DIR}/flash_fs/its_flash_fs_dblock.c
            ${ITS_PARTITION_SOURCE_DIR}/flash_fs/its_flash_fs_mblock.c
    )

    target_link_libraries(ti_tfm_crypto_its
        PRIVATE
            platform_s
            tfm_config
            tfm_sprt
    )

    # Reference source/third_party/tfm/cmake/install.cmake
    set(INTERFACE_INC_DIR ${CMAKE_SOURCE_DIR}/interface/include)
    set(INSTALL_INTERFACE_INC_DIR interface/include)
    install(FILES       ${INTERFACE_INC_DIR}/psa/internal_trusted_storage.h
                        ${INTERFACE_INC_DIR}/psa/storage_common.h
            DESTINATION ${INSTALL_INTERFACE_INC_DIR}/psa)
    # tfm_its_defs.h is not installed because SFN model not being used
endif()

if (ENABLE_KEY_STORAGE)
    target_sources(ti_tfm_psa_rot_partition_crypto
        PRIVATE
            ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.c
            ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.c
            ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.c
            $<$<BOOL:${CRYPTO_TFM_BUILTIN_KEYS_DRIVER}>:
                ${CMAKE_SOURCE_DIR}/secure_fw/partitions/crypto/psa_driver_api/tfm_builtin_key_loader.c
                ${CMAKE_SOURCE_DIR}/secure_fw/partitions/crypto/crypto_library.c>
    )

    ############################### MBEDCRYPTO #####################################
    ## This section was adapted from tfm/secure_fw/partitions/crypto/CMakeLists.txt
    ################################################################################
    target_link_libraries(tfm_config
        INTERFACE
            psa_crypto_config
    )

    ############################### PSA CRYPTO CONFIG ##############################
    add_library(psa_crypto_config INTERFACE)
    target_compile_definitions(psa_crypto_config
        INTERFACE
            MBEDTLS_PSA_CRYPTO_CONFIG_FILE="${TFM_MBEDCRYPTO_PSA_CRYPTO_CONFIG_PATH}"
    )

    ############################### MBEDCRYPTO #####################################

    add_library(crypto_service_mbedcrypto_config INTERFACE)

    target_compile_definitions(crypto_service_mbedcrypto_config
        INTERFACE
            MBEDTLS_CONFIG_FILE="${TFM_MBEDCRYPTO_CONFIG_PATH}"
            $<$<BOOL:${TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH}>:MBEDTLS_USER_CONFIG_FILE="${TFM_MBEDCRYPTO_PLATFORM_EXTRA_CONFIG_PATH}">
            PSA_CRYPTO_SECURE
            # Workaround for https://github.com/ARMmbed/mbedtls/issues/1077
            $<$<OR:$<STREQUAL:${TFM_SYSTEM_ARCHITECTURE},armv8-m.base>,$<STREQUAL:${TFM_SYSTEM_ARCHITECTURE},armv6-m>>:MULADDC_CANNOT_USE_R7>
            $<$<BOOL:${PLATFORM_DEFAULT_NV_SEED}>:PLATFORM_DEFAULT_NV_SEED>
            $<$<BOOL:${PLATFORM_DEFAULT_CRYPTO_KEYS}>:PLATFORM_DEFAULT_CRYPTO_KEYS>
            MBEDTLS_PSA_CRYPTO_DRIVERS
            $<$<BOOL:${CRYPTO_TFM_BUILTIN_KEYS_DRIVER}>:MBEDTLS_PSA_CRYPTO_BUILTIN_KEYS PSA_CRYPTO_DRIVER_TFM_BUILTIN_KEY_LOADER>
    )

    target_link_libraries(crypto_service_mbedcrypto_config
        INTERFACE
            tfm_config
            psa_crypto_config
    )

    cmake_policy(SET CMP0079 NEW)

    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
    set(ENABLE_TESTING OFF)
    set(ENABLE_PROGRAMS OFF)
    set(MBEDTLS_FATAL_WARNINGS OFF)
    set(ENABLE_DOCS OFF)
    set(INSTALL_MBEDTLS_HEADERS OFF)
    set(LIB_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/mbedcrypto/install)
    set(GEN_FILES OFF)

    # Set the prefix to be used by mbedTLS targets
    set(MBEDTLS_TARGET_PREFIX crypto_service_)

    # Mbedcrypto is quite a large lib, and it uses too much memory for it to be
    # reasonable to build it in debug info. As a compromise, if `debug` build type
    # is selected mbedcrypto will build under `relwithdebinfo` which preserved debug
    # symbols whild optimizing space.
    set(SAVED_BUILD_TYPE ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE ${MBEDCRYPTO_BUILD_TYPE})
    add_subdirectory(${MBEDCRYPTO_PATH} ${CMAKE_CURRENT_BINARY_DIR}/mbedcrypto EXCLUDE_FROM_ALL)
    set(CMAKE_BUILD_TYPE ${SAVED_BUILD_TYPE} CACHE STRING "Build type: [Debug, Release, RelWithDebInfo, MinSizeRel]" FORCE)

    if(NOT TARGET ${MBEDTLS_TARGET_PREFIX}mbedcrypto)
        message(FATAL_ERROR "Target ${MBEDTLS_TARGET_PREFIX}mbedcrypto does not exist. Have the patches in ${CMAKE_SOURCE_DIR}/lib/ext/mbedcrypto been applied to the mbedcrypto repo at ${MBEDCRYPTO_PATH} ?
        Hint: The command might be `cd ${MBEDCRYPTO_PATH} && git apply ${CMAKE_SOURCE_DIR}/lib/ext/mbedcrypto/*.patch`")
    endif()

    target_include_directories(${MBEDTLS_TARGET_PREFIX}mbedcrypto
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/secure_fw/partitions/crypto/psa_driver_api
            $<$<BOOL:${ENABLE_MBEDTLS_ALT_HW_CRYPTO}>:${MBEDCRYPTO_PATH}/ti/port>
    )

    # Fix platform_s and crypto_service_mbedcrypto libraries cyclic linking
    set_target_properties(${MBEDTLS_TARGET_PREFIX}mbedcrypto PROPERTIES LINK_INTERFACE_MULTIPLICITY 3)

    # Alternative crypto HW accelerator implementations
    target_sources(${MBEDTLS_TARGET_PREFIX}mbedcrypto
        PRIVATE
            $<$<BOOL:${ENABLE_MBEDTLS_ALT_HW_CRYPTO}>:
                ${MBEDCRYPTO_PATH}/ti/port/aes_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/ccm_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/cmac_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/ecdh_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/ecdsa_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/ecjpake_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/eddsa_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/entropy_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/gcm_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/sha256_alt.c
                ${MBEDCRYPTO_PATH}/ti/port/sha512_alt.c>
    )

    target_compile_options(${MBEDTLS_TARGET_PREFIX}mbedcrypto
        PRIVATE
            $<$<C_COMPILER_ID:GNU>:-Wno-unused-const-variable>
            $<$<C_COMPILER_ID:GNU>:-Wno-unused-parameter>
            $<$<C_COMPILER_ID:ARMClang>:-Wno-unused-const-variable>
            $<$<C_COMPILER_ID:ARMClang>:-Wno-unused-parameter>
    )

    target_compile_definitions(${MBEDTLS_TARGET_PREFIX}mbedcrypto
        PRIVATE
            MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER
    )

    # Add mbedtls config file to PSA driver - ti_sl target
    # In mbedtls 3.5.0 MBEDTLS_CONFIG_FILE must be passed through as ti_sl is
    # not directly linked against any mbedtls targets so ti_sl does not inherit
    # the compile definitions
    target_compile_definitions(${MBEDTLS_TARGET_PREFIX}ti_sl
        PRIVATE
            MBEDTLS_CONFIG_FILE="${TFM_MBEDCRYPTO_CONFIG_PATH}"
    )

    target_link_libraries(${MBEDTLS_TARGET_PREFIX}mbedcrypto
        PRIVATE
            psa_interface
            platform_s
        PUBLIC
            crypto_service_mbedcrypto_config
        INTERFACE
            platform_common_interface
    )
endif()

target_sources(ti_tfm_psa_rot_partition_crypto
    PRIVATE
        # SysConfig generated driver configuration
        ${CMAKE_BINARY_DIR}/generated/syscfg/ti_drivers_config.c

        # Driver Porting Layer
        ${TIDRIVERS_INSTALL_DIR}/kernel/tfm/dpl/HwiP_tfm.c
        ${TIDRIVERS_INSTALL_DIR}/kernel/tfm/dpl/SemaphoreP_tfm.c
        ${TIDRIVERS_INSTALL_DIR}/kernel/tfm/dpl/SwiP_tfm.c

        # Secure Callback
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/tfm/SecureCallback.c

        # Power driver stubs
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/power/PowerCC26X4_stubs.c

        # AES drivers (Crypto HW accelerator)
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESCCM.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesccm/AESCCMCC26X4.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesccm/AESCCMCC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESCBC.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aescbc/AESCBCCC26XX.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aescbc/AESCBCCC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESCMAC.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aescmac/AESCMACCC26XX.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aescmac/AESCMACCC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESCTR.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesctr/AESCTRCC26XX.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesctr/AESCTRCC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESECB.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesecb/AESECBCC26XX.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesecb/AESECBCC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESGCM.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesgcm/AESGCMCC26X4.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesgcm/AESGCMCC26X4_s.c

        # ECC drivers (Crypto HW accelerator)
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/ecc/ECCParamsCC26X2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/sharedresources/PKAResourceCC26XX.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ECDH.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ecdh/ECDHCC26X2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ecdh/ECDHCC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ECDSA.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ecdsa/ECDSACC26X2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ecdsa/ECDSACC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ECJPAKE.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ecjpake/ECJPAKECC26X2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/ecjpake/ECJPAKECC26X4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/EDDSA.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/eddsa/EDDSACC26X2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/eddsa/EDDSACC26X4_s.c

        # SHA2 driver (Crypto HW accelerator)
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/SHA2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/sha2/SHA2CC26X2.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/sha2/SHA2CC26X4_s.c

        # Random number generation drivers (Crypto HW accelerator)
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/AESCTRDRBG.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesctrdrbg/AESCTRDRBGXX.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/aesctrdrbg/AESCTRDRBGX4_s.c

        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/TRNG.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/trng/TRNGCC26XX.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/trng/TRNGCC26X4_s.c

        # Common crypto utils
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/cryptokey/CryptoKey.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/cryptoutils/utils/CryptoUtils.c

        # Utils for TRNG
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/utils/List.c
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/utils/StructRingBuf.c

        # Crypto secure partition main thread
        ${TIDRIVERS_INSTALL_DIR}/source/ti/drivers/crypto/CryptoCC26X4_s.c
)

# The generated sources
target_sources(ti_tfm_psa_rot_partition_crypto
    PRIVATE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_crypto/auto_generated/intermedia_crypto_sp.c
)
target_sources(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_crypto/auto_generated/load_info_crypto_sp.c
)

# Set include directory
target_include_directories(ti_tfm_psa_rot_partition_crypto
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        ${CMAKE_BINARY_DIR}/generated/syscfg/
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_crypto
        $<$<BOOL:${ENABLE_KEY_STORAGE}>:${MBEDCRYPTO_PATH}/../.. ${MBEDCRYPTO_PATH}/include>
)
target_include_directories(tfm_partitions
    INTERFACE
        ${CMAKE_BINARY_DIR}/generated/secure_fw/partitions/ti_crypto
)

target_compile_definitions(ti_tfm_psa_rot_partition_crypto
    PUBLIC
        $<$<BOOL:${ENABLE_ITS_IPC_INTEGRATION}>:ENABLE_ITS_IPC_INTEGRATION>
        $<$<BOOL:${ENABLE_KEY_STORAGE}>:ENABLE_KEY_STORAGE MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER>
        $<$<BOOL:${CRYPTO_TFM_BUILTIN_KEYS_DRIVER}>:PSA_CRYPTO_DRIVER_TFM_BUILTIN_KEY MBEDTLS_PSA_CRYPTO_BUILTIN_KEYS TFM_SP_CRYPTO=CRYPTO_SP>
)

target_compile_options(ti_tfm_psa_rot_partition_crypto
    PUBLIC
        $<$<C_COMPILER_ID:GNU>:-Wno-unused-parameter>
        $<$<C_COMPILER_ID:ARMClang>:-Wno-unused-parameter>
)

target_link_libraries(ti_tfm_psa_rot_partition_crypto
    PRIVATE
        platform_s
        $<$<BOOL:${ENABLE_KEY_STORAGE}>:crypto_service_mbedcrypto>
        $<$<BOOL:${TI_CRYPTO_ITS_INTEGRATION}>:ti_tfm_crypto_its>
        tfm_config
        tfm_sprt
        $<$<C_COMPILER_ID:ARMClang>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc13x4_cc26x4/driverlib/bin/ticlang/driverlib.lib>
        $<$<C_COMPILER_ID:GNU>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc13x4_cc26x4/driverlib/bin/gcc/driverlib.lib>
        $<$<C_COMPILER_ID:IAR>:${DRIVERLIB_SOURCE_DIR}/ti/devices/cc13x4_cc26x4/driverlib/bin/iar/driverlib.lib>
    INTERFACE
        platform_region_defs
)

################################ TFM SPM #######################################

target_sources(tfm_spm
    PRIVATE
        crypto_sp_interrupts.c
)

############################ Partition Defs ####################################

target_link_libraries(tfm_partitions
    INTERFACE
        ti_tfm_psa_rot_partition_crypto
)

############################### TI SysConfig ###################################

# Custom target and command to generate ti_drivers_config.c/.h
set(SYSCFG_DIR ${CMAKE_CURRENT_LIST_DIR}/syscfg)
set(METADATA_PRODUCTS --product ${TIDRIVERS_INSTALL_DIR}/.metadata/product.json --product ${TIUTILS_INSTALL_DIR}/.metadata/product.json)
set(SYSCONFIG_ARGS -d CC2674R10RGZ -o syscfg -c ticlang ${METADATA_PRODUCTS} ${SYSCFG_DIR}/crypto_sp.syscfg)
add_custom_target(ti_sysconfig)
add_custom_command(TARGET ti_sysconfig
                   PRE_BUILD
                   COMMAND ${SYSCONFIG_TOOL} ${SYSCONFIG_ARGS}
                   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/generated
                   DEPENDS ${SYSCFG_DIR}/crypto_sp.syscfg
                   BYPRODUCTS ${CMAKE_BINARY_DIR}/generated/syscfg/ti_drivers_config.c
                              ${CMAKE_BINARY_DIR}/generated/syscfg/ti_drivers_config.h
                   COMMENT "Running SysConfig to generate ti_drivers_config.c/.h"
                   VERBATIM)
