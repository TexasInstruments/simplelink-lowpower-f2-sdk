#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
# Copyright 2023 NXP. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file is exported to NS side during CMake installation phase and renamed
# to CMakeLists.txt. It instructs how to build a platform on non-secture side.
# The structure and sources list are fully platform specific.

add_library(platform_ns)

target_sources(platform_ns
    PUBLIC
        Device/Source/startup_lpcxpresso55s69.c
    PRIVATE
        board.c
        hardware_init.c
        clock_config.c
        pin_mux.c
        Driver_USART.c
        Native_Driver/system_LPC55S69_cm33_core0.c
        Native_Driver/drivers/fsl_clock.c
        Native_Driver/drivers/fsl_power.c
        Native_Driver/drivers/fsl_reset.c
        Native_Driver/components/lists/fsl_component_generic_list.c
        Native_Driver/components/serial_manager/fsl_component_serial_manager.c
        Native_Driver/components/serial_manager/fsl_component_serial_port_uart.c
        Native_Driver/components/uart/fsl_adapter_usart.c
        Native_Driver/drivers/fsl_casper.c
        Native_Driver/drivers/fsl_common.c
        Native_Driver/drivers/fsl_common_arm.c
        Native_Driver/drivers/fsl_ctimer.c
        Native_Driver/drivers/fsl_flexcomm.c
        Native_Driver/drivers/fsl_gpio.c
        Native_Driver/drivers/fsl_hashcrypt.c
        Native_Driver/drivers/fsl_iap.c
        Native_Driver/drivers/fsl_rng.c
        Native_Driver/drivers/fsl_usart.c
        Native_Driver/utilities/fsl_assert.c
        Native_Driver/utilities/fsl_sbrk.c
        Native_Driver/utilities/debug_console/fsl_debug_console.c
        Native_Driver/utilities/str/fsl_str.c
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:plat_test.c>
)


target_include_directories(platform_ns
    PUBLIC
        include
        ext/cmsis
        ext/common
        ext/driver
        Native_Driver
        Device/Include
        Device/Config
        Native_Driver/drivers
        Native_Driver/components/lists
        Native_Driver/components/serial_manager
        Native_Driver/components/uart
        Native_Driver/utilities/debug_console
        Native_Driver/utilities/str
        partition
)

# Include region_defs.h and flash_layout.h
target_include_directories(platform_region_defs
    INTERFACE
        partition
)
