/******************************************************************************

 @file  gpio.c

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_gpio_ifc.h>

#include <ti/drivers/GPIO.h>
#include <ti_drivers_config.h>

#include <stddef.h>
#include <stdbool.h>

extern GPIO_PinConfig gpioPinConfigs[31];

#define TI_GPIO_CHECK(gpio)                             \
    do {                                                \
            if (!sid_pal_gpio_pin_check(gpio)) {        \
                return SID_ERROR_OUT_OF_RESOURCES;  \
            }                                           \
    } while(0)

static bool sid_pal_gpio_pin_check(uint32_t gpio_number)
{
    if ((gpio_number < GPIO_pinLowerBound) || (gpio_number > GPIO_pinUpperBound)) {
        return false;
    }
    GPIO_init();

    return true;
}

sid_error_t sid_pal_gpio_set_direction(uint32_t gpio_number,
                                           sid_pal_gpio_direction_t direction)
{
    GPIO_PinConfig pinCfg;
    TI_GPIO_CHECK(gpio_number);
    GPIO_getConfig((uint8_t)gpio_number, &pinCfg);

    if (direction == SID_PAL_GPIO_DIRECTION_INPUT) {
        // clear output configuration
        pinCfg = pinCfg & ~GPIO_CFG_OUTPUT_OPEN_DRAIN_INTERNAL;
        GPIO_setConfig((uint8_t)gpio_number, pinCfg);
        // enable input direction
        pinCfg = pinCfg | GPIO_CFG_INPUT_INTERNAL;
        GPIO_setConfig((uint8_t)gpio_number, pinCfg);
    }
    else { // SID_PAL_GPIO_DIRECTION_OUTPUT
        // enable output direction
        pinCfg = pinCfg & ~GPIO_CFG_INPUT_INTERNAL;
        pinCfg = pinCfg | GPIO_CFG_OUTPUT_INTERNAL;
        GPIO_setConfig((uint8_t)gpio_number, pinCfg);
    }
    gpioPinConfigs[(uint8_t)gpio_number] = pinCfg;

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_input_mode(uint32_t gpio_number, sid_pal_gpio_input_t mode)
{
    GPIO_PinConfig pinCfg;
    TI_GPIO_CHECK(gpio_number);
    GPIO_getConfig((uint8_t)gpio_number, &pinCfg);

    // clear input mode
    pinCfg = pinCfg & ~GPIO_CFG_INPUT_INTERNAL;
    GPIO_setConfig((uint8_t)gpio_number, pinCfg);

    // configure input mode
    if (mode == SID_PAL_GPIO_INPUT_CONNECT) {
        pinCfg = pinCfg | GPIO_CFG_INPUT_INTERNAL;
    }
    else {
        pinCfg = pinCfg | GPIO_CFG_NO_DIR_INTERNAL;
    }
    GPIO_setConfig((uint8_t)gpio_number, pinCfg);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_output_mode(uint32_t gpio_number, sid_pal_gpio_output_t mode)
{
    GPIO_PinConfig pinCfg;
    TI_GPIO_CHECK(gpio_number);
    GPIO_getConfig((uint8_t)gpio_number, &pinCfg);

    // clear output mode
    pinCfg = pinCfg & ~GPIO_CFG_OUT_OPEN_SOURCE_INTERNAL;
    GPIO_setConfig((uint8_t)gpio_number, pinCfg);

    // configure output mode
    if (mode == SID_PAL_GPIO_OUTPUT_OPEN_DRAIN) {
        pinCfg = pinCfg | GPIO_CFG_OUTPUT_OPEN_DRAIN_INTERNAL;
        GPIO_setConfig((uint8_t)gpio_number, pinCfg);
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_pull_mode(uint32_t gpio_number, sid_pal_gpio_pull_t pull)
{
    GPIO_PinConfig pinCfg;
    TI_GPIO_CHECK(gpio_number);
    GPIO_getConfig((uint8_t)gpio_number, &pinCfg);
    uint8_t direction = gpioPinConfigs[(uint8_t)gpio_number] & 0xFF;
    pinCfg |= direction;

    // clear input pull mode
    pinCfg = pinCfg & ~GPIO_CFG_PULL_NONE_INTERNAL;
    GPIO_setConfig((uint8_t)gpio_number, pinCfg);

    // configure input pull mode
    if (pull == SID_PAL_GPIO_PULL_NONE) {
        pinCfg = pinCfg | GPIO_CFG_PULL_NONE_INTERNAL;
    }
    else if (pull == SID_PAL_GPIO_PULL_UP) {
        pinCfg = pinCfg | GPIO_CFG_PULL_UP_INTERNAL;
    }
    else { //SID_PAL_GPIO_PULL_DOWN
        pinCfg = pinCfg | GPIO_CFG_PULL_DOWN_INTERNAL;
    }
    GPIO_setConfig((uint8_t)gpio_number, pinCfg);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_read(uint32_t gpio_number, uint8_t *value)
{
    *value = GPIO_read((uint8_t)gpio_number);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_write(uint32_t gpio_number, uint8_t value)
{
    GPIO_write((uint8_t)gpio_number, value);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_toggle(uint32_t gpio_number)
{
    GPIO_toggle((uint8_t)gpio_number);

    return SID_ERROR_NONE;
}

// TODO: RSLIGHTS-647 Implement argument passing in ti_sid_pal_gpio irq
sid_error_t sid_pal_gpio_set_irq(uint32_t gpio_number, sid_pal_gpio_irq_trigger_t irq_trigger,
                                     sid_pal_gpio_irq_handler_t gpio_callback, void * callback_arg)
{
    GPIO_PinConfig pinCfg;
    TI_GPIO_CHECK(gpio_number);
    GPIO_getConfig((uint8_t)gpio_number, &pinCfg);
    uint8_t direction = gpioPinConfigs[(uint8_t)gpio_number] & 0xFF;
    pinCfg |= direction;
    GPIO_PinConfig updateMask = GPIO_CFG_IN_INT_NONE;

    // clear irq trigger
    pinCfg = pinCfg & ~GPIO_CFG_IN_INT_BOTH_EDGES;
    // configure irq trigger
    switch (irq_trigger) {
        case SID_PAL_GPIO_IRQ_TRIGGER_NONE:
            updateMask = GPIO_CFG_IN_INT_NONE;
            break;
        case SID_PAL_GPIO_IRQ_TRIGGER_RISING:
            updateMask = GPIO_CFG_IN_INT_RISING;
            break;
        case SID_PAL_GPIO_IRQ_TRIGGER_FALLING:
            updateMask = GPIO_CFG_IN_INT_FALLING;;
            break;
        case SID_PAL_GPIO_IRQ_TRIGGER_EDGE:
            updateMask = GPIO_CFG_IN_INT_BOTH_EDGES;
            break;
        case SID_PAL_GPIO_IRQ_TRIGGER_LOW:
        case SID_PAL_GPIO_IRQ_TRIGGER_HIGH:
            return SID_ERROR_NOSUPPORT;
    }
    pinCfg = pinCfg | updateMask;
    GPIO_setConfig((uint8_t)gpio_number, pinCfg);
    GPIO_setCallback((uint8_t)gpio_number, (GPIO_CallbackFxn)gpio_callback);
    GPIO_enableInt((uint8_t)gpio_number);

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number)
{
    // irq is enabled by sid_pal_gpio_set_irq()
    return SID_ERROR_NONE;
}

sid_error_t sid_pal_gpio_irq_disable(uint32_t gpio_number)
{
    TI_GPIO_CHECK(gpio_number);
    GPIO_disableInt((uint8_t)gpio_number);

    return SID_ERROR_NONE;
}
