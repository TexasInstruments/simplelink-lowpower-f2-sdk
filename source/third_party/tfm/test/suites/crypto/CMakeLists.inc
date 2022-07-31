#-------------------------------------------------------------------------------
# Copyright (c) 2018-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#Definitions to compile the "Crypto test" module.
#This file assumes it will be included from a project specific cmakefile, and
#will not create a library or executable.
#Inputs:
# TFM_ROOT_DIR - root directory of the TF-M repo.
#
#Outputs:
# Will modify include directories to make the source compile.
# ALL_SRC_C: C source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
# ALL_SRC_CXX: C++ source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
# ALL_SRC_ASM: assembly source files to be compiled will be added to this list. This shall be added to your add_executable or add_library command.
# Include directories will be modified by using the include_directories() commands as needed.

#Get the current directory where this file is located.
set(CRYPTO_TEST_DIR ${CMAKE_CURRENT_LIST_DIR})
if(NOT DEFINED TFM_ROOT_DIR)
  message(FATAL_ERROR "Please set TFM_ROOT_DIR before including this file.")
endif()

if (NOT DEFINED ENABLE_CRYPTO_SERVICE_TESTS)
  message(FATAL_ERROR "Incomplete build configuration: ENABLE_CRYPTO_SERVICE_TESTS is undefined. ")
elseif (ENABLE_CRYPTO_SERVICE_TESTS)
  list(APPEND ALL_SRC_C_S "${CRYPTO_TEST_DIR}/secure/crypto_sec_interface_testsuite.c"
                          "${CRYPTO_TEST_DIR}/crypto_tests_common.c")
  list(APPEND ALL_SRC_C_NS "${CRYPTO_TEST_DIR}/non_secure/crypto_ns_interface_testsuite.c"
                           "${CRYPTO_TEST_DIR}/crypto_tests_common.c")

  #Enable the test cases by default
  if (NOT DEFINED TFM_CRYPTO_TEST_ALG_CBC)
    set(TFM_CRYPTO_TEST_ALG_CBC ON)
  endif()
  if (NOT DEFINED TFM_CRYPTO_TEST_ALG_CCM)
    set(TFM_CRYPTO_TEST_ALG_CCM ON)
  endif()
  if (NOT DEFINED TFM_CRYPTO_TEST_ALG_CFB)
    set(TFM_CRYPTO_TEST_ALG_CFB ON)
  endif()
  if (NOT DEFINED TFM_CRYPTO_TEST_ALG_CTR)
    set(TFM_CRYPTO_TEST_ALG_CTR ON)
  endif()
  if (NOT DEFINED TFM_CRYPTO_TEST_ALG_GCM)
    set(TFM_CRYPTO_TEST_ALG_GCM ON)
  endif()
  if (NOT DEFINED TFM_CRYPTO_TEST_ALG_SHA_512)
    set(TFM_CRYPTO_TEST_ALG_SHA_512 ON)
  endif()
  if (NOT DEFINED TFM_CRYPTO_TEST_HKDF)
    set(TFM_CRYPTO_TEST_HKDF ON)
  endif()

  if (TFM_CRYPTO_TEST_ALG_CBC)
    add_definitions(-DTFM_CRYPTO_TEST_ALG_CBC)
  endif()
  if (TFM_CRYPTO_TEST_ALG_CCM)
    add_definitions(-DTFM_CRYPTO_TEST_ALG_CCM)
  endif()
  if (TFM_CRYPTO_TEST_ALG_CFB)
    add_definitions(-DTFM_CRYPTO_TEST_ALG_CFB)
  endif()
  if (TFM_CRYPTO_TEST_ALG_CTR)
    add_definitions(-DTFM_CRYPTO_TEST_ALG_CTR)
  endif()
  if (TFM_CRYPTO_TEST_ALG_GCM)
    add_definitions(-DTFM_CRYPTO_TEST_ALG_GCM)
  endif()
  if (TFM_CRYPTO_TEST_ALG_SHA_512)
    add_definitions(-DTFM_CRYPTO_TEST_ALG_SHA_512)
  endif()
  if (TFM_CRYPTO_TEST_HKDF)
    add_definitions(-DTFM_CRYPTO_TEST_HKDF)
  endif()

  #Setting include directories
  embedded_include_directories(PATH ${TFM_ROOT_DIR} ABSOLUTE)
  embedded_include_directories(PATH ${TFM_ROOT_DIR}/interface/include ABSOLUTE)
  embedded_include_directories(PATH ${TFM_ROOT_DIR}/secure_fw/core/include ABSOLUTE)

endif()
