/******************************************************************************

@file  bt_le_convert.c

@brief This file contains the Zephyr Mesh-BLE porting layer's conversion
       methods and the Provisioning and Proxy services

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
#include <bt_le_porting_layer.h>
#include <logging/log.h>
#include <ti_ble_config.h>

#ifdef CONFIG_BT_MESH_PROXY
#include <host/conn_internal.h>
#endif

/******************************************************************************
* GLOBAL VARIABLES
*/
#ifdef CONFIG_BT_MESH_PROXY
// Pointer to the Provisioning Service structure
static gattService_t *pGattProvService = NULL;

// Pointer to the Proxy Service structure
static gattService_t *pGattProxyService = NULL;
#endif

/******************************************************************************
* LOCAL VARIABLES
*/
#ifdef CONFIG_BT_MESH_PROXY
/* ---------------------------- */
/* --- Provisioning Service --- */
/* ---------------------------- */

// Provisioning Service UUID
static CONST uint8 provServiceUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16( GATT_MESH_PROV_SERVICE_UUID ), HI_UINT16( GATT_MESH_PROV_SERVICE_UUID )
};

// Provisioning Data In Characteristic UUID
static CONST uint8 provDataInCharUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16( GATT_MESH_PROV_DATA_IN_CHAR_UUID ), HI_UINT16( GATT_MESH_PROV_DATA_IN_CHAR_UUID )
};

// Provisioning Data Out Characteristic UUID
static CONST uint8 provDataOutCharUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16( GATT_MESH_PROV_DATA_OUT_CHAR_UUID ), HI_UINT16( GATT_MESH_PROV_DATA_OUT_CHAR_UUID )
};

// Provisioning Service attribute
static CONST gattAttrType_t provService = { ATT_BT_UUID_SIZE, provServiceUUID };

// Provisioning Data In Characteristic Properties
static uint8 provDataInCharProps = GATT_PROP_WRITE_NO_RSP;

// Provisioning Data Out Characteristic Properties
static uint8 provDataOutCharProps = GATT_PROP_NOTIFY;

// Provisioning Data In Characteristic Value
static uint8 provDataInChar[66] = {0};

// Provisioning Data Out Characteristic Value
static uint8 provDataOutChar[66] = {0};

// Provisioning Client Characteristic Configuration
static gattCharCfg_t *pProvClientCharConfig;

// Provisioning Attribute Table
static gattAttribute_t provAttrTbl[] =
{
 // Provisioning Service Declaration
 {
   { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
   GATT_PERMIT_READ,                         /* permissions */
   0,                                        /* handle */
   (uint8 *)&provService                     /* pValue */
 },

 // don't we need the specific uuid????
 // Provisioning Data In Characteristic Declaration
 {
   { ATT_BT_UUID_SIZE, characterUUID },
   GATT_PERMIT_READ,
   0,
   &provDataInCharProps
 },

 // Provisioning Data In Characteristic Value
 {
   { ATT_BT_UUID_SIZE, provDataInCharUUID },
   GATT_PERMIT_WRITE,
   0,
   provDataInChar
 },

 // Provisioning Data Out Characteristic Declaration
 {
   { ATT_BT_UUID_SIZE, characterUUID },
   GATT_PERMIT_READ,
   0,
   &provDataOutCharProps
 },

 // Provisioning Data Out Characteristic Value
 {
   { ATT_BT_UUID_SIZE, provDataOutCharUUID },
   GATT_PERMIT_WRITE,
   0,
   provDataOutChar
 },

 // Client Characteristic configuration
 {
   { ATT_BT_UUID_SIZE, clientCharCfgUUID },
   GATT_PERMIT_READ | GATT_PERMIT_WRITE,
   0,
   (uint8 *)&pProvClientCharConfig
 }
};
/* --------------------- */
/* --- Proxy Service --- */
/* --------------------- */

// Proxy Service UUID
static CONST uint8 proxyServiceUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16( GATT_MESH_PROXY_SERVICE_UUID ), HI_UINT16( GATT_MESH_PROXY_SERVICE_UUID )
};

// Proxy Data In Characteristic UUID
static CONST uint8 proxyDataInCharUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16( GATT_MESH_PROXY_DATA_IN_CHAR_UUID ), HI_UINT16( GATT_MESH_PROXY_DATA_IN_CHAR_UUID )
};

// Proxy Data Out Characteristic UUID
static CONST uint8 proxyDataOutCharUUID[ATT_BT_UUID_SIZE] =
{
  LO_UINT16( GATT_MESH_PROXY_DATA_OUT_CHAR_UUID ), HI_UINT16( GATT_MESH_PROXY_DATA_OUT_CHAR_UUID )
};

// Proxy Service attribute
static CONST gattAttrType_t proxyService = { ATT_BT_UUID_SIZE, proxyServiceUUID };

// Proxy Data In Characteristic Properties
static uint8 proxyDataInCharProps = GATT_PROP_WRITE_NO_RSP;

// Proxy Data Out Characteristic Properties
static uint8 proxyDataOutCharProps = GATT_PROP_NOTIFY;

// Proxy Data In Characteristic Value
static uint8 proxyDataInChar[33] = {0};

// Proxy Data Out Characteristic Value
static uint8 proxyDataOutChar[33] = {0};

// Proxy Client Characteristic Configuration
static gattCharCfg_t *pProxyClientCharConfig;

// Proxy Attribute Table
static gattAttribute_t proxyAttrTbl[] =
{
 // Proxy Service Declaration
 {
   { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
   GATT_PERMIT_READ,                         /* permissions */
   0,                                        /* handle */
   (uint8 *)&proxyService                     /* pValue */
 },

 // Proxy Data In Characteristic Declaration
 {
   { ATT_BT_UUID_SIZE, characterUUID },
   GATT_PERMIT_READ,
   0,
   &proxyDataInCharProps
 },

 // Proxy Data In Characteristic Value
 {
   { ATT_BT_UUID_SIZE, proxyDataInCharUUID },
   GATT_PERMIT_WRITE,
   0,
   proxyDataInChar
 },

 // Proxy Data Out Characteristic Declaration
 {
   { ATT_BT_UUID_SIZE, characterUUID },
   GATT_PERMIT_READ,
   0,
   &proxyDataOutCharProps
 },

 // Proxy Data Out Characteristic Value
 {
   { ATT_BT_UUID_SIZE, proxyDataOutCharUUID },
   GATT_PERMIT_WRITE,
   0,
   proxyDataOutChar
 },

 // Client Characteristic configuration
 {
   { ATT_BT_UUID_SIZE, clientCharCfgUUID },
   GATT_PERMIT_READ | GATT_PERMIT_WRITE,
   0,
   (uint8 *)&pProxyClientCharConfig
 }
};
#endif
/******************************************************************************
 * PUBLIC FUNCTIONS
 */

bStatus_t mesh_portingLayer_convertZephAdvParam(const struct bt_le_adv_param *inParam,
                                                             GapAdv_params_t *outParam)
{
  // Set output params to default - needed?
  memcpy(outParam,&meshAdvParams,sizeof(GapAdv_params_t));

  // Check Validity of Min Interval
  if ((inParam->interval_min == 0) ||
      (inParam->interval_min < LL_ADV_CONN_INTERVAL_MIN) ||
      (inParam->interval_min > LL_ADV_CONN_INTERVAL_MAX))
  {
    return EINVAL;
  }
  else
  {
    outParam->primIntMin = inParam->interval_min;
  }

  // Check Validity of Max Interval
  if ((inParam->interval_max == 0) ||
      (inParam->interval_max < LL_ADV_CONN_INTERVAL_MIN) ||
      (inParam->interval_max > LL_ADV_CONN_INTERVAL_MAX))
  {
    return EINVAL;
  }
  else
  {
    outParam->primIntMax = inParam->interval_max;
  }

  // Set the event properties to Non-connectable Non-scannable
  outParam->eventProps = 0;
#ifndef ZEPHYR_ADV_EXT
  // Set the event properties to Legacy Advertise
  outParam->eventProps |= GAP_ADV_PROP_LEGACY;
#endif

  // Set Event Properties
  if (inParam->options != 0)
  {
    if(inParam->options & BT_LE_ADV_OPT_CONNECTABLE)
    {

      outParam->eventProps |= GAP_ADV_PROP_CONNECTABLE;

#ifndef ZEPHYR_ADV_EXT
      // If the adv is connectable is has to be scannable
      outParam->eventProps |= GAP_ADV_PROP_SCANNABLE;
#endif
    }
  }

  return SUCCESS;
}

bStatus_t mesh_portingLayer_convertZephScanParam(const struct bt_le_scan_param *inParam,
                                                              uint8_t          *type,
                                                              uint8_t          *filter_dup,
                                                              uint16_t          *interval,
                                                              uint16_t          *window)
{
  if (inParam == NULL)
  {
    return EINVAL;
  }

  // Check validity of scan type
  if ((inParam->type != BT_LE_SCAN_TYPE_PASSIVE) &&
      (inParam->type != BT_LE_SCAN_TYPE_ACTIVE))
  {
    return EINVAL;
  }
  else
  {
    (*type) = inParam->type;
  }

  // Set scan duplicate filter
  if((inParam->options & BT_LE_SCAN_OPT_FILTER_DUPLICATE))
  {
    (*filter_dup) = BT_HCI_LE_SCAN_FILTER_DUP_ENABLE;
  }
  else
  {
    (*filter_dup) = BT_HCI_LE_SCAN_FILTER_DUP_DISABLE;
  }

  // Check validity of scan interval
  if ((inParam->interval < LL_SCAN_INTERVAL_MIN) ||
      (inParam->interval > LL_SCAN_INTERVAL_MAX))
  {
    return EINVAL;
  }
  else
  {
    (*interval) = inParam->interval;
  }

  // Check validity of scan window
  if ((inParam->window < LL_SCAN_WINDOW_MIN) ||
      (inParam->window > LL_SCAN_WINDOW_MAX))
  {
    return EINVAL;
  }
  else
  {
    (*window) = inParam->window;
  }

  return SUCCESS;
}

#ifdef CONFIG_BT_MESH_PROXY
bStatus_t mesh_portingLayer_convertZephGattAtt(struct bt_gatt_attr    *tmpInAtt,
                                                    gattAttribute_t   *tmpOutAtt)
{
  uint16_t        tmpHandle      = 0;
  uint8_t        *tmpUuidValAddr = 0;
  uint8_t         tmpUuidLength  = 0;
  uint8_t         tmpPermissions = 0;

  // Copy attribute UUID Length and value
  switch(tmpInAtt->uuid->type)
  {
    case(BT_UUID_TYPE_16):
    {
      tmpUuidLength  = ATT_BT_UUID_SIZE;
      tmpUuidValAddr = (uint8_t *)&(((struct bt_uuid_16 *)(tmpInAtt->uuid))->val);
      break;
    }
    case(BT_UUID_TYPE_128):
    {
      tmpUuidLength = ATT_UUID_SIZE;
      tmpUuidValAddr = (uint8_t *)&(((struct bt_uuid_128 *)(tmpInAtt->uuid))->val);
      break;
    }
  }

  // Copy attribute handle
  tmpHandle = tmpInAtt->handle;

  // Copy attribute permissions bitmap
  if (tmpInAtt->perm & BT_GATT_PERM_READ)
  {
    tmpPermissions |= GATT_PERMIT_READ;
  }
  if (tmpInAtt->perm & BT_GATT_PERM_WRITE)
  {
    tmpPermissions |= GATT_PERMIT_WRITE;
  }
  if (tmpInAtt->perm & BT_GATT_PERM_READ_ENCRYPT)
  {
    tmpPermissions |= GATT_PERMIT_ENCRYPT_READ;
  }
  if (tmpInAtt->perm & BT_GATT_PERM_WRITE_ENCRYPT)
  {
    tmpPermissions |= GATT_PERMIT_ENCRYPT_WRITE;
  }
  if (tmpInAtt->perm & BT_GATT_PERM_READ_AUTHEN)
  {
    tmpPermissions |= GATT_PERMIT_AUTHEN_READ;
  }
  if (tmpInAtt->perm & BT_GATT_PERM_WRITE_AUTHEN)
  {
    tmpPermissions |= GATT_PERMIT_AUTHEN_WRITE;
  }

  tmpOutAtt->type.len    = tmpUuidLength;
  tmpOutAtt->type.uuid   = tmpUuidValAddr;
  tmpOutAtt->permissions = tmpPermissions;
  tmpOutAtt->handle      = tmpHandle;
  // Note: tmpOutAtt->pValue is non generic and so is not handled in
  //       this function!

  return SUCCESS;
}

bStatus_t mesh_portingLayer_convertZephGattService(
                                            struct bt_gatt_service *inService,
                                            gattService_t   *outService)
{
  // Validate Params
  if((!inService) || (!(inService->attr_count)) || (!(inService->attrs)) ||
     (!outService))
  {
    return EINVAL;
  }

  // Fill encryption key size
  outService->encKeySize = GATT_MAX_ENCRYPT_KEY_SIZE;

  // Copy attribute count
  outService->numAttrs = (uint16_t)(inService->attr_count);

  // If service is provisioning service, attributes are known
  if (((struct bt_uuid_16 *)(inService->attrs[0].user_data))->val ==
                                                   GATT_MESH_PROV_SERVICE_UUID)
  {
    // Allocate provisioning client character configuration
    pProvClientCharConfig =
      (gattCharCfg_t *)ICall_malloc(sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS);
    if ( pProvClientCharConfig == NULL )
    {
     return ( bleMemAllocError );
    }

    // Initialize Provisioning Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID, pProvClientCharConfig );

    // Set provisioning attribute table
    outService->attrs = provAttrTbl;

    // Save pointer to service
    pGattProvService = outService;
  }
  // Proxy service
  else if (((struct bt_uuid_16 *)(inService->attrs[0].user_data))->val ==
                                                  GATT_MESH_PROXY_SERVICE_UUID)
  {
    // Allocate proxy client character configuration
    pProxyClientCharConfig =
      (gattCharCfg_t *)ICall_malloc(sizeof(gattCharCfg_t) * MAX_NUM_BLE_CONNS);

    if ( pProxyClientCharConfig == NULL )
    {
     return ( bleMemAllocError );
    }

    // Initialize Proxy Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg(LINKDB_CONNHANDLE_INVALID, pProxyClientCharConfig);

    // Set proxy attribute table
    outService->attrs = proxyAttrTbl;

    // Save pointer to service
    pGattProxyService = outService;
  }
  else // Generic attributes
  {
    // Allocate space for service attributes
    outService->attrs =
        ICall_malloc(sizeof(gattAttribute_t)*(outService->numAttrs));

    // Copy each attribute
    for (int i = 0; i < outService->numAttrs; i++)
    {
      // Create tmp attribute pointer for readability
      struct bt_gatt_attr    *tmpInAtt  = &(inService->attrs[i]);
      gattAttribute_t        tmpOutAtt =
      {.pValue =  (uint8_t *)(tmpInAtt->user_data) };

      mesh_portingLayer_convertZephGattAtt(tmpInAtt, &tmpOutAtt);

      // Copy attribute data into newly allocated space
      memcpy(&(outService->attrs[i]), &tmpOutAtt, sizeof(gattAttribute_t));
    }
  }

  return SUCCESS;
}


bStatus_t mesh_portingLayer_convertNewConnParams(
                                      gapEstLinkReqEvent_t *inputNewConnection,
                                      struct bt_conn *outputNewConnection)
{
  if (NULL == outputNewConnection)
  {
    // invalid param
    return EINVAL;
  }

  // Copy connection handle
  outputNewConnection->handle = inputNewConnection->connectionHandle;

  if (inputNewConnection->connRole == GAP_PROFILE_PERIPHERAL)
  {
    outputNewConnection->role = BT_HCI_ROLE_SLAVE;
  }
  else if (inputNewConnection->connRole == GAP_PROFILE_CENTRAL)
  {
    outputNewConnection->role = BT_HCI_ROLE_MASTER;
  }
  else
  {
    // invalid role
    return EINVAL;
  }

  outputNewConnection->state = BT_CONN_CONNECTED;

  // event params
  outputNewConnection->err = inputNewConnection->hdr.status;


  outputNewConnection->le.interval = inputNewConnection->connInterval;
  outputNewConnection->le.latency = inputNewConnection->connLatency;
  outputNewConnection->le.timeout = inputNewConnection->connTimeout;

  memcpy(outputNewConnection->le.dst.a.val,inputNewConnection->devAddr, 6);

  if ((inputNewConnection->devAddrType == ADDRTYPE_RANDOM_NR) ||
      (inputNewConnection->devAddrType == ADDRTYPE_NONE))
  {
    return EINVAL;
  }
  else
  {
    outputNewConnection->le.dst.type = inputNewConnection->devAddrType;
  }

  outputNewConnection->ref = 0;
  outputNewConnection->type = BT_CONN_TYPE_LE;

  return SUCCESS;
}

bStatus_t mesh_portingLayer_convertTIGattAtt(gattAttribute_t *tmpInAtt,
                                             struct bt_gatt_attr *tmpOutAtt)
{
  uint16_t tmpHandle      = 0;
  uint8_t  tmpPermissions = 0;
  uint16_t tmpUuid;

  // Copy attribute handle
  tmpHandle = tmpInAtt->handle;

  // Copy attribute permissions bitmap
  if (tmpInAtt->permissions  & GATT_PERMIT_READ)
  {
    tmpPermissions |= BT_GATT_PERM_READ;
  }
  if (tmpInAtt->permissions  & GATT_PERMIT_WRITE)
  {
    tmpPermissions |= BT_GATT_PERM_WRITE;
  }
  if (tmpInAtt->permissions  & GATT_PERMIT_ENCRYPT_READ)
  {
    tmpPermissions |= BT_GATT_PERM_READ_ENCRYPT;
  }
  if (tmpInAtt->permissions  & GATT_PERMIT_ENCRYPT_WRITE)
  {
    tmpPermissions |= BT_GATT_PERM_WRITE_ENCRYPT;
  }
  if (tmpInAtt->permissions  & GATT_PERMIT_AUTHEN_READ)
  {
    tmpPermissions |= BT_GATT_PERM_READ_AUTHEN;
  }
  if (tmpInAtt->permissions  & GATT_PERMIT_AUTHEN_WRITE)
  {
    tmpPermissions |= BT_GATT_PERM_WRITE_AUTHEN;
  }

  // Copy attribute UUID Length and value
   switch(tmpInAtt->type.len)
   {
     case(ATT_BT_UUID_SIZE):
     {
       ((struct bt_uuid_16 *)(tmpOutAtt->uuid))->uuid.type = BT_UUID_TYPE_16;
       break;
     }
     case(ATT_UUID_SIZE):
     {
       ((struct bt_uuid_128 *)(tmpOutAtt->uuid))->uuid.type = BT_UUID_TYPE_128;
       break;
     }
     default:
     {
       return FAILURE;
     }
   }

  tmpOutAtt->perm      = tmpPermissions;
  tmpOutAtt->handle    = tmpHandle;

  tmpUuid = BUILD_UINT16( tmpInAtt->type.uuid[0], tmpInAtt->type.uuid[1]);

  if (GATT_MESH_PROXY_DATA_IN_CHAR_UUID == tmpUuid)
  {
    // This is how the zephyr mesh profile distinguishes between provisioning
    // and proxy PDUs
    tmpOutAtt->user_data = NULL;
  }
  else
  {
    tmpOutAtt->user_data = tmpInAtt->pValue;
  }

  return SUCCESS;

}

/******************************************************************************
 * GET/SET FUNCTIONS
 */

gattService_t *getProvService(void)
{
  return pGattProvService;
}

void setProvService(gattService_t *pNewService)
{
  pGattProvService = pNewService;
}

gattCharCfg_t *getProvClientCharConfig(void)
{
  return pProvClientCharConfig;
}

gattService_t *getProxyService(void)
{
  return pGattProxyService;
}

void setProxyService(gattService_t *pNewService)
{
  pGattProxyService = pNewService;
}

gattCharCfg_t *getProxyClientCharConfig(void)
{
  return pProxyClientCharConfig;
}
#endif
