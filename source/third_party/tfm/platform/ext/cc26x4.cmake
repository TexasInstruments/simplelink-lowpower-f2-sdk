#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022 Texas Instruments Incorporated
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include("Common/CpuM33F")

set(PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

set(CC26X4_DIR ${PLATFORM_DIR}/target/ti/cc26x4)

add_definitions(-DDeviceFamily_CC26X4)

#Specify the location of platform specific build dependencies.
#FIXME use CMAKE_C_COMPILER_ID here instead?
if(COMPILER STREQUAL "ARMCLANG")
    set (BL2_SCATTER_FILE_NAME "${CC26X4_DIR}/armclang/cc26x4_bl2.sct")
    set (S_SCATTER_FILE_NAME   "${CC26X4_DIR}/armclang/cc26x4_s.sct")
    set (NS_SCATTER_FILE_NAME  "${CC26X4_DIR}/armclang/cc26x4_ns.sct")
    if (DEFINED CMSIS_DIR)
      # not all project defines CMSIS_DIR, only the ones that use it.
      set (RTX_LIB_PATH "${CMSIS_DIR}/RTOS2/RTX/Library/ARM/RTX_V8MMF.lib")
    endif()
elseif(COMPILER STREQUAL "GNUARM")
if(DEFINED REMOTE_GEN_DIR)
    set (S_SCATTER_FILE_NAME   "${REMOTE_GEN_DIR}/platform/ext/common/gcc/tfm_common_s.ld")
else()
    set (S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/gcc/tfm_common_s.ld")
endif()
    set (BL2_SCATTER_FILE_NAME "${CC26X4_DIR}/gcc/cc26x4_bl2.ld")
    set (NS_SCATTER_FILE_NAME  "${CC26X4_DIR}/gcc/cc26x4_ns.ld")
    if (DEFINED CMSIS_DIR)
      # not all project defines CMSIS_DIR, only the ones that use it.
      set (RTX_LIB_PATH "${CMSIS_DIR}/RTOS2/RTX/Library/GCC/libRTX_V8MMFN.a")
    endif()
elseif(COMPILER STREQUAL "TICLANGARM")
    set (BL2_SCATTER_FILE_NAME "${CC26X4_DIR}/armclang/cc26x4_bl2.sct")
    set (S_SCATTER_FILE_NAME   "${CC26X4_DIR}/armclang/cc26x4_s.cmd")
    set (NS_SCATTER_FILE_NAME  "${CC26X4_DIR}/armclang/cc26x4_ns.sct")
    if (DEFINED CMSIS_DIR)
      # not all project defines CMSIS_DIR, only the ones that use it.
      set (RTX_LIB_PATH "${CMSIS_DIR}/RTOS2/RTX/Library/ARM/RTX_V8MMF.lib")
    endif()
else()
    message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
endif()
#set (FLASH_LAYOUT           "${CC26X4_DIR}/partition/flash_layout.h")
set (PLATFORM_LINK_INCLUDES "${CC26X4_DIR}/partition/")

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
endif()

#Remote generation directory
if (DEFINED REMOTE_GEN_DIR)
    set (FLASH_LAYOUT "${REMOTE_GEN_DIR}/platform/ext/target/ti/cc26x4/partition/flash_layout.h")
    embedded_include_directories(PATH "${REMOTE_GEN_DIR}" ABSOLUTE)
    embedded_include_directories(PATH "${REMOTE_GEN_DIR}/platform/ext/target/ti/cc26x4/partition" ABSOLUTE)
    embedded_include_directories(PATH "${REMOTE_GEN_DIR}/secure_fw/spm/model_ipc" ABSOLUTE)
    embedded_include_directories(PATH "${REMOTE_GEN_DIR}/interface/include" ABSOLUTE)
else ()
    set (FLASH_LAYOUT "${CC26X4_DIR}/partition/flash_layout.h")
endif()

embedded_include_directories(PATH "${PLATFORM_DIR}/cmsis" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/ti" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/../include" ABSOLUTE)
embedded_include_directories(PATH "${CC26X4_DIR}" ABSOLUTE)
embedded_include_directories(PATH "${CC26X4_DIR}/cmsis_core" ABSOLUTE)
embedded_include_directories(PATH "${CC26X4_DIR}/retarget" ABSOLUTE)
embedded_include_directories(PATH "${CC26X4_DIR}/native_drivers" ABSOLUTE)
embedded_include_directories(PATH "${CC26X4_DIR}/partition" ABSOLUTE)
embedded_include_directories(PATH ${SOURCE_REPO} ABSOLUTE)

#Gather all source files we need.
if (NOT DEFINED BUILD_CMSIS_CORE)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_CORE (true|false) is undefined!")
elseif(BUILD_CMSIS_CORE)
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/cmsis_core/system_cc26x4.c")
endif()

if (NOT DEFINED BUILD_RETARGET)
  message(FATAL_ERROR "Configuration variable BUILD_RETARGET (true|false) is undefined!")
elseif(BUILD_RETARGET)
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/retarget/platform_retarget_dev.c")
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
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/native_drivers/arm_uart_drv.c")
endif()

if (NOT DEFINED BUILD_TIME)
  message(FATAL_ERROR "Configuration variable BUILD_TIME (true|false) is undefined!")
elseif(BUILD_TIME)
#  message(FATAL_ERROR "Configuration variable BUILD_TIME (true|false) is not supported!")
endif()

if (NOT DEFINED BUILD_STARTUP)
  message(FATAL_ERROR "Configuration variable BUILD_STARTUP (true|false) is undefined!")
elseif(BUILD_STARTUP)
  if(CMAKE_C_COMPILER_ID STREQUAL "ARMCLANG")
    list(APPEND ALL_SRC_ASM_S "${CC26X4_DIR}/armclang/startup_cc26x4_s.s")
    list(APPEND ALL_SRC_ASM_NS "${CC26X4_DIR}/armclang/startup_cc26x4_ns.s")
    list(APPEND ALL_SRC_ASM_BL2 "${CC26X4_DIR}/armclang/startup_cc26x4_bl2.s")
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNUARM")
    list(APPEND ALL_SRC_ASM_S "${CC26X4_DIR}/gcc/startup_cc26x4_s.S")
    list(APPEND ALL_SRC_ASM_NS "${CC26X4_DIR}/gcc/startup_cc26x4_ns.S")
    list(APPEND ALL_SRC_ASM_BL2 "${CC26X4_DIR}/gcc/startup_cc26x4_bl2.S")
    set_property(SOURCE "${ALL_SRC_ASM_S}" "${ALL_SRC_ASM_NS}" "${ALL_SRC_ASM_BL2}" APPEND
      PROPERTY COMPILE_DEFINITIONS "__STARTUP_CLEAR_BSS_MULTIPLE" "__STARTUP_COPY_MULTIPLE")
  elseif(CMAKE_C_COMPILER_ID STREQUAL "TICLANGARM")
    list(APPEND ALL_SRC_ASM_S "${CC26X4_DIR}/armclang/startup_cc26x4_s.S")
    list(APPEND ALL_SRC_ASM_NS "${CC26X4_DIR}/armclang/startup_cc26x4_ns.s")
    list(APPEND ALL_SRC_ASM_BL2 "${CC26X4_DIR}/armclang/startup_cc26x4_bl2.s")
  else()
    message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
  endif()
endif()

if (NOT DEFINED BUILD_TARGET_CFG)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_CFG (true|false) is undefined!")
elseif(BUILD_TARGET_CFG)
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/target_cfg.c")
  list(APPEND ALL_SRC_C_S "${CC26X4_DIR}/spm_hal.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/ti/cc26x4/attest_hal.c")
  list(APPEND ALL_SRC_C_S "${CC26X4_DIR}/native_drivers/mpu_armv8m_drv.c")
  if (TFM_PARTITION_PLATFORM)
    list(APPEND ALL_SRC_C_S "${CC26X4_DIR}/services/src/tfm_platform_system.c")
  endif()
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/tfm_platform.c")
  list(APPEND ALL_SRC_C_S "${CC26X4_DIR}/platform_svc.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
  # Include directory for driverlib SetupTrimDevice()
  embedded_include_directories(PATH "${SOURCE_REPO}/ti/devices/cc13x4_cc26x4/driverlib/" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_PLAT_TEST)
  message(FATAL_ERROR "Configuration variable BUILD_PLAT_TEST (true|false) is undefined!")
elseif(BUILD_PLAT_TEST)
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/plat_test.c")
endif()

if (NOT DEFINED BUILD_BOOT_HAL)
  message(FATAL_ERROR "Configuration variable BUILD_BOOT_HAL (true|false) is undefined!")
elseif(BUILD_BOOT_HAL)
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/boot_hal.c")
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
  set (PS_ROLLBACK_PROTECTION ON)
endif()

if (NOT DEFINED BUILD_CMSIS_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_DRIVERS (true|false) is undefined!")
elseif(BUILD_CMSIS_DRIVERS)
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/cmsis_drivers/Driver_USART.c")
  embedded_include_directories(PATH "${CC26X4_DIR}/cmsis_drivers" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_FLASH)
  message(FATAL_ERROR "Configuration variable BUILD_FLASH (true|false) is undefined!")
elseif(BUILD_FLASH)
  list(APPEND ALL_SRC_C "${KERNEL_REPO}/tfm/dpl/HwiP_tfm.c")
  list(APPEND ALL_SRC_C "${CC26X4_DIR}/cmsis_drivers/Driver_Flash.c")
  set(ITS_CREATE_FLASH_LAYOUT ON)
  set(ITS_RAM_FS OFF)
  # PS_CREATE_FLASH_LAYOUT must be ON for TF-M regression test and has no effect if TFM_PARTITION_PROTECTED_STORAGE is OFF
  set(PS_CREATE_FLASH_LAYOUT ON)
  set(PS_RAM_FS OFF)
  embedded_include_directories(PATH "${CC26X4_DIR}/cmsis_drivers" ABSOLUTE)
  # Include directory for driverlib flash functions
  embedded_include_directories(PATH "${SOURCE_REPO}/ti/devices/cc13x4_cc26x4/driverlib/" ABSOLUTE)
endif()
