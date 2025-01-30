/******************************************************************************

@file  bleapputil_api_legacy.h

@brief This file contains the bleapputil_api_legacy for use
with the CC13XX_CC26XX Bluetooth Low Energy Protocol Stack.

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2022-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************


*****************************************************************************/

#ifndef BLEAPPUTIL_API_LEGACY_H
#define BLEAPPUTIL_API_LEGACY_H

#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
 * TYPEDEFS
 */

/* Map for legacy devices */
#if !defined(DeviceFamily_CC26X1) && !defined(DeviceFamily_CC13X4) && !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC26X4) && !defined(DeviceFamily_CC23X0R2)

#define GAP_PERIPHERAL_REQUESTED_SECURITY_EVENT   GAP_SLAVE_REQUESTED_SECURITY_EVENT
#define gapPeripheralSecurityReqEvent_t           gapSlaveSecurityReqEvent_t

#endif


#ifdef __cplusplus
}
#endif

#endif /* BLEAPPUTIL_API_LEGACY_H */
