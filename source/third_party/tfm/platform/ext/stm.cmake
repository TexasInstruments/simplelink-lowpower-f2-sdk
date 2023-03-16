#-------------------------------------------------------------------------------
# Copyright (c) 2020, STMicroelectronics - All Rights Reserved
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------
#This file gathers all target of stm.

set(PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

if(${TARGET_PLATFORM} STREQUAL "STM_DISCO_L562QE")
	include("${PLATFORM_DIR}/target/stm/stm32l5xx/stm32l562e_dk.cmake")
elseif(${TARGET_PLATFORM} STREQUAL "STM_NUCLEO_L552ZE_Q")
	include("${PLATFORM_DIR}/target/stm/stm32l5xx/nucleo_l552ze_q.cmake")
else()
	message(FATAL_ERROR "ERROR: Target \"${TARGET_PLATFORM}\" is not supported.")
endif()