/**************************************************************************************************
  Filename:       zcl_ha.h
  Revised:        $Date: 2014-06-23 15:23:54 -0700 (Mon, 23 Jun 2014) $
  Revision:       $Revision: 39166 $

  Description:    This file contains the Zigbee Cluster Library: Home
                  Automation Profile definitions.


  Copyright (c) 2019, Texas Instruments Incorporated
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
**************************************************************************************************/

#ifndef ZCL_HA_H
#define ZCL_HA_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"

/*********************************************************************
 * CONSTANTS
 */
// Zigbee Home Automation Profile Identification
#define ZCL_HA_PROFILE_ID                                    0x0104

// Generic Device
#define ZCL_DEVICEID_GENERIC                                 0xFFFE

// Generic Device IDs
#define ZCL_DEVICEID_ON_OFF_SWITCH                           0x0000
#define ZCL_DEVICEID_LEVEL_CONTROL_SWITCH                    0x0001
#define ZCL_DEVICEID_ON_OFF_OUTPUT                           0x0002
#define ZCL_DEVICEID_LEVEL_CONTROLLABLE_OUTPUT               0x0003
#define ZCL_DEVICEID_SCENE_SELECTOR                          0x0004
#define ZCL_DEVICEID_CONFIGURATION_TOOL                      0x0005
#define ZCL_DEVICEID_REMOTE_CONTROL                          0x0006
#define ZCL_DEVICEID_COMBINED_INTERFACE                      0x0007
#define ZCL_DEVICEID_RANGE_EXTENDER                          0x0008
#define ZCL_DEVICEID_MAINS_POWER_OUTLET                      0x0009
#define ZCL_DEVICEID_DOOR_LOCK                               0x000A
#define ZCL_DEVICEID_DOOR_LOCK_CONTROLLER                    0x000B
#define ZCL_DEVICEID_SIMPLE_SENSOR                           0x000C
#define ZCL_DEVICEID_CONSUMPTION_AWARENESS                   0x000D
#define ZCL_DEVICEID_HOME_GATEWAY                            0x0050
#define ZCL_DEVICEID_SMART_PLUG                              0x0051
#define ZCL_DEVICEID_WHITE_GOODS                             0x0052
#define ZCL_DEVICEID_METER_INTERFACE                         0x0053

// This is a reserved value which could be used for test purposes
#define ZCL_DEVICEID_TEST_DEVICE                             0x00FF

// Lighting Device IDs
#define ZCL_DEVICEID_ON_OFF_LIGHT                            0x0100
#define ZCL_DEVICEID_DIMMABLE_LIGHT                          0x0101
#define ZCL_DEVICEID_COLOR_DIMMABLE                          0x0102
#define ZCL_DEVICEID_ON_OFF_LIGHT_SWITCH                     0x0103
#define ZCL_DEVICEID_DIMMER_SWITCH                           0x0104
#define ZCL_DEVICEID_COLOR_DIMMER_SWITCH                     0x0105
#define ZCL_DEVICEID_LIGHT_SENSOR                            0x0106
#define ZCL_DEVICEID_OCCUPANCY_SENSOR                        0x0107

// Closures Device IDs
#define ZCL_DEVICEID_SHADE                                   0x0200
#define ZCL_DEVICEID_SHADE_CONTROLLER                        0x0201
#define ZCL_DEVICEID_WINDOW_COVERING                         0x0202
#define ZCL_DEVICEID_WINDOW_COVERING_CONTROLLER              0x0203

// HVAC Device IDs
#define ZCL_DEVICEID_HEATING_COOLING_UNIT                    0x0300
#define ZCL_DEVICEID_THERMOSTAT                              0x0301
#define ZCL_DEVICEID_TEMPERATURE_SENSOR                      0x0302
#define ZCL_DEVICEID_PUMP                                    0x0303
#define ZCL_DEVICEID_PUMP_CONTROLLER                         0x0304
#define ZCL_DEVICEID_PRESSURE_SENSOR                         0x0305
#define ZCL_DEVICEID_FLOW_SENSOR                             0x0306
#define ZCL_DEVICEID_MINI_SPLIT_AC                           0x0307

// Intruder Alarm Systems (IAS) Device IDs
#define ZCL_DEVICEID_IAS_CIE                                 0x0400
#define ZCL_DEVICEID_IAS_ACE                                 0x0401
#define ZCL_DEVICEID_IAS_ZONE                                0x0402
#define ZCL_DEVICEID_IAS_WARNING                             0x0403

// Device type to display in LCD
#define ZCL_HA_DEVICE_COORDINATOR       0
#define ZCL_HA_DEVICE_ROUTER            1
#define ZCL_HA_DEVICE_END_DEVICE        2

// Attributes for the Diagnostics Cluster
#define ATTRID_DIAGNOSTICS_NUMBER_OF_RESETS 			        0x0000
#define ATTRID_DIAGNOSTICS_PERSISTENT_MEMORY_WRITES 			0x0001
#define ATTRID_DIAGNOSTICS_MAC_RX_BCAST 			            0x0100
#define ATTRID_DIAGNOSTICS_MAC_TX_BCAST 	            		0x0101
#define ATTRID_DIAGNOSTICS_MAC_RX_UCAST 	            		0x0102
#define ATTRID_DIAGNOSTICS_MAC_TX_UCAST 	            		0x0103
#define ATTRID_DIAGNOSTICS_MAC_TX_UCAST_RETRY 	        		0x0104
#define ATTRID_DIAGNOSTICS_MAC_TX_UCAST_FAIL 	        		0x0105
#define ATTRID_DIAGNOSTICS_APS_RX_BCAST 		            	0x0106
#define ATTRID_DIAGNOSTICS_APS_TX_BCAST 		            	0x0107
#define ATTRID_DIAGNOSTICS_APS_RX_UCAST 		            	0x0108
#define ATTRID_DIAGNOSTICS_APS_TX_UCAST_SUCCESS 	    		0x0109
#define ATTRID_DIAGNOSTICS_APS_TX_UCAST_RETRY 		        	0x010A
#define ATTRID_DIAGNOSTICS_APS_TX_UCAST_FAIL 		        	0x010B
#define ATTRID_DIAGNOSTICS_ROUTE_DISC_INITIATED 	    		0x010C
#define ATTRID_DIAGNOSTICS_NEIGHBOR_ADDED 			            0x010D
#define ATTRID_DIAGNOSTICS_NEIGHBOR_REMOVED 		        	0x010E
#define ATTRID_DIAGNOSTICS_NEIGHBOR_STALE 		            	0x010F
#define ATTRID_DIAGNOSTICS_JOIN_INDICATION 		            	0x0110
#define ATTRID_DIAGNOSTICS_CHILD_MOVED 		                	0x0111
#define ATTRID_DIAGNOSTICS_NWKFC_FAILURE 	            		0x0112
#define ATTRID_DIAGNOSTICS_APSFC_FAILURE 			            0x0113
#define ATTRID_DIAGNOSTICS_APS_UNAUTHORIZED_KEY 		    	0x0114
#define ATTRID_DIAGNOSTICS_NWK_DECRYPT_FAILURES 		    	0x0115
#define ATTRID_DIAGNOSTICS_APS_DECRYPT_FAILURES 		    	0x0116
#define ATTRID_DIAGNOSTICS_PACKET_BUFFER_ALLOCATE_FAILURES 		0x0117
#define ATTRID_DIAGNOSTICS_RELAYED_UCAST 			            0x0118
#define ATTRID_DIAGNOSTICS_PHY_TO_MAC_QUEUE_LIMIT_REACHED 		0x0119
#define ATTRID_DIAGNOSTICS_PACKET_VALIDATE_DROP_COUNT 			0x011A
#define ATTRID_DIAGNOSTICS_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT   0x011B
#define ATTRID_DIAGNOSTICS_LAST_MESSAGE_LQI 			        0x011C
#define ATTRID_DIAGNOSTICS_LAST_MESSAGE_RSSI 			        0x011D

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * FUNCTIONS
 */

extern void zclHA_LcdStatusLine1( uint8_t kind );
#define ZCL_HA_STATUSLINE_ZC    0
#define ZCL_HA_STATUSLINE_ZR    1
#define ZCL_HA_STATUSLINE_ZED   2

// convert 16 bits to an ascii hex number
extern void zclHA_uint16toa( uint16_t u, char *string );

// convert 8 bits to an ascii decimal number
extern void zclHA_uint8toa(uint8_t b, char *string);

// functions for dealing with a bit array
extern bool zclHA_isbit(uint8_t *pArray, uint8_t bitIndex);
extern void zclHA_setbit(uint8_t *pArray, uint8_t bitIndex);
extern void zclHA_clearbit(uint8_t *pArray, uint8_t bitIndex);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_HA_H */
