/**************************************************************************************************
  Filename:       zcl_samplefiredetector.h
  Revised:        $Date: 2015-08-19 17:11:00 -0700 (Wed, 19 Aug 2015) $
  Revision:       $Revision: 44460 $


  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2006-2013 Texas Instruments Incorporated.

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

#ifndef ZCL_SAMPLESW_H
#define ZCL_SAMPLESW_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "nvintf.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif

/*********************************************************************
 * CONSTANTS
 */
#define SAMPLEFIREDETECTOR_ENDPOINT               8


// Events for the sample app
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT   0x0001
#define SAMPLEAPP_AUTO_ENROLL_REQ_EVT     0x0002
#define SAMPLEAPP_WRITE_TO_CIE_DONE_EVT   0x0004

#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY 1000
#define SAMPLEAPP_AUTO_ENROLL_REQ_DELAY   500

//Cluster NV items

// Application NV IDs:
// 0x0001 - 0x007F reserved
// 0x0080 - 0x03FF available for applications
#define IAS_ZONE_NV_ID                   0x0080
#define IAS_ZONE_ZONE_STATE_NV_SUBID     0x0001
#define IAS_ZONE_CIE_ADDRESS_NV_SUBID    0x0002
#define IAS_ZONE_ZONE_ID_NV_SUBID        0x0003

// Green Power Events
#define SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT              0x0100
#define SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT       0x0200
#define SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT            0x0400
/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

typedef enum
{
  TRIP_TO_PAIR = 0,
  // AUTO_ENROLL_RESPONSE, /* not supported */
  AUTO_ENROLL_REQUEST,
  ENROLLMENT_MODE_ENUM_LENGTH,
} IAS_Zone_EnrollmentModes_t;

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclSampleZone_SimpleDesc;

extern SimpleDescriptionFormat_t zclSampleZone9_SimpleDesc;

extern CONST zclAttrRec_t zclSampleZone_Attrs[];

extern uint16_t zclSampleZone_IdentifyTime;
extern CONST uint8_t zclSampleZone_NumAttributes;

extern uint8_t zclSampleZone_ZoneState;
extern uint16_t zclSampleZone_ZoneType;
extern uint16_t zclSampleZone_ZoneStatus;
extern uint8_t zclSampleZone_CIE_Address[8];
extern uint8_t zclSampleZone_ZoneId;
/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclSampleZone_ResetAttributesToDefaultValues(void); //implemented in zcl_samplefiredetector_data.c

#ifndef CUI_DISABLE
extern void zclSampleZone_UiActionChangeEnrollmentMode(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
extern void zclSampleZone_UiActionDiscoverCIE(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
extern void zclSampleZone_UiActionSendEnroll(const int32_t _itemEntry);
extern void zclSampleZone_UiActionToggleAlarm(const int32_t _itemEntry);
#endif
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLEAPP_H */

