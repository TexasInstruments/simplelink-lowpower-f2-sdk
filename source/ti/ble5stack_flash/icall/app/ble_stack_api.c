/******************************************************************************

@file  ble_stack_api.c

@brief This file contains the BLE stack wrapper above ICall

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2013-2025, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************


*****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <stdarg.h>
#include "ble_stack_api.h"
#include <icall_ble_api.h>
#ifndef NO_TI_BLE_CONFIG
#include "ti_ble_config.h"
#endif
#include <gapbondmgr.h>

/*********************************************************************
 * FUNCTIONS
 */
bStatus_t bleStk_GapAdv_loadLocalByHandle(uint8_t advHandle,GapAdv_dataTypes_t advtype,
                                    uint16_t len, uint8_t *pData);

#ifdef ERPC_SERVER
#include "ble_app_services.h"
static pfnBleStkAdvCB_t  remote_bleApp_GapAdvCb = NULL;
static void local_bleApp_GapAdvCb(uint32 event, GapAdv_data_t *pBuf, uint32_t *arg);
static bleStk_pfnGapScanCB_t remote_bleApp_ScanCb = NULL;
#endif

#ifdef ICALL_NO_APP_EVENTS
/*********************************************************************
 * @fn      bleStack_register
 *
 * @brief   register callback in the ICALL for application events
 *
 * @param   selfEntity - return self entity id from the ICALL
 * @param   appCallback- The callback
 *
 * @return  SUCCESS ot FAILURE.
 */
bleStack_errno_t bleStack_register(uint8_t *selfEntity, appCallback_t appCallback)
{
  ICall_EntityID   localSelfEntity;
  ICall_Errno status;
  // ******************************************************************
  // NO STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
  // ******************************************************************
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  status = ICall_registerAppCback(&localSelfEntity, appCallback);

  // Application should use the task entity ID
  *selfEntity = localSelfEntity;

  return status;
}
#endif // ICALL_NO_APP_EVENTS

/*********************************************************************
 * @fn      bleStack_createTasks
 *
 * @brief   Create ICALL tasks
 *
 * @param   None
 *
 * @return  None
 */
void bleStack_createTasks()
{
    /* Initialize ICall module */
    ICall_init();

    /* Start tasks of external images - Priority 5 */
    ICall_createRemoteTasks();
}

/*********************************************************************
 * @fn      bleStack_initGap
 *
 * @brief   Init the GAP
 *
 * @param   role - the role of the application
 * @param   appSelfEntity - self entity id for the ICALL
 * @param   paramUpdateDecision- the param update configuration
 *
 * @return  SUCCESS ot FAILURE.
 */
bStatus_t bleStack_initGap(uint8_t role, ICall_EntityID appSelfEntity, bleStk_pfnGapScanCB_t scanCallback, uint16_t paramUpdateDecision)
{
  bStatus_t status = SUCCESS;
#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )

  // Pass all parameter update requests to the app for it to decide
  GAP_SetParamValue(GAP_PARAM_LINK_UPDATE_DECISION, paramUpdateDecision);
#endif //#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | PERIPHERAL_CFG ) )

#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )
  if(scanCallback != NULL)
  {
    // Register scan callback to process scanner events
    status = GapScan_registerCb(scanCallback, NULL);
    if (status != SUCCESS)
    {
      return status;
    }

    // Set Scanner Event Mask
    GapScan_setEventMask(GAP_EVT_SCAN_EVT_MASK);
  }
#endif //#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )

  // Register with GAP for HCI/Host messages. This is needed to receive HCI
  // events. For more information, see the HCI section in the User's Guide:
  // http://software-dl.ti.com/lprf/ble5stack-latest/
  GAP_RegisterForMsgs(appSelfEntity);

  return status;
}

/*********************************************************************
 * @fn      bleStack_initGapBondParams
 *
 * @brief   Set all GAP bond manager parameters
 *
 * @param   pGapBondParams - the configuration structure
 *
 * @return  SUCCESS ot FAILURE.
 */
void bleStack_initGapBondParams(gapBondParams_t *pGapBondParams)
{
    // Set Pairing Mode
    GAPBondMgr_SetParameter(GAPBOND_PAIRING_MODE, sizeof(uint8_t), &pGapBondParams->pairMode);
    // Set MITM Protection
    GAPBondMgr_SetParameter(GAPBOND_MITM_PROTECTION, sizeof(uint8_t), &pGapBondParams->mitm);
    // Set IO Capabilities
    GAPBondMgr_SetParameter(GAPBOND_IO_CAPABILITIES, sizeof(uint8_t), &pGapBondParams->ioCap);
    // Set Bonding
    GAPBondMgr_SetParameter(GAPBOND_BONDING_ENABLED, sizeof(uint8_t), &pGapBondParams->bonding);
    // Set Secure Connection Usage during Pairing
    GAPBondMgr_SetParameter(GAPBOND_SECURE_CONNECTION, sizeof(uint8_t), &pGapBondParams->secureConnection);
    // Set Authenticated Pairing Only mode
    GAPBondMgr_SetParameter(GAPBOND_AUTHEN_PAIRING_ONLY, sizeof(uint8_t), &pGapBondParams->authenPairingOnly);
    // Set Auto Acceptlist Sync
    GAPBondMgr_SetParameter(GAPBOND_AUTO_SYNC_AL, sizeof(uint8_t), &pGapBondParams->autoSyncAL);
    // Set ECC Key Regeneration Policy
    GAPBondMgr_SetParameter(GAPBOND_ECCKEY_REGEN_POLICY, sizeof(uint8_t), &pGapBondParams->eccReGenPolicy);
    // Set Key Size used in pairing
    GAPBondMgr_SetParameter(GAPBOND_KEYSIZE, sizeof(uint8_t), &pGapBondParams->KeySize);
    // Set LRU Bond Replacement Scheme
    GAPBondMgr_SetParameter(GAPBOND_LRU_BOND_REPLACEMENT, sizeof(uint8_t), &pGapBondParams->removeLRUBond);
    // Set Key Distribution list for pairing
    GAPBondMgr_SetParameter(GAPBOND_KEY_DIST_LIST, sizeof(uint8_t), &pGapBondParams->KeyDistList);
    // Set Secure Connection Debug Keys
    GAPBondMgr_SetParameter(GAPBOND_SC_HOST_DEBUG, sizeof(uint8_t), &pGapBondParams->eccDebugKeys);
    // Set the Erase bond While in Active Connection Flag
    GAPBondMgr_SetParameter(GAPBOND_ERASE_BOND_IN_CONN, sizeof(uint8_t), &pGapBondParams->eraseBondWhileInConn);
    // Set Same IRK Action
    GAPBondMgr_SetParameter(GAPBOND_SAME_IRK_OPTION, sizeof(uint8_t), &pGapBondParams->sameIrkAction);
}

/*********************************************************************
 * @fn      bleStack_initGapBond
 *
 * @brief   Init GAP bond manager
 *
 * @param   pGapBondParams - the configuration structure
 * @param   bleApp_bondMgrCBs - the pairing and passcode callbacks
 *
 * @return  SUCCESS ot FAILURE.
 */
bStatus_t bleStack_initGapBond(gapBondParams_t *pGapBondParams, void *bleApp_bondMgrCBs)
{
#if defined ( GAP_BOND_MGR )
  bleStack_initGapBondParams(pGapBondParams);

  if (bleApp_bondMgrCBs != NULL)
  {
    // Start Bond Manager and register callback
    VOID GAPBondMgr_Register((gapBondCBs_t *)bleApp_bondMgrCBs);
  }
#endif // GAP_BOND_MGR
  return SUCCESS;
}

#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( PERIPHERAL_CFG | CENTRAL_CFG ) )
/*********************************************************************
 * @fn      bleStack_initGatt
 *
 * @brief   Init the GATT
 *
 * @param   role - the role of the application
 * @param   appSelfEntity - self entity id for the ICALL
 * @param   pAttDeviceName- the device name
 *
 * @return  SUCCESS ot FAILURE.
 */
bStatus_t bleStack_initGatt(uint8_t role, ICall_EntityID appSelfEntity, uint8_t *pAttDeviceName)
{
    bStatus_t status = SUCCESS;

  // Set the Device Name characteristic in the GAP GATT Service
  // For more information, see the section in the User's Guide:
  // http://software-dl.ti.com/lprf/ble5stack-latest/
    status = GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, (void *)pAttDeviceName);
    if (status != SUCCESS)
    {
      return status;
    }

  // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);           // GAP GATT Service
  GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT Service

  // Register for GATT local events and ATT Responses pending for transmission
  GATT_RegisterForMsgs(appSelfEntity);

  if (role & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_BROADCASTER))
  // Set default values for Data Length Extension
  // Extended Data Length Feature is already enabled by default
  {
    // Set initial values to maximum
    #define BLEAPP_SUGGESTED_PDU_SIZE 251
    #define BLEAPP_SUGGESTED_TX_TIME 2120 //default is 328us(TX)

    // This API is documented in hci.h
    // See the LE Data Length Extension section in the BLE5-Stack User's Guide for information on using this command:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    HCI_LE_WriteSuggestedDefaultDataLenCmd(BLEAPP_SUGGESTED_PDU_SIZE, BLEAPP_SUGGESTED_TX_TIME);
  }
  if (role & (GAP_PROFILE_CENTRAL | GAP_PROFILE_OBSERVER))
  //Set default values for Data Length Extension
  //Extended Data Length Feature is already enabled by default
  //in build_config.opt in stack project.
  {
    //Change initial values of RX/TX PDU and Time, RX is set to max. by default(251 octets, 2120us)
    #define APP_SUGGESTED_RX_PDU_SIZE 251     //default is 251 octets(RX)
    #define APP_SUGGESTED_RX_TIME     17000   //default is 17000us(RX)
    #define APP_SUGGESTED_TX_PDU_SIZE 27      //default is 27 octets(TX)
    #define APP_SUGGESTED_TX_TIME     328     //default is 328us(TX)

    //This API is documented in hci.h
    //See the LE Data Length Extension section in the BLE5-Stack User's Guide for information on using this command:
    //http://software-dl.ti.com/lprf/ble5stack-latest/
    HCI_EXT_SetMaxDataLenCmd(APP_SUGGESTED_TX_PDU_SIZE, APP_SUGGESTED_TX_TIME, APP_SUGGESTED_RX_PDU_SIZE, APP_SUGGESTED_RX_TIME);
  }

  if (role & (GAP_PROFILE_PERIPHERAL | GAP_PROFILE_CENTRAL))
  {
      // Initialize GATT Client
      GATT_InitClient();
  }

  if (role & (GAP_PROFILE_CENTRAL | GAP_PROFILE_OBSERVER))
  {
      // Register to receive incoming ATT Indications/Notifications
      GATT_RegisterForInd(appSelfEntity);
  }

  return status;
}
#endif // ( CENTRAL_CFG | PERIPHERAL_CFG )

/*********************************************************************
 * @fn      bleStk_initAdvSet
 *
 * @brief   Initialize and starts advertise set (legacy or extended)
 *
 * @param advCallback     - callback for advertising progress states
 * @param advHandle       - return the created advertising handle
 * @param eventMask       - A bitfield to enable / disable events returned to the
 *                          per-advertising set callback function (@ref pfnGapCB_t ).
 *                          See @ref GapAdv_eventMaskFlags_t
 * @param advParams       - pointer to structure of adversing parameters
 * @param advData         - pointer to array containing the advertise data
 * @param advDataLen      - length (in bytes) of advData
 * @param scanRespData    - pointer to array containing the scan response data
 * @param scanRespDataLen - length (in bytes) of scanRespDataLen
 *
 * @return SUCCESS upon successful initialization,
 *         else, relevant error code upon failure
 */
bStatus_t bleStk_initAdvSet(pfnBleStkAdvCB_t advCallback, uint8_t *advHandle,
                                   GapAdv_eventMaskFlags_t eventMask,
                                   GapAdv_params_t *advParams,
                                   uint16_t advDataLen ,uint8_t *advData,
                                   uint16_t scanRespDataLen, uint8_t *scanRespData)
{
  bStatus_t status;

#ifdef ERPC_SERVER
  // keep the remote eRPC app callback
  remote_bleApp_GapAdvCb = advCallback;

  // Create Advertisement set and assign handle
  status = GapAdv_create((pfnGapCB_t)local_bleApp_GapAdvCb, advParams, advHandle);
#else
  status = GapAdv_create((pfnGapCB_t)advCallback, advParams, advHandle);
#endif
  if (status != SUCCESS)
  {
    return status;
  }

  if (advData != NULL)
  {
    // Load advertising data for set that is statically allocated by the app
#ifdef ERPC_SERVER
    status = bleStk_GapAdv_loadLocalByHandle(*advHandle, GAP_ADV_DATA_TYPE_ADV, advDataLen, advData);
#else
    status = GapAdv_loadByHandle(*advHandle, GAP_ADV_DATA_TYPE_ADV, advDataLen, advData);
#endif
    if (status != SUCCESS)
    {
      return status;
    }
  }

  // Load scan response data for set that is statically allocated by the app
  if (scanRespData != NULL)
  {
#ifdef ERPC_SERVER
    status = bleStk_GapAdv_loadLocalByHandle(*advHandle, GAP_ADV_DATA_TYPE_SCAN_RSP, scanRespDataLen, scanRespData);
#else
    status = GapAdv_loadByHandle(*advHandle, GAP_ADV_DATA_TYPE_SCAN_RSP, scanRespDataLen, scanRespData);
#endif
    if (status != SUCCESS)
    {
      return status;
    }
  }

  // Set event mask for set
  status = GapAdv_setEventMask(*advHandle, eventMask);
  if (status != SUCCESS)
  {
    return status;
  }

  return status;
}

#ifdef ERPC_SERVER
//!< Advertising set payload pointer, first set [0..Max] is for ADV, second set [Max+1..2xMax] is for SCAN_RSP
uint8_t  *localAdvPayload[AE_DEFAULT_NUM_ADV_SETS*2];
/*********************************************************************
 * @fn      bleStk_GapAdv_loadLocalByHandle
 *
 * @brief   copy the advertising data into local buffers before loading the ADV data
 *
 * @param advHandle - the ADV set handle
 * @param advtype   - ADV type
 * @param len       - ADV data len
 * @param pData     - ADV data buffer
 *
 * @return  SUCCESS of FAILURE.
 */
bStatus_t bleStk_GapAdv_loadLocalByHandle(uint8_t advHandle,GapAdv_dataTypes_t advtype,
                                    uint16_t len, uint8_t *pData)
{
  bStatus_t status;
  uint8_t index;

  // set the index of the saved payload, first set [0..Max] is for ADV, second set [Max+1..2xMax] is for SCAN_RSP
  // advHandle will be offset in the selected set
  if (advtype == GAP_ADV_DATA_TYPE_SCAN_RSP)
  {
    index = AE_DEFAULT_NUM_ADV_SETS;    // scan data is the last (20) adv set in the buffer
  }
  else if (advtype == GAP_ADV_DATA_TYPE_ADV)
  {
    index = 0;
  }
  else
  {
    return(INVALIDPARAMETER);
  }

  if (advHandle >= AE_DEFAULT_NUM_ADV_SETS )
  {
    // Unknow Handler
    return(bleGAPNotFound);
  }

  // offset = advHandle+index is now the offset to advPayload[offset] to save the ADV payload

  // Free the handle if already exists
  if (localAdvPayload[advHandle+index])
  {
    status = GapAdv_prepareLoadByHandle(advHandle, GAP_ADV_FREE_OPTION_ADV_DATA);
  }

  if (len)
  {
    if(!(localAdvPayload[advHandle+index] = (uint8_t*) bleStack_malloc(len)))
    {
      return(bleNoResources);
    }

    // Copy the ADV data into local DB
    memcpy(localAdvPayload[advHandle+index], pData, len);
  }
  else
  {
    // emptying Adv Data...
    localAdvPayload[advHandle+index] = NULL;
  }

  // Configure the local copied ADV data into the BLE stack
  status |= GapAdv_loadByHandle(advHandle, advtype,
                               len, localAdvPayload[advHandle+index]);
  return(status);
}

/*********************************************************************
* @fn      appContext_GapAdvCb
*
* @brief   ADV callback - call the saved eRPC registered remote_bleApp_GapAdvCb
*
* @param   None.
*
* @return  None.
*/
static void appContext_GapAdvCb(uint32_t event, uint8_t *pBuf, uint32_t *arg)
{
    remote_bleApp_GapAdvCb(event, (GapAdv_data_t *)pBuf, arg);
}

/*********************************************************************
* @fn      local_bleApp_GapAdvCb
*
* @brief   Local ADV callback to be registered in the BLE stack
*          !!! Context issue - In some events, It will be called by the BLE stack in the context of SWI context
*          Switch - It must be handled in the server task context before forwarding to the ERPC client
*          It will call the saved eRPC registered appContext_GapAdvCb
*
* @param   None.
*
* @return  None.
*/
static void local_bleApp_GapAdvCb(uint32 event, GapAdv_data_t *pBuf, uint32_t *arg)
{
    bleSrv_callOnAppContext((callbackFxn_t)appContext_GapAdvCb, event, (uint8_t *)pBuf, arg);
}

#endif //ERPC_SERVER

/*********************************************************************
 * @fn      bleStk_getDevAddr
 *
 * @brief   Get the address of this device
 *
 * @param   wantIA - TRUE  for Identity Address
 *                   FALSE for Resolvable Private Address (if GAP device
 *                   address mode is @ref ADDRMODE_RP_WITH_PUBLIC_ID or
 *                   @ref ADDRMODE_RP_WITH_RANDOM_ID)
 * @param   pAddr - the returned address
 *
 * @return  None.
 */
void bleStk_getDevAddr(uint8_t wantIA, uint8_t *pAddr)
{
  uint8_t* pDevAddr = NULL;

  pDevAddr = GAP_GetDevAddress(wantIA);
  memcpy(pAddr, pDevAddr, B_ADDR_LEN);
}

#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )
#if defined(ERPC_SERVER)
/*********************************************************************
* @fn      appContext_ScanCb
*
* @brief   SCAN callback - call the saved eRPC registered remote_bleApp_ScanCb
*
* @param   None.
*
* @return  None.
*/
void appContext_ScanCb(uint32_t event, uint8_t *pBuf, uint32_t *arg)
{
  // No need to copy the message and internal pData, ERPC will copy the internal pData
  remote_bleApp_ScanCb(event, (GapScan_data_t *)pBuf, arg);
}

/*********************************************************************
* @fn      local_bleApp_ScanCb
*
* @brief   Local SCAN callback to be registered in the BLE stack
*          !!! Context issue - It will be called by the BLE stack in the context of SWI context
*          Switch - It must be handled in the server task context before forwarding to the ERPC client
*          It will call the saved eRPC registered remote_bleApp_ScanCb
*
* @param   None.
*
* @return  None.
*/
void local_bleApp_ScanCb(uint32_t event, GapScan_data_t *pBuf, uint32_t *arg)
{
    bleSrv_callOnAppContext((callbackFxn_t)appContext_ScanCb, event, (uint8_t *)pBuf, arg);
}

/*********************************************************************
* @fn      bleStk_GapScan_registerCb
*
* @brief   Wrapper for stack API - in order not to use void * that is in pfnGapCB_t and to use local callback
*          Original API: status_t GapScan_registerCb(pfnGapCB_t cb, uintptr_t arg)
*
* @param   None.
*
* @return  None.
*/
status_t bleStk_GapScan_registerCb(bleStk_pfnGapScanCB_t cb, uint32_t * arg)
{
  // save the original eRPC app callback
  remote_bleApp_ScanCb = cb;

  // regiter with local callback
  return(GapScan_registerCb(local_bleApp_ScanCb, (uintptr_t) arg));
}
#endif //ERPC_SERVER
#endif //#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )

#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG) )
/*********************************************************************
 * @fn      bleStk_scanInit
 *
 * @brief   Setup initial device scan settings.
 *
 * @return  None.
 */
bStatus_t bleStk_scanInit(bleStk_pfnGapScanCB_t scanCallback,
                              GapScan_EventMask_t eventMask,
                              uint8_t primPhys, uint8_t scanType, uint16_t scanInterval, uint16_t scanWindow,
                              uint16_t advReportFields, uint8_t defaultScanPhy, uint8_t scanDupFilter,
                              uint16_t scanFilterPduType,
                              uint16_t scanMinConnInterval, uint16_t scanMaxConnInterval)
{
  status_t status;
#if defined(ERPC_SERVER)
  uint32_t arg = 0;
  // save the original eRPC app callback
  remote_bleApp_ScanCb = scanCallback;
  // regiter with local callback
  status = GapScan_registerCb(local_bleApp_ScanCb, (uintptr_t) arg);
#else
  // Register callback to process Scanner events
  status = GapScan_registerCb(scanCallback, NULL);
#endif
  if (status != SUCCESS)
  {
    return status;
  }

  // Set Scanner Event Mask
  GapScan_setEventMask(eventMask);

  // Set Scan PHY parameters
  status = GapScan_setPhyParams(primPhys, scanType, scanInterval, scanWindow);
  if (status != SUCCESS)
  {
    return status;
  }

  // set scan params
  {
    uint8_t temp8;
    uint16_t temp16;

    // Set Advertising report fields to keep
    temp16 = advReportFields;
    GapScan_setParam(SCAN_PARAM_RPT_FIELDS, &temp16);
    // Set Scanning Primary PHY
    temp8 = defaultScanPhy;
    GapScan_setParam(SCAN_PARAM_PRIM_PHYS, &temp8);
    // Set LL Duplicate Filter
    temp8 = scanDupFilter;
    GapScan_setParam(SCAN_PARAM_FLT_DUP, &temp8);

    // Set PDU type filter -
    // Only 'Connectable' and 'Complete' packets are desired.
    // It doesn't matter if received packets are
    // whether Scannable or Non-Scannable, whether Directed or Undirected,
    // whether Scan_Rsp's or Advertisements, and whether Legacy or Extended.
    temp16 = scanFilterPduType;
    GapScan_setParam(SCAN_PARAM_FLT_PDU_TYPE, &temp16);
  }

  // Set initiating PHY parameters
  status = GapInit_setPhyParam(DEFAULT_INIT_PHY, INIT_PHYPARAM_CONN_INT_MIN, scanMinConnInterval);

  status = GapInit_setPhyParam(DEFAULT_INIT_PHY, INIT_PHYPARAM_CONN_INT_MAX, scanMaxConnInterval);

  return status;
}

#endif //#if defined( HOST_CONFIG ) && ( HOST_CONFIG & ( CENTRAL_CFG) )
