/**************************************************************************************************
  Filename:       gpd_common.h
  Revised:        $Date: 2014-10-06 15:42:43 -0700 (Mon, 06 Oct 2014) $
  Revision:       $Revision: 40449 $

  Description:    Contains green power command definitions and common stuff.


  Copyright 2005-2014 Texas Instruments Incorporated.

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

#ifndef GPD_COMMON_H_
#define GPD_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "gpd_config.h"
#include "ti_zstack_config.h"

/******************************************************************************
 * DEFINES
 */

#define SEC_KEY_LEN                 16
#define GP_NWK_PROTOCOL_VERSION     3
#define GP_DATA_FRAME               0
#define GP_MAINTENANCE_FRAME        1

#define GP_NONCE_LENGTH            13
#define GP_HEADER_LENGTH           10
#define GP_KEY_LENGTH              16
#define GP_SECURITY_MIC_SIZE        4

#define GP_NONCE_SEC_CONTROL               0x05

#define GP_SRC_ID_LEN              4
#define Z_EXATDDR_LEN              8

// General Clusters
#define ZCL_CLUSTER_ID_GENERAL_BASIC                             0x0000
#define ZCL_CLUSTER_ID_GENERAL_POWER_CFG                         0x0001
#define ZCL_CLUSTER_ID_GENERAL_DEVICE_TEMP_CONFIG                0x0002
#define ZCL_CLUSTER_ID_GENERAL_IDENTIFY                          0x0003
#define ZCL_CLUSTER_ID_GENERAL_GROUPS                            0x0004
#define ZCL_CLUSTER_ID_GENERAL_SCENES                            0x0005
#define ZCL_CLUSTER_ID_GENERAL_ON_OFF                            0x0006
#define ZCL_CLUSTER_ID_GENERAL_ON_OFF_SWITCH_CONFIG              0x0007
#define ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL                     0x0008
#define ZCL_CLUSTER_ID_GENERAL_ALARMS                            0x0009
#define ZCL_CLUSTER_ID_GENERAL_TIME                              0x000A
#define ZCL_CLUSTER_ID_GENERAL_LOCATION                          0x000B
#define ZCL_CLUSTER_ID_GENERAL_ANALOG_INPUT_BASIC                0x000C
#define ZCL_CLUSTER_ID_GENERAL_ANALOG_OUTPUT_BASIC               0x000D
#define ZCL_CLUSTER_ID_GENERAL_ANALOG_VALUE_BASIC                0x000E
#define ZCL_CLUSTER_ID_GENERAL_BINARY_INPUT_BASIC                0x000F
#define ZCL_CLUSTER_ID_GENERAL_BINARY_OUTPUT_BASIC               0x0010
#define ZCL_CLUSTER_ID_GENERAL_BINARY_VALUE_BASIC                0x0011
#define ZCL_CLUSTER_ID_GENERAL_MULTISTATE_INPUT_BASIC            0x0012
#define ZCL_CLUSTER_ID_GENERAL_MULTISTATE_OUTPUT_BASIC           0x0013
#define ZCL_CLUSTER_ID_GENERAL_MULTISTATE_VALUE_BASIC            0x0014
#define ZCL_CLUSTER_ID_GENERAL_COMMISSIONING                     0x0015
#define ZCL_CLUSTER_ID_GENERAL_PARTITION                         0x0016

#define ZCL_CLUSTER_ID_OTA                                   0x0019

#define ZCL_CLUSTER_ID_GENERAL_POWER_PROFILE                     0x001A
#define ZCL_CLUSTER_ID_GENERAL_APPLIANCE_CONTROL                 0x001B

#define ZCL_CLUSTER_ID_GENERAL_POLL_CONTROL                      0x0020

#define ZCL_CLUSTER_ID_GREEN_POWER                           0x0021

// Retail Clusters
#define ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIGURATION           0x0022
#define ZCL_CLUSTER_ID_NEIGHBOR_CLEANING                     0x0023
#define ZCL_CLUSTER_ID_NEAREST_GATEWAY                       0x0024

// Closures Clusters
#define ZCL_CLUSTER_ID_CLOSURES_SHADE_CONFIG                 0x0100
#define ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK                    0x0101
#define ZCL_CLUSTER_ID_CLOSURES_WINDOW_COVERING              0x0102

// HVAC Clusters
#define ZCL_CLUSTER_ID_HVAC_PUMP_CONFIG_CONTROL              0x0200
#define ZCL_CLUSTER_ID_HVAC_THERMOSTAT                       0x0201
#define ZCL_CLUSTER_ID_HVAC_FAN_CONTROL                      0x0202
#define ZCL_CLUSTER_ID_HVAC_DIHUMIDIFICATION_CONTROL         0x0203
#define ZCL_CLUSTER_ID_HVAC_USER_INTERFACE_CONFIG            0x0204

// Lighting Clusters
#define ZCL_CLUSTER_ID_LIGHTING_COLOR_CONTROL                0x0300
#define ZCL_CLUSTER_ID_LIGHTING_BALLAST_CONFIG               0x0301

// Measurement and Sensing Clusters
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT            0x0400
#define ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG   0x0401
#define ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT            0x0402
#define ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT               0x0403
#define ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT                   0x0404
#define ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY                  0x0405
#define ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING                  0x0406

// Security and Safety (SS) Clusters
#define ZCL_CLUSTER_ID_SS_IAS_ZONE                           0x0500
#define ZCL_CLUSTER_ID_SS_IAS_ACE                            0x0501
#define ZCL_CLUSTER_ID_SS_IAS_WD                             0x0502

// Protocol Interfaces
#define ZCL_CLUSTER_ID_PI_GENERIC_TUNNEL                     0x0600
#define ZCL_CLUSTER_ID_PI_BACNET_PROTOCOL_TUNNEL             0x0601
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_REG            0x0602
#define ZCL_CLUSTER_ID_PI_ANALOG_INPUT_BACNET_EXT            0x0603
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_REG           0x0604
#define ZCL_CLUSTER_ID_PI_ANALOG_OUTPUT_BACNET_EXT           0x0605
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_REG            0x0606
#define ZCL_CLUSTER_ID_PI_ANALOG_VALUE_BACNET_EXT            0x0607
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_REG            0x0608
#define ZCL_CLUSTER_ID_PI_BINARY_INPUT_BACNET_EXT            0x0609
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_REG           0x060A
#define ZCL_CLUSTER_ID_PI_BINARY_OUTPUT_BACNET_EXT           0x060B
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_REG            0x060C
#define ZCL_CLUSTER_ID_PI_BINARY_VALUE_BACNET_EXT            0x060D
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_REG        0x060E
#define ZCL_CLUSTER_ID_PI_MULTISTATE_INPUT_BACNET_EXT        0x060F
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_REG       0x0610
#define ZCL_CLUSTER_ID_PI_MULTISTATE_OUTPUT_BACNET_EXT       0x0611
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_REG        0x0612
#define ZCL_CLUSTER_ID_PI_MULTISTATE_VALUE_BACNET_EXT        0x0613
#define ZCL_CLUSTER_ID_PI_11073_PROTOCOL_TUNNEL              0x0614
#define ZCL_CLUSTER_ID_PI_ISO7818_PROTOCOL_TUNNEL            0x0615
#define ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL                      0x0617

// Smart Energy (SE) Clusters
#define ZCL_CLUSTER_ID_SE_PRICE                              0x0700
#define ZCL_CLUSTER_ID_SE_DRLC                               0x0701
#define ZCL_CLUSTER_ID_SE_METERING                           0x0702
#define ZCL_CLUSTER_ID_SE_MESSAGING                          0x0703
#define ZCL_CLUSTER_ID_SE_TUNNELING                          0x0704
#define ZCL_CLUSTER_ID_SE_PREPAYMENT                         0x0705
#define ZCL_CLUSTER_ID_SE_ENERGY_MGMT                        0x0706
#define ZCL_CLUSTER_ID_SE_CALENDAR                           0x0707
#define ZCL_CLUSTER_ID_SE_DEVICE_MGMT                        0x0708
#define ZCL_CLUSTER_ID_SE_EVENTS                             0x0709
#define ZCL_CLUSTER_ID_SE_MDU_PAIRING                        0x070A

#define ZCL_CLUSTER_ID_SE_KEY_ESTABLISHMENT                  0x0800

#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_INFORMATION        0x0900
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_CHATTING           0x0904
#define ZCL_CLUSTER_ID_TELECOMMUNICATIONS_VOICE_OVER_ZIGBEE  0x0905

#define ZCL_CLUSTER_ID_HA_APPLIANCE_IDENTIFICATION           0x0B00
#define ZCL_CLUSTER_ID_HA_METER_IDENTIFICATION               0x0B01
#define ZCL_CLUSTER_ID_HA_APPLIANCE_EVENTS_ALERTS            0x0B02
#define ZCL_CLUSTER_ID_HA_APPLIANCE_STATISTICS               0x0B03
#define ZCL_CLUSTER_ID_MS_ELECTRICAL_MEASUREMENT             0x0B04
#define ZCL_CLUSTER_ID_HA_DIAGNOSTIC                         0x0B05

// Light Link cluster
#define ZCL_CLUSTER_ID_TOUCHLINK                             0x1000

/*** Data Types ***/
#define ZCL_DATATYPE_NO_DATA                            0x00
#define ZCL_DATATYPE_DATA8                              0x08
#define ZCL_DATATYPE_DATA16                             0x09
#define ZCL_DATATYPE_DATA24                             0x0a
#define ZCL_DATATYPE_DATA32                             0x0b
#define ZCL_DATATYPE_DATA40                             0x0c
#define ZCL_DATATYPE_DATA48                             0x0d
#define ZCL_DATATYPE_DATA56                             0x0e
#define ZCL_DATATYPE_DATA64                             0x0f
#define ZCL_DATATYPE_BOOLEAN                            0x10
#define ZCL_DATATYPE_BITMAP8                            0x18
#define ZCL_DATATYPE_BITMAP16                           0x19
#define ZCL_DATATYPE_BITMAP24                           0x1a
#define ZCL_DATATYPE_BITMAP32                           0x1b
#define ZCL_DATATYPE_BITMAP40                           0x1c
#define ZCL_DATATYPE_BITMAP48                           0x1d
#define ZCL_DATATYPE_BITMAP56                           0x1e
#define ZCL_DATATYPE_BITMAP64                           0x1f
#define ZCL_DATATYPE_UINT8                              0x20
#define ZCL_DATATYPE_UINT16                             0x21
#define ZCL_DATATYPE_UINT24                             0x22
#define ZCL_DATATYPE_UINT32                             0x23
#define ZCL_DATATYPE_UINT40                             0x24
#define ZCL_DATATYPE_UINT48                             0x25
#define ZCL_DATATYPE_UINT56                             0x26
#define ZCL_DATATYPE_UINT64                             0x27
#define ZCL_DATATYPE_INT8                               0x28
#define ZCL_DATATYPE_INT16                              0x29
#define ZCL_DATATYPE_INT24                              0x2a
#define ZCL_DATATYPE_INT32                              0x2b
#define ZCL_DATATYPE_INT40                              0x2c
#define ZCL_DATATYPE_INT48                              0x2d
#define ZCL_DATATYPE_INT56                              0x2e
#define ZCL_DATATYPE_INT64                              0x2f
#define ZCL_DATATYPE_ENUM8                              0x30
#define ZCL_DATATYPE_ENUM16                             0x31
#define ZCL_DATATYPE_SEMI_PREC                          0x38
#define ZCL_DATATYPE_SINGLE_PREC                        0x39
#define ZCL_DATATYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATATYPE_OCTET_STR                          0x41
#define ZCL_DATATYPE_CHAR_STR                           0x42
#define ZCL_DATATYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATATYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATATYPE_ARRAY                              0x48
#define ZCL_DATATYPE_STRUCT                             0x4c
#define ZCL_DATATYPE_SET                                0x50
#define ZCL_DATATYPE_BAG                                0x51
#define ZCL_DATATYPE_TOD                                0xe0
#define ZCL_DATATYPE_DATE                               0xe1
#define ZCL_DATATYPE_UTC                                0xe2
#define ZCL_DATATYPE_CLUSTER_ID                         0xe8
#define ZCL_DATATYPE_ATTR_ID                            0xe9
#define ZCL_DATATYPE_BAC_OID                            0xea
#define ZCL_DATATYPE_IEEE_ADDR                          0xf0
#define ZCL_DATATYPE_128_BIT_SEC_KEY                    0xf1
#define ZCL_DATATYPE_UNKNOWN                            0xff

// GreemnPower NV item IDs
#define GP_NV_FLAG                       0x0001
#define GP_NV_SEQUENCE_NUMBER            0x0002
#define GP_NV_APP_BUTTON                 0x0003
#define GP_NV_SEC_FRAME_COUNTER          0x0004
#define GP_NV_SCENE_TABLE                0x0091
#define GP_NV_MIN_FREE_NWK_ADDR          0x0092
#define GP_NV_MAX_FREE_NWK_ADDR          0x0093
#define GP_NV_MIN_FREE_GRP_ID            0x0094
#define GP_NV_MAX_FREE_GRP_ID            0x0095
#define GP_NV_MIN_GRP_IDS                0x0096
#define GP_NV_MAX_GRP_IDS                0x0097
#define GP_NV_OTA_BLOCK_REQ_DELAY        0x0098

enum
{
  GP_COMMAND_IDENTIFY_IDENTIFY,
  GP_COMMAND_RECALL_SCENE0 = 0x10,
  GP_COMMAND_RECALL_SCENE1,
  GP_COMMAND_RECALL_SCENE2,
  GP_COMMAND_RECALL_SCENE3,
  GP_COMMAND_RECALL_SCENE4,
  GP_COMMAND_RECALL_SCENE5,
  GP_COMMAND_RECALL_SCENE6,
  GP_COMMAND_RECALL_SCENE7,
  GP_COMMAND_STORE_SCENE0,
  GP_COMMAND_STORE_SCENE1,
  GP_COMMAND_STORE_SCENE2,
  GP_COMMAND_STORE_SCENE3,
  GP_COMMAND_STORE_SCENE4,
  GP_COMMAND_STORE_SCENE5,
  GP_COMMAND_STORE_SCENE6,
  GP_COMMAND_STORE_SCENE7,
  GP_COMMAND_OFF,
  GP_COMMAND_ON,
  GP_COMMAND_TOGGLE,
  GP_COMMAND_LEVEL_CONTROL_MOVE_UP = 0x30,
  GP_COMMAND_LEVEL_CONTROL_MOVE_DOWN,
  GP_COMMAND_LEVEL_CONTROL_STEP_UP,
  GP_COMMAND_LEVEL_CONTROL_STEP_DOWN,
  GP_COMMAND_LEVEL_CONTROL_STOP,
  GP_COMMAND_LEVEL_CONTROL_MOVE_UP_WITH_ON_OFF,
  GP_COMMAND_LEVEL_CONTROL_MOVE_DOWN_UP_WITH_ON_OFF,
  GP_COMMAND_LEVEL_CONTROL_STEP_UP_UP_WITH_ON_OFF,
  GP_COMMAND_LEVEL_CONTROL_STEP_DOWN_UP_WITH_ON_OFF,
  GP_COMMAND_MOVE_HUE_STOP = 0x40,
  GP_COMMAND_MOVE_HUE_UP,
  GP_COMMAND_MOVE_HUE_DOWN,
  GP_COMMAND_STEP_HUE_UP,
  GP_COMMAND_STEP_HUE_DOWN,
  GP_COMMAND_MOVE_SATURATION_STOP,
  GP_COMMAND_MOVE_SATURATION_UP,
  GP_COMMAND_MOVE_SATURATION_DOWN,
  GP_COMMAND_STEP_SATURATION_UP,
  GP_COMMAND_STEP_SATURATION_DOWN,
  GP_COMMAND_MOVE_COLOR,
  GP_COMMAND_STEP_COLOR,
  GP_COMMAND_LOCK_DOOR = 0x50,
  GP_COMMAND_UNLOCK_DOOR,
  GP_COMMAND_ATTR_REPORTING = 0xA0,
  GP_COMMAND_MFR_SPECIFIC_ATTR_REPORTING,
  GP_COMMAND_MULTICLUSTER_REPORTING,
  GP_COMMAND_MULTICLUSTER_MFR_SPECIFIC_REPORTING,
  GP_COMMAND_REQUEST_ATTR,
  GP_COMMAND_READ_ATTR_RSP,
  GP_COMMAND_ZCL_TUNNELING,
};

/******************************************************************************
 * CONSTANTS
 */
/* GPDF COMMISSIONING COMMAND SETTINGS */

/// COMMISIONING OPTIONS
#define OPT_APPLICATION_INFORMATION                TRUE
#define OPT_PAN_ID_REQUEST                         FALSE
#define OPT_GP_SECURITY_REQUEST                    FALSE
#define OPT_FIXED_LOCATION                         FALSE
#if ((GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT))
#define OPT_EXTENDED_OPTIONS                       TRUE
#elif (GP_SECURITY_LEVEL == GP_SECURITY_LVL_NO_SEC)
#define OPT_EXTENDED_OPTIONS                       FALSE
#endif

/// COMMISIONING EXTENDED OPTIONS
#define EXOPT_SECURITY_LEVEL                       GP_SECURITY_LEVEL
#define EXOPT_KEY_TYPE                             EXT_OPT_KEY_TYPE
#if ((GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC) || (GP_SECURITY_LEVEL == GP_SECURITY_LVL_4FC_4MIC_ENCRYPT))
#define EXOPT_GPD_KEY_PRESENT                      TRUE
#define EXOPT_KEY_ENCRYPTION                       TRUE
#elif (GP_SECURITY_LEVEL == GP_SECURITY_LVL_NO_SEC)
#define EXOPT_GPD_KEY_PRESENT                      FALSE
#define EXOPT_KEY_ENCRYPTION                       FALSE
#endif

#if (EXOPT_KEY_ENCRYPTION == TRUE)
#define EXOPT_OUTGOING_COUNTER                     TRUE
#elif (EXOPT_KEY_ENCRYPTION == FALSE)
#define EXOPT_OUTGOING_COUNTER                     FALSE
#endif

#define GPD_COMMISIONING_OPTIONS         (uint8_t)(((SEQUENCE_NUMBER_CAP)<<0) | ((RX_AFTER_TX)<<1) | ((OPT_APPLICATION_INFORMATION)<<2) | ((OPT_PAN_ID_REQUEST)<<4) | ((OPT_GP_SECURITY_REQUEST)<<5)| ((OPT_FIXED_LOCATION)<<6) | ((OPT_EXTENDED_OPTIONS)<<7))
#define GPD_COMMISIONING_EXOPTIONS       (uint8_t)(((GP_SECURITY_LEVEL)&0x03) | (((EXOPT_KEY_TYPE)&0x07)<<2) | (((EXOPT_GPD_KEY_PRESENT)&0x01)<<5) | (((EXOPT_KEY_ENCRYPTION)&0x01)<<6) | (((EXOPT_OUTGOING_COUNTER)&0x01)<<7))


/* APPLICATION DESCRIPTION COMMAND   */
#define APPDESC_TIMEOUT_PERIOD_PRESENT             FALSE
#define APPDESC_REPORT_OPTIONS                     (uint8_t)((APPDESC_TIMEOUT_PERIOD_PRESENT)<<0)

#define DATAP_NUMBER_ATTRIBUTE_RECORDS             0x000
#define DATAP_CLIENT_SERVER                        TRUE
#define DATAP_MANUFACTURER_PRESENT                 FALSE
#define APPDESC_DATA_POINT_OPTIONS                 (uint8_t)(((DATAP_NUMBER_ATTRIBUTE_RECORDS)<<0) | (((DATAP_CLIENT_SERVER))<<3) | (((DATAP_MANUFACTURER_PRESENT))<<4))

#define ATTOPT_REAMINING_ATTIBUTE_RECORD_LENGTH    0x0000
#define ATTOPT_REPORTED                            TRUE
#define ATTOPT_ATTRIBUTE_VALUE_PRESENT             FALSE
#define APPDESC_ATTRIBUTE_OPTIONS                  (uint8_t)(((ATTOPT_REAMINING_ATTIBUTE_RECORD_LENGTH)<<0) | (((ATTOPT_REPORTED))<<4) | (((ATTOPT_ATTRIBUTE_VALUE_PRESENT))<<5))

#ifdef __cplusplus
}
#endif


#endif /* GPD_COMMON_H_ */
