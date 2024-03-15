/**************************************************************************************************
  Filename:       reflecttrack.h
  Revised:        $Date: 2015-06-02 15:55:43 -0700 (Tue, 02 Jun 2015) $
  Revision:       $Revision: 43961 $

  Description:    APS Reflect Tracking Database Module


  Copyright 2005-2015 Texas Instruments Incorporated.

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

#ifndef REFLECTTRACK_H
#define REFLECTTRACK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "rom_jt_154.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// This is the default number of max entries.  If you would like to
// change this value, DON'T change this constant - call
//   reflectTracking_SetMaxEntries() instead.
#define REFTRACK_MAX_ENTRIES        5

// This is the maximum number of simultaneous messages sent for a reflection.
// For example if a message is to be reflected to 10 devices, this constant
// restricts the number of messages that are currently being sent, the others
// wait for the simultaneous messages to complete.  If you would like to
// change this value, DON'T change this constant - call
//   reflectTracking_SetMaxSimultaneous() instead.
#define REFTRACK_MAX_SIMUL          3

// This is the maximum number of indirect messages of REFTRACK_MAX_SIMUL.
// If you would like to change this value, DON'T change this constant - call
//   reflectTracking_SetMaxIndirect() instead.   The parameter can't be
// larger than REFTRACK_MAX_SIMUL
#define REFTRACK_MAX_INDIRECTS          1

// ReflectDestRec_t options
#define REFDEST_WAITING              0x00
#define REFDEST_SENT                 0x01
#define REFDEST_APS_ACK_EXPECTED     0x02
#define REFDEST_DELIVERED            0x04
#define REFDEST_FAILED               0x08
#define REFDEST_INDIRECT             0x80

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8_t  dstAddrMode;           // 0 - normal short addr, 1 - group Address
  uint16_t dstAddr;               // depends on dstAddrMode
  uint8_t  dstEP;
  uint8_t  handle;
  uint8_t  options;
  uint8_t  addrMgrIndex;          //Index in address manager to search for short address in case of not having it
} ReflectDestRec_t;

typedef struct
{
  void    *next;                // Next in the link List
  uint8_t   ID;                   // Unique ID
  uint8_t   srcEP;
  uint16_t  clusterID;
  uint8_t   confirmed;
  uint16_t  transID;
  uint8_t*  asdu;                 // saved asdu
  uint8_t   numdests;             // number of the destination reflections
  uint16_t  txOptions;            // original options from upper layer used to send the message
  uint8_t   apsRetries;           // number of times the packet has been APS retry
  ReflectDestRec_t  dests[];    // Arrary of destinations
                                // - the number of destinations must
                                // be used to allocate this element
} ReflectTracking_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

 /*
  * Set the Maximum number of tracking entries.  This is also the
  * number of simultaneous reflections that can occur.  Also, if you
  * would like to keep a history, make this value more than you need.
  */
extern void reflectTrack_SetMaxEntries( uint8_t newEntries );

/*
 * Set a new maximum for the maximum number of simultaneous
 * messages sent for a reflection.  For example if a message
 * is to be reflected to 10 devices, this constant restricts
 * the number of messages that are currently being sent, the
 * others wait for the simultaneous messages to complete.
 */
extern void reflectTracking_SetMaxSimultaneous( uint8_t newEntries );

/*
 * Get the number of simultaneous bufs.
 */
extern uint8_t reflectTracking_GetMaxSimultaneous( void );

/*
 * Get the number of simultaneous bufs adjusted for the
 *              number of bufs already sent.
 */
extern uint8_t reflectTracking_GetMaxSimultaneousAdjusted( ReflectTracking_t *entry );

/*
 * Set a new maximum for the maximum number of indirect
 *              messages of REFTRACK_MAX_SIMUL.
 */
extern void reflectTracking_SetMaxIndirect( uint8_t newEntries );

/*
 * Get a new maximum for the maximum number of indirect
 *              messages.
 */
extern uint8_t reflectTracking_GetMaxIndirect( void );

/*
 * Get a new maximum for the maximum number of indirect
 *              messages adjusted for the number being sent.
 */
extern uint8_t reflectTracking_GetMaxIndirectAdjusted( ReflectTracking_t *entry );

/*
  * Gets a memory block for a reflect tracking entry. This doesn't add
  * to the list.  It just gets the memory, you must fill in and add to
  * the list by calling
  */
extern ReflectTracking_t *reflectTrack_GetNewEntry( uint8_t numDests,
                                                    uint8_t asduLen );

 /*
  * Get the number of entries currently in the list.
  */
extern uint8_t reflectTrack_NumEntries( void );

/*
 * Remove an entry from the list and release its memory.
 */
extern void reflectTrack_RemoveEntry( ReflectTracking_t *entry );

/*
 * Stub for notifying user of entry deletion
 */
extern void reflectTrack_RemoveEntryNotify( ReflectTracking_t *entry);

/*
 * Add this entry to the end of the list.
 */
extern void reflectTrack_AddToEnd( ReflectTracking_t *entry );

/*
 * Add entry to the list.  It will add to the end of the list.
 *              if the list is full, the first entry will be deleted.
 *    returns the unique reflect ID
 */
extern uint8_t reflectTrack_AddNewEntry( ReflectTracking_t *newEntry );

/*
 * Find entry in list based on the reflectID returned
 *              from the call to reflectTrack_AddNewEntry().
 */
extern ReflectTracking_t *reflectTrack_FindEntry( byte refID );

/*
 * Find entry in list based on the handle returned
 *              from the call to reflectTrack_AddNewEntry().
 */
extern ReflectTracking_t *reflectTrack_FindHandle( byte handle );

/*
 * Update (OR in) options for the passed in parameters.
 */
extern ReflectDestRec_t *reflectTrack_UpdateOption( ReflectTracking_t *ref, byte handle, byte option );

/*
 * Update the nsduHandle for the passed in parameters.
 */
extern void reflectTrack_UpdateHandle( uint8_t oldHandle, uint8_t newHandle );

/*
 * Find entry in list based on the source address, endpoint,
 *              and clusterID.
 */
extern ReflectTracking_t *reflectTrack_FindEntryStatus( uint8_t endpoint, uint16_t clusterID );

/*
 * Returns the number of reflections completed.
 */
extern uint8_t reflectTracking_GetNumDelivered( ReflectTracking_t *entry );

/*
 * Returns the number of reflections failed.
 */
extern uint8_t reflectTracking_GetNumFailed( ReflectTracking_t *entry );

/*
 * Fills in an array of failed short addresses.
 */
extern void reflectTracking_FillInFailed( ReflectTracking_t *entry, uint16_t *list );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* REFLECTTRACK_H */


