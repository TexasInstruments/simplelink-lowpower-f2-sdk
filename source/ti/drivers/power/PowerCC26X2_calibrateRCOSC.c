/*
 * Copyright (c) 2017-2023, Texas Instruments Incorporated
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
 *  ======== PowerCC26X2_calibrateRCOSC.c ========
 */

#include <stdbool.h>

#include <ti/drivers/dpl/HwiP.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/power/PowerCC26X2_helpers.h>

#include <ti/drivers/utils/Math.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_aux_evctl.h)
#include DeviceFamily_constructPath(inc/hw_aux_smph.h)
#include DeviceFamily_constructPath(inc/hw_aux_sysif.h)
#include DeviceFamily_constructPath(inc/hw_aux_tdc.h)
#include DeviceFamily_constructPath(inc/hw_ddi_0_osc.h)
#include DeviceFamily_constructPath(inc/hw_ddi.h)
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/ddi.h)
#include DeviceFamily_constructPath(driverlib/ioc.h)
#include DeviceFamily_constructPath(driverlib/osc.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)

#define AUX_TDC_SEMAPHORE_NUMBER        1  /* semaphore 1 protects TDC */
#define NUM_RCOSC_LF_PERIODS_TO_MEASURE 32 /* x RCOSC_LF periods vs XOSC_HF */
#define NUM_RCOSC_HF_PERIODS_TO_MEASURE 1  /* x RCOSC_HF periods vs XOSC_HF */
#define ACLK_REF_SRC_RCOSC_HF           0  /* Use RCOSC_HF for ACLK REF */
#define ACLK_REF_SRC_RCOSC_LF           2  /* Use RCOSC_LF for ACLK REF */
#define SCLK_LF_OPTION_RCOSC_LF         3  /* defined in cc26_ccfg.xls */
#define RCOSC_HF_LOW_THRESHOLD_TDC_VALUE \
    1535 /* If TDC value is within threshold range, no need for another TDC measurement */
#define RCOSC_HF_PERFECT_TDC_VALUE 1536 /* RCOSC_HF runs at perfect 48 MHz when ending up with this TDC value */
#define RCOSC_HF_HIGH_THRESHOLD_TDC_VALUE \
    1537 /* If TDC value is within threshold range, no need for another TDC measurement */

/* AUX ISR states */
#define WAIT_SMPH     0 /* just took SMPH, start RCOSC_LF */
#define CAL_RCOSC_LF  1 /* just finished RCOSC_LF, start first RCOSC_HF */
#define CAL_RCOSC_HF1 2 /* just finished 1st RCOSC_HF, start 2nd */
#define CAL_RCOSC_HF2 3 /* just finished 2nd RCOSC_HF, decide best */

/* calibration states */
#define PowerCC26X2_STATE_TDC_INIT  0
#define PowerCC26X2_STATE_CAL_LF_1  1
#define PowerCC26X2_STATE_CAL_LF_2  2
#define PowerCC26X2_STATE_CAL_HF1_1 3
#define PowerCC26X2_STATE_CAL_HF1_2 4
#define PowerCC26X2_STATE_CAL_HF2   5
#define PowerCC26X2_STATE_CLEANUP   6

/* FSM results */
typedef enum
{
    PowerCC26X2_FSM_RESULT_RUN_FSM,
    PowerCC26X2_FSM_RESULT_WAIT_FOR_TDC,
    PowerCC26X2_FSM_RESULT_DONE,
    PowerCC26X2_FSM_RESULT_ERROR,
} PowerCC26X2_FsmResult;

/* macros */
#define Scale_rndInf(x) ((3 * (x) + (((x) < 0) ? -2 : 2)) / 4)

#ifndef PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
    #define PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION 0
#endif

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
volatile unsigned int gotSEM  = 0;
volatile unsigned int calLFi  = 0;
volatile unsigned int calHF1i = 0;
volatile unsigned int calHF2i = 0;
volatile bool doneCal         = false;
unsigned int tdcResult_LF     = 0;
unsigned int tdcResult_HF1    = 0;
unsigned int tdcResult_HF2    = 0;
unsigned int numISRs          = 0;
#endif

/* Forward declarations */
static bool getTdcSemaphore(void);
static void calibrateRcoscHf1(int32_t tdcResult);
static void calibrateRcoscHf2(int32_t tdcResult);
static PowerCC26X2_FsmResult runCalibrateFsm(void);
void PowerCC26X2_calibrate(void);
void PowerCC26X2_RCOSC_clockFunc(uintptr_t arg);

/* Externs */
extern PowerCC26X2_ModuleState PowerCC26X2_module;
extern const PowerCC26X2_Config PowerCC26X2_config;

/*
 *  ======== PowerCC26X2_initiateCalibration ========
 *  Initiate calibration of RCOSC_LF and RCOSCHF
 */
bool PowerCC26X2_initiateCalibration(void)
{
    unsigned int hwiKey;
    bool busy = false;
    bool status;
    bool gotSem;

    if ((PowerCC26X2_module.calLF == false) && (PowerCC26X2_config.calibrateRCOSC_HF == false))
    {
        return (false);
    }

    /* make sure calibration is not already in progress */
    hwiKey = HwiP_disable();

    if (PowerCC26X2_module.busyCal == false)
    {
        PowerCC26X2_module.busyCal = true;
    }
    else
    {
        busy = true;
    }

    HwiP_restore(hwiKey);

    if (busy == true)
    {
        return (false);
    }

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
    gotSEM  = 0;
    calLFi  = 0;
    calHF1i = 0;
    calHF2i = 0;
    doneCal = false;
#endif

    /* set contraint to prohibit standby during calibration sequence */
    Power_setConstraint(PowerCC26XX_DISALLOW_STANDBY);

    /* set dependency to keep XOSC_HF active during calibration sequence */
    Power_setDependency(PowerCC26XX_XOSC_HF);

    /* initiate acquisition of semaphore protecting TDC */
    gotSem = getTdcSemaphore();

    /* if didn't acquire semaphore, must wait for autotake ISR */
    if (gotSem == false)
    {
        PowerCC26X2_module.auxHwiState = WAIT_SMPH;
        status                         = false; /* false: don't do anything else until acquire SMPH */
    }

    /* else, semaphore acquired, OK to proceed with first measurement */
    else
    {
#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
        gotSEM = 1;
#endif
        status = true; /* true: OK to start first measurement */
    }

    return (status);
}

/*
 *  ======== PowerCC26X2_auxISR ========
 *  ISR for the AUX combo interrupt event.  Implements Hwi state machine to
 *  step through the RCOSC calibration steps.
 */
void PowerCC26X2_auxISR(uintptr_t arg)
{
    uint32_t tdcResult;

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
    numISRs++;
#endif

    /*
     * disable all events that are part of AUX_COMBINED_INTERRUPT.
     * This interrupt is reserved for use during RCOSC calibration.
     * Other AUX perihperals that want to generate interrupts to CM3
     * must use dedicated interrupt lines or go through AON combined.
     */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) = 0;

    /* ****** state = WAIT_SMPH: arrive here if just took the SMPH ****** */
    if (PowerCC26X2_module.auxHwiState == WAIT_SMPH)
    {
#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
        gotSEM = 1;
#endif
    }

    /* **** state = CAL_RCOSC_LF: here when just finished LF counting **** */
    else if (PowerCC26X2_module.auxHwiState == CAL_RCOSC_LF)
    {
        /* update the RTC SUBSECINC register based on LF measurement result */
        PowerCC26X2_updateSubSecInc(PowerCC26X2_module.firstLF);
        /* No longer first measurement */
        PowerCC26X2_module.firstLF = false;

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
        calLFi = 1;
#endif
        /* if doing HF calibration initiate it now */
        if (PowerCC26X2_config.calibrateRCOSC_HF)
        {
            PowerCC26X2_module.calStep = PowerCC26X2_STATE_CAL_LF_2; /* next: trigger LF */
        }

        /* else, start cleanup */
        else
        {
            PowerCC26X2_module.calStep = PowerCC26X2_STATE_CLEANUP; /* next: cleanup */
        }
    }

    /* ****** state = CAL_RCOSC_HF1: here when just finished 1st RCOSC_HF */
    else if (PowerCC26X2_module.auxHwiState == CAL_RCOSC_HF1)
    {

        tdcResult = HWREG(AUX_TDC_BASE + AUX_TDC_O_RESULT);

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
        tdcResult_HF1 = tdcResult;
        calHF1i       = 1;
#endif

        /* use first HF measurement to setup new trim values */
        calibrateRcoscHf1(tdcResult);

        /* if HF setting perfect, nothing more to do, calibration is done */
        if ((tdcResult >= RCOSC_HF_LOW_THRESHOLD_TDC_VALUE) && (tdcResult <= RCOSC_HF_HIGH_THRESHOLD_TDC_VALUE))
        {
            PowerCC26X2_module.calStep = PowerCC26X2_STATE_CLEANUP; /* next: cleanup */
        }

        /* else, tweak trims, initiate another HF measurement */
        else
        {

            PowerCC26X2_module.calStep = PowerCC26X2_STATE_CAL_HF1_2; /* next: HF meas. #2 */
        }
    }

    /* ****** state = just finished second RCOSC_HF measurement ****** */
    else if (PowerCC26X2_module.auxHwiState == CAL_RCOSC_HF2)
    {

        tdcResult = HWREG(AUX_TDC_BASE + AUX_TDC_O_RESULT);

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
        tdcResult_HF2 = tdcResult;
#endif
        /* look for improvement on #2, else revert to previous trim values */
        calibrateRcoscHf2(tdcResult);

        PowerCC26X2_module.calStep = PowerCC26X2_STATE_CLEANUP; /* next: cleanup */
    }

    /* do the next calibration step... */
    PowerCC26X2_calibrate();
}

/*
 *  ======== PowerCC26X2_calibrate ========
 */
void PowerCC26X2_calibrate(void)
{
    PowerCC26X2_FsmResult fsmResult;

    do
    {
        fsmResult = runCalibrateFsm();
    } while (fsmResult == PowerCC26X2_FSM_RESULT_RUN_FSM);

    switch (fsmResult)
    {
        case PowerCC26X2_FSM_RESULT_WAIT_FOR_TDC:
            /* Intentional fall-through */
        case PowerCC26X2_FSM_RESULT_DONE:
            /* Do nothing. Calibration is complete or the
             * TDC harware will execute in the background
             * and continue the operation. */
            break;
        default:
            /* Something went wrong. No good way to recover. */
            while (1) {}
    }
}

/*
 *  ======== runCalibrateFsm ========
 *  Execute one state of the clock calibration FSM.
 */
static PowerCC26X2_FsmResult runCalibrateFsm(void)
{

    switch (PowerCC26X2_module.calStep)
    {

        case PowerCC26X2_STATE_TDC_INIT:

            /* set saturation config to 2^24 */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_SATCFG) = AUX_TDC_SATCFG_LIMIT_R24;

            /* set start and stop trigger sources and polarity */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGSRC) = (AUX_TDC_TRIGSRC_STOP_SRC_ACLK_REF | AUX_TDC_TRIGSRC_STOP_POL_HIGH) |
                                           (AUX_TDC_TRIGSRC_START_SRC_ACLK_REF | AUX_TDC_TRIGSRC_START_POL_HIGH);

            /* set TDC_SRC clock to be XOSC_HF/2 = 24 MHz */
            PowerCC26X2_setTdcClkSrc24M();

            /* set AUX_SYSIF:TDCCLKCTL.REQ... */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCCLKCTL) = AUX_SYSIF_TDCCLKCTL_REQ;

            /* finish wait for AUX_SYSIF:TDCCLKCTL.ACK to be set ... */
            while (!(HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCCLKCTL) & AUX_SYSIF_TDCCLKCTL_ACK)) {}

            /* Enable trig count */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGCNTCFG) = AUX_TDC_TRIGCNTCFG_EN;

            /* if LF calibration enabled start LF measurement */
            if (PowerCC26X2_module.calLF)
            {

                /* clear UPD_REQ, new sub-second increment is NOT available */
                HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_RTCSUBSECINCCTL) = 0;

                /* set next Swi state */
                PowerCC26X2_module.calStep = PowerCC26X2_STATE_CAL_LF_1;
            }

            /* else, start first HF measurement */
            else
            {
                /* set next Swi state */
                PowerCC26X2_module.calStep = PowerCC26X2_STATE_CAL_HF1_1;
            }

            /* abort TDC */
            HWREG(AUX_TDC_BASE + AUX_TDC_O_CTL) = AUX_TDC_CTL_CMD_ABORT;

            /* clear AUX_SYSIFTDCREFCLKCTL.REQ... */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = 0;

            /* finish wait for AUX_SYSIFTDCREFCLKCTL.ACK to be cleared ... */
            while (HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) & AUX_SYSIF_TDCREFCLKCTL_ACK) {}

            return PowerCC26X2_FSM_RESULT_RUN_FSM;

        case PowerCC26X2_STATE_CAL_LF_1:

            /* set next Hwi state before triggering TDC */
            PowerCC26X2_module.auxHwiState = CAL_RCOSC_LF;

            /* set the ACLK reference clock */
            PowerCC26X2_setAclkRefSrc(ACLK_REF_SRC_RCOSC_LF);

            /* set AUX_SYSIFTDCREFCLKCTL.REQ */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = AUX_SYSIF_TDCREFCLKCTL_REQ;

            /* Delay for ~110us total until TDCRECLKCTL_ACK is ready */
            ClockP_start(ClockP_handle(&PowerCC26X2_module.calibrationClock));

            return PowerCC26X2_FSM_RESULT_WAIT_FOR_TDC;

        case PowerCC26X2_STATE_CAL_LF_2:

            PowerCC26X2_module.calStep = PowerCC26X2_STATE_CAL_HF1_1;

            /* clear AUX_SYSIFTDCREFCLKCTL.REQ... */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = 0;

            /* wait for AUX_SYSIFTDCREFCLKCTL.ACK to be cleared ... */
            while (HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) & AUX_SYSIF_TDCREFCLKCTL_ACK) {}

            return PowerCC26X2_FSM_RESULT_RUN_FSM;

        case PowerCC26X2_STATE_CAL_HF1_1:

            PowerCC26X2_module.auxHwiState = CAL_RCOSC_HF1;

            /* set the ACLK reference clock */
            PowerCC26X2_setAclkRefSrc(ACLK_REF_SRC_RCOSC_HF);

            /* set AUX_SYSIFTDCREFCLKCTL.REQ */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = AUX_SYSIF_TDCREFCLKCTL_REQ;

            /* Delay for ~110us total until TDCRECLKCTL_ACK is ready */
            ClockP_start(ClockP_handle(&PowerCC26X2_module.calibrationClock));

            return PowerCC26X2_FSM_RESULT_WAIT_FOR_TDC;

        case PowerCC26X2_STATE_CAL_HF1_2:

            PowerCC26X2_module.calStep = PowerCC26X2_STATE_CAL_HF2;

            /* clear AUX_SYSIFTDCREFCLKCTL.REQ... */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = 0;

            /* wait for AUX_SYSIFTDCREFCLKCTL.ACK to be cleared ... */
            while (HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) & AUX_SYSIF_TDCREFCLKCTL_ACK) {}

            return PowerCC26X2_FSM_RESULT_RUN_FSM;

        case PowerCC26X2_STATE_CAL_HF2:

            PowerCC26X2_module.auxHwiState = CAL_RCOSC_HF2;

            /* set the ACLK reference clock */
            PowerCC26X2_setAclkRefSrc(ACLK_REF_SRC_RCOSC_HF);

            /* set AUX_SYSIFTDCREFCLKCTL.REQ */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = AUX_SYSIF_TDCREFCLKCTL_REQ;

            /* Delay for ~110us total until TDCRECLKCTL_ACK is ready */
            ClockP_start(ClockP_handle(&PowerCC26X2_module.calibrationClock));

            return PowerCC26X2_FSM_RESULT_WAIT_FOR_TDC;

        case PowerCC26X2_STATE_CLEANUP:

            /* release the TDC clock request */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCCLKCTL) = 0;

            /* release the TDC reference clock request */
            HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) = 0;

            /* wait for AUX_SYSIF:TDCCLKCTL.ACK to be cleared ... */
            while ((HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCCLKCTL) & AUX_SYSIF_TDCCLKCTL_ACK)) {}
            /* wait for AUX_SYSIFTDCREFCLKCTL.ACK to be cleared ... */
            while (HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) & AUX_SYSIF_TDCREFCLKCTL_ACK) {}

            /*
             * Disable all interrupts as part of AUX_COMBINED interrupt
             * Once we release semaphore, the sensor controller is allowed
             * to use the TDC. When it does, we must ensure that this
             * does not cause any unexpected interrupts to the CM3.
             */
            HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) = 0;

            /* release AUX semaphore */
            HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH1) = 1;

            /* release the power down constraints and XOSC_HF dependency */
            Power_releaseDependency(PowerCC26XX_XOSC_HF);
            Power_releaseConstraint(PowerCC26XX_DISALLOW_STANDBY);

            /* set next state */
            PowerCC26X2_module.calStep = PowerCC26X2_STATE_TDC_INIT;

#if PowerCC26X2_INSTRUMENT_RCOSC_CALIBRATION
            doneCal = true;
            calHF2i = 1;
#endif
            PowerCC26X2_module.busyCal = false;

            return PowerCC26X2_FSM_RESULT_DONE;

        default:
            return PowerCC26X2_FSM_RESULT_ERROR;
    }
}

void PowerCC26X2_RCOSC_clockFunc(uintptr_t arg)
{

    /* Wait any remaining time for TDCREFCLKCTL_ACK. Should not spin here at all. */
    while (!(HWREG(AUX_SYSIF_BASE + NONSECURE_OFFSET + AUX_SYSIF_O_TDCREFCLKCTL) & AUX_SYSIF_TDCREFCLKCTL_ACK)) {}

    /* Set number of periods of ACLK to count */
    if (PowerCC26X2_module.calStep == PowerCC26X2_STATE_CAL_LF_1)
    {
        HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGCNTLOAD) = NUM_RCOSC_LF_PERIODS_TO_MEASURE;
    }
    else
    {
        HWREG(AUX_TDC_BASE + AUX_TDC_O_TRIGCNTLOAD) = NUM_RCOSC_HF_PERIODS_TO_MEASURE;
    }

    /* Reset/clear result of TDC */
    HWREG(AUX_TDC_BASE + AUX_TDC_O_CTL) = AUX_TDC_CTL_CMD_CLR_RESULT;

    /* Clear possible pending interrupt source */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGSCLR) = AUX_EVCTL_EVTOMCUFLAGSCLR_AUX_TDC_DONE;

    /* Enable TDC done interrupt as part of AUX_COMBINED interrupt */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) = AUX_EVCTL_EVTOMCUFLAGSCLR_AUX_TDC_DONE;

    /* Run TDC (start synchronously) */
    HWREG(AUX_TDC_BASE + AUX_TDC_O_CTL) = AUX_TDC_CTL_CMD_RUN_SYNC_START;
}

/*
 *  ======== getTdcSemaphore ========
 *  Get TDC semaphore (number 1)
 */
static bool getTdcSemaphore(void)
{
    unsigned int own;

    /* try to acquire SMPH */
    own = HWREG(AUX_SMPH_BASE + AUX_SMPH_O_SMPH1);

    /* if acquired SMPH: done */
    if (own != 0)
    {
        return (true);
    }

    /* clear the interrupt source, can only be cleared when we don't have semaphore */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGSCLR) = AUX_EVCTL_EVTOMCUFLAGSCLR_AUX_SMPH_AUTOTAKE_DONE;

    /*
     * else, did not acquire the semaphore, enable SMPH_AUTOTAKE_DONE event
     * (don't OR, write entire register, no other interrupts can be enabled!)
     */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_COMBEVTOMCUMASK) = AUX_EVCTL_COMBEVTOMCUMASK_AUX_SMPH_AUTOTAKE_DONE;

    /* start AUTOTAKE of semaphore for TDC access */
    HWREG(AUX_SMPH_BASE + AUX_SMPH_O_AUTOTAKE) = AUX_TDC_SEMAPHORE_NUMBER;

    return (false);
}

/*
 *  ======== PowerCC26X2_calibrateRcoscHf1 ========
 *  Calibrate RCOSC_HF agains XOSC_HF: compute and setup new trims
 */
static void calibrateRcoscHf1(int32_t tdcResult)
{
    /* *** STEP 1: Find RCOSC_HF-XOSC_HF frequency offset with current trim settings */
    /* Read in current trim settings */
    PowerCC26X2_module.nCtrimCurr      = PowerCC26X2_readCtrim();
    PowerCC26X2_module.nCtrimFractCurr = PowerCC26X2_readCtrimFract();
    PowerCC26X2_module.nRtrimCurr      = PowerCC26X2_readRtrim();

    /*
     * Find RCOSC_HF-XOSC_HF frequency offset with current trim settings
     *   Positive value => RCOSC_HF runs slow, CTRIM(FRACT) should be increased
     *   Negative value => RCOSC_HF runs fast, CTRIM(FRACT) should be decreased
     * Resolution: 31.25 kHz; CTRIMFRACT resolution ~30 kHz
     */
    PowerCC26X2_module.nDeltaFreqCurr = (int32_t)tdcResult - RCOSC_HF_PERFECT_TDC_VALUE;

    /* *** STEP 2: Attempt to calculate more optimal settings */
    if (PowerCC26X2_module.nDeltaFreqCurr == 0)
    {
        /* If perfect, don't perform second measurement and keep current settings */
        PowerCC26X2_module.bRefine = false;
        return;
    }
    if (PowerCC26X2_module.bRefine)
    {
        /*
         * Trying to find better match across CTRIM/RTRIM. Due to mismatches the
         * first try might not have been more optimal than the current setting.
         * Continue refining, starting from stored values
         */
    }
    else
    {
        /* Start from current values */
        PowerCC26X2_module.nCtrimFractNew = PowerCC26X2_module.nCtrimFractCurr;
        PowerCC26X2_module.nCtrimNew      = PowerCC26X2_module.nCtrimCurr;
        PowerCC26X2_module.nRtrimNew      = PowerCC26X2_module.nRtrimCurr;
        PowerCC26X2_module.nDeltaFreqNew  = PowerCC26X2_module.nDeltaFreqCurr;
    }

    /*
     * Calculate change to CTRIMFRACT with safe assumptions of gain,
     * apply delta to current CTRIMFRACT and convert to valid CTRIM/CTRIMFRACT
     */
    PowerCC26X2_module.nCtrimFractNew = PowerCC26X2_module.nCtrimFractNew +
                                        Scale_rndInf(PowerCC26X2_module.nDeltaFreqNew);
    PowerCC26X2_module.nCtrimNew = PowerCC26X2_module.nCtrimCurr;

    /* One step of CTRIM is about 500 kHz, so limit to one CTRIM step */
    if (PowerCC26X2_module.nCtrimFractNew < 1)
    {
        if (PowerCC26X2_module.nRtrimNew == 3)
        {
            /* We try the slow RTRIM in this CTRIM first */
            PowerCC26X2_module.nCtrimFractNew = Math_MAX(1, PowerCC26X2_module.nCtrimFractNew + 21);
            PowerCC26X2_module.nRtrimNew      = 0;
        }
        else
        {
            /* Step down one CTRIM and use fast RTRIM */
            PowerCC26X2_module.nCtrimFractNew = Math_MAX(1, PowerCC26X2_module.nCtrimFractNew + 32 - 21);
            PowerCC26X2_module.nCtrimNew      = Math_MAX(0, PowerCC26X2_module.nCtrimNew - 1);
            PowerCC26X2_module.nRtrimNew      = 3;
        }
    }
    else if (PowerCC26X2_module.nCtrimFractNew > 30)
    {
        if (PowerCC26X2_module.nRtrimNew == 0)
        {
            /* We try the slow RTRIM in this CTRIM first */
            PowerCC26X2_module.nCtrimFractNew = Math_MIN(30, PowerCC26X2_module.nCtrimFractNew - 21);
            PowerCC26X2_module.nRtrimNew      = 3;
        }
        else
        {
            /* Step down one CTRIM and use fast RTRIM */
            PowerCC26X2_module.nCtrimFractNew = Math_MIN(30, PowerCC26X2_module.nCtrimFractNew - 32 + 21);
            PowerCC26X2_module.nCtrimNew      = Math_MIN(0x3F, PowerCC26X2_module.nCtrimNew + 1);
            PowerCC26X2_module.nRtrimNew      = 0;
        }
    }
    else
    {
        /* We're within sweet spot of current CTRIM => no change */
    }

    /* Find RCOSC_HF vs XOSC_HF frequency offset with new trim settings */
    PowerCC26X2_writeCtrim(PowerCC26X2_module.nCtrimNew);

    /* Enable RCOSCHFCTRIMFRACT_EN */
    PowerCC26X2_writeCtrimFractEn(1);

    /* Modify CTRIM_FRACT */
    PowerCC26X2_writeCtrimFract(PowerCC26X2_module.nCtrimFractNew);

    /* Modify RTRIM */
    PowerCC26X2_writeRtrim(PowerCC26X2_module.nRtrimNew);
}

/*
 *  ======== Power_calibrateRcoscHf2 ========
 *  Calibrate RCOSC_HF agains XOSC_HF: determine better result, set new trims
 */
static void calibrateRcoscHf2(int32_t tdcResult)
{
    PowerCC26X2_module.nDeltaFreqNew = (int32_t)tdcResult - RCOSC_HF_PERFECT_TDC_VALUE;
    /* Calculate new delta freq */

    /* *** STEP 4: Determine whether the new settings are better or worse */
    if (Math_ABS(PowerCC26X2_module.nDeltaFreqNew) <= Math_ABS(PowerCC26X2_module.nDeltaFreqCurr))
    {
        /* New settings are better or same -> make current by keeping in registers */
        PowerCC26X2_module.bRefine = false;
    }
    else
    {
        /* First measurement was better than second, restore current settings */
        PowerCC26X2_writeCtrim(PowerCC26X2_module.nCtrimCurr);
        PowerCC26X2_writeCtrimFract(PowerCC26X2_module.nCtrimFractCurr);
        PowerCC26X2_writeRtrim(PowerCC26X2_module.nRtrimCurr);

        /* Enter a refinement mode where we keep searching for better matches */
        PowerCC26X2_module.bRefine = true;
    }
}
