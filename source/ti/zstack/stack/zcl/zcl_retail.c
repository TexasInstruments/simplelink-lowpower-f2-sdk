/**************************************************************************************************
  Filename:       zcl_retail.c
  Revised:        $Date: 2018-22-11 13:53:09 -0700 (Thur, 22 Nov 2018) $
  Revision:       $Revision: 34523 $

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


/*********************************************************************
 * INCLUDES
 */
#include "ti_zstack_config.h"
#include "zcl.h"
#include "zcl_general.h"
#include "zcl_retail.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */



typedef struct zclPIRetailCBRec
{
  struct zclPIRetailCBRec     *next;
  uint8_t                 endpoint; // Used to link it into the endpoint descriptor
  zclPIRetail_AppCallbacks_t  *CBs;     // Pointer to Callback function
} zclPIRetailCBRec_t;

typedef struct zclRetailClusterCBRec
{
  struct zclRetailClusterCBRec     *next;
  uint8_t                 endpoint; // Used to link it into the endpoint descriptor
  zclRetailCluster_AppCallbacks_t  *CBs;     // Pointer to Callback function
} zclRetailClusterCBRec_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */



/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclPIRetailCBRec_t *zclPIRetailCBs = (zclPIRetailCBRec_t *)NULL;
static zclRetailClusterCBRec_t *zclRetailClusterCBs = (zclRetailClusterCBRec_t *)NULL;

static uint8_t zclPIRetailPluginRegisted = FALSE;
static uint8_t zclRetailClusterPluginRegisted = FALSE;


/*********************************************************************
 * LOCAL FUNCTIONS
 */

static ZStatus_t zclPIRetailTunnel_HdlIncoming( zclIncoming_t *pInHdlrMsg );
static ZStatus_t zclRetailClusterTunnel_HdlIncoming( zclIncoming_t *pInMsg );

static zclPIRetail_AppCallbacks_t *zclPIRetail_FindCallbacks( uint8_t endpoint );
static zclRetailCluster_AppCallbacks_t *zclRetailCluster_FindCallbacks( uint8_t endpoint );

ZStatus_t zclPIRetailTunnel_RegisterCmdCallbacks( uint8_t endpoint, zclPIRetail_AppCallbacks_t *callbacks )
{
    zclPIRetailCBRec_t *pNewItem;
    zclPIRetailCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclPIRetailPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL,
                        ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL,
                        zclPIRetailTunnel_HdlIncoming );
    zclPIRetailPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclPIRetailCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclPIRetailCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclPIRetailCBs == NULL )
  {
      zclPIRetailCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclPIRetailCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}


ZStatus_t zclRetailCluster_RegisterCmdCallbacks( uint8_t endpoint, zclRetailCluster_AppCallbacks_t *callbacks )
{
    zclRetailClusterCBRec_t *pNewItem;
    zclRetailClusterCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclRetailClusterPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIGURATION,
                        ZCL_CLUSTER_ID_NEIGHBOR_CLEANING,
                        zclRetailClusterTunnel_HdlIncoming );
    zclRetailClusterPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclRetailClusterCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclRetailClusterCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclRetailClusterCBs == NULL )
  {
      zclRetailClusterCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclRetailClusterCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}



ZStatus_t zclPI_Send_RetailTunnelTransferAPDUCmd( uint8_t srcEP, afAddrType_t *dstAddr,zclFrameHdr_t *hdr,
                                           uint8_t len, uint8_t *apdu, uint8_t seqNum )
{
  uint8_t *buf;
  ZStatus_t stat;
  buf = zcl_mem_alloc(len); // 2 for length field (long octet string)
  if ( buf )
  {
    buf[0] = len;
    zcl_memcpy( &(buf[1]), apdu, len );

    // This command shall always be transmitted with the Disable Default
    // Response bit in the ZCL frame control field set to 1.
    if(zcl_ServerCmd(hdr->fc.direction)){
        stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL,
                                COMMAND_ID_RETAIL_TUNNEL_TRANSFER_APDU, TRUE,
                                ZCL_FRAME_CLIENT_SERVER_DIR, TRUE, 0, seqNum, (len), buf );
    }
    else {
        stat = zcl_SendCommand( srcEP, dstAddr, ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL,
                                COMMAND_ID_RETAIL_TUNNEL_TRANSFER_APDU, TRUE,
                                ZCL_FRAME_SERVER_CLIENT_DIR, TRUE, 0, seqNum, (len), buf );
    }

    zcl_mem_free( buf );
  }
  else
  {
    stat = ZMemError;
  }

  return ( stat );
}



static zclPIRetail_AppCallbacks_t *zclPIRetail_FindCallbacks( uint8_t endpoint )
{
    zclPIRetailCBRec_t *pCBs;

  pCBs = zclPIRetailCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
  }
  return ( (zclPIRetail_AppCallbacks_t *)NULL );
}

static zclRetailCluster_AppCallbacks_t *zclRetailCluster_FindCallbacks( uint8_t endpoint )
{
    zclRetailClusterCBRec_t *pCBs;

  pCBs = zclRetailClusterCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
  }
  return ( (zclRetailCluster_AppCallbacks_t *)NULL );
}


static ZStatus_t zclPIRetailTunnel_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
    return ( stat ); // Cluster not supported thru Inter-PAN
#endif

  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {

        //ZStatus_t stat;
        zclPIRetail_AppCallbacks_t *pCBs;

        // make sure endpoint exists
        pCBs = zclPIRetail_FindCallbacks( pInMsg->msg->endPoint );
        if ( pCBs == NULL )
          return ( ZFailure );

        switch ( pInMsg->msg->clusterId )
        {
            case ZCL_CLUSTER_ID_PI_RETAIL_TUNNEL:
                if ( pCBs->pfnPI_RetailTunnelTransferAPDU )
                {
                    zclRetailTunnelTransferAPDU_t cmd;
                    cmd.srcAddr = &(pInMsg->msg->srcAddr);
                    //cmd.len = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
                    cmd.apdu = &(pInMsg->pData[2]);
                    pCBs->pfnPI_RetailTunnelTransferAPDU( &cmd );
                }
          break;

        }//end switch

    }
    else
    {
      // We don't support any manufacturer specific command -- ignore it.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands
    stat = ZFailure;
  }

  return ( stat );
}


static ZStatus_t zclRetailClusterTunnel_HdlIncoming( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  if ( StubAPS_InterPan( pInMsg->msg->srcAddr.panId, pInMsg->msg->srcAddr.endPoint ) )
    return ( stat ); // Cluster not supported thru Inter-PAN
#endif

  if ( zcl_ClusterCmd( pInMsg->hdr.fc.type ) )
  {
    // Is this a manufacturer specific command?
    if ( pInMsg->hdr.fc.manuSpecific == 0 )
    {

        //ZStatus_t stat;
        zclRetailCluster_AppCallbacks_t *pCBs;

        // make sure endpoint exists
        pCBs = zclRetailCluster_FindCallbacks( pInMsg->msg->endPoint );
        if ( pCBs == NULL )
          return ( ZFailure );

        switch ( pInMsg->msg->clusterId )
        {
            case ZCL_CLUSTER_ID_MOBILE_DEVICE_CONFIGURATION:
                if ( pCBs->pfnPI_MobileDeviceConfigurationKeepAliveNotification )
                {
                    zclMobileDeviceConfigurationKeepAliveNotification_t cmd;
                    cmd.srcAddr = &(pInMsg->msg->srcAddr);
                    //cmd.len = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
                    pCBs->pfnPI_MobileDeviceConfigurationKeepAliveNotification( &cmd );
                }
          break;
            case ZCL_CLUSTER_ID_NEIGHBOR_CLEANING:
                if ( pCBs->pfnPI_NeighborCleaningPurgeEntries )
                {
                    zclNeighborCleaningPurgeEntries_t cmd;
                    cmd.srcAddr = &(pInMsg->msg->srcAddr);
                    //cmd.len = BUILD_UINT16( pInMsg->pData[0], pInMsg->pData[1] );
                    pCBs->pfnPI_NeighborCleaningPurgeEntries( &cmd );
                }
          break;

        }//end switch

    }
    else
    {
      // We don't support any manufacturer specific command -- ignore it.
      stat = ZFailure;
    }
  }
  else
  {
    // Handle all the normal (Read, Write...) commands
    stat = ZFailure;
  }

  return ( stat );
}


/********************************************************************************************
*********************************************************************************************/
