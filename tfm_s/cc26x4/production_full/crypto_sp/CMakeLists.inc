#-------------------------------------------------------------------------------
# Copyright (c) 2022 Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# Set to "ON" to enable a subset of PSA crypto APIs to be accessed by other
# secure partitions via PSA calls (IPC model). Required to support attestation.
set (ENABLE_PSA_CRYPTO_INTEGRATION ON)

# Key storage is required - setting this variable to "OFF" is for debug purposes only.
set (ENABLE_KEY_STORAGE ON)

if (ENABLE_KEY_STORAGE)
    # Internal Trusted Storage (ITS) local integration is required for secure key storage.
    set (ENABLE_ITS_LOCAL_INTEGRATION ON)
endif()

# Set to "ON" to enable ITS functions to be accessed by NS code or other secure
# partitions via PSA calls (IPC model).
set (ENABLE_ITS_IPC_INTEGRATION OFF)

add_definitions(-DDeviceFamily_CC26X4)
add_definitions(-DTFM_ENABLED=1)

set(SYSCFG_DIR "${CMAKE_CURRENT_LIST_DIR}/syscfg/")

embedded_include_directories(PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)
embedded_include_directories(PATH "${REMOTE_GEN_DIR}/generated" ABSOLUTE)
embedded_include_directories(PATH "${SOURCE_REPO}" ABSOLUTE)
embedded_include_directories(PATH "${KERNEL_REPO}/tirtos7/packages" ABSOLUTE)

if (ENABLE_PSA_CRYPTO_INTEGRATION)
    list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/ecc/ECCParamsCC26X2.c")
    list(APPEND ALL_SRC_C_S "${CMAKE_CURRENT_LIST_DIR}/psa/PSA_s.c")
    list(APPEND ALL_SRC_C_S "${CMAKE_CURRENT_LIST_DIR}/psa/ti_psa_crypto.c")
    list(APPEND ALL_SRC_C_S "${CMAKE_CURRENT_LIST_DIR}/psa/ti_psa_crypto_secure_api.c")
endif()

if (ENABLE_ITS_LOCAL_INTEGRATION)
    add_definitions(-DENABLE_ITS_LOCAL_INTEGRATION)
    list(APPEND ALL_SRC_C_S "${CMAKE_CURRENT_LIST_DIR}/tfm_its_local_api.c")
endif()

if (ENABLE_ITS_IPC_INTEGRATION)
    add_definitions(-DENABLE_ITS_IPC_INTEGRATION)
endif()

if (ENABLE_KEY_STORAGE)
    add_definitions(-DENABLE_KEY_STORAGE)

    # Secure Key Storage driver
    list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.c")
    list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.c")
    list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_helpers.c")

    # MbedTLS
    set (MBEDCRYPTO_CONAN_INSTALL_DIR ${MBEDCRYPTO_INSTALL_DIR})
    set (MBEDTLS_CONFIG_FILE "<configs/config-tidrivers.h>")
    embedded_include_directories(PATH "${MBEDCRYPTO_CONAN_INSTALL_DIR}/source" ABSOLUTE)
    embedded_include_directories(PATH "${MBEDCRYPTO_CONAN_INSTALL_DIR}/source/third_party/mbedcrypto/include" ABSOLUTE)
    embedded_include_directories(PATH "${MBEDCRYPTO_CONAN_INSTALL_DIR}/source/third_party/mbedcrypto/include/psa" ABSOLUTE)
    embedded_include_directories(PATH "${MBEDCRYPTO_CONAN_INSTALL_DIR}/source/third_party/mbedcrypto/library" ABSOLUTE)
    embedded_include_directories(PATH "${MBEDCRYPTO_CONAN_INSTALL_DIR}/source/third_party/mbedcrypto/ti" ABSOLUTE)
    embedded_include_directories(PATH "${SOURCE_REPO}/third_party/tfm/secure_fw/partitions/crypto/" ABSOLUTE)
    add_definitions(-DMBEDTLS_CONFIG_FILE=${MBEDTLS_CONFIG_FILE})

    # Define location of Mbed Crypto source, build, and install directory.
    set (MBEDCRYPTO_SOURCE_DIR "${MBEDCRYPTO_CONAN_INSTALL_DIR}/source/third_party/mbedcrypto")
    set (MBEDCRYPTO_BINARY_DIR "${REMOTE_GEN_DIR}/mbed-crypto/build")
    set (MBEDCRYPTO_INSTALL_DIR "${MBEDCRYPTO_BINARY_DIR}/../install")
    set (MBEDCRYPTO_TARGET_NAME "mbedcrypto_lib")

    # Set MbedTLS compiler flags
    string(APPEND MBEDCRYPTO_C_FLAGS " ${MBEDCRYPTO_C_FLAGS_SERVICES}")
    string(APPEND MBEDCRYPTO_C_FLAGS " -DMBEDTLS_CONFIG_FILE=\\\"${MBEDTLS_CONFIG_FILE}\\\""
                                     " -I${MBEDCRYPTO_CONAN_INSTALL_DIR}/source/third_party/mbedcrypto/ti")

    # Add TF-M include directory so Mbed Crypto can include PSA ITS headers
    string(APPEND MBEDCRYPTO_C_FLAGS " -I${TFM_ROOT_DIR}/interface/include")

    # Build Mbed Crypto as external project.
    # This ensures Mbed Crypto is built with exactly defined settings.
    # Mbed Crypto will be used from its install location
    include(${TFM_ROOT_DIR}/BuildMbedCrypto.cmake)
endif()

# Sysconfig generated driver config
list(APPEND ALL_SRC_C_S "${REMOTE_GEN_DIR}/generated/ti_drivers_config.c")

# Driver Porting Layer
list(APPEND ALL_SRC_C_S "${KERNEL_REPO}/tfm/dpl/HwiP_tfm.c")
list(APPEND ALL_SRC_C_S "${KERNEL_REPO}/tfm/dpl/SemaphoreP_tfm.c")
list(APPEND ALL_SRC_C_S "${KERNEL_REPO}/tfm/dpl/SwiP_tfm.c")

# Secure Callback
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/tfm/SecureCallback.c")

# Power driver helpers
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/power/PowerCC26X2_helpers.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/power/PowerCC26X2_calibrateRCOSC_helpers.c")

# Power driver stubs
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/power/PowerCC26X4_stubs.c")

# AES drivers
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/sharedresources/CryptoResourceCC26XX.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESCCM.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesccm/AESCCMCC26X4.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesccm/AESCCMCC26X4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESCBC.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aescbc/AESCBCCC26XX.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aescbc/AESCBCCC26X4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESCMAC.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aescmac/AESCMACCC26XX.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aescmac/AESCMACCC26X4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESCTR.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesctr/AESCTRCC26XX.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesctr/AESCTRCC26X4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESECB.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesecb/AESECBCC26XX.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesecb/AESECBCC26X4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESGCM.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesgcm/AESGCMCC26X4.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesgcm/AESGCMCC26X4_s.c")

# ECC drivers
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/ecc/ECCParamsCC26X4_s.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/sharedresources/PKAResourceCC26XX.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/ECDH.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/ecdh/ECDHCC26X2.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/ecdh/ECDHCC26X4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/ECDSA.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/ecdsa/ECDSACC26X2.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/ecdsa/ECDSACC26X4_s.c")

# SHA2 driver
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/SHA2.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/sha2/SHA2CC26X2.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/sha2/SHA2CC26X4_s.c")

# Random number generation drivers
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/AESCTRDRBG.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesctrdrbg/AESCTRDRBGXX.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/aesctrdrbg/AESCTRDRBGX4_s.c")

list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/TRNG.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/trng/TRNGCC26XX.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/trng/TRNGCC26X4_s.c")

# Common crypto utils
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/cryptokey/CryptoKey.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/cryptoutils/utils/CryptoUtils.c")

# Utils for TRNG
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/utils/List.c")
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/utils/StructRingBuf.c")

# Crypto secure partition main thread
list(APPEND ALL_SRC_C_S "${SOURCE_REPO}/ti/drivers/crypto/CryptoCC26X4_s.c")

# Custom target and command to generate ti_drivers_config.c/.h
set(SYSCONFIG_ARGS -d CC2674R10RGZ -s ${SYSCFG_DIR}/product.json -o generated -c ticlang ${SYSCFG_DIR}/crypto_sp.syscfg)
add_custom_target(ti_sysconfig)
add_custom_command(TARGET ti_sysconfig
                   PRE_BUILD
                   COMMAND "${SYSCONFIG_TOOL}" ${SYSCONFIG_ARGS}
                   WORKING_DIRECTORY "${REMOTE_GEN_DIR}"
                   DEPENDS "${SYSCFG_DIR}/crypto_sp.syscfg"
                   BYPRODUCTS "${REMOTE_GEN_DIR}/generated/ti_drivers_config.c"
                   COMMENT "Running SysConfig to generate ti_drivers_config.c/.h"
                   VERBATIM)
