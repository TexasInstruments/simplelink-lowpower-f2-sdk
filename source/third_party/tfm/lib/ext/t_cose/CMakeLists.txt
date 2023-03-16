#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.7)

#Tell cmake where our modules can be found
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../../cmake)

#Some project global settings
set(T_COSE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(TFM_ROOT_DIR "${T_COSE_DIR}/../../.." ABSOLUTE)

#Include common stuff to control cmake.
include("Common/BuildSys")

#Start an embedded project.
embedded_project_start(CONFIG "${TFM_ROOT_DIR}/configs/ConfigDefault.cmake")
project(tfm_t_cose LANGUAGES C)
embedded_project_fixup()

if (NOT DEFINED ATTEST_INCLUDE_TEST_CODE)
	message(FATAL_ERROR "Incomplete build configuration: ATTEST_INCLUDE_TEST_CODE is undefined. ")
endif()

#Append all our source files to global lists.
list(APPEND ALL_SRC_C_SIGN
	"${T_COSE_DIR}/src/t_cose_util.c"
	"${T_COSE_DIR}/src/t_cose_parameters.c"
	"${T_COSE_DIR}/crypto_adapters/t_cose_psa_crypto.c"
	)

if (SYMMETRIC_INITIAL_ATTESTATION)
	list(APPEND T_COSE_COMPILE_TIME_CONFIG "T_COSE_DISABLE_SIGN1")

	list(APPEND ALL_SRC_C_SIGN "${T_COSE_DIR}/src/t_cose_mac0_sign.c")
else()
	list(APPEND T_COSE_COMPILE_TIME_CONFIG "T_COSE_DISABLE_MAC0")

	list(APPEND ALL_SRC_C_SIGN "${T_COSE_DIR}/src/t_cose_sign1_sign.c")
endif()

list(APPEND ALL_SRC_C_VERIFY
	"${T_COSE_DIR}/src/t_cose_util.c"
	"${T_COSE_DIR}/src/t_cose_parameters.c"
	"${T_COSE_DIR}/crypto_adapters/t_cose_psa_crypto.c"
	)

if (SYMMETRIC_INITIAL_ATTESTATION)
	list(APPEND ALL_SRC_C_VERIFY "${T_COSE_DIR}/src/t_cose_mac0_verify.c")
else()
	list(APPEND ALL_SRC_C_VERIFY "${T_COSE_DIR}/src/t_cose_sign1_verify.c")
endif()

if (ENABLE_T_COSE_TESTS)
	list(APPEND ALL_SRC_C_TEST
		"${T_COSE_DIR}/test/run_tests.c"
		"${T_COSE_DIR}/test/t_cose_make_psa_test_key.c"
		"${T_COSE_DIR}/test/t_cose_make_test_messages.c"
		"${T_COSE_DIR}/test/t_cose_sign_verify_test.c"
		"${T_COSE_DIR}/test/t_cose_test.c"
		)
endif()

#Setting include directories
embedded_include_directories(PATH ${T_COSE_DIR}/inc ABSOLUTE)
embedded_include_directories(PATH ${T_COSE_DIR}/../qcbor/inc ABSOLUTE)
embedded_target_include_directories(TARGET tfm_t_cose_sign   PATH ${T_COSE_DIR}/src ABSOLUTE APPEND)
embedded_target_include_directories(TARGET tfm_t_cose_verify PATH ${T_COSE_DIR}/src ABSOLUTE APPEND)
if (ENABLE_T_COSE_TESTS)
	embedded_target_include_directories(TARGET tfm_t_cose_test PATH ${T_COSE_DIR}/src ABSOLUTE APPEND)
endif()

#Specify what we build (t_cose_sign and t_cose_verify build as an object library)
add_library(tfm_t_cose_sign   OBJECT ${ALL_SRC_C_SIGN})
add_library(tfm_t_cose_verify OBJECT ${ALL_SRC_C_VERIFY})
if (ENABLE_T_COSE_TESTS)
	add_library(tfm_t_cose_test OBJECT ${ALL_SRC_C_TEST})
endif()

#Setting target specific defines

list(APPEND T_COSE_COMPILE_TIME_CONFIG
	"T_COSE_USE_PSA_CRYPTO"
	"T_COSE_DISABLE_CONTENT_TYPE"
	"T_COSE_USE_PSA_CRYPTO_FROM_TFM"
	)

if (NOT SYMMETRIC_INITIAL_ATTESTATION)
	list(APPEND T_COSE_COMPILE_TIME_CONFIG
		"T_COSE_DISABLE_ES384"
		"T_COSE_DISABLE_ES512"
		"T_COSE_DISABLE_SIGN_VERIFY_TESTS"
		)
endif()

if (NOT ATTEST_INCLUDE_TEST_CODE)
	list(APPEND T_COSE_COMPILE_TIME_CONFIG "T_COSE_DISABLE_SHORT_CIRCUIT_SIGN")
endif()

embedded_set_target_compile_defines(TARGET tfm_t_cose_sign   LANGUAGE C DEFINES ${T_COSE_COMPILE_TIME_CONFIG} APPEND)
embedded_set_target_compile_defines(TARGET tfm_t_cose_verify LANGUAGE C DEFINES ${T_COSE_COMPILE_TIME_CONFIG} APPEND)
if (ENABLE_T_COSE_TESTS)
	embedded_set_target_compile_defines(TARGET tfm_t_cose_test LANGUAGE C DEFINES ${T_COSE_COMPILE_TIME_CONFIG} APPEND)
endif()

if (DEFINED CMSE_FLAGS)
	embedded_set_target_compile_flags(TARGET tfm_t_cose_sign   LANGUAGE C APPEND FLAGS ${CMSE_FLAGS})
	embedded_set_target_compile_flags(TARGET tfm_t_cose_verify LANGUAGE C APPEND FLAGS ${CMSE_FLAGS})
	embedded_set_target_compile_flags(TARGET tfm_t_cose_test   LANGUAGE C APPEND FLAGS ${CMSE_FLAGS})
endif()

#Set common compiler flags
config_setting_shared_compiler_flags(tfm_t_cose_sign)
config_setting_shared_compiler_flags(tfm_t_cose_verify)
if (ENABLE_T_COSE_TESTS)
	config_setting_shared_compiler_flags(tfm_t_cose_test)
endif()

embedded_project_end(tfm_t_cose_sign)
embedded_project_end(tfm_t_cose_verify)
if (ENABLE_T_COSE_TESTS)
	embedded_project_end(tfm_t_cose_test)
endif()
