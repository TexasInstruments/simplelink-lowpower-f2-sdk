# Copyright (c) 2018, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#This file gathers all stm32l5 hal specific files , CMSIS and Native Driver, application file common to all L5 family soc/board

#hal L5 file
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_uart.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_uart_ex.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rcc.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rcc_ex.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_gpio.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_pwr.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_pwr_ex.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_exti.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_gtzc.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_cortex.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_dma.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_flash.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_flash_ex.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rng.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rng_ex.c")

    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_uart.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_uart_ex.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_cortex.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_dma.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_flash.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_flash_ex.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rcc.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rcc_ex.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_gpio.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_pwr.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_pwr_ex.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_exti.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_gtzc.c")
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal.c")

    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_uart.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_uart_ex.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rcc.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rcc_ex.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_gpio.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_pwr.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_pwr_ex.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_exti.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_gtzc.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_cortex.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_dma.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_flash.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_flash_ex.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rng.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l5xx_hal/Src/stm32l5xx_hal_rng_ex.c")
#secure file
if (NOT DEFINED BUILD_TARGET_CFG)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_CFG (true|false) is undefined!")
elseif(BUILD_TARGET_CFG)
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/secure/target_cfg.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/secure/spm_hal.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/tick.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/template/crypto_keys.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/template/tfm_rotpk.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/template/attest_hal.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/secure/system_stm32l5xx.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/common/template/tfm_initial_attestation_key_material.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/secure/tfm_platform_system.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver" ABSOLUTE)
endif()
#non secure file
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/Templates/system_stm32l5xx.c")
#bl2 file
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/bl2/boot_hal.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/common/template/nv_counters.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/bl2/tfm_low_level_security.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/Device/Source/Templates/system_stm32l5xx.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/tick.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/common/template/crypto_keys.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/common/template/tfm_rotpk.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/common/boot_hal.c")

if (NOT DEFINED BUILD_NATIVE_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_NATIVE_DRIVERS (true|false) is undefined!")
elseif(BUILD_NATIVE_DRIVERS)
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/low_level_rng.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/tick.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/mpu_armv8m_drv.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/mpu_armv8m_drv.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/low_level_rng.c")
    list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver/tick.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/Native_Driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_CMSIS_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_DRIVERS (true|false) is undefined!")
elseif(BUILD_CMSIS_DRIVERS)
  embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver" ABSOLUTE)
  list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver/low_level_com.c")
  list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver/low_level_flash.c")
  list(APPEND ALL_SRC_C_BL2 "${PLATFORM_DIR}/target/stm/stm32l5xx/bl2/low_level_device.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver/low_level_com.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver/low_level_flash.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/stm/stm32l5xx/secure/low_level_device.c")
  list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/stm/stm32l5xx/CMSIS_Driver/low_level_com.c")
endif()
#file used for computing flash addres to update target
set(CONTENT_FOR_BL2_PREPROCESSING
    "#include \"${FLASH_LAYOUT}\"\n\n"
    "enum bl2_image_attributes {\n"
    "\tRE_IMAGE_FLASH_ADDRESS_SECURE = (FLASH_BASE_ADDRESS+FLASH_AREA_0_OFFSET),\n"
    "\tRE_IMAGE_FLASH_ADDRESS_NON_SECURE = (FLASH_BASE_ADDRESS+FLASH_AREA_1_OFFSET),\n"
    "\tRE_BL2_PERSO_ADDRESS = (FLASH_BASE_ADDRESS+FLASH_AREA_BL2_OFFSET),\n"
    "\tRE_BL2_BOOT_ADDRESS = (FLASH_BASE_ADDRESS+FLASH_AREA_BL2_OFFSET),\n"
    "\tRE_BL2_WRP_START = (FLASH_AREA_BL2_OFFSET),\n"
    "\tRE_BL2_WRP_END = (FLASH_AREA_BL2_NOHDP_OFFSET+FLASH_AREA_BL2_NOHDP_SIZE-0x1),\n"
    "\tRE_BL2_HDP_END = (FLASH_AREA_BL2_NOHDP_OFFSET-0x1),\n"
   "\tRE_IMAGE_FLASH_SECURE_UPDATE = (FLASH_BASE_ADDRESS+FLASH_AREA_2_OFFSET),\n"
    "\tRE_IMAGE_FLASH_NON_SECURE_UPDATE = (FLASH_BASE_ADDRESS+FLASH_AREA_3_OFFSET),\n"
    "\tRE_IMAGE_FLASH_UNUSED = (FLASH_BASE_ADDRESS+FLASH_AREA_3_OFFSET+FLASH_AREA_3_SIZE),\n"
    "\tRE_IMAGE_FLASH_SCRATCH = (FLASH_BASE_ADDRESS+FLASH_AREA_SCRATCH_OFFSET),\n"
    "\tRE_IMAGE_FLASH_NV_COUNTERS = (FLASH_BASE_ADDRESS+FLASH_NV_COUNTERS_AREA_OFFSET),\n"
    "\tRE_IMAGE_FLASH_NV_PS = (FLASH_BASE_ADDRESS+FLASH_PS_AREA_OFFSET),\n"
    "\tRE_IMAGE_FLASH_NV_ITS = (FLASH_BASE_ADDRESS+FLASH_ITS_AREA_OFFSET)\n}\;"
)
if (REGRESSION OR CORE_TEST OR IPC_TEST OR PSA_API_TEST)
#the define FLASH_LAYOUT_FOR_TEST
#increase SECURE and NON SECURE IMAGE and modify the  content of  low_level_device.c from BL2 (low_level_flash.c)
set(FLAGS_FOR_BL2_PREPROCESSING
    "#define FLASH_LAYOUT_FOR_TEST\n"
        "#define BL2\n"
     )
add_definitions("-DFLASH_LAYOUT_FOR_TEST")
else()
set(FLAGS_FOR_BL2_PREPROCESSING
    "#define BL2\n"
   )
endif()

set (BL2_FILE_TO_PREPROCESS ${CMAKE_BINARY_DIR}/image_macros_to_preprocess_bl2.c)
string(CONCAT BL2_PREPROCESSING ${FLAGS_FOR_BL2_PREPROCESSING}
       ${CONTENT_FOR_BL2_PREPROCESSING})
file(WRITE ${BL2_FILE_TO_PREPROCESS} ${BL2_PREPROCESSING})
