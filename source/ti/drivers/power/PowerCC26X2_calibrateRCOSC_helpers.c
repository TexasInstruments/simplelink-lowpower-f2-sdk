/*
 * Copyright (c) 2021-2023, Texas Instruments Incorporated
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
 *  ======== PowerCC26X2_calibrateRCOSC_helpers.c ========
 */

#include <stdbool.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ddi.h)
#include DeviceFamily_constructPath(inc/hw_aux_tdc.h)
#include DeviceFamily_constructPath(inc/hw_aux_sysif.h)
#include DeviceFamily_constructPath(inc/hw_ddi_0_osc.h)
#include DeviceFamily_constructPath(inc/hw_ddi.h)
#include DeviceFamily_constructPath(driverlib/ddi.h)
#include DeviceFamily_constructPath(driverlib/osc.h)
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/aon_batmon.h)
#include DeviceFamily_constructPath(driverlib/setup_rom.h)

#define DDI_0_OSC_O_CTL1_LOCAL                                0x00000004 /* offset */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M              0x007C0000 /* mask */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S              18         /* shift */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_M           0x00020000 /* mask */
#define DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_S           17         /* shift */
#define DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M 0x00000C00 /* mask */
#define DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S 10         /* shift */

#define ACLK_REF_SRC_RCOSC_HF 0 /* Use RCOSC_HF for ACLK REF */
#define ACLK_REF_SRC_RCOSC_LF 2 /* Use RCOSC_LF for ACLK REF */

#if TFM_ENABLED
    #include <third_party/tfm/secure_fw/include/security_defs.h>
#else
    /* Define the gateway attributes to nothing instead of ifdefs on each function */
    #define __tz_c_veneer
#endif

/*
 *  ======== PowerCC26X2_updateSubSecInc ========
 *  Update the SUBSECINC register based on measured RCOSC_LF frequency
 *  Reads measured result directly from TDC to guard againse injection
 */
__tz_c_veneer void PowerCC26X2_updateSubSecInc(bool firstLF)
{
    int32_t newSubSecInc;
    uint32_t oldSubSecInc;
    uint32_t subSecInc;
    int32_t hposcOffset;
    int32_t hposcOffsetInv;

    uint32_t tdcResult = HWREG(AUX_TDC_BASE + AUX_TDC_O_RESULT);

    /*
     * Calculate the new SUBSECINC
     * Here's the formula: AON_RTC:SUBSECINC = (45813 * NR) / 256
     * Based on measuring 32 LF clock periods
     */
    newSubSecInc = (45813 * tdcResult) / 256;

    /* Compensate HPOSC drift if HPOSC is in use */
    if (OSC_IsHPOSCEnabled())
    {
        /* Get the HPOSC relative offset at this temperature */
        hposcOffset    = OSC_HPOSCRelativeFrequencyOffsetGet(AONBatMonTemperatureGetDegC());
        /* Convert to RF core format */
        hposcOffsetInv = OSC_HPOSCRelativeFrequencyOffsetToRFCoreFormatConvert(hposcOffset);
        /* Adjust SUBSECINC */
        newSubSecInc += (((newSubSecInc >> 4) * (hposcOffsetInv >> 3)) >> 15);
    }

    /* Apply filter, but not for first calibration */
    if (firstLF)
    {
        /* Don't apply filter first time, to converge faster */
        subSecInc = newSubSecInc;
    }
    else
    {
        /* Read old SUBSECINC value */
        oldSubSecInc = HWREG(AON_RTC_BASE + AON_RTC_O_SUBSECINC) & AON_RTC_SUBSECINC_VALUEINC_M;
        /* Apply filter, 0.5 times old value, 0.5 times new value */
        subSecInc    = (oldSubSecInc * 1 + newSubSecInc * 1) / 2;
    }

    /* Update SUBSECINC values */
    SetupSetAonRtcSubSecInc(subSecInc);
}

/*
 *  ======== PowerCC26X2_readCtrim ========
 */
__tz_c_veneer int32_t PowerCC26X2_readCtrim(void)
{
    return (DDI32RegRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_RCOSCHFCTL) & DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_M) >>
           DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_S;
}

/*
 *  ======== PowerCC26X2_readCtrimFract ========
 */
__tz_c_veneer int32_t PowerCC26X2_readCtrimFract(void)
{
    return (DDI32RegRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_CTL1_LOCAL) & DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M) >>
           DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S;
}

/*
 *  ======== PowerCC26X2_readRtrim ========
 */
__tz_c_veneer int32_t PowerCC26X2_readRtrim(void)
{
    return (DDI32RegRead(AUX_DDI0_OSC_BASE, DDI_0_OSC_O_ATESTCTL) &
            DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M) >>
           DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S;
}

/*
 *  ======== PowerCC26X2_writeCtrim ========
 */
__tz_c_veneer void PowerCC26X2_writeCtrim(int32_t newValue)
{
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                       DDI_0_OSC_O_RCOSCHFCTL,
                       DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_M,
                       DDI_0_OSC_RCOSCHFCTL_RCOSCHF_CTRIM_S,
                       newValue);
}

/*
 *  ======== PowerCC26X2_writeCtrimFract ========
 */
__tz_c_veneer void PowerCC26X2_writeCtrimFract(int32_t newValue)
{
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                       DDI_0_OSC_O_CTL1_LOCAL,
                       DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_M,
                       DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_LOCAL_S,
                       newValue);
}

/*
 *  ======== PowerCC26X2_writeCtrimFractEn ========
 */
__tz_c_veneer void PowerCC26X2_writeCtrimFractEn(int32_t newValue)
{
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                       DDI_0_OSC_O_CTL1_LOCAL,
                       DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_M,
                       DDI_0_OSC_CTL1_RCOSCHFCTRIMFRACT_EN_LOCAL_S,
                       newValue);
}

/*
 *  ======== PowerCC26X2_writeRtrim ========
 */
__tz_c_veneer void PowerCC26X2_writeRtrim(int32_t newValue)
{
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                       DDI_0_OSC_O_ATESTCTL,
                       DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_M,
                       DDI_0_OSC_ATESTCTL_SET_RCOSC_HF_FINE_RESISTOR_LOCAL_S,
                       newValue);
}

/*
 *  ======== PowerCC26X2_setTdcClkSrc24M ========
 */
__tz_c_veneer void PowerCC26X2_setTdcClkSrc24M(void)
{
    /* set TDC_SRC clock to be XOSC_HF/2 = 24 MHz */
    DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                       DDI_0_OSC_O_CTL0,
                       DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_M,
                       DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_S,
                       2);

    /* read back to ensure no race condition between OSC_DIG and AUX_SYSIF */
    DDI16BitfieldRead(AUX_DDI0_OSC_BASE,
                      DDI_0_OSC_O_CTL0,
                      DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_M,
                      DDI_0_OSC_CTL0_ACLK_TDC_SRC_SEL_S);
}

/*
 *  ======== PowerCC26X2_setAclkRefSrc ========
 */
__tz_c_veneer void PowerCC26X2_setAclkRefSrc(uint32_t source)
{
    if (source == ACLK_REF_SRC_RCOSC_HF || source == ACLK_REF_SRC_RCOSC_LF)
    {
        /* set the ACLK reference clock */
        DDI16BitfieldWrite(AUX_DDI0_OSC_BASE,
                           DDI_0_OSC_O_CTL0,
                           DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_M,
                           DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_S,
                           source);

        /* read back to ensure no race condition between OSC_DIG and AUX_SYSIF */
        DDI16BitfieldRead(AUX_DDI0_OSC_BASE,
                          DDI_0_OSC_O_CTL0,
                          DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_M,
                          DDI_0_OSC_CTL0_ACLK_REF_SRC_SEL_M);
    }
}
