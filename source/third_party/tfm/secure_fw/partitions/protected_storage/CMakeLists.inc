#-------------------------------------------------------------------------------
# Copyright (c) 2017-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#Definitions to compile the "protected_storage" module.
#This file assumes it will be included from a project specific cmakefile, and
#will not create a library or executable.
#Inputs:
#	TFM_ROOT_DIR		- root directory of the TF-M repository.
#Outputs:
#	Will modify include directories to make the source compile.
#	ALL_SRC_C: C source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
#	ALL_SRC_CXX: C++ source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
#	ALL_SRC_ASM: assembly source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
#	Include directories will be modified by using the include_directories() commands as needed.

#Get the current directory where this file is located.
set(PROTECTED_STORAGE_DIR ${CMAKE_CURRENT_LIST_DIR})

#Check input variables
if (NOT DEFINED TFM_ROOT_DIR)
	message(FATAL_ERROR "Please set TFM_ROOT_DIR before including this file.")
endif()

if (NOT DEFINED PS_ENCRYPTION)
	message(FATAL_ERROR "Incomplete build configuration: PS_ENCRYPTION is undefined. ")
endif()

if (NOT DEFINED PS_ROLLBACK_PROTECTION)
	message(FATAL_ERROR "Incomplete build configuration: PS_ROLLBACK_PROTECTION is undefined.")
endif()

if (NOT DEFINED PS_CREATE_FLASH_LAYOUT)
	message(FATAL_ERROR "Incomplete build configuration: PS_CREATE_FLASH_LAYOUT is undefined. ")
endif()

if (NOT DEFINED PS_VALIDATE_METADATA_FROM_FLASH)
	message(FATAL_ERROR "Incomplete build configuration: PS_VALIDATE_METADATA_FROM_FLASH is undefined. ")
endif()

if (NOT DEFINED PS_RAM_FS)
	message(FATAL_ERROR "Incomplete build configuration: PS_RAM_FS is undefined. ")
endif()

if (NOT DEFINED PS_TEST_NV_COUNTERS)
	message(FATAL_ERROR "Incomplete build configuration: PS_TEST_NV_COUNTERS is undefined.")
endif()

set (PROTECTED_STORAGE_C_SRC
	"${PROTECTED_STORAGE_DIR}/tfm_ps_secure_api.c"
	"${PROTECTED_STORAGE_DIR}/tfm_ps_req_mngr.c"
	"${PROTECTED_STORAGE_DIR}/tfm_protected_storage.c"
	"${PROTECTED_STORAGE_DIR}/ps_object_system.c"
	"${PROTECTED_STORAGE_DIR}/ps_object_table.c"
	"${PROTECTED_STORAGE_DIR}/ps_utils.c"
)

if (PS_ENCRYPTION)
	list(APPEND PROTECTED_STORAGE_C_SRC
		"${PROTECTED_STORAGE_DIR}/crypto/ps_crypto_interface.c"
		"${PROTECTED_STORAGE_DIR}/ps_encrypted_object.c"
	)
	set_property(SOURCE ${PROTECTED_STORAGE_C_SRC} APPEND PROPERTY COMPILE_DEFINITIONS PS_ENCRYPTION)

	if (PS_ROLLBACK_PROTECTION)
		# Only build the NV counters implementation if the PS_TEST_NV_COUNTERS
		# flag is off. When this flag is on, a virtual implementation of the PS
		# NV counters interface is used instead. Full documentation for this
		# flag can be found in the PS Integration Guide.
		if (NOT PS_TEST_NV_COUNTERS)
			list(APPEND PROTECTED_STORAGE_C_SRC
				"${PROTECTED_STORAGE_DIR}/nv_counters/ps_nv_counters.c")
		endif()
		set_property(SOURCE ${PROTECTED_STORAGE_C_SRC} APPEND PROPERTY COMPILE_DEFINITIONS PS_ROLLBACK_PROTECTION)
	endif()
endif()

if (PS_VALIDATE_METADATA_FROM_FLASH)
	set_property(SOURCE ${PROTECTED_STORAGE_C_SRC} APPEND PROPERTY COMPILE_DEFINITIONS PS_VALIDATE_METADATA_FROM_FLASH)
endif()

if (PS_CREATE_FLASH_LAYOUT)
	set_property(SOURCE ${PROTECTED_STORAGE_C_SRC} APPEND PROPERTY COMPILE_DEFINITIONS PS_CREATE_FLASH_LAYOUT)
endif()

if (PS_RAM_FS)
	set_property(SOURCE ${PROTECTED_STORAGE_C_SRC} APPEND PROPERTY COMPILE_DEFINITIONS PS_RAM_FS)
endif()

#Append all our source files to global lists.
list(APPEND ALL_SRC_C ${PROTECTED_STORAGE_C_SRC})
unset(PROTECTED_STORAGE_C_SRC)

#Inform the user about PS service features selected based on the PS service cmake flags
message("The PS service compile configuration is as follows:")
message("- PS_ENCRYPTION: " ${PS_ENCRYPTION})
if (PS_ENCRYPTION)
	message("- PS_ROLLBACK_PROTECTION: " ${PS_ROLLBACK_PROTECTION})
else()
	message("- PS_ROLLBACK_PROTECTION: N/A")
endif()
message("- PS_VALIDATE_METADATA_FROM_FLASH: " ${PS_VALIDATE_METADATA_FROM_FLASH})
message("- PS_CREATE_FLASH_LAYOUT: " ${PS_CREATE_FLASH_LAYOUT})
message("- PS_RAM_FS: " ${PS_RAM_FS})
message("- PS_TEST_NV_COUNTERS: " ${PS_TEST_NV_COUNTERS})

#Setting include directories
embedded_include_directories(PATH ${TFM_ROOT_DIR} ABSOLUTE)
embedded_include_directories(PATH ${TFM_ROOT_DIR}/interface/include ABSOLUTE)
embedded_include_directories(PATH ${TFM_ROOT_DIR}/secure_fw/spm ABSOLUTE)
embedded_include_directories(PATH ${TFM_ROOT_DIR}/secure_fw/core/include ABSOLUTE)
embedded_include_directories(PATH ${TFM_ROOT_DIR}/platform/ext/common ABSOLUTE)
embedded_include_directories(PATH ${TFM_ROOT_DIR}/platform/ext/driver ABSOLUTE)
set(BUILD_CMSIS_CORE Off)
set(BUILD_RETARGET Off)
set(BUILD_NATIVE_DRIVERS Off)
set(BUILD_STARTUP Off)
set(BUILD_TARGET_CFG Off)
set(BUILD_TARGET_HARDWARE_KEYS Off)
set(BUILD_TARGET_NV_COUNTERS Off)
set(BUILD_CMSIS_DRIVERS Off)
set(BUILD_TIME Off)
set(BUILD_UART_STDOUT Off)
set(BUILD_FLASH Off)
set(BUILD_PLAT_TEST Off)
if(NOT DEFINED PLATFORM_CMAKE_FILE)
	message (FATAL_ERROR "Platform specific CMake is not defined. Please set PLATFORM_CMAKE_FILE.")
elseif(NOT EXISTS ${PLATFORM_CMAKE_FILE})
	message (FATAL_ERROR "Platform specific CMake \"${PLATFORM_CMAKE_FILE}\" file does not exist. Please fix value of PLATFORM_CMAKE_FILE.")
else()
	include(${PLATFORM_CMAKE_FILE})
endif()
