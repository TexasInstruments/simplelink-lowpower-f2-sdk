/**************************************************************************************************
  Filename:       zcl_samplelight_data.c
  Revised:        $Date: 2014-05-12 13:14:02 -0700 (Mon, 12 May 2014) $
  Revision:       $Revision: 38502 $


  Description:    Zigbee Cluster Library - sample device application.


  Copyright 2006-2014 Texas Instruments Incorporated. All rights reserved.

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
#include "zcl_poll_control.h"
#include "zcl_electrical_measurement.h"
#include "zcl_meter_identification.h"
#include "zcl_appliance_identification.h"
#include "zcl_appliance_events_alerts.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_statistics.h"
#include "zcl_hvac.h"

#include "zcl_samplelight.h"
#ifdef BDB_REPORTING
#include "zstackapi.h"
#endif
/*********************************************************************
 * CONSTANTS
 */

#define SAMPLELIGHT_DEVICE_VERSION     1
#define SAMPLELIGHT_FLAGS              0

#define SAMPLELIGHT_HWVERSION          1
#define SAMPLELIGHT_ZCLVERSION         BASIC_ZCL_VERSION

#define DEFAULT_IDENTIFY_TIME 0
#define DEFAULT_ON_OFF_TRANSITION_TIME 20
#define DEFAULT_ON_LEVEL ATTR_LEVEL_ON_LEVEL_NO_EFFECT
#define DEFAULT_ON_TRANSITION_TIME 20
#define DEFAULT_OFF_TRANSITION_TIME 20
#define DEFAULT_MOVE_RATE 0 // as fast as possible

#define DEFAULT_ON_OFF_STATE LIGHT_OFF
#define DEFAULT_LEVEL ATTR_LEVEL_MAX_LEVEL

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * EXTERNAL VARIABLES
 */
extern uint8_t  appServiceTaskId;

/*********************************************************************
 * GLOBAL VARIABLES
 */

//global attributes
const uint16_t zclSampleLight_basic_clusterRevision = 0x0002;
const uint16_t zclSampleLight_identify_clusterRevision = 0x0001;
const uint16_t zclSampleLight_groups_clusterRevision = 0x0001;
const uint16_t zclSampleLight_scenes_clusterRevision = 0x0001;
const uint16_t zclSampleLight_onoff_clusterRevision = 0x0001;
const uint16_t zclSampleLight_level_clusterRevision = 0x0001;


// Basic Cluster
const uint8_t zclSampleLight_HWRevision = SAMPLELIGHT_HWVERSION;
const uint8_t zclSampleLight_ZCLVersion = SAMPLELIGHT_ZCLVERSION;
const uint8_t zclSampleLight_ManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8_t zclSampleLight_PowerSource = POWER_SOURCE_MAINS_1_PHASE;
uint8_t zclSampleLight_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;

// Identify Cluster
uint16_t zclSampleLight_IdentifyTime;

// Groups Cluster
uint8_t zclSampleLight_GroupsNameSupport = 0;

// On/Off Cluster
static uint8_t  zclSampleLight_OnOff;

// Level Control Cluster
#ifdef ZCL_LEVEL_CTRL
static uint8_t  zclSampleLight_LevelCurrentLevel;
uint16_t zclSampleLight_LevelRemainingTime;
uint16_t zclSampleLight_LevelOnOffTransitionTime;
uint8_t  zclSampleLight_LevelOnLevel;
uint16_t zclSampleLight_LevelOnTransitionTime;
uint16_t zclSampleLight_LevelOffTransitionTime;
uint8_t  zclSampleLight_LevelDefaultMoveRate;
#endif



uint8_t  zclSampleLight_ScenesCurrentScene = 0;
uint16_t zclSampleLight_ScenesCurrentGroup = 0;
uint8_t  zclSampleLight_ScenesValid = 0;
uint8_t  zclSampleLight_ScenesNameSupport = 0;


#if ZCL_DISCOVER
CONST zclCommandRec_t zclSampleLight_Cmds[] =
{
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    COMMAND_BASIC_RESET_TO_FACTORY_DEFAULTS,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_ON_OFF,
    COMMAND_ON_OFF_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_ON_OFF,
    COMMAND_ON_OFF_ON,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_ON_OFF,
    COMMAND_ON_OFF_TOGGLE,
    CMD_DIR_SERVER_RECEIVED
  },
#ifdef ZCL_LEVEL_CTRL
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_TO_LEVEL_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_MOVE_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_STEP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    COMMAND_LEVEL_STOP_WITH_ON_OFF,
    CMD_DIR_SERVER_RECEIVED
  }
#endif // ZCL_LEVEL_CTRL
};

CONST uint8_t zclCmdsArraySize = ( sizeof(zclSampleLight_Cmds) / sizeof(zclSampleLight_Cmds[0]) );
#endif // ZCL_DISCOVER

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

// NOTE: The attributes listed in the AttrRec must be in ascending order
// per cluster to allow right function of the Foundation discovery commands

CONST zclAttrRec_t zclSampleLight_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclSampleLight_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclSampleLight_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENVIRONMENT,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleLight_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_basic_clusterRevision
    }
  },
#ifdef ZCL_IDENTIFY
  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleLight_IdentifyTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_identify_clusterRevision
    }
  },
#endif

  // *** On/Off Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GENERAL_ON_OFF,
    { // Attribute record
      ATTRID_ON_OFF_ON_OFF,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void*)&zclSampleLight_OnOff
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_ON_OFF,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_onoff_clusterRevision
    }
  },

#ifdef ZCL_LEVEL_CTRL
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_CURRENT_LEVEL,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_REPORTABLE,
      (void*)&zclSampleLight_LevelCurrentLevel
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_REMAINING_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_LevelRemainingTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_OFF_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOnOffTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_LEVEL,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOnLevel
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_ON_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOnTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_OFF_TRANSITION_TIME,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelOffTransitionTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    { // Attribute record
      ATTRID_LEVEL_DEFAULT_MOVE_RATE,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE,
      (void *)&zclSampleLight_LevelDefaultMoveRate
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_level_clusterRevision
    }
  },
#endif
#ifdef ZCL_GROUPS
  {
    ZCL_CLUSTER_ID_GENERAL_GROUPS,
    {
      ATTRID_GROUPS_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void*)&zclSampleLight_GroupsNameSupport
    }
  },

  {
    ZCL_CLUSTER_ID_GENERAL_GROUPS,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_groups_clusterRevision
    }
  },
#endif
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
      (void *)&zclSampleLight_ScenesCurrentScene
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_CURRENT_GROUP,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_ScenesCurrentGroup
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_SCENE_VALID,
      ZCL_DATATYPE_BOOLEAN,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_ScenesValid
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    { // Attribute record
      ATTRID_SCENES_NAME_SUPPORT,
      ZCL_DATATYPE_BITMAP8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_ScenesNameSupport
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_SCENES,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleLight_scenes_clusterRevision
    }
  },





};

uint8_t CONST zclSampleLight_NumAttributes = ( sizeof(zclSampleLight_Attrs) / sizeof(zclSampleLight_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
const cId_t zclSampleLight_InClusterList[] =
{
  ZCL_CLUSTER_ID_GENERAL_BASIC,
  ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
  ZCL_CLUSTER_ID_GENERAL_GROUPS,
  ZCL_CLUSTER_ID_GENERAL_SCENES,
  ZCL_CLUSTER_ID_GENERAL_ON_OFF
#ifdef ZCL_LEVEL_CTRL
  , ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL
#endif
};

#define ZCLSAMPLELIGHT_MAX_INCLUSTERS   (sizeof(zclSampleLight_InClusterList) / sizeof(zclSampleLight_InClusterList[0]))

SimpleDescriptionFormat_t zclSampleLight_SimpleDesc =
{
  SAMPLELIGHT_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                     //  uint16_t AppProfId;
#ifdef ZCL_LEVEL_CTRL
  ZCL_DEVICEID_DIMMABLE_LIGHT,        //  uint16_t AppDeviceId;
#else
  ZCL_DEVICEID_ON_OFF_LIGHT,          //  uint16_t AppDeviceId;
#endif
  SAMPLELIGHT_DEVICE_VERSION,            //  int   AppDevVer:4;
  SAMPLELIGHT_FLAGS,                     //  int   AppFlags:4;
  ZCLSAMPLELIGHT_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclSampleLight_InClusterList, //  byte *pAppInClusterList;
  0,        //  byte  AppNumInClusters;
  NULL //  byte *pAppInClusterList;
};

// Added to include Touchlink Target functionality
#if defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
bdbTLDeviceInfo_t tlSampleLight_DeviceInfo =
{
  ZCL_HA_PROFILE_ID,                        //uint16_t profileID;
#ifdef ZCL_LEVEL_CTRL
      ZCL_DEVICEID_DIMMABLE_LIGHT,        //  uint16_t AppDeviceId;
#else
      ZCL_DEVICEID_ON_OFF_LIGHT,          //  uint16_t AppDeviceId;
#endif
  SAMPLELIGHT_ENDPOINT,                  //uint8_t endpoint;
  SAMPLELIGHT_DEVICE_VERSION,                    //uint8_t version;
  SAMPLELIGHT_NUM_GRPS                   //uint8_t grpIdCnt;
};
#endif
/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * @fn      zclSampleLight_updateOnOffAttribute
 *
 * @brief   Function that updates the OnOff attribute, this will also update scene attribute.
 *
 * @param   OnOff state
 *
 * @return  none
 */
void zclSampleLight_updateOnOffAttribute(uint8_t OnOff)
{
    if(zclSampleLight_OnOff != OnOff)
    {
        zclSampleLight_OnOff = OnOff;
        zclSampleLight_ScenesValid = FALSE;

#ifdef BDB_REPORTING
        zstack_bdbRepChangedAttrValueReq_t Req;

        Req.attrID = ATTRID_ON_OFF_ON_OFF;
        Req.cluster = ZCL_CLUSTER_ID_GENERAL_ON_OFF;
        Req.endpoint = SAMPLELIGHT_ENDPOINT;

      Zstackapi_bdbRepChangedAttrValueReq(appServiceTaskId,&Req);
#endif
    }
}

/*********************************************************************
 * @fn      zclSampleLight_getOnOffAttribute
 *
 * @brief   Function to get the state of the OnOff attribute.
 *
 * @param   none
 *
 * @return  OnOff attribute state (True if on, False if Off)
 */
uint8_t zclSampleLight_getOnOffAttribute(void)
{
    return zclSampleLight_OnOff;
}

#ifdef ZCL_LEVEL_CTRL
/*********************************************************************
 * @fn      zclSampleLight_updateCurrentLevelAttribute
 *
 * @brief   Function that updates the OnOff attribute, this will also update scene attribute.
 *
 * @param   Current Level state
 *
 * @return  none
 */
void zclSampleLight_updateCurrentLevelAttribute(uint8_t CurrentLevel)
{
    if(zclSampleLight_LevelCurrentLevel != CurrentLevel)
    {
        zclSampleLight_LevelCurrentLevel = CurrentLevel;
        zclSampleLight_ScenesValid = FALSE;

#ifdef BDB_REPORTING
        zstack_bdbRepChangedAttrValueReq_t Req;

        Req.attrID = ATTRID_LEVEL_CURRENT_LEVEL;
        Req.cluster = ZCL_CLUSTER_ID_GENERAL_LEVEL_CONTROL;
        Req.endpoint = SAMPLELIGHT_ENDPOINT;

      Zstackapi_bdbRepChangedAttrValueReq(appServiceTaskId,&Req);
#endif
    }
}

/*********************************************************************
 * @fn      zclSampleLight_getCurrentLevelAttribute
 *
 * @brief   Function to get the state of the OnOff attribute.
 *
 * @param   none
 *
 * @return  Current Level attribute state
 */
uint8_t zclSampleLight_getCurrentLevelAttribute(void)
{
    return zclSampleLight_LevelCurrentLevel;
}
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn      zclSampleLight_ResetAttributesToDefaultValues
 *
 * @brief   Reset all writable attributes to their default values.
 *
 * @param   none
 *
 * @return  none
 */
void zclSampleLight_ResetAttributesToDefaultValues(void)
{
  zclSampleLight_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;

#ifdef ZCL_IDENTIFY
  zclSampleLight_IdentifyTime = DEFAULT_IDENTIFY_TIME;
#endif
#ifdef ZCL_LEVEL_CTRL
  zclSampleLight_LevelRemainingTime = 0;
  zclSampleLight_LevelOnOffTransitionTime = DEFAULT_ON_OFF_TRANSITION_TIME;
  zclSampleLight_LevelOnLevel = DEFAULT_ON_LEVEL;
  zclSampleLight_LevelOnTransitionTime = DEFAULT_ON_TRANSITION_TIME;
  zclSampleLight_LevelOffTransitionTime = DEFAULT_OFF_TRANSITION_TIME;
  zclSampleLight_LevelDefaultMoveRate = DEFAULT_MOVE_RATE;

  zclSampleLight_updateCurrentLevelAttribute(DEFAULT_LEVEL);
#endif

  zclSampleLight_updateOnOffAttribute(DEFAULT_ON_OFF_STATE);

  zclSampleLight_IdentifyTime = 0;
}




/****************************************************************************
****************************************************************************/
