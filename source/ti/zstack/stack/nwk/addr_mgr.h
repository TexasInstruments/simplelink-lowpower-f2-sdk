/**************************************************************************************************
  Filename:       addr_mgr.h
  Revised:        $Date: 2011-05-05 13:52:19 -0700 (Thu, 05 May 2011) $
  Revision:       $Revision: 25878 $

  Description:    This file contains the interface to the Address Manager.


  Copyright 2005-2011 Texas Instruments Incorporated.

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

#ifndef ADDRMGR_H
#define ADDRMGR_H

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcomdef.h"

/******************************************************************************
 * CONSTANTS
 */
// registration IDs - use with <AddrMgrRegister>
#define ADDRMGR_REG_ASSOC    0x00
#define ADDRMGR_REG_SECURITY 0x01
#define ADDRMGR_REG_BINDING  0x02
#define ADDRMGR_REG_PRIVATE1 0x03

// user IDs - use with <AddrMgrEntry_t>
#define ADDRMGR_USER_DEFAULT  0x00
#define ADDRMGR_USER_ASSOC    0x01
#define ADDRMGR_USER_SECURITY 0x02
#define ADDRMGR_USER_BINDING  0x04
#define ADDRMGR_USER_PRIVATE1 0x08

// update types - use with registered callback <AddrMgrUserCB_t>
#define ADDRMGR_ENTRY_NWKADDR_SET 1
#define ADDRMGR_ENTRY_NWKADDR_DUP 2
#define ADDRMGR_ENTRY_EXTADDR_SET 3

// address manager callback feature enable/disable
#define ADDRMGR_CALLBACK_ENABLED 0

/******************************************************************************
 * TYPEDEFS
 */
// entry data
typedef struct
{
  uint8_t  user;
  uint16_t nwkAddr;
  uint8_t  extAddr[Z_EXTADDR_LEN];
  uint16_t index;
} AddrMgrEntry_t;

// user callback set during registration
typedef void (*AddrMgrUserCB_t)( uint8_t           update,
                                 AddrMgrEntry_t* newEntry,
                                 AddrMgrEntry_t* oldEntry );

/******************************************************************************
 * PUBLIC FUNCTIONS
 */
/******************************************************************************
 * @fn          AddrMgrInit
 *
 * @brief       Initialize Address Manager.
 *
 * @param       entryTotal - [in] total number of address entries
 *
 * @return      none
 */
extern void AddrMgrInit( uint16_t entryTotal );

/******************************************************************************
 * @fn          AddrMgrReset
 *
 * @brief       Reset Address Manager.
 *
 * @param       none
 *
 * @return      none
 */
extern void AddrMgrReset( void );

/******************************************************************************
 * @fn          AddrMgrInitNV
 *
 * @brief       Initialize the address entry data in NV.
 *
 * @param       none
 *
 * @return      uint8_t - <osal_nv_item_init> return codes
 */
extern uint8_t AddrMgrInitNV( void );

/******************************************************************************
 * @fn          AddrMgrSetDefaultNV
 *
 * @brief       Set default address entry data in NV.
 *
 * @param       none
 *
 * @return      none
 */
extern void AddrMgrSetDefaultNV( void );

/******************************************************************************
 * @fn          AddrMgrRestoreFromNV
 *
 * @brief       Restore the address entry data from NV.
 *
 * @param       none
 *
 * @return      none
 */
extern void AddrMgrRestoreFromNV( void );

/******************************************************************************
 * @fn          AddrMgrWriteNV
 *
 * @brief       Save the address entry data to NV.
 *
 * @param       none
 *
 * @return      none
 */
extern void AddrMgrWriteNV( void );

/******************************************************************************
 * @fn          AddrMgrWriteNVRequest
 *
 * @brief       Stub routine implemented by NHLE. NHLE should call
 *              <AddrMgrWriteNV> when appropriate.
 *
 * @param       none
 *
 * @return      none
 */
extern void AddrMgrWriteNVRequest( void );

#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
/******************************************************************************
 * @fn          AddrMgrRegister
 *
 * @brief       Register as a user of the Address Manager.
 *
 * @param       reg - [in] register ID
 * @param       cb  - [in] user callback
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrRegister( uint8_t reg, AddrMgrUserCB_t cb );

#endif //ADDRMGR_CALLBACK_ENABLED

/******************************************************************************
 * @fn          AddrMgrExtAddrSet
 *
 * @brief       Set destination address to source address or empty{0x00}.
 *
 * @param       dstExtAddr - [in] destination EXT address
 *              srcExtAddr - [in] source EXT address
 *
 * @return      none
 */
extern void AddrMgrExtAddrSet( uint8_t* dstExtAddr, uint8_t* srcExtAddr );

/******************************************************************************
 * @fn          AddrMgrExtAddrValid
 *
 * @brief       Check if EXT address is valid - not NULL, not empty{0x00}.
 *
 * @param       extAddr - [in] EXT address
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrExtAddrValid( uint8_t* extAddr );

/******************************************************************************
 * @fn          AddrMgrExtAddrEqual
 *
 * @brief       Compare two EXT addresses.
 *
 * @param       extAddr1 - [in] EXT address 1
 *              extAddr2 - [in] EXT address 2
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrExtAddrEqual( uint8_t* extAddr1, uint8_t* extAddr2 );

/******************************************************************************
 * @fn          AddrMgrExtAddrLookup
 *
 * @brief       Lookup EXT address using the NWK address.
 *
 * @param       nwkAddr - [in] NWK address
 *              extAddr - [out] EXT address
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrExtAddrLookup( uint16_t nwkAddr, uint8_t* extAddr );

/******************************************************************************
 * @fn          AddrMgrNwkAddrLookup
 *
 * @brief       Lookup NWK address using the EXT address.
 *
 * @param       extAddr - [in] EXT address
 *              nwkAddr - [out] NWK address
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrNwkAddrLookup( uint8_t* extAddr, uint16_t* nwkAddr );

/******************************************************************************
 * @fn          AddrMgrEntryRelease
 *
 * @brief       Release a user reference from an entry in the Address Manager.
 *
 * @param       entry
 *                ::user  - [in] user ID
 *                ::index - [in] index of data
 *                ::nwkAddr - not used
 *                ::extAddr - not used
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrEntryRelease( AddrMgrEntry_t* entry );

/******************************************************************************
 * @fn          AddrMgrEntryAddRef
 *
 * @brief       Add a user reference to an entry in the Address Manager.
 *
 * @param       entry
 *                ::user  - [in] user ID
 *                ::index - [in] index of data
 *                ::nwkAddr - not used
 *                ::extAddr - not used
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrEntryAddRef( AddrMgrEntry_t* entry );

/******************************************************************************
 * @fn          AddrMgrEntryLookupNwk
 *
 * @brief       Lookup entry based on NWK address.
 *
 * @param       entry
 *                ::user    - [in] user ID
 *                ::nwkAddr - [in] NWK address
 *                ::extAddr - [out] EXT address
 *                ::index   - [out] index of data
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrEntryLookupNwk( AddrMgrEntry_t* entry );

/******************************************************************************
 * @fn          AddrMgrEntryLookupExt
 *
 * @brief       Lookup entry based on EXT address.
 *
 * @param       entry
 *                ::user    - [in] user ID
 *                ::extAddr - [in] EXT address
 *                ::nwkAddr - [out] NWK address
 *                ::index   - [out] index of data
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrEntryLookupExt( AddrMgrEntry_t* entry );

/******************************************************************************
 * @fn          AddrMgrEntryGet
 *
 * @brief       Get NWK address and EXT address based on index.
 *
 * @param       entry
 *                ::user    - [in] user ID
 *                ::index   - [in] index of data
 *                ::nwkAddr - [out] NWK address
 *                ::extAddr - [out] EXT address
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
extern uint8_t AddrMgrEntryGet( AddrMgrEntry_t* entry );

/******************************************************************************
 * @fn          AddrMgrEntryUpdate
 *
 * @brief       Update an entry into the Address Manager.
 *
 * @param       entry
 *                ::user    - [in] user ID
 *                ::nwkAddr - [in] NWK address
 *                ::extAddr - [in] EXT address
 *                ::index   - [out] index of data
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
uint8_t AddrMgrEntryUpdate( AddrMgrEntry_t* entry );

/******************************************************************************
 * @fn          AddrMgrIsFull
 *
 * @brief       Checks to see if the address manager is full.
 *
 * @param       none
 *
 * @return      uint8_t - success(TRUE:FALSE)
 *                      TRUE if there are no more empty slots,
 *                      FALSE if available slot
 */
extern uint8_t AddrMgrIsFull( void );


/******************************************************************************
******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ADDRMGR_H */
