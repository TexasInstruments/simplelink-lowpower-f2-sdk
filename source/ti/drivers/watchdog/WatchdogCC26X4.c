/*
 * Copyright (c) 2022, Texas Instruments Incorporated
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
#include <stdlib.h>

#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>

#include <ti/drivers/watchdog/WatchdogCC26X4.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_aon_pmctl.h)
#include DeviceFamily_constructPath(driverlib/ccfgread.h)

/* Function prototypes */
void WatchdogCC26X4_clear(Watchdog_Handle handle);
void WatchdogCC26X4_close(Watchdog_Handle handle);
int_fast16_t WatchdogCC26X4_control(Watchdog_Handle handle, uint_fast16_t cmd, void *arg);
void WatchdogCC26X4_init(Watchdog_Handle handle);
Watchdog_Handle WatchdogCC26X4_open(Watchdog_Handle handle, Watchdog_Params *params);
int_fast16_t WatchdogCC26X4_setReload(Watchdog_Handle handle, uint32_t ticks);
uint32_t WatchdogCC26X4_convertMsToTicks(Watchdog_Handle handle, uint32_t milliseconds);

/* WatchdogCC26X4 internal functions */
static void WatchdogCC26X4_initHw(Watchdog_Handle handle);

/* WatchdogCC26X4 global variables */
static volatile uint32_t reloadValue;  /* Reload value in ticks */
static volatile uint32_t sclkLfFreqHz; /* SCLK_LF frequency in Hz clocking Watchdog */

/* Watchdog function table for CC26X4 implementation */
const Watchdog_FxnTable WatchdogCC26X4_fxnTable = {WatchdogCC26X4_clear,
                                                   WatchdogCC26X4_close,
                                                   WatchdogCC26X4_control,
                                                   WatchdogCC26X4_init,
                                                   WatchdogCC26X4_open,
                                                   WatchdogCC26X4_setReload,
                                                   WatchdogCC26X4_convertMsToTicks};

#define MAX_RELOAD_VALUE 0xFFFFFFFF /* Maximum allowable reload value */
#define MS_RATIO         1000       /* millisecond to second ratio */
#define WATCHDOG_UNLOCK  0x1ACCE551 /* Watchdog unlocking value */

/*
 *  ======== WatchdogCC26X4_lock ========
 */
static inline void WatchdogCC26X4_lock(void)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_WDTLOCK) = 0x0;
}

/*
 *  ======== WatchdogCC26X4_unlock ========
 */
static inline void WatchdogCC26X4_unlock(void)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_WDTLOCK) = WATCHDOG_UNLOCK;
}

/*
 *  ======== WatchdogCC26X4_isLocked ========
 */
static inline bool WatchdogCC26X4_isLocked(void)
{
    return (HWREG(AON_PMCTL_BASE + AON_PMCTL_O_WDTLOCK) ? false : true);
}

/*
 *  ======== WatchdogCC26X4_setReloadValue ========
 */
static inline void WatchdogCC26X4_setReloadValue(uint32_t ticks)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_WDTLOAD) = ticks;
}

/*
 *  ======== WatchdogCC26X4_enableStall ========
 */
static inline void WatchdogCC26X4_enableStall(void)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_WDTTEST) = AON_PMCTL_WDTTEST_STALLEN;
}

/*
 *  ======== WatchdogCC26X4_disableStall ========
 */
static inline void WatchdogCC26X4_disableStall(void)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_WDTTEST) = 0x0;
}

/*
 *  ======== WatchdogCC26X4_clear ========
 */
void WatchdogCC26X4_clear(Watchdog_Handle handle)
{
    unsigned int key;

    /* disable preemption while unlocking Watchdog registers */
    key = HwiP_disable();

    /* unlock the Watchdog configuration registers */
    WatchdogCC26X4_unlock();

    /* make sure the Watchdog is unlocked before continuing */
    while (WatchdogCC26X4_isLocked()) {}

    WatchdogCC26X4_setReloadValue(reloadValue);

    /* lock the Watchdog configuration registers */
    WatchdogCC26X4_lock();

    HwiP_restore(key);
}

/*
 *  ======== WatchdogCC26X4_close ========
 */
void WatchdogCC26X4_close(Watchdog_Handle handle)
{
    /*
     *  Not supported for CC13X4/CC26X4 - Once the Watchdog module is started
     *  it can only be stopped by a device reset.
     */
    DebugP_assert(false);
}

/*
 *  ======== WatchdogCC26X4_control ========
 *  @pre    Function assumes that the handle is not NULL
 */
int_fast16_t WatchdogCC26X4_control(Watchdog_Handle handle, uint_fast16_t cmd, void *arg)
{
    /* Not supported on CC13X4/CC26X4 */
    return (Watchdog_STATUS_UNDEFINEDCMD);
}

/*
 *  ======== Watchdog_init ========
 */
void WatchdogCC26X4_init(Watchdog_Handle handle)
{
    WatchdogCC26X4_Object *object = handle->object;
    object->isOpen                = false;
}

/*
 *  ======== WatchdogCC26X4_open ========
 */
Watchdog_Handle WatchdogCC26X4_open(Watchdog_Handle handle, Watchdog_Params *params)
{
    unsigned int key;
    WatchdogCC26X4_Object *object;

    /* get the pointer to the object and hwAttrs */
    object = handle->object;

    /* disable preemption while checking if the Watchdog is open. */
    key = HwiP_disable();

    /* Check if the Watchdog is open already with the hwAttrs */
    if (object->isOpen == true)
    {
        HwiP_restore(key);
        DebugP_log1("Watchdog: Handle %x already in use.", (uintptr_t)handle);
        return (NULL);
    }

    object->isOpen = true;
    HwiP_restore(key);

    /* initialize the Watchdog object */
    object->debugStallMode = params->debugStallMode;

    /*
     *  Register SCLK_LF source frequency. SCLK_LF sources have slightly different
     *  CLK frequencies and this has to be accounted for when converting milliseconds
     *  into watchdog ticks.
     */
    switch (CCFGRead_SCLK_LF_OPTION())
    {
        case CCFGREAD_SCLK_LF_OPTION_XOSC_HF_DLF:
            sclkLfFreqHz = 31250;
            break;
        case CCFGREAD_SCLK_LF_OPTION_XOSC_LF:
        case CCFGREAD_SCLK_LF_OPTION_RCOSC_LF:
        case CCFGREAD_SCLK_LF_OPTION_EXTERNAL_LF:
        default:
            sclkLfFreqHz = 32768;
            break;
    }

    /* initialize the watchdog hardware */
    WatchdogCC26X4_initHw(handle);

    DebugP_log1("Watchdog: handle %x opened", (uintptr_t)handle);

    /* return handle of the Watchdog object */
    return (handle);
}

/*
 *  ======== WatchdogCC26X4_setReload ========
 */
int_fast16_t WatchdogCC26X4_setReload(Watchdog_Handle handle, uint32_t ticks)
{
    unsigned int key;

    /* disable preemption while unlocking Watchdog registers */
    key = HwiP_disable();

    /* unlock the Watchdog configuration registers */
    WatchdogCC26X4_unlock();

    /* make sure the Watchdog is unlocked before continuing */
    while (WatchdogCC26X4_isLocked()) {}

    /* update the reload value */
    reloadValue = ticks;
    WatchdogCC26X4_setReloadValue(reloadValue);

    /* lock register access */
    WatchdogCC26X4_lock();

    HwiP_restore(key);

    DebugP_log2("Watchdog: WDT with handle 0x%x has been set to reload to 0x%x", (uintptr_t)handle, ticks);

    return (Watchdog_STATUS_SUCCESS);
}

/*
 *  ======== WatchdogCC26X4_hwInit ========
 *  This function initializes the Watchdog hardware module.
 *
 *  @pre    Function assumes that the Watchdog handle is pointing to a hardware
 *          module which has already been opened.
 */
static void WatchdogCC26X4_initHw(Watchdog_Handle handle)
{
    unsigned int key;
    uint32_t tickValue;
    WatchdogCC26X4_Object *object;
    WatchdogCC26X4_HWAttrs const *hwAttrs;

    /* get the pointer to the object and hwAttrs */
    object  = handle->object;
    hwAttrs = handle->hwAttrs;

    /* convert milliseconds to watchdog timer ticks */
    tickValue = WatchdogCC26X4_convertMsToTicks(handle, hwAttrs->reloadValue);

    /* disable preemption while unlocking WatchDog registers */
    key = HwiP_disable();

    /* unlock the Watchdog configuration registers */
    WatchdogCC26X4_unlock();

    /* make sure the Watchdog is unlocked before continuing */
    while (WatchdogCC26X4_isLocked()) {}

    /* set debug stall mode */
    if (object->debugStallMode == Watchdog_DEBUG_STALL_ON)
    {
        WatchdogCC26X4_enableStall();
    }
    else
    {
        WatchdogCC26X4_disableStall();
    }

    /* store reloadValue for future clear() operations */
    reloadValue = tickValue;
    WatchdogCC26X4_setReloadValue(reloadValue);

    /* lock the Watchdog configuration registers */
    WatchdogCC26X4_lock();

    HwiP_restore(key);
}

/*
 *  ======== WatchdogCC26X4_convertMsToTicks ========
 *  This function converts the input value from milliseconds to
 *  Watchdog clock ticks.
 */
uint32_t WatchdogCC26X4_convertMsToTicks(Watchdog_Handle handle, uint32_t milliseconds)
{
    uint32_t tickValue;
    uint32_t maxConvertMs;

    /* Determine maximum allowed period with the current SCLK_LF frequency */
    maxConvertMs = (MAX_RELOAD_VALUE / sclkLfFreqHz) * MS_RATIO;

    /* convert milliseconds to watchdog timer ticks */
    /* check if value exceeds maximum */
    if (milliseconds > maxConvertMs)
    {
        tickValue = 0; /* return zero to indicate overflow */
    }
    else
    {
        /* avoid overflow of the intermediate multiplication when calculating tickValue */
        tickValue = (uint32_t)(((uint64_t)milliseconds * sclkLfFreqHz) / MS_RATIO);
    }

    return (tickValue);
}
