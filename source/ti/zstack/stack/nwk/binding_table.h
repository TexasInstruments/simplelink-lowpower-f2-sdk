/**************************************************************************************************
  Filename:       BindingTable.h
  Revised:        $Date: 2014-07-16 11:03:22 -0700 (Wed, 16 Jul 2014) $
  Revision:       $Revision: 39430 $

  Description:    Device binding table.


  Copyright 2004-2013 Texas Instruments Incorporated.

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

#ifndef BINDINGTABLE_H
#define BINDINGTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "rom_jt_154.h"
#include "nwk.h"
#include "assoc_list.h"
#include "ti_zstack_config.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define MAX_DEVICE_PAIRS 255  // temp value

#define DSTGROUPMODE_ADDR     0
#define DSTGROUPMODE_GROUP    1

// this value is built into the libraries, so it is not
// configurable in the release projects
#define MAX_BINDING_CLUSTER_IDS 4

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint16_t numRecs;
} nvBindingHdr_t;

// Don't use sizeof( BindingEntry_t ) use gBIND_REC_SIZE when calculating
// the size of each binding table entry. gBIND_REC_SIZE is defined in nwk_global.c.
typedef struct
{
                        // No src address since the src is always the local device
  uint8_t srcEP;
  uint8_t dstGroupMode;   // Destination address type; 0 - Normal address index, 1 - Group address
  uint16_t dstIdx;        // This field is used in both modes (group and non-group) to save NV and RAM space
                        // dstGroupMode = 0 - Address Manager index
                        // dstGroupMode = 1 - Group Address
  uint8_t dstEP;
  uint8_t numClusterIds;
  uint16_t clusterIdList[MAX_BINDING_CLUSTER_IDS];
                      // Don't use MAX_BINDING_CLUSTERS_ID when
                      // using the clusterIdList field.  Use
                      // gMAX_BINDING_CLUSTER_IDS
} BindingEntry_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

// BindingTable is defined in nwk_globals.c and NWK_MAX_BINDING_ENTRIES
// is defined in f8wConfig.cfg. Don't use NWK_MAX_BINDING_ENTRIES as the
// number of records - use gNWK_MAX_BINDING_ENTRIES.
extern BindingEntry_t BindingTable[];

/*********************************************************************
 * FUNCTIONS
 */

/*
 * This function is used to initialise the binding table
 */
extern void InitBindingTable( void );

/*
 * Removes a binding table entry.
 */
extern byte bindRemoveEntry( BindingEntry_t *pBind );

/*
 * Is the clusterID in the clusterID list?
 */
extern byte bindIsClusterIDinList( BindingEntry_t *entry, uint16_t clusterId );

/*
 * Removes a ClusterID from a list of ClusterIDs.
 */
extern byte bindRemoveClusterIdFromList( BindingEntry_t *entry, uint16_t clusterId );

/*
 * Adds a ClusterID to a list of ClusterIDs.
 */
extern byte bindAddClusterIdToList( BindingEntry_t *entry, uint16_t clusterId );

/*
 * Finds an existing src/epint to dst/epint bind record
 */
extern BindingEntry_t *bindFindExisting( byte srcEpInt,
                                     zAddrType_t *dstShortAddr, byte dstEpInt );

/*
 *  Remove bind(s) associated to a address (source or destination)
 */
extern void bindRemoveDev( zAddrType_t *shortAddr);

/*
 *  Remove bind(s) associated to a address (source)
 */
extern void bindRemoveSrcDev( uint8_t ep );

/*
 * Calculate the number items this device is bound to.
 */
extern byte bindNumBoundTo( zAddrType_t *devAddr, byte devEpInt, byte srcMode );

/*
 * Count the number of reflections.
 */
extern uint16_t bindNumReflections( uint8_t ep, uint16_t clusterID );

/*
 * Finds the binding entry for the source address,
 * endpoint and clusterID passed in as a parameter.
 */
extern BindingEntry_t *bindFind( uint8_t ep, uint16_t clusterID, uint8_t skipping );

/*
 * Lookup a binding entry by specific Idx, if none is found
 * clears the BINDING user from Address Manager.
 */
extern void bindAddressClear( uint16_t dstIdx );

/*
 * Processes the Hand Binding Timeout.
 */
extern void nwk_HandBindingTimeout( void );

/*
 * Initialize Binding Table NV Item
 */
extern byte BindInitNV( void );

/*
 * Initialize Binding Table NV Item
 */
extern void BindSetDefaultNV( void );

/*
 * Restore Binding Table from NV
 */
extern uint16_t BindRestoreFromNV( void );

/*
 * Write Binding Table out to NV
 */
extern void BindWriteNV( void );

/*
 * Update network address in binding table
 */
extern void bindUpdateAddr( uint16_t oldAddr, uint16_t newAddr );

/*
 * This function is used to Add an entry to the binding table
 */
extern BindingEntry_t *bindAddEntry( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16_t *clusterIds );

/*
 * This function returns the number of binding table entries
 */
extern uint16_t bindNumOfEntries( void );

/*
 *  This function returns the number of binding entries
 *          possible and used.
 */
extern void bindCapacity( uint16_t *maxEntries, uint16_t *usedEntries );


/*
 *  This function returns the bind address index
 */
extern uint16_t bindAddrIndexGet( zAddrType_t* addr );

/*********************************************************************
 * FUNCTION POINTERS
 */

/*
 * This function is used to Add an entry to the binding table
 */
extern BindingEntry_t *(*pbindAddEntry)( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16_t *clusterIds );

/*
 * This function returns the number of binding table entries
 */
extern uint16_t (*pbindNumOfEntries)( void );

/*
 *  Remove bind(s) associated to a address (source or destination)
 */
extern void (*pbindRemoveDev)( zAddrType_t *Addr );

/*
 * Initialize Binding Table NV Item
 */
extern byte (*pBindInitNV)( void );

/*
 * Initialize Binding Table NV Item
 */
extern void (*pBindSetDefaultNV)( void );

/*
 *  Restore binding table from NV
 */
extern uint16_t (*pBindRestoreFromNV)( void );

/*
 *  Write binding table to NV
 */
extern void (*pBindWriteNV)( void );

/*
 * Convert address manager index to zAddrType_t for an extended address
 */
extern uint8_t bindingAddrMgsHelperConvert( uint16_t idx, zAddrType_t *addr );

/*
 * Convert address manager index to short address
 */
extern uint16_t bindingAddrMgsHelperConvertShort( uint16_t idx );

/*
 * Get a pointer to the Nth valid binding table entry.
 */
extern BindingEntry_t *GetBindingTableEntry( uint16_t Nth );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* BINDINGTABLE_H */


