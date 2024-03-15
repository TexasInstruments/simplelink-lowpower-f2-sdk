/**************************************************************************************************
  Filename:       assoc_list.h
  Revised:        $Date: 2015-01-22 13:22:52 -0800 (Thu, 22 Jan 2015) $
  Revision:       $Revision: 41965 $

  Description:    Associated Device List.


  Copyright 2004-2015 Texas Instruments Incorporated.

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

#ifndef ASSOCLIST_H
#define ASSOCLIST_H

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

#define NVINDEX_NOT_FOUND   0xFFFF
#define NVINDEX_THIS_DEVICE 0xFFFE

#define ASSOC_INDEX_NOT_FOUND 0xFFFF

// Bitmap of associated devices status fields
#define DEV_LINK_STATUS     0x01           // link is in-active ?
#define DEV_LINK_REPAIR     0x02           // link repair in progress ?
#define DEV_SEC_INIT_STATUS 0x04           // security init
#define DEV_SEC_AUTH_STATUS 0x08           // security authenticated
#define DEV_SEC_AUTH_TC_REJOIN_STATUS 0x10 // Assume secure device that performs TC rejoin

#define DEV_SECURED_JOIN    0x20 // Device joined secure
#define DEV_REJOIN_STATUS   0x40 // Device rejoined
#define DEV_HIGH_SEC_STATUS 0x80 // Device joined as High Security

// Node Relations
#define PARENT              0
#define CHILD_RFD           1
#define CHILD_RFD_RX_IDLE   2
#define CHILD_FFD           3
#define CHILD_FFD_RX_IDLE   4
#define NEIGHBOR            5
#define OTHER               6
#define NOTUSED             0xFF

// Child Table age out values
#define TIMEOUT_DONT_AGE_OUT    0xFFFFFFFE
#define TIMEOUT_NOT_USED        0xFFFFFFFF

#define ASSOC_LIST_CTRL_SAVE 0x80

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint8_t endDevCfg;
  uint32_t deviceTimeout;
} aging_end_device_t;

typedef struct
{
  uint16_t shortAddr;                 // Short address of associated device
  uint16_t addrIdx;                   // Index from the address manager
  byte nodeRelation;
  byte devStatus;                   // bitmap of various status values
  byte assocCnt;
  byte age;
  linkInfo_t linkInfo;
  aging_end_device_t endDev;
  uint32_t timeoutCounter;
  bool keepaliveRcv;
  uint8_t ctrl;
} associated_devices_t;

typedef struct
{
  uint16_t numRecs;
} nvDeviceListHdr_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
//extern byte _numAssocDev;
extern associated_devices_t AssociatedDevList[];

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Variable initialization
 */
extern void AssocInit( void );

/*
 * Create a new or update a previous association.
 */
extern associated_devices_t *AssocAddNew( uint16_t shortAddr, byte *extAddr,
                                                            byte nodeRelation );

/*
 * Count number of devices.
 */
extern uint16_t AssocCount( byte startRelation, byte endRelation );

/*
 * Check if the device is a child.
 */
extern byte AssocIsChild( uint16_t shortAddr );

/*
 * Check if the device is a reduced funtion child
 */
byte AssocIsRFChild( uint16_t shortAddr );

/*
 * Check if the device is my parent.
 */
extern byte AssocIsParent( uint16_t shortAddr );

/*
 * Search the Device list using shortAddr.
 */
extern associated_devices_t *AssocGetWithShort( uint16_t shortAddr );

/*
 * Search the Device list using extended Address.
 */
extern associated_devices_t *AssocGetWithExt( byte *extAddr );

/*
 * Remove a device from the list. Uses the extended address.
 */
extern byte AssocRemove( byte *extAddr );

/*
 * Returns the next inactive child node
 */
extern uint16_t AssocGetNextInactiveNode( uint16_t shortAddr );

/*
 * Returns the next child node
 */
extern uint16_t AssocGetNextChildNode( uint16_t shortAddr );

/*
 * Remove all devices from the list and reset it
 */
extern void AssocReset( void );

/*
 * AssocMakeList - Make a list of associate devices
 *  NOTE:  this function returns a dynamically allocated buffer
 *         that MUST be deallocated (OsalPort_free).
 */
extern uint16_t *AssocMakeList( byte *pCount );

/*
 * Gets next device that matches the status parameter
 */
extern associated_devices_t *AssocMatchDeviceStatus( uint8_t status );

/*
 * Initialize the Assoc Device List NV Item
 */
extern byte AssocInitNV( void );

/*
 * Set Assoc Device list NV Item to defaults
 */
extern void AssocSetDefaultNV( void );

/*
 * Restore the device list (assoc list) from NV
 */
extern uint8_t AssocRestoreFromNV( void );

/*
 * Save the device list (assoc list) to NV
 */
extern void AssocWriteNV( void );

/*
 * Find Nth active device in list
 */
extern associated_devices_t *AssocFindDevice( uint16_t number );

extern uint8_t AssocChangeNwkAddr( uint16_t nwkAddr, uint8_t *ieeeAddr );

extern void AssocCheckDupNeighbors( void );

extern void AssocChildAging( void );

extern void AssocChildSecurityTimeout( void );

extern uint8_t AssocChildTableUpdateTimeout( uint16_t nwkAddr );

extern void AssocChildTableManagement( OsalPort_EventHdr *inMsg );

extern uint8_t *AssocMakeListOfRfdChild( uint8_t *pCount );

/*********************************************************************
*********************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* ASSOCLIST_H */


