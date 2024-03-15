/**************************************************************************************************
  Filename:       ZDiags.c
  Revised:        $Date: 2014-03-13 15:53:56 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37678 $

  Description:    Diagnostics module.


  Copyright 2014 Texas Instruments Incorporated.

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
#include "osal_nv.h"
#include "zdiags.h"
#include "zmac.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
#if defined ( FEATURE_SYSTEM_STATS )
static DiagStatistics_t DiagsStatsTable;
#endif

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */


/****************************************************************************
 * @fn          ZDiagsInitStats
 *
 * @brief       Initialize the statistics table in NV or restore values from
 *              NV into the Statistics table in RAM
 *
 * @param       none.
 *
 * @return      ZSuccess - if NV data was initialized successfully.
 *              ZFailure - Otherwise
 */
uint8_t ZDiagsInitStats( void )
{
  uint8_t retValue = ZSuccess;

#if defined ( FEATURE_SYSTEM_STATS )
  uint8_t status;

  // Initialize structure with default values, and do not try to write to NV
  // because item does not exist yet
  (void)ZDiagsClearStats( FALSE );

  status = osal_nv_item_init( ZCD_NV_DIAGNOSTIC_STATS,
                              (uint16_t)sizeof( DiagStatistics_t ),
                              &DiagsStatsTable );

  if ( status == NV_OPER_FAILED )
  {
    retValue = ZFailure;
  }
  else
  {
    // Item existed, restore NV values into RAM table
    if ( status == SUCCESS )
    {
      if ( NV_OPER_FAILED == ZDiagsRestoreStatsFromNV() )
      {
        retValue = ZFailure;
      }
    }
  }
#endif // FEATURE_SYSTEM_STATS

  return ( retValue );
}

/****************************************************************************
 * @fn          ZDiagsClearStats
 *
 * @brief       Clears the statistics table in RAM and NV if option flag set.
 *
 * @param       clearNV   - Option flag to clear NV data.
 *
 * @return      System Clock.
 */
uint32_t ZDiagsClearStats( bool clearNV )
{
  uint32_t retValue = 0;

#if defined ( FEATURE_SYSTEM_STATS )
  // clears statistics table
  memset( &DiagsStatsTable, 0, sizeof( DiagStatistics_t ) );

  // saves System Clock when statistics were cleared
  retValue = DiagsStatsTable.SysClock = MAP_osal_GetSystemClock();

  if ( clearNV )
  {
    uint16_t bootCnt = 0;

    // Boot count is not part of DiagsStatsTable, it has to be initialized separately
    osal_nv_write( ZCD_NV_BOOTCOUNTER, sizeof(bootCnt), &bootCnt );

    // Clears values in NV and saves the system clock for the last time stats were cleared
    osal_nv_write( ZCD_NV_DIAGNOSTIC_STATS, sizeof( DiagStatistics_t ), &DiagsStatsTable );
  }
#endif // FEATURE_SYSTEM_STATS

  return ( retValue );
}

/****************************************************************************
 * @fn          ZDiagsUpdateStats
 *
 * @brief       Update statistics and/or metrics for a specific Attribute Id
 *
 * @param       attributeId  input  - unique identifier for the required attribute
 *
 * @return      none.
 */
void ZDiagsUpdateStats( uint16_t attributeId )
{
#if defined ( FEATURE_SYSTEM_STATS )
  switch ( attributeId )
  {
    // System and Hardware Diagnostics
    case ZDIAGS_SYSTEM_CLOCK:
      DiagsStatsTable.SysClock = MAP_osal_GetSystemClock();
      break;

    case ZDIAGS_PERSISTENT_MEMORY_WRITES:
      DiagsStatsTable.PersistentMemoryWrites++;
      break;

    // NWK Diagnostics
    case ZDIAGS_ROUTE_DISC_INITIATED:
      DiagsStatsTable.RouteDiscInitiated++;
      break;

    case ZDIAGS_NEIGHBOR_ADDED:
      DiagsStatsTable.NeighborAdded++;
      break;

    case ZDIAGS_NEIGHBOR_REMOVED:
      DiagsStatsTable.NeighborRemoved++;
      break;

    case ZDIAGS_NEIGHBOR_STALE:
      DiagsStatsTable.NeighborStale++;
      break;

    case ZDIAGS_JOIN_INDICATION:
      DiagsStatsTable.JoinIndication++;
      break;

    case ZDIAGS_CHILD_MOVED:
      DiagsStatsTable.ChildMoved++;
      break;

    case ZDIAGS_NWK_FC_FAILURE:
      DiagsStatsTable.NwkFcFailure++;
      break;

    case ZDIAGS_NWK_DECRYPT_FAILURES:
      DiagsStatsTable.NwkDecryptFailures++;
      break;

    case ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES:
      DiagsStatsTable.PacketBufferAllocateFailures++;
      break;

    case ZDIAGS_RELAYED_UCAST:
      DiagsStatsTable.RelayedUcast++;
      break;

    case ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED:
      DiagsStatsTable.PhyToMacQueueLimitReached++;
      break;

    case ZDIAGS_PACKET_VALIDATE_DROP_COUNT:
      DiagsStatsTable.PacketValidateDropCount++;
      break;

    // APS Diagnostics
    case ZDIAGS_APS_RX_BCAST:
      DiagsStatsTable.ApsRxBcast++;
      break;

    case ZDIAGS_APS_TX_BCAST:
      DiagsStatsTable.ApsTxBcast++;
      break;

    case ZDIAGS_APS_RX_UCAST:
      DiagsStatsTable.ApsRxUcast++;
      break;

    case ZDIAGS_APS_TX_UCAST_SUCCESS:
      DiagsStatsTable.ApsTxUcastSuccess++;
      break;

    case ZDIAGS_APS_TX_UCAST_RETRY:
      DiagsStatsTable.ApsTxUcastRetry++;
      break;

    case ZDIAGS_APS_TX_UCAST_FAIL:
      DiagsStatsTable.ApsTxUcastFail++;
      break;

    case ZDIAGS_APS_FC_FAILURE:
      DiagsStatsTable.ApsFcFailure++;
      break;

    case ZDIAGS_APS_UNAUTHORIZED_KEY:
      DiagsStatsTable.ApsUnauthorizedKey++;
      break;

    case ZDIAGS_APS_DECRYPT_FAILURES:
      DiagsStatsTable.ApsDecryptFailures++;
      break;

    case ZDIAGS_APS_INVALID_PACKETS:
      DiagsStatsTable.ApsInvalidPackets++;
      break;

    case ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS:
      DiagsStatsTable.MacRetriesPerApsTxSuccess++;
      break;

    default:
      break;
  }
#endif // FEATURE_SYSTEM_STATS
}

/****************************************************************************
 * @fn          ZDiagsGetStatsAttr
 *
 * @brief       Reads specific systemID statistics and/or metrics
 *
 * @param       attributeId  input  - unique identifier for the required attribute
 *
 *   NOTE: the user of this function will have to cast the value
 *         based on the type of the attributeID, the returned value
 *         will allways be uint32_t.
 *
 * @return      Value of the attribute requested.
 */
uint32_t ZDiagsGetStatsAttr( uint16_t attributeId )
{
  uint32_t diagsValue = 0;

#if defined ( FEATURE_SYSTEM_STATS )
  switch ( attributeId )
  {
    // System and Hardware Diagnostics
    case ZDIAGS_SYSTEM_CLOCK:
      // this is the system clock when statistics were cleared;
      diagsValue = DiagsStatsTable.SysClock;
      break;

    case ZDIAGS_NUMBER_OF_RESETS:
      // Get the value from NV memory
      osal_nv_read( ZCD_NV_BOOTCOUNTER, 0, sizeof(uint16_t), &diagsValue );
      break;

    case ZDIAGS_PERSISTENT_MEMORY_WRITES:
      diagsValue = DiagsStatsTable.PersistentMemoryWrites;
      break;

    // MAC Diagnostics
    case ZDIAGS_MAC_RX_CRC_PASS:
      ZMacGetReq( ZMacDiagsRxCrcPass, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacRxCrcPass = diagsValue;
      break;

    case ZDIAGS_MAC_RX_CRC_FAIL:
      ZMacGetReq( ZMacDiagsRxCrcFail, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacRxCrcFail = diagsValue;
      break;

    case ZDIAGS_MAC_RX_BCAST:
      ZMacGetReq( ZMacDiagsRxBcast, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacRxBcast = diagsValue;
      break;

    case ZDIAGS_MAC_TX_BCAST:
      ZMacGetReq( ZMacDiagsTxBcast, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacTxBcast = diagsValue;
      break;

    case ZDIAGS_MAC_RX_UCAST:
      ZMacGetReq( ZMacDiagsRxUcast, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacRxUcast = diagsValue;
      break;

    case ZDIAGS_MAC_TX_UCAST:
      ZMacGetReq( ZMacDiagsTxUcast, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacTxUcast = diagsValue;
      break;

    case ZDIAGS_MAC_TX_UCAST_RETRY:
      ZMacGetReq( ZMacDiagsTxUcastRetry, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacTxUcastRetry = diagsValue;
      break;

    case ZDIAGS_MAC_TX_UCAST_FAIL:
      ZMacGetReq( ZMacDiagsTxUcastFail, (uint8_t *)&diagsValue );
      // Update the statistics table with this value from MAC
      DiagsStatsTable.MacTxUcastFail = diagsValue;
      break;

    // NWK Diagnostics
    case ZDIAGS_ROUTE_DISC_INITIATED:
      diagsValue = DiagsStatsTable.RouteDiscInitiated;
      break;

    case ZDIAGS_NEIGHBOR_ADDED:
      diagsValue = DiagsStatsTable.NeighborAdded;
      break;

    case ZDIAGS_NEIGHBOR_REMOVED:
      diagsValue = DiagsStatsTable.NeighborRemoved;
      break;

    case ZDIAGS_NEIGHBOR_STALE:
      diagsValue = DiagsStatsTable.NeighborStale;
      break;

    case ZDIAGS_JOIN_INDICATION:
      diagsValue = DiagsStatsTable.JoinIndication;
      break;

    case ZDIAGS_CHILD_MOVED:
      diagsValue = DiagsStatsTable.ChildMoved;
      break;

    case ZDIAGS_NWK_FC_FAILURE:
      diagsValue = DiagsStatsTable.NwkFcFailure;
      break;

    case ZDIAGS_NWK_DECRYPT_FAILURES:
      diagsValue = DiagsStatsTable.NwkDecryptFailures;
      break;

    case ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES:
      diagsValue = DiagsStatsTable.PacketBufferAllocateFailures;
      break;

    case ZDIAGS_RELAYED_UCAST:
      diagsValue = DiagsStatsTable.RelayedUcast;
      break;

    case ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED:
      diagsValue = DiagsStatsTable.PhyToMacQueueLimitReached;
      break;

    case ZDIAGS_PACKET_VALIDATE_DROP_COUNT:
      diagsValue = DiagsStatsTable.PacketValidateDropCount;
      break;

    // APS Diagnostics
    case ZDIAGS_APS_RX_BCAST:
      diagsValue = DiagsStatsTable.ApsRxBcast;
      break;

    case ZDIAGS_APS_TX_BCAST:
      diagsValue = DiagsStatsTable.ApsTxBcast;
      break;

    case ZDIAGS_APS_RX_UCAST:
      diagsValue = DiagsStatsTable.ApsRxUcast;
      break;

    case ZDIAGS_APS_TX_UCAST_SUCCESS:
      diagsValue = DiagsStatsTable.ApsTxUcastSuccess;
      break;

    case ZDIAGS_APS_TX_UCAST_RETRY:
      diagsValue = DiagsStatsTable.ApsTxUcastRetry;
      break;

    case ZDIAGS_APS_TX_UCAST_FAIL:
      diagsValue = DiagsStatsTable.ApsTxUcastFail;
      break;

    case ZDIAGS_APS_FC_FAILURE:
      diagsValue = DiagsStatsTable.ApsFcFailure;
      break;

    case ZDIAGS_APS_UNAUTHORIZED_KEY:
      diagsValue = DiagsStatsTable.ApsUnauthorizedKey;
      break;

    case ZDIAGS_APS_DECRYPT_FAILURES:
      diagsValue = DiagsStatsTable.ApsDecryptFailures;
      break;

    case ZDIAGS_APS_INVALID_PACKETS:
      diagsValue = DiagsStatsTable.ApsInvalidPackets;
      break;

    case ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS:
      diagsValue = DiagsStatsTable.MacRetriesPerApsTxSuccess;
      break;


    default:
      break;
  }
#endif // FEATURE_SYSTEM_STATS

  return ( diagsValue );
}

/****************************************************************************
 * @fn          ZDiagsGetStatsTable
 *
 * @brief       Reads the statistics and metrics table
 *
 * @return      pointer to ZDiagStatistics_t structure.
 */
DiagStatistics_t *ZDiagsGetStatsTable( void )
{
#if defined ( FEATURE_SYSTEM_STATS )
  // call this function to update the DiagsStatsTable with MAC values,
  // the return value does not need to be saved because the function
  // is updating the value in DiagsStatsTable
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_CRC_PASS );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_CRC_FAIL );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_BCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_BCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_UCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_UCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_UCAST_RETRY );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_UCAST_FAIL );

  return ( &DiagsStatsTable );
#else
  return ( NULL );
#endif  // FEATURE_SYSTEM_STATS
}

/****************************************************************************
 * @fn          ZDiagsRestoreStatsFromNV
 *
 * @brief       Restores the statistics table from NV into the RAM table.
 *
 * @param       none.
 *
 * @return      ZSuccess - if NV data was restored from NV.
 *              ZFailure - Otherwise, NV_OPER_FAILED for failure.
 */
uint8_t ZDiagsRestoreStatsFromNV( void )
{
  uint8_t retValue = ZFailure;

#if defined ( FEATURE_SYSTEM_STATS )

  // restore diagnostics table from NV into RAM table
  if ( osal_nv_read( ZCD_NV_DIAGNOSTIC_STATS, 0,
                         (uint16_t)sizeof( DiagStatistics_t ),
                         &DiagsStatsTable ) == SUCCESS )
  {
    // restore MAC values into the PIB
    /*
    ZMacSetReq( ZMacDiagsRxCrcPass, (uint8_t *)&(DiagsStatsTable.MacRxCrcPass) );
    ZMacSetReq( ZMacDiagsRxCrcFail, (uint8_t *)&(DiagsStatsTable.MacRxCrcFail) );
    ZMacSetReq( ZMacDiagsRxBcast, (uint8_t *)&(DiagsStatsTable.MacRxBcast) );
    ZMacSetReq( ZMacDiagsTxBcast, (uint8_t *)&(DiagsStatsTable.MacTxBcast) );
    ZMacSetReq( ZMacDiagsRxUcast, (uint8_t *)&(DiagsStatsTable.MacRxUcast) );
    ZMacSetReq( ZMacDiagsTxUcast, (uint8_t *)&(DiagsStatsTable.MacTxUcast) );
    ZMacSetReq( ZMacDiagsTxUcastRetry, (uint8_t *)&(DiagsStatsTable.MacTxUcastRetry) );
    ZMacSetReq( ZMacDiagsTxUcastFail, (uint8_t *)&(DiagsStatsTable.MacTxUcastFail) );
*/
    retValue = ZSuccess;
  }
#endif // FEATURE_SYSTEM_STATS

  return ( retValue );
}

/****************************************************************************
 * @fn          ZDiagsSaveStatsToNV
 *
 * @brief       Saves the statistics table from RAM to NV.
 *
 * @param       none.
 *
 * @return      System Time.
 */
uint32_t ZDiagsSaveStatsToNV( void )
{
  uint32_t sysClock = 0;

#if defined ( FEATURE_SYSTEM_STATS )
  // call this function to update the DiagsStatsTable with MAC values,
  // the return value does not need to be saved because the function
  // is updating the value in DiagsStatsTable
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_CRC_PASS );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_CRC_FAIL );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_BCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_BCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_RX_UCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_UCAST );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_UCAST_RETRY );
  (void)ZDiagsGetStatsAttr( ZDIAGS_MAC_TX_UCAST_FAIL );

  // System Clock when statistics were saved
  sysClock = DiagsStatsTable.SysClock = MAP_osal_GetSystemClock();

  // save the statistics table from RAM to NV
  osal_nv_write( ZCD_NV_DIAGNOSTIC_STATS,
                 sizeof( DiagStatistics_t ), &DiagsStatsTable );
#endif

  // returns the System Time
  return ( sysClock );
}

/****************************************************************************
****************************************************************************/

