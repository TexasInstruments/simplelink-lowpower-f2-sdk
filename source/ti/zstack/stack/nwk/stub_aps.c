/**************************************************************************************************
  Filename:       stub_aps.c
  Revised:        $Date: 2014-03-26 10:01:05 -0700 (Wed, 26 Mar 2014) $
  Revision:       $Revision: 37899 $

  Description:    Stub APS processing functions


  Copyright 2008 - 2014 Texas Instruments Incorporated.

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
#include "rom_jt_154.h"
#include "mac_spec.h"
#include "nwk_util.h"
#include "af.h"

#include "stub_aps.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// Stub NWK header length
#define STUB_NWK_HDR_LEN                2

// Start of the Stub APS header in the Inter-PAN frame
#define STUB_APS_HDR_FRAME_CTRL         STUB_NWK_HDR_LEN

// Stub APS event identifiers
#define CHANNEL_CHANGE_EVT              0x0001

#define CHANNEL_CHANGE_RETRY_TIMEOUT    100

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  zAddrType_t addr;
  uint16_t panId;
} pan_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint8_t StubAPS_TaskID = 0xFF;    // Task ID for internal task/event processing

/*********************************************************************
 * EXTERNAL VARIABLES
 */


/*********************************************************************
 * EXTERNAL FUNCTIONS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

static uint8_t newChannel;
static uint8_t channelChangeInProgress = FALSE;

// Application info
static uint8_t appTaskID = 0xFF;  // Application task id
uint8_t appEndPoint = 0;   // Application endpoint


/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void StubNWK_ParseMsg( uint8_t *buf, uint8_t bufLength, NLDE_FrameFormat_t *snff );
static void StubAPS_ParseMsg( NLDE_FrameFormat_t *snff, aps_FrameFormat_t *saff );
static void StubNWK_BuildMsg( uint8_t *nwkHdr );
static void StubAPS_BuildMsg( uint8_t *apsHdr, uint8_t frmCtrl, uint16_t groupID, APSDE_DataReq_t *req );
static ZStatus_t StubAPS_BuildFrameControl( uint8_t *frmCtrl, zAddrType_t *dstAddr,
                                            uint16_t *groupID, APSDE_DataReq_t *req );
static ZStatus_t StubAPS_SetNewChannel( uint8_t channel );
static void StubAPS_NotifyApp( uint8_t status );

uint8_t StubAPS_ZMacCallback( uint8_t *msgPtr );

/*********************************************************************
 * @fn      StubAPS_Init()
 *
 * @brief   Initialize stub APS layer
 *
 * @param   task_id - Task identifier for the desired task
 *
 * @return  none
 */
void StubAPS_Init( uint8_t task_id )
{
  StubAPS_TaskID = task_id;

  // register with ZMAC
  pZMac_AppCallback = StubAPS_ZMacCallback;

} /* StubAPS_Init() */

/*********************************************************************
 * @fn      StubAPS_ProcessEvent()
 *
 * @brief   Main event loop for Stub APS task. This function should be called
 *          at periodic intervals when event occur.
 *
 * @param   task_id - Task ID
 * @param   events  - Bitmap of events
 *
 * @return  none
 */
uint32_t StubAPS_ProcessEvent( uint8_t task_id, uint32_t events )
{
  (void)task_id; // Intentionally unreferenced parameter

  if ( events & SYS_EVENT_MSG )
  {
    OsalPort_EventHdr *msg_ptr;

    while ( (msg_ptr = (OsalPort_EventHdr *)OsalPort_msgReceive( StubAPS_TaskID )) != NULL )
    {
      if ( msg_ptr->event == MAC_MCPS_DATA_CNF )
      {
        INTERP_DataConfirm( (ZMacDataCnf_t *)msg_ptr );
      }
      else if ( msg_ptr->event == MAC_MCPS_DATA_IND )
      {
        INTERP_DataIndication( (macMcpsDataInd_t *)msg_ptr );
      }

      OsalPort_msgDeallocate( (uint8_t *)msg_ptr );
    }

    // Return unproccessed events
    return ( events ^ SYS_EVENT_MSG );
  }

  if ( events & CHANNEL_CHANGE_EVT )
  {
    // try to change to the new channel
    ZStatus_t status = StubAPS_SetNewChannel( newChannel );
    if ( status != ZSuccess )
    {
      // turn MAC receiver back on
      uint8_t rxOnIdle = true;
      ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

      // set NWK task to run
      nwk_setStateIdle( FALSE );

      channelChangeInProgress = FALSE;
    }

    // notify the application
    StubAPS_NotifyApp( status );

    return ( events ^ CHANNEL_CHANGE_EVT );
  }

  // If reach here, the events are unknown
  // Discard or make more handlers
  return 0;

} /* StubAPS_ProcessEvent() */


/*********************************************************************
 * @fn          StubNWK_ParseMsg
 *
 * @brief       Call this function to parse an incoming Stub NWK frame.
 *
 * @param       buf - pointer incoming message buffer
 * @param       bufLength - length of incoming message
 * @param       snff  - pointer Frame Format Parameters
 *
 * @return      pointer to network packet, NULL if error
 */
static void StubNWK_ParseMsg( uint8_t *buf, uint8_t bufLength, NLDE_FrameFormat_t *snff )
{
  uint16_t fc;

  memset( snff, 0, sizeof(NLDE_FrameFormat_t) );

  snff->bufLength = bufLength;

  // get the frame control
  fc = BUILD_UINT16( buf[NWK_HDR_FRAME_CTRL_LSB], buf[NWK_HDR_FRAME_CTRL_MSB] );

  // parse the frame control
  NLDE_ParseFrameControl( fc, snff );

  snff->hdrLen = STUB_NWK_HDR_LEN;

  // Stub NWK payload
  snff->nsdu = buf + snff->hdrLen;
  snff->nsduLength = snff->bufLength - snff->hdrLen;

} /* StubNWK_ParseMsg */

/*********************************************************************
 * @fn          StubAPS_ParseMsg
 *
 * @brief       Call this function to parse an incoming Stub APS frame.
 *
 * @param       naff  - pointer Stub NWK Frame Format Parameters
 * @param       saff  - pointer Stub APS Format Parameters
 *
 * @return      none
 */
static void StubAPS_ParseMsg( NLDE_FrameFormat_t *snff, aps_FrameFormat_t *saff )
{
  uint8_t fcb;
  uint8_t *asdu;

  memset( saff, 0, sizeof(aps_FrameFormat_t) );

  saff->asduLength = snff->nsduLength;
  asdu = snff->nsdu;
  saff->macDestAddr = snff->macDstAddr;

  // First byte is Frame Control.
  saff->FrmCtrl = *asdu++;

  fcb = saff->FrmCtrl & APS_FRAME_TYPE_MASK;
  if ( fcb == STUB_APS_FRAME )
  {
    fcb = saff->FrmCtrl & APS_DELIVERYMODE_MASK;
    if ( fcb == APS_FC_DM_BROADCAST )
      saff->wasBroadcast = true;
    else
      saff->wasBroadcast = false;

    if ( fcb == APS_FC_DM_GROUP )
    {
      saff->GroupID = BUILD_UINT16( asdu[0], asdu[1] );
      asdu += sizeof( uint16_t );
    }

    // Pull out the Cluster ID
    saff->ClusterID = BUILD_UINT16( asdu[0], asdu[1] );
    asdu += sizeof( uint16_t );

    // Pull out the profile ID
    saff->ProfileID = BUILD_UINT16( asdu[0], asdu[1] );
    asdu += 2;
  }

  saff->asdu = asdu;
  saff->asduLength -= (uint8_t) (asdu - snff->nsdu);
  saff->apsHdrLen = snff->nsduLength - saff->asduLength;

} /* StubAPS_ParseMsg */

/******************************************************************************
 * @fn          StubAPS_BuildFrameControl
 *
 * @brief       This function builds Stub APS Frame Control and the destination
 *              address parameter for the MCPS-DATA Request.
 *
 * @param       frmCtrl - frame control
 * @param       dstAddr - destination address for MCPS-DATA Request
 * @param       groupID - group id
 * @param       req - APSDE_DataReq_t
 *
 * @return      ZStatus_t
 */
static ZStatus_t StubAPS_BuildFrameControl( uint8_t *frmCtrl, zAddrType_t *dstAddr,
                                            uint16_t *groupID, APSDE_DataReq_t *req )
{
  // Security
  if ( req->txOptions & APS_TX_OPTIONS_SECURITY_ENABLE )
    return ( ZApsNotSupported );

  // Ack request
  if ( req->txOptions & APS_TX_OPTIONS_ACK )
    return ( ZApsNotSupported );

   // Fragmentation
  if ( req->txOptions & APS_TX_OPTIONS_PERMIT_FRAGMENT )
    return ( ZApsNotSupported );

  // set delivery mode
  if ( req->dstAddr.addrMode == AddrNotPresent )
    return ( ZApsNotSupported ); // No REFLECTOR

  // set frame type
  *frmCtrl = STUB_APS_FRAME;

  // set DstAddrMode of MCPS-DATA Request to DstAddrMode of INTERP-Data Request
  dstAddr->addrMode = req->dstAddr.addrMode;

  // set DstAddr of MCPS-DATA Request to DstAddr of INTERP-Data Request
  if ( req->dstAddr.addrMode == AddrBroadcast )
  {
    *frmCtrl |= APS_FC_DM_BROADCAST;

    // set DstAddrMode of MCPS-DATA Request to short address
    dstAddr->addrMode = Addr16Bit;
    dstAddr->addr.shortAddr = req->dstAddr.addr.shortAddr;
  }
  else if ( req->dstAddr.addrMode == Addr16Bit )
  {
    *frmCtrl |= APS_FC_DM_UNICAST;
    dstAddr->addr.shortAddr = req->dstAddr.addr.shortAddr;
  }
  else if ( req->dstAddr.addrMode == Addr64Bit )
  {
    *frmCtrl |= APS_FC_DM_UNICAST;
    osal_cpyExtAddr( dstAddr->addr.extAddr, req->dstAddr.addr.extAddr );
  }
  else if ( req->dstAddr.addrMode == AddrGroup )
  {
    *frmCtrl |= APS_FC_DM_GROUP;

    // set DstAddrMode of MCPS-DATA Request to short address
    dstAddr->addrMode = Addr16Bit;

    // set DstAddr of MCPS-DATA Request to 0xFFFF
    dstAddr->addr.shortAddr = NWK_BROADCAST_SHORTADDR_DEVALL;

    // set Group ID to DstAddr of INTERP-Data Request
    *groupID = req->dstAddr.addr.shortAddr;
  }

  return ( ZSuccess );

} /* StubAPS_BuildFrameControl */

/******************************************************************************
 * @fn          StubNWK_BuildMsg
 *
 * @brief       This function builds a Stub NWK frame.
 *
 * @param       nwkHdr - stub NWK header
 *
 * @return      none
 */
static void StubNWK_BuildMsg( uint8_t *nwkHdr )
{
  uint16_t frmCtrl = 0;
  uint8_t  protoVer = NLME_GetProtocolVersion();

  // frame type
  frmCtrl |= (STUB_NWK_FRAME_TYPE << NWK_FC_FRAME_TYPE);

  // protocol version
  frmCtrl |= (protoVer << NWK_FC_PROT_VERSION);

  // set Stub NWK header
  *nwkHdr++ = LO_UINT16( frmCtrl );
  *nwkHdr++ = HI_UINT16( frmCtrl );

} /* StubNWK_BuildMsg */

/******************************************************************************
 * @fn          StubAPS_BuildMsg
 *
 * @brief       This function builds a Stub APS frame.
 *
 * @param       apsHdr - stub APS header
 * @param       frmCtrl - stub APS frame control
 * @param       groupID - group id
 * @param       req - APSDE_DataReq_t
 *
 * @return      none
 */
static void StubAPS_BuildMsg( uint8_t *apsHdr, uint8_t frmCtrl, uint16_t groupID, APSDE_DataReq_t *req )
{
  // add frame type
  *apsHdr++ = frmCtrl;

  // add Group ID
  if ( ( frmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_GROUP )
  {
    *apsHdr++ = LO_UINT16( groupID );
    *apsHdr++ = HI_UINT16( groupID );
  }

  // add clusterID
  *apsHdr++ = LO_UINT16( req->clusterID );
  *apsHdr++ = HI_UINT16( req->clusterID );

  // add profile ID
  *apsHdr++ = LO_UINT16( req->profileID );
  *apsHdr++ = HI_UINT16( req->profileID );

  // copy ASDU data into frame
  OsalPort_memcpy ( apsHdr, req->asdu, req->asduLen );

} /* StubAPS_BuildMsg */

/******************************************************************************
 * @fn          StubAPS_setNewChannel
 *
 * @brief       This function changes the device's channel.
 *
 * @param       none
 *
 * @return      ZStatus_t
 */
static ZStatus_t StubAPS_SetNewChannel( uint8_t channel )
{
  uint8_t rxOnIdle;

  // make sure MAC has nothing to transmit
  if ( ( nwkDB_CountTypes( NWK_DATABUF_SENT ) == 0 ) && ZMacStateIdle() )
  {
    // set the new channel
    ZMacSetReq( ZMacChannel, &channel );

    // turn MAC receiver back on
    rxOnIdle = true;
    ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

    channelChangeInProgress = FALSE;
    nwk_setStateIdle( FALSE );

    return ( ZSuccess );
  }

  return ( ZFailure );

} /* StubAPS_setNewChannel */


/******************************************************************************
 * @fn          StubAPS_NotifyApp
 *
 * @brief       This function sends an OSAL message to the Application task.
 *
 * @param       status - command status
 *
 * @return      none
 */
static void StubAPS_NotifyApp( uint8_t status )
{
  OsalPort_EventHdr *msgPtr;

  // Notify the application task
  msgPtr = (OsalPort_EventHdr *)OsalPort_msgAllocate( sizeof(OsalPort_EventHdr) );
  if ( msgPtr )
  {
    msgPtr->event = SAPS_CHANNEL_CHANGE;
    msgPtr->status = status;

    OsalPort_msgSend( appTaskID, (uint8_t *)msgPtr );
  }

} /* StubAPS_NotifyApp */

/******************************************************************************
 *
 *  External APIs provided to the Application.
 */

/******************************************************************************
 * @fn          StubAPS_SetInterPanChannel
 *
 * @brief       This function changes the device's channel for inter-PAN communication.
 *
 * @param       channel - new channel
 *
 * @return      ZStatus_t
 */
ZStatus_t StubAPS_SetInterPanChannel( uint8_t channel )
{
  uint8_t currChannel;
  uint8_t rxOnIdle;

  if ( channelChangeInProgress )
    return ( ZFailure );

  ZMacGetReq( ZMacChannel, &currChannel );
  if ( currChannel == channel )
  {
    // inter PANs communication within the same channel
    return ( ZSuccess );
  }

  // go into channel transition state
  channelChangeInProgress = TRUE;

  // set NWK task to idle
  nwk_setStateIdle( TRUE );

  // turn MAC receiver off
  rxOnIdle = false;
  ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

  // try to change to the new channel
  if ( StubAPS_SetNewChannel( channel ) == ZSuccess )
    return ( ZSuccess );

  // save the new channel for retry
  newChannel = channel;

  // ask StubAPS task to retry it later
  OsalPortTimers_startTimer( StubAPS_TaskID, CHANNEL_CHANGE_EVT, CHANNEL_CHANGE_RETRY_TIMEOUT );

  return ( ZApsNotAllowed );

} /* StubAPS_SetInterPanChannel */

/******************************************************************************
 * @fn          StubAPS_SetIntraPanChannel
 *
 * @brief       This function sets the device's channel back to the NIB channel.
 *
 * @param       none
 *
 * @return      ZStatus_t
 */
ZStatus_t StubAPS_SetIntraPanChannel( void )
{
  uint8_t currChannel;
  uint8_t rxOnIdle;

  if ( channelChangeInProgress )
    return ( ZFailure );

  ZMacGetReq( ZMacChannel, &currChannel );
  if ( currChannel == _NIB.nwkLogicalChannel )
    return ( ZSuccess );

  channelChangeInProgress = TRUE;

  // turn MAC receiver off
  rxOnIdle = false;
  ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

  // Valid channels for 2.4Ghz
  if ((_NIB.nwkLogicalChannel >= 11) && (_NIB.nwkLogicalChannel <= 26))
  {
    // set the PIB channel
    ZMacSetReq( ZMacChannel, &(_NIB.nwkLogicalChannel) );
  }

  // turn MAC receiver back on
  rxOnIdle = true;
  ZMacSetReq( ZMacRxOnIdle, &rxOnIdle );

  // set NWK task to run
  nwk_setStateIdle( FALSE );

  channelChangeInProgress = FALSE;

  return ( ZSuccess );

} /* StubAPS_SetIntraPanChannel */

/******************************************************************************
 * @fn          StubAPS_InterPan
 *
 * @brief       This function checks to see if a PAN is an Inter-PAN.
 *
 * @param       panId - PAN ID
 * @param       endPoint - endpoint
 *
 * @return      TRUE if PAN is Inter-PAN, FALSE otherwise
 */
uint8_t StubAPS_InterPan( uint16_t panId, uint8_t endPoint )
{
  (void)panId; // Intentionally unreferenced parameter

  // No need to check the MAC/NIB Channels or Source/Destination PAN IDs
  // since it's possible to send Inter-PAN messages within the same network.
  if ( endPoint == STUBAPS_INTER_PAN_EP )
  {
    // Inter-PAN endpoint
    return ( TRUE );
  }

  return ( FALSE );

} /* StubAPS_InterPan */

/******************************************************************************
 * @fn          StubAPS_RegisterApp
 *
 * @brief       This function registers the Application with the Stub APS layer.
 *
 *              NOTE: Since Stub APS messages don't include the application
 *                    endpoint, the application has to register its endpoint
 *                    with Stub APS.
 *
 * @param       epDesc - application's endpoint descriptor
 *
 * @return      none
 */
void StubAPS_RegisterApp( endPointDesc_t *epDesc )
{
  appTaskID = *epDesc->task_id;
  appEndPoint = epDesc->endPoint;

} /* StubAPS_RegisterApp */

/******************************************************************************
 * @fn          StubAPS_ZMacCallback
 *
 * @brief       This function accepts an inter-PAN message from ZMac.
 *
 * @param       msgPtr - received message
 *
 * @return      TRUE if message is processed. FALSE otherwise.
 */
uint8_t StubAPS_ZMacCallback( uint8_t *msgPtr )
{
  uint16_t nwk_fc;
  uint8_t  aps_fc;
  uint8_t  frameType;
  uint8_t *buf = NULL;
  uint8_t  event = ((OsalPort_EventHdr *)msgPtr)->event;

  if ( event == MAC_MCPS_DATA_IND )
  {
    buf = ((macMcpsDataInd_t *)msgPtr)->msdu.p;
  }
  else if ( event == MAC_MCPS_DATA_CNF )
  {
    // Adjust the msdu buffer in dataCnf from MAC header to MAC payload
    buf = (uint8_t *)((macMcpsDataCnf_t *)msgPtr)->pDataReq + sizeof(macMcpsDataReq_t) + MAC_DATA_OFFSET;
  }

  if ( buf )
  {
    // get the NWK frame control
    nwk_fc = BUILD_UINT16( buf[NWK_HDR_FRAME_CTRL_LSB], buf[NWK_HDR_FRAME_CTRL_MSB] );

    // frame type
    frameType = (uint8_t)((nwk_fc >> NWK_FC_FRAME_TYPE) & NWK_FC_FRAME_TYPE_MASK);

    // check if incoming frame is of the right type
    if ( frameType != STUB_NWK_FRAME_TYPE )
    {
      // message doesn't belong to Stub APS
      return ( FALSE );
    }

    // get the APS frame control
    aps_fc = buf[STUB_APS_HDR_FRAME_CTRL];

    // frame type
    frameType = aps_fc & APS_FRAME_TYPE_MASK;

    // check if incoming frame is of the right type
    if ( frameType != STUB_APS_FRAME )
    {
      // message doesn't belong to Stub APS
      return ( FALSE );
    }

    // message belongs to Stub APS
    OsalPort_msgSend( StubAPS_TaskID, (uint8_t *)msgPtr );

    return ( TRUE );
  }

  // message doesn't belong to Stub APS
  return ( FALSE );

} /* StubAPS_ZMacCallback */

/******************************************************************************
 *
 *  Stub APS Inter-PAN interface INTERP and its callbacks.
 */

/******************************************************************************
 * @fn          INTERP_DataReq
 *
 * @brief       This function requests the transfer of data from the next
 *              higher layer to a single peer entity.
 *
 * @param       req - APSDE_DataReq_t
 *
 * @return      ZStatus_t
 */
ZStatus_t INTERP_DataReq( APSDE_DataReq_t *req )
{
  uint8_t apsFrmCtrl;
  uint16_t groupID = 0;
  uint8_t *buf;
  uint8_t hdrLen;
  ZMacDataReq_t dataReq;
  ZStatus_t status;

  if ( channelChangeInProgress || !StubAPS_InterPan( req->dstPanId, req->dstEP ) )
    return ( ZFailure );

  memset( &dataReq, 0, sizeof( ZMacDataReq_t ) );

  // Build Stub APS header
  status = StubAPS_BuildFrameControl( &apsFrmCtrl, &(dataReq.DstAddr), &groupID, req );
  if ( status != ZSuccess )
    return ( status );

  // set default Stub APS header length
  hdrLen = APS_FRAME_CTRL_FIELD_LEN;

  // add group ID length
  if ( ( apsFrmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_GROUP )
    hdrLen += APS_GROUP_ID_FIELD_LEN;

  // add cluster ID length
  hdrLen += APS_CLUSTERID_FIELD_LEN;

  // add profile ID length
  hdrLen += APS_PROFILEID_FIELD_LEN;

  // add default Stub NWK header length
  hdrLen += STUB_NWK_HDR_LEN;

  // calculate MSDU length
  dataReq.msduLength = hdrLen + req->asduLen;

  // allocate buffer
  buf = OsalPort_malloc( dataReq.msduLength );
  if ( buf != NULL )
  {
    dataReq.msdu = buf;

    // Add Stub APS header and data
    StubAPS_BuildMsg( &buf[STUB_APS_HDR_FRAME_CTRL], apsFrmCtrl, groupID, req );

    // Add Stub NWK header
    StubNWK_BuildMsg( buf );

    // Set ZMac data request
    dataReq.DstPANId = req->dstPanId;
    dataReq.SrcAddrMode = Addr64Bit;
    dataReq.Handle = req->transID;

    if ( ( apsFrmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_UNICAST )
      dataReq.TxOptions = NWK_TXOPTIONS_ACK;
    else
      dataReq.TxOptions = 0;

    // send the frame
    status = ZMacDataReq( &dataReq );

    // free the frame
    OsalPort_free( buf );
  }
  else
  {
    // flag a memory error
    status = ZMemError;
  }

  return ( status );

} /* INTERP_DataReq */

/******************************************************************************
 * @fn          INTERP_DataReqMTU
 *
 * @brief       This function requests the MTU (Max Transport Unit) of the
 *              Inter-PAN Data Service.
 *
 * @param       none
 *
 * @return      uint8_t - MTU
 */
uint8_t INTERP_DataReqMTU( void )
{
  uint8_t mtu;
  uint8_t hdrLen;

  // Use maximum header size for Stub APS header
  hdrLen = APS_FRAME_CTRL_FIELD_LEN +
           APS_GROUP_ID_FIELD_LEN   +
           APS_CLUSTERID_FIELD_LEN  +
           APS_PROFILEID_FIELD_LEN;

  mtu = (macCfg.macMaxFrameSize - MAC_A_MAX_FRAME_OVERHEAD) -
        STUB_NWK_HDR_LEN - hdrLen;

  return ( mtu );

} /* INTERP_DataReqMTU */

/****************************************************************************
 * @fn          INTERP_DataConfirm
 *
 * @brief       This function processes the data confirm from the MAC layer.
 *
 * @param       dataCnf - data confirm primitive
 *
 * @return      none
 */
void INTERP_DataConfirm( ZMacDataCnf_t *dataCnf )
{
  afDataConfirm( appEndPoint, dataCnf->msduHandle, dataCnf->msduHandle, dataCnf->hdr.Status );

} /* INTERP_DataConfirm */

/****************************************************************************
 * @fn          INTERP_DataIndication
 *
 * @brief       This function indicates the transfer of a data SPDU (MSDU)
 *              from the MAC layer to the local application layer entity.
 *
 * @param       dataInd - data indicate primitive
 *
 * @return      none
 */
void INTERP_DataIndication( macMcpsDataInd_t *dataInd )
{
  NLDE_FrameFormat_t snff;
  aps_FrameFormat_t saff;
  zAddrType_t srcAddr;
  NLDE_Signal_t sig;

  // parse the Stub NWK header
  StubNWK_ParseMsg( dataInd->msdu.p, dataInd->msdu.len, &snff );

  // Fill in MAC destination address
  snff.macDstAddr = dataInd->mac.dstAddr.addr.shortAddr;

  // fill in MAC source address (Stub NWK frame doesn't have address fields)
  osal_copyAddress( &srcAddr, (zAddrType_t *)&(dataInd->mac.srcAddr) );

  // check if incoming frame is of the right type
  if ( snff.frameType != STUB_NWK_FRAME_TYPE )
    return;

  // check if incoming frame is of the right version
  if ( snff.protocolVersion != NLME_GetProtocolVersion() )
    return;

  // check if the remaining sun-fields are zero
  if ( ( snff.discoverRoute != 0 ) || ( snff.multicast != 0 )   ||
       ( snff.secure != 0 )        || ( snff.srcRouteSet != 0 ) ||
       ( snff.dstExtAddrSet != 0 ) || ( snff.srcExtAddrSet != 0 ) )
  {
    return;
  }

  // parse the Stub APS header
  StubAPS_ParseMsg( &snff, &saff );

  // check if incoming frame is of the right type
  if ( ( saff.FrmCtrl & APS_FRAME_TYPE_MASK ) != STUB_APS_FRAME )
    return;

  // check if delivery mode is of the right type
  if ( ( saff.FrmCtrl & APS_DELIVERYMODE_MASK ) == APS_FC_DM_INDIRECT )
    return;

  // check if incoming frame is unsecured
  if ( saff.FrmCtrl & APS_FC_SECURITY )
    return;

  // check if there's no extended header
  if ( saff.FrmCtrl & APS_FC_EXTENDED )
      return;

  // Set the endpoints
  saff.DstEndPoint = appEndPoint;
  saff.SrcEndPoint = STUBAPS_INTER_PAN_EP;

  // Set the signal strength information
  sig.LinkQuality = dataInd->mac.mpduLinkQuality;
  sig.correlation = dataInd->mac.correlation;
  sig.rssi = dataInd->mac.rssi;

  APSDE_DataIndication( &saff, &srcAddr, dataInd->mac.srcPanId,
                        &sig, snff.broadcastId, FALSE, dataInd->mac.timestamp, 0 );

} /* INTERP_DataIndication */


/*********************************************************************
*********************************************************************/
