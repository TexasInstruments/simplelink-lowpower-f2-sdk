/**************************************************************************************************
  Filename:       zcl_retail.h
  Revised:        $Date: 2018-22-11 10:12:34 -0700 (Thur, 22 Nov 2018) $
  Revision:       $Revision: 25678 $

  Description:    Zigbee Cluster Library - Retail Services Cluster


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

#ifndef ZCL_RETAIL_H
#define ZCL_RETAIL_H

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

/**********************************************/
/*** Retail Tunnel Clusters Attributes List ***/
/**********************************************/
#define ATTRID_RETAIL_TUNNEL_MANUFACTURER_CODE        0x0000
#define ATTRID_RETAIL_TUNNEL_MSPROFILE                0x0001

/**********************************************/
/*** Mobile Device Clusters Attributes List ***/
/**********************************************/
#define ATTRID_RETAIL_MOBILE_KEEP_ALIVE_TIME          0x0000
#define ATTRID_RETAIL_MOBILE_REJOIN_TIMEOUT           0x0001

/**************************************************/
/*** Neighbor Cleaning Clusters Attributes List ***/
/**************************************************/
#define ATTRID_RETAIL_NEIGHBOR_CLEANING_TIMEOUT       0x0000

/************************************************/
/*** Nearest Gateway Clusters Attributes List ***/
/************************************************/
#define ATTRID_RETAIL_NEAREST_GW_NEAREST_GW           0x0000
#define ATTRID_RETAIL_NEAREST_GW_NEW_MOBILE_NODE      0x0001

/*************************************/
/***Retail Tunnel Cluster Commands ***/
/*************************************/
// Commands Received by Retail tunnel Cluster Server
#define COMMAND_ID_RETAIL_TUNNEL_TRANSFER_APDU              0x00


/****************************************/
/***  Mobile Device Cluster Commands  ***/
/****************************************/
// Commands Received by Mobile Device Cluster Server
#define COMMAND_ID_RETAIL_MOBILE_KEEP_ALIVE_NOTI            0x00


/***********************************************/
/***    Neighbor Cleaning Cluster Commands   ***/
/***********************************************/
// Commands Received by Neighbor Cleaning Cluster Server
#define COMMAND_ID_RETAIL_NEIGHBOR_PURGE_ENTRIES            0x00

/******************************************************************/
/***        Other Constants                                     ***/
/******************************************************************/




/********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

/*** Structures used for callback functions ***/

typedef struct
{
    afAddrType_t *srcAddr;  // requestor's address
    uint8_t *apdu;            // retail tunnel APDU
}zclRetailTunnelTransferAPDU_t;

typedef struct
{
    afAddrType_t *srcAddr;  // requestor's address
    uint16_t keepAliveTime;
    uint16_t rejoinTimeout;

}zclMobileDeviceConfigurationKeepAliveNotification_t;

typedef struct
{
    afAddrType_t *srcAddr;  // requestor's address
}zclNeighborCleaningPurgeEntries_t;


typedef void (*zclPICB_RetailTunnelTransferAPDU_t)( zclRetailTunnelTransferAPDU_t *pCmd );

typedef void (*zclPICB_MobileDeviceConfigurationKeepAliveNotification_t)( zclMobileDeviceConfigurationKeepAliveNotification_t *pCmd );

typedef void (*zclPICB_NeighborCleaningPurgeEntries_t)( zclNeighborCleaningPurgeEntries_t *pCmd );


typedef struct
{
    zclPICB_RetailTunnelTransferAPDU_t           pfnPI_RetailTunnelTransferAPDU;
}zclPIRetail_AppCallbacks_t;

typedef struct
{
    zclPICB_MobileDeviceConfigurationKeepAliveNotification_t    pfnPI_MobileDeviceConfigurationKeepAliveNotification;
    zclPICB_NeighborCleaningPurgeEntries_t                      pfnPI_NeighborCleaningPurgeEntries;
}zclRetailCluster_AppCallbacks_t;


/*********************************************************************
 * CALLBACKS
 */

/*********************************************************************
 * VARIABLES
 */

/*********************************************************************
 * FUNCTION MACROS
 */


/*********************************************************************
 * FUNCTIONS
 */

extern ZStatus_t zclRetailCluster_RegisterCmdCallbacks( uint8_t endpoint, zclRetailCluster_AppCallbacks_t *callbacks );

extern ZStatus_t zclPIRetailTunnel_RegisterCmdCallbacks( uint8_t endpoint, zclPIRetail_AppCallbacks_t *callbacks );
extern ZStatus_t zclRetailCluster_RegisterCmdCallbacks( uint8_t endpoint, zclRetailCluster_AppCallbacks_t *callbacks );
extern ZStatus_t zclPI_Send_RetailTunnelTransferAPDUCmd( uint8_t srcEP, afAddrType_t *dstAddr, zclFrameHdr_t *hdr,
                                           uint8_t len, uint8_t *apdu, uint8_t seqNum );

#ifdef __cplusplus
}
#endif

#endif /* ZCL_RETAIL_H */
