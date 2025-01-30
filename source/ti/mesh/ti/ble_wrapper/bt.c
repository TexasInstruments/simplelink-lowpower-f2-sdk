/******************************************************************************

@file  bt.c

@brief This file contains the BLE porting layer between zephyr mesh and TI BLE

Group: WCS, BTS
$Target Device: DEVICES $

******************************************************************************
$License: BSD3 2013 $
******************************************************************************
$Release Name: PACKAGE NAME $
$Release Date: PACKAGE RELEASE DATE $
*****************************************************************************/

/******************************************************************************
* INCLUDES
*/
// Includes that need to be on top
#include <autoconf.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <settings/settings.h>

#if !(defined __TI_COMPILER_VERSION__) && !(defined __GNUC__)
#include <intrinsics.h>
#endif

// TI RTOS
#include <ti/drivers/utils/List.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <ti/display/Display.h>

// General
#include <string.h>
#include "util.h"
#include <bcomdef.h>
#include <board_key.h>
#include <icall.h>
#include <icall_ble_api.h>
#include "hal_mcu.h"

// Sysconfig
#include "ti_ble_config.h"
#include <ti_drivers_config.h>

// TI BLE Stack
#ifndef ICALL_NO_APP_EVENTS
#error "ICall is not configured properly, ICALL_NO_APP_EVENTS is not defined"
#endif /* ICALL_NO_APP_EVENTS */
#include "ble_stack_api.h"
#include <devinfoservice.h>
#include <simple_gatt_profile.h>
#include <gapgattserver.h>
#include <gap.h>
#include <hci.h>

//Zephyr
#include <subsys/bluetooth/host/ecc.h>
#include <logging/log.h>

// Zephyr BLE
#include <bluetooth/conn.h>
#include <bluetooth/host/conn_internal.h>
#include <bluetooth/crypto.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

// Porting layer main include
#include <bt_le_porting_layer.h>

/******************************************************************************
* CONSTANTS
*/
#ifdef CONFIG_BT_MESH_PROXY
#define BT_MESH_CONN_FREE 0xFFFF
#define CONN_LIST_SIZE CONFIG_BT_MAX_CONN
#endif /* CONFIG_BT_MESH_PROXY */


/******************************************************************************
 * LOCAL FUNCTIONS DECLARATIONS
 */
#ifdef CONFIG_BT_MESH_PROXY
static void            mesh_portingLayer_initConnHandle(void);
static struct bt_conn *mesh_portingLayer_getConnByHandle(
                                                    uint16_t connectionHandle);
static struct bt_conn *mesh_portingLayer_getFreeConn(void);
static int             mesh_portingLayer_freeConnByHandle(
                                                    uint16_t connectionHandle);
#endif /* CONFIG_BT_MESH_PROXY */
static bStatus_t       mesh_portingLayer_registerZephyrScanCB(
                                                    bt_le_scan_cb_t cb);

/******************************************************************************
* CALLBACKS
*/
#ifdef CONFIG_BT_MESH_PROXY
bStatus_t mesh_portingLayer_gattReadAttCB(uint16 connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8 *pValue, uint16 *pLen,
                                          uint16 offset, uint16 maxLen,
                                          uint8 method );
#endif /* CONFIG_BT_MESH_PROXY */

/******************************************************************************
* GLOBAL VARIABLES
*/

/******************************************************************************
* LOCAL VARIABLES
*/
/* --------------- */
/* --- General --- */
/* --------------- */

// Zephyr BT Structure
struct bt_dev bt_dev = {.hci_version = BT_HCI_VERSION_5_1};

#ifdef CONFIG_BT_MESH_PROXY
// BLE device name
static const char *pBleName;
#endif /* CONFIG_BT_MESH_PROXY */

// Zephyr work Q init flag
static bool bWorkQInitialized = FALSE;


/* --------------------------- */
/* ---- Advertise related ---- */
/* --------------------------- */

// Flag that indicates if this is the first time an adv is done,
// it is used in order to call ICall_registerApp only once
static bool meshAdvFirst = true;

// Advertising handle
static uint8 meshAdvHandle;

#ifdef ZEPHYR_ADV_EXT
// Callback given by mesh profile to BLE stack for extended
// advertise set adv and connection
const struct bt_le_ext_adv_cb *pZephyrAdvSetCBs = NULL;
#endif

/* --------------------- */
/* --- iCall related --- */
/* --------------------- */
// Advertising thread entity ID globally used to check for source and/or
// destination of messages
ICall_EntityID advThreadSelfEntity;     // advertisement thread
ICall_EntityID workQThreadSelfEntity;   // WorkQ thread

/* ------------------------ */
/* --- Scanning related --- */
/* ------------------------ */
// Callback given by mesh profile to BLE stack for scan events
static bt_le_scan_cb_t *pZephyrScanCb = NULL;

// Flag to indicate if currently scanning
static bool bMeshIsScanning = FALSE;

// Reenable scan parameters
struct bt_le_scan_param scanReEnableParam;

// Reenable scan flag
static bool bReEnableScan = FALSE;

#ifdef CONFIG_BT_MESH_PROXY
/* --------------------- */
/* --- Proxy related --- */
/* --------------------- */
// Zephyr side provisioning/proxy write cb
static pFxnZephyrWriteGattCb_t pZephyrProxyProvWriteCb;

// Zephyr provisioning CCC cbs
static pFxnZephyrGattCccWriteCb_t   zephyrProvCCCWriteCb;
static pFxnZephyrGattCccChangedCb_t zephyrProvCCCChangedCb;

// Zephyr proxy CCC cbs
static pFxnZephyrGattCccWriteCb_t   zephyrProxyCCCWriteCb;
static pFxnZephyrGattCccChangedCb_t zephyrProxyCCCChangedCb;

// Application side provisioning/proxy gatt cbs
static CONST gattServiceCBs_t mesh_portingLayer_provProxyServiceCBs = {
  NULL,                                             // pfnReadAttrCB
  (pfnGATTWriteAttrCB_t)SimpleMeshNode_gattWriteCB, // pfnWriteAttrCB
  NULL                                              // pfnAuthorizeAttrCB
};

// Zephyr connection formation related cbs
static struct bt_conn_cb *pZephyrConnCBs = NULL;

// Existing link mtuSize
static uint16 mtuSize = 0;

// Zephyr connection object
static struct bt_conn connectionList[CONN_LIST_SIZE];

// Size of the last written prov_out
static uint8 lastProvDataOutValSize;

// Size of the last written proxy_out
static uint8 lastProxyDataOutValSize;
#endif /* CONFIG_BT_MESH_PROXY */

/******************************************************************************
 * EMPTY FUNCTIONS
 */
#ifdef CONFIG_BT_MESH_PROXY
ssize_t bt_gatt_attr_read_ccc(struct bt_conn *conn,
                              const struct bt_gatt_attr *attr, void *buf,
                              uint16_t len, uint16_t offset){return 0;}
ssize_t bt_gatt_attr_read_chrc(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr, void *buf,
                               uint16_t len, uint16_t offset){return 0;}
ssize_t bt_gatt_attr_read_service(struct bt_conn *conn,
                                 const struct bt_gatt_attr *attr,
                                 void *buf, uint16_t len, uint16_t offset){return 0;}
ssize_t bt_gatt_attr_write_ccc(struct bt_conn *conn,
                               const struct bt_gatt_attr *attr,
                               const void *buf, uint16_t len, uint16_t offset,
                               uint8_t flags){return 0;}
#endif /* CONFIG_BT_MESH_PROXY */

/******************************************************************************
* CALLBACKS
*/
/******************************************************************************
 * @fn      mesh_portingLayer_advCB
 *
 * @brief   GapAdv module callback
 *
 * @param   event - Advertisement event
 * @param   pBuf  - Data potentially accompanying event
 * @param   arg  -  Custom application argument that can be returned
 *                  through this callback
 */
void mesh_portingLayer_advCB(uint32_t event, void *pBuf)
{
#ifdef ZEPHYR_ADV_EXT
  // Call the Zephyr adv cb
  if ( (event & GAP_EVT_ADV_SET_TERMINATED) && pZephyrAdvSetCBs != NULL && pZephyrAdvSetCBs->sent != NULL)
  {
      pZephyrAdvSetCBs->sent(NULL, NULL);
  }
#endif
}

/******************************************************************************
 * @fn      mesh_portingLayer_scanCB
 *
 * @brief   Callback called by GapScan module
 *
 * @param   evt - event
 * @param   msg - message coming with the event
 * @param   arg - user argument
 *
 * @return  none
 */
void mesh_portingLayer_scanCB(uint32_t evt, void* pMsg, uintptr_t arg)
{
  int8_t                rssi;
  uint8_t               adv_type;
  bt_addr_le_t          addr;
  struct net_buf_simple buf;

  // Get Adv Report
  GapScan_Evt_AdvRpt_t* pAdvRpt = (GapScan_Evt_AdvRpt_t*)(pMsg);

  // Verify scan report is non-connectable, non-scannable and legacy
  if ((evt & GAP_EVT_ADV_REPORT)                         &&
#ifndef ZEPHYR_ADV_EXT
      (pAdvRpt->evtType & ADV_RPT_EVT_TYPE_LEGACY)       &&
#endif
     !(pAdvRpt->evtType & ADV_RPT_EVT_TYPE_CONNECTABLE)  &&
     !(pAdvRpt->evtType & ADV_RPT_EVT_TYPE_SCANNABLE))
  {

      // Set scanned adv type to non-connectable in Zephyr format
      adv_type = BT_HCI_ADV_NONCONN_IND;

      // Copy RSSI
      rssi = pAdvRpt->rssi;

      // Copy information to a zephyr formated buffer
      buf.data  = pAdvRpt->pData;
      buf.len   = pAdvRpt->dataLen;
      buf.size  = pAdvRpt->dataLen;
      buf.__buf = pAdvRpt->pData;

      // Copy BLE address
      memcpy(addr.a.val, pAdvRpt->addr, B_ADDR_LEN);

      // Copy BLE address type
      switch (pAdvRpt->addrType)
      {
        case ADDRTYPE_PUBLIC:
          addr.type = BT_ADDR_LE_PUBLIC;
        break;

        case ADDRTYPE_RANDOM:
        case ADDRTYPE_RANDOM_NR:
          addr.type = BT_ADDR_LE_RANDOM;
        break;

        case ADDRTYPE_PUBLIC_ID:
          addr.type = BT_ADDR_LE_PUBLIC_ID;
        break;

        case ADDRTYPE_RANDOM_ID:
          addr.type = BT_ADDR_LE_RANDOM_ID;
        break;

        case ADDRTYPE_NONE:
        default:
          addr.type = 0xFF; // Not defined in the zephyr profile but defined in
                            // the BLE spec Vol 2, Part E, 7.7.65.13
        break;
      }

      // Make sure zephyr scan cb was set
      if (NULL != pZephyrScanCb)
      {
        // Call zephyr scan callback
        (pZephyrScanCb)(&addr, rssi, adv_type, &buf);
      }
      else
      {
         BT_LE_PORTING_LAYER_ASSERT(FALSE);
      }
  }
  else if (evt & GAP_EVT_SCAN_ENABLED)
  {
    bMeshIsScanning = TRUE;
  }
  else if (evt & GAP_EVT_SCAN_DISABLED)
  {
    // Lower flag
    bMeshIsScanning = FALSE;

    // Reenable scanning if necessary
    if (bReEnableScan)
    {
      bReEnableScan = FALSE;
      bt_le_scan_start(&scanReEnableParam, pZephyrScanCb);
    }
  }
  return;
}


/******************************************************************************
 * @fn      mesh_portingLayer_registerZephyrScanCB
 *
 * @brief   Registers Zephyr side scan callback called by
 *          mesh_portingLayer_scanCB
 *
 * @param   cb - callback fxn to register
 *
 * @return  Zero on success or error code otherwise
 */
static bStatus_t mesh_portingLayer_registerZephyrScanCB(bt_le_scan_cb_t cb)
{
  // Verify input parameters are legal
  if (cb == NULL)
  {
    return INVALIDPARAMETER;
  }

  // Register cb
  pZephyrScanCb = cb;

  return SUCCESS;
}

#ifdef CONFIG_BT_MESH_PROXY
/******************************************************************************
 * @fn      mesh_portingLayer_newConnectionCB
 *
 * @brief   Converts parameters into zephyr type and calls zephyr
 *          connection cb
 *
 * @param   inputConn  - New link parameters
 *
 * @return  Zero on success or error code otherwise
 */
bStatus_t mesh_portingLayer_newConnectionCB(gapEstLinkReqEvent_t *inputConn)
{
  bStatus_t        status;
  struct bt_conn  *outputConn;

  // Verify input parameters are legal
  if (NULL == inputConn)
  {
    return INVALIDPARAMETER;
  }
#ifndef ZEPHYR_ADV_EXT
  // Stop the Mesh advertising
  status = bt_le_adv_stop();
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status || bleAlreadyInRequestedMode == status);
#endif
  // Get connection object pointer to fill
  outputConn = mesh_portingLayer_getFreeConn();
  if (NULL == outputConn)
  {
    // Max connections reached
    return FAILURE;
  }

  // Convert new connection params
  status = mesh_portingLayer_convertNewConnParams(inputConn,
                                                  outputConn);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Call zephyr proxy cb
  if (pZephyrConnCBs != NULL && pZephyrConnCBs->connected != NULL)
  {
    (pZephyrConnCBs->connected)(outputConn, outputConn->err);
  }
#ifdef ZEPHYR_ADV_EXT
  // Call zephyr extended adv cb
  if (pZephyrAdvSetCBs != NULL && pZephyrAdvSetCBs->connected != NULL)
  {
    (pZephyrAdvSetCBs->connected)(NULL, NULL);
  }
#endif
  return status;
}

/******************************************************************************
 * @fn      mesh_portingLayer_disconnectCB
 *
 * @brief   Converts parameters into zephyr type and calls zephyr
 *          disconnection cb
 *
 * @param   inputConn  - Terminated link parameters
 *
 * @return  Zero on success or error code otherwise
 */
bStatus_t mesh_portingLayer_disconnectCB(gapTerminateLinkEvent_t *inputConn)
{
  struct bt_conn  *connToDisconnect;
  bStatus_t        status;

  // Verify input parameters are legal
  if (NULL == inputConn)
  {
    return INVALIDPARAMETER;
  }

  // Get connection object pointer
  connToDisconnect = mesh_portingLayer_getConnByHandle(
                                                  inputConn->connectionHandle);
  BT_LE_PORTING_LAYER_ASSERT(NULL != connToDisconnect);

  if (pZephyrConnCBs != NULL && pZephyrConnCBs->disconnected != NULL)
  {
    (pZephyrConnCBs->disconnected)(connToDisconnect, inputConn->reason);
  }

  status = mesh_portingLayer_freeConnByHandle(connToDisconnect->handle);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  return status;
}

/******************************************************************************
 * @fn          mesh_portingLayer_gattReadAttCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
bStatus_t mesh_portingLayer_gattReadAttCB( uint16 connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8 *pValue, uint16 *pLen,
                                           uint16 offset, uint16 maxLen,
                                           uint8 method )
{
  bStatus_t status = SUCCESS;

  // Make sure it's not a blob operation(no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }

  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    switch ( uuid )
    {
      case GATT_MESH_PROV_DATA_OUT_CHAR_UUID:
        *pLen = lastProvDataOutValSize;
        VOID memcpy( pValue, pAttr->pValue, lastProvDataOutValSize );
        break;

      case GATT_MESH_PROXY_DATA_OUT_CHAR_UUID:
        *pLen = lastProxyDataOutValSize;
        VOID memcpy( pValue, pAttr->pValue, lastProxyDataOutValSize );
        break;

      case GATT_CLIENT_CHAR_CFG_UUID:
        VOID memcpy( pValue, pAttr->pValue, *pLen );

      default:
        // Should never get here!
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/******************************************************************************
 * @fn      mesh_portingLayer_proxyProvAttWriteCB
 *
 * @brief  Converts parameters into zephyr type and calls zephyr cb
 *
 * @param   params  - Write callback parameters structure
 *
 * @return  Zero on success or error code otherwise
 */
bStatus_t mesh_portingLayer_proxyProvAttWriteCB( writeCbParams_t *params )
{
  bStatus_t        status     = SUCCESS;

  uint16           connHandle = params->connHandle;
  gattAttribute_t *pAttr      = params->pAttr;
  uint8           *pValue     = params->pValue;
  uint16           len        = params->len;
  uint16           offset     = params->offset;


  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // Attribute to send zephyr
    struct bt_gatt_attr tmpAttr;
    struct bt_uuid_16 tmpUuid;
    tmpAttr.uuid = (struct bt_uuid *)(&tmpUuid);

    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    // Get zephyr connection structure
    struct bt_conn *zephyrConn = mesh_portingLayer_getConnByHandle(connHandle);

    switch ( uuid )
    {
      case GATT_MESH_PROV_DATA_IN_CHAR_UUID:
      case GATT_MESH_PROXY_DATA_IN_CHAR_UUID:
      {
        // Validate the value
        // Make sure it's not a blob operation
        if ( len == 0 )
        {
          status = ATT_ERR_INVALID_VALUE_SIZE;
        }

        // Convert received TI attribute to a zephyr type
        status = mesh_portingLayer_convertTIGattAtt(pAttr, &tmpAttr);

        // Call zephyr write cb
        BT_LE_PORTING_LAYER_ASSERT(NULL != pZephyrProxyProvWriteCb);
        pZephyrProxyProvWriteCb(zephyrConn, &tmpAttr, pValue, len, offset);
        break;
      }

      case GATT_CLIENT_CHAR_CFG_UUID:
      {
        uint16 value = BUILD_UINT16( pValue[0], pValue[1] );

        bool cccValChanged = (GATTServApp_ReadCharCfg( connHandle,
                                       GATT_CCC_TBL(pAttr->pValue)) != value);

        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue,
                                                 len, offset,
                                                 GATT_CLIENT_CFG_NOTIFY );
        BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

        // Convert received TI attribute to a zephyr type
        status = mesh_portingLayer_convertTIGattAtt(pAttr, &tmpAttr);
        BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

        // Get service uuid related to the relevant ccc
        // "-5" means we transition back 5 attributes before the CCC to get
        // the service declaration
        uint16_t serviceUuid =
            BUILD_UINT16(((gattAttrType_t *)(pAttr[-5].pValue))->uuid[0],
                         ((gattAttrType_t *)(pAttr[-5].pValue))->uuid[1]);

        // Check which CCC was written, provisioning or proxy
        if (serviceUuid == GATT_MESH_PROV_SERVICE_UUID)
        {
          // Call zephyr ccc write cb
          BT_LE_PORTING_LAYER_ASSERT(NULL != zephyrProvCCCWriteCb);
          zephyrProvCCCWriteCb(zephyrConn, &tmpAttr, (uint16_t)(*pValue));

          // If CCC changed, call zephyr cb
          if (cccValChanged)
          {
            zephyrProvCCCChangedCb(&tmpAttr, (uint16_t)(*pValue));
          }
        }
        else if (serviceUuid == GATT_MESH_PROXY_SERVICE_UUID)
        {
          // Call zephyr ccc write cb
          BT_LE_PORTING_LAYER_ASSERT(NULL != zephyrProxyCCCWriteCb);
          zephyrProxyCCCWriteCb(zephyrConn, &tmpAttr, (uint16_t)(*pValue));

          // If CCC changed, call zephyr cb
          if (cccValChanged)
          {
            zephyrProxyCCCChangedCb(&tmpAttr, (uint16_t)(*pValue));
          }
        }

        break;
      }
      default:
      {
        // Should never get here! (other characteristics do not have write
        // permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
      }
    }
  }
  else
  {
    // 128-bit UUID
    status = ATT_ERR_INVALID_HANDLE;
  }

  // Free data from app
  ICall_free(pValue);

  return SUCCESS;
}

/******************************************************************************
 * LOCAL FUNCTIONS
 */
/******************************************************************************
 * @fn      mesh_portingLayer_initConnHandle
 *
 * @brief   Initializes the handle in internal connection structure
 */

static void mesh_portingLayer_initConnHandle(void)
{
  int i = 0;

  // Initialize connection objects list
  for (i = 0; i < CONN_LIST_SIZE; i++)
  {
    struct bt_conn * currConn = &connectionList[i];

    currConn->handle = BT_MESH_CONN_FREE;
  }

}

/******************************************************************************
 * @fn      mesh_portingLayer_getConnByHandle
 *
 * @brief   Get the connection object
 *
 * @param   connHandle  - Connection handle to find
 *
 * @return  Pointer to found bt_conn structure or NULL if handle does not match
 *          existing handle
 */
static struct bt_conn *mesh_portingLayer_getConnByHandle(
                                                     uint16_t connectionHandle)
{
  int i = 0;

  // Search for a connection with the given connection handle
  // and return it's instance if found
  for (i = 0; i < CONN_LIST_SIZE; i++)
  {
    struct bt_conn * currConn = &connectionList[i];

    if ( (currConn->handle != BT_MESH_CONN_FREE) && (currConn->handle == connectionHandle) )
    {
      return currConn;
    }
  }

  // If no connection object is found, return NULL
  return NULL;
}

/******************************************************************************
 * @fn      mesh_portingLayer_getFreeConn
 *
 * @brief   Get the free connection object
 *
 * @return  Pointer to new free bt_conn structure or NULL if not free
 */
static struct bt_conn *mesh_portingLayer_getFreeConn(void)
{
  int i = 0;


  // Find the first available connection object and return it
  for (i = 0; i < CONN_LIST_SIZE; i++)
  {
    struct bt_conn * currConn = &connectionList[i];

    if (currConn->handle == BT_MESH_CONN_FREE)
    {
      return currConn;
    }
  }

  // If no connection object is available, return NULL
  return NULL;
}

/******************************************************************************
 * @fn      mesh_portingLayer_freeConnByHandle
 *
 * @brief   Free connection object
 *
 * @param   connHandle  - Connection handle to mark as free
 *
 * @return  Zero on success or error code otherwise
 */
static int mesh_portingLayer_freeConnByHandle(uint16_t connHandle)
{
  struct bt_conn *connToFree = NULL;

  // Get pointer to connection object in need of freeing
  connToFree = mesh_portingLayer_getConnByHandle(connHandle);
  BT_LE_PORTING_LAYER_ASSERT(NULL != connToFree);

  connToFree->handle = BT_MESH_CONN_FREE;

  return SUCCESS;
}

/******************************************************************************
 * @fn      mesh_portingLayer_updateLinkMTU
 *
 * @brief   Update link information.
 *
 *          Called from application on GAP_LINK_PARAM_UPDATE_EVENT
 *
 * @param   - lnkInf Link information structure
 */
void mesh_portingLayer_updateLinkMTU(uint16_t linkMTU)
{
  mtuSize = linkMTU;
}

/******************************************************************************
 * @fn      mesh_portingLayer_setDeviceName
 *
 * @brief   Set device name that comes from the ti_ble_config.c
 *
 *          Called from application on bleEnable
 *
 * @param   - name name to set
 */
void mesh_portingLayer_setDeviceName(uint8_t *name)
{
  pBleName = (const char *)name;
}
#endif /* CONFIG_BT_MESH_PROXY */


/******************************************************************************
 * PUBLIC FUNCTIONS
 */

/******************************************************************************
 * @fn      bt_le_adv_start
 *
 * @brief   Create advertisement set and start advertising
 *
 * @param   - param  - pointer to a bt_le_adv_param struct
 * @param   - ad     - pointer to advertisement data
 * @param   - ad_len - number of AD structures for advertisement data
 * @param   - sd     - pointer to scan data
 * @param   - sd_len - number of AD structures for scan response data
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_adv_start(const struct bt_le_adv_param *param,
                    const struct bt_data *ad, size_t ad_len,
                    const struct bt_data *sd, size_t sd_len)
{
  bStatus_t        status;
  GapAdv_params_t  tiParams;
  ICall_Errno      err          = ICALL_ERRNO_SUCCESS;
  static uint8_t  *advData      = NULL;
  static uint16_t  advLen       = 0;
  uint8_t          newAdvLen     = 0;
  uint8_t          insertIdx;
#ifdef CONFIG_BT_MESH_PROXY
  static uint8_t  *scnRspData   = NULL;
  static uint8_t   scnRspLen    = 0;
  uint8_t          newSdLen      = 0;
#endif /* CONFIG_BT_MESH_PROXY */

  // Verify input parameters are legal
  if (!param || !ad || (0 == ad_len))
  {
    return -EINVAL;
  }

  // Sum up total size of all AD structs of advertising data
  for (int i = 0; i < ad_len; i++)
  {
    newAdvLen += ad[i].data_len+2;  // add 2 for length and type parameters
  }
#ifdef CONFIG_BT_MESH_PROXY
  // Sum up total size of all AD structs of scan response data
  for (int i = 0; i < sd_len; i++)
  {
    newSdLen += sd[i].data_len+2;  // add 2 for length and type parameters
  }
#endif /* CONFIG_BT_MESH_PROXY */

  // Parse Zephyr BLE Stack structures into TI BLE Stack structures
  status = mesh_portingLayer_convertZephAdvParam(param, &tiParams);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // GapAdv_create (with meshAdvHandle) & ICall_registerApp (in the adv_thread)
  // must not be called more than once
  if (meshAdvFirst)
  {
    err = bleStack_register(&advThreadSelfEntity, 0);
    BT_LE_PORTING_LAYER_ASSERT(ICALL_ERRNO_SUCCESS == err);

    // Create Advertisement set and assign handle
    status = GapAdv_create(&SimpleMeshNode_advCB, &tiParams, &meshAdvHandle);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Set event mask for set
    status = GapAdv_setEventMask(meshAdvHandle, GAP_ADV_EVT_MASK_ALL);

#ifdef CONFIG_BT_MESH_PROXY
    mesh_portingLayer_initConnHandle();
#endif /* CONFIG_BT_MESH_PROXY */
  }

  // If data length has changed, free buffer and reallocate a new one
  if (newAdvLen != advLen)
  {
    // Only free if previously allocated
    if (!meshAdvFirst)
    {
      status = GapAdv_prepareLoadByHandle(meshAdvHandle,
                                          GAP_ADV_FREE_OPTION_ADV_DATA);
      BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);
    }

    // Adv total length must also include len+type to the adv buffer
    advLen = newAdvLen;

    // Allocate a data buffer for the advertisement data
    advData = ICall_malloc(advLen);
    BT_LE_PORTING_LAYER_ASSERT(NULL != advData);
  }

  // Trigger the setParam cmd to update the adv eventProps
  status = GapAdv_setParam(meshAdvHandle, GAP_ADV_PARAM_PROPS,
                           &(tiParams.eventProps));
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Trigger the setParam cmd to update the adv primIntMin
  status = GapAdv_setParam(meshAdvHandle, GAP_ADV_PARAM_PRIMARY_INTERVAL_MIN,
                           &(tiParams.primIntMin));
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Trigger the setParam cmd to update the adv primIntMax
  status = GapAdv_setParam(meshAdvHandle, GAP_ADV_PARAM_PRIMARY_INTERVAL_MAX,
                           &(tiParams.primIntMax));
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Prepare insert index
  insertIdx = 0;

  // Create advertisement data - iterate over all AD structures
  for (int i = 0; i < ad_len; i++)
  {

    // Setup adv header
    advData[insertIdx++] = ad[i].data_len+1; // Length of adv data + type
    advData[insertIdx++] = ad[i].type;

    // Copy advertisement data
    memcpy(&advData[insertIdx], ad[i].data, ad[i].data_len);

    // Advance insertion index by data_len
    insertIdx += ad[i].data_len;
  }

  // Load advertising data for adv set that is statically allocated by the app
  status = GapAdv_loadByHandle(meshAdvHandle, GAP_ADV_DATA_TYPE_ADV, advLen,
                               advData);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Load scan response data for proxy advertisements
  if (sd_len)
  {
#ifdef CONFIG_BT_MESH_PROXY
    // If data length has changed
    if (newSdLen != scnRspLen)
    {
      // Pointer has been initialized and is not null
      if (NULL != scnRspData)
      {
        // Free the old buffer and disable advertising if running
        status = GapAdv_prepareLoadByHandle(meshAdvHandle,
                                           GAP_ADV_FREE_OPTION_SCAN_RESP_DATA);
        BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);
      }

      // Update total scan response data length
      scnRspLen = newSdLen;

      // Allocate a data buffer for the scan response data
      scnRspData = ICall_malloc(scnRspLen);
      BT_LE_PORTING_LAYER_ASSERT(NULL != scnRspData);
    }

    // Prepare insert index
    insertIdx = 0;

    // Create scan response data - iterate over all AD structures
    for (int i = 0; i < sd_len; i++)
    {

      // Setup adv header
      scnRspData[insertIdx++] = sd[i].data_len+1; // Length of adv data + type
      scnRspData[insertIdx++] = sd[i].type;

      // Copy advertisement data
      memcpy(&scnRspData[insertIdx], sd[i].data, sd[i].data_len);

      // Advance insertion index by data_len
      insertIdx += sd[i].data_len;
    }

    // Load scan response data to adv set
    status = GapAdv_loadByHandle(meshAdvHandle, GAP_ADV_DATA_TYPE_SCAN_RSP,
                                 scnRspLen, scnRspData);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);
#else /* CONFIG_BT_MESH_PROXY */
    // Proxy must be enabled for scannable advertisement - Profile defines are
    // not aligned
    BT_LE_PORTING_LAYER_ASSERT(FAILURE);
#endif /* CONFIG_BT_MESH_PROXY */
  }

  // Enable continuous advertising (The zephyr default)
  status = GapAdv_enable(meshAdvHandle, GAP_ADV_ENABLE_OPTIONS_USE_MAX , 0);

  // Update flag
  meshAdvFirst = false;

  return status;
}

/******************************************************************************
 * @fn      bt_le_adv_stop
 *
 * @brief   Stop advertising
 *
 * @param   void
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_adv_stop(void)
{
  bStatus_t status;

 // Disable Advertising of mesh adv set
  status = GapAdv_disable(meshAdvHandle);

  return status;
}

#ifdef ZEPHYR_ADV_EXT
/******************************************************************************
 * @fn      bt_le_ext_adv_create
 *
 * @brief   Create advertisement set
 *
 * @param   - param - pointer to a bt_le_adv_param struct
 * @param   - cb    - pointer to a bt_le_ext_adv_cb struct
 * @param   - adv   - pointer to a bt_le_ext_adv struct
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_ext_adv_create(const struct bt_le_adv_param *param,
                         const struct bt_le_ext_adv_cb *cb,
                         struct bt_le_ext_adv **adv)
{
    bStatus_t        status;
    GapAdv_params_t  tiParams;

    (*adv) = ICall_malloc(sizeof(struct bt_le_ext_adv));

    // Verify input parameters are legal
    if (!(*adv) || !param)
    {
      return -EINVAL;
    }

    // Register the advertise set CBs
    pZephyrAdvSetCBs = cb;

    // Parse Zephyr BLE Stack structures into TI BLE Stack structures
    status = mesh_portingLayer_convertZephAdvParam(param, &tiParams);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Create Advertisement set and assign handle
    status = GapAdv_create(&SimpleMeshNode_advCB, &tiParams, &((*adv)->handle));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Set event mask for set
    status = GapAdv_setEventMask(((*adv)->handle), GAP_ADV_EVT_MASK_ALL);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Trigger the setParam cmd to update the adv eventProps
    status = GapAdv_setParam(((*adv)->handle), GAP_ADV_PARAM_PROPS,
                             &(tiParams.eventProps));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Trigger the setParam cmd to update the adv primIntMin
    status = GapAdv_setParam(((*adv)->handle), GAP_ADV_PARAM_PRIMARY_INTERVAL_MIN,
                             &(tiParams.primIntMin));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Trigger the setParam cmd to update the adv primIntMax
    status = GapAdv_setParam(((*adv)->handle), GAP_ADV_PARAM_PRIMARY_INTERVAL_MAX,
                             &(tiParams.primIntMax));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    return status;
}

/******************************************************************************
 * @fn      bt_le_ext_adv_start
 *
 * @brief   Start advertising
 *
 * @param   - adv   - pointer to a bt_le_ext_adv struct
 * @param   - param - pointer to a bt_le_adv_param struct
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_ext_adv_start(struct bt_le_ext_adv *adv,
                        struct bt_le_ext_adv_start_param *param)
{
    bStatus_t status;

    // Verify input parameters are legal
    if (!adv || !param)
    {
      return -EINVAL;
    }

    if (param->timeout == 0 && param->num_events == 0)
    {
        // Enable continuous advertising
        status = GapAdv_enable(adv->handle, GAP_ADV_ENABLE_OPTIONS_USE_MAX, 0);
    }
    else if (param->timeout > param->num_events)
    {
        // Enable advertising for a specific duration (param->timeout)
        status = GapAdv_enable(adv->handle, GAP_ADV_ENABLE_OPTIONS_USE_DURATION, param->timeout);
    }
    else
    {
        // Enable advertising for maximum number of events (param->num_events)
        status = GapAdv_enable(adv->handle, GAP_ADV_ENABLE_OPTIONS_USE_MAX_EVENTS, param->num_events);
    }
    // Update flag
    if (status == SUCCESS)
    {
        meshAdvFirst = false;
    }

    return status;
}

/******************************************************************************
 * @fn      bt_le_ext_adv_stop
 *
 * @brief   Stop advertising
 *
 * @param   - adv - pointer to a bt_le_ext_adv struct
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_ext_adv_stop(struct bt_le_ext_adv *adv)
{
    bStatus_t status;

    // Verify input parameters are legal
    if (!adv)
    {
      return -EINVAL;
    }

   // Disable advertising of mesh extended adv set
    status = GapAdv_disable(adv->handle);

    return status;
}

/******************************************************************************
 * @fn      bt_le_ext_adv_set_data
 *
 * @brief   Set advertise data and scan response data
 *          Note: According to the spec, for extended advertisement,
 *                the advertisement shall not be both connectable and
 *                scannable.
 *                Since the advertise set is extended and connectable,
 *                the scan response (sd) data is ignored in this function.
 *
 * @param   - adv    - pointer to a bt_le_ext_adv struct
 * @param   - ad     - pointer to advertisement data
 * @param   - ad_len - number of AD structures for advertisement data
 * @param   - sd     - pointer to scan data
 * @param   - sd_len - number of AD structures for scan response data
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_ext_adv_set_data(struct bt_le_ext_adv *adv,
                           const struct bt_data *ad, size_t ad_len,
                           const struct bt_data *sd, size_t sd_len)
{
    bStatus_t status;
    static uint8_t  *advDataExt      = NULL;
    static uint16_t  advLenExt       = 0;
    uint8_t          newAdvLen       = 0;
    uint8_t          insertIdx;

    // Verify input parameters are legal
    if (!adv || !ad || (0 == ad_len))
    {
      return -EINVAL;
    }

    // Sum up total size of all AD structs of advertising data
    for (int i = 0; i < ad_len; i++)
    {
      newAdvLen += ad[i].data_len+2;  // add 2 for length and type parameters
    }

    // If data length has changed, free buffer and reallocate a new one
    if (newAdvLen != advLenExt)
    {
      // Only free if previously allocated
      if (!meshAdvFirst)
      {
        status = GapAdv_prepareLoadByHandle(adv->handle,
                                            GAP_ADV_FREE_OPTION_ADV_DATA);
        BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);
      }

      // Adv total length must also include len+type to the adv buffer
      advLenExt = newAdvLen;

      // Allocate a data buffer for the advertisement data
      advDataExt = ICall_malloc(advLenExt);
      BT_LE_PORTING_LAYER_ASSERT(NULL != advDataExt);
    }

    // Prepare insert index
    insertIdx = 0;

    // Create advertisement data - iterate over all AD structures
    for (int i = 0; i < ad_len; i++)
    {
      // Setup adv header
      advDataExt[insertIdx++] = ad[i].data_len+1; // Length of adv data + type
      advDataExt[insertIdx++] = ad[i].type;

      // Copy advertisement data
      memcpy(&advDataExt[insertIdx], ad[i].data, ad[i].data_len);

      // Advance insertion index by data_len
      insertIdx += ad[i].data_len;
    }

    // Load advertising data for adv set that is statically allocated by the app
    status = GapAdv_loadByHandle(adv->handle, GAP_ADV_DATA_TYPE_ADV, advLenExt, advDataExt);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  return status;
}

/******************************************************************************
 * @fn      bt_le_ext_adv_update_param
 *
 * @brief   Update the advertise set parameters
 *
 * @param   - adv   - pointer to a bt_le_ext_adv struct
 * @param   - param - pointer to a bt_le_adv_param struct
 *
 * @return Zero on success or (negative) error code otherwise.
 */
int bt_le_ext_adv_update_param(struct bt_le_ext_adv *adv,
                               const struct bt_le_adv_param *param)
{
    bStatus_t status;
    GapAdv_params_t  tiParams;

    // Parse Zephyr BLE Stack structures into TI BLE Stack structures
    status = mesh_portingLayer_convertZephAdvParam(param, &tiParams);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Trigger the setParam cmd to update the adv eventProps
    status = GapAdv_setParam(adv->handle, GAP_ADV_PARAM_PROPS,
                             &(tiParams.eventProps));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Trigger the setParam cmd to update the adv primIntMin
    status = GapAdv_setParam(adv->handle, GAP_ADV_PARAM_PRIMARY_INTERVAL_MIN,
                             &(tiParams.primIntMin));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    // Trigger the setParam cmd to update the adv primIntMax
    status = GapAdv_setParam(adv->handle, GAP_ADV_PARAM_PRIMARY_INTERVAL_MAX,
                             &(tiParams.primIntMax));
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

    return status;
}
#endif

/******************************************************************************
 * @fn      bt_le_scan_start
 *
 * @brief   Start scanning with parameters
 *
 * @param   param - pointer to scanning parameters
 * @param   cb    - pointer to advertisement data
 *
 * @return  Generic return status
 */
int bt_le_scan_start(const struct bt_le_scan_param *param, bt_le_scan_cb_t cb)
{
  bStatus_t status;
  uint8_t   temp8, type, filter_dup;
  uint16_t  temp16, interval, window;

  status = mesh_portingLayer_convertZephScanParam(param, &type, &filter_dup,
                                                  &interval, &window);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Verify input parameters are legal
  if (NULL != cb)
  {
    status = mesh_portingLayer_registerZephyrScanCB(cb);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);
  }

  // Make sure scanning is not already disabled
  if (bMeshIsScanning)
  {
    // Disable scanning
    GapScan_disable();

    // Raise reenable flag
    bReEnableScan = TRUE;

    // Save param for reenabling
    memcpy(&scanReEnableParam, param, sizeof(struct bt_le_scan_param));

    return SUCCESS;
  }

  // Set Scanner Event Mask
  GapScan_setEventMask(GAP_EVT_SCAN_ENABLED | GAP_EVT_SCAN_DISABLED |
                       GAP_EVT_ADV_REPORT);

  // Set Scan PHY parameters
  status = GapScan_setPhyParams(DEFAULT_MESH_SCAN_PHY, type, interval, window);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Set LL Duplicate Filter
  status = GapScan_setParam(SCAN_PARAM_FLT_DUP, &filter_dup);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Set Advertising report fields to keep
  temp16 = DEFAULT_MESH_ADV_RPT_FIELDS;
  status = GapScan_setParam(SCAN_PARAM_RPT_FIELDS, &temp16);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Set Scanning Primary PHY
  temp8 = DEFAULT_MESH_SCAN_PHY;
  status = GapScan_setParam(SCAN_PARAM_PRIM_PHYS, &temp8);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Set PDU type filter
  temp16 = DEFAULT_MESH_SCAN_FLT_PDU;
  status = GapScan_setParam(SCAN_PARAM_FLT_PDU_TYPE, &temp16);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  bMeshIsScanning = TRUE;

  // Enable scanning
  status = GapScan_enable(0, DEFAULT_MESH_SCAN_DURATION, 0);

  return status;
}

/******************************************************************************
 * @fn      bt_le_scan_stop
 *
 * @brief   Stop scanning
 *
 * @param   none
 *
 * @return  Zero on success or error code otherwise, positive in case
 *          of protocol error or negative (POSIX) in case of stack internal
 *          error
 */
int bt_le_scan_stop(void)
{

  bStatus_t status;
  bReEnableScan = FALSE;

  // Make sure scanning is not already disabled
  if (bMeshIsScanning)
  {
    // Disable scanning of mesh adv set
    status = GapScan_disable();
  }
  else
  {
    // Already disabled
    status = SUCCESS;
  }
  return status;

}

/******************************************************************************
 * @fn      bt_le_register_work_q
 *
 * @brief first call in the context of Zephyr work_q, used to prepare/register to ICALL
 *
 */
void bt_le_register_work_q()
{
  ICall_Errno      err = ICALL_ERRNO_SUCCESS;
  // If the calling thread is the work_q, then it has not yet been registered
  // to ICall. This is needed in order to enable calling BLE stack APIs. This
  // must only be called once!
  if (bWorkQInitialized == FALSE)
  {
    err = bleStack_register(&workQThreadSelfEntity, 0);
    BT_LE_PORTING_LAYER_ASSERT(ICALL_ERRNO_SUCCESS == err);

    bWorkQInitialized = TRUE;
  }
}

#ifdef CONFIG_BT_MESH_PROXY
/******************************************************************************
 * @fn      bt_conn_cb_register
 *
 * @brief Register connection callbacks.
 *
 * Register callbacks to monitor the state of connections.
 *
 * @param cb Callback struct.
 */
void bt_conn_cb_register(struct bt_conn_cb *cb)
{
  pZephyrConnCBs = cb;
}

/******************************************************************************
 * @fn      bt_gatt_service_register
 *
 * @brief Register GATT service.
 *
 * Register GATT service. Applications can make use of
 * macros such as BT_GATT_PRIMARY_SERVICE, BT_GATT_CHARACTERISTIC,
 * BT_GATT_DESCRIPTOR, etc.
 *
 * @param svc Service containing the available attributes
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_gatt_service_register(struct bt_gatt_service *svc)
{
  bStatus_t      status;
  gattService_t *newGattService = ICall_malloc(sizeof(gattService_t));

  // Get first service type uuid
  uint16_t uuid = ((struct bt_uuid_16 *)(svc->attrs[0].uuid))->val;

  // Get service uuid
  uint16_t serviceUuid = ((struct bt_uuid_16 *)(svc->attrs[0].user_data))->val;

  // Convert Params
  status = mesh_portingLayer_convertZephGattService(svc, newGattService);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Register the service
  status = GATT_RegisterService( newGattService );
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // If provisioning or proxy service
  if ((uuid == GATT_PRIMARY_SERVICE_UUID) &&
      ((serviceUuid == GATT_MESH_PROV_SERVICE_UUID) ||
       (serviceUuid == GATT_MESH_PROXY_SERVICE_UUID)))
  {
    // Update zephyr attribute handles
    // Assign attribute handles
    for ( uint16 i = 0; i < newGattService->numAttrs; i++ )
    {
      svc->attrs[i].handle = newGattService->attrs[i].handle;
    }

    // Pointer to zephyr proxy_recv cb
    pZephyrProxyProvWriteCb = (pFxnZephyrWriteGattCb_t)(svc->attrs[2].write);

    // Set pointer to ccc cbs
    if (serviceUuid == GATT_MESH_PROV_SERVICE_UUID)
    {
      // The CCC is the 6th("[5]") attribute in the service
      zephyrProvCCCWriteCb =
          ((struct _bt_gatt_ccc *)(svc->attrs[5].user_data))->cfg_write;
      zephyrProvCCCChangedCb =
          ((struct _bt_gatt_ccc *)(svc->attrs[5].user_data))->cfg_changed;
    }
    else // GATT_MESH_PROXY_SERVICE_UUID
    {
      // The CCC is the 6th("[5]") attribute in the service
      zephyrProxyCCCWriteCb =
          ((struct _bt_gatt_ccc *)(svc->attrs[5].user_data))->cfg_write;
      zephyrProxyCCCChangedCb =
          ((struct _bt_gatt_ccc *)(svc->attrs[5].user_data))->cfg_changed;
    }

    // Register the service CBs with GATT Server Application
    status = gattServApp_RegisterServiceCBs(
                                    GATT_SERVICE_HANDLE(newGattService->attrs),
                                    &mesh_portingLayer_provProxyServiceCBs);
    BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);
  }
  else // generic zephyr service
  {
    // To be implemented
    BT_LE_PORTING_LAYER_ASSERT(FAILURE);
  }

  return status;
}

/******************************************************************************
 * @fn      bt_gatt_service_unregister
 *
 * @brief Unregister GATT service.
 *
 * @param svc Service to be unregistered.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_gatt_service_unregister(struct bt_gatt_service *svc)
{
  bStatus_t     status;
  gattService_t *pService = NULL;
  uint16_t       handle   = svc->attrs[0].handle;

  if (svc->attrs[0].handle == (getProvService())->attrs[0].handle)
  {
    pService = getProvService();
    setProvService(NULL);
  }
  else if (svc->attrs[0].handle == (getProxyService())->attrs[0].handle)
  {
    pService = getProxyService();
    setProxyService(NULL);
  }
  else
  {
    // Generic service - To be implemented
    BT_LE_PORTING_LAYER_ASSERT(FAILURE);
  }

  // Register the service
  status = GATT_DeregisterService(handle, pService );
  BT_LE_PORTING_LAYER_ASSERT((SUCCESS == status) && (NULL != pService));

  ICall_free(pService);

  return status;
}

/******************************************************************************
 * @fn      bt_get_name
 *
 * @brief   Returns BLE Device Name
 *
 * @param   None
 *
 * @return  Device name as a string
 */
const char *bt_get_name(void)
{
  return pBleName;
}

/******************************************************************************
 * @fn      bt_conn_disconnect
 *
 * @brief   Disconnect from a remote device or cancel pending connection.
 *
 * @param conn Connection to disconnect.
 * @param reason Reason code for the disconnection.
 *
 * @return Zero on success or (negative) error code on failure.
 */
int bt_conn_disconnect(struct bt_conn *conn, uint8_t reason)
{
    GAP_TerminateLinkReq(conn->handle, HCI_DISCONNECT_REMOTE_USER_TERM);
    return 0;
}

/******************************************************************************
 * @fn      bt_conn_ref
 *
 * @brief Increment a connection's reference count.
 *
 * Increment the reference count of a connection object.
 *
 * @param conn Connection object.
 *
 * @return Connection object with incremented reference count.
 */
struct bt_conn *bt_conn_ref(struct bt_conn *conn)
{
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION(cs);
  (conn->ref)++;
  HAL_EXIT_CRITICAL_SECTION(cs);

  return conn;
}

/******************************************************************************
 * @fn      bt_conn_unref
 *
 * @brief Decrement a connection's reference count.
 *
 * Decrement the reference count of a connection object.
 *
 * @param conn Connection object.
 *
 * @return Connection object with incremented reference count.
 */
void bt_conn_unref(struct bt_conn *conn)
{
  halIntState_t cs;

  HAL_ENTER_CRITICAL_SECTION(cs);
  (conn->ref)--;
  HAL_EXIT_CRITICAL_SECTION(cs);

}

/******************************************************************************
 * @fn      bt_gatt_get_mtu
 *
 * @brief Get ATT MTU for a connection
 *
 * Get negotiated ATT connection MTU, note that this does not equal the largest
 * amount of attribute data that can be transferred within a single packet.
 *
 * @param conn Connection object.
 *
 * @return MTU in bytes
 */
uint16_t bt_gatt_get_mtu(struct bt_conn *conn)
{
    return mtuSize;
}

/******************************************************************************
 * @fn      bt_gatt_notify_cb
 *
 * @brief Notify attribute value change.
 *
 * This function works in the same way as @ref bt_gatt_notify.
 * With the addition that after sending the notification the
 * callback function will be called.
 *
 * The callback is run from System Workqueue context.
 *
 * Alternatively it is possible to notify by UUID by setting it on the
 * parameters, when using this method the attribute given is used as the
 * start range when looking up for possible matches.
 *
 * @param conn Connection object.
 * @param params Notification parameters.
 *
 * @return 0 in case of success or negative value in case of error.
 */
int bt_gatt_notify_cb(struct bt_conn *conn,
                      struct bt_gatt_notify_params *params)
{
  gattAttribute_t *pAttr = NULL;
  bStatus_t        status;
  gattService_t   *pService;
  gattCharCfg_t   *pChrCfg;

  // Get pointers for gatt server app. 6 is the number of attributes in the svc
  for (int i = 0; i < 6; i++)
  {
    // Check if attribute is from provisioning table
    if (getProvService() && (getProvService())->attrs[i].handle == params->attr->handle)
    {
      pService = getProvService();
      pChrCfg = getProvClientCharConfig();
      pAttr = &((getProvService())->attrs[i+1]);
      lastProvDataOutValSize = params->len;
      break;
    }
    // Check if attribute is from proxy table
    else if (getProxyService() && (getProxyService())->attrs[i].handle == params->attr->handle)
    {
      pService = getProxyService();
      pChrCfg = getProxyClientCharConfig();
      pAttr = &((getProxyService())->attrs[i+1]);
      lastProxyDataOutValSize = params->len;
      break;
    }
  }
  // This should not fail when no other services where registered by zephyr
  BT_LE_PORTING_LAYER_ASSERT(NULL != pAttr);

  // Write to attribute
  memcpy(pAttr->pValue, params->data, params->len);

  // Send Notification if enabled
  status = GATTServApp_ProcessCharCfg(pChrCfg, pAttr->pValue, FALSE,
                             pService->attrs, pService->numAttrs,
                             INVALID_TASK_ID, mesh_portingLayer_gattReadAttCB);
  BT_LE_PORTING_LAYER_ASSERT(SUCCESS == status);

  // Clear attribute data for security purposes
  memset(pAttr->pValue, 0, params->len);

  // Call the notification value callback
  if(params->func)
  {
      params->func(conn, params->user_data);
  }

  return SUCCESS;
}
#endif /* CONFIG_BT_MESH_PROXY */
