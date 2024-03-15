/**************************************************************************************************
  Filename:       zcl_samplesw_data.c
  Revised:        $Date: 2014-07-30 12:57:37 -0700 (Wed, 30 Jul 2014) $
  Revision:       $Revision: 39591 $


  Description:    Zigbee Cluster Library - sample device application.


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

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_poll_control.h"
#include "zcl_electrical_measurement.h"
#include "zcl_diagnostic.h"
#include "zcl_meter_identification.h"
#include "zcl_appliance_identification.h"
#include "zcl_appliance_events_alerts.h"
#include "zcl_power_profile.h"
#include "zcl_appliance_control.h"
#include "zcl_appliance_statistics.h"
#include "zcl_hvac.h"


#ifdef OTA_CLIENT_INTEGRATED
#include "ota_client.h"
#include "zcl_ota.h"
#endif

#include "zcl_samplesw.h"

/*********************************************************************
 * CONSTANTS
 */

#define SAMPLESW_DEVICE_VERSION     1
#define SAMPLESW_FLAGS              0

#define SAMPLESW_HWVERSION          0
#define SAMPLESW_ZCLVERSION         BASIC_ZCL_VERSION

#define DEFAULT_IDENTIFY_TIME 0

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

//global attributes
const uint16_t zclSampleSw_basic_clusterRevision = 0x0002;
const uint16_t zclSampleSw_identify_clusterRevision = 0x0001;
const uint16_t zclSampleSw_onoff_clusterRevision = 0x0001;
const uint16_t zclSampleSw_onoffswconfig_clusterRevision = 0x0001;

// Basic Cluster
const uint8_t zclSampleSw_HWRevision = SAMPLESW_HWVERSION;
const uint8_t zclSampleSw_ZCLVersion = SAMPLESW_ZCLVERSION;
const uint8_t zclSampleSw_ManufacturerName[] = { 16, 'T','e','x','a','s','I','n','s','t','r','u','m','e','n','t','s' };
const uint8_t zclSampleSw_PowerSource = POWER_SOURCE_MAINS_1_PHASE;
uint8_t zclSampleSw_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;

// Identify Cluster
uint16_t zclSampleSw_IdentifyTime = 0;

#ifdef OTA_CLIENT_INTEGRATED
// global attributes
const uint16_t zclSampleSw_ota_clusterRevision = 0x0001;
// OTA Client Cluster
static uint16_t zclSampleSw_ImageType = OTA_TYPE_ID;
static uint8_t  zclSampleSw_ImageUpgradeStatus = 0;
static uint8_t  zclSampleSw_UpgradeServerID[Z_EXTADDR_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint16_t zclSampleSw_MinBlockReqDelay = 0;
static uint32_t zclSampleSw_FileOffset = 0xFFFFFFFF;
static uint32_t zclSampleSw_CurrentFileVersion = OTA_APP_VERSION;
static uint16_t zclSampleSw_CurrentZigBeeStackVersion = OTA_STACK_VER_PRO;
static uint32_t zclSampleSw_DownloadedFileVersion = 0xFFFFFFFF;
static uint16_t zclSampleSw_DownloadedZigBeeStackVersion = 0xFFFF;
static uint16_t zclSampleSw_ManufacturerID = OTA_MANUFACTURER_ID;
#endif

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

  // NOTE: The attributes listed in the AttrRec must be in ascending order
  // per cluster to allow right function of the Foundation discovery commands

CONST zclAttrRec_t zclSampleSw_Attrs[] =
{
  // *** General Basic Cluster Attributes ***
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_ZCL_VERSION,
      ZCL_DATATYPE_UINT8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleSw_ZCLVersion
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,             // Cluster IDs - defined in the foundation (ie. zcl.h)
    {  // Attribute record
      ATTRID_BASIC_HW_VERSION,            // Attribute ID - Found in Cluster Library header (ie. zcl_general.h)
      ZCL_DATATYPE_UINT8,                 // Data Type - found in zcl.h
      ACCESS_CONTROL_READ,                // Variable access control - found in zcl.h
      (void *)&zclSampleSw_HWRevision  // Pointer to attribute variable
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_MANUFACTURER_NAME,
      ZCL_DATATYPE_CHAR_STR,
      ACCESS_CONTROL_READ,
      (void *)zclSampleSw_ManufacturerName
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_POWER_SOURCE,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleSw_PowerSource
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    { // Attribute record
      ATTRID_BASIC_PHYSICAL_ENVIRONMENT,
      ZCL_DATATYPE_ENUM8,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleSw_PhysicalEnvironment
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_BASIC,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ,
      (void *)&zclSampleSw_basic_clusterRevision
    }
  },

  // *** Identify Cluster Attribute ***
  {
    ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
    { // Attribute record
      ATTRID_IDENTIFY_IDENTIFY_TIME,
      ZCL_DATATYPE_UINT16,
      (ACCESS_CONTROL_READ | ACCESS_CONTROL_WRITE),
      (void *)&zclSampleSw_IdentifyTime
    }
  },
  {
    ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_GLOBAL,
      (void *)&zclSampleSw_identify_clusterRevision
    }
  },
  // *** On / Off Cluster *** //
  {
    ZCL_CLUSTER_ID_GENERAL_ON_OFF,
    {  // Attribute record
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      (void *)&zclSampleSw_onoff_clusterRevision
    }
  }
#if defined (OTA_CLIENT_INTEGRATED)
  ,{
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_UPGRADE_SERVER_ID,
      ZCL_DATATYPE_IEEE_ADDR ,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_UpgradeServerID
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_FILE_OFFSET,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_FileOffset
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_CURRENT_FILE_VERSION,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_CurrentFileVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_CURRENT_ZIG_BEE_STACK_VERSION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_CurrentZigBeeStackVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_DOWNLOADED_FILE_VERSION,
      ZCL_DATATYPE_UINT32,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_DownloadedFileVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_DOWNLOADED_ZIG_BEE_STACK_VERSION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_DownloadedZigBeeStackVersion
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_IMAGE_UPGRADE_STATUS,
      ZCL_DATATYPE_ENUM8,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_ImageUpgradeStatus
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_MANUFACTURER_ID,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_ManufacturerID
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_IMAGE_TYPE_ID,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_ImageType
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_OTA_UPGRADE_MINIMUM_BLOCK_PERIOD,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_MinBlockReqDelay
    }
  },
  {
    ZCL_CLUSTER_ID_OTA,
    {
      ATTRID_CLUSTER_REVISION,
      ZCL_DATATYPE_UINT16,
      ACCESS_CONTROL_READ | ACCESS_CLIENT,
      ( void * ) &zclSampleSw_ota_clusterRevision
    }
  }
#endif
};

uint8_t CONST zclSampleSw_NumAttributes = ( sizeof(zclSampleSw_Attrs) / sizeof(zclSampleSw_Attrs[0]) );

/*********************************************************************
 * SIMPLE DESCRIPTOR
 */
// This is the Cluster ID List and should be filled with Application
// specific cluster IDs.
const cId_t zclSampleSw_InClusterList[] =
{
  ZCL_CLUSTER_ID_GENERAL_BASIC,
  ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
};

#define ZCLSAMPLESW_MAX_INCLUSTERS    ( sizeof( zclSampleSw_InClusterList ) / sizeof( zclSampleSw_InClusterList[0] ))

const cId_t zclSampleSw_OutClusterList[] =
{
  ZCL_CLUSTER_ID_GENERAL_IDENTIFY,
  ZCL_CLUSTER_ID_GENERAL_ON_OFF,
  ZCL_CLUSTER_ID_GENERAL_GROUPS,
#if defined (OTA_CLIENT_INTEGRATED)
  ZCL_CLUSTER_ID_OTA
#endif
};

#define ZCLSAMPLESW_MAX_OUTCLUSTERS   ( sizeof( zclSampleSw_OutClusterList ) / sizeof( zclSampleSw_OutClusterList[0] ))

SimpleDescriptionFormat_t zclSampleSw_SimpleDesc =
{
  SAMPLESW_ENDPOINT,                  //  int Endpoint;
  ZCL_HA_PROFILE_ID,                  //  uint16_t AppProfId[2];
  ZCL_DEVICEID_ON_OFF_LIGHT_SWITCH,//  uint16_t AppDeviceId[2];
  SAMPLESW_DEVICE_VERSION,            //  int   AppDevVer:4;
  SAMPLESW_FLAGS,                     //  int   AppFlags:4;
  ZCLSAMPLESW_MAX_INCLUSTERS,         //  byte  AppNumInClusters;
  (cId_t *)zclSampleSw_InClusterList, //  byte *pAppInClusterList;
  ZCLSAMPLESW_MAX_OUTCLUSTERS,        //  byte  AppNumInClusters;
  (cId_t *)zclSampleSw_OutClusterList //  byte *pAppInClusterList;
};

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

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
void zclSampleSw_ResetAttributesToDefaultValues(void)
{

  zclSampleSw_PhysicalEnvironment = PHY_UNSPECIFIED_ENV;
  zclSampleSw_IdentifyTime = DEFAULT_IDENTIFY_TIME;

#ifdef OTA_CLIENT_INTEGRATED
  zclSampleSw_ImageUpgradeStatus = 0;
  memset(zclSampleSw_UpgradeServerID, 0xFF, Z_EXTADDR_LEN);
  zclSampleSw_MinBlockReqDelay = 0;
  zclSampleSw_FileOffset = 0xFFFFFFFF;
  zclSampleSw_DownloadedFileVersion = 0xFFFFFFFF;
  zclSampleSw_DownloadedZigBeeStackVersion = 0xFFFF;
#endif
}

/****************************************************************************
****************************************************************************/


