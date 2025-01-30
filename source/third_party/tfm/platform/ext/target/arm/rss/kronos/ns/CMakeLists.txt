#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

include(common.cmake)

target_sources(platform_ns
    PRIVATE
        native_drivers/uart_pl011_drv.c
        cmsis_drivers/Driver_USART.c
        device/host_device_definition.c
)
