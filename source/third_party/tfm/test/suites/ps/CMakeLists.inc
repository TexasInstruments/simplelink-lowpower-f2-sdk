#-------------------------------------------------------------------------------
# Copyright (c) 2017-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#Definitions to compile the "protected storage test" module.
#This file assumes it will be included from a project specific cmakefile, and
#will not create a library or executable.
#Inputs:
#	TFM_ROOT_DIR - root directory of the TF-M repo.
#
#Outputs:
#	Will modify include directories to make the source compile.
#	ALL_SRC_C: C source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
#	ALL_SRC_CXX: C++ source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
#	ALL_SRC_ASM: assembly source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
#	Include directories will be modified by using the include_directories() commands as needed.

#Get the current directory where this file is located.
set(PROTECTED_STORAGE_TEST_DIR ${CMAKE_CURRENT_LIST_DIR})
if(NOT DEFINED TFM_ROOT_DIR)
	message(FATAL_ERROR "Please set TFM_ROOT_DIR before including this file.")
endif()

if (NOT DEFINED ENABLE_PROTECTED_STORAGE_SERVICE_TESTS)
	message(FATAL_ERROR "Incomplete build configuration: ENABLE_PROTECTED_STORAGE_SERVICE_TESTS is undefined. ")
elseif (ENABLE_PROTECTED_STORAGE_SERVICE_TESTS)
	list(APPEND ALL_SRC_C_NS "${PROTECTED_STORAGE_TEST_DIR}/non_secure/ns_test_helpers.c"
				 "${PROTECTED_STORAGE_TEST_DIR}/non_secure/psa_ps_ns_interface_testsuite.c")

	list(APPEND ALL_SRC_C_S "${PROTECTED_STORAGE_TEST_DIR}/secure/psa_ps_s_interface_testsuite.c"
				"${PROTECTED_STORAGE_TEST_DIR}/secure/psa_ps_s_reliability_testsuite.c")

	if (PS_ENCRYPTION AND PS_ROLLBACK_PROTECTION AND PS_TEST_NV_COUNTERS)
		list(APPEND ALL_SRC_C_S "${PROTECTED_STORAGE_TEST_DIR}/secure/ps_rollback_protection_testsuite.c"
					"${PROTECTED_STORAGE_TEST_DIR}/secure/nv_counters/test_ps_nv_counters.c")
		set_property(SOURCE ${ALL_SRC_C_S} APPEND PROPERTY COMPILE_DEFINITIONS PS_TEST_NV_COUNTERS)
	endif()

	if (NOT DEFINED TFM_NS_CLIENT_IDENTIFICATION)
		message(FATAL_ERROR "Incomplete build configuration: TFM_NS_CLIENT_IDENTIFICATION is undefined.")
	elseif (TFM_NS_CLIENT_IDENTIFICATION)
		set_property(SOURCE ${ALL_SRC_C_NS} APPEND PROPERTY COMPILE_DEFINITIONS TFM_NS_CLIENT_IDENTIFICATION)
	endif()

	#Setting include directories
	embedded_include_directories(PATH ${TFM_ROOT_DIR} ABSOLUTE)
	embedded_include_directories(PATH ${TFM_ROOT_DIR}/test/interface/include ABSOLUTE)
	embedded_include_directories(PATH ${TFM_ROOT_DIR}/interface/include ABSOLUTE)
	embedded_include_directories(PATH ${TFM_ROOT_DIR}/secure_fw/core/include ABSOLUTE)
endif()
