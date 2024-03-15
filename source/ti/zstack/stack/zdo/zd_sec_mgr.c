/******************************************************************************
  Filename:       ZDSecMgr.c
  Revised:        $Date: 2014-06-05 11:25:00 -0700 (Thu, 05 Jun 2014) $
  Revision:       $Revision: 38833 $

  Description:    The ZigBee Device Security Manager.


  Copyright 2005-2014 Texas Instruments Incorporated.

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
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "osal_nv.h"
#include "zglobals.h"
#include "ssp.h"
#include "nwk_globals.h"
#include "nwk.h"
#include "nl_mede.h"
#include "addr_mgr.h"
#include "assoc_list.h"
#include "aps_mede.h"
#include "zd_config.h"
#include "zd_sec_mgr.h"
#include "bdb.h"

#include <ti/drivers/TRNG.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

/******************************************************************************
 * CONSTANTS
 */

// maximum number of devices managed by this Security Manager
#if !defined ( ZDSECMGR_DEVICE_MAX )
  #define ZDSECMGR_DEVICE_MAX 3
#endif

// maximum number of LINK keys this device may store
#define ZDSECMGR_ENTRY_MAX ZDSECMGR_DEVICE_MAX

// total number of stored devices
#if !defined ( ZDSECMGR_STORED_DEVICES )
  #define ZDSECMGR_STORED_DEVICES 3
#endif

// Joining Device Policies: r21 spec 4.9.1
// This boolean indicates whether the device will request a new Trust Center Link key after joining.
// TC link key cannot be requested if join is performed on distributed nwk
bool  requestNewTrustCenterLinkKey = TRUE;
//This integer indicates the maximum time in seconds that a device will wait for a response to a
//request for a Trust Center link key.
uint32_t  requestLinkKeyTimeout = BDBC_TC_LINK_KEY_EXANGE_TIMEOUT;
 //bool acceptNewUnsolicitedApplicationLinkKey;

// APSME Stub Implementations
#define ZDSecMgrLinkKeySet       APSME_LinkKeySet
#define ZDSecMgrLinkKeyNVIdGet   APSME_LinkKeyNVIdGet
#define ZDSecMgrKeyFwdToChild    APSME_KeyFwdToChild
#define ZDSecMgrIsLinkKeyValid   APSME_IsLinkKeyValid

/******************************************************************************
 * TYPEDEFS
 */
typedef struct
{
  uint16_t            ami;
  uint16_t            keyNvId;   // index to the Link Key table in NV
  ZDSecMgr_Authentication_Option authenticateOption;
} ZDSecMgrEntry_t;

typedef struct
{
  uint16_t          nwkAddr;
  uint8_t*          extAddr;
  uint16_t          parentAddr;
  uint8_t           secure;
  uint8_t           devStatus;
} ZDSecMgrDevice_t;

/******************************************************************************
 * EXTERNAL VARIABLES
 */
extern CONST uint8_t gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES;
extern pfnZdoCb zdoCBFunc[MAX_ZDO_CB_FUNC];

/******************************************************************************
 * EXTERNAL FUNCTIONS
 */
extern void   ZDApp_ResetTimerCancel( void );
/******************************************************************************
 * LOCAL VARIABLES
 */
uint8_t ZDSecMgrTCExtAddr[Z_EXTADDR_LEN]=
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

uint8_t ZDSecMgrTCAuthenticated = FALSE;

ZDSecMgrEntry_t* ZDSecMgrEntries  = NULL;

void ZDSecMgrAddrMgrCB( uint8_t update, AddrMgrEntry_t* newEntry, AddrMgrEntry_t* oldEntry );

uint8_t ZDSecMgrPermitJoiningEnabled;
uint8_t ZDSecMgrPermitJoiningTimed;

APSME_TCLinkKeyNVEntry_t TrustCenterLinkKey;

APSME_ApsLinkKeyRAMEntry_t ApsLinkKeyRAMEntry[ZDSECMGR_ENTRY_MAX];
APSME_TCLinkKeyRAMEntry_t TCLinkKeyRAMEntry[ZDSECMGR_TC_DEVICE_MAX];

CONST uint16_t gZDSECMGR_ENTRY_MAX = ZDSECMGR_ENTRY_MAX;
CONST uint16_t gZDSECMGR_TC_DEVICE_MAX = ZDSECMGR_TC_DEVICE_MAX;
CONST uint16_t gZDSECMGR_TC_DEVICE_IC_MAX = ZDSECMGR_TC_DEVICE_IC_MAX;
uint8_t  gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY = ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;
/******************************************************************************
 * PRIVATE FUNCTIONS
 *
 *   ZDSecMgrAddrStore
 *   ZDSecMgrExtAddrStore
 *   ZDSecMgrExtAddrLookup
 *   ZDSecMgrEntryInit
 *   ZDSecMgrEntryLookup
 *   ZDSecMgrEntryLookupAMI
 *   ZDSecMgrEntryLookupExt
 *   ZDSecMgrEntryLookupExtGetIndex
 *   ZDSecMgrEntryFree
 *   ZDSecMgrEntryNew
 *   ZDSecMgrAppKeyGet
 *   ZDSecMgrAppKeyReq
 *   ZDSecMgrTclkReq
 *   ZDSecMgrAppConfKeyReq
 *   ZDSecMgrSendNwkKey
 *   ZDSecMgrDeviceRemove
 *   ZDSecMgrDeviceValidate
 *   ZDSecMgrDeviceJoin
 *   ZDSecMgrDeviceJoinDirect
 *   ZDSecMgrDeviceJoinFwd
 *   ZDSecMgrDeviceNew
 *   ZDSecMgrAssocDeviceAuth
 *   ZDSecMgrAuthNwkKey
 *   APSME_TCLinkKeyInit
 *   APSME_IsDefaultTCLK
 */

//-----------------------------------------------------------------------------
// address management
//-----------------------------------------------------------------------------
ZStatus_t ZDSecMgrAddrStore( uint16_t nwkAddr, uint8_t* extAddr, uint16_t* ami );
ZStatus_t ZDSecMgrExtAddrStore( uint16_t nwkAddr, uint8_t* extAddr, uint16_t* ami );
ZStatus_t ZDSecMgrExtAddrLookup( uint8_t* extAddr, uint16_t* ami );

//-----------------------------------------------------------------------------
// Trust Center management
//-----------------------------------------------------------------------------
uint8_t ZDSecMgrTCExtAddrCheck( uint8_t* extAddr );
void ZDSecMgrTCDataLoad( uint8_t* extAddr );

//-----------------------------------------------------------------------------
// entry data
//-----------------------------------------------------------------------------
void ZDSecMgrEntryInit(uint8_t state);
ZStatus_t ZDSecMgrEntryLookup( uint16_t nwkAddr, ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrEntryLookupAMI( uint16_t ami, ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrEntryLookupExt( uint8_t* extAddr, ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrEntryLookupExtGetIndex( uint8_t* extAddr, ZDSecMgrEntry_t** entry, uint16_t* entryIndex );
ZStatus_t ZDSecMgrEntryLookupAMIGetIndex( uint16_t ami, uint16_t* entryIndex );
void ZDSecMgrEntryFree( ZDSecMgrEntry_t* entry );
ZStatus_t ZDSecMgrEntryNew( ZDSecMgrEntry_t** entry );
ZStatus_t ZDSecMgrAuthenticationSet( uint8_t* extAddr, ZDSecMgr_Authentication_Option option );
void ZDSecMgrApsLinkKeyInit(uint8_t setDefault);
#if defined ( NV_RESTORE )
static void ZDSecMgrWriteNV(void);
static void ZDSecMgrRestoreFromNV(void);
static void ZDSecMgrUpdateNV( uint16_t index );
#endif

//-----------------------------------------------------------------------------
// key support
//-----------------------------------------------------------------------------
ZStatus_t ZDSecMgrAppKeyGet( uint16_t  initNwkAddr,
                             uint8_t*  initExtAddr,
                             uint16_t  partNwkAddr,
                             uint8_t*  partExtAddr,
                             uint8_t** key,
                             uint8_t*  keyType );
void ZDSecMgrAppKeyReq( ZDO_RequestKeyInd_t* ind );
void ZDSecMgrTclkReq( ZDO_RequestKeyInd_t* ind );
void ZDSecMgrAppConfKeyReq( ZDO_VerifyKeyInd_t* ind );
ZStatus_t ZDSecMgrSendNwkKey( ZDSecMgrDevice_t* device );
void ZDSecMgrNwkKeyInit(uint8_t setDefault);

//-----------------------------------------------------------------------------
// device management
//-----------------------------------------------------------------------------
void ZDSecMgrDeviceRemove( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceValidate( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceJoin( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceJoinDirect( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceJoinFwd( ZDSecMgrDevice_t* device );
ZStatus_t ZDSecMgrDeviceNew( ZDSecMgrDevice_t* device );

//-----------------------------------------------------------------------------
// association management
//-----------------------------------------------------------------------------
void ZDSecMgrAssocDeviceAuth( associated_devices_t* assoc );

//-----------------------------------------------------------------------------
// authentication management
//-----------------------------------------------------------------------------
void ZDSecMgrAuthNwkKey( void );

//-----------------------------------------------------------------------------
// APSME function
//-----------------------------------------------------------------------------
void APSME_TCLinkKeyInit( uint8_t setDefault );
uint8_t APSME_IsDefaultTCLK( uint8_t *extAddr );
void ZDSecMgrGenerateSeed(uint8_t setDefault );
void ZDSecMgrGenerateKeyFromSeed(uint8_t *extAddr, uint8_t shift, uint8_t *key);
/******************************************************************************
 * @fn          ZDSecMgrAddrStore
 *
 * @brief       Store device addresses.
 *
 * @param       nwkAddr - [in] NWK address
 * @param       extAddr - [in] EXT address
 * @param       ami     - [out] Address Manager index
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAddrStore( uint16_t nwkAddr, uint8_t* extAddr, uint16_t* ami )
{
  ZStatus_t      status;
  AddrMgrEntry_t entry;


  // add entry
  entry.user    = ADDRMGR_USER_SECURITY;
  entry.nwkAddr = nwkAddr;
  AddrMgrExtAddrSet( entry.extAddr, extAddr );

  if ( AddrMgrEntryUpdate( &entry ) == TRUE )
  {
    // return successful results
    *ami   = entry.index;
    status = ZSuccess;
  }
  else
  {
    // return failed results
    *ami   = entry.index;
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrExtAddrStore
 *
 * @brief       Store EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       ami     - [out] Address Manager index
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrExtAddrStore( uint16_t nwkAddr, uint8_t* extAddr, uint16_t* ami )
{
  ZStatus_t      status;
  AddrMgrEntry_t entry;


  // add entry
  entry.user    = ADDRMGR_USER_SECURITY;
  entry.nwkAddr = nwkAddr;
  AddrMgrExtAddrSet( entry.extAddr, extAddr );

  if ( AddrMgrEntryUpdate( &entry ) == TRUE )
  {
    // return successful results
    *ami   = entry.index;
    status = ZSuccess;
  }
  else
  {
    // return failed results
    *ami   = entry.index;
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrExtAddrLookup
 *
 * @brief       Lookup index for specified EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       ami     - [out] Address Manager index
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrExtAddrLookup( uint8_t* extAddr, uint16_t* ami )
{
  ZStatus_t      status;
  AddrMgrEntry_t entry;


  // lookup entry
  entry.user = ADDRMGR_USER_SECURITY;
  AddrMgrExtAddrSet( entry.extAddr, extAddr );

  if ( AddrMgrEntryLookupExt( &entry ) == TRUE )
  {
    // return successful results
    *ami   = entry.index;
    status = ZSuccess;
  }
  else
  {
    // return failed results
    *ami   = entry.index;
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAddrClear
 *
 * @brief       Clear security bit from Address Manager for specific device.
 *
 * @param       extAddr - [in] EXT address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAddrClear( uint8_t* extAddr )
{
  ZStatus_t status;
  uint16_t entryAmi;

  // get Address Manager Index
  status = ZDSecMgrExtAddrLookup( extAddr, &entryAmi );

  if ( status == ZSuccess )
  {
    AddrMgrEntry_t addrEntry;

    // Clear SECURITY User bit from the address manager
    addrEntry.user = ADDRMGR_USER_SECURITY;
    addrEntry.index = entryAmi;

    if ( AddrMgrEntryRelease( &addrEntry ) != TRUE )
    {
      // return failure results
      status = ZFailure;
    }
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryInit
 *
 * @brief       Initialize entry sub module
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
void ZDSecMgrEntryInit(uint8_t state)
{
  if (ZDSecMgrEntries == NULL)
  {
    uint16_t index;

    if ((ZDSecMgrEntries = OsalPort_malloc(sizeof(ZDSecMgrEntry_t) * gZDSECMGR_ENTRY_MAX)) == NULL)
    {
      return;
    }

    for (index = 0; index < gZDSECMGR_ENTRY_MAX; index++)
    {
      ZDSecMgrEntries[index].ami = INVALID_NODE_ADDR;

      ZDSecMgrEntries[index].keyNvId = SEC_NO_KEY_NV_ID;
    }
  }

#if defined NV_RESTORE
  if (state == ZDO_INITDEV_RESTORED_NETWORK_STATE)
  {
    ZDSecMgrRestoreFromNV();
  }
#else
  (void)state;
#endif
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookup
 *
 * @brief       Lookup entry index using specified NWK address.
 *
 * @param       nwkAddr - [in] NWK address
 * @param       entry   - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookup( uint16_t nwkAddr, ZDSecMgrEntry_t** entry )
{
  uint16_t         index;
  AddrMgrEntry_t addrMgrEntry;

  // initialize results
  *entry = NULL;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    addrMgrEntry.user    = ADDRMGR_USER_SECURITY;
    addrMgrEntry.nwkAddr = nwkAddr;

    if ( AddrMgrEntryLookupNwk( &addrMgrEntry ) == TRUE )
    {
      for ( index = 0; index < gZDSECMGR_ENTRY_MAX ; index++ )
      {
        if ( addrMgrEntry.index == ZDSecMgrEntries[index].ami )
        {
          // return successful results
          *entry = &ZDSecMgrEntries[index];

          // break from loop
          return ZSuccess;
        }
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupAMI
 *
 * @brief       Lookup entry using specified address index
 *
 * @param       ami   - [in] Address Manager index
 * @param       entry - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupAMI( uint16_t ami, ZDSecMgrEntry_t** entry )
{
  uint16_t index;

  // initialize results
  *entry = NULL;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    for ( index = 0; index < gZDSECMGR_ENTRY_MAX ; index++ )
    {
      if ( ZDSecMgrEntries[index].ami == ami )
      {
        // return successful results
        *entry = &ZDSecMgrEntries[index];

        // break from loop
        return ZSuccess;
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupExt
 *
 * @brief       Lookup entry index using specified EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       entry   - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupExt( uint8_t* extAddr, ZDSecMgrEntry_t** entry )
{
  ZStatus_t status;
  uint16_t    ami;

  // initialize results
  *entry = NULL;
  status = ZNwkUnknownDevice;

  // lookup address index
  if ( ZDSecMgrExtAddrLookup( extAddr, &ami ) == ZSuccess )
  {
    status = ZDSecMgrEntryLookupAMI( ami, entry );
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupExtGetIndex
 *
 * @brief       Lookup entry index using specified EXT address.
 *
 * @param       extAddr - [in] EXT address
 * @param       entryIndex - [out] valid index to the entry table
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupExtGetIndex( uint8_t* extAddr, ZDSecMgrEntry_t** entry, uint16_t* entryIndex )
{
  uint16_t ami;
  uint16_t index;

  // lookup address index
  if ( ZDSecMgrExtAddrLookup( extAddr, &ami ) == ZSuccess )
  {
    // verify data is available
    if ( ZDSecMgrEntries != NULL )
    {
      for ( index = 0; index < gZDSECMGR_ENTRY_MAX ; index++ )
      {
        if ( ZDSecMgrEntries[index].ami == ami )
        {
          // return successful results
          *entry = &ZDSecMgrEntries[index];
          *entryIndex = index;

          // break from loop
          return ZSuccess;
        }
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryLookupAMIGetIndex
 *
 * @brief       Lookup entry using specified address index
 *
 * @param       ami   - [in] Address Manager index
 * @param       entryIndex - [out] valid index to the entry table
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryLookupAMIGetIndex( uint16_t ami, uint16_t* entryIndex )
{
  uint16_t index;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    for ( index = 0; index < gZDSECMGR_ENTRY_MAX ; index++ )
    {
      if ( ZDSecMgrEntries[index].ami == ami )
      {
        // return successful results
        *entryIndex = index;

        // break from loop
        return ZSuccess;
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrEntryFree
 *
 * @brief       Free entry.
 *
 * @param       entry - [in] valid entry
 *
 * @return      ZStatus_t
 */
void ZDSecMgrEntryFree( ZDSecMgrEntry_t* entry )
{
  APSME_ApsLinkKeyNVEntry_t   *pApsLinkKey = NULL;

#if defined ( NV_RESTORE )
  ZStatus_t status;
  uint16_t entryIndex;

  status = ZDSecMgrEntryLookupAMIGetIndex( entry->ami, &entryIndex );
#endif

  pApsLinkKey = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

  if (pApsLinkKey != NULL)
  {
    memset( pApsLinkKey, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t) );

    // Clear the APS Link key in NV
    osal_nv_write( entry->keyNvId,
                        sizeof(APSME_ApsLinkKeyNVEntry_t), pApsLinkKey);

    // set entry to invalid Key
    entry->keyNvId = SEC_NO_KEY_NV_ID;

    OsalPort_free(pApsLinkKey);
  }

  // marking the entry as INVALID_NODE_ADDR
  entry->ami = INVALID_NODE_ADDR;

  // set to default value
  entry->authenticateOption = ZDSecMgr_Not_Authenticated;

#if defined ( NV_RESTORE )
  if ( status == ZSuccess )
  {
    ZDSecMgrUpdateNV(entryIndex);
  }
#endif
}

/******************************************************************************
 * @fn          ZDSecMgrEntryNew
 *
 * @brief       Get a new entry.
 *
 * @param       entry - [out] valid entry
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrEntryNew( ZDSecMgrEntry_t** entry )
{
  uint16_t index;

  // initialize results
  *entry = NULL;

  // verify data is available
  if ( ZDSecMgrEntries != NULL )
  {
    // find available entry
    for ( index = 0; index < gZDSECMGR_ENTRY_MAX ; index++ )
    {
      if ( ZDSecMgrEntries[index].ami == INVALID_NODE_ADDR )
      {
        // return successful result
        *entry = &ZDSecMgrEntries[index];

        // Set the authentication option to default
        ZDSecMgrEntries[index].authenticateOption = ZDSecMgr_Not_Authenticated;

        // break from loop
        return ZSuccess;
      }
    }
  }

  return ZNwkUnknownDevice;
}

/******************************************************************************
 * @fn          ZDSecMgrAppKeyGet
 *
 * @brief       get an APP key - option APP(MASTER or LINK) key
 *
 * @param       initNwkAddr - [in] NWK address of initiator device
 * @param       initExtAddr - [in] EXT address of initiator device
 * @param       partNwkAddr - [in] NWK address of partner device
 * @param       partExtAddr - [in] EXT address of partner device
 * @param       key         - [out] APP(MASTER or LINK) key
 * @param       keyType     - [out] APP(MASTER or LINK) key type
 *
 * @return      ZStatus_t
 */
uint8_t ZDSecMgrAppKeyType = KEY_TYPE_APP_LINK;    // Set the default key type
                                                 // to KEY_TYPE_APP_LINK since
                                                 // only specific requirement
                                                 // right now comes from SE profile
ZStatus_t ZDSecMgrAppKeyGet( uint16_t  initNwkAddr,
                             uint8_t*  initExtAddr,
                             uint16_t  partNwkAddr,
                             uint8_t*  partExtAddr,
                             uint8_t** key,
                             uint8_t*  keyType )
{
  // Intentionally unreferenced parameters
  (void)initNwkAddr;
  (void)initExtAddr;
  (void)partNwkAddr;
  (void)partExtAddr;

  //---------------------------------------------------------------------------
  // note:
  // should use a robust mechanism to generate keys, for example
  // combine EXT addresses and call a hash function
  //---------------------------------------------------------------------------
  SSP_GetTrueRand( SEC_KEY_LEN, *key );

  *keyType = ZDSecMgrAppKeyType;

  return ZSuccess;
}




/******************************************************************************
 * @fn          ZDSecMgrAppKeyReq
 *
 * @brief       Process request for APP key between two devices.
 *
 * @param       device - [in] ZDO_RequestKeyInd_t, request info
 *
 * @return      none
 */
void ZDSecMgrAppKeyReq( ZDO_RequestKeyInd_t* ind )
{
  APSME_TransportKeyReq_t req;
  uint8_t                   initExtAddr[Z_EXTADDR_LEN];
  uint16_t                  partNwkAddr;
  uint8_t                   key[SEC_KEY_LEN];


  // validate initiator and partner
  if ( ( APSME_LookupNwkAddr( ind->partExtAddr, &partNwkAddr ) == TRUE ) &&
       ( APSME_LookupExtAddr( ind->srcAddr, initExtAddr ) == TRUE      )   )
  {
    // point the key to some memory
    req.key = key;

    // get an APP key - option APP (MASTER or LINK) key
    if ( ZDSecMgrAppKeyGet( ind->srcAddr,
                            initExtAddr,
                            partNwkAddr,
                            ind->partExtAddr,
                            &req.key,
                            &req.keyType ) == ZSuccess )
    {
      // always secure
      req.nwkSecure = TRUE;
      req.apsSecure = TRUE;
      req.tunnel    = NULL;

      // send key to initiator device
      req.dstAddr   = ind->srcAddr;
      req.extAddr   = ind->partExtAddr;
      req.initiator = TRUE;
      APSME_TransportKeyReq( &req );

      // send key to partner device
      req.dstAddr   = partNwkAddr;
      req.extAddr   = initExtAddr;
      req.initiator = FALSE;

      APSME_TransportKeyReq( &req );

      // clear copy of key in RAM
      memset( key, 0x00, SEC_KEY_LEN);

    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrTclkReq
 *
 * @brief       Process request for TCLK.
 *
 * @param       device - [in] ZDO_RequestKeyInd_t, request info
 *
 * @return      none
 */
void ZDSecMgrTclkReq( ZDO_RequestKeyInd_t* ind )
{
  APSME_TransportKeyReq_t req;
  uint8_t                   initExtAddr[Z_EXTADDR_LEN];
  uint16_t                  partNwkAddr;
  uint8_t                   key[SEC_KEY_LEN];
  APSME_TCLinkKeyNVEntry_t    TCLKDevEntry;
  uint8_t                   found;

  // validate initiator and partner
  if ( ( ( APSME_LookupNwkAddr( ind->partExtAddr, &partNwkAddr ) == TRUE ) || ( ind->keyType != KEY_TYPE_APP_MASTER ) ) &&
       (   APSME_LookupExtAddr( ind->srcAddr, initExtAddr )      == TRUE ) )
  {
    // point the key to some memory
    req.key = key;

    //Search for the entry
    APSME_SearchTCLinkKeyEntry(initExtAddr,&found, &TCLKDevEntry);

    //If found, generate the key accordingly to the key attribute
    if(found)
    {
      //Generate key from the seed, which would be the unique key
      ZDSecMgrGenerateKeyFromSeed(TCLKDevEntry.extAddr,TCLKDevEntry.SeedShift,req.key);

      // always secure
      req.nwkSecure = TRUE;
      req.apsSecure = TRUE;
      req.tunnel    = NULL;

      // send key to initiator device
      req.dstAddr   = ind->srcAddr;
      req.extAddr   = initExtAddr;
      req.initiator = TRUE;
      req.keyType   = KEY_TYPE_TC_LINK;

      APSME_TransportKeyReq( &req );

      // clear copy of key in RAM
      memset( key, 0x00, SEC_KEY_LEN);

    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrAppConfKeyReq
 *
 * @brief       Process request for APP key between two devices.
 *
 * @param       device - [in] ZDO_VerifyKeyInd_t, request info
 *
 * @return      none
 */
void ZDSecMgrAppConfKeyReq( ZDO_VerifyKeyInd_t* ind )
{
  APSME_ConfirmKeyReq_t   req;

  // send key to initiator device
  req.dstAddr      = ind->srcAddr;
  req.status       = ind->verifyKeyStatus;
  req.dstExtAddr   = ind->extAddr;
  req.keyType      = ind->keyType;

  if ( ( ZSTACK_DEVICE_BUILD & DEVICE_BUILD_COORDINATOR ) != 0 )
  {
    APSME_ConfirmKeyReq( &req );
  }

}


/******************************************************************************
 * @fn          ZDSecMgrSendNwkKey
 *
 * @brief       Send NWK key to device joining network.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSendNwkKey( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;
  APSME_TransportKeyReq_t req;
  APSDE_FrameTunnel_t tunnel;
  nwkKeyDesc tmpKey;

  req.dstAddr   = device->nwkAddr;
  req.extAddr   = device->extAddr;

  req.keyType   = KEY_TYPE_NWK;

  // get the Active Key into a local variable
  if ( NLME_ReadNwkKeyInfo( 0, sizeof(tmpKey), &tmpKey,
                           ZCD_NV_NWK_ACTIVE_KEY_INFO ) != SUCCESS )
  {
    // set key data to all 0s if NV read fails
    memset(&tmpKey, 0x00, sizeof(tmpKey));
  }

  // set values
  req.keySeqNum = tmpKey.keySeqNum;
  req.key       = tmpKey.key;

  //devtag.pro.security.todo - make sure that if there is no link key the NWK
  //key isn't used to secure the frame at the APS layer -- since the receiving
  //device may not have a NWK key yet
  req.apsSecure = TRUE;

  // check if using secure hop to parent
  if ( device->parentAddr == NLME_GetShortAddr() )
  {
    req.nwkSecure = FALSE;
    req.tunnel    = NULL;
  }
  else
  {
    req.nwkSecure   = TRUE;
    req.tunnel      = &tunnel;
    req.tunnel->tna = device->parentAddr;
    req.tunnel->dea = device->extAddr;
  }

  status = APSME_TransportKeyReq( &req );

  // clear copy of key in RAM before return
  memset( &tmpKey, 0x00, sizeof(nwkKeyDesc) );

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceRemoveByExtAddr
 *
 * @brief       Remove device entry by its ext address.
 *
 * @param       pAddr - pointer to the extended address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceRemoveByExtAddr( uint8_t *pAddr )
{
  ZDSecMgrEntry_t *pEntry;
  uint8_t           retValue;

  retValue = (uint8_t)ZDSecMgrEntryLookupExt( pAddr, &pEntry );

  if( retValue == ZSuccess )
  {
    // remove device from entry data
    ZDSecMgrEntryFree( pEntry );
  }

  return retValue;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceRemove
 *
 * @brief       Remove device from network.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      none
 */
void ZDSecMgrDeviceRemove( ZDSecMgrDevice_t* device )
{
  APSME_RemoveDeviceReq_t remDevReq;
  NLME_LeaveReq_t         leaveReq;
  associated_devices_t*   assoc;
  uint8_t TC_ExtAddr[Z_EXTADDR_LEN];

  // check if parent, remove the device
  if ( device->parentAddr == NLME_GetShortAddr() )
  {
    // this is the parent of the device
    leaveReq.extAddr        = device->extAddr;
    leaveReq.removeChildren = FALSE;
    leaveReq.rejoin         = FALSE;

    // find child association
    assoc = AssocGetWithExt( device->extAddr );

    if ( ( assoc != NULL                            ) &&
         ( assoc->nodeRelation >= CHILD_RFD         ) &&
         ( assoc->nodeRelation <= CHILD_FFD_RX_IDLE )    )
    {
      // check if associated device is authenticated
      if ( assoc->devStatus & DEV_SEC_AUTH_STATUS )
      {
        leaveReq.silent = FALSE;
      }
      else
      {
        leaveReq.silent = TRUE;
      }

      NLME_LeaveReq( &leaveReq );
    }
    else if ( device->nwkAddr == NLME_GetShortAddr() )
    {
      // this is when ZC wants that ZR removes itself from the network
      leaveReq.extAddr = NULL;
      leaveReq.silent = FALSE;

      NLME_LeaveReq( &leaveReq );
    }
  }
  else
  {
    uint8 found;

    // this is not the parent of the device
    remDevReq.parentAddr   = device->parentAddr;
    remDevReq.childExtAddr = device->extAddr;

    APSME_GetRequest( apsTrustCenterAddress,0, TC_ExtAddr );

    APSME_SearchTCLinkKeyEntry(TC_ExtAddr,&found,NULL);

    // For ZG_GLOBAL_LINK_KEY the message has to be sent twice, one
    // APS un-encrypted and one APS encrypted, to make sure that it can interoperate
    // with legacy Coordinator devices which can only handle one or the other.
#if defined ( APP_TP2_TEST_MODE )
    if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( guTxApsSecON != TRUE ) )
#else
    if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( found == FALSE ) )
#endif
    {
      remDevReq.apsSecure = FALSE;

      APSME_RemoveDeviceReq( &remDevReq );
    }

#if defined ( APP_TP2_TEST_MODE )
    if ( guTxApsSecON != FALSE )
    {
      remDevReq.apsSecure = TRUE;
      APSME_RemoveDeviceReq( &remDevReq );
    }
#else
    remDevReq.apsSecure = TRUE;

    APSME_RemoveDeviceReq( &remDevReq );
#endif
  }
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceValidate
 *
 * @brief       Decide whether device is allowed.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceValidate( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;

  if ( (ZDSecMgrPermitJoiningEnabled == TRUE) && (zgSecurePermitJoin == TRUE) )
  {
    status = ZSuccess;
  }
  else
  {
    status = ZNwkUnknownDevice;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceJoin
 *
 * @brief       Try to join this device.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceJoin( ZDSecMgrDevice_t* device )
{
  ZStatus_t status = ZSuccess;
  uint16_t    ami;

  // attempt to validate device that joined/rejoined without security
  if ( device->secure == FALSE && (!(device->devStatus & DEV_SEC_AUTH_TC_REJOIN_STATUS)))
  {
    status = ZDSecMgrDeviceValidate( device );
  }

  if ( status == ZSuccess )
  {
    // Add the device to the address manager
    ZDSecMgrAddrStore( device->nwkAddr, device->extAddr, &ami );

    // Send the nwk key if joining/rejoining has no nwk security
    if ( device->secure == FALSE )
    {
        uint8_t sendNwkKey = FALSE;

        if ( device->devStatus & DEV_SEC_AUTH_TC_REJOIN_STATUS )
        {
          // check if we recognize the device performing an unsecure rejoin. If we have
          // previously authenticated it on the TC, we can consider that it is performing
          // a trust center rejoin, and we can send it the nwk key using the unique TCLK
          // that was previously established. If we do not recognize it, it is an unknown
          // device performing an unsecure rejoin, so we would have to encrypt the nwk key
          // using the well-known TCLK. zgAllowRejoinsWithWellKnownKey determines if we are allowed to
          // encrypt the nwk key using the well-known TCLK or not, see zglobals.c definition for
          // more info. Additionally, if the nwk is open (i.e. NLME_PermitJoining == TRUE), we
          // should permit devices to unsecure TC rejoin anyways
          if( (zgAllowRejoinsWithWellKnownKey == FALSE) && (NLME_PermitJoining == FALSE) )
          {
            uint8_t found = 0;
            APSME_TCLinkKeyNVEntry_t TCLKDevEntry = {0};
            APSME_SearchTCLinkKeyEntry(device->extAddr, &found, &TCLKDevEntry);

            // if we found the device and its key is in the
            // ZG_VERIFIED_KEY state, that means we have established a
            // unique TCLK, so we can send it the NWK key using it
            if( found && (TCLKDevEntry.keyAttributes == ZG_VERIFIED_KEY) )
            {
              sendNwkKey = TRUE;
            }
          }
          // zgAllowRejoinsWithWellKnownKey == TRUE
          else
          {
            sendNwkKey = TRUE;
          }
        }
        else if ( device->devStatus & DEV_SEC_INIT_STATUS )
        {
          // initial join
          sendNwkKey = TRUE;
        }

        if ( sendNwkKey == TRUE )
        {
          //send the nwk key data to the joining device
          status = ZDSecMgrSendNwkKey( device );
        }
        else if ( device->devStatus & DEV_SEC_AUTH_STATUS )
        {
          // device has already joined and been authenticated so we do not
          // need to send it the key again
          status = ZSuccess;
        }
        else
        {
          status = ZSecFailure;
        }
    }

    if ( status != ZSuccess )
    {
      ZDSecMgrAddrClear( device->extAddr );
    }
  }

  if ( status != ZSuccess )
  {
    // not allowed or transport key failed, remove the device
    ZDSecMgrDeviceRemove( device );
  }
  else
  {
    // Pass the Trust Center Device Indication to higher layer if callback registered
    if (zdoCBFunc[ZDO_TC_DEVICE_CBID] != NULL )
    {
      ZDO_TC_Device_t dev;

      dev.nwkAddr = device->nwkAddr;
      OsalPort_memcpy( dev.extAddr, device->extAddr, Z_EXTADDR_LEN );
      dev.parentAddr = device->parentAddr;

      zdoCBFunc[ZDO_TC_DEVICE_CBID]( (void*)&dev );
    }
  }



  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceJoinDirect
 *
 * @brief       Try to join this device as a direct child.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceJoinDirect( ZDSecMgrDevice_t* device )
{
  ZStatus_t status;

  if(device->secure == FALSE)
  {
    uint8_t  found;
    uint16_t keyNvIndex;
    APSME_TCLinkKeyNVEntry_t TCLKDevEntry;

    keyNvIndex = APSME_SearchTCLinkKeyEntry(device->extAddr,&found, &TCLKDevEntry);

    //If not doing a TC rejoin...
    if(!(device->devStatus & DEV_SEC_AUTH_TC_REJOIN_STATUS))
    {
        //... and found and it was verified, then allow it to join in a fresh state by erasing the key entry
        if((found == TRUE) && (TCLKDevEntry.keyAttributes == ZG_VERIFIED_KEY))
        {
          uint16_t index;
          TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
          //Increase the shift by one. Validate the maximum shift of the seed which is 15
          TCLKDevEntry.SeedShift++;
          TCLKDevEntry.SeedShift &= 0x0F;

          TCLKDevEntry.rxFrmCntr = 0;
          TCLKDevEntry.txFrmCntr = 0;

          index = keyNvIndex;

          TCLinkKeyRAMEntry[index].rxFrmCntr = 0;
          TCLinkKeyRAMEntry[index].txFrmCntr = 0;

          //Update the entry
          osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, keyNvIndex,
                           sizeof(APSME_TCLinkKeyNVEntry_t),
                           &TCLKDevEntry);
        }
    }
  }

  status = ZDSecMgrDeviceJoin( device );

  if ( status == ZSuccess )
  {
  #if (ZG_BUILD_COORDINATOR_TYPE)
    // if the device joined unsecured and is not performing a
    // trust center rejoin, add it as a joining device
    if( device->secure == FALSE &&
      !(device->devStatus & DEV_SEC_AUTH_TC_REJOIN_STATUS) )
    {
      bdb_TCAddJoiningDevice(NLME_GetShortAddr(),device->extAddr);
    }
  #endif
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceJoinFwd
 *
 * @brief       Forward join to Trust Center.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceJoinFwd( ZDSecMgrDevice_t* device )
{
  ZStatus_t               status;
  APSME_UpdateDeviceReq_t req;
  uint8_t TC_ExtAddr[Z_EXTADDR_LEN];
  uint8_t found;

  // forward any joining device to the Trust Center -- the Trust Center will
  // decide if the device is allowed to join
  status = ZSuccess;

  // forward authorization to the Trust Center
  req.dstAddr    = APSME_TRUSTCENTER_NWKADDR;
  req.devAddr    = device->nwkAddr;
  req.devExtAddr = device->extAddr;

  // set security status, option for router to reject if policy set
  if ( (device->devStatus & DEV_HIGH_SEC_STATUS) )
  {
    if ( device->devStatus & DEV_REJOIN_STATUS )
    {
      if ( device->secure == TRUE )
      {
        req.status = APSME_UD_HIGH_SECURED_REJOIN;
      }
      else
      {
        req.status = APSME_UD_HIGH_UNSECURED_REJOIN;
      }
    }
    else
    {
      req.status = APSME_UD_HIGH_UNSECURED_JOIN;
    }
  }
  else
  {
    if ( device->devStatus & DEV_REJOIN_STATUS )
    {
      if ( device->secure == TRUE )
      {
        req.status = APSME_UD_STANDARD_SECURED_REJOIN;
      }
      else
      {
        req.status = APSME_UD_STANDARD_TRUST_CENTER_REJOIN;
      }
    }
    else
    {
      req.status = APSME_UD_STANDARD_UNSECURED_JOIN;
    }
  }

  APSME_GetRequest( apsTrustCenterAddress,0, TC_ExtAddr );

  APSME_SearchTCLinkKeyEntry(TC_ExtAddr,&found,NULL);

  // For ZG_GLOBAL_LINK_KEY the message has to be sent twice one
  // un-encrypted and one APS encrypted, to make sure that it can interoperate
  // with legacy Coordinator devices which can only handle one or the other.
#if defined ( APP_TP2_TEST_MODE )
  if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( guTxApsSecON != TRUE ) )
#else
  if ( ( zgApsLinkKeyType == ZG_GLOBAL_LINK_KEY ) && ( found == FALSE ) )
#endif
  {
    req.apsSecure = FALSE;

    // send and APSME_UPDATE_DEVICE request to the trust center
    status = APSME_UpdateDeviceReq( &req );
  }

#if defined ( APP_TP2_TEST_MODE )
  if ( guTxApsSecON != FALSE )
  {
    // send the message APS encrypted
    req.apsSecure = TRUE;

    // send and APSME_UPDATE_DEVICE request to the trust center
    status = APSME_UpdateDeviceReq( &req );
  }
#else
  // send the message APS encrypted
  req.apsSecure = TRUE;

  // send and APSME_UPDATE_DEVICE request to the trust center
  status = APSME_UpdateDeviceReq( &req );
#endif

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrDeviceNew
 *
 * @brief       Process a new device.
 *
 * @param       device - [in] ZDSecMgrDevice_t, device info
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrDeviceNew( ZDSecMgrDevice_t* joiner )
{
  ZStatus_t status;

  if ( ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
      || ( ( ZG_BUILD_RTR_TYPE ) && APSME_IsDistributedSecurity() ) )
  {
    // try to join this device
    status = ZDSecMgrDeviceJoinDirect( joiner );
  }
  else
  {
    status = ZDSecMgrDeviceJoinFwd( joiner );
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAssocDeviceAuth
 *
 * @brief       Set associated device status to authenticated
 *
 * @param       assoc - [in, out] associated_devices_t
 *
 * @return      none
 */
void ZDSecMgrAssocDeviceAuth( associated_devices_t* assoc )
{
  if ( assoc != NULL )
  {
    assoc->devStatus &= ~DEV_SEC_AUTH_TC_REJOIN_STATUS;
    assoc->devStatus |= DEV_SEC_AUTH_STATUS;
  }
}

/******************************************************************************
 * @fn          ZDSecMgrAuthNwkKey
 *
 * @brief       Handle next step in authentication process
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrAuthNwkKey()
{
  if ( devState == DEV_END_DEVICE_UNAUTH )
  {
    // inform ZDO that device has been authenticated
    OsalPort_setEvent ( ZDAppTaskID, ZDO_DEVICE_AUTH );
  }
}

/******************************************************************************
 * PUBLIC FUNCTIONS
 */
/******************************************************************************
 * @fn          ZDSecMgrInit
 *
 * @brief       Initialize ZigBee Device Security Manager.
 *
 * @param       state - device initialization state
 *
 * @return      none
 */
#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
void ZDSecMgrAddrMgrCB( uint8_t update, AddrMgrEntry_t* newEntry, AddrMgrEntry_t* oldEntry );
void ZDSecMgrAddrMgrCB( uint8_t           update,
                        AddrMgrEntry_t* newEntry,
                        AddrMgrEntry_t* oldEntry )
{
  (void)update;
  (void)newEntry;
  (void)oldEntry;
}
#endif // ( ADDRMGR_CALLBACK_ENABLED == 1 )

void ZDSecMgrInit(uint8_t state)
{
  // initialize sub modules
  ZDSecMgrEntryInit(state);

  if ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
  {
    APSME_SetRequest( apsTrustCenterAddress, 0, NLME_GetExtAddr() );
  }

  // register with Address Manager
#if ( ADDRMGR_CALLBACK_ENABLED == 1 )
  AddrMgrRegister( ADDRMGR_REG_SECURITY, ZDSecMgrAddrMgrCB );
#endif


  if ( ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
         || ( ( ZG_BUILD_RTR_TYPE ) && APSME_IsDistributedSecurity() ) )
  {
    // setup joining permissions
    ZDSecMgrPermitJoiningEnabled = TRUE;
    ZDSecMgrPermitJoiningTimed   = FALSE;
  }

  // configure security based on security mode and type of device
  ZDSecMgrConfig();
}

/******************************************************************************
 * @fn          ZDSecMgrConfig
 *
 * @brief       Configure ZigBee Device Security Manager.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrConfig( void )
{
  SSP_Init();

  if ( ( ZG_BUILD_COORDINATOR_TYPE ) && ( ZG_DEVICE_COORDINATOR_TYPE ) )
  {
    // COMMERCIAL MODE - COORDINATOR DEVICE
    APSME_SecurityCM_CD();
  }
  else if ( ZSTACK_ROUTER_BUILD )
  {
    // COMMERCIAL MODE - ROUTER DEVICE
    APSME_SecurityCM_RD();
  }
  else
  {
    // COMMERCIAL MODE - END DEVICE
    APSME_SecurityCM_ED();
  }
}

/******************************************************************************
 * @fn          ZDSecMgrPermitJoining
 *
 * @brief       Process request to change joining permissions.
 *
 * @param       duration - [in] timed duration for join in seconds
 *                         - 0x00 not allowed
 *                         - 0xFF allowed without timeout
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
uint8_t ZDSecMgrPermitJoining( uint8_t duration )
{
  uint8_t accept;

  ZDSecMgrPermitJoiningTimed = FALSE;

  if ( duration > 0 )
  {
    ZDSecMgrPermitJoiningEnabled = TRUE;

    ZDSecMgrPermitJoiningTimed = TRUE;
  }
  else
  {
    ZDSecMgrPermitJoiningEnabled = FALSE;
  }

  accept = TRUE;

  return accept;
}

/******************************************************************************
 * @fn          ZDSecMgrPermitJoiningTimeout
 *
 * @brief       Process permit joining timeout
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrPermitJoiningTimeout( void )
{
  if ( ZDSecMgrPermitJoiningTimed == TRUE )
  {
    ZDSecMgrPermitJoiningEnabled = FALSE;
    ZDSecMgrPermitJoiningTimed   = FALSE;
  }
}

/******************************************************************************
 * @fn          ZDSecMgrNewDeviceEvent
 *
 * @brief       Process a the new device event, if found reset new device
 *              event/timer.
 *
 * @param       ShortAddr - of New Device to process
 *
 * @return      uint8_t - found(TRUE:FALSE)
 */
uint8_t ZDSecMgrNewDeviceEvent( uint16_t ShortAddr )
{
  uint8_t                 found;
  ZDSecMgrDevice_t      device;
  AddrMgrEntry_t        addrEntry;
  associated_devices_t* assoc;
  ZStatus_t             status;

  // initialize return results
  found = FALSE;

  assoc = AssocGetWithShort( ShortAddr );

  if ( assoc != NULL )
  {
    // device found
    found = TRUE;

    // check for preconfigured security
    if ( zgPreConfigKeys == TRUE )
    {
      // set association status to authenticated
      ZDSecMgrAssocDeviceAuth( assoc );
    }

    // set up device info
    addrEntry.user  = ADDRMGR_USER_DEFAULT;
    addrEntry.index = assoc->addrIdx;
    AddrMgrEntryGet( &addrEntry );

    device.nwkAddr    = assoc->shortAddr;
    device.extAddr    = addrEntry.extAddr;
    device.parentAddr = NLME_GetShortAddr();

    // the new device performed Secured Rejoin
    if ( ( assoc->devStatus & DEV_SECURED_JOIN ) &&
         ( assoc->devStatus & DEV_REJOIN_STATUS ) )
    {
      device.secure     = TRUE;
    }
    else
    {
      device.secure     = FALSE;
    }
    device.devStatus  = assoc->devStatus;

    // process new device
    status = ZDSecMgrDeviceNew( &device );

    if ( status == ZSuccess )
    {
      assoc->devStatus &= ~DEV_SEC_INIT_STATUS;
    }
    else
    {
      // Clear SECURITY bit from Address Manager
      ZDSecMgrAddrClear( addrEntry.extAddr );

      // Remove the Association completely
      AssocRemove( addrEntry.extAddr );
    }
  }

  return found;
}

/******************************************************************************
 * @fn          ZDSecMgrTCExtAddrCheck
 *
 * @brief       Verifies if received ext. address matches TC ext. address.
 *
 * @param       extAddr - Extended address to be verified.
 *
 * @return      TRUE - extended address matches
 *              FALSE - otherwise
 */
uint8_t ZDSecMgrTCExtAddrCheck( uint8_t* extAddr )
{
  uint8_t  lookup[Z_EXTADDR_LEN];
  APSME_GetRequest( apsTrustCenterAddress, 0, lookup );
  return osal_ExtAddrEqual( extAddr, lookup );
}

/******************************************************************************
 * @fn          ZDSecMgrTCDataLoad
 *
 * @brief       Stores the address of TC into address manager.
 *
 * @param       extAddr - Extended address to be verified.
 *
 * @return      none
 */
void ZDSecMgrTCDataLoad( uint8_t* extAddr )
{
  uint16_t ami;
  AddrMgrEntry_t entry;

  // lookup using TC short address
  entry.user    = ADDRMGR_USER_DEFAULT;
  osal_cpyExtAddr( entry.extAddr, extAddr );

  // Verify if TC address has been added to Address Manager
  if ( !APSME_IsDistributedSecurity() && ( AddrMgrEntryLookupExt( &entry ) != TRUE ) )
  {
    ZDSecMgrAddrStore( APSME_TRUSTCENTER_NWKADDR, extAddr, &ami );
  }
}

/******************************************************************************
 * @fn          ZDSecMgrTransportKeyInd
 *
 * @brief       Process the ZDO_TransportKeyInd_t message.
 *
 * @param       ind - [in] ZDO_TransportKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrTransportKeyInd( ZDO_TransportKeyInd_t* ind )
{
  uint8_t index;
  uint8_t zgPreConfigKey[SEC_KEY_LEN];

  ZDSecMgrUpdateTCAddress( ind->srcExtAddr );

#if ZG_BUILD_JOINING_TYPE
  if(ZG_DEVICE_JOINING_TYPE)
  {
    //Update the TC address in the entry
    APSME_TCLinkKeyNVEntry_t TCLKDevEntryCpy;
    uint8_t status;
    uint16_t entryIndex;
    uint8_t keyAttributes = ZG_PROVISIONAL_KEY;

    // Find TCLK entry with keyAttribute ZG_PROVISIONAL_KEY
    status = osal_nv_read_match_entry( ZCD_NV_EX_TCLK_TABLE, &entryIndex, 0,
                                       sizeof(APSME_TCLinkKeyNVEntry_t),
                                       &TCLKDevEntryCpy, sizeof(TCLKDevEntryCpy.keyAttributes),
          (uint16_t)((uint32_t)&TCLKDevEntryCpy.keyAttributes - (uint32_t)&TCLKDevEntryCpy),
                                       &keyAttributes );

    // If entry is found with keyAttribute ZG_PROVISIONAL_KEY, update extAddr and write to NV
    if(status == SUCCESS)
    {
        OsalPort_memcpy(TCLKDevEntryCpy.extAddr, ind->srcExtAddr, Z_EXTADDR_LEN);

        //Save the KeyAttribute for joining device
        osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, entryIndex,
                         sizeof(APSME_TCLinkKeyNVEntry_t),
                         &TCLKDevEntryCpy);
    }
    else
    {
        // Find TCLK entry with TC extAddr or first unused entry
        uint8_t entryFound;
        entryIndex = APSME_SearchTCLinkKeyEntry(AIB_apsTrustCenterAddress,&entryFound,&TCLKDevEntryCpy);

        if(entryIndex < gZDSECMGR_TC_DEVICE_MAX)
        {
          OsalPort_memcpy(TCLKDevEntryCpy.extAddr, ind->srcExtAddr, Z_EXTADDR_LEN);

          //Save the KeyAttribute for joining device
          osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, entryIndex,
                          sizeof(APSME_TCLinkKeyNVEntry_t),
                          &TCLKDevEntryCpy);
        }
    }

}
#endif

  // check for distributed security
  if ( ( ZG_BUILD_RTR_TYPE ) && OsalPort_isBufSet( ind->srcExtAddr, 0xFF, Z_EXTADDR_LEN ) )
  {
    ZDSecMgrPermitJoiningEnabled = TRUE;
  }

  // load Trust Center data if needed
  ZDSecMgrTCDataLoad( ind->srcExtAddr );

  if ( ( ind->keyType == KEY_TYPE_NWK ) ||
       ( ind->keyType == 6            ) )
  {
    // check for dummy NWK key (all zeros)
    for ( index = 0;
          ( (index < SEC_KEY_LEN) && (ind->key[index] == 0) );
          index++ );

    if ( index == SEC_KEY_LEN )
    {
      // load preconfigured key - once!!
      if ( !_NIB.nwkKeyLoaded )
      {
        ZDSecMgrReadKeyFromNv(ZCD_NV_EX_LEGACY, ZCD_NV_PRECFGKEY, zgPreConfigKey);
        SSP_UpdateNwkKey( zgPreConfigKey, 0 );
        SSP_SwitchNwkKey( 0 );

        // clear local copy of key
        memset(zgPreConfigKey, 0x00, SEC_KEY_LEN);
      }
    }
    else
    {
      SSP_UpdateNwkKey( ind->key, ind->keySeqNum );
      // only use this key if we don't currently have a key, or if
      // we are performing a Trust Center Rejoin
      if ( (!_NIB.nwkKeyLoaded) || (bdb_performingTCRejoin == TRUE) )
      {
        SSP_SwitchNwkKey( ind->keySeqNum );
        bdb_performingTCRejoin = FALSE;
      }
    }

    // handle next step in authentication process
    ZDSecMgrAuthNwkKey();
  }
  else if ( ind->keyType == KEY_TYPE_TC_LINK )
  {
    uint16_t entryIndex;
    uint8_t found;
    APSME_TCLinkKeyNVEntry_t TCLKDevEntry;

    // check to see if we are expecting a new Trust Center Link Key
    if ( bdb_acceptNewTrustCenterLinkKey == TRUE )
    {
      //Search the entry, which should exist at this point
      entryIndex = APSME_SearchTCLinkKeyEntry(ind->srcExtAddr, &found, &TCLKDevEntry);

      if(found)
      {
        TCLKDevEntry.keyAttributes = ZG_UNVERIFIED_KEY;
        TCLKDevEntry.keyType = ZG_UNIQUE_LINK_KEY;
        TCLKDevEntry.rxFrmCntr = 0;
        TCLKDevEntry.txFrmCntr = 0;
        TCLKDevEntry.SeedShift = 0;

        //Update the entry
        osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE,
                         entryIndex,
                         sizeof(APSME_TCLinkKeyNVEntry_t),
                         &TCLKDevEntry);

        //Create the entry for the key
        if( SUCCESS == osal_nv_item_init(ZCD_NV_TCLK_JOIN_DEV,SEC_KEY_LEN,ind->key) )
        {
          //Or replace it if already existed
          osal_nv_write(ZCD_NV_TCLK_JOIN_DEV, SEC_KEY_LEN, ind->key);
        }

        bdb_acceptNewTrustCenterLinkKey = FALSE;
        bdb_tcLinkKeyExchangeAttempt(TRUE,BDB_REQ_VERIFY_TC_LINK_KEY);
      }
    }
  }
  else if ( ind->keyType == KEY_TYPE_APP_LINK )
  {
    uint16_t           ami;
    ZDSecMgrEntry_t* entry;

    // get the address index
    if ( ZDSecMgrExtAddrLookup( ind->srcExtAddr, &ami ) != ZSuccess )
    {
      // store new EXT address
      ZDSecMgrAddrStore( INVALID_NODE_ADDR, ind->srcExtAddr, &ami );
      ZDP_NwkAddrReq( ind->srcExtAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
    }

    ZDSecMgrEntryLookupAMI( ami, &entry );

    if ( entry == NULL )
    {
      // get new entry
      if ( ZDSecMgrEntryNew( &entry ) == ZSuccess )
      {
        // finish setting up entry
        entry->ami = ami;
      }
    }

    ZDSecMgrLinkKeySet( ind->srcExtAddr, ind->key );

#if defined NV_RESTORE
    ZDSecMgrWriteNV();  // Write the control record for the new established link key to NV.
#endif
  }
}

/******************************************************************************
 * @fn          ZDSecMgrUpdateDeviceInd
 *
 * @brief       Process the ZDO_UpdateDeviceInd_t message.
 *
 * @param       ind - [in] ZDO_UpdateDeviceInd_t indication
 *
 * @return      none
 */
void ZDSecMgrUpdateDeviceInd( ZDO_UpdateDeviceInd_t* ind )
{
#if (ZG_BUILD_COORDINATOR_TYPE)
  uint8_t tcJoin = FALSE;
#endif
  ZDSecMgrDevice_t device;

  device.nwkAddr    = ind->devAddr;
  device.extAddr    = ind->devExtAddr;
  device.parentAddr = ind->srcAddr;
  device.devStatus  = DEV_SEC_INIT_STATUS;
  device.secure     = FALSE;

  // Trust Center should identify the type of JOIN/REJOIN and
  // Transport the NWK key accordingly, it will only be transported for:
  //              APSME_UD_STANDARD_UNSECURED_JOIN
  //   OR         APSME_UD_STANDARD_TRUST_CENTER_REJOIN
  if ( ind->status != APSME_UD_DEVICE_LEFT )
  {
    if ( ind->status == APSME_UD_STANDARD_SECURED_REJOIN )
    {
      device.devStatus &= ~DEV_SEC_INIT_STATUS;
      device.devStatus |=  DEV_SEC_AUTH_STATUS;
      device.secure = TRUE;
    }
    else if ( ind->status == APSME_UD_STANDARD_TRUST_CENTER_REJOIN )
    {
      device.devStatus &= ~DEV_SEC_INIT_STATUS;
      device.devStatus |=  DEV_SEC_AUTH_TC_REJOIN_STATUS;
      device.secure = FALSE;
    }
    else
    {
#if (ZG_BUILD_COORDINATOR_TYPE)
      uint8_t  found;
      uint16_t keyNvIndex, index;
      APSME_TCLinkKeyNVEntry_t TCLKDevEntry;

      keyNvIndex = APSME_SearchTCLinkKeyEntry(device.extAddr,&found, &TCLKDevEntry);

      //If found and it was verified, then allow it to join in a fresh state by erasing the key entry
      if((found == TRUE) && (TCLKDevEntry.keyAttributes == ZG_VERIFIED_KEY))
      {
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
        //Increase the shift by one. Validate the maximum shift of the seed which is 15
        TCLKDevEntry.SeedShift++;
        TCLKDevEntry.SeedShift &= 0x0F;

        TCLKDevEntry.rxFrmCntr = 0;
        TCLKDevEntry.txFrmCntr = 0;

        index = keyNvIndex;
        TCLinkKeyRAMEntry[index].rxFrmCntr = 0;
        TCLinkKeyRAMEntry[index].txFrmCntr = 0;

        //Update the entry
        osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, keyNvIndex,
                         sizeof(APSME_TCLinkKeyNVEntry_t),
                         &TCLKDevEntry);
      }

      tcJoin = TRUE;
#endif
    }

    if( ZSuccess == ZDSecMgrDeviceJoin( &device ) )
    {
#if (ZG_BUILD_COORDINATOR_TYPE)
      if( tcJoin == TRUE )
      {
        bdb_TCAddJoiningDevice(device.parentAddr,device.extAddr);
      }
#endif
    }
  }
  else
  {
    // remove the TCLK NV entry for a device which has left the network
#if (ZG_BUILD_COORDINATOR_TYPE)
    if ( zgClearTCLKOnDeviceLeft == TRUE )
    {
      uint8_t  found;
      uint16_t keyNvIndex, index;
      APSME_TCLinkKeyNVEntry_t TCLKDevEntry;

      keyNvIndex = APSME_SearchTCLinkKeyEntry(device.extAddr,&found, &TCLKDevEntry);

      //If found and it was verified, erase the key entry
      if((found == TRUE) && (TCLKDevEntry.keyAttributes == ZG_VERIFIED_KEY))
      {
        memset(&TCLKDevEntry,0,sizeof(APSME_TCLinkKeyNVEntry_t));
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;

        index = keyNvIndex;
        TCLinkKeyRAMEntry[index].rxFrmCntr = 0;
        TCLinkKeyRAMEntry[index].txFrmCntr = 0;
        TCLinkKeyRAMEntry[index].entryUsed = FALSE;

        //Update the entry
        osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, keyNvIndex,
                         sizeof(APSME_TCLinkKeyNVEntry_t),
                         &TCLKDevEntry);
      }
    }
#endif
  }
}

/******************************************************************************
 * @fn          ZDSecMgrRemoveDeviceInd
 *
 * @brief       Process the ZDO_RemoveDeviceInd_t message.
 *
 * @param       ind - [in] ZDO_RemoveDeviceInd_t indication
 *
 * @return      none
 */
void ZDSecMgrRemoveDeviceInd( ZDO_RemoveDeviceInd_t* ind )
{
  ZDSecMgrDevice_t device;

  // only accept from Trust Center
  if ( ind->srcAddr == APSME_TRUSTCENTER_NWKADDR )
  {
    // look up NWK address
    if ( APSME_LookupNwkAddr( ind->childExtAddr, &device.nwkAddr ) == TRUE )
    {
      device.parentAddr = NLME_GetShortAddr();
      device.extAddr    = ind->childExtAddr;

      // remove device
      ZDSecMgrDeviceRemove( &device );
    }
  }
}

/******************************************************************************
 * @fn          ZDSecMgrRequestKeyInd
 *
 * @brief       Process the ZDO_RequestKeyInd_t message.
 *
 * @param       ind - [in] ZDO_RequestKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrRequestKeyInd( ZDO_RequestKeyInd_t* ind )
{
  if ( ind->keyType == KEY_TYPE_NWK )
  {
  }
  else if ( ind->keyType == KEY_TYPE_APP_MASTER )
  {
    ZDSecMgrAppKeyReq( ind );
  }
  else if ( ind->keyType == KEY_TYPE_TC_LINK )
  {
    ZDSecMgrTclkReq( ind );
  }
  //else ignore
}


/******************************************************************************
 * @fn          ZDSecMgrVerifyKeyInd
 *
 * @brief       Process the ZDO_VerifyKeyInd_t message.
 *
 * @param       ind - [in] ZDO_VerifyKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrVerifyKeyInd( ZDO_VerifyKeyInd_t* ind )
{
  ZDSecMgrAppConfKeyReq( ind );
}


/******************************************************************************
 * @fn          ZDSecMgrSwitchKeyInd
 *
 * @brief       Process the ZDO_SwitchKeyInd_t message.
 *
 * @param       ind - [in] ZDO_SwitchKeyInd_t indication
 *
 * @return      none
 */
void ZDSecMgrSwitchKeyInd( ZDO_SwitchKeyInd_t* ind )
{
  SSP_SwitchNwkKey( ind->keySeqNum );

  // Save if nv
  ZDApp_NVUpdate();
}

/******************************************************************************
 * @fn          ZDSecMgrGenerateSeed
 *
 * @brief       Generate the seed for TC link keys and store it in Nv
 *
 * @param       SetDefault, force to use new seed
 *
 * @return      none
 */
void ZDSecMgrGenerateSeed(uint8_t SetDefault)
{
  uint8_t SeedKey[SEC_KEY_LEN];

  ZDSecMgrGenerateRndKey(SeedKey);

  if((SUCCESS == osal_nv_item_init(ZCD_NV_TCLK_SEED,SEC_KEY_LEN,SeedKey)) && SetDefault)
  {
    //Force to use a new seed
    osal_nv_write(ZCD_NV_TCLK_SEED, SEC_KEY_LEN, SeedKey);
  }

  memset(SeedKey,0,SEC_KEY_LEN);
}


/******************************************************************************
 * @fn          ZDSecMgrGenerateKeyFromSeed
 *
 * @brief       Generate the TC link key for an specific device using seed and ExtAddr
 *
 * @param       [in]  extAddr
 * @param       [in]  shift    number of byte shifts that the seed will do to
 *                             generate a new key for the same device.
 *                             This value must be less than SEC_KEY_LEN
 * @param       [out] key      buffer in which the key will be copied
 *
 * @return      none
 */
void ZDSecMgrGenerateKeyFromSeed(uint8_t *extAddr, uint8_t shift, uint8_t *key)
{
  uint8_t i;
  uint8_t tempKey[SEC_KEY_LEN];
  //shift must be less than SEC_KEY_LEN. This is to handle the cases where SeedShift is >= SEC_KEY_LEN.
  shift &= 0x0F;

  if((key != NULL) && (extAddr != NULL))
  {
    //Read the key
    osal_nv_read(ZCD_NV_TCLK_SEED,0,SEC_KEY_LEN,tempKey);

    //shift the seed
    OsalPort_memcpy(key, &tempKey[shift], SEC_KEY_LEN - shift);
    OsalPort_memcpy(&key[SEC_KEY_LEN - shift], tempKey, shift);

    //Create the key from the seed
    for(i = 0; i < Z_EXTADDR_LEN; i++)
    {
      key[i] ^= extAddr[i];
      key[i+Z_EXTADDR_LEN] ^= extAddr[i];
    }
  }
}


/******************************************************************************
 * @fn          ZDSecMgrGenerateRndKey
 *
* @brief       Generate a random key using the TI-RTOS TRNG driver.
 *
 * @param       pKey - [out] Buffer pointer in which the key will be passed.
 *
 * @return      none
 */
void ZDSecMgrGenerateRndKey(uint8_t* pKey)
{
  extern TRNG_Handle TRNG_handle;

  int_fast16_t result;

  CryptoKey entropyKey;
  uint8_t entropyBuffer[SEC_KEY_LEN];

  CryptoKeyPlaintext_initBlankKey(&entropyKey, entropyBuffer, SEC_KEY_LEN);

  result = TRNG_generateEntropy(TRNG_handle, &entropyKey);

  if (result != TRNG_STATUS_SUCCESS) {
      // Handle error
  }

  OsalPort_memcpy(pKey, &entropyKey.u.plaintext.keyMaterial[0], SEC_KEY_LEN);
}


#if ( ZG_BUILD_COORDINATOR_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrUpdateNwkKey
 *
 * @brief       Load a new NWK key and trigger a network update to the dstAddr.
 *
 * @param       key       - [in] new NWK key
 * @param       keySeqNum - [in] new NWK key sequence number
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrUpdateNwkKey( uint8_t* key, uint8_t keySeqNum, uint16_t dstAddr )
{
  ZStatus_t               status;
  APSME_TransportKeyReq_t req;

  // initialize common elements of local variables
  req.keyType   = KEY_TYPE_NWK;

  req.dstAddr   = dstAddr;
  req.keySeqNum = keySeqNum;
  req.key       = key;
  req.extAddr   = NULL;
  req.nwkSecure = TRUE;
  req.tunnel    = NULL;


  // Broadcast transport NWK key
  if (( dstAddr == NWK_BROADCAST_SHORTADDR_DEVALL ) ||
      ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVZCZR) ||
      ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVRXON))
  {
    req.apsSecure = FALSE;
    status = APSME_TransportKeyReq( &req );
  }
  else
  {
    AddrMgrEntry_t          addrEntry;

    addrEntry.user = ADDRMGR_USER_SECURITY;
    addrEntry.nwkAddr = dstAddr;

    status = ZFailure;

    if ( AddrMgrEntryLookupNwk( &addrEntry ) == TRUE )
    {
      req.extAddr = addrEntry.extAddr;
      req.apsSecure = TRUE;
      status = APSME_TransportKeyReq( &req );
    }
  }


  SSP_UpdateNwkKey( key, keySeqNum );

  // Save if nv
  ZDApp_NVUpdate();

  return status;
}
#endif // ( ZG_BUILD_COORDINATOR_TYPE )

#if ( ZG_BUILD_COORDINATOR_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrSwitchNwkKey
 *
 * @brief       Causes the NWK key to switch via a network command to the dstAddr.
 *
 * @param       keySeqNum - [in] new NWK key sequence number
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSwitchNwkKey( uint8_t keySeqNum, uint16_t dstAddr )
{
  ZStatus_t            status;
  APSME_SwitchKeyReq_t req;

  // initialize common elements of local variables
  req.dstAddr = dstAddr;
  req.keySeqNum = keySeqNum;

  // Broadcast switch NWK key
  if (( dstAddr == NWK_BROADCAST_SHORTADDR_DEVALL ) ||
      ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVZCZR) ||
      ( dstAddr == NWK_BROADCAST_SHORTADDR_DEVRXON))
  {
    req.apsSecure = FALSE;
    status = APSME_SwitchKeyReq( &req );
  }
  else
  {
    AddrMgrEntry_t          addrEntry;

    addrEntry.user = ADDRMGR_USER_SECURITY;
    addrEntry.nwkAddr = dstAddr;

    status = ZFailure;

    if ( AddrMgrEntryLookupNwk( &addrEntry ) == TRUE )
    {
      req.dstAddr = addrEntry.nwkAddr;
      req.apsSecure = TRUE;
      status = APSME_SwitchKeyReq( &req );
    }
  }


  if ( dstAddr >= NWK_BROADCAST_SHORTADDR_DEVZCZR)
  {
    zgSwitchCoordKey = TRUE;
    zgSwitchCoordKeyIndex = keySeqNum;
  }
  // Save if nv
  ZDApp_NVUpdate();

  return status;
}
#endif // ( ZG_BUILD_COORDINATOR_TYPE )

/******************************************************************************
 * @fn          ZDSecMgrRequestAppKey
 *
 * @brief       Request an application key with partner.
 *
 * @param       partExtAddr - [in] partner extended address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrRequestAppKey( uint8_t *partExtAddr )
{
  ZStatus_t status;
  APSME_RequestKeyReq_t req;

  req.dstAddr = 0;
  req.keyType = KEY_TYPE_APP_MASTER;

  req.partExtAddr = partExtAddr;
  status = APSME_RequestKeyReq( &req );

  return status;
}

#if ( ZG_BUILD_JOINING_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrSetupPartner
 *
 * @brief       Setup for application key partner.
 *
 * @param       partNwkAddr - [in] partner network address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrSetupPartner( uint16_t partNwkAddr, uint8_t* partExtAddr )
{
  AddrMgrEntry_t entry;
  ZStatus_t      status;

  status = ZFailure;

  // update the address manager
  entry.user    = ADDRMGR_USER_SECURITY;
  entry.nwkAddr = partNwkAddr;
  AddrMgrExtAddrSet( entry.extAddr, partExtAddr );

  if ( AddrMgrEntryUpdate( &entry ) == TRUE )
  {
    status = ZSuccess;

    // check for address discovery
    if ( partNwkAddr == INVALID_NODE_ADDR )
    {
      status = ZDP_NwkAddrReq( partExtAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
    }
    else if ( !AddrMgrExtAddrValid( partExtAddr ) )
    {
      status = ZDP_IEEEAddrReq( partNwkAddr, ZDP_ADDR_REQTYPE_SINGLE, 0, 0 );
    }
  }

  return status;
}
#endif // ( ZG_BUILD_JOINING_TYPE )

#if ( ZG_BUILD_COORDINATOR_TYPE )
/******************************************************************************
 * @fn          ZDSecMgrAppKeyTypeSet
 *
 * @brief       Set application key type.
 *
 * @param       keyType - [in] application key type (KEY_TYPE_APP_MASTER@2 or
 *                                                   KEY_TYPE_APP_LINK@3
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAppKeyTypeSet( uint8_t keyType )
{
  if ( keyType == KEY_TYPE_APP_LINK )
  {
    ZDSecMgrAppKeyType = KEY_TYPE_APP_LINK;
  }
  else
  {
    ZDSecMgrAppKeyType = KEY_TYPE_APP_MASTER;
  }

  return ZSuccess;
}
#endif

/******************************************************************************
 * ZigBee Device Security Manager - Stub Implementations
 */


/******************************************************************************
 * @fn          ZDSecMgrLinkKeySet (stubs APSME_LinkKeySet)
 *
 * @brief       Set <APSME_ApsLinkKeyNVEntry_t> for specified NWK address.
 *
 * @param       extAddr - [in] EXT address
 * @param       data    - [in] APSME_ApsLinkKeyNVEntry_t
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrLinkKeySet( uint8_t* extAddr, uint8_t* key )
{
  ZStatus_t status;
  ZDSecMgrEntry_t* entry;
  APSME_ApsLinkKeyNVEntry_t *pApsLinkKey = NULL;
  uint16_t Index;

  // lookup entry index for specified EXT address
  status = ZDSecMgrEntryLookupExtGetIndex( extAddr, &entry, &Index );

  if ( status == ZSuccess )
  {
    // point to NV item
    entry->keyNvId = Index;

    pApsLinkKey = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

    if (pApsLinkKey != NULL)
    {
      // read the key form NV
      osal_nv_read_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, entry->keyNvId, 0,
                       sizeof(APSME_ApsLinkKeyNVEntry_t),
                       pApsLinkKey );

      // set new values of the key
      OsalPort_memcpy( pApsLinkKey->key, key, SEC_KEY_LEN );
      pApsLinkKey->rxFrmCntr = 0;
      pApsLinkKey->txFrmCntr = 0;

      osal_nv_write_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, entry->keyNvId,
                        sizeof(APSME_ApsLinkKeyNVEntry_t),
                        pApsLinkKey );

      // clear copy of key in RAM
      memset(pApsLinkKey, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t));

      OsalPort_free(pApsLinkKey);

      // set initial values for counters in RAM
      ApsLinkKeyRAMEntry[entry->keyNvId].txFrmCntr = 0;
      ApsLinkKeyRAMEntry[entry->keyNvId].rxFrmCntr = 0;
    }
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAuthenticationSet
 *
 * @brief       Mark the specific device as authenticated or not
 *
 * @param       extAddr - [in] EXT address
 * @param       option  - [in] authenticated or not
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAuthenticationSet( uint8_t* extAddr, ZDSecMgr_Authentication_Option option )
{
  ZStatus_t        status;
  ZDSecMgrEntry_t* entry;


  // lookup entry index for specified EXT address
  status = ZDSecMgrEntryLookupExt( extAddr, &entry );

  if ( status == ZSuccess )
  {
    entry->authenticateOption = option;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrAuthenticationCheck
 *
 * @brief       Check if the specific device has been authenticated or not
 *              For non-trust center device, always return TRUE
 *
 * @param       shortAddr - [in] short address
 *
 * @return      TRUE @ authenticated with CBKE
 *              FALSE @ not authenticated
 */

uint8_t ZDSecMgrAuthenticationCheck( uint16_t shortAddr )
{
#if defined (TC_LINKKEY_JOIN)

  ZDSecMgrEntry_t* entry;
  uint8_t extAddr[Z_EXTADDR_LEN];

  // If the local device is not the trust center, always return TRUE
  APSME_GetRequest( apsTrustCenterAddress, 0, extAddr );
  if ( ! osal_ExtAddrEqual( extAddr , NLME_GetExtAddr() ) )
  {
    return TRUE;
  }
  // Otherwise, check the authentication option
  else if ( AddrMgrExtAddrLookup( shortAddr, extAddr ) )
  {
    // lookup entry index for specified EXT address
    if ( ZDSecMgrEntryLookupExt( extAddr, &entry ) == ZSuccess )
    {
      if ( entry->authenticateOption != ZDSecMgr_Not_Authenticated )
      {
        return TRUE;
      }
      else
      {
        return FALSE;
      }
    }
    else
    {
      // it may have been secured with TCLK only
      uint16_t    ami;

      // lookup address index in address manager
      if ( ZDSecMgrExtAddrLookup( extAddr, &ami ) == ZSuccess )
      {
        return TRUE;
      }
    }
  }
  return FALSE;

#else
  (void)shortAddr;  // Intentionally unreferenced parameter

  // For non AMI/SE Profile, perform no check and always return TRUE.
  return TRUE;

#endif // TC_LINKKEY_JOIN
}


/******************************************************************************
 * @fn          ZDSecMgrLinkKeyNVIdGet (stubs APSME_LinkKeyNVIdGet)
 *
 * @brief       Get Key NV ID for specified NWK address.
 *
 * @param       extAddr - [in] EXT address
 * @param       keyNvId - [out] NV ID
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrLinkKeyNVIdGet(uint8_t* extAddr, uint16_t *pKeyNvId)
{
  ZStatus_t status;
  ZDSecMgrEntry_t* entry;

  // lookup entry index for specified NWK address
  status = ZDSecMgrEntryLookupExt( extAddr, &entry );

  if ( status == ZSuccess )
  {
    // return the index to the NV table
    *pKeyNvId = entry->keyNvId;
  }
  else
  {
    *pKeyNvId = SEC_NO_KEY_NV_ID;
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrIsLinkKeyValid (stubs APSME_IsLinkKeyValid)
 *
 * @brief       Verifies if Link Key in NV has been set.
 *
 * @param       extAddr - [in] EXT address
 *
 * @return      TRUE - Link Key has been established
 *              FALSE - Link Key in NV has default value.
 */
uint8_t ZDSecMgrIsLinkKeyValid(uint8_t* extAddr)
{
  APSME_ApsLinkKeyNVEntry_t *pKeyData;
  uint16_t apsLinkKeyNvId;
  uint8_t nullKey[SEC_KEY_LEN];
  uint8_t status = FALSE;

  // initialize default vealue to compare to
  memset(nullKey, 0x00, SEC_KEY_LEN);

  // check for APS link NV ID
  APSME_LinkKeyNVIdGet( extAddr, &apsLinkKeyNvId );

  if (apsLinkKeyNvId != SEC_NO_KEY_NV_ID )
  {
    pKeyData = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

    if (pKeyData != NULL)
    {
      // retrieve key from NV
      if ( osal_nv_read( apsLinkKeyNvId, 0,
                        sizeof(APSME_ApsLinkKeyNVEntry_t), pKeyData) == ZSUCCESS)
      {
        // if stored key is different than default value, then a key has been established
        if (!OsalPort_memcmp(pKeyData, nullKey, SEC_KEY_LEN))
        {
          status = TRUE;
        }
      }

      // clear copy of key in RAM
      memset(pKeyData, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t));

      OsalPort_free(pKeyData);
    }
  }

  return status;
}

/******************************************************************************
 * @fn          ZDSecMgrKeyFwdToChild (stubs APSME_KeyFwdToChild)
 *
 * @brief       Verify and process key transportation to child.
 *
 * @param       ind - [in] APSME_TransportKeyInd_t
 *
 * @return      uint8_t - success(TRUE:FALSE)
 */
uint8_t ZDSecMgrKeyFwdToChild( APSME_TransportKeyInd_t* ind )
{
  // verify from Trust Center
  if ( ind->srcAddr == APSME_TRUSTCENTER_NWKADDR )
  {
    return TRUE;
  }

  return FALSE;
}

/******************************************************************************
 * @fn          ZDSecMgrAddLinkKey
 *
 * @brief       Add the application link key to ZDSecMgr. Also mark the device
 *              as authenticated in the authenticateOption. Note that this function
 *              is hardwared to CBKE right now.
 *
 * @param       shortAddr - short address of the partner device
 * @param       extAddr - extended address of the partner device
 * @param       key - link key
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAddLinkKey( uint16_t shortAddr, uint8_t *extAddr, uint8_t *key)
{
  uint16_t           ami;
  ZDSecMgrEntry_t* entry;

  /* Store the device address in the addr manager */
  if( ZDSecMgrAddrStore( shortAddr, extAddr, &ami ) != ZSuccess )
  {
    /* Adding to Addr Manager fails */
    return ZFailure;
  }

  /* Lookup entry using specified address index */
  ZDSecMgrEntryLookupAMI( ami, &entry );

  // If no existing entry, create one
  if ( entry == NULL )
  {
    if ( ZDSecMgrEntryNew( &entry ) == ZSuccess )
    {
      entry->ami = ami;
    }
    else
    {
      /* Security Manager full */
      return ZBufferFull;
    }
  }
  // Write the link key
  APSME_LinkKeySet( extAddr, key );

#if defined (TC_LINKKEY_JOIN)
  // Mark the device as authenticated.
  ZDSecMgrAuthenticationSet( extAddr, ZDSecMgr_Authenticated_CBCK );
#endif

#if defined NV_RESTORE
  ZDSecMgrWriteNV();  // Write the new established link key to NV.
#endif

  return ZSuccess;
}

/******************************************************************************
 * @fn          ZDSecMgrInitNV
 *
 * @brief       Initialize the SecMgr entry data in NV with all values set to 0
 *
 * @param       none
 *
 * @return      uint8_t - <osal_nv_item_init> return codes
 */
uint8_t ZDSecMgrInitNV(void)
{
  uint8_t *pDefaultBuf;
  uint8_t rtrn;
  uint16_t size;

  size = sizeof(nvDeviceListHdr_t) + (sizeof(ZDSecMgrEntry_t) * gZDSECMGR_ENTRY_MAX);

  // Allocate temporary buffer
  pDefaultBuf = (uint8_t *)OsalPort_malloc(size);

  if (pDefaultBuf)
  {
      memset( pDefaultBuf, 0x00, size );

      rtrn = osal_nv_item_init(ZCD_NV_APS_LINK_KEY_TABLE, size, pDefaultBuf);

      // If the item is uninitialized already exist, set all values to 0
      if (rtrn == NV_ITEM_UNINIT)
      {
        osal_nv_write(ZCD_NV_APS_LINK_KEY_TABLE, size, pDefaultBuf);
      }

      OsalPort_free(pDefaultBuf);

      rtrn |= osal_nv_item_init( ZCD_NV_TRUSTCENTER_ADDR, Z_EXTADDR_LEN,
                                 zgApsTrustCenterAddr );
  }
  else {
      rtrn = NV_OPER_FAILED;
  }

  return rtrn;
}

#if defined ( NV_RESTORE )
/*********************************************************************
 * @fn      ZDSecMgrWriteNV()
 *
 * @brief   Save off the APS link key list to NV
 *
 * @param   none
 *
 * @return  none
 */
static void ZDSecMgrWriteNV( void )
{
    uint16_t i;
    nvDeviceListHdr_t hdr;

    hdr.numRecs = 0;

    // Allocate temporary buffer for writing to NV in a single transaction
    uint8_t *pSecMgrTable = (uint8_t *)OsalPort_malloc(((sizeof(nvDeviceListHdr_t)) + (gZDSECMGR_ENTRY_MAX * sizeof(ZDSecMgrEntry_t))));

    if ( pSecMgrTable != NULL )
    {
        if (ZDSecMgrEntries != NULL)
        {
            for ( i = 0; i < gZDSECMGR_ENTRY_MAX; i++ )
            {
                OsalPort_memcpy(pSecMgrTable + (uint16_t)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))), &ZDSecMgrEntries[i], sizeof ( ZDSecMgrEntry_t ));

                if ( ZDSecMgrEntries[i].ami != INVALID_NODE_ADDR )
                {
                    hdr.numRecs++;
                }
            }
            // Save off the header
            OsalPort_memcpy(pSecMgrTable, &hdr, sizeof ( nvDeviceListHdr_t ));

            // Save the record to NV
            osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE, sizeof(nvDeviceListHdr_t) + (gZDSECMGR_ENTRY_MAX * sizeof(ZDSecMgrEntry_t)), pSecMgrTable );
        }

        OsalPort_free(pSecMgrTable);
    }
}
#endif // NV_RESTORE

#if defined ( NV_RESTORE )
/******************************************************************************
 * @fn          ZDSecMgrRestoreFromNV
 *
 * @brief       Restore the APS Link Key entry data from NV. It does not restore
 *              the key data itself as they remain in NV until they are used.
 *              Only list data is restored.
 *              Restore zgTrustCenterAdress from NV.
 *
 * @param       none
 *
 * @return      None.
 */
static void ZDSecMgrRestoreFromNV( void )
{
  nvDeviceListHdr_t hdr;
  APSME_ApsLinkKeyNVEntry_t *pApsLinkKey = NULL;

  if ((osal_nv_read(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr) == ZSUCCESS) &&
      ((hdr.numRecs > 0) && (hdr.numRecs <= gZDSECMGR_ENTRY_MAX)))
  {
    uint8_t x;

    pApsLinkKey = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

    for (x = 0; x < gZDSECMGR_ENTRY_MAX; x++)
    {
      if ( osal_nv_read( ZCD_NV_APS_LINK_KEY_TABLE,
                        (uint16_t)(sizeof(nvDeviceListHdr_t) + (x * sizeof(ZDSecMgrEntry_t))),
                        sizeof(ZDSecMgrEntry_t), &ZDSecMgrEntries[x] ) == SUCCESS )
      {
        // update data only for valid entries
        if ( ZDSecMgrEntries[x].ami != INVALID_NODE_ADDR )
        {
          if (pApsLinkKey != NULL)
          {
            // read the key form NV
            osal_nv_read_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, ZDSecMgrEntries[x].keyNvId, 0,
                         sizeof(APSME_ApsLinkKeyNVEntry_t), pApsLinkKey );

            // set new values for the counter
            pApsLinkKey->txFrmCntr += ( MAX_APS_FRAMECOUNTER_CHANGES + 1 );

            // restore values for counters in RAM
            ApsLinkKeyRAMEntry[ZDSecMgrEntries[x].keyNvId].txFrmCntr =
                                            pApsLinkKey->txFrmCntr;

            ApsLinkKeyRAMEntry[ZDSecMgrEntries[x].keyNvId].rxFrmCntr =
                                            pApsLinkKey->rxFrmCntr;

            osal_nv_write_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, ZDSecMgrEntries[x].keyNvId,
                          sizeof(APSME_ApsLinkKeyNVEntry_t), pApsLinkKey );

            // clear copy of key in RAM
            memset(pApsLinkKey, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t));
          }
        }
      }
    }

    if (pApsLinkKey != NULL)
    {
      OsalPort_free(pApsLinkKey);
    }
  }

  osal_nv_read( ZCD_NV_TRUSTCENTER_ADDR, 0, Z_EXTADDR_LEN, zgApsTrustCenterAddr );
}
#endif // NV_RESTORE

/*********************************************************************
 * @fn          ZDSecMgrSetDefaultNV
 *
 * @brief       Write the defaults to NV for Entry table and for APS key data table
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrSetDefaultNV( void )
{
  uint16_t i;
  nvDeviceListHdr_t hdr;
  ZDSecMgrEntry_t secMgrEntry;
  APSME_ApsLinkKeyNVEntry_t *pApsLinkKey = NULL;

  // Initialize the header
  hdr.numRecs = 0;

  memset( &secMgrEntry, 0x00, sizeof(ZDSecMgrEntry_t) );

  // Allocate temporary buffer for writing to NV in a single transaction
  uint8_t *pSecMgrTable = (uint8_t *)OsalPort_malloc(((sizeof(nvDeviceListHdr_t)) + (gZDSECMGR_ENTRY_MAX * sizeof(ZDSecMgrEntry_t))));

  if ( pSecMgrTable != NULL )
  {
      if (ZDSecMgrEntries != NULL)
      {
          for ( i = 0; i < gZDSECMGR_ENTRY_MAX; i++ )
          {
              OsalPort_memcpy(pSecMgrTable + (uint16_t)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))), &secMgrEntry, sizeof ( ZDSecMgrEntry_t ));
          }

          // Save off the header
          OsalPort_memcpy(pSecMgrTable, &hdr, sizeof ( nvDeviceListHdr_t ));

          // Save the record to NV
          osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE, sizeof(nvDeviceListHdr_t) + (gZDSECMGR_ENTRY_MAX * sizeof(ZDSecMgrEntry_t)), pSecMgrTable );
      }
      OsalPort_free(pSecMgrTable);
  }

  pApsLinkKey = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

  if (pApsLinkKey != NULL)
  {
    memset( pApsLinkKey, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t) );

    for ( i = 0; i < gZDSECMGR_ENTRY_MAX; i++ )
    {
      // Clear the record
      osal_nv_write_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, i,
                        sizeof(APSME_ApsLinkKeyNVEntry_t),
                        pApsLinkKey);
    }

    OsalPort_free(pApsLinkKey);
  }
}

#if defined ( NV_RESTORE )
/*********************************************************************
 * @fn      ZDSecMgrUpdateNV()
 *
 * @brief   Updates one entry of the APS link key table to NV
 *
 * @param   index - to the entry in security manager table
 *
 * @return  none
 */
static void ZDSecMgrUpdateNV( uint16_t index )
{
  nvDeviceListHdr_t hdr;

  if (ZDSecMgrEntries != NULL)
  {
    // Save off the record
    ZDSecMgrEntry_t secMgrEntryCpy;
    uint16_t i;

    // Allocate temporary buffer for writing to NV in a single transaction
    uint8_t *pSecMgrTable = (uint8_t *)OsalPort_malloc(((sizeof(nvDeviceListHdr_t)) + (gZDSECMGR_ENTRY_MAX * sizeof(ZDSecMgrEntry_t))));

    if ( pSecMgrTable != NULL )
    {
        osal_nv_read(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr);

        for ( i = 0; i < gZDSECMGR_ENTRY_MAX; i++ )
        {
            // Store updated NV item in local memory
            if (i == index) {
                OsalPort_memcpy(pSecMgrTable + (uint16_t)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))), &ZDSecMgrEntries[index], sizeof ( ZDSecMgrEntry_t ));
            }
            // Read item from NV
            else
            {
                osal_nv_read(ZCD_NV_APS_LINK_KEY_TABLE, (uint16_t)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))), sizeof(ZDSecMgrEntry_t), &secMgrEntryCpy);
                OsalPort_memcpy(pSecMgrTable + (uint16_t)((sizeof(nvDeviceListHdr_t)) + (i * sizeof(ZDSecMgrEntry_t))), &secMgrEntryCpy, sizeof ( ZDSecMgrEntry_t ));
            }
        }

        // Save the record to NV
        osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE, sizeof(nvDeviceListHdr_t) + (gZDSECMGR_ENTRY_MAX * sizeof(ZDSecMgrEntry_t)), pSecMgrTable );

        OsalPort_free(pSecMgrTable);
    }
  }

  if (osal_nv_read(ZCD_NV_APS_LINK_KEY_TABLE, 0, sizeof(nvDeviceListHdr_t), &hdr) == ZSUCCESS)
  {
    if (ZDSecMgrEntries && (ZDSecMgrEntries[index].ami == INVALID_NODE_ADDR))
    {
      if (hdr.numRecs > 0)
      {
        hdr.numRecs--;
      }
    }
    else
    {
      hdr.numRecs++;
    }

    // Save off the header
    osal_nv_write( ZCD_NV_APS_LINK_KEY_TABLE, sizeof( nvDeviceListHdr_t ), &hdr );
  }
}
#endif // NV_RESTORE

/******************************************************************************
 * @fn          ZDSecMgrAPSRemove
 *
 * @brief       Remove device from network.
 *
 * @param       nwkAddr - device's NWK address
 * @param       extAddr - device's Extended address
 * @param       parentAddr - parent's NWK address
 *
 * @return      ZStatus_t
 */
ZStatus_t ZDSecMgrAPSRemove( uint16_t nwkAddr, uint8_t *extAddr, uint16_t parentAddr )
{
  ZDSecMgrDevice_t device;

  if ( ( nwkAddr == INVALID_NODE_ADDR ) ||
       ( extAddr == NULL )              ||
       ( parentAddr == INVALID_NODE_ADDR ) )
  {
    return ( ZFailure );
  }

  device.nwkAddr = nwkAddr;
  device.extAddr = extAddr;
  device.parentAddr = parentAddr;

  // remove device
  ZDSecMgrDeviceRemove( &device );

  return ( ZSuccess );
}

/******************************************************************************
 * @fn          APSME_TCLinkKeyInit
 *
 * @brief       Initialize the NV table for preconfigured TC link key
 *
 *              When zgUseDefaultTCL is set to TRUE, the default preconfig
 *              Trust Center Link Key is written to NV. A single tclk is used
 *              by all devices joining the network.
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
void APSME_TCLinkKeyInit(uint8_t setDefault)
{
  APSME_TCLinkKeyNVEntry_t TCLKDevEntry;
  uint8_t                rtrn;
  uint16_t               i;

  uint8_t defaultEntry[Z_EXTADDR_LEN];
  MAP_osal_memset(defaultEntry, 0, Z_EXTADDR_LEN);

  // Clear the data for the keys
  memset( &TCLKDevEntry, 0x00, sizeof(APSME_TCLinkKeyNVEntry_t) );
  TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;

  // Initialize all NV items
  for( i = 0; i < gZDSECMGR_TC_DEVICE_MAX; i++ )
  {
    // If the item doesn't exist in NV memory, create and initialize
    // it with the default value passed in, either defaultTCLK or 0
    rtrn = osal_nv_item_init_ex( ZCD_NV_EX_TCLK_TABLE, i,
                                 sizeof(APSME_TCLinkKeyNVEntry_t),
                                 &TCLKDevEntry);

    if (rtrn == SUCCESS)
    {
      if(setDefault)
      {
        //Force to initialize the entry
        osal_nv_write_ex( ZCD_NV_EX_TCLK_TABLE, i,
                          sizeof(APSME_TCLinkKeyNVEntry_t),
                          &TCLKDevEntry );

        TCLinkKeyRAMEntry[i].txFrmCntr = 0;
        TCLinkKeyRAMEntry[i].rxFrmCntr = 0;
        TCLinkKeyRAMEntry[i].entryUsed = FALSE;
      }
      else
      {
        // set the Frame counters to 0 to existing keys in NV
        osal_nv_read_ex( ZCD_NV_EX_TCLK_TABLE, i, 0,
                         sizeof(APSME_TCLinkKeyNVEntry_t),
                         &TCLKDevEntry );

        // only update frame counters if this is not a default entry
        if( OsalPort_memcmp(TCLKDevEntry.extAddr, defaultEntry, Z_EXTADDR_LEN) == FALSE )
        {
          // increase the value stored in NV
          TCLKDevEntry.txFrmCntr += ( MAX_TCLK_FRAMECOUNTER_CHANGES + 1 );

          // set initial values for counters in RAM
          TCLinkKeyRAMEntry[i].txFrmCntr = TCLKDevEntry.txFrmCntr;
          TCLinkKeyRAMEntry[i].rxFrmCntr = TCLKDevEntry.rxFrmCntr;
          TCLinkKeyRAMEntry[i].entryUsed = TRUE;
        }

        osal_nv_write_ex( ZCD_NV_EX_TCLK_TABLE, i,
                          sizeof(APSME_TCLinkKeyNVEntry_t),
                          &TCLKDevEntry );

        // Making sure data is cleared and set to default for every key all the time
        memset( &TCLKDevEntry, 0x00, sizeof(APSME_TCLinkKeyNVEntry_t) );
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
      }
    }
  }

  if(setDefault)
  {
    //Force to erase all IC
    APSME_EraseICEntry(NULL);
  }
}


/******************************************************************************
 * @fn          APSME_TCLinkKeySync
 *
 * @brief       Sync Trust Center LINK key data.
 *
 * @param       srcAddr - [in] srcAddr
 * @param       si      - [in, out] SSP_Info_t
 *
 * @return      ZStatus_t
 */
ZStatus_t APSME_TCLinkKeySync( uint16_t srcAddr, SSP_Info_t* si )
{
  APSME_TCLinkKeyNVEntry_t  TCLKDevEntry;
  ZStatus_t             status = ZSecNoKey;
  uint32_t                *tclkRxFrmCntr;
  uint16_t                entryIndex = 0xFFFF;
  uint16_t                selectedId = gZDSECMGR_TC_DEVICE_MAX;
  uint8_t                 entryFound = FALSE;

  // Look up the IEEE address of the trust center if it's available
  if ( AddrMgrExtAddrValid( si->extAddr ) == FALSE )
  {
    APSME_LookupExtAddr( srcAddr, si->extAddr );
  }

  entryIndex = APSME_SearchTCLinkKeyEntry(si->extAddr,&entryFound,&TCLKDevEntry);

#if ZG_BUILD_JOINING_TYPE
  if(ZG_DEVICE_JOINING_TYPE && !entryFound)
  {
    uint8_t defaultEntry[Z_EXTADDR_LEN];
    memset(defaultEntry, 0, Z_EXTADDR_LEN);
    entryIndex = APSME_SearchTCLinkKeyEntry(defaultEntry,&entryFound,&TCLKDevEntry);

    // if previous call to APSME_SearchTCLinkKeyEntry() returned valid
    // entryIndex, we have an empty table entry index
    if(entryIndex != 0xFFFF)
    {
      entryFound = TRUE;
    }
  }
#endif

  if(entryFound && (entryIndex != 0xFFFF))
  {
    status = ZSuccess;

    selectedId = entryIndex;

    switch(TCLKDevEntry.keyAttributes)
    {
      case ZG_UNVERIFIED_KEY:
        #if ZG_BUILD_JOINING_TYPE
        if(ZG_DEVICE_JOINING_TYPE)
        {
          si->keyNvSection = ZCD_NV_EX_LEGACY;
          si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
          break;
        }
        #endif
      case ZG_DEFAULT_KEY:
        if(ZG_DEVICE_JOINING_TYPE)
        {
          //If default was found, then it is joining as FN, then try distributed key as well
          si->distributedKeyTry = TRUE;
        }
        si->keyNvSection = ZCD_NV_EX_LEGACY;
        si->keyNvId = ZCD_NV_TCLK_DEFAULT;
      break;
      case ZG_PROVISIONAL_KEY:
        si->keyNvSection = ZCD_NV_EX_TCLK_IC_TABLE;
        si->keyNvId = (TCLKDevEntry.IcIndex & APSME_IC_INDEX_MASK);
        //Attempt to use default keys (centralized and distributed) if IC fails?
        si->distributedKeyTry = gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;
        si->defaultKeyTry = gZDSECMGR_TC_ATTEMPT_DEFAULT_KEY;
      break;
      case ZG_VERIFIED_KEY:
      case ZG_NON_R21_NWK_JOINED:
        //Only verify the frame counter for Verified keys, or keys used with non R21 TC
        tclkRxFrmCntr = &TCLinkKeyRAMEntry[selectedId].rxFrmCntr;

        if ( si->frmCntr >= *tclkRxFrmCntr )
        {
          // update the rx frame counter
          *tclkRxFrmCntr = si->frmCntr + 1;
          status = ZSuccess;

          #if ZG_BUILD_JOINING_TYPE
          if(ZG_DEVICE_JOINING_TYPE)
          {
            si->keyNvSection = ZCD_NV_EX_LEGACY;
            si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
            break;
          }
          #endif
          #if ZG_BUILD_COORDINATOR_TYPE
          if(ZG_DEVICE_COORDINATOR_TYPE)
          {
            si->keyNvSection = ZCD_NV_EX_LEGACY;
            si->keyNvId = ZCD_NV_TCLK_SEED;
            si->seedShift = TCLKDevEntry.SeedShift;
          }
          #endif
          OsalPort_memcpy(si->dstExtAddr, TCLKDevEntry.extAddr,Z_EXTADDR_LEN);
        }
        else
        {
          status = ZSecOldFrmCount;
        }
      break;
      default:
        //This should not happen
      break;
    }
  }
  else
  {
    if (ZG_DEVICE_COORDINATOR_TYPE)
    {
      if(bdb_doTrustCenterRequireKeyExchange() == FALSE)
      {
        //If the TCLK exchange is not mandated, and the key cannot be found for this device,
        //is because we have erased the entry. Try with default key.
        si->keyNvSection = ZCD_NV_EX_LEGACY;
        si->keyNvId = ZCD_NV_TCLK_DEFAULT;
        status = ZSuccess;
      }
    }
  }

  return status;
}

/******************************************************************************
 * @fn          APSME_TCLinkKeyLoad
 *
 * @brief       Load Trust Center LINK key data.
 *
 * @param       dstAddr - [in] dstAddr
 * @param       si      - [in, out] SSP_Info_t
 *
 * @return      ZStatus_t
 */
ZStatus_t APSME_TCLinkKeyLoad( uint16_t dstAddr, SSP_Info_t* si )
{
  APSME_TCLinkKeyNVEntry_t TCLKDevEntry;
  AddrMgrEntry_t       addrEntry;
  ZStatus_t            status = ZSecNoKey;
  uint16_t               entryIndex;
  uint8_t                extAddrFound;
  uint8_t                found;


  // Look up the ami of the srcAddr if available
  addrEntry.user    = ADDRMGR_USER_DEFAULT;
  addrEntry.nwkAddr = dstAddr;

  extAddrFound = AddrMgrExtAddrValid( si->extAddr );

  if(extAddrFound)
  {
    entryIndex = APSME_SearchTCLinkKeyEntry(si->extAddr,&found,&TCLKDevEntry);

    if(entryIndex != 0xFFFF)
    {
      uint16_t i = entryIndex;

      if(found)
      {
        switch(TCLKDevEntry.keyAttributes)
        {
          case ZG_UNVERIFIED_KEY:
            #if ZG_BUILD_JOINING_TYPE
            if(ZG_DEVICE_JOINING_TYPE)
            {
              si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
              si->keyNvSection = ZCD_NV_EX_LEGACY;
              break;
            }
            #endif
          case ZG_DEFAULT_KEY:
          case ZG_NON_R21_NWK_JOINED:
          {
            si->keyNvId = ZCD_NV_TCLK_DEFAULT;
            si->keyNvSection = ZCD_NV_EX_LEGACY;
          }
          break;
          case ZG_PROVISIONAL_KEY:
          {
            si->keyNvId = (TCLKDevEntry.IcIndex & APSME_IC_INDEX_MASK);
            si->keyNvSection = ZCD_NV_EX_TCLK_IC_TABLE;
          }
          break;
          case ZG_VERIFIED_KEY:
            #if ZG_BUILD_JOINING_TYPE
            if(ZG_DEVICE_JOINING_TYPE)
            {
              si->keyNvId = ZCD_NV_TCLK_JOIN_DEV;
              si->keyNvSection = ZCD_NV_EX_LEGACY;
              break;
            }
            #endif
            #if ZG_BUILD_COORDINATOR_TYPE
            if(ZG_DEVICE_COORDINATOR_TYPE)
            {
              si->keyNvId = ZCD_NV_TCLK_SEED;
              si->seedShift = TCLKDevEntry.SeedShift;
              si->keyNvSection = ZCD_NV_EX_LEGACY;
            }
            #endif
            OsalPort_memcpy(si->dstExtAddr, TCLKDevEntry.extAddr,Z_EXTADDR_LEN);
          break;
          default:
            //This should not happen
          break;
        }
      }
      //Not found, then create an entry for it. Adding the device to the TCLKDev Entries for first time
      else
      {
        //Initialize the entry
        OsalPort_memcpy(TCLKDevEntry.extAddr, si->extAddr, Z_EXTADDR_LEN);
        TCLKDevEntry.keyAttributes = ZG_DEFAULT_KEY;
        TCLKDevEntry.keyType = ZG_GLOBAL_LINK_KEY;
        //Create the entry with a random shift of the seed. Validate the maximum shift of the seed which is 15
        TCLKDevEntry.SeedShift = OsalPort_rand() & 0x000F;
        TCLKDevEntry.txFrmCntr = 0;
        TCLKDevEntry.rxFrmCntr = 0;
        //save entry in nv
        osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, entryIndex,
                         sizeof(APSME_TCLinkKeyNVEntry_t),
                         &TCLKDevEntry);

        //Initialize framecounter
        memset(&TCLinkKeyRAMEntry[i],0,sizeof(APSME_TCLinkKeyRAMEntry_t));
        TCLinkKeyRAMEntry[i].entryUsed = TRUE;
        // set the keyNvId to use
        si->keyNvId = ZCD_NV_TCLK_DEFAULT;
        si->keyNvSection = ZCD_NV_EX_LEGACY;
      }

      // update link key related fields
      si->keyID   = SEC_KEYID_LINK;
      si->frmCntr = TCLinkKeyRAMEntry[i].txFrmCntr;

      // update outgoing frame counter
      TCLinkKeyRAMEntry[i].txFrmCntr++;

  #if defined ( NV_RESTORE )
      // write periodically to NV
      if ( !(TCLinkKeyRAMEntry[i].txFrmCntr % MAX_TCLK_FRAMECOUNTER_CHANGES) )
      {
        // set the flag to write key to NV
        TCLinkKeyRAMEntry[i].pendingFlag = TRUE;

        // Notify the ZDApp that the frame counter has changed.
        OsalPort_setEvent( ZDAppTaskID, ZDO_TCLK_FRAMECOUNTER_CHANGE );
      }
  #endif
    }
    else
    {
      //If no more TCLK entries, try global so we can try to add legacy devices
      si->keyNvId = ZCD_NV_TCLK_DEFAULT;
      si->keyNvSection = ZCD_NV_EX_LEGACY;
    }
    status = ZSuccess;
  }

  // If no TC link key found, remove the device from the address manager
  if ( (status != ZSuccess) && (AddrMgrEntryLookupNwk(&addrEntry) == TRUE) )
  {
    AddrMgrEntryRelease( &addrEntry );
  }

  return status;
}

/******************************************************************************
 * @fn          APSME_IsDefaultTCLK
 *
 * @brief       Return TRUE or FALSE based on the extended address.  If the
 *              input ext address is all FFs, it means the trust center link
 *              assoiciated with the address is the default trust center link key
 *
 * @param       extAddr - [in] extended address
 *
 * @return      uint8_t TRUE/FALSE
 */
uint8_t APSME_IsDefaultTCLK( uint8_t *extAddr )
{
  return OsalPort_isBufSet( extAddr, 0xFF, Z_EXTADDR_LEN );
}

/******************************************************************************
 * @fn          ZDSecMgrNwkKeyInit
 *
 * @brief       Initialize the NV items for
 *                  ZCD_NV_NWKKEY,
 *                  ZCD_NV_NWK_ACTIVE_KEY_INFO and
 *                  ZCD_NV_NWK_ALTERN_KEY_INFO
 *
 * @param       setDefault
 *
 * @return      none
 */
void ZDSecMgrNwkKeyInit(uint8_t setDefault)
{
  uint8_t status;
  nwkKeyDesc nwkKey;
  // Initialize NV items for NWK key, this structure contains the frame counter
  // and is only used when NV_RESTORE is enabled
  nwkActiveKeyItems keyItems;


  uint8_t i,nwkFrameCounterReset = FALSE;
  nwkSecMaterialDesc_t nwkSecMaterialDesc;

  //NwkSecMaterial entry is empty if set to 0s
  memset(&nwkSecMaterialDesc,0,sizeof(nwkSecMaterialDesc_t));
#ifdef NV_RESTORE
  // Has been set to reset the nwk security material?
  if ( zgReadStartupOptions() & ZCD_STARTOPT_CLEAR_NWK_FRAME_COUNTER )
  {
    nwkFrameCounterReset = TRUE;
  }
#else
  nwkFrameCounterReset = TRUE;
#endif

  //Initialize the nwk security material
  for( i = 0; i < gMAX_NWK_SEC_MATERIAL_TABLE_ENTRIES; i++)
  {
    if( (osal_nv_item_init_ex(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, i,
                              sizeof(nwkSecMaterialDesc_t),
                              &nwkSecMaterialDesc) == SUCCESS)
                              && (nwkFrameCounterReset) )
    {
      osal_nv_write_ex(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, i,
                    sizeof(nwkSecMaterialDesc_t),
                    &nwkSecMaterialDesc);
    }
  }
  //Set the last item to the generic nwk security material by setting extPanId to 0xFFs
  memset(nwkSecMaterialDesc.extendedPanID,0xFF,Z_EXTADDR_LEN);

  if( (osal_nv_item_init_ex(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, i - 1,
                           sizeof(nwkSecMaterialDesc_t),
                           &nwkSecMaterialDesc) == SUCCESS)
                           && (nwkFrameCounterReset) )
  {
    osal_nv_write_ex(ZCD_NV_EX_NWK_SEC_MATERIAL_TABLE, i - 1,
                     sizeof(nwkSecMaterialDesc_t),
                     &nwkSecMaterialDesc);
  }

  memset( &keyItems, 0, sizeof( nwkActiveKeyItems ) );

  status = osal_nv_item_init( ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), (void *)&keyItems );

#if defined ( NV_RESTORE )
  // reset the values of NV items if NV_RESTORE is not enabled
  if ((status == SUCCESS) && (setDefault == TRUE))
  {
    // clear NV data to default values
    osal_nv_write( ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), &keyItems );
  }
#else
  (void)setDefault;   // to eliminate compiler warning

  // reset the values of NV items if NV_RESTORE is not enabled
  if (status == SUCCESS)
  {
    osal_nv_write( ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), &keyItems );
  }
#endif // defined (NV_RESTORE)

  // Initialize NV items for NWK Active and Alternate keys. These items are used
  // all the time, independently of NV_RESTORE being set or not
  memset( &nwkKey, 0x00, sizeof(nwkKey) );

  status = osal_nv_item_init( ZCD_NV_NWK_ACTIVE_KEY_INFO, sizeof(nwkKey), &nwkKey);

#if defined ( NV_RESTORE )
  // reset the values of NV items if NV_RESTORE is not enabled
  if ((status == SUCCESS) && (setDefault == TRUE))
  {
    // clear NV data to default values
    osal_nv_write( ZCD_NV_NWK_ACTIVE_KEY_INFO, sizeof(nwkKey), &nwkKey );
  }
#else
  // reset the values of NV items if NV_RESTORE is not enabled
  if (status == SUCCESS)
  {
    osal_nv_write( ZCD_NV_NWK_ACTIVE_KEY_INFO, sizeof(nwkKey), &nwkKey );
  }
#endif // defined (NV_RESTORE)

  status = osal_nv_item_init( ZCD_NV_NWK_ALTERN_KEY_INFO, sizeof(nwkKey), &nwkKey );

#if defined ( NV_RESTORE )
  // reset the values of NV items if NV_RESTORE is not enabled
  if ((status == SUCCESS) && (setDefault == TRUE))
  {
    // clear NV data to default values
    osal_nv_write( ZCD_NV_NWK_ALTERN_KEY_INFO, sizeof(nwkKey), &nwkKey );
  }
#else
  // reset the values of NV items if NV_RESTORE is not enabled
  if (status == SUCCESS)
  {
    osal_nv_write( ZCD_NV_NWK_ALTERN_KEY_INFO, sizeof(nwkKey), &nwkKey );
  }
#endif // defined (NV_RESTORE)

}



/*********************************************************************
 * @fn          ZDSecMgrReadKeyFromNv
 *
 * @brief       Looks for a specific key in NV based on Index value
 *
 * @param   keyNvSection - NV Item ID used to find key
 *                       ZCD_NV_EX_LEGACY
 *                       ZCD_NV_EX_TCLK_IC_TABLE
 *                       ZCD_NV_EX_APS_KEY_DATA_TABLE
 *
 * @param   keyNvId - Index of key to look in NV
 *                    valid values are:
 *                    ZCD_NV_NWK_ACTIVE_KEY_INFO
 *                    ZCD_NV_NWK_ALTERN_KEY_INFO
 *                    <offset_in_table>
 *                    ZCD_NV_APS_LINK_KEY_DATA_START + <offset_in_table>
 *                    ZCD_NV_PRECFGKEY
 *
 * @param  *keyinfo - Data is read into this buffer.
 *
 * @return  SUCCESS if NV data was copied to the keyinfo parameter .
 *          Otherwise, NV_OPER_FAILED for failure.
 */
ZStatus_t ZDSecMgrReadKeyFromNv(uint16_t keyNvSection, uint16_t keyNvId, void *keyinfo)
{
  if (keyNvSection == ZCD_NV_EX_LEGACY)
  {
    if ((keyNvId == ZCD_NV_NWK_ACTIVE_KEY_INFO) ||
        (keyNvId == ZCD_NV_NWK_ALTERN_KEY_INFO))
    {
      // get NWK active or alternate key from NV
      return (osal_nv_read(keyNvId,
                           OsalPort_OFFSET_OF(nwkKeyDesc, key),
                           SEC_KEY_LEN,
                           keyinfo));
    }

    else if(keyNvId == ZCD_NV_TCLK_DEFAULT)
    {
      OsalPort_memcpy(keyinfo,defaultTCLinkKey,SEC_KEY_LEN);
      return ZSuccess;
    }

  #if (ZG_BUILD_JOINING_TYPE)
    else if(keyNvId == ZCD_NV_DISTRIBUTED_KEY)
    {
      if(ZG_DEVICE_JOINING_TYPE)
      {
        OsalPort_memcpy(keyinfo,distributedDefaultKey,SEC_KEY_LEN);
        return ZSuccess;
      }
    }
  #endif

    else if((keyNvId == ZCD_NV_TCLK_JOIN_DEV) || (keyNvId == ZCD_NV_PRECFGKEY))
    {
      return (osal_nv_read(keyNvId,
                           0,
                           SEC_KEY_LEN,
                           keyinfo));
    }
  }
  else if (keyNvSection == ZCD_NV_EX_TCLK_IC_TABLE)
  {
    //Read the key derived from the IC
    return (osal_nv_read_ex(ZCD_NV_EX_TCLK_IC_TABLE,
                            keyNvId, 0,
                            SEC_KEY_LEN,
                            keyinfo));
  }
  else if (keyNvSection == ZCD_NV_EX_APS_KEY_DATA_TABLE)
  {
    // Read entry keyNvId of the APS link key table from NV.
    return (osal_nv_read_ex(ZCD_NV_EX_APS_KEY_DATA_TABLE, keyNvId,
                            OsalPort_OFFSET_OF(APSME_ApsLinkKeyNVEntry_t, key),
                            SEC_KEY_LEN,
                            keyinfo));
  }

  return NV_OPER_FAILED;
}

/******************************************************************************
 * @fn          ZDSecMgrApsLinkKeyInit
 *
 * @brief       Initialize the NV table for Application link keys
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
void ZDSecMgrApsLinkKeyInit(uint8_t setDefault)
{
  APSME_ApsLinkKeyNVEntry_t pApsLinkKey;
  uint8_t i;
  uint8_t status;

  // Initialize all NV items for APS link key, if not exist already.
  memset( &pApsLinkKey, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t) );

  for( i = 0; i < gZDSECMGR_ENTRY_MAX; i++ )
  {
    status = osal_nv_item_init_ex(ZCD_NV_EX_APS_KEY_DATA_TABLE, i,
                                  sizeof(APSME_ApsLinkKeyNVEntry_t),
                                  &pApsLinkKey );

#if defined ( NV_RESTORE )
  // If the set default is requested, the APS Link key needs to be erased, regardless of the NV_RESTORE enabled
      if ((status == SUCCESS) && (setDefault == TRUE ))
      {
        osal_nv_write_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, i,
                       sizeof(APSME_ApsLinkKeyNVEntry_t),
                       &pApsLinkKey );

      }
#else
    // reset the values of NV items if NV_RESTORE is not enabled
    if (status == SUCCESS)
    {
        osal_nv_write_ex( ZCD_NV_EX_APS_KEY_DATA_TABLE, i,
                       sizeof(APSME_ApsLinkKeyNVEntry_t),
                       &pApsLinkKey );

    }
#endif // defined (NV_RESTORE)
  }
}


/******************************************************************************
 * @fn          ZDSecMgrInitNVKeyTables
 *
 * @brief       Initialize the NV table for All keys: NWK, Master, TCLK and APS
 *
 * @param       setDefault - TRUE to set default values
 *
 * @return      none
 */
void ZDSecMgrInitNVKeyTables(uint8_t setDefault)
{
  ZDSecMgrNwkKeyInit(setDefault);
  ZDSecMgrApsLinkKeyInit(setDefault);
  APSME_TCLinkKeyInit(setDefault);

#if ZG_BUILD_COORDINATOR_TYPE
  if(ZG_DEVICE_COORDINATOR_TYPE)
  {
    ZDSecMgrGenerateSeed(setDefault);

  }
#endif
}


/******************************************************************************
 * @fn          ZDSecMgrSaveApsLinkKey
 *
 * @brief       Save APS Link Key to NV. It will loop through all the keys
 *              to see which one to save.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrSaveApsLinkKey(void)
{
  APSME_ApsLinkKeyNVEntry_t *pKeyData = NULL;
  int i;

  pKeyData = (APSME_ApsLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_ApsLinkKeyNVEntry_t));

  if (pKeyData != NULL)
  {
    // checks all pending flags to know which one to save
    for (i = 0; i < gZDSECMGR_ENTRY_MAX; i++)
    {
      if (ApsLinkKeyRAMEntry[i].pendingFlag == TRUE)
      {
        // retrieve key from NV
        if (osal_nv_read_ex(ZCD_NV_EX_APS_KEY_DATA_TABLE, i, 0,
                            sizeof(APSME_ApsLinkKeyNVEntry_t),
                            pKeyData) == SUCCESS)
        {
          pKeyData->txFrmCntr = ApsLinkKeyRAMEntry[i].txFrmCntr;
          pKeyData->rxFrmCntr = ApsLinkKeyRAMEntry[i].rxFrmCntr;

          // Write the APS link key back to the NV
          osal_nv_write_ex(ZCD_NV_EX_APS_KEY_DATA_TABLE, i,
                        sizeof(APSME_ApsLinkKeyNVEntry_t),
                        pKeyData);

          // clear the pending write flag
          ApsLinkKeyRAMEntry[i].pendingFlag = FALSE;
        }
      }
    }

    // clear copy of key in RAM
    memset( pKeyData, 0x00, sizeof(APSME_ApsLinkKeyNVEntry_t) );

    OsalPort_free(pKeyData);
  }
}

/******************************************************************************
 * @fn          ZDSecMgrSaveTCLinkKey
 *
 * @brief       Save TC Link Key to NV. It will loop through all the keys
 *              to see which one to save.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrSaveTCLinkKey(void)
{
  APSME_TCLinkKeyNVEntry_t *pKeyData = NULL;
  uint16_t i;

  pKeyData = (APSME_TCLinkKeyNVEntry_t *)OsalPort_malloc(sizeof(APSME_TCLinkKeyNVEntry_t));

  if (pKeyData != NULL)
  {
    for( i = 0; i < gZDSECMGR_TC_DEVICE_MAX; i++ )
    {
      if (TCLinkKeyRAMEntry[i].pendingFlag == TRUE)
      {
        if (osal_nv_read_ex(ZCD_NV_EX_TCLK_TABLE, i, 0,
                            sizeof(APSME_TCLinkKeyNVEntry_t),
                            pKeyData) == SUCCESS)
        {
          pKeyData->txFrmCntr = TCLinkKeyRAMEntry[i].txFrmCntr;
          pKeyData->rxFrmCntr = TCLinkKeyRAMEntry[i].rxFrmCntr;

          // Write the TC link key back to the NV
          osal_nv_write_ex(ZCD_NV_EX_TCLK_TABLE, i,
                           sizeof(APSME_TCLinkKeyNVEntry_t),
                           pKeyData);

          // clear the pending write flag
          TCLinkKeyRAMEntry[i].pendingFlag = FALSE;
        }
      }
    }

    OsalPort_free(pKeyData);
  }
}

/******************************************************************************
 * @fn          ZDSecMgrUpdateTCAddress
 *
 * @brief       Update Trust Center address and save to NV.
 *
 * @param       extAddr - [in] extended address or NULL if no TC protected
 *
 * @return      none
 */
void ZDSecMgrUpdateTCAddress( uint8_t *extAddr )
{
  uint8_t noTCAddress[Z_EXTADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  APSME_SetRequest( apsTrustCenterAddress, 0, ( extAddr != NULL) ? extAddr : noTCAddress );
  osal_cpyExtAddr( zgApsTrustCenterAddr, ( extAddr != NULL) ? extAddr : noTCAddress );

#if defined ( NV_RESTORE )
  osal_nv_write( ZCD_NV_TRUSTCENTER_ADDR, Z_EXTADDR_LEN, zgApsTrustCenterAddr );
#endif
}

#if defined ( ZBA_FALLBACK_NWKKEY )
/******************************************************************************
 * @fn          ZDSecMgrFallbackNwkKey
 *
 * @brief       Use the ZBA fallback network key.
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrFallbackNwkKey( void )
{
  if ( !_NIB.nwkKeyLoaded )
  {
    uint8_t fallbackKey[SEC_KEY_LEN];

    ZDSecMgrReadKeyFromNv( ZCD_NV_EX_LEGACY, ZCD_NV_PRECFGKEY, fallbackKey );
    SSP_UpdateNwkKey( fallbackKey, 0);
    SSP_SwitchNwkKey( 0 );

    // clear local copy of key
    memset( fallbackKey, 0x00, SEC_KEY_LEN );

    // handle next step in authentication process
    ZDSecMgrAuthNwkKey();
  }
}
#endif // defined ( ZBA_FALLBACK_NWKKEY )

#if defined ( NV_RESTORE )
/******************************************************************************
 * @fn          ZDSecMgrClearNVKeyValues
 *
 * @brief       If NV_RESTORE is enabled and the status of the network needs
 *              default values this fuction clears ZCD_NV_NWKKEY,
 *              ZCD_NV_NWK_ACTIVE_KEY_INFO and ZCD_NV_NWK_ALTERN_KEY_INFO link
 *
 * @param       none
 *
 * @return      none
 */
void ZDSecMgrClearNVKeyValues(void)
{
  nwkActiveKeyItems keyItems;
  nwkKeyDesc nwkKey;

  memset(&keyItems, 0x00, sizeof(nwkActiveKeyItems));

  osal_nv_write(ZCD_NV_NWKKEY, sizeof(nwkActiveKeyItems), &keyItems);

  // Initialize NV items for NWK Active and Alternate keys.
  memset( &nwkKey, 0x00, sizeof(nwkKeyDesc) );

  osal_nv_write(ZCD_NV_NWK_ACTIVE_KEY_INFO, sizeof(nwkKeyDesc), &nwkKey);

  osal_nv_write(ZCD_NV_NWK_ALTERN_KEY_INFO, sizeof(nwkKeyDesc), &nwkKey);

  _NIB.nwkKeyLoaded = FALSE;

  if ( ZG_DEVICE_JOINING_TYPE )
  {
    // Set initial TCLK for joining device to default TCLK
    if( SUCCESS == osal_nv_item_init(ZCD_NV_TCLK_JOIN_DEV, SEC_KEY_LEN, (void *)defaultTCLinkKey) )
    {
      //Or replace it if already existed
      osal_nv_write(ZCD_NV_TCLK_JOIN_DEV, SEC_KEY_LEN, (void *)defaultTCLinkKey);
    }
  }
}
#endif // defined ( NV_RESTORE )

/******************************************************************************
******************************************************************************/
