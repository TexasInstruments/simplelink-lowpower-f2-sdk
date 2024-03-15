/*
 * Copyright (c) 2018, Texas Instruments Incorporated
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
 *
 */

/*
 *                           ti_zstack_config.h
 *
 *  Macros used to define a TI Z-Stack ZigBee device.
 *
 */

#ifndef TI_ZSTACK_CONFIG_H
#define TI_ZSTACK_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "gpd.h"

/******************************************************************************
 * CONSTANTS
 */
#define GPD_APP_TYPE_SRC_ID           0
#define GPD_APP_TYPE_IEEE_ID          2

//VALUES OF GPSECURITYKEYTYPE
#define KEY_TYPE_NO_KEY                          0
#define KEY_TYPE_ZIGBEE_NWK_KEY                  1
#define KEY_TYPE_GPD_GROUP_KEY                   2
#define KEY_TYPE_NWK_KEY_DERIVED_GPD_GROUP_KEY   3
#define KEY_TYPE_OUT_BOX_GPD_KEY                 4
#define KEY_TYPE_DERIVED_INDIVIDUAL_GPD_KEY      7

/* SECURITY LEVEL */
#define GP_SECURITY_LVL_NO_SEC             0
#define GP_SECURITY_LVL_RESERVED           1
#define GP_SECURITY_LVL_4FC_4MIC           2
#define GP_SECURITY_LVL_4FC_4MIC_ENCRYPT   3

/******************************************************************************
 * GPD CONFIGURATION
 */


#ifndef GPD_CHANNEL
#define GPD_CHANNEL            11
#endif

#ifndef DEVICE_ID
#define DEVICE_ID              0x30
#endif

#ifndef GPD_APP_ID
#define GPD_APP_ID             GPD_APP_TYPE_SRC_ID
#endif

#if ( GPD_APP_ID == GPD_APP_TYPE_SRC_ID )
#define GPD_ID                 0x11223344
#elif ( GPD_APP_ID == GPD_APP_TYPE_IEEE_ID )
#define GPD_ID                 ApiMac_extAddr
#endif

#ifndef GP_SECURITY_LEVEL
#define GP_SECURITY_LEVEL      GP_SECURITY_LVL_4FC_4MIC_ENCRYPT
#endif

#ifndef EXT_OPT_KEY_TYPE
#define EXT_OPT_KEY_TYPE       KEY_TYPE_OUT_BOX_GPD_KEY
#endif

#ifndef GP_SECURITY_KEY
#define GP_SECURITY_KEY        {0xCF,0xCE,0xCD,0xCC,0xCB,0xCA,0xC9,0xC8,0xC7,0xC6,0xC5,0xC4,0xC3,0xC2,0xC1,0xC0}
#endif

#ifndef GPDF_FRAME_DUPLICATES
#define GPDF_FRAME_DUPLICATES  3
#endif

#ifndef AUTO_COMMISSIONING
#define AUTO_COMMISSIONING     FALSE
#endif

#ifndef RX_AFTER_TX
#define RX_AFTER_TX            FALSE
#endif

#ifndef SEQUENCE_NUMBER_CAP
#define SEQUENCE_NUMBER_CAP    FALSE
#endif

#ifdef __cplusplus
}
#endif

#endif /* TI_ZSTACK_CONFIG_H */

