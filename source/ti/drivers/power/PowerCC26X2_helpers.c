/*
 * Copyright (c) 2021-2023, Texas Instruments Incorporated - http://www.ti.com
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

#include <stdbool.h>
#include <stdint.h>

#include <ti/drivers/power/PowerCC26X2_helpers.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_aon_pmctl.h)
#include DeviceFamily_constructPath(inc/hw_ddi_0_osc.h)
#include DeviceFamily_constructPath(driverlib/ccfgread.h)
#include DeviceFamily_constructPath(driverlib/osc.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/setup_rom.h)

#if TFM_ENABLED
    #include <third_party/tfm/secure_fw/include/security_defs.h> /* __tz_c_veneer */
    #include <third_party/tfm/secure_fw/spm/include/utilities.h> /* tfm_core_panic() */
#else
    /* Define the gateway attributes to nothing instead of ifdefs on each function */
    #define __tz_c_veneer
#endif

/* Constants for SubSecInc values at different SCLK_LF frequencies */
#define SUBSECINC_31250_HZ 0x8637BD
#define SUBSECINC_32768_HZ 0x800000

/*
 *  ======== PowerCC26X2_oscCtlClearXtal ========
 *  Clears the XTAL bit in OSC_CTL0
 */
__tz_c_veneer void PowerCC26X2_oscCtlClearXtal(void)
{
    HWREG(AUX_DDI0_OSC_BASE + DDI_O_CLR + DDI_0_OSC_O_CTL0) = DDI_0_OSC_CTL0_XTAL_IS_24M;
}

/*
 *  ======== PowerCC26X2_pmctlDisableJtag ========
 *  Disables the JTAG power domain
 */
__tz_c_veneer void PowerCC26X2_pmctlDisableJtag(void)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_JTAGCFG) = 0;
}

/*
 *  ======== PowerCC26X2_sysctrlShutdownWithAbort ========
 *  Shuts down the device while
 */
__tz_c_veneer void PowerCC26X2_sysctrlShutdownWithAbort(void)
{
    SysCtrlShutdownWithAbort();
}

/*
 *  ======== PowerCC26X2_prcmEnableCacheRetention ========
 */
__tz_c_veneer void PowerCC26X2_prcmEnableCacheRetention(void)
{
    PRCMCacheRetentionEnable();
}

/*
 *  ======== PowerCC26X2_oschfSwitchToRcosc ========
 */
__tz_c_veneer void PowerCC26X2_oschfSwitchToRcosc(void)
{
    OSCHF_SwitchToRcOscTurnOffXosc();
}

/*
 *  ======== PowerCC26X2_oscClockSourceGet ========
 */
__tz_c_veneer uint32_t PowerCC26X2_oscClockSourceGet(uint32_t ui32SrcClk)
{
    return OSCClockSourceGet(ui32SrcClk);
}

/*
 *  ======== PowerCC26X2_oscDisableQualifiers ========
 */
__tz_c_veneer void PowerCC26X2_oscDisableQualifiers(void)
{
    /* yes, disable the LF clock qualifiers */
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                       DDI_0_OSC_O_CTL0,
                       DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_M | DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_M,
                       DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_S,
                       0x3);

    /* enable clock loss detection */
    OSCClockLossEventEnable();
}

/*
 *  ======== PowerCC26X2_getOscHfSourceReady ========
 */
__tz_c_veneer bool PowerCC26X2_getOscHfSourceReady(void)
{
    return OSCHfSourceReady();
}

/*
 *  ======== PowerCC26X2_oschfTrySwitchToXosc ========
 */
__tz_c_veneer uint32_t PowerCC26X2_oschfTrySwitchToXosc(void)
{
    /* Switch to the XOSC_HF. Since this function is only called after we get an
     * interrupt signifying it is ready to switch, it should always succeed. If
     * it does not succeed, the caller should retry.
     */
    return OSCHF_AttemptToSwitchToXosc();
}

/*
 *  ======== PowerCC26X2_oscIsHPOSCEnabledWithHfDerivedLfClock ========
 */
__tz_c_veneer bool PowerCC26X2_oscIsHPOSCEnabledWithHfDerivedLfClock(void)
{
    return OSC_IsHPOSCEnabledWithHfDerivedLfClock();
}

/*
 *  ======== PowerCC26X2_enableTCXOQual ========
 */
__tz_c_veneer void PowerCC26X2_enableTCXOQual(void)
{
    /* Enable clock qualification on 48MHz signal from TCXO */
    if (CCFGRead_TCXO_TYPE() == 0x1)
    {
        /* If the selected TCXO type is clipped-sine, also enable internal common-mode bias */
        HWREG(AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_XOSCHFCTL) = DDI_0_OSC_XOSCHFCTL_TCXO_MODE_XOSC_HF_EN |
                                                                       DDI_0_OSC_XOSCHFCTL_TCXO_MODE;
    }
    else
    {
        HWREG(AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_XOSCHFCTL) = DDI_0_OSC_XOSCHFCTL_TCXO_MODE;
    }
}

/*
 *  ======== PowerCC26X2_disableTCXOQual ========
 */
__tz_c_veneer void PowerCC26X2_disableTCXOQual(void)
{
    /* Disable clock qualification on 48MHz signal from TCXO and turn
     * off TCXO bypass.
     * If we do not disable clock qualificaition, it will not run the
     * next time we switch to TCXO.
     */
    if (CCFGRead_TCXO_TYPE() == 1)
    {
        /* Also turn off bias if clipped sine TCXO type. The bias
         * consumes a few hundred uA. That is fine while the TCXO is
         * running but we should not incur this penalty when not running
         * on TCXO.
         */
        HWREG(AUX_DDI0_OSC_BASE + DDI_O_CLR + DDI_0_OSC_O_XOSCHFCTL) = DDI_0_OSC_XOSCHFCTL_TCXO_MODE |
                                                                       DDI_0_OSC_XOSCHFCTL_BYPASS |
                                                                       DDI_0_OSC_XOSCHFCTL_TCXO_MODE_XOSC_HF_EN;
    }
    else
    {
        HWREG(AUX_DDI0_OSC_BASE + DDI_O_CLR + DDI_0_OSC_O_XOSCHFCTL) = DDI_0_OSC_XOSCHFCTL_TCXO_MODE |
                                                                       DDI_0_OSC_XOSCHFCTL_BYPASS;
    }
}

/*
 *  ======== PowerCC26X2_turnOnXosc ========
 */
__tz_c_veneer void PowerCC26X2_turnOnXosc(void)
{
    OSCHF_TurnOnXosc();
}

/*
 *  ======== PowerCC26X2_switchToTCXO ========
 *  Switching to TCXO after TCXO startup time has expired.
 */
__tz_c_veneer void PowerCC26X2_switchToTCXO(void)
{
    /* Set bypass bit */
    HWREG(AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_XOSCHFCTL) = DDI_0_OSC_XOSCHFCTL_BYPASS;

    /* Request XOSC_HF. In this instance, that is the TCXO. */
    OSCHF_TurnOnXosc();
}

/*
 *  ======== PowerCC26X2_sysCtrlStandby ========
 */
__tz_c_veneer void PowerCC26X2_sysCtrlStandby(bool retainCache)
{
    SysCtrlStandby(retainCache, VIMS_ON_CPU_ON_MODE, SYSCTRL_PREFERRED_RECHARGE_MODE);
}

/*
 *  ======== PowerCC26X2_sysCtrlVoltageConditionalControl ========
 */
__tz_c_veneer void PowerCC26X2_sysCtrlVoltageConditionalControl(void)
{
    SysCtrl_DCDC_VoltageConditionalControl();
}

/*
 *  ======== PowerCC26X2_sysCtrlIdle ========
 */
__tz_c_veneer void PowerCC26X2_sysCtrlIdle(uint32_t vimsPdMode)
{
    SysCtrlIdle(vimsPdMode);
}

#if TFM_ENABLED
/*
 *  ======== PowerCC26X2_isSecurePeriph ========
 */
static bool PowerCC26X2_isSecurePeriph(uint32_t prcmPeriph)
{
    bool isSecurePeriph = false;

    if ((prcmPeriph == PRCM_PERIPH_PKA) || (prcmPeriph == PRCM_PERIPH_CRYPTO) || (prcmPeriph == PRCM_PERIPH_TRNG) ||
        (prcmPeriph == PRCM_PERIPH_UDMA))
    {
        isSecurePeriph = true;
    }

    return isSecurePeriph;
}
#endif

/*
 *  ======== PowerCC26X2_setPeriphDependency ========
 *  @param prcmPeriph   Must be set to a PRCM_PERIPH_XXXX define
 */
__tz_c_veneer void PowerCC26X2_setPeriphDependency(uint32_t prcmPeriph)
{
#if TFM_ENABLED
    if (!PowerCC26X2_isSecurePeriph(prcmPeriph))
    {
        tfm_core_panic();
    }
#endif

    PRCMPeripheralRunEnable(prcmPeriph);
    PRCMPeripheralSleepEnable(prcmPeriph);
    PRCMPeripheralDeepSleepEnable(prcmPeriph);

    PRCMLoadSet();
    while (!PRCMLoadGet()) {}
}

/*
 *  ======== PowerCC26X2_releasePeriphDependency ========
 *  @param prcmPeriph   Must be set to a PRCM_PERIPH_XXXX define
 */
__tz_c_veneer void PowerCC26X2_releasePeriphDependency(uint32_t prcmPeriph)
{
#if TFM_ENABLED
    if (!PowerCC26X2_isSecurePeriph(prcmPeriph))
    {
        tfm_core_panic();
    }
#endif

    PRCMPeripheralRunDisable(prcmPeriph);
    PRCMPeripheralSleepDisable(prcmPeriph);
    PRCMPeripheralDeepSleepDisable(prcmPeriph);

    PRCMLoadSet();
    while (!PRCMLoadGet()) {}
}

/*
 *  ======== PowerCC26X2_setPeriphDeepSleepEnable ========
 *  @param prcmPeriph   Must be set to a PRCM_PERIPH_XXXX define
 */
__tz_c_veneer void PowerCC26X2_setPeriphDeepSleepEnable(uint32_t prcmPeriph)
{
#if TFM_ENABLED
    if (!PowerCC26X2_isSecurePeriph(prcmPeriph))
    {
        tfm_core_panic();
    }
#endif

    PRCMPeripheralDeepSleepEnable(prcmPeriph);
}

/*
 *  ======== PowerCC26X2_sysCtrlGetResetSource ========
 */
__tz_c_veneer uint32_t PowerCC26X2_sysCtrlGetResetSource(void)
{
    return SysCtrlResetSourceGet();
}

/*
 *  ======== PowerCC26X2_setSubSecIncToXoscLf ========
 */
__tz_c_veneer void PowerCC26X2_setSubSecIncToXoscLf(uint32_t subsecinc)
{
    /* We only want to set SubSecInc if we are running on XOSC_LF */
    if (CCFGRead_SCLK_LF_OPTION() == CCFGREAD_SCLK_LF_OPTION_XOSC_LF)
    {
        /* Check that the argument is within allowed limits. The range is
         * the nominal RTC increment value (0x800000) +/- 2%. This is to
         * allow RTC SWTCXO to set the value within a reasonable range,
         * without allowing arbitrary values to be written
         */
        if (subsecinc > 0x7D70A3 && subsecinc < 0x828F5C)
        {
            /* Set SubSecInc back to 32.768 kHz, or a compensated value
             * if RTC SWTCXO is enabled. In that case, the argument will
             * be an already temperature-adjusted value
             */
            SetupSetAonRtcSubSecInc(subsecinc);
        }
    }
}

/*
 *  ======== PowerCC26X2_sysCtrlUpdateVoltageRegulator ========
 */
__tz_c_veneer void PowerCC26X2_sysCtrlUpdateVoltageRegulator(void)
{
    SysCtrl_DCDC_VoltageConditionalControl();
}

/*
 *  ======== PowerCC26X2_sysCtrlReset ========
 */
__tz_c_veneer void PowerCC26X2_sysCtrlReset(void)
{
    SysCtrlSystemReset();
}