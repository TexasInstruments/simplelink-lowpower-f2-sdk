/**************************************************************************************************
  Filename:       bdb_touchlink.c
  Revised:        $Date: 2013-12-06 15:53:38 -0800 (Fri, 06 Dec 2013) $
  Revision:       $Revision: 36460 $

  Description:    Zigbee Cluster Library - Light Link Profile.


  Copyright 2011-2013 Texas Instruments Incorporated.

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
#include "ti_zstack_config.h"
#include "osal_nv.h"
#include "ssp_hash.h"
#include "nwk_util.h"
#include "zd_sec_mgr.h"
#include "zd_object.h"

#include "stub_aps.h"
#if defined ( BDB_TL_INITIATOR )
  #include "bdb_touchlink_initiator.h"
#endif // BDB_TL_INITIATOR
#if defined ( BDB_TL_TARGET )
  #include "bdb_touchlink_target.h"
#endif // BDB_TL_TARGET


#include "bdb.h"
#include "bdb_tl_commissioning.h"
#include "bdb_touchlink.h"

#if defined ( BDB_TL_TARGET ) || defined ( BDB_TL_INITIATOR )

/*********************************************************************
 * MACROS
 */
#define TOUCHLINK_NEW_MIN( min, max )                  ( ( (uint32_t)(max) + (uint32_t)(min) + 1 ) / 2 )

/*********************************************************************
 * CONSTANTS
 */

#define TOUCHLINK_NUM_DEVICE_INFO_ENTRIES              5


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

uint32_t touchLinkLastAcceptedTransID;

// Used for Network Discovery
touchLinkDiscoveredNwkParam_t discoveredTouchlinkNwk;

// Current Touch Link Transaction ID
uint32_t touchLinkTransID;

// Scan Response ID
uint32_t touchLinkResponseID;

// Our group ID range
uint16_t touchLinkGrpIDsBegin;
uint16_t touchLinkGrpIDsEnd;

// Flag for leave
uint8_t touchLinkLeaveInitiated;

// Device Information Table
bdbTLDeviceInfo_t *touchLinkSubDevicesTbl[5];

bool touchlinkFNReset;

// Touchlink distributed network flag
bool touchlinkDistNwk = FALSE;

uint16_t touchlinkPanId = INVALID_NODE_ADDR;

/*********************************************************************
 * EXTERNAL VARIABLES
 */
uint8_t touchLinkTaskId;

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
// TOUCHLINK Profile attributes - Our free network address and group ID ranges
static uint16_t touchLinkFreeNwkAddrBegin;
static uint16_t touchLinkFreeNwkAddrEnd;
static uint16_t touchLinkFreeGrpIdBegin;
static uint16_t touchLinkFreeGrpIdEnd;

static bool touchLinkIsInitiator;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void touchLink_BuildAesKey( uint8_t *pAesKey, uint32_t transID, uint32_t responseID, uint8_t keyIndex );
void touchLink_ItemInit( uint16_t id, uint16_t len, void *pBuf );
static void *touchLink_BeaconIndCB ( void *param );
static void *touchLink_NwkDiscoveryCnfCB ( void *param );
static void touchLink_InitNV( void );

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      touchLink_InitVariables
 *
 * @brief   Initialize the TOUCHLINK global and local variables.
 *
 * @param   initiator - if caller is Initiator
 *
 * @return  none
 */
void touchLink_InitVariables( bool initiator )
{
  touchLinkTransID = 0;
  touchLinkIsInitiator = initiator;

  if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    _NIB.nwkDevAddress = INVALID_NODE_ADDR;
  }

  // verify groups communication is initiated by broadcasts rather than multicasts
  _NIB.nwkUseMultiCast = FALSE;
  // detect and remove stored deprecated end device children after power up
  zgRouterOffAssocCleanup = TRUE;
  osal_nv_write(ZCD_NV_ROUTER_OFF_ASSOC_CLEANUP, sizeof(zgRouterOffAssocCleanup), &zgRouterOffAssocCleanup);

  touchLink_InitFreeRanges( initiator );

  touchLink_InitNV();

  touchLinkLeaveInitiated = FALSE;

  // Initialize device info table
  memset( touchLinkSubDevicesTbl, 0, sizeof( touchLinkSubDevicesTbl ) );

  touchLinkIsInitiator = FALSE;

  // set broadcast address mask to support broadcast filtering
  NLME_SetBroadcastFilter( ZDO_Config_Node_Descriptor.CapabilityFlags );
}

/*********************************************************************
 * @fn      touchLink_InitFreeRanges
 *
 * @brief   Initialize the TOUCHLINK free range global variables.
 *
 * @param   initiator - if caller is link initiator
 *
 * @return  none
 */
void touchLink_InitFreeRanges( bool initiator )
{
  // Initialize our free network address and group ID ranges
  if ( initiator )
  {
    touchLinkFreeNwkAddrBegin = APL_FREE_NWK_ADDR_RANGE_BEGIN;
    touchLinkFreeNwkAddrEnd = APL_FREE_NWK_ADDR_RANGE_END;

    touchLinkFreeGrpIdBegin = APL_FREE_GROUP_ID_RANGE_BEGIN;
    touchLinkFreeGrpIdEnd = APL_FREE_GROUP_ID_RANGE_END;
  }
  else
  {
    touchLinkFreeNwkAddrBegin = touchLinkFreeNwkAddrEnd = 0;
    touchLinkFreeGrpIdBegin = touchLinkFreeGrpIdEnd = 0;
  }

  // Initialize our local group ID range
  touchLinkGrpIDsBegin = touchLinkGrpIDsEnd = 0;
}

/*********************************************************************
 * @fn      touchLink_UpdateFreeRanges
 *
 * @brief   Update the TOUCHLINK free range global variables.
 *
 * @param   pParams - new parameters
 *
 * @return  none
 */
void touchLink_UpdateFreeRanges( bdbTLNwkParams_t *pParams )
{
  // Set our free network address and group ID ranges
  touchLinkFreeNwkAddrBegin = pParams->freeNwkAddrBegin;
  touchLinkFreeNwkAddrEnd = pParams->freeNwkAddrEnd;
  touchLinkFreeGrpIdBegin = pParams->freeGrpIDBegin;
  touchLinkFreeGrpIdEnd = pParams->freeGrpIDEnd;

  // Set our group ID range
  touchLinkGrpIDsBegin = pParams->grpIDsBegin;
  touchLinkGrpIDsEnd = pParams->grpIDsEnd;
}

/*********************************************************************
 * @fn      touchLink_GerFreeRanges
 *
 * @brief   Get the TOUCHLINK free range global variables.
 *
 * @param   pParams - parameter to get
 *
 * @return  none
 */
void touchLink_GerFreeRanges( bdbTLNwkParams_t *pParams )
{
  // Set our free network address and group ID ranges
  pParams->freeNwkAddrBegin = touchLinkFreeNwkAddrBegin;
  pParams->freeNwkAddrEnd = touchLinkFreeNwkAddrEnd;
  pParams->freeGrpIDBegin = touchLinkFreeGrpIdBegin;
  pParams->freeGrpIDEnd = touchLinkFreeGrpIdEnd;

    // Set our group ID range
  pParams->grpIDsBegin = touchLinkGrpIDsBegin;
  pParams->grpIDsEnd = touchLinkGrpIDsEnd;
}

/*********************************************************************
 * @fn      touchLink_IsValidSplitFreeRanges
 *
 * @brief   Checks to see if the resulting two ranges are not smaller
 *          than the threshold after division of a network address or
 *          group ID range. The Initiator splits its own free range
 *          in half and assigns the top half to the new device.
 *
 *          Note: A range (Nmin...Nmax) is split as follows:
 *
 *                N'min = (Nmax + Nmin + 1)/2
 *                N'max = Nmax
 *                Nmax = N'min - 1
 *
 * @param   totalGrpIDs - total number of group IDs needed
 *
 * @return  TRUE if split possible. FALSE, otherwise.
 */
bool touchLink_IsValidSplitFreeRanges( uint8_t totalGrpIDs )
{
  if ( ( touchLinkFreeNwkAddrBegin != 0 ) && ( touchLinkFreeGrpIdBegin != 0 ) )
  {
      return ( ( ( ( touchLinkFreeNwkAddrEnd - touchLinkFreeNwkAddrBegin ) / 2 ) >= APL_FREE_ADDR_THRESHOLD ) &&
               ( ( ( touchLinkFreeGrpIdEnd - ( touchLinkFreeGrpIdBegin + totalGrpIDs - 1 ) ) / 2 ) >= APL_FREE_GROUP_ID_THRESHOLD ) );

  }

  return ( FALSE );
}

/*********************************************************************
 * @fn      touchLink_SplitFreeRanges
 *
 * @brief   Split our own free network address and group ID ranges
 *          in half and assign the top half to the new device.
 *
 *          Note: A range (Nmin...Nmax) is split as follows:
 *
 *                N'min = (Nmax + Nmin + 1)/2
 *                N'max = Nmax
 *                Nmax = N'min - 1
 *
 * output parameters
 *
 * @param   pAddrBegin - new address range begin
 * @param   pAddrEnd - new address range end
 * @param   pGrpIdBegin - new group id range begin
 * @param   pGrpIdEnd - new group id range end
 *
 * @return  none
 */
void touchLink_SplitFreeRanges( uint16_t *pAddrBegin, uint16_t *pAddrEnd,
                        uint16_t *pGrpIdBegin, uint16_t *pGrpIdEnd )
{
  if ( ( touchLinkFreeNwkAddrBegin != 0 ) && ( touchLinkFreeGrpIdBegin != 0 ) )
  {
    *pAddrBegin = TOUCHLINK_NEW_MIN( touchLinkFreeNwkAddrBegin, touchLinkFreeNwkAddrEnd );
    *pGrpIdBegin = TOUCHLINK_NEW_MIN( touchLinkFreeGrpIdBegin, touchLinkFreeGrpIdEnd );

    *pAddrEnd = touchLinkFreeNwkAddrEnd;
    *pGrpIdEnd = touchLinkFreeGrpIdEnd;

    // Update our max
    touchLinkFreeNwkAddrEnd = *pAddrBegin - 1;
    touchLinkFreeGrpIdEnd = *pGrpIdBegin - 1;
  }
  else
  {
    *pAddrBegin = *pAddrEnd = 0;
    *pGrpIdBegin = *pGrpIdEnd = 0;
  }
}

/*********************************************************************
 * @fn      touchLink_PopGrpIDRange
 *
 * @brief   Pop the requested number of group IDs out of the free group IDs range.
 *
 * input parameters
 *
 * @param   numGrpIDs - number of group IDs needed
 *
 * output parameters
 *
 * @param   pGrpIdBegin - new group id range begin, or 0 if unavaialable
 * @param   pGrpIdEnd - new group id range end, , or 0 if unavaialable
 *
 * @return  none
 */
void touchLink_PopGrpIDRange( uint8_t numGrpIDs, uint16_t *pGrpIDsBegin, uint16_t *pGrpIDsEnd )
{
  if ( ( touchLinkFreeGrpIdBegin != 0 )
       && ( touchLinkFreeGrpIdBegin <= touchLinkFreeGrpIdEnd )
       && ( ( touchLinkFreeGrpIdEnd - touchLinkFreeGrpIdBegin ) >= numGrpIDs ) )
  {
    *pGrpIDsBegin = touchLinkFreeGrpIdBegin;

    // Update min free group id
    touchLinkFreeGrpIdBegin += numGrpIDs;

    *pGrpIDsEnd = touchLinkFreeGrpIdBegin - 1;
  }
  else
  {
    *pGrpIDsBegin = *pGrpIDsEnd = 0;
  }
}

/*********************************************************************
 * @fn      touchLink_PopNwkAddress
 *
 * @brief   Pop an avaialble short address out of the free network addresses range.
 *
 * @param   none
 *
 * @return  free address if available. 0, otherwise.
 */
uint16_t touchLink_PopNwkAddress( void )
{
  if ( ( touchLinkFreeNwkAddrBegin == 0 ) || ( touchLinkFreeNwkAddrBegin > touchLinkFreeNwkAddrEnd ) )
  {
    return ( 0 );
  }

  return ( touchLinkFreeNwkAddrBegin++ );
}

/*********************************************************************
 * @fn      touchLink_GetNumSubDevices
 *
 * @brief   Get the total number of sub-devices (endpoints) registered.
 *
 * @param   startIndex - starting index
 *
 * @return  number of sub-devices
 */
uint8_t touchLink_GetNumSubDevices( uint8_t startIndex )
{
  uint8_t numEPs = 0;

  for ( uint8_t i = startIndex; i < TOUCHLINK_NUM_DEVICE_INFO_ENTRIES; i++ )
  {
    if ( touchLinkSubDevicesTbl[i] != NULL )
    numEPs++;
  }

  return ( numEPs );
}

/*********************************************************************
 * @fn      touchLink_GetNumGrpIDs
 *
 * @brief   Get the total number of group IDs required by this device.
 *
 * @param   none
 *
 * @return  number of group IDs
 */
uint8_t touchLink_GetNumGrpIDs( void )
{
  uint8_t numGrpIDs = 0;

  for ( uint8_t i = 0; i < TOUCHLINK_NUM_DEVICE_INFO_ENTRIES; i++ )
  {
    if ( touchLinkSubDevicesTbl[i] != NULL )
    {
      numGrpIDs += touchLinkSubDevicesTbl[i]->grpIdCnt;
    }
  }

  return ( numGrpIDs );
}

/*********************************************************************
 * @fn      touchLink_GetSubDeviceInfo
 *
 * @brief   Get the sub-device information.
 *
 * input parameter
 *
 * @param   index - index of sub-device
 *
 * output parameter
 *
 * @param   pInfo - sub-device info (to be returned)
 *
 * @return  none
 */
void touchLink_GetSubDeviceInfo( uint8_t index, bdbTLDeviceInfo_t *pInfo )
{
  if ( pInfo == NULL )
  {
    return;
  }
  if ( ( index < TOUCHLINK_NUM_DEVICE_INFO_ENTRIES ) &&
       ( touchLinkSubDevicesTbl[index] != NULL ) )
  {
    endPointDesc_t *epDesc = afFindEndPointDesc( touchLinkSubDevicesTbl[index]->endpoint );
    if ( epDesc != NULL )
    {
      // Copy sub-device info
      *pInfo = *(touchLinkSubDevicesTbl[index]);
    }
  }
  else
  {
    memset( pInfo, 0, sizeof( bdbTLDeviceInfo_t ) );
  }
}


/*********************************************************************
 * @fn      touchLink_EncryptNwkKey
 *
 * @brief   Encrypt the current network key to be sent to a Target.
 *          In case of Factory New device generate new key.
 *
 * output parameter
 *
 * @param   pNwkKey - pointer to encrypted network key
 *
 * input parameters
 *
 * @param   keyIndex - key index
 * @param   transID - transaction id
 * @param   responseID - response id
 *
 * @return  none
 */
void touchLink_EncryptNwkKey( uint8_t *pNwkKey, uint8_t keyIndex, uint32_t transID, uint32_t responseID )
{
  uint8_t aesKeyKey[SEC_KEY_LEN] = TOUCHLINK_DEFAULT_AES_KEY;
  uint8_t masterKey[SEC_KEY_LEN] = TOUCHLINK_ENC_KEY;
  uint8_t nwkKey[SEC_KEY_LEN];

  if ( bdbAttributes.bdbNodeIsOnANetwork == FALSE )
  {
    touchLink_GenerateRandNum( nwkKey, SEC_KEY_LEN );
  }
  else
  {
    nwkActiveKeyItems keyItems;
    SSP_ReadNwkActiveKey( &keyItems );
    OsalPort_memcpy( nwkKey, keyItems.active.key , SEC_KEY_LEN);
  }

  // Build the AES key
  touchLink_BuildAesKey( aesKeyKey, transID, responseID, keyIndex );
  //From ccmStar
  HAL_AES_ENTER_WORKAROUND();
  if ( ( keyIndex == TOUCHLINK_KEY_INDEX_MASTER ) || ( keyIndex == TOUCHLINK_KEY_INDEX_CERT ) )
  {
    // Encypt with the master key
    sspAesEncrypt( masterKey, aesKeyKey );
  }
  // Encrypt the network key with the AES key
  sspAesEncrypt( aesKeyKey, nwkKey );

  // Copy in the encrypted network key
  OsalPort_memcpy( pNwkKey, nwkKey, SEC_KEY_LEN );
  HAL_AES_EXIT_WORKAROUND();
}

/*********************************************************************
 * @fn      touchLink_DecryptNwkKey
 *
 * @brief   Decrypt the received network key and update.
 *
 * @param   pNwkKey - pointer to the encrypted network key
 * @param   keyIndex - key index
 * @param   transID - transaction id
 * @param   responseID - response id
 *
 * @return  none
 */
void touchLink_DecryptNwkKey( uint8_t *pNwkKey, uint8_t keyIndex, uint32_t transID, uint32_t responseID )
{
  uint8_t aesKeyKey[SEC_KEY_LEN] = TOUCHLINK_DEFAULT_AES_KEY;

  uint8_t nwkKey[SEC_KEY_LEN];

  uint8_t masterKey[SEC_KEY_LEN] = TOUCHLINK_ENC_KEY;

  // Copy in the encrypted network key
  OsalPort_memcpy( nwkKey, pNwkKey, SEC_KEY_LEN );

  touchLink_BuildAesKey( aesKeyKey, transID, responseID, keyIndex );
  //From ccmStar
  HAL_AES_ENTER_WORKAROUND();
  if ( ( keyIndex == TOUCHLINK_KEY_INDEX_MASTER ) || ( keyIndex == TOUCHLINK_KEY_INDEX_CERT ) )
  {
    //encypt with the master key
    sspAesEncrypt( masterKey, aesKeyKey );
  }
  // Decrypt the network key with the AES key
  sspAesDecrypt( aesKeyKey, nwkKey );

  touchLink_UpdateNwkKey( nwkKey, keyIndex );
  HAL_AES_EXIT_WORKAROUND();
}

/*********************************************************************
 * @fn      touchLink_BuildAesKey
 *
 * @brief   Build an AES key using Transaction ID and Response ID.
 *
 * @param   pAesKey - pointer to AES to be built
 * @param   transID - transaction id
 * @param   responseID - response id
 *
 * @return  none
 */
static void touchLink_BuildAesKey( uint8_t *pAesKey, uint32_t transID, uint32_t responseID, uint8_t keyIndex )
{

  if ( ( keyIndex == TOUCHLINK_KEY_INDEX_MASTER ) || ( keyIndex == TOUCHLINK_KEY_INDEX_CERT ) )
  {
    // Copy transaction identifier to 1st byte
    pAesKey[0] = BREAK_UINT32( transID, 3 );
    pAesKey[1] = BREAK_UINT32( transID, 2 );
    pAesKey[2] = BREAK_UINT32( transID, 1 );
    pAesKey[3] = BREAK_UINT32( transID, 0 );

    // Copy response identifier 3rd bute
    pAesKey[8] = BREAK_UINT32( responseID, 3 );
    pAesKey[9] = BREAK_UINT32( responseID, 2 );
    pAesKey[10] = BREAK_UINT32( responseID, 1 );
    pAesKey[11] = BREAK_UINT32( responseID, 0 );
  }

  // Copy in the transaction identifier
  pAesKey[4] = BREAK_UINT32( transID, 3 );
  pAesKey[5] = BREAK_UINT32( transID, 2 );
  pAesKey[6] = BREAK_UINT32( transID, 1 );
  pAesKey[7] = BREAK_UINT32( transID, 0 );

  // Copy in the response identifier
  pAesKey[12] = BREAK_UINT32( responseID, 3 );
  pAesKey[13] = BREAK_UINT32( responseID, 2 );
  pAesKey[14] = BREAK_UINT32( responseID, 1 );
  pAesKey[15] = BREAK_UINT32( responseID, 0 );
}

/*********************************************************************
 * @fn      touchLink_UpdateNwkKey
 *
 * @brief   Update the network key.
 *
 * @param   pNwkParams - pointer to new network key
 * @param   keyIndex - key index
 *
 * @return  none
 */
void touchLink_UpdateNwkKey( uint8_t *pNwkKey, uint8_t keyIndex )
{
  uint32_t nwkFrameCounterTmp;
  (void)keyIndex;

  // To prevent Framecounter out of sync issues, store the lastkey
  nwkFrameCounterTmp = nwkFrameCounter;  // (Global in SSP).

  // Update the network key
  SSP_UpdateNwkKey( pNwkKey, 0 );

  SSP_SwitchNwkKey( 0 );

  nwkFrameCounter  = nwkFrameCounterTmp; // restore

  // Save off the security
  ZDApp_SaveNwkKey();
}

/*********************************************************************
 * @fn      touchLink_GetNwkKeyBitmask
 *
 * @brief   Get the supported network key bitmask.
 *
 * @param   none
 *
 * @return  network key bitmask
 */
uint16_t touchLink_GetNwkKeyBitmask( void )
{
  return ( (uint16_t)1 << TOUCHLINK_KEY_INDEX );
}

/*********************************************************************
 * @fn      touchLink_GenerateRandNum
 *
 * @brief   Fill buffer with random bytes.
 *
 * input parameter
 *
 * @param   numSize - size of buffer in bytes
 *
 * output parameter
 *
 * @param   pNum - pointer to buffer to be filled with random values
 *
 * @return  none
 */
void touchLink_GenerateRandNum( uint8_t *pNum, uint8_t numSize )
{
  if ( pNum && numSize )
  {
    uint8_t lastByte = ( numSize - 1 );
    for ( uint8_t i = 0; i < lastByte; i += 2 )
    {
      uint16_t rand = OsalPort_rand();
      pNum[i]   = LO_UINT16( rand );
      pNum[i+1] = HI_UINT16( rand );
    }

    // In case the number is odd
    if ( numSize % 2 )
    {
      pNum[lastByte] = LO_UINT16( OsalPort_rand() );
    }
  }
}

/*********************************************************************
 * @fn      touchLink_GetRandPrimaryChannel
 *
 * @brief   Get randomly chosen TOUCHLINK primary channel.
 *
 * @return  channel
 */
uint8_t touchLink_GetRandPrimaryChannel()
{
  uint8_t channel;
#if (TOUCHLINK_FIXED_CHANNEL_ENABLE == TRUE )
  channel = TOUCHLINK_FIXED_CHANNEL;
#else
  channel = OsalPort_rand() & 0x1F;
  if ( channel <= TOUCHLINK_FIRST_CHANNEL )
  {
    channel = TOUCHLINK_FIRST_CHANNEL;
  }
  else if ( channel <= TOUCHLINK_SECOND_CHANNEL )
  {
    channel = TOUCHLINK_SECOND_CHANNEL;
  }
  else if ( channel <= TOUCHLINK_THIRD_CHANNEL )
  {
    channel = TOUCHLINK_THIRD_CHANNEL;
  }
  else
  {
    channel = TOUCHLINK_FOURTH_CHANNEL;
  }
#endif
  return channel;
}

/*********************************************************************
 * @fn      touchLink_SetNIB
 *
 * @brief   Copy new Network Parameters to the NIB.
 *
 * @param   nwkState - network state
 * @param   nwkAddr - short address
 * @param   pExtendedPANID - pointer to extended PAN ID
 * @param   logicalChannel - channel
 * @param   panId - PAN identifier
 * @param   nwkUpdateId - nwtwork update identifier
 *
 * @return      void
 */
void touchLink_SetNIB( nwk_states_t nwkState, uint16_t nwkAddr, uint8_t *pExtendedPANID,
                 uint8_t logicalChannel, uint16_t panId, uint8_t nwkUpdateId )
{
  // Copy the new network parameters to NIB
  _NIB.nwkState = nwkState;
  _NIB.nwkDevAddress = nwkAddr;
  _NIB.nwkLogicalChannel = logicalChannel;
  _NIB.nwkCoordAddress = INVALID_NODE_ADDR;
  _NIB.channelList = (uint32_t)1 << logicalChannel;
  _NIB.nwkPanId = panId;
  _NIB.nodeDepth = 1;
  _NIB.MaxRouters = (uint8_t)gNWK_MAX_DEVICE_LIST;
  _NIB.MaxChildren = (uint8_t)gNWK_MAX_DEVICE_LIST;
  _NIB.allocatedRouterAddresses = 1;
  _NIB.allocatedEndDeviceAddresses = 1;

  //Update NV values
  //touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );

  if ( _NIB.nwkUpdateId != nwkUpdateId )
  {
    NLME_SetUpdateID( nwkUpdateId );
  }

  osal_cpyExtAddr( _NIB.extendedPANID, pExtendedPANID );

  // Save the NIB
  if ( ( nwkState == NWK_ROUTER ) || ( nwkState == NWK_ENDDEVICE ) )
  {
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );
  }
  // else will be updated when ED joins its parent
}

/*********************************************************************
 * @fn      touchLink_ProcessNwkUpdate
 *
 * @brief   Update our local network update id and logical channel.
 *
 * @param   nwkUpdateId - new network update id
 * @param   logicalChannel - new logical channel
 *
 * @return  void
 */
void touchLink_ProcessNwkUpdate( uint8_t nwkUpdateId, uint8_t logicalChannel )
{
  // Update the network update id
  NLME_SetUpdateID( nwkUpdateId );

  // Switch channel
  if ( _NIB.nwkLogicalChannel != logicalChannel )
  {
    _NIB.nwkLogicalChannel = logicalChannel;
    touchLink_SetChannel( logicalChannel );
  }

  // Update channel list
  _NIB.channelList = (uint32_t)1 << logicalChannel;

  // Our Channel has been changed -- notify to save info into NV
  ZDApp_NwkStateUpdateCB();
  touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_NIB );

  // Reset the total transmit count and the transmit failure counters
  _NIB.nwkTotalTransmissions = 0;
  nwkTransmissionFailures( TRUE );
}

/*********************************************************************
 * @fn      touchLink_UpdateNV
 *
 * @brief   Updates NV with NIB and free ranges items
 *
 * @param   enables - specifies what to update
 *
 * @return  none
 */
void touchLink_UpdateNV( uint8_t enables )
{
#if defined ( NV_RESTORE )

 #if defined ( NV_TURN_OFF_RADIO )
  // Turn off the radio's receiver during an NV update
  uint8_t RxOnIdle;
  uint8_t x = FALSE;
  ZMacGetReq( ZMacRxOnIdle, &RxOnIdle );
  ZMacSetReq( ZMacRxOnIdle, &x );
 #endif

  if ( enables & TOUCHLINK_UPDATE_NV_NIB )
  {
    // Update NIB in NV
    osal_nv_write( ZCD_NV_NIB, sizeof( nwkIB_t ), &_NIB );

    // Reset the NV startup option to resume from NV by clearing
    // the "New" join option.
    zgWriteStartupOptions( ZG_STARTUP_CLEAR, ZCD_STARTOPT_DEFAULT_NETWORK_STATE );
  }

  if ( enables & TOUCHLINK_UPDATE_NV_RANGES )
  {
    // Store our free network address and group ID ranges
    osal_nv_write( ZCD_NV_MIN_FREE_NWK_ADDR, sizeof( touchLinkFreeNwkAddrBegin ), &touchLinkFreeNwkAddrBegin );
    osal_nv_write( ZCD_NV_MAX_FREE_NWK_ADDR, sizeof( touchLinkFreeNwkAddrEnd ), &touchLinkFreeNwkAddrEnd );
    osal_nv_write( ZCD_NV_MIN_FREE_GRP_ID, sizeof( touchLinkFreeGrpIdBegin ), &touchLinkFreeGrpIdBegin );
    osal_nv_write( ZCD_NV_MAX_FREE_GRP_ID, sizeof( touchLinkFreeGrpIdEnd ), &touchLinkFreeGrpIdEnd );

    // Store our group ID range
    osal_nv_write( ZCD_NV_MIN_GRP_IDS, sizeof( touchLinkGrpIDsBegin ), &touchLinkGrpIDsBegin );
    osal_nv_write( ZCD_NV_MAX_GRP_IDS, sizeof( touchLinkGrpIDsEnd ), &touchLinkGrpIDsEnd );
  }

 #if defined ( NV_TURN_OFF_RADIO )
  ZMacSetReq( ZMacRxOnIdle, &RxOnIdle );
 #endif

#endif // NV_RESTORE
}

/*********************************************************************
 * @fn          touchLink_InitNV
 *
 * @brief       Initialize free range RAM variables from NV. If NV items
 *              don't exist, then the NV is initialize with what is in
 *              RAM variables.
 *
 * @param       none
 *
 * @return      none
 */
static void touchLink_InitNV( void )
{
  // Initialize our free network address and group ID ranges
  touchLink_ItemInit( ZCD_NV_MIN_FREE_NWK_ADDR, sizeof( touchLinkFreeNwkAddrBegin ), &touchLinkFreeNwkAddrBegin );
  touchLink_ItemInit( ZCD_NV_MAX_FREE_NWK_ADDR, sizeof( touchLinkFreeNwkAddrEnd ), &touchLinkFreeNwkAddrEnd );
  touchLink_ItemInit( ZCD_NV_MIN_FREE_GRP_ID, sizeof( touchLinkFreeGrpIdBegin ), &touchLinkFreeGrpIdBegin );
  touchLink_ItemInit( ZCD_NV_MAX_FREE_GRP_ID, sizeof( touchLinkFreeGrpIdEnd ), &touchLinkFreeGrpIdEnd );

  // Initialize our group ID range
  touchLink_ItemInit( ZCD_NV_MIN_GRP_IDS, sizeof( touchLinkGrpIDsBegin ), &touchLinkGrpIDsBegin );
  touchLink_ItemInit( ZCD_NV_MAX_GRP_IDS, sizeof( touchLinkGrpIDsEnd ), &touchLinkGrpIDsEnd );
}

/*********************************************************************
 * @fn      touchLink_ItemInit
 *
 * @brief   Initialize an NV item. If the item doesn't exist in NV memory,
 *          write the default (value passed in) into NV memory. But if
 *          it exists, set the item to the value stored in NV memory.
 *
 * @param   id - item id
 * @param   len - item len
 * @param   buf - pointer to the item
 *
 * @return  none
 */
void touchLink_ItemInit( uint16_t id, uint16_t len, void *pBuf )
{
#if defined ( NV_RESTORE )
  // If the item doesn't exist in NV memory, create and initialize
  // it with the value passed in.
  if ( osal_nv_item_init( id, len, pBuf ) == ZSuccess )
  {
    // The item already exists in NV memory, read it from NV memory
    osal_nv_read( id, 0, len, pBuf );
  }
#endif // NV_RESTORE
}

/*********************************************************************
 * @fn      touchLink_SetMacNwkParams
 *
 * @brief   Configure MAC with our Network Parameters.
 *
 * @param   nwkAddr - network address
 * @param   panId - PAN identifier
 * @param   channel
 *
 * @return  void
 */
void touchLink_SetMacNwkParams( uint16_t nwkAddr, uint16_t panId, uint8_t channel )
{
  // Set our short address
  ZMacSetReq( ZMacShortAddress, (byte*)&nwkAddr );

  // Set our PAN ID
  ZMacSetReq( ZMacPanId, (byte*)&panId );

  // Tune to the selected logical channel
  touchLink_SetChannel( channel );
}

/*********************************************************************
 * @fn      touchLink_SetChannel
 *
 * @brief   Set our channel.
 *
 * @param   channel - new channel to change to
 *
 * @return  void
 */
void touchLink_SetChannel( uint8_t channel )
{
  bdb_setChannel( (uint32_t) ( 1L << channel ) );

  // Set the new channel
  ZMacSetReq( ZMacChannel, &channel );
}

/*********************************************************************
 * @fn      touchLink_SendLeaveReq
 *
 * @brief   Send out a Leave Request command.
 *
 * @param   void
 *
 * @return  ZStatus_t
 */
ZStatus_t touchLink_SendLeaveReq( void )
{
  NLME_LeaveReq_t leaveReq;

  // Set every field to 0
  memset( &leaveReq, 0, sizeof( NLME_LeaveReq_t ) );

  // Send out our leave
  return ( NLME_LeaveReq( &leaveReq ) );
}

/*********************************************************************
 * @fn      touchLink_GetMsgRssi
 *
 * @brief   Get the RSSI of the message just received through a ZCL callback.
 *
 * @param   none
 *
 * @return  RSSI if AF message was received, TOUCHLINK_WORST_RSSI otherwise.
 */
int8_t touchLink_GetMsgRssi( void )
{
  afIncomingMSGPacket_t *pAF = zcl_getRawAFMsg();

  if ( pAF != NULL )
  {
    return ( pAF->rssi );
  }

  return ( TOUCHLINK_WORST_RSSI );
}

/*********************************************************************
 * @fn      touchLink_NewNwkUpdateId
 *
 * @brief   Determine the new network update id. The nwkUpdateId attribute
 *          can take the value of 0x00 - 0xff and may wrap around so care
 *          must be taken when comparing for newness.
 *
 * @param   ID1 - first nwk update id
 * @param   ID2 - second nwk update id
 *
 * @return  new nwk update ID
 */
uint8_t touchLink_NewNwkUpdateId( uint8_t ID1, uint8_t ID2 )
{
  if ( ( (ID1 >= ID2) && ((ID1 - ID2) > 200) )
      || ( (ID1 < ID2) && ((ID2 - ID1) > 200) ) )
  {
    return ( MIN( ID1, ID2 ) );
  }

  return ( MAX( ID1, ID2 ) );
}

/*********************************************************************
 * @fn      touchLink_PerformNetworkDisc
 *
 * @brief   Perform a Network Discovery scan.
 *          Scan results will be stored locally to analyze.
 *
 * @param   scanChannelList - channels to perform discovery scan
 *
 * @return  void
 */
void touchLink_PerformNetworkDisc( uint32_t scanChannelList )
{
  NLME_ScanFields_t scan;

  scan.channels = scanChannelList;
  scan.duration = BEACON_ORDER_240_MSEC;
  scan.scanType = ZMAC_ACTIVE_SCAN;
  scan.scanApp  = NLME_DISC_SCAN;

  if ( NLME_NwkDiscReq2( &scan ) == ZSuccess )
  {
    // Register ZDO callback to handle the network discovery confirm and
    // beacon notification confirm
    ZDO_RegisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID, touchLink_NwkDiscoveryCnfCB );
    ZDO_RegisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID, touchLink_BeaconIndCB );
  }
  else
  {
    NLME_NwkDiscTerm();
  }
}

/*********************************************************************
 * @fn      touchLink_BeaconIndCB
 *
 * @brief   Process the incoming beacon indication.
 *
 * @param   param -  pointer to a parameter and a structure of parameters
 *
 * @return  void
 */
static void *touchLink_BeaconIndCB ( void *param )
{
    NLME_beaconInd_t *pBeacon = param;

    if ( touchlinkPanId == pBeacon->panID )
    {
        // Clear the network descriptor
        memset( &discoveredTouchlinkNwk, 0, sizeof( touchLinkDiscoveredNwkParam_t )  );

        // Initialize the descriptor
        discoveredTouchlinkNwk.chosenRouter = INVALID_NODE_ADDR;
        discoveredTouchlinkNwk.chosenRouterDepth = 0xFF;

        // Update the descriptor with the incoming beacon
        discoveredTouchlinkNwk.logicalChannel = pBeacon->logicalChannel;
        discoveredTouchlinkNwk.PANID          = pBeacon->panID;

        // Save the extended PAN ID from the beacon payload only if 1.1 version network
        if ( pBeacon->protocolVersion != ZB_PROT_V1_0 )
        {
            osal_cpyExtAddr( discoveredTouchlinkNwk.extendedPANID, pBeacon->extendedPanID );
        }
        else
        {
            memset( discoveredTouchlinkNwk.extendedPANID, 0xFF, Z_EXTADDR_LEN );
        }

        // check if this device is a better choice to join...
        // ...dont bother checking assocPermit flag is doing a rejoin
        if ( pBeacon->LQI > gMIN_TREE_LQI )
        {
            uint8_t selected = FALSE;
            uint8_t capacity = FALSE;


            if ( ((pBeacon->LQI   > discoveredTouchlinkNwk.chosenRouterLinkQuality) &&
                    (pBeacon->depth <= MAX_NODE_DEPTH)) ||
                    ((pBeacon->LQI   == discoveredTouchlinkNwk.chosenRouterLinkQuality) &&
                            (pBeacon->depth < discoveredTouchlinkNwk.chosenRouterDepth)) )
            {
                selected = TRUE;
            }
            else
            {
                if ( pBeacon->depth < discoveredTouchlinkNwk.chosenRouterDepth )
                {
                    selected = TRUE;
                }
            }

            capacity = pBeacon->routerCapacity;

            if ( (capacity) && (selected) )
            {
                // this is the new chosen router for joining...
                discoveredTouchlinkNwk.chosenRouter            = pBeacon->sourceAddr;
                discoveredTouchlinkNwk.chosenRouterLinkQuality = pBeacon->LQI;
                discoveredTouchlinkNwk.chosenRouterDepth       = pBeacon->depth;
            }

            if ( (pBeacon->deviceCapacity == TRUE ) || ( pBeacon->routerCapacity == TRUE ))
                discoveredTouchlinkNwk.deviceCapacity = 1;
        }
    }

    return ( NULL );
}

/*********************************************************************
 * @fn      touchLink_NwkDiscoveryCnfCB
 *
 * @brief   Send an event to inform the target the completion of
 *          network discovery scan
 *
 * @param   param - pointer to a parameter and a structure of parameters
 *
 * @return  void
 */
static void *touchLink_NwkDiscoveryCnfCB ( void *param )
{
#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
  // Scan completed. De-register the callbacks with ZDO
  ZDO_DeregisterForZdoCB( ZDO_NWK_DISCOVERY_CNF_CBID );
  ZDO_DeregisterForZdoCB( ZDO_BEACON_NOTIFY_IND_CBID );

  NLME_NwkDiscTerm();

  if ( touchlinkPanId == discoveredTouchlinkNwk.PANID )
  {
    // proceed to join the network, otherwise
    // Notify our task
      OsalPortTimers_startTimer( touchLinkTaskId, TOUCHLINK_NWK_DISC_CNF_EVT, BDBCTL_MIN_STARTUP_DELAY_TIME );
  }
  else
  {
    // no suitable network in secondary channel list, then just wait for touchlink
#if ( ZSTACK_ROUTER_BUILD )
    // Try to create a new distributed network
    OsalPort_setEvent( touchLinkTaskId, TOUCHLINK_NWK_DISC_CNF_EVT );
#elif ( ZSTACK_END_DEVICE_BUILD )
    // Notify the BDB state machine
    bdbAttributes.bdbCommissioningStatus = BDB_COMMISSIONING_NO_NETWORK;
    bdb_reportCommissioningState( BDB_COMMISSIONING_STATE_TL, FALSE );
    // No parent to join in
    bdbCommissioningProcedureState.bdbCommissioningState = BDB_PARENT_LOST;
    NLME_OrphanStateSet( );
    bdb_recoverNwk( );
#endif
  }

#else
  (void)touchLinkTaskId;
  (void)param;
#endif
  return ( NULL );
}

/****************************************************************************
 * @fn      touchLink_FreeNwkParamList
 *
 * @brief   This function frees any network discovery data.
 *
 * @param   none
 *
 * @return  none
 */
void touchLink_FreeNwkParamList( void )
{
  // Clear the network descriptor
  memset( &discoveredTouchlinkNwk, 0, sizeof( touchLinkDiscoveredNwkParam_t )  );
  touchlinkPanId = INVALID_NODE_ADDR;
}

/****************************************************************************
 * @fn      touchLink_IsValidTransID
 *
 * @brief   Transaction ID Filter for Touch-Link received commands.
 *
 * @param   transID - received transaction ID
 *
 * @return  FALSE if not matching current or transaction expired
 */
bool touchLink_IsValidTransID( uint32_t transID )
{
  if ( ( touchLinkTransID == 0 ) || ( ( touchLinkTransID != transID ) && ( touchLinkLastAcceptedTransID != transID ) ) )
  {
    return ( FALSE );
  }
  return ( TRUE );
}

/*********************************************************************
 * @fn      touchLink_RouterProcessZDOMsg
 *
 * @brief   Process incoming ZDO messages (for routers)
 *
 * @param   inMsg - message to process
 *
 * @return  none
 */
void touchLink_RouterProcessZDOMsg( zdoIncomingMsg_t *inMsg )
{
  ZDO_DeviceAnnce_t devAnnce;

  switch ( inMsg->clusterID )
  {
    case Device_annce:
      {
        // all devices should send link status, including the one sending it
        ZDO_ParseDeviceAnnce( inMsg, &devAnnce );

        linkInfo_t *linkInfo;

        // check if entry exists
        linkInfo = nwkNeighborGetLinkInfo( devAnnce.nwkAddr, _NIB.nwkPanId );

        // if not, look for a vacant entry to add this node...
        if ( linkInfo == NULL )
        {
          nwkNeighborAdd( devAnnce.nwkAddr, _NIB.nwkPanId, DEF_LQI );
          linkInfo = nwkNeighborGetLinkInfo( devAnnce.nwkAddr, _NIB.nwkPanId );
          linkInfo->txCost = DEF_LINK_COST;
          linkInfo->rxLqi = MIN_LQI_COST_1;

          // if we have end device childs, send link status
          if ( AssocCount(CHILD_RFD, CHILD_RFD_RX_IDLE) > 0 )
          {
            NLME_UpdateLinkStatus();
          }
        }
      }
      break;

    case Mgmt_Permit_Join_req:
      {
        uint8_t duration = inMsg->asdu[ZDP_MGMT_PERMIT_JOIN_REQ_DURATION];
        ZStatus_t stat = NLME_PermitJoiningRequest( duration );
        // Send a response if unicast
        if ( !inMsg->wasBroadcast )
        {
          ZDP_MgmtPermitJoinRsp( inMsg->TransSeq, &(inMsg->srcAddr), stat, false );
        }
      }
      break;

    default:
      break;
  }
}

/*********************************************************************
 * @fn      touchLink_PermitJoin
 *
 * @brief   Set the router permit join flag, to allow or deny classical
 *          commissioning by other ZigBee devices.
 *
 * @param   duration - enable up to aplcMaxPermitJoinDuration seconds,
 *                     0 to disable
 *
 * @return  status
 */
ZStatus_t touchLink_PermitJoin( uint8_t duration )
{
  if ( duration > APLC_MAX_PERMIT_JOIN_DURATION )
  {
    duration = APLC_MAX_PERMIT_JOIN_DURATION;
  }
  return NLME_PermitJoiningRequest( duration );
}

/*********************************************************************
 * @fn      targetStartRtr
 *
 * @brief   Start operating on the new network.
 *
 * @param   pParams - pointer to received network parameters
 * @param   transID - transaction id
 *
 * @return  none
 */
void touchLinkStartRtr( bdbTLNwkParams_t *pParams, uint32_t transID )
{
  // Copy the new network parameters to
  touchLink_SetNIB( ( ZSTACK_ROUTER_BUILD ? NWK_ROUTER : NWK_REJOINING ), pParams->nwkAddr, pParams->extendedPANID,
              pParams->logicalChannel, pParams->panId, _NIB.nwkUpdateId );

  // Apply the received network key
  touchLink_DecryptNwkKey( pParams->nwkKey, pParams->keyIndex, transID, touchLinkResponseID );

  // setting apsTrustCenterAddress to 0xffffffff
  ZDSecMgrUpdateTCAddress( NULL );

  NLME_PermitJoiningRequest(0);

  // Touchlink distributed network flag
  touchlinkDistNwk = TRUE;

  // Use the new free ranges
  //touchLink_UpdateFreeRanges( pParams );

  // Save free ranges
  touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

  // In case we're here after a leave
  touchLinkLeaveInitiated = FALSE;

  // Clear leave control logic
  ZDApp_LeaveCtrlReset();

  // Start operating on the new network
  ZDOInitDeviceEx( 0, 1 );
}

/*********************************************************************
 * @fn      touchLink_DevRejoin
 *
 * @brief   Perform a network rejoin
 *
 * @param   rejoinInf - pointer to received network parameters
 *
 * @return  none
 */
void touchLink_DevRejoin( bdbTLNwkRejoin_t *rejoinInf )
{
    networkDesc_t *pNwkDesc = NULL;

    // Initialize the security for type of device
    ZDApp_SecInit( ZDO_INITDEV_RESTORED_NETWORK_STATE );

    pNwkDesc = (networkDesc_t *)OsalPort_malloc( sizeof( networkDesc_t ) );
    if( pNwkDesc == NULL )
    {
      return;
    }

    NwkDescList = pNwkDesc;
    pNwkDesc->panId = rejoinInf->panId;
    pNwkDesc->logicalChannel = rejoinInf->logicalChannel;
    OsalPort_memcpy( pNwkDesc->extendedPANID, rejoinInf->extendedPANID, Z_EXTADDR_LEN);
    pNwkDesc->chosenRouterDepth = 1;
    pNwkDesc->routerCapacity = 1;
    pNwkDesc->deviceCapacity = 1;
    pNwkDesc->version = 2;
    pNwkDesc->stackProfile = 2;
    pNwkDesc->chosenRouterLinkQuality = DEF_LQI;
    pNwkDesc->chosenRouter = rejoinInf->nwkAddr;
    pNwkDesc->updateId = rejoinInf->nwkUpdateId;
    pNwkDesc->nextDesc = NULL;

#if (ZG_BUILD_ENDDEVICE_TYPE)
    if(ZG_DEVICE_ENDDEVICE_TYPE)
    {
      // Enable response poll rare
      nwk_SetCurrentPollRateType(POLL_RATE_TYPE_JOIN_REJOIN, TRUE);
      nwk_SetCurrentPollRateType(POLL_RATE_DISABLED | POLL_RATE_RX_ON_TRUE, FALSE);
    }
#endif

    // Save free ranges
    touchLink_UpdateNV( TOUCHLINK_UPDATE_NV_RANGES );

    // In case we're here after a leave
    touchLinkLeaveInitiated = FALSE;

    // Clear leave control logic
    ZDApp_LeaveCtrlReset();

    // Let's join the network started by the target
    NLME_ReJoinRequest( rejoinInf->extendedPANID, _NIB.nwkLogicalChannel);
}

/*********************************************************************
 * @fn      touchLink_DeviceIsInitiator
 *
 * @brief   Set device initiator flag.
 *
 * @param   initiator - new flag value
 *
 * @return  none
 */
void touchLink_DeviceIsInitiator( bool initiator )
{
  touchLinkIsInitiator = initiator;
}

/*********************************************************************
 * @fn      touchLink_DeviceIsInitiator
 *
 * @brief   Get device initiator flag.
 *
 * @param   none
 *
 * @return  touchLinkIsInitiator - flag value
 */
bool touchLink_GetDeviceInitiator( void )
{
  return touchLinkIsInitiator;
}

#endif // BDB_TL_TARGET || BDB_TL_INITIATOR

/*********************************************************************
*********************************************************************/
