/**************************************************************************************************
  Filename:       BindingTable.c
  Revised:        $Date: 2014-10-08 08:37:03 -0700 (Wed, 08 Oct 2014) $
  Revision:       $Revision: 40512 $

  Description:    Device binding table functions.


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

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "osal_nv.h"
#include "nwk_globals.h"
#include "addr_mgr.h"
#include "binding_table.h"
#include "nwk_util.h"
#include "bdb.h"
#ifdef BDB_REPORTING
#include "bdb_reporting.h"
#endif

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define NV_BIND_EMPTY   0xFF
#define NV_BIND_REC_SIZE (gBIND_REC_SIZE)
#define NV_BIND_ITEM_SIZE  (gBIND_REC_SIZE * gNWK_MAX_BINDING_ENTRIES)

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint8_t        srcEP;
  uint16_t       dstIndex;
  uint8_t        dstEP;
  uint8_t        dstAddrMode;
  uint8_t        clusterIDs;
  uint16_t*      clusterIDList;
} bindFields_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
extern bdbGCB_BindNotification_t        pfnBindNotificationCB;
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void BindAddrMgrCB( uint8_t update, AddrMgrEntry_t *entryOld,
                    AddrMgrEntry_t *entryNew );
BindingEntry_t *bindFindEmpty(  bindTableIndex_t* bindTableIndex  );
uint16_t bindingAddrMgsHelperFind( zAddrType_t *addr );
uint8_t bindingAddrMgsHelperConvert( uint16_t idx, zAddrType_t *addr );
void bindAddrMgrLocalLoad( void );


/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8_t bindAddrMgrLocalLoaded = FALSE;

/*********************************************************************
 * Function Pointers
 */

BindingEntry_t *(*pbindAddEntry)( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt,
                                  byte numClusterIds, uint16_t *clusterIds ) = (void*)NULL;
uint16_t (*pbindNumOfEntries)( void ) = (void*)NULL;
void (*pbindRemoveDev)( zAddrType_t *Addr ) = (void*)NULL;
byte (*pBindInitNV)( void ) = (void*)NULL;
void (*pBindSetDefaultNV)( void ) = (void*)NULL;
uint16_t (*pBindRestoreFromNV)( void ) = (void*)NULL;
void (*pBindWriteNV)( void ) = (void*)NULL;

#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
/*********************************************************************
 * @fn      BindAddrMgrCB()
 *
 * @brief   Address Manager Callback function
 *
 * @param   update -
 * @param   entry -
 *
 * @return  pointer to
 */
void BindAddrMgrCB( uint8_t update, AddrMgrEntry_t *entryNew,
                    AddrMgrEntry_t *entryOld )
{
  // Check for either deleted items or changed Extended (Duplicate) addresses
}
#endif // ( ADDRMGR_CALLBACK_ENABLED == 1 )

/*********************************************************************
 * @fn      InitBindingTable()
 *
 * @brief
 *
 *   This function is used to initialise the binding table
 *
 * @param   none
 *
 * @return  none
 */
void InitBindingTable( void )
{
  memset( BindingTable, 0xFF, gBIND_REC_SIZE * gNWK_MAX_BINDING_ENTRIES );

  pbindAddEntry = bindAddEntry;
  pbindNumOfEntries = bindNumOfEntries;
  pbindRemoveDev = bindRemoveDev;
  pBindInitNV = BindInitNV;
  pBindSetDefaultNV = BindSetDefaultNV;
  pBindRestoreFromNV = BindRestoreFromNV;
  pBindWriteNV = BindWriteNV;

  bindAddrMgrLocalLoaded = FALSE;

#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
  // Register with the address manager
  AddrMgrRegister( ADDRMGR_REG_BINDING, BindAddrMgrCB );
#endif
}

/*********************************************************************
 * @fn      bindFindEmpty()
 *
 * @brief   This function returns a pointer to an empty binding slot
 *
 * @param   Pointer to return the index of the entry found
 *
 * @return  pointer to binding table entry, NULL if not added
 */
BindingEntry_t *bindFindEmpty( bindTableIndex_t* bindTableIndex  )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // It's empty if the index is "Not Found"
    if ( BindingTable[x].srcEP == NV_BIND_EMPTY )
    {
      *bindTableIndex = x;
      return ( &BindingTable[x] );
    }
  }

#if ( NWK_MAX_BINDING_ENTRIES > 255 ) || defined ( LARGE_BINDING_TABLE )
  *bindTableIndex = 0xFFFF;
#else
  *bindTableIndex = 0xFF;
#endif


  return ( (BindingEntry_t *)NULL );
}

/*********************************************************************
 * @fn      bindNumOfEntries()
 *
 * @brief   This function returns the number of binding table entries.
 *          The return for this is the number of clusters in the
 *          table NOT the number of entries.
 *
 * @param   none
 *
 * @return  number of entries
 */
uint16_t bindNumOfEntries( void )
{
  bindTableIndex_t x;
  uint16_t found;

  for ( found = 0, x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // It's empty if the index is "Not Found"
    if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
    {
      found += BindingTable[x].numClusterIds;
    }
  }

  return ( found );
}

/*********************************************************************
 * @fn      bindCapacity()
 *
 * @brief   This function returns the number of binding entries
 *          possible and used.
 *
 * @param   maxEntries - pointer to a place to put the max entries
 * @param   usedEntries - pointer to a place to put the number
 *               of used entries
 *
 * @return  none
 */
void bindCapacity( uint16_t *maxEntries, uint16_t *usedEntries  )
{
  bindTableIndex_t x;
  bindTableIndex_t used;

  for ( used = 0, x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // It's empty if the index is "Not Found"
    if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
    {
      used++;
    }
  }

  *maxEntries = gNWK_MAX_BINDING_ENTRIES;
  *usedEntries = used;
}

/*********************************************************************
 * @fn      bindAddEntry()
 *
 * @brief   This function is used to Add an entry to the binding table
 *
 * @param       srcEpInt - source endpoint
 * @param       dstAddr - destination Address
 * @param       dstEpInt - destination endpoint
 * @param       numClusterIds - number of cluster Ids in the list
 * @param       clusterIds - pointer to the Object ID list
 *
 * @return  pointer to binding table entry, NULL if not added
 */
BindingEntry_t *bindAddEntry( byte srcEpInt,
                              zAddrType_t *dstAddr, byte dstEpInt,
                              byte numClusterIds, uint16_t *clusterIds )
{
  uint8_t            index;
  bindTableIndex_t bindIdx;
  BindingEntry_t*  entry;
  bindFields_t     fields;
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  bdbBindNotificationData_t bindData;
#endif
#ifdef BDB_REPORTING
  uint8_t bindAdded = FALSE;
#endif

  //Zigbee Spec 2.2.4.3.1.1
  //Cannot create an endpoint for invalid endpoint index, neither for non-Group
  //or Non-Extended IEEE Address modes
  if( (dstAddr->addrMode != AddrGroup) && (dstAddr->addrMode != Addr64Bit) ||
      (srcEpInt == 0) || (srcEpInt == 0xFF) )
  {
    return NULL;
  }
  //Do not accept neither binds to IEEE Addr with invalid endpoints
  if(( dstAddr->addrMode == Addr64Bit ) && (dstEpInt == 0))
  {
    return NULL;
  }

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
  OsalPort_memcpy( &bindData.dstAddr, dstAddr, sizeof( zAddrType_t) );
  bindData.ep = dstEpInt;
#endif

  // initialize results
  entry = NULL;

  // make sure local addresses have been loaded
  bindAddrMgrLocalLoad();

  // setup fields
  fields.dstIndex = bindAddrIndexGet( dstAddr );
  fields.srcEP    = srcEpInt;

  if ( dstAddr->addrMode == AddrGroup )
  {
    fields.dstAddrMode = DSTGROUPMODE_GROUP;
    fields.dstEP       = 0;
  }
  else
  {
    fields.dstAddrMode = DSTGROUPMODE_ADDR;
    fields.dstEP       = dstEpInt;
  }

  if ( fields.dstIndex != INVALID_NODE_ADDR  )
  {
    for ( bindIdx = 0; bindIdx < gNWK_MAX_BINDING_ENTRIES; bindIdx++ )
    {
      if ( ( fields.srcEP       == BindingTable[bindIdx].srcEP        ) &&
           ( fields.dstAddrMode == BindingTable[bindIdx].dstGroupMode ) &&
           ( fields.dstIndex    == BindingTable[bindIdx].dstIdx       ) &&
           ( fields.dstEP       == BindingTable[bindIdx].dstEP        )    )
      {
        entry = &BindingTable[bindIdx];

        // break from loop
        break;
      }
    }

    if ( entry != NULL )
    {
      // Loop through the cluster IDs
      for ( index = 0; index < numClusterIds; index++ )
      {
        // Found - is the cluster already defined?
        if ( bindIsClusterIDinList( entry, clusterIds[index] ) == FALSE )
        {
          // Nope, add this cluster
          if ( bindAddClusterIdToList( entry, clusterIds[index] ) == FALSE )
          {
            // Indicate error if cluster list was full
            entry = NULL;
          }
          else
          {
            // Save the record to NV
            osal_nv_write_ex( ZCD_NV_EX_BINDING_TABLE, bindIdx,
                           (uint16_t)NV_BIND_REC_SIZE, &BindingTable[bindIdx] );

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
            // new bind added - notify application
            bindData.clusterId = clusterIds[index];

            if ( pfnBindNotificationCB != NULL )
            {
              pfnBindNotificationCB( &bindData );
            }
#endif
#ifdef BDB_REPORTING
            bdb_RepMarkHasBindingInEndpointClusterArray(srcEpInt, clusterIds[index], BDBREPORTING_FALSE, BDBREPORTING_TRUE);
            bindAdded = TRUE;
#endif
          }
        }
      }
    }
    else
    {
      uint8_t  bindTableIndex;
      // Find an empty slot
      entry = bindFindEmpty(&bindTableIndex);

      // Check against the maximum number allowed
      if ( entry != NULL )
      {
        // Add new entry
        entry->srcEP         = fields.srcEP;
        entry->dstGroupMode  = fields.dstAddrMode;
        entry->dstIdx        = fields.dstIndex;
        entry->dstEP         = fields.dstEP;

        if ( numClusterIds > gMAX_BINDING_CLUSTER_IDS )
        {
          numClusterIds = gMAX_BINDING_CLUSTER_IDS;
        }

        for(index = 0; index < numClusterIds; index++)
        {
#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED==1)
          // new bind added - notify application
          bindData.clusterId = clusterIds[index];
          if ( pfnBindNotificationCB != NULL )
          {
            pfnBindNotificationCB( &bindData );
          }
#endif
#ifdef BDB_REPORTING
            bdb_RepMarkHasBindingInEndpointClusterArray(srcEpInt, clusterIds[index], BDBREPORTING_FALSE, BDBREPORTING_TRUE);
            bindAdded = TRUE;
#endif
        }

        entry->numClusterIds = numClusterIds;

        OsalPort_memcpy( entry->clusterIdList,
                     clusterIds,
                     numClusterIds * sizeof(uint16_t) );

        // Save the record to NV
        osal_nv_write_ex( ZCD_NV_EX_BINDING_TABLE, bindTableIndex,
                         (uint16_t)NV_BIND_REC_SIZE, &BindingTable[bindTableIndex] );
      }
    }
  }
#ifdef BDB_REPORTING
  if(bindAdded == TRUE)
  {
    bdb_RepStartOrContinueReporting( );
  }
#endif
  return entry;
}

/*********************************************************************
 * @fn      bindRemoveEntry
 *
 * @brief   Removes a binding table entry.
 *
 * @param   pBind - pointer to binding table entry to delete
 *
 * @return  TRUE if Removed, FALSE if not
 */
byte bindRemoveEntry( BindingEntry_t *pBind )
{
  memset( pBind, 0xFF, gBIND_REC_SIZE );
#ifdef BDB_REPORTING
  bdb_RepUpdateMarkBindings();
#endif
  return ( TRUE );
}

/*********************************************************************
 * @fn      bindIsClusterIDinList()
 *
 * @brief   Is the clusterID in the clusterID list?
 *
 * @param   enter - binding table entry
 * @param   clusterId  - Cluster ID to look for
 *
 * @return  TRUE if found, FALSE if not found
 */
byte bindIsClusterIDinList( BindingEntry_t *entry, uint16_t clusterId )
{
  uint8_t x;

  if ( entry != NULL )
  {
    for ( x = 0; x < entry->numClusterIds; x++ )
    {
      if ( entry->clusterIdList[x] == clusterId )
      {
        return ( TRUE );
      }
    }
  }

  return ( FALSE );
}

/*********************************************************************
 * @fn      bindRemoveClusterIdFromList()
 *
 * @brief   Removes a ClusterID from a list of ClusterIDs.
 *
 * @param   enter - binding table entry
 * @param   clusterId  - Cluster ID to look for
 *
 * @return  TRUE if there are at least 1 clusterID left, FALSE if none
 */
byte bindRemoveClusterIdFromList( BindingEntry_t *entry, uint16_t clusterId )
{
  byte x;
  uint16_t *listPtr;
  byte numIds;

#ifdef BDB_REPORTING
  uint8_t numRemoved = 0;
#endif
  if ( entry )
  {
    if ( entry->numClusterIds > 0 )
    {
      listPtr = entry->clusterIdList;
      numIds = entry->numClusterIds;

      // Copy the new list over
      for ( x = 0; x < numIds; x++ )
      {
        if ( entry->clusterIdList[x] != clusterId )
        {
          *listPtr++ = entry->clusterIdList[x];
        }
        else
        {
          entry->numClusterIds--;

#ifdef BDB_REPORTING
           numRemoved++;
#endif
          if ( entry->numClusterIds == 0 )
          {
            break;
          }
        }
      }

    }
  }

#ifdef BDB_REPORTING
  if(numRemoved>0)
    bdb_RepUpdateMarkBindings();
#endif

  if ( entry && (entry->numClusterIds > 0) )
  {
    return ( TRUE );
  }
  else
  {
    return ( FALSE );
  }
}

/*********************************************************************
 * @fn      bindAddClusterIdToList()
 *
 * @brief   Adds a ClusterID to a list of ClusterIDs.
 *
 * @param   enter - binding table entry
 * @param   clusterId  - Cluster ID to Add
 *
 * @return  TRUE if Added, FALSE if not
 */
byte bindAddClusterIdToList( BindingEntry_t *entry, uint16_t clusterId )
{
  if ( entry && entry->numClusterIds < gMAX_BINDING_CLUSTER_IDS )
  {
    // Add the new one
    entry->clusterIdList[entry->numClusterIds] = clusterId;
    entry->numClusterIds++;
    return ( TRUE );
  }
  return ( FALSE );
}

/*********************************************************************
 * @fn      bindFindExisting
 *
 * @brief   Finds an existing src/epint to dst/epint bind record
 *
 * @param   srcEpInt - Source Endpoint/Interface
 * @param   dstAddr - Destination address
 * @param   dstEpInt - Destination Endpoint/Interface
 *
 * @return  pointer to existing entry or NULL
 */
BindingEntry_t *bindFindExisting( byte srcEpInt,
                                  zAddrType_t *dstAddr, byte dstEpInt )
{
  uint16_t dstIdx;
  bindTableIndex_t x;

  // Find the records in the assoc list
  if ( dstAddr->addrMode == AddrGroup )
  {
    dstIdx = dstAddr->addr.shortAddr;
  }
  else
  {
    dstIdx = bindingAddrMgsHelperFind( dstAddr );
  }

  if ( dstIdx == INVALID_NODE_ADDR )
  {
    return ( (BindingEntry_t *)NULL );
  }

  // Start at the beginning
  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( BindingTable[x].srcEP == srcEpInt )
    {
      if ( ((dstAddr->addrMode == AddrGroup)
              && (BindingTable[x].dstGroupMode == DSTGROUPMODE_GROUP)
              && (dstIdx == BindingTable[x].dstIdx))
         || ((dstAddr->addrMode != AddrGroup)
             && (BindingTable[x].dstGroupMode == DSTGROUPMODE_ADDR)
             && (dstIdx == BindingTable[x].dstIdx) && (BindingTable[x].dstEP == dstEpInt)) )
      {
        return ( &BindingTable[x] );
      }
    }
  }

  return ( (BindingEntry_t *)NULL );
}

/*********************************************************************
 * @fn       bindRemoveDev()
 *
 * @brief
 *
 *   Remove all bind(s) entries associated to a device address (destination).
 *   Updates binding table.
 *
 * @param   Addr - address of device to be removed from Binding Table
 *
 * @return  none
 */
void bindRemoveDev( zAddrType_t *Addr )
{
  uint16_t idx;
  bindTableIndex_t x;

  if ( Addr->addrMode == AddrGroup )
  {
    idx = Addr->addr.shortAddr;
  }
  else
  {
    idx = bindingAddrMgsHelperFind( Addr );
  }

  if ( idx == INVALID_NODE_ADDR )
  {
    return;
  }

  // Removes all the entries that match the destination Address/Index
  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( ( (Addr->addrMode == AddrGroup) && (BindingTable[x].dstGroupMode == DSTGROUPMODE_GROUP)
                                         && (BindingTable[x].dstIdx == idx) ) ||
         ( (Addr->addrMode != AddrGroup) && (BindingTable[x].dstGroupMode == DSTGROUPMODE_ADDR)
                                         && (BindingTable[x].dstIdx == idx) ) )
    {
      bindRemoveEntry( &BindingTable[x] );
    }
  }

  // If this is the last Bind Entry for that idx then clear BINDING
  // user from Address Manager
  bindAddressClear( idx );
}

/*********************************************************************
 * @fn       bindRemoveSrcDev()
 *
 * @brief
 *
 *   Remove binds(s) associated to device address (source).
 *   Updates binding table.
 *
 * @param   ep - endpoint to remove, 0xFF is all endpoints
 *
 * @return  none
 */
void bindRemoveSrcDev( uint8_t ep )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( (ep == 0xFF) || (ep == BindingTable[x].srcEP) )
    {
      uint16_t idx = BindingTable[x].dstIdx;

      bindRemoveEntry( &BindingTable[x] );

      // If this is the last Bind Entry for that idx then clear BINDING
      // user from Address Manager
      bindAddressClear( idx );
    }
  }
}

/*********************************************************************
 * @fn          bindNumBoundTo
 *
 * @brief       Calculate the number items this device is bound to.
 *              When srcMode is set to TRUE, discard what value devAddr
 *              has, it returns number count bound to the local dev.
 *
 * @param       devAddr - device Address
 * @param       devEP - endpoint
 * @param       srcMode - TRUE - assume devHandle is a source address
 *                        FALSE - destination address
 *
 * @return      status
 */
byte bindNumBoundTo( zAddrType_t *devAddr, byte devEpInt, byte srcMode )
{
  BindingEntry_t *pBind;
  uint16_t idx;
  byte   num;
  bindTableIndex_t x;

  // Init
  num = 0;

  if ( devAddr->addrMode == AddrGroup )
  {
    idx = devAddr->addr.shortAddr;
  }
  else
  {
    idx = bindingAddrMgsHelperFind( devAddr );
  }

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];
    if ( srcMode )
    {
      if ( pBind->srcEP == devEpInt )
      {
        num++;
      }
    }
    else
    {
      if ( ((devAddr->addrMode == AddrGroup)
              && (pBind->dstGroupMode == DSTGROUPMODE_GROUP) && (pBind->dstIdx == idx))
          || ((devAddr->addrMode != AddrGroup) && (pBind->dstGroupMode == DSTGROUPMODE_ADDR)
                                && (pBind->dstIdx == idx) && (pBind->dstEP == devEpInt)) )
      {
        num++;
      }
    }
  }

  return num;
}

/*********************************************************************
 * @fn          bindNumReflections
 *
 * @brief       Counts the number of reflections needed for a
 *              endpoint and cluster ID combo.
 *
 * @param       ep - source endpoint
 * @param       clusterID - matching clusterID
 *
 * @return      number of reflections needed.
 */
uint16_t bindNumReflections( uint8_t ep, uint16_t clusterID )
{
  bindTableIndex_t x;
  BindingEntry_t *pBind;
  uint16_t cnt = 0;
  uint8_t bindEP;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];
    bindEP = pBind->srcEP;

    if ( (bindEP == ep) && (bindIsClusterIDinList( pBind, clusterID )) )
    {
      cnt++;
    }
  }

  return ( cnt );
}

/*********************************************************************
 * @fn          bindFind
 *
 * @brief       Finds the binding entry for the source address, endpoint
 *              and cluster ID passed in as a parameter.
 *
 * @param       ep - source endpoint
 * @param       clusterID - matching clusterID
 * @param       skip - number of matches to skip before returning
 *
 * @return      pointer to the binding table entry, NULL if not found
 */
BindingEntry_t *bindFind( uint8_t ep, uint16_t clusterID, uint8_t skipping )
{
  BindingEntry_t *pBind;
  byte skipped = 0;
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];

    if ( ( pBind->srcEP == ep) && bindIsClusterIDinList( pBind, clusterID ))
    {
      if ( skipped < skipping )
      {
        skipped++;
      }
      else
      {
        return ( pBind );
      }
    }
  }

  return ( (BindingEntry_t *)NULL );
}

/*********************************************************************
 * @fn      bindAddressClear
 *
 * @brief   Lookup a binding entry by specific Idx, if none is found
 *          clears the BINDING user from Address Manager.
 *
 * @param   dstIdx - Source Address Manager Index
 *
 * @return  none
 */
void bindAddressClear( uint16_t dstIdx )
{
  bindTableIndex_t i;

  if ( dstIdx != INVALID_NODE_ADDR )
  {
    // Looks for a specific Idx
    for ( i = 0; i < gNWK_MAX_BINDING_ENTRIES; i++ )
    {
      if ( ( BindingTable[i].dstGroupMode != AddrGroup ) &&
           ( BindingTable[i].dstGroupMode == DSTGROUPMODE_ADDR ) &&
           ( BindingTable[i].dstIdx == dstIdx ) )
      {
        break;  // found at least one
      }
    }

    if ( i == gNWK_MAX_BINDING_ENTRIES )
    {
      // No binding entry is associated with dstIdx.
      // Remove user binding bit from the address manager entry corresponding to dstIdx.
      AddrMgrEntry_t addrEntry;

      addrEntry.user = ADDRMGR_USER_BINDING;
      addrEntry.index = dstIdx;

      AddrMgrEntryRelease( &addrEntry );
    }
  }
}

/*********************************************************************
 * @fn          bindUpdateAddr
 *
 * @brief       Update the network address in the binding table.
 *
 * @param       oldAddr - old network address
 * @param       newAddr - new network address
 *
 * @return      none
 */
void bindUpdateAddr( uint16_t oldAddr, uint16_t newAddr )
{
  uint16_t oldIdx;
  uint16_t newIdx;
  zAddrType_t addr;
  bindTableIndex_t x;
  BindingEntry_t *pBind;

  addr.addrMode = Addr16Bit;
  addr.addr.shortAddr = oldAddr;
  oldIdx = bindingAddrMgsHelperFind( &addr );
  addr.addr.shortAddr = newAddr;
  newIdx = bindingAddrMgsHelperFind( &addr );

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    pBind = &BindingTable[x];

    if ( pBind->dstIdx == oldIdx )
    {
      pBind->dstIdx = newIdx;
    }
  }
}

/*********************************************************************
 * @fn      bindingAddrMgsHelperFind
 *
 * @brief   Turns an zAddrType_t to an Addr Manager index
 *
 * @param   addr - zAddrType_t
 *
 * @return  INVALID_NODE_ADDR if not found, otherwise an index
 */
uint16_t bindingAddrMgsHelperFind( zAddrType_t *addr )
{
  AddrMgrEntry_t entry;

  // Resolve addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;
  if ( addr->addrMode == Addr16Bit )
  {
    entry.nwkAddr = addr->addr.shortAddr;
    AddrMgrEntryLookupNwk( &entry );
  }
  else
  {
    AddrMgrExtAddrSet( entry.extAddr, addr->addr.extAddr );
    AddrMgrEntryLookupExt( &entry );
  }

  return ( entry.index );
}

/*********************************************************************
 * @fn      bindingAddrMgsHelperConvert
 *
 * @brief   Convert an index into an zAddrType_t
 *
 * @param   idx -
 * @param   addr - zAddrType_t
 *
 * @return  TRUE if found, FALSE if not
 */
uint8_t bindingAddrMgsHelperConvert( uint16_t idx, zAddrType_t *addr )
{
  AddrMgrEntry_t entry;
  uint8_t stat;

  // Resolve addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;
  entry.index = idx;
  stat = AddrMgrEntryGet( &entry );
  if ( stat )
  {
    addr->addrMode = Addr64Bit;
    osal_cpyExtAddr( addr->addr.extAddr, entry.extAddr );
  }

  return ( stat );
}

/*********************************************************************
 * @fn      bindingAddrMgsHelperConvertShort
 *
 * @brief   Convert an index into a short address
 *
 * @param   idx -
 *
 * @return  INVALID_NODE_ADDR if not available, otherwise the short address
 */
uint16_t bindingAddrMgsHelperConvertShort( uint16_t idx )
{
  AddrMgrEntry_t entry;

  // Resolve addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;
  entry.index = idx;
  AddrMgrEntryGet( &entry );

  return ( entry.nwkAddr );
}

/*********************************************************************
 * @fn      bindAddrMgrLocalLoad
 *
 * @brief   Load local(self and parent) address information into
 *          Address Manager
 *
 * @param   none
 *
 * @return  none
 */
void bindAddrMgrLocalLoad( void )
{
  AddrMgrEntry_t entry;
  uint16_t         parent;

  // add "local"(self and parent) address informtion into the Address
  // Manager
  if ( bindAddrMgrLocalLoaded == FALSE )
  {
    // add the device's address information
    entry.user    = ADDRMGR_USER_BINDING;
    entry.nwkAddr = _NIB.nwkDevAddress;
    AddrMgrExtAddrSet( entry.extAddr, NLME_GetExtAddr() );
    AddrMgrEntryUpdate( &entry );

    // make sure parent address is valid
    parent = NLME_GetCoordShortAddr();
    if ( ( parent != entry.nwkAddr     ) &&
         ( parent != INVALID_NODE_ADDR )    )
    {
      // add the parent's address information
      entry.nwkAddr = parent;
      NLME_GetCoordExtAddr( entry.extAddr );
      AddrMgrEntryUpdate( &entry );
    }

    bindAddrMgrLocalLoaded = TRUE;
  }
}

/*********************************************************************
 * @fn      bindAddrIndexGet
 *
 * @brief   Get bind address index.
 *
 * @param   addr - <zAddrType_t>
 *
 * @return  (uint16_t) address index
 */
uint16_t bindAddrIndexGet( zAddrType_t* addr )
{
  AddrMgrEntry_t entry;
  uint8_t          update;

  update = FALSE;

  // sync binding addresses with the address manager
  entry.user = ADDRMGR_USER_BINDING;

  if ( addr->addrMode == Addr16Bit )
  {
    entry.nwkAddr = addr->addr.shortAddr;

    if ( AddrMgrEntryLookupNwk( &entry ) == FALSE )
    {
      update = TRUE;
    }
  }
  else if ( addr->addrMode == Addr64Bit )
  {
    AddrMgrExtAddrSet( entry.extAddr, addr->addr.extAddr );

    if ( AddrMgrEntryLookupExt( &entry ) == FALSE )
    {
      update = TRUE;
    }
  }
  else if ( addr->addrMode == AddrGroup )
  {
    entry.index = addr->addr.shortAddr;
  }
  else
  {
    entry.index = INVALID_NODE_ADDR;
  }

  if ( update )
  {
    AddrMgrEntryUpdate( &entry );
  }

  return entry.index;
}

/*********************************************************************
 * @fn      GetBindingTableEntry
 *
 * @brief   Get a pointer to the Nth valid binding table entry.
 *
 * @param   Nth valid entry being requested.
 *
 * @return  The Nth valid binding table entry.
 */
BindingEntry_t *GetBindingTableEntry( uint16_t Nth )
{
  BindingEntry_t *rtrn = NULL;

  bindTableIndex_t idx, cnt = 0;

  for ( idx = 0; idx < gNWK_MAX_BINDING_ENTRIES; idx++ )
  {
    if ( BindingTable[idx].srcEP != NV_BIND_EMPTY )
    {
      if ( cnt++ == Nth )
      {
        rtrn = BindingTable+idx;
        break;
      }
    }
  }

  return rtrn;
}

/*********************************************************************
 * @fn          BindInitNV
 *
 * @brief       Initialize the Binding NV Item
 *
 * @param       none
 *
 * @return      ZSUCCESS if successful, NV_ITEM_UNINIT if item did not
 *              exist in NV, NV_OPER_FAILED if failure.
 */
byte BindInitNV( void )
{
  uint8_t *pDefaultBuf;
  bindTableIndex_t x;

  pDefaultBuf = (uint8_t *)OsalPort_malloc(NV_BIND_REC_SIZE);
  if (pDefaultBuf)
  {
      memset( pDefaultBuf, 0xFF, NV_BIND_REC_SIZE );

      for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
      {
        // Initialize each binding record
        osal_nv_item_init_ex( ZCD_NV_EX_BINDING_TABLE, x, NV_BIND_REC_SIZE, pDefaultBuf );
      }

      OsalPort_free(pDefaultBuf);
  }

  return ( ZSUCCESS );
}

/*********************************************************************
 * @fn          BindSetDefaultNV
 *
 * @brief       Write the defaults to NV
 *
 * @param       none
 *
 * @return      none
 */
void BindSetDefaultNV( void )
{
  BindingEntry_t bind;
  bindTableIndex_t x;

  // Initialize a binding record
  memset( &bind, 0xFF, sizeof ( BindingEntry_t ) );

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // Over write each binding record with an "empty" record
    osal_nv_write_ex( ZCD_NV_EX_BINDING_TABLE, x, NV_BIND_REC_SIZE, &bind );
  }
}

/*********************************************************************
 * @fn          BindRestoreFromNV
 *
 * @brief       Restore the binding table from NV
 *
 * @param       none
 *
 * @return      Number of entries restored (non-emtpy)
 */
uint16_t BindRestoreFromNV( void )
{
  bindTableIndex_t x;
  uint16_t validRecsCount = 0;

  // Read in the device list
  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    if ( osal_nv_read_ex( ZCD_NV_EX_BINDING_TABLE, x, 0,
                     (uint16_t)NV_BIND_REC_SIZE, &BindingTable[x] ) == ZSUCCESS )
    {
      // Check for non-empty record
      if ( BindingTable[x].srcEP != NV_BIND_EMPTY )
      {
        // Count non-empty records
        validRecsCount++;
      }
    }
  }
  return ( validRecsCount );
}

/*********************************************************************
 * @fn          BindWriteNV
 *
 * @brief       Copy the Binding Table in NV
 *
 * @param       none
 *
 * @return      none
 */
void BindWriteNV( void )
{
  bindTableIndex_t x;

  for ( x = 0; x < gNWK_MAX_BINDING_ENTRIES; x++ )
  {
    // Save the record to NV
    osal_nv_write_ex( ZCD_NV_EX_BINDING_TABLE, x,
                     (uint16_t)NV_BIND_REC_SIZE, &BindingTable[x] );
  }
}

/*********************************************************************
*********************************************************************/
