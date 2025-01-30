/*
 * Copyright (c) 2015-2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <ti/drivers/dpl/HwiP.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_ioc.h)
#include DeviceFamily_constructPath(inc/hw_gpio.h)
#include DeviceFamily_constructPath(inc/hw_aon_event.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(driverlib/gpio.h)
#include DeviceFamily_constructPath(driverlib/ccfgread.h)

// Link to config values defined by sysconfig
extern GPIO_Config GPIO_config;

/*
 *  ======== GPIO_clearInt ========
 */
void GPIO_clearInt(uint_least8_t index)
{
    if (index != GPIO_INVALID_INDEX)
    {
        GPIO_clearEventDio(index);
    }
}

/*
 *  ======== GPIO_disableInt ========
 */
void GPIO_disableInt(uint_least8_t index)
{
    /* Interrupt enable is bit 18. Here we mask 0x4 out of byte 2 to disable
     * interrupts. Note we cannot just read-write the whole register.
     * See the IOCFG comment in setConfig().
     */
    if (index != GPIO_INVALID_INDEX)
    {
        uint32_t iocfgRegAddr = IOC_BASE + IOC_O_IOCFG0 + (4 * index) + 2;
        HWREGB(iocfgRegAddr)  = HWREGB(iocfgRegAddr) & ~0x4;
    }
}

/*
 *  ======== GPIO_enableInt ========
 */
void GPIO_enableInt(uint_least8_t index)
{
    /* Interrupt enable is bit 18. Here we or 0x4 into byte 2 to enable
     * interrupts. Note we cannot just read-write the whole register.
     * See the IOCFG comment in setConfig().
     */
    if (index != GPIO_INVALID_INDEX)
    {
        uint32_t iocfgRegAddr = IOC_BASE + IOC_O_IOCFG0 + (4 * index) + 2;
        HWREGB(iocfgRegAddr)  = HWREGB(iocfgRegAddr) | 0x4;
    }
}

/*
 *  ======== GPIO_read ========
 */
uint_fast8_t GPIO_read(uint_least8_t index)
{
    return GPIO_readDio(index);
}

/*
 *  ======== GPIO_setConfig ========
 */
int_fast16_t GPIO_setConfig(uint_least8_t index, GPIO_PinConfig pinConfig)
{
    return GPIO_setConfigAndMux(index, pinConfig, GPIO_MUX_GPIO);
}

/*
 *  ======== GPIO_setInterruptConfig ========
 */
void GPIO_setInterruptConfig(uint_least8_t index, GPIO_PinConfig config)
{
    uintptr_t key;

    if (index != GPIO_INVALID_INDEX)
    {
        uint32_t iocfgRegAddr = IOC_BASE + IOC_O_IOCFG0 + (4 * index) + 2;

        /* Shift down and mask away all non-interrupt configuration */
        uint8_t maskedConfig = (config >> 16) & 0x7;

        key = HwiP_disable();

        /* Mask out current interrupt config and apply the new one */
        uint8_t currentRegisterConfig = HWREGB(iocfgRegAddr) & 0xF8;
        HWREGB(iocfgRegAddr)          = currentRegisterConfig | maskedConfig;
        HwiP_restore(key);
    }
}

/*
 *  ======== GPIO_getConfig ========
 */
void GPIO_getConfig(uint_least8_t index, GPIO_PinConfig *pinConfig)
{
    uint32_t iocfgRegAddr = IOC_BASE + IOC_O_IOCFG0 + (4 * index);

    /* Mask off the bits reserved for non-IOC configuration values.
     * The non-IOC configuration values will be written further below.
     */
    uint32_t configValue = HWREG(iocfgRegAddr) & GPIOCC26XX_CFG_IOC_M;

    if (GPIO_getOutputEnableDio(index))
    {
        /* We use XOR here because if INVERT and HIGH are both true, the SW setting is LOW */
        if (GPIO_read(index) ^ ((configValue & GPIO_CFG_INVERT_ON_INTERNAL) != 0))
        {
            configValue |= GPIO_CFG_OUT_HIGH;
        }
    }
    else
    {
        configValue |= GPIO_getOutputEnableDio(index) ? GPIOCC26XX_CFG_PIN_IS_OUTPUT_INTERNAL
                                                      : GPIOCC26XX_CFG_PIN_IS_INPUT_INTERNAL;
    }

    *pinConfig = configValue;
}

/*
 *  ======== GPIO_getMux ========
 */
uint32_t GPIO_getMux(uint_least8_t index)
{
    uint32_t iocfgRegAddr = IOC_BASE + IOC_O_IOCFG0 + (4 * index);
    return HWREG(iocfgRegAddr) & 0xFF;
}

/*
 *  ======== GPIO_setConfigAndMux ========
 */
int_fast16_t GPIO_setConfigAndMux(uint_least8_t index, GPIO_PinConfig pinConfig, uint32_t mux)
{
    uintptr_t key;

    if (index == GPIO_INVALID_INDEX)
    {
        return GPIO_STATUS_ERROR;
    }

    uint32_t iocfgRegAddr   = IOC_BASE + IOC_O_IOCFG0 + (4 * index);
    uint32_t previousConfig = HWREG(iocfgRegAddr);

    /* Note: Do not change this to check PIN_IS_OUTPUT, because that is 0x0 */
    uint32_t pinWillBeOutput = !(pinConfig & GPIOCC26XX_CFG_PIN_IS_INPUT_INTERNAL);

    /* Special configurations are stored in the lowest 8 bits and need to be removed
     * We can make choices based on these values, but must not write them to hardware */
    GPIO_PinConfig tmpConfig = pinConfig & GPIOCC26XX_CFG_IOC_M;

    if ((previousConfig & 0xFF) != mux)
    {
        /* If we're changing the mux to GPIO, then we need to change the output
         * configuration before changing the mux, to ensure that the correct
         * configuration is ready when we change the mux. Otherwise a glitch
         * might occur.
         */
        if (mux == GPIO_MUX_GPIO)
        {
            if (pinWillBeOutput)
            {
                GPIO_write(index, pinConfig & GPIO_CFG_OUT_HIGH ? 1 : 0);
            }
            GPIO_setOutputEnableDio(index, pinWillBeOutput ? GPIO_OUTPUT_ENABLE : GPIO_OUTPUT_DISABLE);
        }

        /* If we're updating mux as well, we can write the whole register */
        HWREG(iocfgRegAddr) = tmpConfig | mux;
    }
    else
    {
        /*
         * Writes to the first byte of the IOCFG register will cause a glitch
         * on the internal IO line. To avoid this, we only want to write
         * the upper 24-bits of the IOCFG register when updating the configuration
         * bits. We do this 1 byte at a time.
         */
        key                      = HwiP_disable();
        HWREGB(iocfgRegAddr + 1) = (uint8_t)(tmpConfig >> 8);
        HWREGB(iocfgRegAddr + 2) = (uint8_t)(tmpConfig >> 16);
        HWREGB(iocfgRegAddr + 3) = (uint8_t)(tmpConfig >> 24);
        HwiP_restore(key);

        /* The output configuration needs to be changed after changing the IOCFG
         * register, to ensure that the output is only enabled/disabled for the
         * desired IO configuration. For example if the configuration is changed
         * from an input to an open drain output, then we don't want to enable
         * the output before the IO has been configured to open drain, since
         * driving the signal high might have some undesired side effect.
         * Another example is if the IO is changed from a high output to an
         * input with a pull-up, then to avoid glitches, we want the pull-up to
         * be enabled before we disable the output.
         *
         * If this pin is being configured to an output, set the new output
         * value. It's important to do this before we change from INPUT to
         * OUTPUT if applicable. If we're already an output this is fine, and if
         * we're input changing to input this statement will not execute.
         */
        if (pinWillBeOutput)
        {
            GPIO_write(index, pinConfig & GPIO_CFG_OUT_HIGH ? 1 : 0);
        }

        GPIO_setOutputEnableDio(index, pinWillBeOutput ? GPIO_OUTPUT_ENABLE : GPIO_OUTPUT_DISABLE);
    }

    return GPIO_STATUS_SUCCESS;
}

/*
 *  ======== GPIO_toggle ========
 */
void GPIO_toggle(uint_least8_t index)
{
    if (index != GPIO_INVALID_INDEX)
    {
        GPIO_toggleDio(index);
    }
}

/*
 *  ======== GPIO_write ========
 */
void GPIO_write(uint_least8_t index, unsigned int value)
{
    if (index != GPIO_INVALID_INDEX)
    {
        GPIO_writeDio(index, value);
    }
}
