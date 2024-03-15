/**************************************************************************************************
  Filename:       zcl_samplethermostat.h
  Revised:        $Date: 2013-04-22 14:49:05 -0700 (Mon, 22 Apr 2013) $
  Revision:       $Revision: 33994 $

  Description:    This file contains the Zigbee Cluster Library Home
                  Automation Sample Application.


  Copyright 2013 Texas Instruments Incorporated.

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

#ifndef ZCL_SAMPLETHERMOSTAT_H
#define ZCL_SAMPLETHERMOSTAT_H

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
#define SAMPLETHERMOSTAT_ENDPOINT            8
#if BDB_REPORTING_2ENDPOINTS
#define SAMPLETHERMOSTAT_ENDPOINT2           9
#endif

// Application Events
#define SAMPLETHERMOSTAT_POLL_CONTROL_TIMEOUT_EVT     0x0001
#define SAMPLEAPP_END_DEVICE_REJOIN_EVT               0x0002


// Green Power Events
#define SAMPLEAPP_PROCESS_GP_DATA_SEND_EVT              0x0100
#define SAMPLEAPP_PROCESS_GP_EXPIRE_DUPLICATE_EVT       0x0200
#define SAMPLEAPP_PROCESS_GP_TEMP_MASTER_EVT            0x0400

#define SAMPLEAPP_END_DEVICE_REJOIN_DELAY 1000

/*********************************************************************
 * MACROS
 */
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * VARIABLES
 */
extern SimpleDescriptionFormat_t zclSampleThermostat_SimpleDesc;

extern CONST zclAttrRec_t zclSampleThermostat_Attrs[];

extern CONST uint8_t zclSampleThermostat_NumAttributes;

extern uint16_t zclSampleThermostat_IdentifyTime;

extern int16_t zclSampleThermostat_OccupiedCoolingSetpoint;

extern int16_t zclSampleThermostat_OccupiedHeatingSetpoint;

extern int16_t zclSampleThermostat_LocalTemperature;

extern int16_t zclSampleThermostat_AbsMinHeatSetpointLimit;

extern int16_t zclSampleThermostat_AbsMaxHeatSetpointLimit;

extern int16_t zclSampleThermostat_AbsMinCoolSetpointLimit;

extern int16_t zclSampleThermostat_AbsMaxCoolSetpointLimit;


extern uint8_t zclSampleThermostat_ControlSequenceOfOperation;

extern uint8_t zclSampleThermostat_SystemMode;

/*********************************************************************
 * FUNCTIONS
 */

/*
 *  Reset all writable attributes to their default values.
 */
extern void zclSampleThermostat_ResetAttributesToDefaultValues(void);

#ifndef CUI_DISABLE
extern void zclSampleThermostat_UiActionSetCoolPoint(const char _input, char* _lines[3], CUI_cursorInfo_t * _curInfo);
extern void zclSampleThermostat_UiActionSetHeatPoint(const char _input, char* _lines[3], CUI_cursorInfo_t * _curInfo);
#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
extern void zclSampleThermostat_setGPSinkCommissioningMode(const int32_t _itemEntry);
#endif
#endif
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_SAMPLETHERMOSTAT_H */
