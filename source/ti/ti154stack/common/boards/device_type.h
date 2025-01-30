/******************************************************************************

 @file device_type.h

 @brief This file contains definitions for device type identification

 @detail The module assigns a DeviceType_ID macro based on the defined
         device type in ti_drivers_config.h. If no board or device type
         is defined in ti_drivers_config.h, a generic DeviceType_ID is
         assigned.

 Group: LPRF SW RND
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2019-2025, Texas Instruments Incorporated
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

#ifndef DEVICE_TYPE_H
#define DEVICE_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 Includes
 *****************************************************************************/
#include "ti_radio_config.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/* DeviceType_ID_XYZ values */
#define DeviceType_ID_GENERIC       0
#define DeviceType_ID_CC1310        1
#define DeviceType_ID_CC1350        2
#define DeviceType_ID_CC2640R2      3
#define DeviceType_ID_CC1312R1      4
#define DeviceType_ID_CC1352R1      5
#define DeviceType_ID_CC1352P1      6
#define DeviceType_ID_CC1352P_2     7
#define DeviceType_ID_CC1352P_4     8
#define DeviceType_ID_CC2642R1      9
#define DeviceType_ID_CC2652R1      10
#define DeviceType_ID_CC2652RB      11
#define DeviceType_ID_CC2652RSIP    12
#define DeviceType_ID_CC2652PSIP    13
#define DeviceType_ID_CC1352P7_1    14
#define DeviceType_ID_CC1352P7_4    15
#define DeviceType_ID_CC1312R7      16
#define DeviceType_ID_CC2652R7      17
/*
 * Lookup table that sets DeviceType_ID based on the defined device type
 * in ti_drivers_config.h
 */
#if defined(CONFIG_CC1310_LAUNCHXL)
    #define DeviceType_ID       DeviceType_ID_CC1310
#elif defined(CONFIG_CC1350_LAUNCHXL)
    #define DeviceType_ID       DeviceType_ID_CC1350
#elif defined(CONFIG_CC2640R2_LAUNCHXL)
    #define DeviceType_ID       DeviceType_ID_CC2640R2
#elif defined(LAUNCHXL_CC1312R1)
    #define DeviceType_ID       DeviceType_ID_CC1312R1
#elif defined(LAUNCHXL_CC1352R1)
    #define DeviceType_ID       DeviceType_ID_CC1352R1
#elif defined(LAUNCHXL_CC1352P1)
    #define DeviceType_ID       DeviceType_ID_CC1352P1
#elif defined(LAUNCHXL_CC1352P_2)
    #define DeviceType_ID       DeviceType_ID_CC1352P_2
#elif defined(LAUNCHXL_CC1352P_4)
    #define DeviceType_ID       DeviceType_ID_CC1352P_4
#elif defined(CONFIG_CC2642R1FRGZ)
    #define DeviceType_ID       DeviceType_ID_CC2642R1
#elif defined(LAUNCHXL_CC26X2R1)
    #define DeviceType_ID       DeviceType_ID_CC2652R1
#elif defined(LP_CC2652RB)
    #define DeviceType_ID       DeviceType_ID_CC2652RB
#elif defined(LP_CC2652RSIP)
    #define DeviceType_ID       DeviceType_ID_CC2652RSIP
#elif defined(LP_CC2652PSIP)
    #define DeviceType_ID       DeviceType_ID_CC2652PSIP
#elif defined(LP_CC1352P7_1)
    #define DeviceType_ID       DeviceType_ID_CC1352P7_1
#elif defined(LP_CC1352P7_4)
    #define DeviceType_ID       DeviceType_ID_CC1352P7_4
#elif defined(LP_CC1312R7)
    #define DeviceType_ID       DeviceType_ID_CC1312R7
#elif defined(LP_CC2652R7)
    #define DeviceType_ID       DeviceType_ID_CC2652R7
#else
    #define DeviceType_ID       DeviceType_ID_GENERIC
#endif

/* Ensure that only one DeviceType was specified */
#if (defined(CONFIG_CC1310_LAUNCHXL) + defined(CONFIG_CC1350_LAUNCHXL)  \
    + defined(CONFIG_CC2640R2_LAUNCHXL)                                 \
    + defined(LAUNCHXL_CC1312R1)                                        \
    + defined(LAUNCHXL_CC1352R1)                                        \
    + defined(LAUNCHXL_CC1352P1)                                        \
    + defined(LAUNCHXL_CC1352P_2)                                       \
    + defined(LAUNCHXL_CC1352P_4)                                       \
    + defined(CONFIG_CC2642R1FRGZ)                                      \
    + defined(LAUNCHXL_CC26X2R1)                                        \
    + defined(LP_CC2652RB)                                        \
    + defined(LP_CC2652RSIP)                                            \
    + defined(LP_CC2652PSIP)                                            \
    + defined(LP_CC1352P7_1)                                            \
    + defined(LP_CC1352P7_4)                                            \
    + defined(LP_CC1312R7)                                              \
    + defined(LP_CC2652R7)                                              \
    ) > 1
    #error "More then one DeviceType has been defined!"
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_TYPE_H */
