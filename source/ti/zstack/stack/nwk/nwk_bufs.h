/**************************************************************************************************
  Filename:       nwk_bufs.h
  Revised:        $Date: 2014-11-18 02:32:26 -0800 (Tue, 18 Nov 2014) $
  Revision:       $Revision: 41160 $

  Description:    Network buffer utility functions.


  Copyright 2004-2014 Texas Instruments Incorporated.

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

#ifndef NWK_BUFS_H
#define NWK_BUFS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "zmac.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
// Data buffer queue states
#define NWK_DATABUF_INIT        0     // Initialized but not queued.
#define NWK_DATABUF_WAITING     1     // Waiting to be sent out.
#define NWK_DATABUF_SENT        2     // Sent to the MAC - waiting for confirm.
#define NWK_DATABUF_CONFIRMED   3     // Waiting to be resent or deleted.
#define NWK_DATABUF_SCHEDULED   4     // Queued and waiting N msecs til send.
#define NWK_DATABUF_HOLD        5     // Hold msg for sleeping end device.
#define NWK_DATABUF_DONE        6     // Waiting to be deleted.

// Handle options
#define HANDLE_NONE               0x0000u
#define HANDLE_CNF                0x0001u
#define HANDLE_WAIT_FOR_ACK       0x0002u
#define HANDLE_BROADCAST          0x0004u
#define HANDLE_REFLECT            0x0008u
#define HANDLE_DELAY              0x0010u
#define HANDLE_HI_DELAY           0x0020u
//#define HANDLE_DIRECT             0x0040
#define HANDLE_SKIP_ROUTING       0x0040u
#define HANDLE_RTRY_MASK          0x0380u
#define HANDLE_RTRY_SHIFT         7
#define HANDLE_FORCE_INDIRECT     0x0400u
#define HANDLE_INDIRECT_HOLD      0x0800u      // A bit indicating the indirect msg has been in HOLD state
#define HANDLE_MCAST              0x1000u
#define HANDLE_MASK  \
  ~( HANDLE_CNF | HANDLE_WAIT_FOR_ACK | HANDLE_BROADCAST | HANDLE_DELAY | \
     HANDLE_HI_DELAY | HANDLE_FORCE_INDIRECT | HANDLE_MCAST)

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint8_t type;
  uint16_t txOptions;
  void* load;
} nwkDB_UserData_t;

typedef struct
{
  ZMacDataReq_t *pDataReq;
  void *next;
  uint16_t dataX;
  uint16_t handleOptions;     // Packet type options
  uint8_t nsduHandle;         // unique ID
  uint8_t state;              // state of buffer
  uint8_t retries;            // number of APS retries
  uint8_t lastCnfStatus;      // The last MAC_MCPS_DATA_CNF status
  nwkDB_UserData_t ud;      // user data
  uint16_t macSrcAddr;        // original MAC src address
  uint8_t  apsRetries;        // Number of retries by APS layer
} nwkDB_t;

typedef uint8_t (*nwkDB_FindMatchCB_t)( nwkDB_t* db, void* mf );

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Variable initialization
 */
extern void nwkbufs_init( void );

/*
 * Send the next buffer
 */
extern void nwk_SendNextDataBuf( void );

/*
 * Delete all buffers for a device
 */
extern uint8_t nwk_DeleteDataBufs( uint16_t nwkAddr );

/*
 * Determines whether or not the data buffers are full.
 */
extern byte nwk_MacDataBuffersFull( void );

/*
 * Add buffer to the send queue, if already in queue - set
 */
extern uint8_t nwk_MacDataBuffersAdd( nwkDB_t* db, uint8_t sent );

/*
 * Deallocate the sent MAC Data Buffer
 *
 */
extern uint8_t nwk_MacDataBuffersDealloc( byte handle );

/*
 * Checks if an end device has a message pending in the MAC NP.
 */
extern uint8_t nwkDB_MaxIndirectSent( uint16_t addr );


/*********************************************************************
*  Queued Allocated functions
*/

/*
 * Create the header
 */
extern nwkDB_t *nwkDB_CreateHdr( ZMacDataReq_t *pkt, byte handle, uint16_t handleOptions );

/*
 * Add a buffer to the queue.
 */
extern ZStatus_t nwkDB_Add( nwkDB_t *pkt, byte type, uint16_t dataX );

/*
 * Find the number of buffers with type.
 */
extern byte nwkDB_CountTypes( byte type );

/*
 * Find the next type in list.
 */
extern nwkDB_t *nwkDB_FindNextType( nwkDB_t *pkt, byte type, byte direct );

/*
 * Find the rec with handle.
 */
extern nwkDB_t *nwkDB_FindHandle( byte handle );

/*
 * Find the rec with destination address.
 */
extern nwkDB_t *nwkDB_FindDstAddr( uint16_t addr );

/*
 * Find the rec with MAC data packet.
 */
extern nwkDB_t *nwkDB_FindDataPkt( ZMacDataReq_t *pkt );

/*
 * Find a buffer match.
 */
extern nwkDB_t* nwkDB_FindMatch( nwkDB_FindMatchCB_t cb, void* mf );

/*
 * Find and remove from the list.  This function doesn't
 *           free the memory used by the record.
 */
extern void nwkDB_RemoveFromList( nwkDB_t *pkt );

/*
 * Frees the data, mac packet and hdr
 */
extern void nwkDB_DeleteRecAll( nwkDB_t *rec );

/*
 * Setup hold state and timer tick.
 */
extern void nwkbufs_hold( nwkDB_t *rec );

/*
 * Return cntIndirectHolding
 */
extern uint8_t nwkDB_ReturnIndirectHoldingCnt( void );

/*
 * Count Indirect held message
 */
extern uint8_t nwkDB_CountIndirectHold( void );

/*
 * Delete all records and reset queue
 */
extern void nwkbufs_reset( void );

/*
 * Stub to load the user frame data
 */
extern void* nwkDB_UserDataLoad( nwkDB_UserData_t* ud );

/*********************************************************************
*  Broadcast Message Queue functions
*/

/*
 * Add a broadcast data indication to the network broadcast queue
 */
extern uint8_t nwk_broadcastSend( uint8_t *msg_ptr );

/*
 * Remove a broadcast data indication to the network broadcast queue
 */
extern uint8_t *nwk_broadcastReceive( void );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* NWK_BUFS_H */


