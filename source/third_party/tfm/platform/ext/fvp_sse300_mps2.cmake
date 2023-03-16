#-------------------------------------------------------------------------------
# Copyright (c) 2019-2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#This file gathers all fvp_sse300_mps2 specific files in the application.

#sse300 has a v8.1-M.Main processor
include("Common/CpuArmv81mml")

set(PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

set(FVP_SSE300_DIR ${PLATFORM_DIR}/target/mps2/fvp_sse300)

#Specify the location of platform specific build dependencies.
#FIXME use CMAKE_C_COMPILER_ID here instead?
if(COMPILER STREQUAL "ARMCLANG")
    set (S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/armclang/tfm_common_s.sct")
    set (BL2_SCATTER_FILE_NAME  "${FVP_SSE300_DIR}/device/source/armclang/fvp_sse300_mps2_bl2.sct")
    set (NS_SCATTER_FILE_NAME  "${FVP_SSE300_DIR}/device/source/armclang/fvp_sse300_mps2_ns.sct")
    if (DEFINED CMSIS_DIR)
      # not all project defines CMSIS_DIR, only the ones that use it.
      set (RTX_LIB_PATH "${CMSIS_DIR}/RTOS2/RTX/Library/ARM/RTX_V8MMN.lib")
    endif()
elseif(COMPILER STREQUAL "GNUARM")
    message(FATAL_ERROR "No GNUARM support yet for target '${TARGET_PLATFORM}'.")
elseif(COMPILER STREQUAL "IARARM")
    message(FATAL_ERROR "No IARARM support yet for target '${TARGET_PLATFORM}'.")
else()
    message(FATAL_ERROR "Invalid compiler specified: '${COMPILER}'.")
endif()
set (FLASH_LAYOUT           "${FVP_SSE300_DIR}/partition/flash_layout.h")
set (PLATFORM_LINK_INCLUDES "${FVP_SSE300_DIR}/partition")

if (BL2)
  set (BL2_LINKER_CONFIG ${BL2_SCATTER_FILE_NAME})
  if (${MCUBOOT_UPGRADE_STRATEGY} STREQUAL "RAM_LOADING")
      message(FATAL_ERROR "ERROR: RAM_LOADING upgrade strategy is not supported on target '${TARGET_PLATFORM}'.")
  endif()
  #FixMe: MCUBOOT_SIGN_RSA_LEN can be removed when ROTPK won't be hard coded in platform/ext/common/template/tfm_rotpk.c
  #       instead independently loaded from secure code as a blob.
  if (${MCUBOOT_SIGNATURE_TYPE} STREQUAL "RSA-2048")
      add_definitions(-DMCUBOOT_SIGN_RSA_LEN=2048)
  endif()
  if (${MCUBOOT_SIGNATURE_TYPE} STREQUAL "RSA-3072")
      add_definitions(-DMCUBOOT_SIGN_RSA_LEN=3072)
  endif()
else()
    message(FATAL_ERROR "BL2 is mandatory on target '${TARGET_PLATFORM}'.")
endif()

embedded_include_directories(PATH "${PLATFORM_DIR}/cmsis" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/../include" ABSOLUTE)
embedded_include_directories(PATH "${FVP_SSE300_DIR}" ABSOLUTE)
embedded_include_directories(PATH "${FVP_SSE300_DIR}/partition" ABSOLUTE)
embedded_include_directories(PATH "${FVP_SSE300_DIR}/device/include" ABSOLUTE)
embedded_include_directories(PATH "${FVP_SSE300_DIR}/device/config" ABSOLUTE)
embedded_include_directories(PATH "${FVP_SSE300_DIR}/cmsis_drivers/config" ABSOLUTE)
embedded_include_directories(PATH "${FVP_SSE300_DIR}/native_drivers" ABSOLUTE)

#Gather all source files we need.
if (NOT DEFINED BUILD_CMSIS_CORE)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_CORE (true|false) is undefined!")
elseif(BUILD_CMSIS_CORE)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/device/source/system_core_init.c")
endif()

if (NOT DEFINED BUILD_RETARGET)
  message(FATAL_ERROR "Configuration variable BUILD_RETARGET (true|false) is undefined!")
elseif(BUILD_RETARGET)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/device/source/device_definition.c")
endif()

if (NOT DEFINED BUILD_UART_STDOUT)
  message(FATAL_ERROR "Configuration variable BUILD_UART_STDOUT (true|false) is undefined!")
elseif(BUILD_UART_STDOUT)
  if (NOT DEFINED SECURE_UART1)
    message(FATAL_ERROR "Configuration variable SECURE_UART1 (true|false) is undefined!")
  elseif(SECURE_UART1)
    add_definitions(-DSECURE_UART1)
  endif()
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/uart_stdout.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
  set(BUILD_NATIVE_DRIVERS true)
  set(BUILD_CMSIS_DRIVERS true)
endif()

if (NOT DEFINED BUILD_NATIVE_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_NATIVE_DRIVERS (true|false) is undefined!")
elseif(BUILD_NATIVE_DRIVERS)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/native_drivers/uart_cmsdk_drv.c")

  list(APPEND ALL_SRC_C_S "${FVP_SSE300_DIR}/native_drivers/mpc_sie_drv.c"
                          "${FVP_SSE300_DIR}/native_drivers/ppc_sse300_drv.c")
endif()

if (NOT DEFINED BUILD_TIME)
  message(FATAL_ERROR "Configuration variable BUILD_TIME (true|false) is undefined!")
elseif(BUILD_TIME)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/native_drivers/systimer_armv8-m_drv.c")
endif()

if (NOT DEFINED BUILD_STARTUP)
  message(FATAL_ERROR "Configuration variable BUILD_STARTUP (true|false) is undefined!")
elseif(BUILD_STARTUP)
  if(CMAKE_C_COMPILER_ID STREQUAL "ARMCLANG")
    list(APPEND ALL_SRC_C_S "${FVP_SSE300_DIR}/device/source/startup_fvp_sse300_mps2_s.c")
    list(APPEND ALL_SRC_C_NS "${FVP_SSE300_DIR}/device/source/startup_fvp_sse300_mps2_ns.c")
    list(APPEND ALL_SRC_C_BL2 "${FVP_SSE300_DIR}/device/source/startup_fvp_sse300_mps2_bl2.c")
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNUARM")
    message(FATAL_ERROR "No GNUARM support yet for target '${TARGET_PLATFORM}'.")
  else()
    message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
  endif()
endif()

if (NOT DEFINED BUILD_TARGET_CFG)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_CFG (true|false) is undefined!")
elseif(BUILD_TARGET_CFG)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/target_cfg.c")
  list(APPEND ALL_SRC_C_S "${FVP_SSE300_DIR}/spm_hal.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/template/attest_hal.c")
  list(APPEND ALL_SRC_C_S "${FVP_SSE300_DIR}/native_drivers/mpu_armv8m_drv.c")
  if (TFM_PARTITION_PLATFORM)
    list(APPEND ALL_SRC_C_S "${FVP_SSE300_DIR}/services/src/tfm_platform_system.c")
  endif()
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/tfm_platform.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_PLAT_TEST)
  message(FATAL_ERROR "Configuration variable BUILD_PLAT_TEST (true|false) is undefined!")
elseif(BUILD_PLAT_TEST)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/plat_test.c")
endif()

if (NOT DEFINED BUILD_BOOT_HAL)
  message(FATAL_ERROR "Configuration variable BUILD_BOOT_HAL (true|false) is undefined!")
elseif(BUILD_BOOT_HAL)

  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/boot_hal.c")
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/boot_hal.c")
endif()

if (NOT DEFINED BUILD_TARGET_HARDWARE_KEYS)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_HARDWARE_KEYS (true|false) is undefined!")
elseif(BUILD_TARGET_HARDWARE_KEYS)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/template/tfm_initial_attestation_key_material.c")
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/template/tfm_rotpk.c")
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/template/crypto_keys.c")
endif()

if (NOT DEFINED BUILD_TARGET_NV_COUNTERS)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_NV_COUNTERS (true|false) is undefined!")
elseif(BUILD_TARGET_NV_COUNTERS)
  # NOTE: This non-volatile counters implementation is a dummy
  #       implementation. Platform vendors have to implement the
  #       API ONLY if the target has non-volatile counters.
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/template/nv_counters.c")
  set(TARGET_NV_COUNTERS_ENABLE ON)
  # Sets PS_ROLLBACK_PROTECTION flag to compile in the PS services
  # rollback protection code as the target supports nv counters.
  set(PS_ROLLBACK_PROTECTION ON)
endif()

if (NOT DEFINED BUILD_CMSIS_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_DRIVERS (true|false) is undefined!")
elseif(BUILD_CMSIS_DRIVERS)
  list(APPEND ALL_SRC_C_S "${FVP_SSE300_DIR}/cmsis_drivers/Driver_MPC.c"
                          "${FVP_SSE300_DIR}/cmsis_drivers/Driver_Flash.c"
                          "${FVP_SSE300_DIR}/cmsis_drivers/Driver_SSE300_PPC.c")
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/cmsis_drivers/Driver_USART.c")
  embedded_include_directories(PATH "${FVP_SSE300_DIR}/cmsis_drivers" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_FLASH)
  message(FATAL_ERROR "Configuration variable BUILD_FLASH (true|false) is undefined!")
elseif(BUILD_FLASH)
  list(APPEND ALL_SRC_C "${FVP_SSE300_DIR}/cmsis_drivers/Driver_Flash.c")
  # There is no real flash memory for code on MPS2 fvp. Instead data SRAMs are used for code
  # storage: SSRAM2 and SSRAM3. The Driver_Flash driver just emulates a flash interface and behaviour on
  # top of the SRAM memory.
  # As the PS area is going to be in RAM, it is required to set PS_CREATE_FLASH_LAYOUT
  # to be sure the PS service knows that when it starts the PS area does not contain any
  # valid PS flash layout and it needs to create one. The same for ITS.
  set(PS_CREATE_FLASH_LAYOUT ON)
  set(ITS_CREATE_FLASH_LAYOUT ON)
  embedded_include_directories(PATH "${FVP_SSE300_DIR}/cmsis_drivers" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()
