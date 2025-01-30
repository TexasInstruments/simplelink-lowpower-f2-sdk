#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)

set(NUCLEO_L552ZE_Q_DIR ${CMAKE_CURRENT_LIST_DIR})
set(STM_COMMON_DIR ${NUCLEO_L552ZE_Q_DIR}/../common)

#def for stm32l5xx
include(${STM_COMMON_DIR}/stm32l5xx/CMakeLists.txt) 
#========================= Platform defs ===============================#

# Specify the location of platform specific build dependencies.
target_sources(tfm_s
    PRIVATE
        ${STM_COMMON_DIR}/stm32l5xx/Device/Source/startup_stm32l5xx_s.c
)
# cpuarch.cmake is used to set things that related to the platform that are both
install(FILES
    ${TARGET_PLATFORM_PATH}/cpuarch.cmake
    DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(FILES
    ${STM_COMMON_DIR}/stm32l5xx/Device/Source/startup_stm32l5xx_ns.c
    DESTINATION ${INSTALL_PLATFORM_NS_DIR}/Device/Source
)

install(DIRECTORY
    ${TARGET_PLATFORM_PATH}/ns/
    DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(DIRECTORY
    ${TARGET_PLATFORM_PATH}/include
    DESTINATION ${INSTALL_PLATFORM_NS_DIR}
)

install(FILES
    ${TARGET_PLATFORM_PATH}/accelerator/crypto_accelerator_config.h
    DESTINATION ${INSTALL_PLATFORM_NS_DIR}/include
)

install(FILES
    ${TARGET_PLATFORM_PATH}/partition/flash_layout.h
    ${TARGET_PLATFORM_PATH}/partition/region_defs.h
    DESTINATION ${INSTALL_PLATFORM_NS_DIR}/partition
)


if(BL2)
    target_sources(bl2
        PRIVATE
            ${STM_COMMON_DIR}/stm32l5xx/Device/Source/startup_stm32l5xx_bl2.c
    )
endif()

install(FILES ${TARGET_PLATFORM_PATH}/partition/flash_layout.h
              ${TARGET_PLATFORM_PATH}/partition/region_defs.h
        DESTINATION  ${CMAKE_INSTALL_PREFIX} )
set (BL2_FILE_TO_PREPROCESS ${CMAKE_CURRENT_BINARY_DIR}/image_macros_to_preprocess_bl2.c)
file(WRITE ${BL2_FILE_TO_PREPROCESS} ${BL2_PREPROCESSING})
install(FILES ${BL2_FILE_TO_PREPROCESS}
        DESTINATION ${CMAKE_INSTALL_PREFIX} )
