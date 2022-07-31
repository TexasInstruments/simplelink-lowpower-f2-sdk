/*
 * Copyright (c) 2021-2022 Texas Instruments Incorporated - http://www.ti.com
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti/drivers/Temperature.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_ddi_0_osc.h)
#include DeviceFamily_constructPath(driverlib/prcm.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/ccfgread.h)

#if SPE_ENABLED
    #include <third_party/tfm/interface/include/tfm_api.h>
    #include <third_party/tfm/secure_fw/spm/include/tfm_secure_api.h>
    #include "system_cc26x4.h"
    #include "secure_utilities.h"
#else
    /* Define the gateway attributes to nothing instead of ifdefs on each function */
    #define __tfm_secure_gateway_attributes__
#endif

/*
 *  ======== PowerCC26X2_oscCtlClearXtal ========
 *  Clears the XTAL bit in OSC_CTL0
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_oscCtlClearXtal(void)
{
    HWREG(AUX_DDI0_OSC_BASE + DDI_O_CLR + DDI_0_OSC_O_CTL0) = DDI_0_OSC_CTL0_XTAL_IS_24M;
}

/*
 *  ======== PowerCC26X2_pmctlDisableJtag ========
 *  Disables the JTAG power domain
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_pmctlDisableJtag(void)
{
    HWREG(AON_PMCTL_BASE + AON_PMCTL_O_JTAGCFG) = 0;
}

/*
 *  ======== PowerCC26X2_sysctrlShutdownWithAbort ========
 *  Shuts down the device while
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_sysctrlShutdownWithAbort(void)
{
    SysCtrlShutdownWithAbort();
}

/*
 *  ======== PowerCC26X2_prcmEnableCacheRetention ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_prcmEnableCacheRetention(void)
{
    PRCMCacheRetentionEnable();
}

/*
 *  ======== PowerCC26X2_oschfSwitchToRcosc ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_oschfSwitchToRcosc(void)
{
    OSCHF_SwitchToRcOscTurnOffXosc();
}

/*
 *  ======== PowerCC26X2_oscClockSourceGet ========
 */
__tfm_secure_gateway_attributes__ uint32_t PowerCC26X2_oscClockSourceGet(uint32_t ui32SrcClk)
{
    return OSCClockSourceGet(ui32SrcClk);
}

/*
 *  ======== PowerCC26X2_oscDisableQualifiers ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_oscDisableQualifiers(void)
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
 *  ======== PowerCC26X2_oschfTrySwitchToXosc ========
 *  Attempts twice to switch to XOSC_HF, returns 0 on failure.
 */
__tfm_secure_gateway_attributes__ uint32_t PowerCC26X2_oschfTrySwitchToXosc(void)
{
    /* Switch to the XOSC_HF. Since this function is only called
     * after we get an interrupt signifying it is ready to switch,
     * it should always succeed. If it does not succeed, try once more.
     */
    return OSCHF_AttemptToSwitchToXosc();
}

/*
 *  ======== PowerCC26X2_oscIsHPOSCEnabledWithHfDerivedLfClock ========
 */
__tfm_secure_gateway_attributes__ bool PowerCC26X2_oscIsHPOSCEnabledWithHfDerivedLfClock(void)
{
    return OSC_IsHPOSCEnabledWithHfDerivedLfClock();
}

/*
 *  ======== PowerCC26X2_enableTCXOQual ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_enableTCXOQual(void)
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
__tfm_secure_gateway_attributes__ void PowerCC26X2_disableTCXOQual(void)
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

__tfm_secure_gateway_attributes__ void PowerCC26X2_turnOnXosc(void)
{
    OSCHF_TurnOnXosc();
}

/*
 *  ======== PowerCC26X2_switchToTCXO ========
 *  Switching to TCXO after TCXO startup time has expired.
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_switchToTCXO(void)
{
    /* Set bypass bit */
    HWREG(AUX_DDI0_OSC_BASE + DDI_O_SET + DDI_0_OSC_O_XOSCHFCTL) = DDI_0_OSC_XOSCHFCTL_BYPASS;

    /* Request XOSC_HF. In this instance, that is the TCXO. */
    OSCHF_TurnOnXosc();
}

/*
 *  ======== PowerCC26X2_sysCtrlStandby ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_sysCtrlStandby(bool retainCache)
{
    SysCtrlStandby(retainCache, VIMS_ON_CPU_ON_MODE, SYSCTRL_PREFERRED_RECHARGE_MODE);
}

/*
 *  ======== PowerCC26X2_sysCtrlVoltageConditionalControl ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_sysCtrlVoltageConditionalControl(void)
{
    SysCtrl_DCDC_VoltageConditionalControl();
}

/*
 *  ======== PowerCC26X2_sysCtrlIdle ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_sysCtrlIdle(uint32_t vimsPdMode)
{
    SysCtrlIdle(vimsPdMode);
}

/*
 *  ======== PowerCC26X2_sysCtrlIdle ========
 */
__tfm_secure_gateway_attributes__ void PowerCC26X2_setSECDMADependency(uint32_t setActive)
{
    if (setActive)
    {
        PRCMPeripheralRunEnable(PRCM_PERIPH_UDMA);
        PRCMPeripheralSleepEnable(PRCM_PERIPH_UDMA);
        PRCMPeripheralDeepSleepEnable(PRCM_PERIPH_UDMA);
    }
    else
    {
        PRCMPeripheralRunDisable(PRCM_PERIPH_UDMA);
        PRCMPeripheralSleepDisable(PRCM_PERIPH_UDMA);
        PRCMPeripheralDeepSleepDisable(PRCM_PERIPH_UDMA);
    }

    PRCMLoadSet();
    while (!PRCMLoadGet())
    {
        ;
    }
}

__tfm_secure_gateway_attributes__ uint32_t PowerCC26X2_sysCtrlGetResetSource(void)
{
    return SysCtrlResetSourceGet();
}
