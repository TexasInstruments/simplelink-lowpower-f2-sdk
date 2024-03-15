/**************************************************************************************************
  Filename:       zcl_ms.c
  Revised:        $Date: 2013-06-11 13:53:09 -0700 (Tue, 11 Jun 2013) $
  Revision:       $Revision: 34523 $

  Description:    Zigbee Cluster Library - Measurements and Sensing ( MS )


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
#include "zcl_ms.h"

#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  #include "stub_aps.h"
#endif

#ifdef ZCL_MS
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct zclMSCBRec
{
  struct zclMSCBRec     *next;
  uint8_t                 endpoint; // Used to link it into the endpoint descriptor
  zclMS_AppCallbacks_t  *CBs;     // Pointer to Callback function
} zclMSCBRec_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static zclMSCBRec_t *zclMSCBs = (zclMSCBRec_t *)NULL;
static uint8_t zclMSPluginRegisted = FALSE;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static ZStatus_t zclMS_HdlIncoming( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_HdlInSpecificCommands( zclIncoming_t *pInMsg );
static zclMS_AppCallbacks_t *zclMS_FindCallbacks( uint8_t endpoint );

static ZStatus_t zclMS_ProcessIn_IlluminanceMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_IlluminanceLevelSensingCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_TemperatureMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_PressureMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_FlowMeasurementCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_RelativeHumidityCmds( zclIncoming_t *pInMsg );
static ZStatus_t zclMS_ProcessIn_OccupancySensingCmds( zclIncoming_t *pInMsg );

/*********************************************************************
 * @fn      zclMS_RegisterCmdCallbacks
 *
 * @brief   Register an applications command callbacks
 *
 * @param   endpoint - application's endpoint
 * @param   callbacks - pointer to the callback record.
 *
 * @return  ZMemError if not able to allocate
 */
ZStatus_t zclMS_RegisterCmdCallbacks( uint8_t endpoint, zclMS_AppCallbacks_t *callbacks )
{
  zclMSCBRec_t *pNewItem;
  zclMSCBRec_t *pLoop;

  // Register as a ZCL Plugin
  if ( !zclMSPluginRegisted )
  {
    zcl_registerPlugin( ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT,
                        ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING,
                        zclMS_HdlIncoming );
    zclMSPluginRegisted = TRUE;
  }

  // Fill in the new profile list
  pNewItem = zcl_mem_alloc( sizeof( zclMSCBRec_t ) );
  if ( pNewItem == NULL )
    return (ZMemError);

  pNewItem->next = (zclMSCBRec_t *)NULL;
  pNewItem->endpoint = endpoint;
  pNewItem->CBs = callbacks;

  // Find spot in list
  if ( zclMSCBs == NULL )
  {
    zclMSCBs = pNewItem;
  }
  else
  {
    // Look for end of list
    pLoop = zclMSCBs;
    while ( pLoop->next != NULL )
      pLoop = pLoop->next;

    // Put new item at end of list
    pLoop->next = pNewItem;
  }
  return ( ZSuccess );
}

/*********************************************************************
 * @fn      zclMS_FindCallbacks
 *
 * @brief   Find the callbacks for an endpoint
 *
 * @param   endpoint
 *
 * @return  pointer to the callbacks
 */
static zclMS_AppCallbacks_t *zclMS_FindCallbacks( uint8_t endpoint )
{
  zclMSCBRec_t *pCBs;

  pCBs = zclMSCBs;
  while ( pCBs )
  {
    if ( pCBs->endpoint == endpoint )
      return ( pCBs->CBs );
    pCBs = pCBs->next;
  }
  return ( (zclMS_AppCallbacks_t *)NULL );
}

/*********************************************************************
 * @fn      zclMS_HdlIncoming
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library or Profile commands for attributes
 *          that aren't in the attribute list
 *
 * @param   pInMsg - pointer to the incoming message
 * @param   logicalClusterID
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_HdlIncoming( zclIncoming_t *pInMsg )
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
      stat = zclMS_HdlInSpecificCommands( pInMsg );
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

/*********************************************************************
 * @fn      zclMS_HdlInSpecificCommands
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_HdlInSpecificCommands( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  zclMS_AppCallbacks_t *pCBs;

  // make sure endpoint exists
  pCBs = (void*)zclMS_FindCallbacks( pInMsg->msg->endPoint );
  if ( pCBs == NULL )
    return ( ZFailure );

  switch ( pInMsg->msg->clusterId )
  {
    case ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT:
      stat = zclMS_ProcessIn_IlluminanceMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG:
      stat = zclMS_ProcessIn_IlluminanceLevelSensingCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_TEMPERATURE_MEASUREMENT:
      stat = zclMS_ProcessIn_TemperatureMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_PRESSURE_MEASUREMENT:
      stat = zclMS_ProcessIn_PressureMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_FLOW_MEASUREMENT:
      stat = zclMS_ProcessIn_FlowMeasurementCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_RELATIVE_HUMIDITY:
      stat = zclMS_ProcessIn_RelativeHumidityCmds( pInMsg );
      break;

    case ZCL_CLUSTER_ID_MS_OCCUPANCY_SENSING:
      stat = zclMS_ProcessIn_OccupancySensingCmds( pInMsg );
      break;

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_IlluminanceMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_IlluminanceMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZFailure;

  // there are no specific command for this cluster yet.
  // instead of suppressing a compiler warnings( for a code porting reasons )
  // fake unused call here and keep the code skeleton intact
 (void)pInMsg;
  if ( stat != ZFailure )
    zclMS_FindCallbacks( 0 );

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_IlluminanceLevelSensingCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_IlluminanceLevelSensingCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_TemperatureMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_TemperatureMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_PressureMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_PressureMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )
  {
    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_FlowMeasurementCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_FlowMeasurementCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_RelativeHumidityCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_RelativeHumidityCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/*********************************************************************
 * @fn      zclMS_ProcessIn_OccupancySensingCmds
 *
 * @brief   Callback from ZCL to process incoming Commands specific
 *          to this cluster library on a command ID basis
 *
 * @param   pInMsg - pointer to the incoming message
 *
 * @return  ZStatus_t
 */
static ZStatus_t zclMS_ProcessIn_OccupancySensingCmds( zclIncoming_t *pInMsg )
{
  ZStatus_t stat = ZSuccess;
  uint8_t cmdID;

  cmdID = pInMsg->hdr.commandID;

  switch ( cmdID )
  {

    default:
      stat = ZFailure;
      break;
  }

  return ( stat );
}

/****************************************************************************
****************************************************************************/

#endif // ZCL_MS

