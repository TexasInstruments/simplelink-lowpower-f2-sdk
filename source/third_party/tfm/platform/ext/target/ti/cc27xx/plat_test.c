/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "platform_retarget.h"
#include "platform_retarget_dev.h"
#include "tfm_plat_test.h"

void tfm_plat_test_wait_user_button_pressed(void)
{
}

void tfm_plat_test_wait_user_button_released(void)
{
}

uint32_t tfm_plat_test_get_led_status(void)
{
    return  1;
}

void tfm_plat_test_set_led_status(uint32_t status)
{
}

uint32_t tfm_plat_test_get_userled_mask(void)
{
    return 0;
}

void tfm_plat_test_secure_timer_start(void)
{
}

void tfm_plat_test_secure_timer_stop(void)
{
}

void tfm_plat_test_non_secure_timer_start(void)
{
}

void tfm_plat_test_non_secure_timer_stop(void)
{
}
