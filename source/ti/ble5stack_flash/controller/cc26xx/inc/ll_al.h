/******************************************************************************

 @file  ll_al.h

 @brief This file contains the data structures and APIs for handling
        Bluetooth Low Energy Accept List structures using the CC26xx
        RF Core Firmware Specification.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2025, Texas Instruments Incorporated

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

 ******************************************************************************
 
 
 *****************************************************************************/

/*********************************************************************
 *
 * WARNING!!!
 *
 * THE API'S FOUND IN THIS FILE ARE FOR INTERNAL STACK USE ONLY!
 * FUNCTIONS SHOULD NOT BE CALLED DIRECTLY FROM APPLICATIONS, AND ANY
 * CALLS TO THESE FUNCTIONS FROM OUTSIDE OF THE STACK MAY RESULT IN
 * UNEXPECTED BEHAVIOR.
 *
 */

#ifndef AL_H
#define AL_H

/*******************************************************************************
 * INCLUDES
 */

#ifndef USE_RCL
#include "rf_hal.h"
#endif
#include "bcomdef.h"
#include "ll.h"
#include "ll_config.h"

/*******************************************************************************
 * MACROS
 */

// Advertising Accept List
// Note: Assumes alEntryFlags = accept list entry's flags.

#define CLR_AL_ENTRY( alEntryFlags )                                           \
  (alEntryFlags) = 0;

#define SET_AL_ENTRY_FREE( alEntryFlags )                                      \
  (alEntryFlags) &= ~BV(0)

#define SET_AL_ENTRY_BUSY( alEntryFlags )                                      \
  (alEntryFlags) |= BV(0)

#define IS_AL_ENTRY_FREE( alEntryFlags )                                       \
  ((alEntryFlags) & BV(0)) == 0

#define IS_AL_ENTRY_BUSY( alEntryFlags )                                       \
  ((alEntryFlags) & BV(0)) == 1

#define SET_AL_ENTRY_PUBLIC( alEntryFlags )                                    \
  (alEntryFlags) &= ~BV(1)

#define SET_AL_ENTRY_RANDOM( alEntryFlags )                                    \
  (alEntryFlags) |= BV(1)

#define GET_AL_ENTRY_ADDR_TYPE( alEntryFlags )                                 \
  (((alEntryFlags) & BV(1)) >> 1)

#define CLR_AL_ENTRY_IGNORE( alEntryFlags )                                    \
  (alEntryFlags) &= ~BV(2)

#define SET_AL_ENTRY_IGNORE( alEntryFlags )                                    \
  (alEntryFlags) |= BV(2)

#ifdef USE_RCL
#define SET_AL_ENTRY_PRIV_IGNORE( alEntryFlags )                               \
  (alEntryFlags) |= BV(3)

#define CLR_AL_ENTRY_PRIV_IGNORE( alEntryFlags )                               \
  (alEntryFlags) &= ~BV(3)

#define GET_AL_TABLE_POINTER( pAlEntry )                                       \
    ((alTable_t *)((uint8 *)(pAlEntry) - sizeof(alTable_t) + sizeof(uint32_t)))

#else
#define SET_AL_ENTRY_PRIV_IGNORE( alEntryFlags )                               \
  (alEntryFlags) |= BV(4)

#define CLR_AL_ENTRY_PRIV_IGNORE( alEntryFlags )                               \
  (alEntryFlags) &= ~BV(4)

#define GET_AL_TABLE_POINTER( pAlEntry )                                       \
    ((alTable_t *)((uint8 *)(pAlEntry) - sizeof(alTable_t)))
#endif

/*******************************************************************************
 * CONSTANTS
 */

// API
#define BLE_IGNORE_AL_ENTRY            0
#define BLE_USE_AL_ENTRY               1

// Miscellaneous
#define BLE_BDADDR_SIZE                6
#define BLE_MAX_NUM_AL_ENTRIES         (alSize)  // at 8 bytes per AL entry
#define BLE_NO_AL_MATCH_FOUND          0xFF

#ifdef CC23X0
#define BLE_MAX_NUM_AL_SCAN_ENTRIES    15
#else
#define BLE_MAX_NUM_AL_SCAN_ENTRIES    BLE_MAX_NUM_AL_ENTRIES
#endif // USE_RCL

#define BLE_NUM_AL_ENTRIES_ZERO        0   // Error return value for number of accept list entries
#define BLE_NUM_AL_ENTRIES             ((BLE_MAX_NUM_AL_ENTRIES) + (2 * (BLE_RESOLVING_LIST_SIZE)) + 1)

/*******************************************************************************
 * TYPEDEFS
 */
#ifdef USE_RCL
// BLE Filter List Flags
// | 15..4 |        3       |        2          |      1       |      0       |
// |  N/A  | Privacy Ignore | Duplicate Ignored | Address Type | Entry In Use |
//
typedef uint16_t alFlgs_t;

// Accept List Entry
// Note: see RCL filter list entry struct (RCL_FL_Entry).
PACKED_TYPEDEF_STRUCT
{
  alFlgs_t alFlags;                    // W:  accept list flags (RW for bit 2)
  uint8    devAddr[BLE_BDADDR_SIZE];   // W:  BLE address
} alEntry_t;

PACKED_TYPEDEF_STRUCT
{
  // LL structure part
  uint8     numAlEntries;
  uint8     numBusyAlEntries;
  uint16    reserve;
  alEntry_t *pAlEntries;
  // RCL structure part - according to the RCL filter list struct (RCL_FilterList)
  uint32    numEntries;
  // all 16 entries located here while pAlEntries will point to them
  // while the RCL filterList will point to the numEntries (start of the RCL_FilterList)
} alTable_t;

#else
// BLE Accept List Flags
// | 7..5 |        4       |  3  |        2         |      1       |      0       |
// |  N/A | Privacy Ignore | N/A | AL Entry Ignored | Address Type | Entry In Use |
//
typedef uint8 alFlgs_t;

// Accept List Entry
// Note: The layout of this structure can not be changed.
PACKED_TYPEDEF_STRUCT
{
  uint8    numEntries;                 // W:  number of accept list entries
  alFlgs_t alFlags;                    // W:  accept list flags (RW for bit 2)
  uint8    devAddr[BLE_BDADDR_SIZE];   // W:  BLE address
} alEntry_t;

// Accept List Entry Table
// Note: The layout of this structure can be changed as long as pAlEntries is
//       aligned on a word boundary.
PACKED_TYPEDEF_STRUCT
{
  uint8     numAlEntries;
  uint8     numBusyAlEntries;
  uint16    reserve;
  alEntry_t *pAlEntries;
} alTable_t;

#endif

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

extern alTable_t *alTable;
extern alTable_t *alTableScan;

/*******************************************************************************
 * GLOBAL ROUTINES
 */

extern void       AL_Init( alTable_t * );

extern void       AL_Scan_Init( alTable_t * );

extern llStatus_t AL_Clear( alTable_t * );

extern void       AL_ClearEntry( alEntry_t * );

extern uint8      AL_GetSize( alTable_t * );

extern uint8      AL_GetNumFreeEntries(  alTable_t * );

extern uint8      AL_FindEntry( alTable_t *, uint8 *, uint8 );

extern llStatus_t AL_AddEntry( alTable_t *, uint8 *, uint8, uint8 );

extern llStatus_t AL_RemoveEntry( alTable_t *, uint8 *, uint8 );

extern llStatus_t AL_SetAlIgnore( alTable_t *, uint8 *, uint8 );

extern llStatus_t AL_ClearIgnoreList( alTable_t * );

extern alEntry_t *AL_Alloc( uint8 );

extern void       AL_Free( alEntry_t * );

extern alEntry_t *AL_Copy( alEntry_t *, alEntry_t * );

/*******************************************************************************
 */

#endif /* AL_H */
