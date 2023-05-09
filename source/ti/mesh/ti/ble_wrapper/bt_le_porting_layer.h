/*
 * porting_layer.h
 *
 *  Created on: Nov 27, 2019
 *      Author: a0220089
 */

#ifndef MESH_PORTING_LAYER_H_
#define MESH_PORTING_LAYER_H_

#include <stdint.h>

#include "autoconf.h"
#include <zephyr.h>
#include <hci.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include <gap_advertiser.h>
#include <gap_scanner.h>
#include <host/hci_core.h>
#include <bluetooth/hci.h>
#include <menu/two_btn_menu.h>
#include <ti_ble_config.h>

#ifdef CONFIG_BT_MESH_PROXY
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>

#include <simple_gatt_profile.h>
#include <gatt.h>
#include <gatt_uuid.h>
#include <gattservapp.h>
#include <linkdb.h>
#endif

// Spin if the expression is not true
#define BT_LE_PORTING_LAYER_ASSERT(expr) if (!(expr)) mesh_portingLayer_spin();


/******************************************************************************
 *  DEFINES
 */

#ifdef CONFIG_BT_MESH_LOG_PRINTS
#define BT_MESH_LOG_LAST_ROW (TBM_ROW_APP + 10)
#endif

/* Window and Interval are equal for continuous scanning */
// Default scan interval - 30 ms (in 625 us ticks)
#define DEFAULT_MESH_SCAN_INTERVAL   48
// Default scan window   - 30 ms (in 625 us ticks)
#define DEFAULT_MESH_SCAN_WINDOW     48
// Default scan duration - continuously scanning(in 10 ms units)
#define DEFAULT_MESH_SCAN_DURATION   0
// The default state of mesh scanning
#define DEFAULT_MESH_SCAN_TYPE       SCAN_TYPE_PASSIVE
#ifdef ZEPHYR_ADV_EXT
// Extended adv can be sent over 1M or coded PHY
#define DEFAULT_MESH_SCAN_PHY        SCAN_PRIM_PHY_CODED
#else
// Mesh 1.0 supports only 1M phy
#define DEFAULT_MESH_SCAN_PHY        SCAN_PRIM_PHY_1M
#endif
// Duplicate filter is disabled by default
#define DEFAULT_MESH_SCAN_FLT_DUP    SCAN_FLT_DUP_DISABLE
// PDU type filter
#ifdef ZEPHYR_ADV_EXT
  // Set PDU type filter (Only Non-connectable, Complete and Extended packets)
#define DEFAULT_MESH_SCAN_FLT_PDU   (SCAN_FLT_PDU_NONCONNECTABLE_ONLY | \
                                     SCAN_FLT_PDU_COMPLETE_ONLY       | \
                                     SCAN_FLT_PDU_EXTENDED_ONLY)
#else
  // Set PDU type filter (Only Non-connectable, Complete and Legacy packets)
#define DEFAULT_MESH_SCAN_FLT_PDU   (SCAN_FLT_PDU_NONCONNECTABLE_ONLY | \
                                     SCAN_FLT_PDU_COMPLETE_ONLY       | \
                                     SCAN_FLT_PDU_LEGACY_ONLY)
#endif

// Advertising report fields to keep in the list - by default keep all fields
// except secondary phy
#define DEFAULT_MESH_ADV_RPT_FIELDS (SCAN_ADVRPT_FLD_EVENTTYPE        | \
                                     SCAN_ADVRPT_FLD_ADDRTYPE         | \
                                     SCAN_ADVRPT_FLD_ADDRESS          | \
                                     SCAN_ADVRPT_FLD_PRIMPHY          | \
                                     SCAN_ADVRPT_FLD_ADVSID           | \
                                     SCAN_ADVRPT_FLD_TXPOWER          | \
                                     SCAN_ADVRPT_FLD_RSSI             | \
                                     SCAN_ADVRPT_FLD_DIRADDRTYPE      | \
                                     SCAN_ADVRPT_FLD_DIRADDRESS       | \
                                     SCAN_ADVRPT_FLD_PRDADVINTERVAL   | \
                                     SCAN_ADVRPT_FLD_DATALEN)


#ifdef CONFIG_BT_MESH_PROXY
// GATT Declarations
#define GATT_MESH_PROV_SERVICE_UUID        0x1827
#define GATT_MESH_PROXY_SERVICE_UUID       0x1828

#define GATT_MESH_PROV_DATA_IN_CHAR_UUID   0x2ADB
#define GATT_MESH_PROV_DATA_OUT_CHAR_UUID  0x2ADC
#define GATT_MESH_PROXY_DATA_IN_CHAR_UUID  0x2ADD
#define GATT_MESH_PROXY_DATA_OUT_CHAR_UUID 0x2ADE
#endif

/******************************************************************************
 * STRUCTURES
 */

static GapAdv_params_t meshAdvParams = {
  .eventProps =   GAP_ADV_PROP_LEGACY,
  .primIntMin =   160,
  .primIntMax =   160,
  .primChanMap =  GAP_ADV_CHAN_ALL,
  .peerAddrType = PEER_ADDRTYPE_PUBLIC_OR_PUBLIC_ID,
  .peerAddr =     { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa },
  .filterPolicy = GAP_ADV_WL_POLICY_ANY_REQ,
  .txPower =      GAP_ADV_TX_POWER_NO_PREFERENCE,
#ifdef ZEPHYR_ADV_EXT
  .primPhy =      GAP_ADV_PRIM_PHY_CODED_S2,
  .secPhy =       GAP_ADV_SEC_PHY_CODED_S2,
#else
  .primPhy =      GAP_ADV_PRIM_PHY_1_MBPS,
  .secPhy =       GAP_ADV_SEC_PHY_1_MBPS,
#endif
  .sid =          0
};

#ifdef CONFIG_BT_MESH_PROXY
typedef ssize_t (*pFxnZephyrWriteGattCb_t)(struct bt_conn *conn,
                  const struct bt_gatt_attr *attr,
                  void *buf, uint16_t len, uint16_t offset);

typedef void (*pFxnZephyrGattCccChangedCb_t)(const struct bt_gatt_attr *attr,
               uint16_t value);

typedef ssize_t (*pFxnZephyrGattCccWriteCb_t)(struct bt_conn *conn,
             const struct bt_gatt_attr *attr,
             uint16_t value);

typedef struct
{
  uint16 connHandle;
  gattAttribute_t *pAttr;
  uint8 *pValue;
  uint16 len;
  uint16 offset;
  uint8 method;
}writeCbParams_t;
#endif

/******************************************************************************
 * EXTERN
 */

#ifdef CONFIG_BT_MESH_PROXY
// Primary Service UUID
extern CONST uint8 primaryServiceUUID[ATT_BT_UUID_SIZE];

// Characteristic UUID
extern CONST uint8 characterUUID[ATT_BT_UUID_SIZE];

// Client Characteristic Configuration UUID
extern CONST uint8 clientCharCfgUUID[ATT_BT_UUID_SIZE];
#endif

/******************************************************************************
 * SPINLOCK
 */

/******************************************************************************
 * @fn      mesh_portingLayer_spin
 *
 * @brief   Spin forever
 *
 * @param   none
 */
static void mesh_portingLayer_spin(void)
{
  volatile uint8_t x = 0;

  while(1)
  {
    x++;
  }
}

/******************************************************************************
 *  CALLBACKS
 */
/*********************************************************************
 * @fn      SimpleMeshNode_scanCB
 *
 * @brief   Callback called by GapScan module
 *
 * This callback should be defined in the application
 *
 * @param   evt - event
 * @param   msg - message coming with the event
 * @param   arg - user argument
 *
 * @return  none
 */
void SimpleMeshNode_scanCB(uint32_t evt, void* pMsg, uintptr_t arg);

/*********************************************************************
 * @fn      SimpleMeshNode_advCB
 *
 * @brief   GapAdv module callback
 *
 * This callback should be defined in the application
 *
 * @param   pMsg - message to process
 */
void SimpleMeshNode_advCB(uint32_t event, void *pBuf, uintptr_t arg);

#ifdef CONFIG_BT_MESH_PROXY
/******************************************************************************
 * @fn      mesh_portingLayer_proxyProvAttWriteCB
 *
 * @brief  Converts parameters into zephyr type and calls zephyr cb
 *
 * @param   params  - Write callback parameters structure
 *
 * @return  Zero on success or error code otherwise
 */
bStatus_t mesh_portingLayer_proxyProvAttWriteCB(writeCbParams_t *params);
#endif

/******************************************************************************
 * DATA CONVERSION
 */
bStatus_t mesh_portingLayer_convertZephAdvParam(
                                      const struct bt_le_adv_param *inParam,
                                      GapAdv_params_t *outParam);

bStatus_t mesh_portingLayer_convertZephScanParam(
                                      const struct bt_le_scan_param *inParam,
                                      uint8_t *type,
                                      uint8_t *filter_dup,
                                      uint16_t *interval,
                                      uint16_t *window);
#ifdef CONFIG_BT_MESH_PROXY
bStatus_t mesh_portingLayer_convertNewConnParams(
                                      gapEstLinkReqEvent_t *inputNewConnection,
                                      struct bt_conn *outputNewConnection);

bStatus_t mesh_portingLayer_convertTIGattAtt(
                                      gattAttribute_t *tmpInAtt,
                                      struct bt_gatt_attr *tmpOutAtt);

bStatus_t mesh_portingLayer_convertZephGattService(
                                      struct bt_gatt_service *inService,
                                      gattService_t *outService);
#endif

/******************************************************************************
 * PORTING LAYER PROTOTYPES
 */
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
bStatus_t mesh_portingLayer_newConnectionCB(
                                    gapEstLinkReqEvent_t *inputNewConnection);

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
bStatus_t mesh_portingLayer_disconnectCB(gapTerminateLinkEvent_t *inputConn);

/*********************************************************************
 * @fn      SimpleMeshNode_gattWriteCB
 *
 * @brief   Callback called by GATT Server module
 *
 * This callback enqueues a message to call in the application context
 *
 * @param   evt - event
 * @param   msg - message coming with the event
 * @param   arg - user argument
 *
 * @return  none
 */
void SimpleMeshNode_gattWriteCB( uint16 connHandle,
                                   gattAttribute_t *pAttr,
                                   uint8 *pValue, uint16 len,
                                   uint16 offset, uint8 method );
#endif

/******************************************************************************
 * GET/SET FUNCTIONS
 */
#ifdef CONFIG_BT_MESH_PROXY
/******************************************************************************
 * @fn      mesh_portingLayer_updateLinkMTU
 *
 * @brief   Update link information.
 *
 *          Called from application on GAP_LINK_PARAM_UPDATE_EVENT
 *
 * @param   - lnkInf Link information structure
 */
void mesh_portingLayer_updateLinkMTU(uint16_t linkMTU);

/******************************************************************************
 * @fn      mesh_portingLayer_setDeviceName
 *
 * @brief   Set device name that comes from the ti_ble_config.c
 *
 *          Called from application on bleEnable
 *
 * @param   - name name to set
 */
void mesh_portingLayer_setDeviceName(uint8_t *name);

gattService_t *getProvService(void);
void           setProvService(gattService_t *pNewService);
gattCharCfg_t *getProvClientCharConfig(void);

gattService_t *getProxyService(void);
void           setProxyService(gattService_t *pNewService);
gattCharCfg_t *getProxyClientCharConfig(void);

extern bStatus_t gattServApp_RegisterServiceCBs( uint16 handle, CONST gattServiceCBs_t *pServiceCBs );
#endif
#endif /* MESH_PORTING_LAYER_H_ */
