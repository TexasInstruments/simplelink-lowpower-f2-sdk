#-------------------------------------------------------------------------------
# Copyright (c) 2018-2020, Arm Limited. All rights reserved.
# Copyright (c) 2019-2022, Texas Instruments Incorporated.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

if(NOT DEFINED REGRESSION)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: REGRESSION not defined, Include this file from a Config*.cmake")
elseif(NOT DEFINED CORE_TEST)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: CORE_TEST not defined, Include this file from a Config*.cmake")
elseif(NOT DEFINED TFM_LVL)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: TFM_LVL not defined, Include this file from a Config*.cmake")
elseif(NOT DEFINED CORE_IPC)
	message(FATAL_ERROR "ERROR: Incomplete Configuration: CORE_IPC not defined. Include this file from a Config*.cmake")
endif()

if(NOT DEFINED COMPILER)
	message(FATAL_ERROR "ERROR: COMPILER is not set in command line")
elseif((NOT ${COMPILER} STREQUAL "ARMCLANG") AND (NOT ${COMPILER} STREQUAL "GNUARM") AND (NOT ${COMPILER} STREQUAL "IARARM") AND (NOT ${COMPILER} STREQUAL "TICLANGARM"))
	message(FATAL_ERROR "ERROR: Compiler \"${COMPILER}\" is not supported.")
endif()

#Configure the default build type
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (i.e. Debug)")

#Ignore case on the cmake build types
string(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

if(CORE_IPC)
	if (TFM_LVL EQUAL 3)
		message(FATAL_ERROR "ERROR: Invalid isolation level!")
	endif()
else()
	if(NOT TFM_LVL EQUAL 1)
		message(FATAL_ERROR "ERROR: Invalid isolation level!")
	endif()
endif()

#BL2 bootloader (MCUBoot) related settings
include(${CMAKE_CURRENT_LIST_DIR}/bl2/ext/mcuboot/MCUBootConfig.cmake)

set(BUILD_CMSIS_CORE Off)
set(BUILD_RETARGET Off)
set(BUILD_NATIVE_DRIVERS Off)
set(BUILD_TIME Off)
set(BUILD_STARTUP Off)
set(BUILD_TARGET_CFG Off)
set(BUILD_TARGET_HARDWARE_KEYS Off)
set(BUILD_TARGET_NV_COUNTERS Off)
set(BUILD_CMSIS_DRIVERS Off)
set(BUILD_UART_STDOUT Off)
set(BUILD_FLASH Off)
set(BUILD_PLAT_TEST Off)
set(BUILD_BOOT_HAL Off)
if(NOT DEFINED PLATFORM_CMAKE_FILE)
	message (FATAL_ERROR "Platform specific CMake is not defined. Please set PLATFORM_CMAKE_FILE.")
elseif(NOT EXISTS ${PLATFORM_CMAKE_FILE})
	message (FATAL_ERROR "Platform specific CMake \"${PLATFORM_CMAKE_FILE}\" file does not exist. Please fix value of PLATFORM_CMAKE_FILE.")
else()
	include(${PLATFORM_CMAKE_FILE})
endif()

# Select the corresponding CPU type and configuration according to current
# building status in multi-core scenario.
# The updated configuration will be used in following compiler setting.
if (DEFINED TFM_MULTI_CORE_TOPOLOGY AND TFM_MULTI_CORE_TOPOLOGY)
	if (NOT CORE_IPC)
		message(FATAL_ERROR "CORE_IPC is OFF. Multi-core topology should work in IPC model.")
	endif()

	include("Common/MultiCore")

	if (NOT DEFINED TFM_BUILD_IN_SPE)
		message(FATAL_ERROR "Flag of building in SPE is not specified. Please set TFM_BUILD_IN_SPE.")
	else()
		select_arm_cpu_type(${TFM_BUILD_IN_SPE})
	endif()

	# CMSE is unnecessary in multi-core scenarios.
	# TODO: Need further discussion about if CMSE is required when an Armv8-M
	# core acts as secure core in multi-core scenario.
	# leave CMSE_FLAGS undefined
else()
	if(${COMPILER} STREQUAL "IARARM")
		set (CMSE_FLAGS "--cmse")
	else()
		set (CMSE_FLAGS "-mcmse")
	endif()

	# Clear multi-core test setting
	set (TFM_MULTI_CORE_TEST OFF)
endif()

if(${COMPILER} STREQUAL "ARMCLANG")
	#Use any ARMCLANG version found on PATH. Note: Only versions supported by the
	#build system will work. A file cmake/Common/CompilerArmClangXY.cmake
	#must be present with a matching version.
	include("Common/FindArmClang")
	include("Common/${ARMCLANG_MODULE}")

	set (COMMON_COMPILE_FLAGS -fshort-enums -fshort-wchar -funsigned-char -mfpu=none -ffunction-sections -fdata-sections -fno-builtin -nostdlib)
	##Shared compiler settings.
	function(config_setting_shared_compiler_flags tgt)
		embedded_set_target_compile_flags(TARGET ${tgt} LANGUAGE C APPEND FLAGS -xc -std=c99 ${COMMON_COMPILE_FLAGS} -Wall -Werror)
	endfunction()

	##Shared linker settings.
	function(config_setting_shared_linker_flags tgt)
		embedded_set_target_link_flags(TARGET ${tgt} FLAGS --strict --map --symbols --xref --entry=Reset_Handler --remove --info=summarysizes,sizes,totals,unused,veneers)
	endfunction()
elseif(${COMPILER} STREQUAL "GNUARM")
	#Use any GNUARM version found on PATH. Note: Only versions supported by the
	#build system will work. A file cmake/Common/CompilerGNUARMXY.cmake
	#must be present with a matching version.
	include("Common/FindGNUARM")
	include("Common/${GNUARM_MODULE}")

	##[TI-TFM] Removed "-msoft-float" from the common compile flags.
	##Floating point flags are set in /cmake/Common/CompilerGNUARMxx.cmake based on CPU type to allow support for Cortex-M33F.
	set (COMMON_COMPILE_FLAGS -fshort-enums -fshort-wchar -funsigned-char -ffunction-sections -fdata-sections --specs=nano.specs -fno-builtin)
	##Shared compiler and linker settings.
	function(config_setting_shared_compiler_flags tgt)
		embedded_set_target_compile_flags(TARGET ${tgt} LANGUAGE C APPEND FLAGS -xc -std=c99 ${COMMON_COMPILE_FLAGS} -Wall -Werror -Wno-format -Wno-return-type -Wno-unused-but-set-variable)
	endfunction()

	##[TI-TFM] Added "--nmagic" to the common linker flags to avoid 0x10000 alignment of sections.
	##Shared linker settings.
	function(config_setting_shared_linker_flags tgt)
		#--no-wchar-size-warning flag is added because TF-M sources are compiled
		#with short wchars, however the standard library is compiled with normal
		#wchar, and this generates linker time warnings. TF-M code does not use
		#wchar, so the warning can be suppressed.
		embedded_set_target_link_flags(TARGET ${tgt} FLAGS -Wl,--check-sections,--fatal-warnings,--gc-sections,--no-wchar-size-warning,--print-memory-usage,--nmagic --entry=Reset_Handler --specs=nano.specs)
	endfunction()
elseif(${COMPILER} STREQUAL "IARARM")
	#Use any IARARM version found on PATH. Note: Only versions supported by the
	#build system will work. A file cmake/Common/CompilerIARARMXY.cmake
	#must be present with a matching version.
	include("Common/FindIARARM")
	include("Common/${IARARM_MODULE}")

	set (COMMON_COMPILE_FLAGS -e --dlib_config=full --vla --silent -DNO_TYPEOF --diag_suppress Pe546,Pe940,Pa082,Pa084)
	##Shared compiler and linker settings.
	function(config_setting_shared_compiler_flags tgt)
		embedded_set_target_compile_flags(TARGET ${tgt} LANGUAGE C APPEND FLAGS ${COMMON_COMPILE_FLAGS} "-DImage$$= " "-DLoad$$LR$$= " "-D$$ZI$$Base=$$Base" "-D$$ZI$$Limit=$$Limit" "-D$$RO$$Base=$$Base" "-D$$RO$$Limit=$$Limit" "-D$$RW$$Base=$$Base" "-D$$RW$$Limit=$$Limit" "-D_DATA$$RW$$Base=_DATA$$Base" "-D_DATA$$RW$$Limit=_DATA$$Limit" "-D_DATA$$ZI$$Base=_DATA$$Base" "-D_DATA$$ZI$$Limit=_DATA$$Limit" "-D_STACK$$ZI$$Base=_STACK$$Base" "-D_STACK$$ZI$$Limit=_STACK$$Limit" )
	endfunction()

	##Shared linker settings.
	function(config_setting_shared_linker_flags tgt)
		#--no-wchar-size-warning flag is added because TF-M sources are compiled
		#with short wchars, however the standard library is compiled with normal
		#wchar, and this generates linker time warnings. TF-M code does not use
		#wchar, so the warning can be suppressed.
		embedded_set_target_link_flags(TARGET ${tgt} FLAGS --silent --semihosting --redirect __write=__write_buffered)
	endfunction()
#[TI-TFM] Added TI Clang support.
elseif(${COMPILER} STREQUAL "TICLANGARM")
	#Extract compiler version, compute compiler module name
	include("Common/CompilerTIClangArmCommon")
	include("Common/${TICLANGARM_MODULE}")

	set (COMMON_COMPILE_FLAGS -funsigned-char -mcmse)

	##Shared compiler settings.
	function(config_setting_shared_compiler_flags tgt)
		embedded_set_target_compile_flags(TARGET ${tgt} LANGUAGE C FLAGS -x c -std=c99 ${COMMON_COMPILE_FLAGS} -Wall -Werror -Wno-format -Wno-return-type)
	endfunction()

	##Shared linker settings.
	function(config_setting_shared_linker_flags tgt)
		embedded_set_target_link_flags(TARGET ${tgt} FLAGS -Wl,-q -Wl,-e,Reset_Handler)
	endfunction()
endif()

#Create a string from the compile flags list, so that it can be used later
#in this file to set mbedtls and BL2 flags
list_to_string(COMMON_COMPILE_FLAGS_STR ${COMMON_COMPILE_FLAGS})

#Settings which shall be set for all projects the same way based
# on the variables above.
set (TFM_PARTITION_TEST_CORE OFF)
set (TFM_PARTITION_TEST_CORE_IPC OFF)
set (CORE_TEST_POSITIVE OFF)
set (CORE_TEST_INTERACTIVE OFF)
set (TFM_PARTITION_TEST_SECURE_SERVICES OFF)
set (TFM_PARTITION_TEST_PS OFF)
set (SERVICES_TEST_ENABLED OFF)
set (TEST_FRAMEWORK_S  OFF)
set (TEST_FRAMEWORK_NS OFF)
set (TFM_PSA_API OFF)

if (NOT DEFINED TFM_PARTITION_AUDIT_LOG)
	# Enable the TF-M Audit Log partition
	set(TFM_PARTITION_AUDIT_LOG ON)
endif()
if (NOT DEFINED TFM_PARTITION_PLATFORM)
	# Enable the TF-M Platform partition
	set(TFM_PARTITION_PLATFORM ON)
endif()
if (NOT DEFINED TFM_PARTITION_PROTECTED_STORAGE)
	# Enable the TF-M Protected storage partition
	set(TFM_PARTITION_PROTECTED_STORAGE ON)
endif()
if (NOT DEFINED TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
	# Enable the TF-M internal trusted storage partition
	set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON)
endif()
if (NOT DEFINED TFM_PARTITION_CRYPTO)
	# Enable the TF-M crypto partition
	set(TFM_PARTITION_CRYPTO ON)
endif()
if (NOT DEFINED TFM_PARTITION_INITIAL_ATTESTATION)
	# Enable the TF-M initial attestation partition
	set(TFM_PARTITION_INITIAL_ATTESTATION ON)
endif()

if (NOT TFM_LVL EQUAL 1 AND NOT DEFINED CONFIG_TFM_ENABLE_MEMORY_PROTECT)
	set (CONFIG_TFM_ENABLE_MEMORY_PROTECT ON)
endif()

# [TI-TFM] SPE implements its own crypto partition and does not use TFM_PARTITION_CRYPTO
if (NOT DEFINED TI_TFM)
	if (TFM_PARTITION_INITIAL_ATTESTATION OR TFM_PARTITION_PROTECTED_STORAGE)
		#PSA Initial Attestation and Protected storage rely on Cryptography API
		set(TFM_PARTITION_CRYPTO ON)
	endif()
endif()

if (TFM_PARTITION_PROTECTED_STORAGE)
	set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE ON)
endif()

# Option to demonstrate usage of secure-only peripheral
set (SECURE_UART1 OFF)

if (PLATFORM_SVC_HANDLERS)
	add_definitions(-DPLATFORM_SVC_HANDLERS)
endif()

if (REGRESSION)
	set(SERVICES_TEST_ENABLED ON)
endif()

if (CORE_IPC)
	set(TFM_PSA_API ON)

	# Disable IPC Test by default if the config or platform doesn't explicitly
	# require it
	if (NOT DEFINED IPC_TEST)
		set(IPC_TEST OFF)
	endif()
else()
	set(IPC_TEST OFF)
endif()

if (TFM_PSA_API)
	add_definitions(-DTFM_PSA_API)
endif()

if (DEFINED TFM_MULTI_CORE_TOPOLOGY AND TFM_MULTI_CORE_TOPOLOGY)
	add_definitions(-DTFM_MULTI_CORE_TOPOLOGY)

	# Skip multi-core test cases if regression test is disabled
	if (NOT REGRESSION)
		set(TFM_MULTI_CORE_TEST OFF)
	endif()
endif()

if (SERVICES_TEST_ENABLED)
	set(SERVICE_TEST_S ON)
	set(SERVICE_TEST_NS ON)
endif()

if (CORE_TEST)
	if (NOT CORE_IPC OR TFM_LVL EQUAL 1)
		set(CORE_TEST_POSITIVE ON)
	endif()
	set(CORE_TEST_INTERACTIVE OFF)
endif()

if (CORE_TEST_INTERACTIVE)
	add_definitions(-DCORE_TEST_INTERACTIVE)
	set(TEST_FRAMEWORK_NS ON)
	set(TFM_PARTITION_TEST_CORE ON)
endif()

if (CORE_TEST_POSITIVE)
	add_definitions(-DCORE_TEST_POSITIVE)
	set(TEST_FRAMEWORK_NS ON)
	set(TFM_PARTITION_TEST_CORE ON)
endif()

if (TFM_PARTITION_TEST_CORE)
	# If the platform or the topology doesn't specify whether IRQ test is
	# supported, enable it by default.
	if (NOT DEFINED TFM_ENABLE_IRQ_TEST)
		set(TFM_ENABLE_IRQ_TEST ON)
	endif()

	if (TFM_ENABLE_IRQ_TEST)
		add_definitions(-DTFM_ENABLE_IRQ_TEST)
	endif()
else()
	set(TFM_ENABLE_IRQ_TEST OFF)
endif()

if (IPC_TEST)
	add_definitions(-DENABLE_IPC_TEST)
	set(TEST_FRAMEWORK_NS ON)
	set(TFM_PARTITION_TEST_CORE_IPC ON)
endif()

if (SERVICE_TEST_S)
	add_definitions(-DSERVICES_TEST_S)
	set(TEST_FRAMEWORK_S ON)
endif()

if (SERVICE_TEST_NS)
	add_definitions(-DSERVICES_TEST_NS)
	set(TEST_FRAMEWORK_NS ON)
endif()

if (TEST_FRAMEWORK_S)
	add_definitions(-DTEST_FRAMEWORK_S)
	# The secure client partition is required to run secure tests
	set(TFM_PARTITION_TEST_SECURE_SERVICES ON)
endif()

if (TEST_FRAMEWORK_NS)
	add_definitions(-DTEST_FRAMEWORK_NS)
endif()

if (CORE_IPC)
	set(TFM_PARTITION_AUDIT_LOG OFF)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/test/TestConfig.cmake)

if (TFM_PARTITION_AUDIT_LOG)
	add_definitions(-DTFM_PARTITION_AUDIT_LOG)
endif()

if (TFM_PARTITION_PLATFORM)
	add_definitions(-DTFM_PARTITION_PLATFORM)
endif()

if (TFM_PARTITION_PROTECTED_STORAGE)
	add_definitions(-DTFM_PARTITION_PROTECTED_STORAGE)
endif()

if (TFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
	add_definitions(-DTFM_PARTITION_INTERNAL_TRUSTED_STORAGE)
endif()

if (TFM_PARTITION_CRYPTO)
	add_definitions(-DTFM_PARTITION_CRYPTO)
endif()

if (TFM_PARTITION_INITIAL_ATTESTATION)
	add_definitions(-DTFM_PARTITION_INITIAL_ATTESTATION)

	if (NOT DEFINED SYMMETRIC_INITIAL_ATTESTATION)
		set(SYMMETRIC_INITIAL_ATTESTATION OFF)
	endif()

	if (SYMMETRIC_INITIAL_ATTESTATION)
		add_definitions(-DSYMMETRIC_INITIAL_ATTESTATION)
	endif()
endif()

if (TFM_PARTITION_TEST_CORE)
	add_definitions(-DTFM_PARTITION_TEST_CORE)
endif()

if (TFM_PARTITION_TEST_CORE_IPC)
	add_definitions(-DTFM_PARTITION_TEST_CORE_IPC)
endif()

if (TFM_PARTITION_TEST_SECURE_SERVICES)
	add_definitions(-DTFM_PARTITION_TEST_SECURE_SERVICES)
endif()

if (CONFIG_TFM_ENABLE_MEMORY_PROTECT)
	add_definitions(-DCONFIG_TFM_ENABLE_MEMORY_PROTECT)
endif()

if (PSA_API_TEST)
	add_definitions(-DPSA_API_TEST_NS)
	set(PSA_API_TEST_NS ON)
	if (NOT DEFINED PSA_API_TEST_CRYPTO)
		set(PSA_API_TEST_CRYPTO OFF)
	endif()
	if (NOT DEFINED PSA_API_TEST_STORAGE)
		set(PSA_API_TEST_STORAGE OFF)
	endif()
	if (NOT DEFINED PSA_API_TEST_INTERNAL_TRUSTED_STORAGE)
		set(PSA_API_TEST_INTERNAL_TRUSTED_STORAGE OFF)
	endif()
	if (NOT DEFINED PSA_API_TEST_PROTECTED_STORAGE)
		set(PSA_API_TEST_PROTECTED_STORAGE OFF)
	endif()
	if (NOT DEFINED PSA_API_TEST_INITIAL_ATTESTATION)
		set(PSA_API_TEST_INITIAL_ATTESTATION OFF)
	endif()
	if (NOT DEFINED PSA_API_TEST_IPC)
		set(PSA_API_TEST_IPC OFF)
	endif()

	#Set PSA API compliance test build path
	if(NOT DEFINED PSA_API_TEST_BUILD_PATH)
		#If not specified, assume it's the default build folder checked out at the same level of TFM root dir
		set(PSA_API_TEST_BUILD_PATH "${TFM_ROOT_DIR}/../psa-arch-tests/api-tests/BUILD")
	endif()
endif()

# The config for enable secure context management in TF-M
if (NOT DEFINED CONFIG_TFM_ENABLE_CTX_MGMT)
	set(CONFIG_TFM_ENABLE_CTX_MGMT ON)
endif()

if (CONFIG_TFM_ENABLE_CTX_MGMT)
	add_definitions(-DCONFIG_TFM_ENABLE_CTX_MGMT)
endif()

# This flag indicates if the non-secure OS is capable of identify the non-secure clients
# which call the secure services. It is diabled in IPC model.
if (NOT DEFINED TFM_NS_CLIENT_IDENTIFICATION)
	if (TFM_PSA_API)
		set(TFM_NS_CLIENT_IDENTIFICATION OFF)
	else()
		if (CONFIG_TFM_ENABLE_CTX_MGMT)
			set(TFM_NS_CLIENT_IDENTIFICATION ON)
		else()
			set(TFM_NS_CLIENT_IDENTIFICATION OFF)
		endif()
	endif()
endif()

if (NOT CONFIG_TFM_ENABLE_CTX_MGMT AND TFM_NS_CLIENT_IDENTIFICATION)
	# NS client ID is part of context management.
	message(FATAL_ERROR "TFM_NS_CLIENT_IDENTIFICATION cannot be ON when CONFIG_TFM_ENABLE_CTX_MGMT is OFF")
endif()

if (BL2)
	# Add MCUBOOT_IMAGE_NUMBER definition to the compiler command line.
	add_definitions(-DMCUBOOT_IMAGE_NUMBER=${MCUBOOT_IMAGE_NUMBER})

	if (${MCUBOOT_UPGRADE_STRATEGY} STREQUAL "NO_SWAP")
		set(LINK_TO_BOTH_MEMORY_REGION ON)
	endif()

	if (MCUBOOT_REPO STREQUAL "TF-M")
		# FixMe: LEGACY_TFM_TLV_HEADER could be removed when MCUBoot fork is deleted.
		set(LEGACY_TFM_TLV_HEADER ON)
	endif()
endif()

##Set Mbed Crypto compiler flags and variables for crypto service
set(MBEDCRYPTO_C_FLAGS_SERVICES "${CMSE_FLAGS} -D__thumb2__ ${COMMON_COMPILE_FLAGS_STR} -I${CMAKE_CURRENT_LIST_DIR}/platform/ext/common")

#Default TF-M protected storage flags.
#These flags values can be overwritten by setting them in platform/ext/<TARGET_NAME>.cmake
#Documentation about these flags can be found in docs/user_guides/services/tfm_ps_integration_guide.rst
if (NOT DEFINED PS_ENCRYPTION)
	set (PS_ENCRYPTION ON)
endif()

if (NOT DEFINED PS_ROLLBACK_PROTECTION)
	set (PS_ROLLBACK_PROTECTION OFF)
endif()

if (NOT DEFINED PS_CREATE_FLASH_LAYOUT)
	set (PS_CREATE_FLASH_LAYOUT OFF)
endif()

if (NOT DEFINED PS_VALIDATE_METADATA_FROM_FLASH)
	set (PS_VALIDATE_METADATA_FROM_FLASH ON)
endif()

if (NOT DEFINED PS_RAM_FS)
	set (PS_RAM_FS OFF)
endif()

if (NOT DEFINED PS_TEST_NV_COUNTERS)
	if (REGRESSION AND ENABLE_PROTECTED_STORAGE_SERVICE_TESTS)
		set(PS_TEST_NV_COUNTERS ON)
	else()
		set(PS_TEST_NV_COUNTERS OFF)
	endif()
endif()

# The PS NV counter tests depend on the PS test partition to call
# ps_system_prepare().
if (PS_TEST_NV_COUNTERS)
	set(TFM_PARTITION_TEST_PS ON)
	add_definitions(-DTFM_PARTITION_TEST_PS)
endif()

#Default TF-M internal trusted storage flags.
#These flags values can be overwritten by setting them in platform/ext/<TARGET_NAME>.cmake
#Documentation about these flags can be found in the TF-M ITS integration guide
option(ITS_CREATE_FLASH_LAYOUT "Create an empty ITS Flash Layout" OFF)

if (NOT DEFINED ITS_VALIDATE_METADATA_FROM_FLASH)
	set (ITS_VALIDATE_METADATA_FROM_FLASH ON)
endif()

if (NOT DEFINED ITS_RAM_FS)
	set (ITS_RAM_FS OFF)
endif()

if (NOT DEFINED MBEDCRYPTO_DEBUG)
	set(MBEDCRYPTO_DEBUG OFF)
endif()

#Default TF-M initial-attestation service flags.
#Documentation about these flags can be found in docs/user_guides/services/tfm_attestation_integration_guide.rst
if (NOT DEFINED ATTEST_INCLUDE_OPTIONAL_CLAIMS)
	set(ATTEST_INCLUDE_OPTIONAL_CLAIMS ON)
endif()

if (NOT DEFINED ATTEST_INCLUDE_COSE_KEY_ID)
	set(ATTEST_INCLUDE_COSE_KEY_ID OFF)
endif()

if (NOT DEFINED ATTEST_INCLUDE_TEST_CODE)
	if (CMAKE_BUILD_TYPE STREQUAL "debug")
		set(ATTEST_INCLUDE_TEST_CODE ON)
	else()
		set(ATTEST_INCLUDE_TEST_CODE OFF)
	endif()
endif()

if (NOT DEFINED BOOT_DATA_AVAILABLE)
	if (BL2)
		set(BOOT_DATA_AVAILABLE ON)
	else()
		set(BOOT_DATA_AVAILABLE OFF)
	endif()
endif()

if (NOT DEFINED ATTEST_CLAIM_VALUE_CHECK)
	set(ATTEST_CLAIM_VALUE_CHECK OFF)
endif()

##Set mbedTLS compiler flags for BL2 bootloader
set(MBEDCRYPTO_C_FLAGS_BL2 "${CMSE_FLAGS} -D__thumb2__ ${COMMON_COMPILE_FLAGS_STR} -DMBEDTLS_CONFIG_FILE=\\\\\\\"config-rsa.h\\\\\\\" -I${CMAKE_CURRENT_LIST_DIR}/bl2/ext/mcuboot/include")
if (MCUBOOT_SIGNATURE_TYPE STREQUAL "RSA-3072")
	string(APPEND MBEDCRYPTO_C_FLAGS_BL2 " -DMCUBOOT_SIGN_RSA_LEN=3072")
endif()
