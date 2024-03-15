/**************************************************************************************************
  Filename:       zcl_sampledoorlock_data.c
  Revised:        $Date: 2014-09-25 13:20:41 -0700 (Thu, 25 Sep 2014) $
  Revision:       $Revision: 40295 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2013-2014 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_hvac.h"
#include "zcl_closures.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_events_alerts.h"

#include "zcl_sampledoorlock.h"

/*********************************************************************
 * CONSTANTS
 */

#define SAMPLEDOORLOCK_DEVICE_VERSION     0
#define SAMPLEDOORLOCK_FLAGS              0

#define SAMPLEDOORLOCK_HWVERSION          1
#define SAMPLEDOORLOCK_ZCLVERSION         BASIC_ZCL_VERSION

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Global attributes
const uint16_t zclSampleDoorLock_basic_clusterRevision = 0x0002;
const uint16_t zclSampleDoorLock_identify_clusterRevision = 0x0001;
const uint16_t zclSampleDoorLock_scenes_clusterRevision = 0x0001;
const uint16_t zclSampleDoorLock_groups_clusterRevision = 0x0001;
const uint16_t zclSampleDoorLock_doorlock_clusterRevision = 0x0001;

// Basic Cluster
const uint8_t zclSampleDoorLock_HWRevision = SAMPLEDOORLOCK_HWVERSION;
const uint8_t zclSampleDoorLock_ZCLVersion = SAMPLEDOORLOCK_ZCLVERSION;
const uint8_t zclSampleDoorLock_ManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8_t zclSampleDoorLock_PowerSource = POWER_SOURCE_MAINS_1_PHASE;
uint8_t zclSampleDoorLock_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;

// Identify Cluster
uint16_t zclSampleDoorLock_IdentifyTime;

// Doorlock Cluster
static uint8_t zclSampleDoorLock_LockState = CLOSURES_LOCK_STATE_LOCKED;
uint8_t zclSampleDoorLock_LockType = CLOSURES_LOCK_TYPE_DEADBOLT;
bool zclSampleDoorLock_ActuatorEnabled = TRUE;
uint8_t zclSampleDoorLock_GroupsNameSupport = 0;

// Scenes attributes
uint8_t zclSampleDoorLock_ScenesCurrentScene = 0;
uint16_t zclSampleDoorLock_ScenesCurrentGroup = 0;
uint8_t zclSampleDoorLock_ScenesValid = 0;
uint8_t zclSampleDoorLock_ScenesNameSupport = 0;


/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

// NOTE: The attributes listed in the AttrRec must be in ascending order
// per cluster to allow right function of the Foundation discovery commands

CONST zclAttrRec_t zclSampleDoorLock_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclSampleDoorLock_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclSampleDoorLock_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENVIRONMENT,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleDoorLock_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_basic_clusterRevision
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleDoorLock_IdentifyTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_identify_clusterRevision
    }
  },

    // *** Scene Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_SCENE_COUNT,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      NULL // Use application's callback to Read this attribute
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_CURRENT_SCENE,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_ScenesCurrentScene
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_CURRENT_GROUP,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_ScenesCurrentGroup
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_SCENE_VALID,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_ScenesValid
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_ScenesNameSupport
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_scenes_clusterRevision
    }
  },
  // *** Door Lock Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
    { // Attribute record
      ATTRID_DOOR_LOCK_LOCK_STATE,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_REPORTABLE),
      (void*)&zclSampleDoorLock_LockState
    }
  },
  {
    ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
    { // Attribute record
      ATTRID_DOOR_LOCK_LOCK_TYPE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_LockType
    }
  },
  {
    ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
    { // Attribute record
      ATTRID_DOOR_LOCK_ACTUATOR_ENABLED,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_ActuatorEnabled
    }
  },
  {
    ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_doorlock_clusterRevision
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_GROUPS,
    {
      ATTRID_GROUPS_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void*)&zclSampleDoorLock_GroupsNameSupport
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_GROUPS,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleDoorLock_groups_clusterRevision
    }
  },
};

uint8_t CONST zclSampleDoorLock_NumAttributes = ( sizeof(zclSampleDoorLock_Attrs) / sizeof(zclSampleDoorLock_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
#define ZCLSAMPLEDOORLOCK_MAX_INCLUSTERS       5
const cId_t zclSampleDoorLock_InClusterList[ZCLSAMPLEDOORLOCK_MAX_INCLUSTERS] =
{
  ZCL_CLUSTER_ID_GENERAL_BASIC,
  ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
  ZCL_CLUSTER_ID_GENERAL_GROUPS,
  ZCL_CLUSTER_ID_GENERAL_SCENES,
  ZCL_CLUSTER_ID_CLOSURES_DOOR_LOCK
};

#define ZCLSAMPLEDOORLOCK_MAX_OUTCLUSTERS       0
/*
const cId_t zclSampleDoorLock_OutClusterList[ZCLSAMPLEDOORLOCK_MAX_OUTCLUSTERS] =
{
  //Add the output clusters if needed. Don't forget to update ZCLSAMPLEDOORLOCK_MAX_OUTCLUSTERS and zclSampleDoorLock_SimpleDesc accordingly.
};
*/

SimpleDescriptionFormat_t zclSampleDoorLock_SimpleDesc =
{
  SAMPLEDOORLOCK_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                        //  uint16_t AppProfId[2];
  ZCL_DEVICEID_DOOR_LOCK,                //  uint16_t AppDeviceId[2];
  SAMPLEDOORLOCK_DEVICE_VERSION,            //  int   AppDevVer:4;
  SAMPLEDOORLOCK_FLAGS,                     //  int   AppFlags:4;
  ZCLSAMPLEDOORLOCK_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclSampleDoorLock_InClusterList, //  byte *pAppInClusterList;
  ZCLSAMPLEDOORLOCK_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  NULL,//  byte *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      zclSampleDoorLock_ResetAttributesToDefaultValues
 *
 * @brief   Reset all writable attributes to their default values.
 *
 * @param   none
 *
 * @return  none
 */
void zclSampleDoorLock_ResetAttributesToDefaultValues(void)
{
  zclSampleDoorLock_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;

#ifdef ZCL_IDENTIFY
  zclSampleDoorLock_IdentifyTime = 0;
#endif

  zclSampleDoorLock_updateLockStateAttribute(CLOSURES_LOCK_STATE_LOCKED);
}


/*********************************************************************
 * @fn      zclSampleDoorLock_getLockStateAttribute
 *
 * @brief   Function to get the state of the Lock State attribute.
 *
 * @param   none
 *
 * @return  Lock State attribute state (True if lock, False if unlock)
 */
uint8_t zclSampleDoorLock_getLockStateAttribute(void)
{
    return zclSampleDoorLock_LockState;
}


/*********************************************************************
 * @fn      zclSampleDoorLock_updateLockStateAttribute
 *
 * @brief   Function that updates the Lock State attribute, this will also update scene attribute.
 *
 * @param   Lock state
 *
 * @return  none
 */
void zclSampleDoorLock_updateLockStateAttribute(uint8_t LockState)
{
    if(zclSampleDoorLock_LockState != LockState)
    {
        zclSampleDoorLock_LockState = LockState;
        zclSampleDoorLock_ScenesValid = FALSE;
    }
}

/****************************************************************************
****************************************************************************/


