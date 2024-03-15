/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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

static bool initCalled = false;

// HW interrupt structure for I/O interrupt handler
static HwiP_Struct gpioHwi;

// Link to config values defined by sysconfig
extern GPIO_Config GPIO_config;
extern const uint_least8_t GPIO_pinLowerBound;
extern const uint_least8_t GPIO_pinUpperBound;

/*
 *  ======== GPIO_intEnabled ========
 */
static inline bool GPIO_intEnabled(uint32_t index)
{
    return ((HWREG(IOC_BASE + IOC_O_IOCFG0 + (4 * index)) & IOC_IOCFG0_EDGE_IRQ_EN) ? true : false);
}

/*
 *  ======== GPIO_hwiIntFxn ========
 *  Hwi function that processes GPIO interrupts.
 */
void GPIO_hwiIntFxn(uintptr_t arg)
{
    uint32_t flagIndex;
    uint32_t eventMask = HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0);

    // Clear the interrupt mask
    HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) = eventMask;

    while (eventMask)
    {
        // MASK_TO_PIN only detects the highest set bit
        flagIndex = GPIO_MASK_TO_PIN(eventMask);

        // So it's safe to use PIN_TO_MASK to clear that bit
        eventMask &= ~GPIO_PIN_TO_MASK(flagIndex);

        if (GPIO_config.callbacks[flagIndex] != NULL && GPIO_intEnabled(flagIndex))
        {
            /*  Only invoke callback if this pin has interrupt enabled in IOC
             *  This is because event-flags (checked above) can be generated independently of interrupts
             */
            GPIO_config.callbacks[flagIndex](flagIndex);
        }
    }
}

/*
 *  ======== GPIO_init ========
 */
void GPIO_init(void)
{
    uintptr_t key;
    unsigned int i;
    HwiP_Params hwiParams;
    uint32_t tempPinConfigs[32];
    uint32_t enableMask = 0x0;

    key = HwiP_disable();

    if (initCalled)
    {
        HwiP_restore(key);
        return;
    }
    initCalled = true;
    HwiP_restore(key);

    // This is safe even if Power_init has already been called.
    Power_init();

    // Set Power dependecies & constraints
    Power_setDependency(PowerCC26XX_PERIPH_GPIO);

    // Setup HWI handler
    HwiP_Params_init(&hwiParams);
    hwiParams.priority = GPIO_config.intPriority;
    HwiP_construct(&gpioHwi, INT_AON_GPIO_EDGE, GPIO_hwiIntFxn, &hwiParams);

    // Note: pinUpperBound is inclusive, so we use <= instead of just <
    for (i = GPIO_pinLowerBound; i <= GPIO_pinUpperBound; i++)
    {
        uint32_t pinConfig = GPIO_config.configs[i];

        /* Mask off the bits containing non-IOC configuration values */
        tempPinConfigs[i] = pinConfig & GPIOCC26XX_CFG_IOC_M;

        if (!(pinConfig & GPIOCC26XX_CFG_PIN_IS_INPUT_INTERNAL))
        {
            enableMask |= 1 << i;
            GPIO_write(i, pinConfig & GPIO_CFG_OUT_HIGH ? 1 : 0);
        }
    }

    HWREG(GPIO_BASE + GPIO_O_DOE31_0) = enableMask;

    /* If the user has configured a pin as the external clock pin in CCFG, we
     * should respect this. Add the mux directly into the temporary pin config.
     */
    if (CCFGRead_SCLK_LF_OPTION() == CCFGREAD_SCLK_LF_OPTION_EXTERNAL_LF)
    {
        uint32_t externalLfPin = CCFGRead_EXT_LF_CLK_DIO();
        tempPinConfigs[externalLfPin] |= IOC_IOCFG0_PORT_ID_AON_CLK32K;
    }

    /* Apply all the masked values directly to IOC
     * pinUpperBound is inclusive, so we need to add 1 to get the full range
     * Multiply by 4 because each pin config and IOC register is 4 bytes wide
     */
    memcpy((void *)(IOC_BASE + IOC_O_IOCFG0 + (4 * GPIO_pinLowerBound)),
           (void *)&tempPinConfigs[GPIO_pinLowerBound],
           ((GPIO_pinUpperBound + 1) - GPIO_pinLowerBound) * 4);

    // Setup wakeup source to wake up from standby (use MCU_WU1)
    HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) = (HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) &
                                                          (~AON_EVENT_MCUWUSEL_WU1_EV_M)) |
                                                    AON_EVENT_MCUWUSEL_WU1_EV_PAD;
}
