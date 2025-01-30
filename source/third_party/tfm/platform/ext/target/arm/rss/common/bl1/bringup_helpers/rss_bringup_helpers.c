/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */


#include "rss_bringup_helpers.h"

#include "rss_bringup_helper_gpio_pin_ids.h"
#include "platform_base_address.h"
#include "boot_hal.h"
#include "flash_layout.h"

#include <stdbool.h>
#include <stdint.h>

static bool check_gpio_pin(uint32_t pin_id)
{
    if (pin_id < 8) {
        return (*((uint32_t*)GPIO0_CMSDK_BASE_S) >> pin_id) & 0b1;
    } else if (pin_id < 16) {
        return (*((uint32_t*)GPIO1_CMSDK_BASE_S) >> pin_id) & 0b1;
    } else {
        return 0;
    }
}

static void boot_address(struct boot_arm_vector_table *vt)
{
    /* Clang at O0, stores variables on the stack with SP relative addressing.
     * When manually set the SP then the place of reset vector is lost.
     * Static variables are stored in 'data' or 'bss' section, change of SP has
     * no effect on them.
     */
    static struct boot_arm_vector_table *vt_cpy;

    vt_cpy = vt;

    /* Restore the Main Stack Pointer Limit register's reset value
     * before passing execution to runtime firmware to make the
     * bootloader transparent to it.
     */
    __set_MSPLIM(0);

    __set_MSP(vt_cpy->msp);
    __DSB();
    __ISB();

    boot_jump_to_next_image(vt_cpy->reset);
}

static void boot_from_vm0(void)
{
    boot_address((void *)VM0_BASE_S);
}

static void boot_from_qspi(void)
{
    boot_address((void *)FLASH_BASE_ADDRESS);
}

static void boot_from_side_band(void)
{
    /* TODO implement this */
}

void rss_run_bringup_helpers_if_requested(void)
{
    #ifdef RSS_BRINGUP_BOOT_FROM_VM0_PIN
    if (check_gpio_pin(RSS_BRINGUP_BOOT_FROM_VM0_PIN)) {
        boot_from_vm0();
    }
    #endif /* RSS_BRINGUP_BOOT_FROM_VM0_PIN */

    #ifdef RSS_BRINGUP_BOOT_FROM_QSPI_PIN
    if (check_gpio_pin(RSS_BRINGUP_BOOT_FROM_QSPI_PIN)) {
        boot_from_qspi();
    }
    #endif /* RSS_BRINGUP_BOOT_FROM_QSPI_PIN */

    #ifdef RSS_BRINGUP_BOOT_FROM_SIDE_BAND_PIN
    if (check_gpio_pin(RSS_BRINGUP_BOOT_FROM_SIDE_BAND_PIN)) {
        boot_from_side_band();
    }
    #endif /* RSS_BRINGUP_BOOT_FROM_SIDE_BAND_PIN */
}
