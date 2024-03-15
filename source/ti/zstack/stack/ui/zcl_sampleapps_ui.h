 /**************************************************************************************************
  Filename:       zcl_sampleapps_ui.h
  Revised:        $Date: 2016-08-01 08:38:22 -0700 (Thu, 19 Jun 2014) $
  Revision:       $Revision: 39101 $

  Description:    This file contains the generic Sample Application User Interface.


  Copyright 2006-2014 Texas Instruments Incorporated.

  All rights reserved not granted herein.
  Limited License.

  Texas Instruments Incorporated grants a world-wide, royalty-free,
  non-exclusive license under copyrights and patents it now or hereafter
  owns or controls to make, have made, use, import, offer to sell and sell
  ("Utilize") this software subject to the terms herein. With respect to the
  foregoing patent license, such license is granted solely to the extent that
  any such patent is necessary to Utilize the software alone. The patent
  license shall not apply to any combinations which include this software,
  other than combinations with devices manufactured by or for TI ("TI
  Devices"). No hardware patent is licensed hereunder.

  Redistributions must preserve existing copyright notices and reproduce
  this license (including the above copyright notice and the disclaimer and
  (if applicable) source code license limitations below) in the documentation
  and/or other materials provided with the distribution.

  Redistribution and use in binary form, without modification, are permitted
  provided that the following conditions are met:

    * No reverse engineering, decompilation, or disassembly of this software
      is permitted with respect to any software provided in binary form.
    * Any redistribution and use are licensed by TI for use only with TI Devices.
    * Nothing shall obligate TI to provide you with source code for the software
      licensed and provided to you in object code.

  If software source code is provided to you, modification and redistribution
  of the source code are permitted provided that the following conditions are
  met:

    * Any redistribution and use of the source code, including any resulting
      derivative works, are licensed by TI for use only with TI Devices.
    * Any redistribution and use of any object code compiled from the source
      code and any resulting derivative works, are licensed by TI for use
      only with TI Devices.

  Neither the name of Texas Instruments Incorporated nor the names of its
  suppliers may be used to endorse or promote products derived from this
  software without specific prior written permission.

  DISCLAIMER.

  THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

#ifndef ZCL_SAMPLEAPPS_UI_H
#define ZCL_SAMPLEAPPS_UI_H


#ifndef CUI_DISABLE

#include "zstackmsg.h"
#include "zstackapi.h"
#include "cui.h"
#include <string.h>
#include "zcl_sample_app_def.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/apps/Button.h>
#include "zcl_sample_app_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * MACROS
 */

// UI Events
#define SAMPLEAPP_UI_BDB_NWK_LINE_UPDATE_EVT  0x0020
#define SAMPLEAPP_UI_INPUT_EVT                0x0040
#define SAMPLEAPP_UI_GP_LINE_UPDATE_EVT       0x0100
#define SAMPLEAPP_KEY_EVT_UI                  0x0200

/*********************************************************************
 * TYPEDEFS
 */

typedef void (* uiAppFNResetCB_t)(void);
typedef void (* uiAppProcessKeyCB_t)(uint8_t key, Button_EventMask _buttonEvents);

/*********************************************************************
* FUNCTIONS
*/

/*
 * User-Interface intialization
 */
CUI_clientHandle_t UI_Init(uint8_t  zclSampleApp_Entity, uint32_t *zclSampleAppEvents, Semaphore_Handle zclSampleAppSem,
              uint16_t *ui_IdentifyTimeAttribute_value, uint16_t *defaultBdbCommisioningModes,
              CONST char *pAppStr, uiAppProcessKeyCB_t zclSampleApp_processKey, uiAppFNResetCB_t _uiAppFNResetCB);

/*
 * Process a change in the device's network-state
 */
void UI_DeviceStateUpdated(zstack_devStateChangeInd_t *pReq);
/*
 * Process updating the Nwk Status line
 */
void UI_UpdateNwkStatusLine(void);
/*
 * Process updating the Device Info line
 */
void UI_UpdateDeviceInfoLine(void);

/*
 * Process updating the Bdb Status line
 */
void UI_UpdateBdbStatusLine(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);
/*
 * Process updating the Bind info
 */
void UI_UpdateBindInfoLine(void);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*
 * Process updating the Gp Status line
 */
void UI_UpdateGpStatusLine(void);
#endif

void zclsampleApp_ui_event_loop(void);

void uiProcessIdentifyTimeChange( uint8_t *endpoint );

void uiProcessBindNotification( bdbBindNotificationData_t *data );

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*
 * @brief   Interface to set the GP Proxy commissioning mode
 */
void UI_SetGPPCommissioningMode( zstack_gpCommissioningMode_t *Req );
#endif

#ifdef __cplusplus
}
#endif

#endif //#ifndef CUI_DISABLE

#endif
