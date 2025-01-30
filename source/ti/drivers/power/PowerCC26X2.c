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
/*
 *  ======== PowerCC26X2.c ========
 */

#include <stdbool.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/SwiP.h>
#include <ti/drivers/dpl/DebugP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/power/PowerCC26X2_helpers.h>
#include <ti/drivers/Temperature.h>

#include <ti/log/Log.h>

/* driverlib header files */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_prcm.h)
#include DeviceFamily_constructPath(inc/hw_nvic.h)
#include DeviceFamily_constructPath(inc/hw_aux_sysif.h)
#include DeviceFamily_constructPath(inc/hw_aon_rtc.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_memmap_common.h)
#include DeviceFamily_constructPath(inc/hw_ccfg.h)
#include DeviceFamily_constructPath(inc/hw_rfc_pwr.h)
#include DeviceFamily_constructPath(inc/hw_aon_pmctl.h)
#include DeviceFamily_constructPath(inc/hw_fcfg1.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(driverlib/pwr_ctrl.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)
#include DeviceFamily_constructPath(driverlib/aon_ioc.h)
#include DeviceFamily_constructPath(driverlib/aon_rtc.h)
#include DeviceFamily_constructPath(driverlib/aon_event.h)
#include DeviceFamily_constructPath(driverlib/osc.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include DeviceFamily_constructPath(driverlib/vims.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/setup.h)
#include DeviceFamily_constructPath(driverlib/setup_rom.h)
#include DeviceFamily_constructPath(driverlib/ccfgread.h)

static unsigned int configureXOSCHF(unsigned int action);
static unsigned int nopResourceHandler(unsigned int action);
static unsigned int configureRFCoreClocks(unsigned int action);
static void switchXOSCHF(void);
static void disableLFClockQualifiers(void);
static void emptyClockFunc(uintptr_t arg);
static int_fast16_t notify(uint_fast16_t eventType);
static void oscillatorISR(uintptr_t arg);
static void switchToTCXO(void);
static void hposcRtcCompensateFxn(int16_t currentTemperature,
                                  int16_t thresholdTemperature,
                                  uintptr_t clientArg,
                                  Temperature_NotifyObj *notifyObject);
static void xosclfRtcCompensateFxn(int16_t currentTemperature,
                                   int16_t thresholdTemperature,
                                   uintptr_t clientArg,
                                   Temperature_NotifyObj *notifyObject);

/* RCOSC calibration functions functions */
extern void PowerCC26X2_calibrate(void);
extern bool PowerCC26X2_initiateCalibration(void);
extern void PowerCC26X2_auxISR(uintptr_t arg);
extern void PowerCC26X2_RCOSC_clockFunc(uintptr_t arg);

/* Externs */
extern const PowerCC26X2_Config PowerCC26X2_config;

/* Macro for weak definition of the Power Log module */
Log_MODULE_DEFINE_WEAK(LogModule_Power, {0});

/* Module_State */
PowerCC26X2_ModuleState PowerCC26X2_module = {
    .notifyList       = {0},          /* list of registered notifications    */
    .constraintMask   = 0,            /* the constraint mask                 */
    .clockObj         = {0},          /* Clock object for scheduling wakeups */
    .calibrationClock = {0},          /* Clock object for RCOSC calibration  */
    .tcxoEnableClock  = {0},          /* Clock object for TCXO startup       */
    .tdcHwi           = {0},          /* hwi object for calibration          */
    .oscHwi           = {0},          /* hwi object for oscillators          */
    .nDeltaFreqCurr   = 0,            /* RCOSC calibration variable          */
    .nCtrimCurr       = 0,            /* RCOSC calibration variable          */
    .nCtrimFractCurr  = 0,            /* RCOSC calibration variable          */
    .nCtrimNew        = 0,            /* RCOSC calibration variable          */
    .nCtrimFractNew   = 0,            /* RCOSC calibration variable          */
    .nRtrimNew        = 0,            /* RCOSC calibration variable          */
    .nRtrimCurr       = 0,            /* RCOSC calibration variable          */
    .nDeltaFreqNew    = 0,            /* RCOSC calibration variable          */
    .bRefine          = false,        /* RCOSC calibration variable          */
    .state            = Power_ACTIVE, /* current transition state            */
    .xoscPending      = false,        /* is XOSC_HF activation in progress?  */
    .calLF            = false,        /* calibrate RCOSC_LF?                 */
    .auxHwiState      = 0,            /* calibration AUX ISR state           */
    .busyCal          = false,        /* already busy calibrating            */
    .calStep          = 0,            /* current calibration step            */
    .firstLF          = true,         /* is this first LF calibration?       */
    .enablePolicy     = false,        /* default value is false              */
    .initialized      = false,        /* whether Power_init has been called  */
    .constraintCounts = {0, 0, 0, 0, 0, 0, 0},
    .resourceHandlers = {configureRFCoreClocks, configureXOSCHF, nopResourceHandler}, /* special resource handler
                                                                                         functions */
    .policyFxn        = 0,                                                            /* power policyFxn */
};

/*! Temperature notification to compensate the RTC when SCLK_LF is derived
 *  from SCLK_HF when SCLK_HF is configured as HPOSC.
 */
static Temperature_NotifyObj PowerCC26X2_hposcRtcCompNotifyObj = {0};

/*! Temperature notification to compensate the RTC when SCLK_LF is derived
 *  from XOSC_LF
 */
static Temperature_NotifyObj PowerCC26X2_xosclfRtcCompNotifyObj = {0};

/*! Default RTC subsecond increment corresponding to precisely 32768 Hz
 *  increment frequency
 */
#define RTC_SUBSECINC_32768_HZ 0x800000

/*! This internal variable to the power-driver keeps track of the most recent
 *  RTC increment value, whenever it is compensated due to temperature drift.
 *  In case of a clock-switch from XOSC_LF to RCOSC_LF or vice versa, this
 *  value is used to reprogram the RTC with a correct compensated value.
 */
static volatile uint32_t PowerCC26X2_rtcSubSecInc = RTC_SUBSECINC_32768_HZ;

/* resource database */
const PowerCC26XX_ResourceRecord resourceDB[PowerCC26X2_NUMRESOURCES] = {
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER0}, /* PERIPH_GPT0 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER1}, /* PERIPH_GPT1 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER2}, /* PERIPH_GPT2 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER3}, /* PERIPH_GPT3 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_SERIAL, PRCM_PERIPH_SSI0},   /* PERIPH_SSI0 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_SERIAL, PRCM_PERIPH_UART0},  /* PERIPH_UART0 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_SERIAL, PRCM_PERIPH_I2C0},   /* PERIPH_I2C0 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TRNG},   /* PERIPH_TRNG */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_GPIO},   /* PERIPH_GPIO */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_UDMA},   /* PERIPH_UDMA */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_CRYPTO}, /* PERIPH_CRYPTO */
    {PowerCC26XX_PERIPH | PowerCC26XX_PERIPH_UDMA, PRCM_PERIPH_I2S},      /* PERIPH_I2S */
    {PowerCC26XX_SPECIAL | PowerCC26XX_DOMAIN_RFCORE, 0},                 /* PERIPH_RFCORE */
    {PowerCC26XX_SPECIAL | PowerCC26XX_NOPARENT, 1},                      /* XOSC_HF */
    {PowerCC26XX_DOMAIN | PowerCC26XX_NOPARENT, PRCM_DOMAIN_PERIPH},      /* DOMAIN_PERIPH */
    {PowerCC26XX_DOMAIN | PowerCC26XX_NOPARENT, PRCM_DOMAIN_SERIAL},      /* DOMAIN_SERIAL */
    {PowerCC26XX_DOMAIN | PowerCC26XX_NOPARENT, PRCM_DOMAIN_RFCORE},      /* DOMAIN_RFCORE */
    {PowerCC26XX_SPECIAL | PowerCC26XX_NOPARENT, 2},                      /* DOMAIN_SYSBUS */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2 || \
     DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_PKA},   /* PERIPH_PKA */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_UART1}, /* PERIPH_UART1 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_SSI1},  /* PERIPH_SSI1 */
#endif
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_UART2}, /* PERIPH_UART2 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_UART3}, /* PERIPH_UART3 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_SSI2},  /* PERIPH_SSI2 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_SSI3},  /* PERIPH_SSI3 */
    {PowerCC26XX_PERIPH | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_I2C1},  /* PERIPH_I2C1 */
#endif
};

/* Defines */
#define TCXO_RAMP_DELAY        10
#define CC26X2_CLOCK_FREQUENCY 48000000

/* This is an approximate scaling factor previously used in test firmware. */
#define DELAY_SCALING_FACTOR 6000000

/* ****************** Power APIs ******************** */

/*
 *  ======== Power_disablePolicy ========
 *  Do not run the configured policy
 */
bool Power_disablePolicy(void)
{
    bool enablePolicy               = PowerCC26X2_module.enablePolicy;
    PowerCC26X2_module.enablePolicy = false;

    return (enablePolicy);
}

/*
 *  ======== Power_enablePolicy ========
 *  Run the configured policy
 */
void Power_enablePolicy(void)
{
    PowerCC26X2_module.enablePolicy = true;
}

/*
 *  ======== Power_getConstraintMask ========
 *  Get a bitmask indicating the constraints that have been registered with
 *  Power.
 */
uint_fast32_t Power_getConstraintMask(void)
{
    return (PowerCC26X2_module.constraintMask);
}

/*
 *  ======== Power_getDependencyCount ========
 *  Get the count of dependencies that are currently declared upon a resource.
 */
int_fast16_t Power_getDependencyCount(Power_Resource resourceId)
{
    DebugP_assert(resourceId < PowerCC26X2_NUMRESOURCES);

    return ((int_fast16_t)PowerCC26X2_module.resourceCounts[resourceId]);
}

/*
 *  ======== Power_getConstraintCount ========
 *  Get the count of constraints that are currently set on a certain
 *  operational transition
 */
int_fast16_t Power_getConstraintCount(uint_fast16_t constraintId)
{
    DebugP_assert(constraintId < PowerCC26X2_NUMCONSTRAINTS);

    if (constraintId < PowerCC26X2_NUMCONSTRAINTS)
    {
        return (int_fast16_t)PowerCC26X2_module.constraintCounts[constraintId];
    }
    else
    {
        return (int_fast16_t)Power_EINVALIDINPUT;
    }
}

/*
 *  ======== Power_getTransitionLatency ========
 *  Get the transition latency for a sleep state.  The latency is reported
 *  in units of microseconds.
 */
uint_fast32_t Power_getTransitionLatency(uint_fast16_t sleepState, uint_fast16_t type)
{
    uint32_t latency = 0;

    if (type == Power_RESUME)
    {
        if (sleepState == PowerCC26XX_STANDBY)
        {
            latency = PowerCC26X2_RESUMETIMESTANDBY;
        }
    }
    else
    {
        if (sleepState == PowerCC26XX_STANDBY)
        {
            latency = PowerCC26X2_TOTALTIMESTANDBY;
        }
    }

    return (latency);
}

/*
 *  ======== Power_getTransitionState ========
 *  Get the current sleep transition state.
 */
uint_fast16_t Power_getTransitionState(void)
{
    return (PowerCC26X2_module.state);
}

/*
 *  ======== Power_idleFunc ========
 *  Function needs to be plugged into the idle loop.
 *  It calls the configured policy function if the
 *  'enablePolicy' flag is set.
 */
void Power_idleFunc(void)
{
    if (PowerCC26X2_module.enablePolicy)
    {
        if (PowerCC26X2_module.policyFxn != NULL)
        {
            (*(PowerCC26X2_module.policyFxn))();
        }
    }
}

/*
 *  ======== Power_init ========
 */
int_fast16_t Power_init(void)
{
    ClockP_Params clockParams;
    uint32_t ccfgLfClkSrc;

    /* if this function has already been called, just return */
    if (PowerCC26X2_module.initialized)
    {
        return (Power_SOK);
    }

    /* set module state field 'initialized' to true */
    PowerCC26X2_module.initialized = true;

    PowerCC26X2_module.lastResetReason = (PowerCC26X2_ResetReason)PowerCC26X2_sysCtrlGetResetSource();

    /* set the module state enablePolicy field */
    PowerCC26X2_module.enablePolicy = PowerCC26X2_config.enablePolicy;

    /* copy the Power policy function to module state */
    PowerCC26X2_module.policyFxn = PowerCC26X2_config.policyFxn;

    /* Check if TCXO is selected in CCFG and in addition configured to be
     * enabled by the function pointed to by PowerCC26X2_config.enableTCXOFxn
     */
    if ((CCFGRead_XOSC_FREQ() == CCFGREAD_XOSC_FREQ_TCXO) && (PowerCC26X2_config.enableTCXOFxn != NULL))
    {
        /* Construct the Clock object for TCXO startup time.
         * Set timeout to TCXO startup time as specified in CCFG.
         */
        ClockP_construct(&PowerCC26X2_module.tcxoEnableClock,
                         (ClockP_Fxn)&switchToTCXO,
                         (CCFGRead_TCXO_MAX_START() * 100) / ClockP_getSystemTickPeriod(),
                         NULL);

        PowerCC26X2_oscCtlClearXtal();
    }

    /* construct the Clock object for scheduling of wakeups */
    /* initiated and started by the power policy */
    ClockP_Params_init(&clockParams);
    clockParams.period    = 0;
    clockParams.startFlag = false;
    clockParams.arg       = 0;
    ClockP_construct(&PowerCC26X2_module.clockObj, &emptyClockFunc, 0, &clockParams);

    /*
     *  If RCOSC calibration is enabled, construct a Clock object for
     *  delays. Set timeout to 8 Clock tick periods to get
     *  ceil(8x10us/30.5us/SCLK_LF_period)*30.5us/SCLK_LF_period = ~90us.
     *  The total time we need to wait for AUX_SYSIF_TDCREFCLKCTL_ACK
     *  is about 105us and the ClockP_start() call needs about 21us.
     *  All together, that makes ~111us. A decent approximation of the
     *  ideal wait duration.
     *  In practice, the COMPARE_MARGIN that is currently still in
     *  the kernel Timer.c implementation may make it take longer
     *  than 90us to time out.
     */
    ClockP_Params_init(&clockParams);
    clockParams.period    = 0;
    clockParams.startFlag = false;
    clockParams.arg       = 0;
    ClockP_construct(&PowerCC26X2_module.calibrationClock, &PowerCC26X2_RCOSC_clockFunc, 8, &clockParams);

    HwiP_construct(&PowerCC26X2_module.oscHwi, INT_OSC_COMB, oscillatorISR, NULL);
    HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC) = 0;

    /* construct the TDC hwi */
    HwiP_construct(&PowerCC26X2_module.tdcHwi, INT_AUX_COMB, PowerCC26X2_auxISR, NULL);

    /* read the LF clock source from CCFG */
    ccfgLfClkSrc = CCFGRead_SCLK_LF_OPTION();

    /* check if should calibrate RCOSC_LF */
    if (PowerCC26X2_config.calibrateRCOSC_LF)
    {
        /* verify RCOSC_LF is the LF clock source */
        if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_RCOSC_LF)
        {
            PowerCC26X2_module.calLF = true;
        }
    }

    if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_RCOSC_LF || ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_XOSC_LF ||
        ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_XOSC_HF_DLF)
    {
        /* Turn on oscillator interrupt for SCLK_LF switching.
         * When using HPOSC-derived LF, the LF clock will already have switched
         * and the interrupt will fire once interrupts are enabled
         * again when the OS starts.
         * When using a regular HF crystal to derive the LF source, it may take
         * up to a few hundred microseconds for the crystal to start up.
         * When using RCOSC_LF, the clock switch will happen very fast and
         * the interrupt will often immediately trigger once interrupts are
         * enabled again when the OS starts.
         */
        HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC) |= PRCM_OSCIMSC_LFSRCDONEIM_M;

        /* Disallow STANDBY pending LF clock quailifier disabling */
        Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }
    else if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_EXTERNAL_LF)
    {
        /* If the LF clock source is external, can disable clock qualifiers
         * now; no need to assert DISALLOW_STANDBY
         */

        /* yes, disable the LF clock qualifiers */
        PowerCC26X2_oscDisableQualifiers();
    }

    /* if VIMS RAM is configured as GPRAM: set retention constraint */
    if (!CCFGRead_DIS_GPRAM())
    {
        Power_setConstraint(PowerCC26XX_RETAIN_VIMS_CACHE_IN_STANDBY);
    }

    return (Power_SOK);
}

/*
 *  ======== Power_registerNotify ========
 *  Register a function to be called on a specific power event.
 *
 */
int_fast16_t Power_registerNotify(Power_NotifyObj *pNotifyObj,
                                  uint_fast16_t eventTypes,
                                  Power_NotifyFxn notifyFxn,
                                  uintptr_t clientArg)
{
    int_fast16_t status = Power_SOK;

    /* check for NULL pointers  */
    if ((pNotifyObj == NULL) || (notifyFxn == NULL))
    {
        Log_printf(LogModule_Power, Log_WARNING,
        "Power_registerNotify: Notify registration failed due to NULL pointer");

        status = Power_EINVALIDPOINTER;
    }

    else
    {
        /* fill in notify object elements */
        pNotifyObj->eventTypes = eventTypes;
        pNotifyObj->notifyFxn  = notifyFxn;
        pNotifyObj->clientArg  = clientArg;

        Log_printf(LogModule_Power,
                   Log_INFO,
                   "Power_registerNotify: Register fxn at address 0x%x with event types 0x%x and clientArg 0x%x",
                   notifyFxn,
                   eventTypes,
                   clientArg);

        /* place notify object on event notification queue */
        List_put(&PowerCC26X2_module.notifyList, (List_Elem *)pNotifyObj);
    }

    return (status);
}

/*
 *  ======== Power_releaseConstraint ========
 *  Release a previously declared constraint.
 */
int_fast16_t Power_releaseConstraint(uint_fast16_t constraintId)
{
    unsigned int key;
    uint8_t count;

    /* assert constraintId is valid */
    DebugP_assert(constraintId < PowerCC26X2_NUMCONSTRAINTS);

    key = HwiP_disable();

    /* get the count of the constraint */
    count = PowerCC26X2_module.constraintCounts[constraintId];

    DebugP_assert(count != 0);

    count--;

    /* save the updated count */
    PowerCC26X2_module.constraintCounts[constraintId] = count;

    if (count == 0)
    {
        PowerCC26X2_module.constraintMask &= ~(1 << constraintId);
    }

    HwiP_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_releaseDependency ========
 *  Release a previously declared dependency.
 */
int_fast16_t Power_releaseDependency(Power_Resource resourceId)
{
    PowerCC26XX_Resource parent;
    uint8_t count;
    uint32_t id;
    unsigned int key;

    /* assert resourceId is valid */
    DebugP_assert(resourceId < PowerCC26X2_NUMRESOURCES);

    /* disable interrupts */
    key = HwiP_disable();

    /* read and decrement the reference count */
    count = PowerCC26X2_module.resourceCounts[resourceId];

    DebugP_assert(count != 0);

    count--;

    /* save the reference count */
    PowerCC26X2_module.resourceCounts[resourceId] = count;

    /* if this was the last dependency being released.., */
    if (count == 0)
    {
        /* deactivate this resource ... */
        id = resourceDB[resourceId].driverlibID;

/* Conditional compile required since CC26X1 does not define PRCM_PERIPH_PKA */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
        /*
         * Special handling for the Crypto, TRNG, PKA, and DMA peripherals
         * which are on the secure side for CC26X3/X4.
         */
        if ((id == PRCM_PERIPH_CRYPTO) || (id == PRCM_PERIPH_TRNG) || (id == PRCM_PERIPH_PKA) ||
            (id == PRCM_PERIPH_UDMA))
        {
            PowerCC26X2_releasePeriphDependency(id);
        }
        else
        {
#endif
            /* is resource a peripheral?... */
            if (resourceDB[resourceId].flags & PowerCC26XX_PERIPH)
            {
                PRCMPeripheralRunDisable(id);
                PRCMPeripheralSleepDisable(id);
                PRCMPeripheralDeepSleepDisable(id);
                PRCMLoadSet();
                while (!PRCMLoadGet()) {}
            }
            /* else, does resource require a special handler?... */
            else if (resourceDB[resourceId].flags & PowerCC26XX_SPECIAL)
            {
                /* call the special handler */
                PowerCC26X2_module.resourceHandlers[id](PowerCC26XX_DISABLE);
            }
            /* else resource is a power domain */
            else
            {
                PRCMPowerDomainOff(id);
                while (PRCMPowerDomainsAllOff(id) != PRCM_DOMAIN_POWER_OFF) {}
            }

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
        }
#endif
        /* propagate release up the dependency tree ... */

        /* check for a first parent */
        parent = resourceDB[resourceId].flags & PowerCC26XX_PARENTMASK;

        /* if 1st parent, make recursive call to release that dependency */
        if (parent < PowerCC26X2_NUMRESOURCES)
        {
            Power_releaseDependency(parent);
        }
    }

    Log_printf(LogModule_Power,
               Log_INFO,
               "Power_releaseDependency: Updated resource counter = %d for resource ID = 0x%x",
               PowerCC26X2_module.resourceCounts[resourceId], resourceId);

    /* re-enable interrupts */
    HwiP_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_setConstraint ========
 *  Declare an operational constraint.
 */
int_fast16_t Power_setConstraint(uint_fast16_t constraintId)
{
    unsigned int key;

    /* assert constraint id is valid */
    DebugP_assert(constraintId < PowerCC26X2_NUMCONSTRAINTS);

    /* disable interrupts */
    key = HwiP_disable();

    /* set the specified constraint in the constraintMask */
    PowerCC26X2_module.constraintMask |= 1 << constraintId;

    /* increment the specified constraint count */
    PowerCC26X2_module.constraintCounts[constraintId]++;

    /* re-enable interrupts */
    HwiP_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_setDependency ========
 *  Declare a dependency upon a resource.
 */
int_fast16_t Power_setDependency(Power_Resource resourceId)
{
    PowerCC26XX_Resource parent;
    uint8_t count;
    uint32_t id;
    unsigned int key;

    /* assert resourceId is valid */
    DebugP_assert(resourceId < PowerCC26X2_NUMRESOURCES);

    /* disable interrupts */
    key = HwiP_disable();

    /* read and increment reference count */
    count = PowerCC26X2_module.resourceCounts[resourceId]++;

    /* if resource was NOT activated previously ... */
    if (count == 0)
    {
        /* propagate set up the dependency tree ... */

        /* check for a first parent */
        parent = resourceDB[resourceId].flags & PowerCC26XX_PARENTMASK;

        /* if first parent, make recursive call to set that dependency */
        if (parent < PowerCC26X2_NUMRESOURCES)
        {
            Power_setDependency(parent);
        }

        /* now activate this resource ... */
        id = resourceDB[resourceId].driverlibID;

/* Conditional compile required since CC26X1 does not define PRCM_PERIPH_PKA */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
        /*
         * Special handling for the Crypto, TRNG, PKA, and DMA peripherals
         * which are on the secure side for CC26X3/X4.
         */
        if ((id == PRCM_PERIPH_CRYPTO) || (id == PRCM_PERIPH_TRNG) || (id == PRCM_PERIPH_PKA) ||
            (id == PRCM_PERIPH_UDMA))
        {
            PowerCC26X2_setPeriphDependency(id);
        }
        else
        {
#endif
            /* is resource a peripheral?... */
            if (resourceDB[resourceId].flags & PowerCC26XX_PERIPH)
            {
                PRCMPeripheralRunEnable(id);
                PRCMPeripheralSleepEnable(id);
                PRCMPeripheralDeepSleepEnable(id);
                PRCMLoadSet();
                while (!PRCMLoadGet()) {}
            }
            /* else, does resource require a special handler?... */
            else if (resourceDB[resourceId].flags & PowerCC26XX_SPECIAL)
            {
                /* call the special handler */
                PowerCC26X2_module.resourceHandlers[id](PowerCC26XX_ENABLE);
            }
            /* else resource is a power domain */
            else
            {
                PRCMPowerDomainOn(id);
                while (PRCMPowerDomainsAllOn(id) != PRCM_DOMAIN_POWER_ON) {}
            }
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)
        }
#endif
    }

    Log_printf(LogModule_Power,
               Log_INFO,
               "Power_setDependency: Updated resource counter = %d for resource ID = 0x%x",
               PowerCC26X2_module.resourceCounts[resourceId], resourceId);

    /* re-enable interrupts */
    HwiP_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_setPolicy ========
 *  Set the Power policy function
 */
void Power_setPolicy(Power_PolicyFxn policy)
{
    PowerCC26X2_module.policyFxn = policy;
}

/*
 *  ======== Power_shutdown ========
 */
int_fast16_t Power_shutdown(uint_fast16_t shutdownState, uint_fast32_t shutdownTime)
{
    int_fast16_t status = Power_EFAIL;
    unsigned int constraints;
    unsigned int hwiKey;

    /* disable interrupts */
    hwiKey = HwiP_disable();

    /* check if there is a constraint to prohibit shutdown */
    constraints = Power_getConstraintMask();
    if (constraints & (1 << PowerCC26XX_DISALLOW_SHUTDOWN))
    {
        status = Power_ECHANGE_NOT_ALLOWED;
    }

    /* OK to shutdown ... */
    else if (PowerCC26X2_module.state == Power_ACTIVE)
    {
        /* set new transition state to entering shutdown */
        PowerCC26X2_module.state = Power_ENTERING_SHUTDOWN;

        /* signal all clients registered for pre-shutdown notification */
        status = notify(PowerCC26XX_ENTERING_SHUTDOWN);

        /* check for any error */
        if (status != Power_SOK)
        {
            PowerCC26X2_module.state = Power_ACTIVE;
            HwiP_restore(hwiKey);
            return (status);
        }

        /* Ensure the JTAG domain is turned off
         * otherwise MCU domain can't be turned off.
         */
        PowerCC26X2_pmctlDisableJtag();

        SysCtrlAonSync();

        /* now proceed with shutdown sequence ... */
        PowerCC26X2_sysctrlShutdownWithAbort();
    }
    else
    {
        status = Power_EBUSY;
    }

    /* NOTE: if shutdown succeeded, should never get here */

    /* return failure status */
    PowerCC26X2_module.state = Power_ACTIVE;

    /* re-enable interrupts */
    HwiP_restore(hwiKey);

    /* if get here, failed to shutdown, return error code */
    return (status);
}

/*
 *  ======== Power_sleep ========
 */
int_fast16_t Power_sleep(uint_fast16_t sleepState)
{
    int_fast16_t status           = Power_SOK;
    int_fast16_t notifyStatus     = Power_SOK;
    int_fast16_t lateNotifyStatus = Power_SOK;
    unsigned int xosc_hf_active   = false;
    uint_fast16_t postEventLate;
    uint32_t poweredDomains = 0;
    uint_fast16_t preEvent;
    uint_fast16_t postEvent;
    unsigned int constraints;
    bool retainCache = false;
    uint32_t modeVIMS;

    /* first validate the sleep code */
    if (sleepState != PowerCC26XX_STANDBY)
    {
        status = Power_EINVALIDINPUT;

        Log_printf(LogModule_Power,
                   Log_WARNING,
                   "Power_sleep: Entering standby failed with status = 0x%x",
                   status);
    }
    else
    {
        /* check to make sure Power is not busy with another transition */
        if (PowerCC26X2_module.state == Power_ACTIVE)
        {
            /* set transition state to entering sleep */
            PowerCC26X2_module.state = Power_ENTERING_SLEEP;
        }
        else
        {
            status = Power_EBUSY;

            Log_printf(LogModule_Power,
                       Log_WARNING,
                       "Power_sleep: Entering standby failed with status = 0x%x",
                       status);
        }

        if (status == Power_SOK)
        {
            /* setup sleep vars */
            preEvent      = PowerCC26XX_ENTERING_STANDBY;
            postEvent     = PowerCC26XX_AWAKE_STANDBY;
            postEventLate = PowerCC26XX_AWAKE_STANDBY_LATE;

            /* disable scheduling */
            PowerCC26XX_schedulerDisable();

            /* signal all clients registered for pre-sleep notification */
            status = notify(preEvent);

            /* check for any error */
            if (status != Power_SOK)
            {
                Log_printf(LogModule_Power,
                           Log_WARNING,
                           "Power_sleep: Entering standby failed due to pre-sleep notification status = 0x%x",
                           status);

                PowerCC26X2_module.state = Power_ACTIVE;
                PowerCC26XX_schedulerRestore();
                return (status);
            }

            /* 1. Query and save domain states before powering them off */
            if (Power_getDependencyCount(PowerCC26XX_DOMAIN_RFCORE))
            {
                poweredDomains |= PRCM_DOMAIN_RFCORE;
            }
            if (Power_getDependencyCount(PowerCC26XX_DOMAIN_SERIAL))
            {
                poweredDomains |= PRCM_DOMAIN_SERIAL;
            }
            if (Power_getDependencyCount(PowerCC26XX_DOMAIN_PERIPH))
            {
                poweredDomains |= PRCM_DOMAIN_PERIPH;
            }

            /* 2. If XOSC_HF is active or we are waiting to switch
             *    to it, force it off. Otherwise, the XOSC_HF may be
             *    automatically turned on by the hardware without
             *    a call to configureXOSCHF(PowerCC26XX_ENABLE)
             *    This is not necessarily a problem. However exactly
             *    what the cutoff point is where the hardware considers
             *    the XOSC_HF "on" without having switched to is not
             *    considered by this driver.
             */
            if (PowerCC26X2_oscClockSourceGet(OSC_SRC_CLK_HF) == OSC_XOSC_HF || PowerCC26X2_module.xoscPending == true)
            {
                Log_printf(LogModule_Power, Log_VERBOSE, "Power_sleep: Forcing XOSC_HF off");

                xosc_hf_active = true;
                configureXOSCHF(PowerCC26XX_DISABLE);
            }

            /* query constraints to determine if cache should be retained */
            constraints = Power_getConstraintMask();
            if (constraints & (1 << PowerCC26XX_RETAIN_VIMS_CACHE_IN_STANDBY))
            {
                retainCache = true;
            }
            else
            {
                retainCache = false;

                // Get the current VIMS mode
                do
                {
                    modeVIMS = VIMSModeGet(VIMS_BASE);
                } while (modeVIMS == VIMS_MODE_CHANGING);
            }

            /* 3.
             *  - Freeze the IOs on the boundary between MCU and AON
             *  - Make sure AON writes take effect
             *  - Request power off of every PD in the MCU voltage domain
             *  - Ensure that no clocks are forced on in Crypto, DMA and I2S
             *  - Gate running deep sleep clocks for Crypto, DMA and I2S
             *  - Load the new clock settings
             *  - Configure the VIMS power domain mode to power up flash
             *    again after coming out of standby.
             *  - Request uLDO during standby
             *  - Use recharge comparator
             *  - Ensure all writes have taken effect
             *  - Ensure UDMA, Crypto and I2C clocks are turned off
             *  - Ensure all non-CPU power domains are turned off
             *  - Turn off cache retention if requested
             *  - Invoke deep sleep to go to standby
             */
            PowerCC26X2_sysCtrlStandby(retainCache);

            /* 4. If didn't retain VIMS in standby, re-enable retention now */
            if (retainCache == false)
            {
                /* 5.1 If previously in a cache mode, restore the mode now */
                if (modeVIMS == VIMS_MODE_ENABLED)
                {
                    VIMSModeSet(VIMS_BASE, modeVIMS);
                }

                /* 5.2 Re-enable retention */
                PowerCC26X2_prcmEnableCacheRetention();
            }

            /* 6. Start re-powering power domains */
            PRCMPowerDomainOn(poweredDomains);

            /* 7. Restore deep sleep clocks of Crypto and DMA */
            if (Power_getDependencyCount(PowerCC26XX_PERIPH_CRYPTO))
            {
                PowerCC26X2_setPeriphDeepSleepEnable(resourceDB[PowerCC26XX_PERIPH_CRYPTO].driverlibID);
            }
            if (Power_getDependencyCount(PowerCC26XX_PERIPH_UDMA))
            {
                PowerCC26X2_setPeriphDeepSleepEnable(resourceDB[PowerCC26XX_PERIPH_UDMA].driverlibID);
            }

            /* 8. Make sure clock settings take effect */
            PRCMLoadSet();

            /* 9. Release request for uLDO */
            PRCMMcuUldoConfigure(false);

            /* 10. Set transition state to EXITING_SLEEP */
            PowerCC26X2_module.state = Power_EXITING_SLEEP;

            /* 11. Wait until all power domains are back on */
            while (PRCMPowerDomainsAllOn(poweredDomains) != PRCM_DOMAIN_POWER_ON) {}

            /* 12. Wait for the RTC shadow values to be updated so that
             * the early notification callbacks can read out valid RTC values.
             * This can likely be removed as the 2MHz MF clock will have ticked by now.
             */
            SysCtrlAonSync();

            /*
             * 13. Signal clients registered for early post-sleep notification;
             * this should be used to initialize any timing critical or IO
             * dependent hardware
             */
            notifyStatus = notify(postEvent);

            /* 14. Disable IO freeze and ensure RTC shadow value is updated */
            AONIOCFreezeDisable();
            SysCtrlAonSync();

            /* 15. If XOSC_HF was forced off above, initiate switch back */
            if (xosc_hf_active == true)
            {
                Log_printf(LogModule_Power,
                           Log_VERBOSE,
                           "Power_sleep: Forcing XOSC_HF back on");

                configureXOSCHF(PowerCC26XX_ENABLE);
            }

            /* 16. Re-enable interrupts */
            CPUcpsie();

            /*
             * 17. Signal all clients registered for late post-sleep
             * notification
             */
            lateNotifyStatus = notify(postEventLate);

            /*
             * 18. Now clear the transition state before re-enabling
             * scheduler
             */
            PowerCC26X2_module.state = Power_ACTIVE;

            /* 19. Re-enable scheduling */
            PowerCC26XX_schedulerRestore();

            /* if there was a notification error, set return status */
            if ((notifyStatus != Power_SOK) || (lateNotifyStatus != Power_SOK))
            {
                Log_printf(LogModule_Power,
                           Log_WARNING,
                           "Power_sleep: Notification error leaving standby. Status = 0x%x and late status = 0x%x",
                           notifyStatus,
                           lateNotifyStatus);

                status = Power_EFAIL;
            }
        }
    }

    return (status);
}

/*
 *  ======== Power_unregisterNotify ========
 *  Unregister for a power notification.
 *
 */
void Power_unregisterNotify(Power_NotifyObj *pNotifyObj)
{
    unsigned int key;

    /* remove notify object from its event queue */
    key = HwiP_disable();

    Log_printf(LogModule_Power,
               Log_INFO,
               "Power_unregisterNotify: Unregister fxn at address 0x%x with event types 0x%x and clientArg 0x%x",
               pNotifyObj->notifyFxn,
               pNotifyObj->eventTypes,
               pNotifyObj->clientArg);

    /* remove notify object from its event queue */
    List_remove(&PowerCC26X2_module.notifyList, (List_Elem *)pNotifyObj);

    HwiP_restore(key);
}

/*
 *  ======== Power_reset ========
 */
void Power_reset(void)
{
    PowerCC26X2_sysCtrlReset();
}

/* ****************** CC26XX specific APIs ******************** */

/*
 *  ======== PowerCC26X2_enableHposcRtcCompensation ========
 *  This function enabled temperature based compensation of the RTC when
 *  SCLK_LF is derived from HPOSC.
 */
void PowerCC26X2_enableHposcRtcCompensation(void)
{
    /* If we are using HPOSC and SCLK_LF is derived from it, we need to
     * compensate the RTC to account for HPOSC frequency drift over temperature.
     */
    if (PowerCC26X2_oscIsHPOSCEnabledWithHfDerivedLfClock())
    {
        Log_printf(LogModule_Power,
                   Log_INFO,
                   "PowerCC26X2_enableHposcRtcCompensation: Enable HPOSC RTC compensation");

        Temperature_init();

        int16_t currentTemperature = Temperature_getTemperature();

        OSC_HPOSCInitializeFrequencyOffsetParameters();

        /* The compensation fxn will register itself with updated thresholds
         * based on the current temperature each time it is invoked. If we
         * call it from the init function, it will register itself for the
         * first time and handle initial RTC compensation.
         */
        hposcRtcCompensateFxn(currentTemperature,
                              currentTemperature + PowerCC26X2_HPOSC_RTC_COMPENSATION_DELTA,
                              (uintptr_t)NULL,
                              &PowerCC26X2_hposcRtcCompNotifyObj);
    }
}

/*
 *  ======== PowerCC26X2_enableXoscLfRtcCompensation ========
 *  This function enables temperature based compensation of the RTC when
 *  SCLK_LF is derived from XOSC_LF.
 */
void PowerCC26X2_enableXoscLfRtcCompensation(void)
{
    /* Check that SCLK_LF is derived from XOSC_LF */
    if (CCFGRead_SCLK_LF_OPTION() == CCFGREAD_SCLK_LF_OPTION_XOSC_LF)
    {
        Log_printf(LogModule_Power,
                   Log_INFO,
                   "PowerCC26X2_enableXoscLfRtcCompensation: Enable XOSC_LF RTC compensation");

        Temperature_init();

        int16_t currentTemperature = Temperature_getTemperature();

        /* This must be called before xosclfRtcCompensateFxn, as the compensation parameters must be initialised
         * with respect to device-specifc temperature trim values.
         */
        OSC_LFXOSCInitStaticOffset();

        /* The compensation fxn will register itself with updated thresholds
         * based on the current temperature each time it is invoked. If we
         * call it from the init function, it will register itself for the
         * first time and handle initial RTC compensation.
         */
        xosclfRtcCompensateFxn(currentTemperature, 0, (uintptr_t)NULL, &PowerCC26X2_xosclfRtcCompNotifyObj);
    }
}

/*
 *  ======== PowerCC26XX_calibrate ========
 *  Plug this function into the PowerCC26X2_Config structure
 *  if calibration is needed.
 */
bool PowerCC26XX_calibrate(unsigned int arg)
{
    bool retVal = false;

    switch (arg)
    {
        case PowerCC26X2_INITIATE_CALIBRATE:
            retVal = PowerCC26X2_initiateCalibration();
            break;

        case PowerCC26X2_DO_CALIBRATE:
            PowerCC26X2_calibrate();
            break;
        default:
            while (1) {}
    }

    return (retVal);
}

/*
 *  ======== PowerCC26XX_doWFI ========
 */
void PowerCC26XX_doWFI(void)
{
    __asm(" wfi");
}

/*
 *  ======== PowerCC26X2_getClockHandle ========
 */
ClockP_Handle PowerCC26XX_getClockHandle(void)
{
    return ((ClockP_Handle)&PowerCC26X2_module.clockObj);
}

/*
 *  ======== PowerCC26XX_noCalibrate ========
 *  Plug this function into the PowerCC26X2 config structure if calibration
 *  is not needed.
 */
bool PowerCC26XX_noCalibrate(unsigned int arg)
{
    return (0);
}

/*
 *  ======== PowerCC26XX_getXoscStartupTime ========
 *  Get the estimated crystal oscillator startup time
 */
uint32_t PowerCC26XX_getXoscStartupTime(uint32_t timeUntilWakeupInMs)
{
    return (OSCHF_GetStartupTime(timeUntilWakeupInMs));
}

/*
 *  ======== PowerCC26X2_injectCalibration ========
 *  Explicitly trigger RCOSC calibration
 */
bool PowerCC26XX_injectCalibration(void)
{
    if ((*(PowerCC26X2_config.calibrateFxn))(PowerCC26X2_INITIATE_CALIBRATE))
    {
        /* here if AUX SMPH was available, start calibration now ... */
        (*(PowerCC26X2_config.calibrateFxn))(PowerCC26X2_DO_CALIBRATE);
        return (true);
    }

    return (false);
}

/*
 *  ======== PowerCC26XX_isStableXOSC_HF ========
 *  Check if XOSC_HF has stabilized.
 */
bool PowerCC26XX_isStableXOSC_HF(void)
{
    bool ready = true;
    unsigned int key;

    key = HwiP_disable();

    /* only query if HF source is ready if there is a pending change */
    if (PowerCC26X2_module.xoscPending)
    {
        ready = PowerCC26X2_getOscHfSourceReady();
    }

    HwiP_restore(key);

    return (ready);
}

/*
 *  ======== PowerCC26XX_switchXOSC_HF ========
 *  Switch to enable XOSC_HF.
 *  May only be called when using the PowerCC26XX_SWITCH_XOSC_HF_MANUALLY
 *  constraint.
 *  May only be called after ensuring the XOSC_HF is stable by calling
 *  PowerCC26XX_isStableXOSC_HF().
 */
void PowerCC26XX_switchXOSC_HF(void)
{
    bool readyToCal;
    unsigned int key;

    key = HwiP_disable();

    Log_printf(LogModule_Power,
               Log_INFO,
               "PowerCC26XX_switchXOSC_HF: Switch to enable XOSC_HF");

    /* Since PowerCC26X2_isStableXOSC_HF() should have been called before this
     * function, we can just switch without handling the case when the XOSC_HF
     * is not ready or PowerCC26X2_module.xoscPending is not true.
     */
    PowerCC26X2_oschfTrySwitchToXosc();

    /* Since configureXOSCHF() was called prior to this function to turn
     * on the XOSC_HF, PowerCC26X2_module.xoscPending will be true and
     * we can safely set it to false.
     */
    PowerCC26X2_module.xoscPending = false;

    /* Allow going into IDLE again since we sucessfully switched
     * to XOSC_HF
     */
    Power_releaseConstraint(PowerCC26XX_DISALLOW_IDLE);

    HwiP_restore(key);

    /* initiate RCOSC calibration */
    readyToCal = (*(PowerCC26X2_config.calibrateFxn))(PowerCC26X2_INITIATE_CALIBRATE);

    /* now notify clients that were waiting for a switch notification */
    notify(PowerCC26XX_XOSC_HF_SWITCHED);

    /* if ready to start first cal measurment, do it now */
    if (readyToCal == true)
    {
        (*(PowerCC26X2_config.calibrateFxn))(PowerCC26X2_DO_CALIBRATE);
    }
}

/*
 *  ======== PowerCC26XX_getResetReason ========
 *  Returns the root  for latest reset.
 */
PowerCC26X2_ResetReason PowerCC26X2_getResetReason(void)
{
    return PowerCC26X2_module.lastResetReason;
}

/* * * * * * * * * * * internal and support functions * * * * * * * * * * */

/*
 *  ======== hposcRtcCompensateFxn ========
 */
static void hposcRtcCompensateFxn(int16_t currentTemperature,
                                  int16_t thresholdTemperature,
                                  uintptr_t clientArg,
                                  Temperature_NotifyObj *notifyObject)
{
    int_fast16_t status;
    int32_t relFreqOffset;

    Log_printf(LogModule_Power,
               Log_INFO,
               "hposcRtcCompensateFxn: HPOSC RTC compensation");

    relFreqOffset = OSC_HPOSCRelativeFrequencyOffsetGet(currentTemperature);

    OSC_HPOSCRtcCompensate(relFreqOffset);

    /* Register the notification again with updated thresholds */
    status = Temperature_registerNotifyRange(notifyObject,
                                             currentTemperature + PowerCC26X2_HPOSC_RTC_COMPENSATION_DELTA,
                                             currentTemperature - PowerCC26X2_HPOSC_RTC_COMPENSATION_DELTA,
                                             hposcRtcCompensateFxn,
                                             (uintptr_t)NULL);

    if (status != Temperature_STATUS_SUCCESS)
    {
        Log_printf(LogModule_Power,
                   Log_ERROR,
                   "hposcRtcCompensateFxn: Notification registgration faild with status = 0x%x",
                   status);

        while (1) {}
    }
}

/*
 *  ======== xosclfRtcCompensateFxn ========
 */
static void xosclfRtcCompensateFxn(int16_t currentTemperature,
                                   int16_t thresholdTemperature,
                                   uintptr_t clientArg,
                                   Temperature_NotifyObj *notifyObject)
{
    int_fast16_t status;
    int32_t xoscLfTemperatureOffset;
    int32_t subsecIncCompensated;

    Log_printf(LogModule_Power,
               Log_INFO,
               "xosclfRtcCompensateFxn: XOSC RTC compensation");

    /* Get PPM offset of crystal */
    xoscLfTemperatureOffset = OSC_LFXOSCRelativeFrequencyOffsetGet(currentTemperature);

    /* Calculate new subsecond increment, given offset value */
    subsecIncCompensated = RTC_SUBSECINC_32768_HZ -
                           (int32_t)((RTC_SUBSECINC_32768_HZ * (int64_t)xoscLfTemperatureOffset) / 1000000LL);

    /* Update global RTC subsecond increment value, used by the Oscillator ISR when switching clocks */
    PowerCC26X2_rtcSubSecInc = subsecIncCompensated;

    /* Apply new RTC increment value */
    PowerCC26X2_setSubSecIncToXoscLf(subsecIncCompensated);

    /* Register the notification again with updated thresholds */
    status = Temperature_registerNotifyRange(notifyObject,
                                             currentTemperature + PowerCC26X2_XOSC_LF_RTC_COMPENSATION_DELTA,
                                             currentTemperature - PowerCC26X2_XOSC_LF_RTC_COMPENSATION_DELTA,
                                             xosclfRtcCompensateFxn,
                                             (uintptr_t)NULL);

    if (status != Temperature_STATUS_SUCCESS)
    {
        Log_printf(LogModule_Power,
                   Log_ERROR,
                   "xosclfRtcCompensateFxn: Notification registration faild with status = 0x%x",
                   status);

        while (1) {}
    }
}

/*
 *  ======== oscillatorISR ========
 */
static void oscillatorISR(uintptr_t arg)
{
    uint32_t rawStatus     = HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCRIS);
    uint32_t intStatusMask = HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC);

    /* Turn off mask for all flags we will handle */
    HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC) = intStatusMask & ~rawStatus;

    /* XOSC_LF or RCOSC_LF qualified */
    if (rawStatus & PRCM_OSCRIS_LFSRCDONERIS_M & intStatusMask)
    {
        /* Only switch SubSecInc for XOSC_LF.
         * XOSC_HF_DLF and RCOSC_LF do not spend enough time waiting for the
         * clock switch after enabling interrupts to accumulate any notable
         * drift.
         * External LF is just driven immediately and has no transition period.
         */
        if (CCFGRead_SCLK_LF_OPTION() == CCFGREAD_SCLK_LF_OPTION_XOSC_LF)
        {
            /* Set SubSecInc back to 32.768 kHz now that we have switched to
             * the RCOSC_LF or XOSC_LF target clock.
             */
            PowerCC26X2_setSubSecIncToXoscLf(PowerCC26X2_rtcSubSecInc);
        }

        disableLFClockQualifiers();

        /* Call all registered callback functions waiting on LF clock switch */
        Log_printf(LogModule_Power,
                   Log_INFO,
                   "oscillatorISR: Call all registered callback functions waiting on LF clock switch");

        notify(PowerCC26XX_SCLK_LF_SWITCHED);
    }

    /* XOSC_HF ready to switch to */
    if (rawStatus & PRCM_OSCIMSC_HFSRCPENDIM_M & intStatusMask)
    {
        Log_printf(LogModule_Power,
                   Log_INFO,
                   "oscillatorISR: Switch to XOSC_HF");

        switchXOSCHF();
    }

    /* Clear flags we will handle. Does not really work as expected as
     * the flags seem to level-detect and not edge-detect. Until the
     * underlying trigger is taken care of, the flag will not deassert
     * even when cleared.
     * We're clearing at the end in order to prevent the flag from
     * immediately asserting again if the underlying trigger was
     * not handled yet.
     * SCLK_LF switched can never be cleared after triggering
     * only masked out. XOSC_HF ready to switch can be cleared
     * after switching to XOSC_HF.
     */
    HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCICR) = intStatusMask & rawStatus;
}

/*
 *  ======== emptyClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void emptyClockFunc(uintptr_t arg)
{}

/*
 *  ======== disableLFClockQualifiers ========
 *  Clock function used for delayed disable of LF clock qualifiers.
 */
static void disableLFClockQualifiers(void)
{
    uint32_t sourceLF;

    /* query LF clock source */
    sourceLF = PowerCC26X2_oscClockSourceGet(OSC_SRC_CLK_LF);

    /* is LF source either RCOSC_LF or XOSC_LF yet? */
    if ((sourceLF == OSC_RCOSC_LF) || (sourceLF == OSC_XOSC_LF))
    {

        /* yes, disable the LF clock qualifiers */
        PowerCC26X2_oscDisableQualifiers();

        /* now finish by releasing the standby disallow constraint */
        Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);
    }
    else if (sourceLF == OSC_XOSC_HF)
    {
        /* yes, disable the LF clock qualifiers */
        PowerCC26X2_oscDisableQualifiers();

        /* do not allow standby since the LF clock is HF derived */
    }
}

/*
 *  ======== nopResourceHandler ========
 *  special resource handler
 */
static unsigned int nopResourceHandler(unsigned int action)
{
    return (0);
}

/*
 *  ======== delayUs ========
 *  Polls for an approximate number of us. Not very accurate.
 *  In this use case we only care about waiting 'at least X'
 *  and a few extra microseconds will only allow additional
 *  stabilisation time.
 */
static void delayUs(uint32_t us)
{
    CPUdelay((CC26X2_CLOCK_FREQUENCY / DELAY_SCALING_FACTOR) * us);
}

/*
 *  ======== notify ========
 *  Send notifications to registered clients.
 *  Note: Task scheduling is disabled when this function is called.
 */
static int_fast16_t notify(uint_fast16_t eventType)
{
    int_fast16_t notifyStatus;
    Power_NotifyFxn notifyFxn;
    uintptr_t clientArg;
    List_Elem *elem;

    /* if queue is empty, return immediately */
    if (!List_empty(&PowerCC26X2_module.notifyList))
    {
        /* point to first client notify object */
        elem = List_head(&PowerCC26X2_module.notifyList);

        /* walk the queue and notify each registered client of the event */
        do
        {
            if (((Power_NotifyObj *)elem)->eventTypes & eventType)
            {
                /* pull params from notify object */
                notifyFxn = ((Power_NotifyObj *)elem)->notifyFxn;
                clientArg = ((Power_NotifyObj *)elem)->clientArg;

                /* call the client's notification function */
                Log_printf(LogModule_Power,
                           Log_VERBOSE,
                           "notify: Invoking notification fxn at address 0x%x with event type 0x%x and clientArg 0x%x",
                           notifyFxn,
                           eventType,
                           clientArg);

                notifyStatus = (int_fast16_t)(*(Power_NotifyFxn)notifyFxn)(eventType, 0, clientArg);

                /* if client declared error stop all further notifications */
                if (notifyStatus != Power_NOTIFYDONE)
                {
                    Log_printf(LogModule_Power,
                               Log_WARNING,
                               "notify: Notification fxn reported error, fxn at address 0x%x with event type 0x%x and notifyStatus 0x%x",
                               notifyFxn,
                               eventType,
                               notifyStatus);

                    return (Power_EFAIL);
                }
            }

            /* get next element in the notification queue */
            elem = List_next(elem);

        } while (elem != NULL);
    }

    return (Power_SOK);
}

/*
 *  ======== configureRFCoreClocks ========
 *  Special dependency function for controlling RF core clocks.
 *  This function does nothing, but is kept for legacy reasons.
 *  All functionality has been integrated into the RF driver.
 */
static unsigned int configureRFCoreClocks(unsigned int action)
{
    return (0);
}

/*
 *  ======== switchXOSCHF ========
 *  Switching to XOSC_HF when it has stabilized.
 */
static void switchXOSCHF(void)
{
    bool readyToCal;
    unsigned int key;

    key = HwiP_disable();

    /* Attempt to switch to XOSC_HF. The call will check whether the
     * PENDINGSCLKHFSWITCHING bit is set before initiating the
     * switch.
     * - XOSC_HF xtal: This should succeed on first try since the OSC
     *   interrupt is triggered only when the XOSC_HF is ready to switch.
     * - XOSC_HF TCXO: At this point, the last stage of the clock qualification
     *   initiated by setting XOSCHFCTL_BYPASS is still ongoing and
     *   PENDINGSCLKHFSWITCHING is not set. It will take variable amount of
     *   time, but at least 10us, before the XOSC_HF is ready to switch.
     *   The loop ensures that we wait until it is ready.
     */
    while (PowerCC26X2_oschfTrySwitchToXosc() == false) {}

    /* The only time we should get here is when PowerCC26X2_module.xoscPending == true
     * holds.
     * Allow going into IDLE again since we sucessfully switched
     * to XOSC_HF
     */
    Power_releaseConstraint(PowerCC26XX_DISALLOW_IDLE);

    PowerCC26X2_module.xoscPending = false;

    HwiP_restore(key);

    /* initiate RCOSC calibration */
    readyToCal = (*(PowerCC26X2_config.calibrateFxn))(PowerCC26X2_INITIATE_CALIBRATE);

    /* now notify clients that were waiting for a switch notification */
    notify(PowerCC26XX_XOSC_HF_SWITCHED);

    /* if ready to start first cal measurment, do it now */
    if (readyToCal == true)
    {
        (*(PowerCC26X2_config.calibrateFxn))(PowerCC26X2_DO_CALIBRATE);
    }
}

/*
 *  ======== configureXOSCHF ========
 */
static unsigned int configureXOSCHF(unsigned int action)
{
    /* By checking action == PowerCC26XX_ENABLE and PowerCC26X2_module.xoscPending
     * carefully, the function should be idempotent. Calling it with the same
     * action more than once will not have any effect until the hardware triggers
     * a software state change.
     */
    if (action == PowerCC26XX_ENABLE && PowerCC26X2_oscClockSourceGet(OSC_SRC_CLK_HF) != OSC_XOSC_HF &&
        PowerCC26X2_module.xoscPending == false)
    {
        /* Check if TCXO is selected in CCFG and in addition configured to be enabled
         * by the function pointed to by PowerCC26X2_config.enableTCXOFxn.
         */
        if ((CCFGRead_XOSC_FREQ() == CCFGREAD_XOSC_FREQ_TCXO) && (PowerCC26X2_config.enableTCXOFxn != NULL))
        {
            Log_printf(LogModule_Power,
                       Log_INFO,
                       "configureXOSCHF: Enable TCXO qualification");

            PowerCC26X2_enableTCXOQual();

            /* Wait for ~10 us for common mode bias to stabilise and clock qual to take effect. */
            delayUs(TCXO_RAMP_DELAY);

            /* Enable power on TCXO */
            (*(PowerCC26X2_config.enableTCXOFxn))(true);

            /* Start clock to wait for TCXO startup time before clock switch
             * can be attempted.
             */
            ClockP_start(ClockP_handle(&PowerCC26X2_module.tcxoEnableClock));
        }
        else
        {
            /* Turn on and request XOSC_HF from the hardware for regular
             * XOSC and HPOSC. TCXO does not require this call until right
             * before switching since we do not rely on the harware to
             * interrupt the system once the XOSC is stable.
             */
            Log_printf(LogModule_Power,
                       Log_INFO,
                       "configureXOSCHF: Turn on XOSC_HF");

            PowerCC26X2_turnOnXosc();
        }

        PowerCC26X2_module.xoscPending = true;

        /* Unless it is disallowed, unmask the XOSC_HF ready to switch flag */
        if (!((CCFGRead_XOSC_FREQ() == CCFGREAD_XOSC_FREQ_TCXO) && (PowerCC26X2_config.enableTCXOFxn != NULL)))
        {
            if (!(Power_getConstraintMask() & (1 << PowerCC26XX_SWITCH_XOSC_HF_MANUALLY)))
            {

                /* Clearing the flag in the ISR does not always work. Clear it
                 * again just in case
                 * */
                HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCICR) = PRCM_OSCICR_HFSRCPENDC_M;

                /* Turn on oscillator interrupt for SCLK_HF switching */
                HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC) |= PRCM_OSCIMSC_HFSRCPENDIM_M;
            }
        }

        /* If the device goes into IDLE in between turning on XOSC_HF and
         * and switching SCLK_HF to XOSC_HF, the INT_OSC_COMB HFSRCPEND
         * trigger will be suppressed.
         * The DISALLOW_IDLE constraint should only ever be set whenever
         * we transition from xoscPending == false to true.
         */
        Power_setConstraint(PowerCC26XX_DISALLOW_IDLE);
    }

    /* when release XOSC_HF, auto switch to RCOSC_HF */
    else if (action == PowerCC26XX_DISABLE)
    {
        Log_printf(LogModule_Power, Log_INFO, "configureXOSCHF: Switch to RCOSC_HF");

        PowerCC26X2_oschfSwitchToRcosc();

        /* Handle TCXO if selected in CCFG */
        if ((CCFGRead_XOSC_FREQ() == CCFGREAD_XOSC_FREQ_TCXO) && (PowerCC26X2_config.enableTCXOFxn != NULL))
        {
            /* Disable Clock in case we have started it and are waiting for
             * the TCXO to stabilise.
             * If the Clock is not currently active, this should do nothing.
             */
            ClockP_stop(ClockP_handle(&PowerCC26X2_module.tcxoEnableClock));

            PowerCC26X2_disableTCXOQual();

            /* Check if function for enabling/disabling TCXO is supported */
            if (PowerCC26X2_config.enableTCXOFxn != NULL)
            {

                /* Disable TCXO by turning off power */
                Log_printf(LogModule_Power, Log_INFO, "configureXOSCHF: Disable TCXO");

                (*(PowerCC26X2_config.enableTCXOFxn))(false);
            }
        }

        /* If we have not actually switched to XOSC_HF yet, we need to
         * undo what we did above when turning on XOSC_HF. Otherwise,
         * we may not balance the constraints correctly or get
         * unexpected interrupts.
         */
        if (PowerCC26X2_module.xoscPending)
        {
            /* Remove HFSRCPEND from the OSC_COMB interrupt mask */
            uint32_t oscMask                                     = HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC);
            HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCIMSC) = oscMask & ~PRCM_OSCIMSC_HFSRCPENDIM_M;

            /* Clear any residual trigger for HFSRCPEND */
            HWREG(PRCM_BASE + NONSECURE_OFFSET + PRCM_O_OSCICR) = PRCM_OSCICR_HFSRCPENDC;

            Power_releaseConstraint(PowerCC26XX_DISALLOW_IDLE);

            PowerCC26X2_module.xoscPending = false;
        }
    }
    return (0);
}

/*
 *  ======== switchToTCXO ========
 *  Switching to TCXO after TCXO startup time has expired.
 */
static void switchToTCXO(void)
{
    /* Request XOSC_HF. In this instance, that is the TCXO and it will
     * immediately be ready to switch to after requesting since we turned it on
     * earlier with a GPIO and waited for it to stabilise.
     */
    Log_printf(LogModule_Power, Log_INFO, "switchToTCXO: Switch to TCXO");

    PowerCC26X2_switchToTCXO();

    /* Switch to TCXO */
    switchXOSCHF();
}
