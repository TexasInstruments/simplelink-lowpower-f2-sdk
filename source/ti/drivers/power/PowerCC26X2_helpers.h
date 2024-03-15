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

#include <string.h>

/* This file is for internal use only. These functions are called by the power
 * driver during normal operation, but may trigger a secure gateway instruction
 * on devices where the core supports it. Other calls to these functions may
 * have a negative impact on the power driver.
 */

/* Functions implemented in PowerCC26X2_helpers.c */
void PowerCC26X2_oscCtlClearXtal(void);
void PowerCC26X2_pmctlDisableJtag(void);
void PowerCC26X2_sysctrlShutdownWithAbort(void);
void PowerCC26X2_prcmEnableCacheRetention(void);
void PowerCC26X2_oschfSwitchToRcosc(void);
uint32_t PowerCC26X2_oscClockSourceGet(uint32_t ui32SrcClk);
void PowerCC26X2_oscDisableQualifiers(void);
bool PowerCC26X2_getOscHfSourceReady(void);
uint32_t PowerCC26X2_oschfTrySwitchToXosc(void);
bool PowerCC26X2_oscIsHPOSCEnabledWithHfDerivedLfClock(void);
void PowerCC26X2_enableTCXOQual(void);
void PowerCC26X2_disableTCXOQual(void);
void PowerCC26X2_switchToTCXO(void);
void PowerCC26X2_turnOnXosc(void);
void PowerCC26X2_sysCtrlStandby(bool retainCache);
void PowerCC26X2_sysCtrlVoltageConditionalControl(void);
void PowerCC26X2_sysCtrlIdle(uint32_t vimsPdMode);
void PowerCC26X2_setPeriphDependency(uint32_t prcmPeriph);
void PowerCC26X2_releasePeriphDependency(uint32_t prcmPeriph);
void PowerCC26X2_setPeriphDeepSleepEnable(uint32_t prcmPeriph);
uint32_t PowerCC26X2_sysCtrlGetResetSource(void);
void PowerCC26X2_setSubSecIncToXoscLf(uint32_t subsecinc);
void PowerCC26X2_sysCtrlUpdateVoltageRegulator(void);
void PowerCC26X2_sysCtrlReset(void);

/* Functions implemented in PowerCC26X2_calibrateRCOSC_helpers.c */
void PowerCC26X2_updateSubSecInc(bool firstLF);
int32_t PowerCC26X2_readCtrim(void);
int32_t PowerCC26X2_readCtrimFract(void);
int32_t PowerCC26X2_readRtrim(void);
void PowerCC26X2_writeCtrim(int32_t newValue);
void PowerCC26X2_writeCtrimFract(int32_t newValue);
void PowerCC26X2_writeCtrimFractEn(int32_t newValue);
void PowerCC26X2_writeRtrim(int32_t newValue);
void PowerCC26X2_setTdcClkSrc24M(void);
void PowerCC26X2_setAclkRefSrc(uint32_t source);
