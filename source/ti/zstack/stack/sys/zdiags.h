/**************************************************************************************************
  Filename:       ZDiags.h
  Revised:        $Date: 2014-03-13 15:53:56 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37678 $

  Description:    This interface provides all the definitions for the
                  Diagnostics module.


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

#ifndef ZDIAGS_H
#define ZDIAGS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * CONSTANTS
 */
// System and Hardware Attributes, ID range 0 - 99
#define ZDIAGS_SYSTEM_CLOCK                             0x0000  // System Clock when stats were saved/cleared
#define ZDIAGS_NUMBER_OF_RESETS                         0x0001  // Increments every time the system resets
#define ZDIAGS_PERSISTENT_MEMORY_WRITES                 0x0002  // *** NOT IMPLEMENTED ***

// MAC Attributes, ID range 100 - 199
#define ZDIAGS_MAC_RX_CRC_PASS                          0x0064  // MAC diagnostic CRC success counter
#define ZDIAGS_MAC_RX_CRC_FAIL                          0x0065  // MAC diagnostic CRC failure counter
#define ZDIAGS_MAC_RX_BCAST                             0x0066  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_TX_BCAST                             0x0067  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_RX_UCAST                             0x0068  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_TX_UCAST                             0x0069  // *** NOT IMPLEMENTED ***
#define ZDIAGS_MAC_TX_UCAST_RETRY                       0x006A  // MAC layer retries a unicast
#define ZDIAGS_MAC_TX_UCAST_FAIL                        0x006B  // Mac layer fails to send a unicast

// NWK Atributes, ID range 200 - 299
#define ZDIAGS_ROUTE_DISC_INITIATED                     0x00C8  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NEIGHBOR_ADDED                           0x00C9  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NEIGHBOR_REMOVED                         0x00CA  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NEIGHBOR_STALE                           0x00CB  // *** NOT IMPLEMENTED ***
#define ZDIAGS_JOIN_INDICATION                          0x00CC  // *** NOT IMPLEMENTED ***
#define ZDIAGS_CHILD_MOVED                              0x00CD  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NWK_FC_FAILURE                           0x00CE  // *** NOT IMPLEMENTED ***
#define ZDIAGS_NWK_DECRYPT_FAILURES                     0x00CF  // NWK packet decryption failed
#define ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES          0x00D0  // *** NOT IMPLEMENTED ***
#define ZDIAGS_RELAYED_UCAST                            0x00D1  // *** NOT IMPLEMENTED ***
#define ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED           0x00D2  // *** NOT IMPLEMENTED ***
#define ZDIAGS_PACKET_VALIDATE_DROP_COUNT               0x00D3  // NWK packet drop because of validation error

// APS Attributes, ID range 300 - 399
#define ZDIAGS_APS_RX_BCAST                             0x012C  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_TX_BCAST                             0x012D  // APS layer transmits broadcast
#define ZDIAGS_APS_RX_UCAST                             0x012E  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_TX_UCAST_SUCCESS                     0x012F  // APS layer successfully transmits a unicast
#define ZDIAGS_APS_TX_UCAST_RETRY                       0x0130  // APS layer retries the sending of a unicast
#define ZDIAGS_APS_TX_UCAST_FAIL                        0x0131  // APS layer fails to send a unicast
#define ZDIAGS_APS_FC_FAILURE                           0x0132  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_UNAUTHORIZED_KEY                     0x0133  // *** NOT IMPLEMENTED ***
#define ZDIAGS_APS_DECRYPT_FAILURES                     0x0134  // APS packet decryption failed
#define ZDIAGS_APS_INVALID_PACKETS                      0x0135  // APS invalid packet dropped
#define ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS           0x0136  // Number of MAC retries per APS message successfully Tx

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint32_t SysClock;                          // ZDIAGS_SYSTEM_CLOCK
  uint16_t PersistentMemoryWrites;            // ZDIAGS_PERSISTENT_MEMORY_WRITES *** NOT IMPLEMENTED ***

  uint32_t MacRxCrcPass;                      // ZDIAGS_MAC_RX_CRC_PASS
  uint32_t MacRxCrcFail;                      // ZDIAGS_MAC_RX_CRC_FAIL
  uint32_t MacRxBcast;                        // ZDIAGS_MAC_RX_BCAST *** NOT IMPLEMENTED ***
  uint32_t MacTxBcast;                        // ZDIAGS_MAC_TX_BCAST *** NOT IMPLEMENTED ***
  uint32_t MacRxUcast;                        // ZDIAGS_MAC_RX_UCAST *** NOT IMPLEMENTED ***
  uint32_t MacTxUcast;                        // ZDIAGS_MAC_TX_UCAST *** NOT IMPLEMENTED ***
  uint32_t MacTxUcastRetry;                   // ZDIAGS_MAC_TX_UCAST_RETRY
  uint32_t MacTxUcastFail;                    // ZDIAGS_MAC_TX_UCAST_FAIL

  uint16_t RouteDiscInitiated;                // ZDIAGS_ROUTE_DISC_INITIATED *** NOT IMPLEMENTED ***
  uint16_t NeighborAdded;                     // ZDIAGS_NEIGHBOR_ADDED *** NOT IMPLEMENTED ***
  uint16_t NeighborRemoved;                   // ZDIAGS_NEIGHBOR_REMOVED *** NOT IMPLEMENTED ***
  uint16_t NeighborStale;                     // ZDIAGS_NEIGHBOR_STALE *** NOT IMPLEMENTED ***
  uint16_t JoinIndication;                    // ZDIAGS_JOIN_INDICATION *** NOT IMPLEMENTED ***
  uint16_t ChildMoved;                        // ZDIAGS_CHILD_MOVED *** NOT IMPLEMENTED ***
  uint16_t NwkFcFailure;                      // ZDIAGS_NWK_FC_FAILURE *** NOT IMPLEMENTED ***
  uint16_t NwkDecryptFailures;                // ZDIAGS_NWK_DECRYPT_FAILURES
  uint16_t PacketBufferAllocateFailures;      // ZDIAGS_PACKET_BUFFER_ALLOCATE_FAILURES *** NOT IMPLEMENTED ***
  uint16_t RelayedUcast;                      // ZDIAGS_RELAYED_UCAST *** NOT IMPLEMENTED ***
  uint16_t PhyToMacQueueLimitReached;         // ZDIAGS_PHY_TO_MAC_QUEUE_LIMIT_REACHED *** NOT IMPLEMENTED ***
  uint16_t PacketValidateDropCount;           // ZDIAGS_PACKET_VALIDATE_DROP_COUNT

  uint16_t ApsRxBcast;                        // ZDIAGS_APS_RX_BCAST *** NOT IMPLEMENTED ***
  uint16_t ApsTxBcast;                        // ZDIAGS_APS_TX_BCAST
  uint16_t ApsRxUcast;                        // ZDIAGS_APS_RX_UCAST *** NOT IMPLEMENTED ***
  uint16_t ApsTxUcastSuccess;                 // ZDIAGS_APS_TX_UCAST_SUCCESS
  uint16_t ApsTxUcastRetry;                   // ZDIAGS_APS_TX_UCAST_RETRY
  uint16_t ApsTxUcastFail;                    // ZDIAGS_APS_TX_UCAST_FAIL
  uint16_t ApsFcFailure;                      // ZDIAGS_APS_FC_FAILURE *** NOT IMPLEMENTED ***
  uint16_t ApsUnauthorizedKey;                // ZDIAGS_APS_UNAUTHORIZED_KEY *** NOT IMPLEMENTED ***
  uint16_t ApsDecryptFailures;                // ZDIAGS_APS_DECRYPT_FAILURES
  uint16_t ApsInvalidPackets;                 // ZDIAGS_APS_INVALID_PACKETS
  uint16_t MacRetriesPerApsTxSuccess;         // ZDIAGS_MAC_RETRIES_PER_APS_TX_SUCCESS
} DiagStatistics_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * FUNCTIONS
 */
extern uint8_t ZDiagsInitStats( void );

extern uint32_t ZDiagsClearStats( bool clearNV );

extern void ZDiagsUpdateStats( uint16_t attributeId );

extern uint32_t ZDiagsGetStatsAttr( uint16_t attributeId );

extern DiagStatistics_t *ZDiagsGetStatsTable( void );

extern uint8_t ZDiagsRestoreStatsFromNV( void );

extern uint32_t ZDiagsSaveStatsToNV( void );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZDIAGS_H */
