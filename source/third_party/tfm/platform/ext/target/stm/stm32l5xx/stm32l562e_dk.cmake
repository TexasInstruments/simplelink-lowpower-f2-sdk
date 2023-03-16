# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#This file gathers all stm32l562_dk specific files in the application.

#stm  has a Cortex M33 CPU.
include("Common/CpuM33")
set(PS_CREATE_FLASH_LAYOUT ON)
set(ITS_CREATE_FLASH_LAYOUT ON)
set(ATTEST_INCLUDE_TEST_CODE ON)
set(MCUBOOT_HW_KEY OFF)
set(MBEDCRYPTO_DEBUG OFF)
set(MCUBOOT_IMAGE_NUMBER 2)
set(TFM_ENABLE_IRQ_TEST OFF)
set(TFM_ENABLE_PERIPH_ACCESS_TEST OFF)
set(ITS_RAM_FS ON)
set(PS_RAM_FS ON)
add_definitions("-DSTM32L562xx")

# force RDP level 0 usage
add_definitions("-DTFM_OB_RDP_LEVEL_VALUE=0xAA")
include ("${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx.cmake")
#Specify the location of platform specific build dependencies.

if(COMPILER STREQUAL "GNUARM")
  set(BL2_SCATTER_FILE_NAME "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/gcc/stm32l5xx_bl2.ld")
  set(S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/gcc/tfm_common_s.ld")
  set(NS_SCATTER_FILE_NAME  "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/gcc/stm32l5xx_ns.ld")
  if (DEFINED CMSIS_DIR)
    # not all project defines CMSIS_DIR, only the ones that use it.
    set(RTX_LIB_PATH "${CMSIS_DIR}/RTOS2/RTX/Library/GCC/libRTX_V8MMN.a")
  endif()
else()
  message(FATAL_ERROR "'${COMPILER}' is not supported.")
endif()
if (REGRESSION OR CORE_TEST OR IPC_TEST OR PSA_API_TEST)
  set(FLASH_LAYOUT          "${PLATFORM_DIR}/target/stm/stm32l5xx/boards/stm32l562e_dk/flash_layout_test.h")
else()
  set(FLASH_LAYOUT          "${PLATFORM_DIR}/target/stm/stm32l5xx/boards/stm32l562e_dk/flash_layout.h")
endif()
set(PLATFORM_LINK_INCLUDES "${PLATFORM_DIR}/target/stm/stm32l5xx/boards/stm32l562e_dk" "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/include")

if (BL2)
  set(BL2_LINKER_CONFIG ${BL2_SCATTER_FILE_NAME})
endif()

embedded_include_directories(PATH "${PLATFORM_DIR}/cmsis" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/boards" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Include" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Inc" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/boards/stm32l562e_dk" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/../include" ABSOLUTE)

if (NOT DEFINED BUILD_UART_STDOUT)
  message(FATAL_ERROR "Configuration variable BUILD_UART_STDOUT (true|false) is undefined!")
elseif(BUILD_UART_STDOUT)
  if (NOT DEFINED SECURE_UART1)
    message(FATAL_ERROR "Configuration variable SECURE_UART1 (true|false) is undefined!")
  elseif(SECURE_UART1)
    message(FATAL_ERROR "Configuration SECURE_UART1 TRUE is invalid for this target!")
  endif()
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/uart_stdout.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
  set(BUILD_NATIVE_DRIVERS true)
  set(BUILD_CMSIS_DRIVERS true)
endif()



if (NOT DEFINED BUILD_TIME)
  message(FATAL_ERROR "Configuration variable BUILD_TIME (true|false) is undefined!")
elseif(BUILD_TIME)
  #fix me add a cmsis_driver for L5
endif()

if (NOT DEFINED BUILD_STARTUP)
  message(FATAL_ERROR "Configuration variable BUILD_STARTUP (true|false) is undefined!")
elseif(BUILD_STARTUP)
  list(APPEND ALL_SRC_ASM_S "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/gcc/startup_stm32l562xx_s.S")
  list(APPEND ALL_SRC_ASM_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/gcc/startup_stm32l562xx_ns.S")
  list(APPEND ALL_SRC_ASM_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/gcc/startup_stm32l562xx_bl2.S")
  set_property(SOURCE "${ALL_SRC_ASM_S}"   APPEND
  PROPERTY COMPILE_DEFINITIONS "__STARTUP_CLEAR_BSS_MULTIPLE" "__STARTUP_COPY_MULTIPLE")
  set_property(SOURCE "${ALL_SRC_ASM_NS}"   APPEND
  PROPERTY COMPILE_DEFINITIONS "__STARTUP_CLEAR_BSS_MULTIPLE" "__STARTUP_COPY_MULTIPLE")
endif()

if (NOT DEFINED BUILD_FLASH)
  message(FATAL_ERROR "Configuration variable BUILD_FLASH (true|false) is undefined!")
elseif(BUILD_FLASH)
  #list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver/Driver_Flash.c")
  # As the PS area is going to be in RAM, it is required to set PS_CREATE_FLASH_LAYOUT
  # to be sure the PS service knows that when it starts the PS area does not contain any
  # valid PS flash layout and it needs to create one.
  set(ITS_CREATE_FLASH_LAYOUT ON)
  set(ITS_RAM_FS OFF)
  set(PS_CREATE_FLASH_LAYOUT ON)
  set(PS_RAM_FS OFF)
  embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_TARGET_NV_COUNTERS)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_NV_COUNTERS (true|false) is undefined!")
elseif(BUILD_TARGET_NV_COUNTERS)
  # NOTE: This non-volatile counters implementation is a dummy
  #       implementation. Platform vendors have to implement the
  #       API ONLY if the target has non-volatile counters.
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/template/nv_counters.c")
  set(TARGET_NV_COUNTERS_ENABLE ON)
  # Sets PS_ROLLBACK_PROTECTION flag to compile in the PS services
  # rollback protection code as the target supports nv counters.
  set(PS_ROLLBACK_PROTECTION ON)
endif()

if (NOT BL2)
  message(STATUS "WARNING: BL2 is mandatory on target \"${TARGET_PLATFORM}\" Your choice was override.")
  set(BL2 True)
endif()
if (${MCUBOOT_SIGNATURE_TYPE} STREQUAL "RSA-2048")
  add_definitions(-DMCUBOOT_SIGN_RSA_LEN=2048)
endif()
if (${MCUBOOT_SIGNATURE_TYPE} STREQUAL "RSA-3072")
  add_definitions(-DMCUBOOT_SIGN_RSA_LEN=3072)
endif()
if (NOT ${MCUBOOT_UPGRADE_STRATEGY} STREQUAL "OVERWRITE_ONLY")
  message(WARNING "OVERWRITE ONLY upgrade strategy is preferred on target '${TARGET_PLATFORM}'. Your choice was overriden.")
  set(MCUBOOT_UPGRADE_STRATEGY "OVERWRITE_ONLY")
endif()
#install files reuqired for flashing and performing regression
install(FILES
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/imgtool.py
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/macro_parser.py
        DESTINATION ./scripts/ )
install(FILES
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/imgtool_lib/image.py
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/imgtool_lib/boot_record.py
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/imgtool_lib/keys.py
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/imgtool_lib/version.py
        ${PLATFORM_DIR}/../../bl2/ext/mcuboot/scripts/imgtool_lib/__init__.py
        DESTINATION ./scripts/imgtool_lib )
install(FILES ${PLATFORM_DIR}/target/stm/stm32l5xx/boards/stm32l562e_dk/flash_layout.h
        ${PLATFORM_DIR}/target/stm/stm32l5xx/boards/stm32l562e_dk/region_defs.h
        DESTINATION ./ )
install(FILES ${PLATFORM_DIR}/target/stm/stm32l5xx/boards/scripts/regression.sh
        ${PLATFORM_DIR}/target/stm/stm32l5xx/boards/scripts/TFM_UPDATE.sh
        ${PLATFORM_DIR}/target/stm/stm32l5xx/boards/scripts/postbuild.sh
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        DESTINATION ./ )
