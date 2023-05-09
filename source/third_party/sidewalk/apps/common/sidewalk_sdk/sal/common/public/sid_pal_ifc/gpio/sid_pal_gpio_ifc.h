/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_PAL_GPIO_IFC_H
#define SID_PAL_GPIO_IFC_H

/** @file
 *
 * @defgroup sid_pal_lib_gpio sid gpio interface
 * @{
 * @ingroup sid_pal_ifc
 *
 * @details  Provides gpio interface to be implemented by platform
 */
#include <sid_error.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief enum for configuring GPIO pull
 */
typedef enum {
    SID_PAL_GPIO_PULL_NONE,             /* Configure GPIO as no pull */
    SID_PAL_GPIO_PULL_UP,               /* Configure GPIO as pull up */
    SID_PAL_GPIO_PULL_DOWN,             /* Configure GPIO as pull down */
} sid_pal_gpio_pull_t;

/**
 * @brief enum for configuring GPIO output type
 */
typedef enum {
    SID_PAL_GPIO_OUTPUT_PUSH_PULL,      /* Configure GPIO as push pull */
    SID_PAL_GPIO_OUTPUT_OPEN_DRAIN,     /* Configure GPIO as open drain output */
} sid_pal_gpio_output_t;

/**
 * @brief enum for configuring GPIO input buffer connect state
 */
typedef enum {
    SID_PAL_GPIO_INPUT_CONNECT,         /* Configure GPIO as input connected */
    SID_PAL_GPIO_INPUT_DISCONNECT,      /* Configure GPIO as input disconnected */
} sid_pal_gpio_input_t;

/**
 * @brief enum for configuring direction of GPIO.
 */
typedef enum {
    SID_PAL_GPIO_DIRECTION_INPUT,       /* Configure GPIO as input */
    SID_PAL_GPIO_DIRECTION_OUTPUT,      /* Configure GPIO as output */
} sid_pal_gpio_direction_t;

/**
 * @brief   GPIO pin interrupt config types.
 */
typedef enum {
    SID_PAL_GPIO_IRQ_TRIGGER_NONE,        /* Disable interrupt */
    SID_PAL_GPIO_IRQ_TRIGGER_RISING,      /* Trigger interrupt on rising edge */
    SID_PAL_GPIO_IRQ_TRIGGER_FALLING,     /* Trigger interrupt on falling edge */
    SID_PAL_GPIO_IRQ_TRIGGER_EDGE,        /* Trigger interrupt on both edges */
    SID_PAL_GPIO_IRQ_TRIGGER_LOW,         /* Low level triggered interrupt */
    SID_PAL_GPIO_IRQ_TRIGGER_HIGH,        /* High level triggered interrupt */
} sid_pal_gpio_irq_trigger_t;

/**
 * @brief GPIO Pin IRQ handler.
 *
 * @note The callback to be called when the configured transition occurs.
 *
 * @param[in] gpio_number       The logical GPIO number.
 */
typedef void(* sid_pal_gpio_irq_handler_t)(uint32_t gpio_number, void * callback_arg);

/**
 * @brief   sid_pal_gpio_set_direction is used to set the direction of the GPIO.
 *
 * @param[in]  gpio_number    The logical GPIO number.
 * @param[in]  direction      Direction of GPIO.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_set_direction(uint32_t gpio_number,
                                           sid_pal_gpio_direction_t direction);

/**
 * @brief   sid_pal_gpio_read is used to read data from GPIO pin.
 *
 * @param[in]  gpio_number    The logical GPIO number.
 * @param[out] value          Value read from the GPIO.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_read(uint32_t gpio_number, uint8_t *value);

/**
 * @brief   sid_pal_gpio_write is used to write data to the GPIO.
 *
 * @param[in]  gpio_number    The logical GPIO number.
 * @param[out] value          Value to write to GPIO.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_write(uint32_t gpio_number, uint8_t value);

/**
 * @brief   sid_pal_gpio_toggle is used to toggle the GPIO.
 *
 * @param[in]  gpio_number    The logical GPIO number.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_toggle(uint32_t gpio_number);

/**
 * @brief   sid_pal_gpio_set_irq is used to generate an interrupt based on the configuration
 *          and set callbacket function.
 *
 * @param[in] gpio_number      The logical GPIO number.
 * @param[in] irq_trigger      The interrupt config types to generate an interrupt
 *                             based on the configuration.
 * @param[in] gpio_irq_handler The callback function to be called on interrupt.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_set_irq(uint32_t gpio_number, sid_pal_gpio_irq_trigger_t irq_trigger,
                                     sid_pal_gpio_irq_handler_t gpio_irq_handler, void *callback_arg);

/**
 * @brief   sid_pal_gpio_irq_enable is used to disable an interrupt.
 *
 * @param[in] gpio_number      The logical GPIO number to enable interrupts.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_irq_enable(uint32_t gpio_number);

/**
 * @brief   sid_pal_gpio_irq_disable is used to disable an interrupt.
 *
 * @param[in] gpio_number      The logical GPIO number to disable interrupts.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_irq_disable(uint32_t gpio_number);

/**
 * @brief   sid_pal_gpio_input_mode is used to configure input mode of GPIO.
 *
 * @param[in] gpio_number      The logical GPIO number to operate on.
 * @param[in] mode             The input mode to set.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_input_mode(uint32_t gpio_number, sid_pal_gpio_input_t mode);

/**
 * @brief   sid_pal_gpio_output_mode is used to configure output mode of GPIO.
 *
 * @param[in] gpio_number      The logical GPIO number to operate on.
 * @param[in] mode             The output mode to set.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_output_mode(uint32_t gpio_number, sid_pal_gpio_output_t mode);

/**
 * @brief   sid_pal_gpio_pull_mode is used to configure pull type of GPIO.
 *
 * @param[in] gpio_number      The logical GPIO number to operate on.
 * @param[in] pull             The pull mode to set.
 *
 * @retval SID_ERROR_NONE in case of success.
 */
sid_error_t sid_pal_gpio_pull_mode(uint32_t gpio_number, sid_pal_gpio_pull_t pull);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
