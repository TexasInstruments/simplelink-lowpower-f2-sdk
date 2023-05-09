/******************************************************************************

 @file  iot_ble_hal_gatt_server.c

 @brief Hardware Abstraction Layer for GATT server ble stack.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

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

/**
 * @file iot_ble_hal_gatt_server.c
 * @brief Hardware Abstraction Layer for GATT server ble stack.
 */

/* Standard includes. */
#include <string.h>
#include <TI_heap_wrapper.h>
/* FreeRTOS includes */
#include <FreeRTOS.h>
/* BLE HAL includes. */
#include "bt_hal_manager_adapter_ble.h"
#include "bt_hal_manager.h"
#include "bt_hal_gatt_server.h"
#include "iot_ble_hal_internals.h"

#include "ti_ble_config.h"

/* BLE-Stack API definitions: Must be the last file in list of stack includes */
#include <icall_ble_api.h>

/*-----------------------------------------------------------*/

typedef struct {
    gattCharCfg_t *pxTable;                   /* Pointer to the char config table */
    gattCharCfg_t xTable[MAX_NUM_BLE_CONNS];  /* char config table, one record per connection */
}CccdTable_t;

typedef struct servicesList_t
{
    gattAttribute_t *pService;
    uint8_t numAttributes;
    struct servicesList_t *next;
} servicesList_t;

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTRegisterServer( BTUuid_t * pxUuid );
static BTStatus_t _prvBTUnregisterServer( uint8_t ucServerIf );
static BTStatus_t _prvBTGattServerInit( const BTGattServerCallbacks_t * pxCallbacks );
static BTStatus_t _prvBTConnect( uint8_t ucServerIf,
                                 const BTBdaddr_t * pxBdAddr,
                                 bool bIsDirect,
                                 BTTransport_t xTransport );
static BTStatus_t _prvBTDisconnect( uint8_t ucServerIf,
                                    const BTBdaddr_t * pxBdAddr,
                                    uint16_t usConnId );
static BTStatus_t _prvBTAddService( uint8_t ucServerIf,
                                    BTGattSrvcId_t * pxSrvcId,
                                    uint16_t usNumHandles );
static BTStatus_t _prvBTAddIncludedService( uint8_t ucServerIf,
                                            uint16_t usServiceHandle,
                                            uint16_t usIncludedHandle );
static BTStatus_t _prvBTAddCharacteristic( uint8_t ucServerIf,
                                           uint16_t usServiceHandle,
                                           BTUuid_t * pxUuid,
                                           BTCharProperties_t xProperties,
                                           BTCharPermissions_t xPermissions );
static BTStatus_t _prvBTSetVal( BTGattResponse_t * pxValue );
static BTStatus_t _prvBTAddDescriptor( uint8_t ucServerIf,
                                       uint16_t usServiceHandle,
                                       BTUuid_t * pxUuid,
                                       BTCharPermissions_t xPermissions );
static BTStatus_t _prvBTStartService( uint8_t ucServerIf,
                                      uint16_t usServiceHandle,
                                      BTTransport_t xTransport );
static BTStatus_t _prvBTStopService( uint8_t ucServerIf,
                                     uint16_t usServiceHandle );
static BTStatus_t _prvBTDeleteService( uint8_t ucServerIf,
                                       uint16_t usServiceHandle );
static BTStatus_t _prvBTSendIndication( uint8_t ucServerIf,
                                        uint16_t usAttributeHandle,
                                        uint16_t usConnId,
                                        size_t xLen,
                                        uint8_t * pucValue,
                                        bool bConfirm );
static BTStatus_t _prvBTSendResponse( uint16_t usConnId,
                                      uint32_t ulTransId,
                                      BTStatus_t xStatus,
                                      BTGattResponse_t * pxResponse );
static BTStatus_t _prvAddServiceBlob( uint8_t ucServerIf,
                                      BTService_t * pxService );
static uint8_t _prvGetAfrUuidLen( BTuuidType_t ucType );
static uint8_t _prvConvertAfrPermissions( BTCharPermissions_t permissions );
static size_t _prvConvertAfrNumAttributes ( BTService_t * pxService );
static bStatus_t _prvReadAttrCB( uint16_t connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8_t *pValue,
                                 uint16_t *pLen,
                                 uint16_t offset,
                                 uint16_t maxLen,
                                 uint8_t method );
static bStatus_t _prvWriteAttrCB( uint16_t connHandle,
                                  gattAttribute_t *pAttr,
                                  uint8_t *pValue,
                                  uint16_t len,
                                  uint16_t offset,
                                  uint8_t method );
static bool _prvIsCCCD( BTCharacteristicDescr_t * descriptor );
static BTStatus_t _prvAddToServiceList( gattAttribute_t *newAtt,
                                        uint16_t numAtts );

/*-----------------------------------------------------------*/

static servicesList_t *pServList = NULL;

static BTGattServerInterface_t xGATTserverInterface =
{
    .pxRegisterServer     = _prvBTRegisterServer,
    .pxUnregisterServer   = _prvBTUnregisterServer,
    .pxGattServerInit     = _prvBTGattServerInit,
    .pxConnect            = _prvBTConnect,
    .pxDisconnect         = _prvBTDisconnect,
    .pxAddServiceBlob     = _prvAddServiceBlob,
    .pxAddService         = _prvBTAddService,
    .pxAddIncludedService = _prvBTAddIncludedService,
    .pxAddCharacteristic  = _prvBTAddCharacteristic,
    .pxSetVal             = _prvBTSetVal,
    .pxAddDescriptor      = _prvBTAddDescriptor,
    .pxStartService       = _prvBTStartService,
    .pxStopService        = _prvBTStopService,
    .pxDeleteService      = _prvBTDeleteService,
    .pxSendIndication     = _prvBTSendIndication,
    .pxSendResponse       = _prvBTSendResponse,
};

/*-----------------------------------------------------------*/

/* Borrow the connected address from the MQ task so as to not duplicate memory
 * A more robust solution is to make a proper link database where addresses
 * can be looked up by connection handles, or upgrade the stack's linkDB to
 * support connHandle to address lookup.
 *
 * However, since the BTAddr is returned often through stack callbacks the
 * function that maps addresses should not go through icall.
 */
extern BTBdaddr_t xPeerBTAddr;

/*-----------------------------------------------------------*/

BTGattServerCallbacks_t _xGattServerCallbacks;
uint32_t ulGattServerIFhandle = 0;
static bool bGattInitialized = false;

gattServiceCBs_t xServiceCBs =
{
  _prvReadAttrCB,  /* Read callback function pointer */
  _prvWriteAttrCB, /* Write callback function pointer */
  NULL
};

/*-----------------------------------------------------------*/

static bStatus_t _prvReadAttrCB( uint16_t connHandle,
                                 gattAttribute_t *pAttr,
                                 uint8_t *pValue,
                                 uint16_t *pLen,
                                 uint16_t offset,
                                 uint16_t maxLen,
                                 uint8_t method )
{
    bStatus_t xStatus = ATT_ERR_INSUFFICIENT_RESOURCES;

    /* Only the following ATT read procedures support delayed response
     * Return blePending for now and then the values are set by
     * _prvBTSendResponse
     */
    if( ( method == ATT_READ_REQ ) ||
        ( method == ATT_READ_BLOB_REQ ) ||
        ( method == ATT_READ_BY_TYPE_REQ ) )
    {
        xStatus = blePending;
    }

    /* Directly issue the application callback
     * This was tested and appeared to be fine, but since this is in the
     * context of the stack, processing should be kept minimal
     * Thankfully GATTServApp executes these callbacks in the \
     * stack task context and not HWI
     */
    if( _xGattServerCallbacks.pxRequestReadCb != NULL )
    {
        _xGattServerCallbacks.pxRequestReadCb( connHandle,
                                               (uint32_t)method,
                                               &xPeerBTAddr,
                                               pAttr->handle,
                                               offset );
    }

    return (xStatus);
}

/*-----------------------------------------------------------*/

static bStatus_t _prvWriteAttrCB( uint16_t connHandle,
                                  gattAttribute_t *pAttr,
                                  uint8_t *pValue,
                                  uint16_t len,
                                  uint16_t offset,
                                  uint8_t method )
{

    bStatus_t xStatus = SUCCESS;
    bool bNeedRsp = method != ATT_WRITE_CMD;
    bool bIsPrep = method == ATT_PREPARE_WRITE_REQ;

    if ( pAttr->type.len == ATT_BT_UUID_SIZE )
    {
        /* 16-bit UUID */
        uint16_t usUuid = BUILD_UINT16( pAttr->type.uuid[ 0 ],
                                        pAttr->type.uuid[ 1 ] );

        if( usUuid == GATT_CLIENT_CHAR_CFG_UUID )
        {
            /* GATTServApp manages CCCD operations directly, nothing to
             * do at the application level
             */
            xStatus = GATTServApp_ProcessCCCWriteReq( connHandle,
                                                      pAttr,
                                                      pValue,
                                                      len,
                                                      offset,
                                                      (GATT_CLIENT_CFG_NOTIFY | GATT_CLIENT_CFG_INDICATE) );
        }
    }

    if( _xGattServerCallbacks.pxRequestWriteCb != NULL )
    {
        _xGattServerCallbacks.pxRequestWriteCb( connHandle,
                                                (uint32_t)method,
                                                &xPeerBTAddr,
                                                pAttr->handle,
                                                offset,
                                                len,
                                                bNeedRsp,
                                                bIsPrep,
                                                pValue );
    }

    return ( xStatus );
}

/*-----------------------------------------------------------*/

/**
 * @brief      Determine if an AFR descriptor is a CCCD
 *
 * @param      descriptor  descriptor to test
 *
 * @return     true if is CCCD
 *             false otherwise
 */
bool _prvIsCCCD( BTCharacteristicDescr_t * descriptor )
{
    bool xStatus = false;

    /*
     */
    if( ( descriptor->xUuid.ucType == eBTuuidType16 ) &&
        ( descriptor->xUuid.uu.uu16 == BUILD_UINT16( clientCharCfgUUID[0],
                                                     clientCharCfgUUID[1] ) ) )
    {
        xStatus = true;
    }

    return xStatus;
}

/*-----------------------------------------------------------*/

/**
 * @brief      Determine the number of required TI attributes based on the
 *             AFR service structure. TI has separate entries for characteristic
 *             declaration and characteristic value
 *
 * @param      pxService  AFR services
 *
 * @return     Number of attributes in the TI service
 */
static size_t _prvConvertAfrNumAttributes ( BTService_t * pxService )
{
    uint16_t usAfrIndex = 0;
    size_t xNumTiAttributes = pxService->xNumberOfAttributes;
    for( usAfrIndex = 0; usAfrIndex < pxService->xNumberOfAttributes; usAfrIndex++ )
    {
        if( pxService->pxBLEAttributes[ usAfrIndex ].xAttributeType == eBTDbCharacteristic )
        {
            xNumTiAttributes++;
        }
    }

    return ( xNumTiAttributes );
}

/*-----------------------------------------------------------*/

/**
 * @brief      Convert AFR UUID types to TI lengths
 *
 * @param[in]  ucType  UUID type
 *
 * @return     Length of UUID in bytes
 */
static uint8_t _prvGetAfrUuidLen( BTuuidType_t ucType )
{
    uint8_t usTiUuidSize = 0xFF;

    switch ( ucType )
    {
        case eBTuuidType16:
        {
            usTiUuidSize = ATT_BT_UUID_SIZE;
            break;
        }
        case eBTuuidType32:
        {
            usTiUuidSize = ATT_32_BIT_UUID_SIZE;
            break;
        }
        case eBTuuidType128:
        {
            usTiUuidSize = ATT_UUID_SIZE;
            break;
        }
        default:
            break;
    }

    return ( usTiUuidSize );
}

/*-----------------------------------------------------------*/

/**
 * @brief      Convert AFR attribute permissions to TI attribute permissions
 *
 * @param[in]  permissions  AFR characteristic permissions
 *
 * @return     TI permission bit mask
 */
static uint8_t _prvConvertAfrPermissions( BTCharPermissions_t permissions )
{
    uint8_t ucTiPermission = 0x00;

    /* First handle the read */
    switch ( permissions & 0x0007 )
    {
        case eBTPermRead:
        {
            ucTiPermission |= GATT_PERMIT_READ;
            break;
        }
        case eBTPermReadEncrypted:
        {
            ucTiPermission |= GATT_PERMIT_ENCRYPT_READ;
            break;
        }
        case eBTPermReadEncryptedMitm:
        {
            ucTiPermission |= GATT_PERMIT_AUTHEN_READ;
            break;
        }
        default:
            break;
    }

    /* THen handle write,
     * use bitwise OR in case a characteristic is read and write
     */
    switch ( permissions & 0x01F0 )
    {
        case eBTPermWrite:
        {
            ucTiPermission |= GATT_PERMIT_WRITE;
            break;
        }
        case eBTPermWriteEncrypted:
        {
            ucTiPermission |= GATT_PERMIT_ENCRYPT_WRITE;
            break;
        }
        case eBTPermWriteEncryptedMitm:
        {
            ucTiPermission |= GATT_PERMIT_AUTHEN_WRITE;
            break;
        }
        default:
            break;
    }

    return ( ucTiPermission );
}

/*-----------------------------------------------------------*/

/**
 * @brief      Convert AFR service declaration to TI GATT service entry
 *
 * @param           xTiService      Pointer to TI attribute array
 * @param[in, out]  usTiIndex       Index within the TI attribute array
 * @param[in, out]  usCurrentHandle Current attribute handle of the TI GATT
 *                                  server
 * @param           pxAfrService    Pointer to the AFR service definition
 * @param[in]       usAfrIndex      Index within the AFR attribute array
 *
 * @return          eBTStatusSuccess - if service was successfully created
 *                  eBTStatusNoMem - if there was not enough heap memory
 *                                   to allocate the UUID
 */
static BTStatus_t _prvCreateServiceEntry( gattAttribute_t * xTiService,
                                          uint16_t *usTiIndex,
                                          uint16_t *usCurrentHandle,
                                          BTService_t * pxAfrService,
                                          uint16_t usAfrIndex )
{
    /* In a primary service declaration, the pValue field of the
     * attribute holds the service UUID. This is allocated from
     * the heap, and stored in this temporary variable.
     * which is then linked to the attr table through the temp
     * variable below
     */
    uint8_t* pucUuidAttr;

    /* This is a double pointer to the location in memory that holds
     * the attribute value. Since the attribute value field is
     * 'const uint8_t *' this requires us to modify it by another
     * layer of indirection
     */
    uint8_t **pucAttrVal;

    /* The UUID of a primary service declaration is SIG defined
     * as are its properties. Fill these out.
     * Use GATT_GetNextHandle to hint at the handles
     */
    xTiService[ *usTiIndex ].type.len = ATT_BT_UUID_SIZE;
    xTiService[ *usTiIndex ].type.uuid = primaryServiceUUID;
    if( pxAfrService->pxBLEAttributes[ usAfrIndex ].xAttributeType  == eBTDbSecondaryService)
    {
        xTiService[ *usTiIndex ].type.uuid = secondaryServiceUUID;
    }
    xTiService[ *usTiIndex ].permissions = GATT_PERMIT_READ;
    xTiService[ *usTiIndex ].handle = 0;
    /* Fill in handle in the buffer of handles, be wary of operator precedence */
    pxAfrService->pusHandlesBuffer[ usAfrIndex ] = (*usCurrentHandle)++;

    /* Allocate memory to store service's UUID */
    pucUuidAttr = pvPortMalloc( sizeof( gattAttrType_t ) );
    if ( pucUuidAttr == NULL)
    {
        return ( eBTStatusNoMem );
    }

    /* Fill in the service UUID and length */
    gattAttrType_t * xSvcUuid = ( gattAttrType_t * )pucUuidAttr;
    xSvcUuid->len = _prvGetAfrUuidLen( pxAfrService->pxBLEAttributes[ usAfrIndex ].xServiceUUID.ucType );

    /* Regardless of the UUID type we want to use uu128 here because
     * the TI GATT is expecting an array of uint8.
     * The size will be correctly populated by _prvGetAfrUuidLen
     */
    xSvcUuid->uuid = pxAfrService->pxBLEAttributes[ usAfrIndex ].xServiceUUID.uu.uu128;

    /* Populate the pointer value in the table using temp variables
     * this is to work around the const pointer in the TI
     * definition of the table
     */
    pucAttrVal = ( uint8_t ** )&xTiService[ *usTiIndex ].pValue;
    *pucAttrVal = pucUuidAttr;

    /* Service declaration is only one characteristic in TI
     * table, increment counter
     * Warn: operator precedence matters here!
     *       we wish to increment the _value_ of usTiIndex
     */
    (*usTiIndex)++;

    return (eBTStatusSuccess);
}

/*-----------------------------------------------------------*/

/**
 * @brief      Convert AFR include service declaration to TI GATT
 *
 * @param           xTiService      Pointer to TI attribute array
 * @param[in, out]  usTiIndex       Index within the TI attribute array
 * @param[in, out]  usCurrentHandle Current attribute handle of the TI GATT
 *                                  server
 * @param           pxAfrService    Pointer to the AFR service definition
 * @param[in]       usAfrIndex      Index within the AFR attribute array
 *
 * @return          eBTStatusSuccess - if service was successfully created
 *                  eBTStatusNoMem - if there was not enough heap memory
 *                                   to allocate the UUID
 */
static BTStatus_t _prvCreateIncludeServiceEntry( gattAttribute_t * xTiService,
                                                 uint16_t *usTiIndex,
                                                 uint16_t *usCurrentHandle,
                                                 BTService_t * pxAfrService,
                                                 uint16_t usAfrIndex )
{
    /* In an included service declaration, the pValue field of the
     * attribute holds the handle of the included service.
     * This is allocated from the heap as it must persist
     */
    uint8_t* pusIncludeHdl;

    /* This is a double pointer to the location in memory that holds
     * the attribute value. Since the attribute value field is
     * 'const uint8_t *' this requires us to modify it by another
     * layer of indirection
     */
    uint8_t **pucAttrVal;

    /* The UUID of a primary service declaration is SIG defined
     * as are its properties. Fill these out.
     * Use GATT_GetNextHandle to hint at the handles
     */
    xTiService[ *usTiIndex ].type.len = ATT_BT_UUID_SIZE;
    xTiService[ *usTiIndex ].type.uuid = includeUUID;
    xTiService[ *usTiIndex ].permissions = GATT_PERMIT_READ;
    xTiService[ *usTiIndex ].handle = 0;
    /* Fill in handle in the buffer of handles */
    pxAfrService->pusHandlesBuffer[ usAfrIndex ] = (*usCurrentHandle)++;

    /* Allocate memory to store service's UUID */
    pusIncludeHdl = pvPortMalloc( sizeof( uint16_t ) );
    if ( pusIncludeHdl == NULL)
    {
        return ( eBTStatusNoMem );
    }

    /* Get the pointer to the service being included */
    BTService_t * pxIncService = pxAfrService->pxBLEAttributes[ usAfrIndex ].xIncludedService.pxPtrToService;

    /* Get the handle of the included service. A service handle
     * is just the handle of its first characteristic
     */
    *pusIncludeHdl = pxIncService->pusHandlesBuffer[0];

    /* Populate the pointer value in the table using temp variables
     * this is to work around the const pointer in the TI
     * definition of the table
     */
    pucAttrVal = ( uint8_t ** )&xTiService[ *usTiIndex ].pValue;
    *pucAttrVal = pusIncludeHdl;

    /* Include Service declaration is only one characteristic in TI
     * table, increment counter
     */
    (*usTiIndex)++;

    return ( eBTStatusSuccess );
}

/*-----------------------------------------------------------*/

/**
 * @brief      Convert AFR characteristic definition to TI GATT
 *
 * @param           xTiService      Pointer to TI attribute array
 * @param[in, out]  usTiIndex       Index within the TI attribute array
 * @param[in, out]  usCurrentHandle Current attribute handle of the TI GATT
 *                                  server
 * @param           pxAfrService    Pointer to the AFR service definition
 * @param[in]       usAfrIndex      Index within the AFR attribute array
 *
 * @return          eBTStatusSuccess - if service was successfully created
 *                  eBTStatusNoMem - if there was not enough heap memory
 *                                   to allocate the UUID
 */
static BTStatus_t _prvCreateCharEntry( gattAttribute_t * xTiService,
                                       uint16_t *usTiIndex,
                                       uint16_t *usCurrentHandle,
                                       BTService_t * pxAfrService,
                                       uint16_t usAfrIndex )
{
    /* The UUID of a char declaration is SIG defined
     * as are its properties. Fill these out.
     */
    uint8_t **pucAttrVal;
    xTiService[ *usTiIndex ].type.len = ATT_BT_UUID_SIZE;
    xTiService[ *usTiIndex ].type.uuid = characterUUID;
    xTiService[ *usTiIndex ].permissions = GATT_PERMIT_READ;
    xTiService[ *usTiIndex ].handle = 0;
    pucAttrVal = ( uint8_t ** )&xTiService[ *usTiIndex ].pValue;
    *pucAttrVal = ( uint8_t * )&( pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristic.xProperties );

    /* Fill in handle in the buffer of handles */
    pxAfrService->pusHandlesBuffer[ usAfrIndex ] = (*usCurrentHandle)++;

    /* Characteristic declaration takes two entries in TI table
     * One for the characteristic declaration and one for the value
     */
    (*usTiIndex)++;

    xTiService[ *usTiIndex ].type.len = _prvGetAfrUuidLen( pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristic.xUuid.ucType );
    xTiService[ *usTiIndex ].type.uuid = pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristic.xUuid.uu.uu128;
    xTiService[ *usTiIndex ].permissions = _prvConvertAfrPermissions( pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristic.xPermissions );
    xTiService[ *usTiIndex ].handle = 0;

    /* Fill in handle in the buffer of handles */
    pxAfrService->pusHandlesBuffer[ usAfrIndex ] = (*usCurrentHandle)++;

    (*usTiIndex)++;

    return (eBTStatusSuccess);
}

/*-----------------------------------------------------------*/

/**
 * @brief      Convert AFR descriptor definition to TI GATT
 *
 * @param           xTiService      Pointer to TI attribute array
 * @param[in, out]  usTiIndex       Index within the TI attribute array
 * @param[in, out]  usCurrentHandle Current attribute handle of the TI GATT
 *                                  server
 * @param           pxAfrService    Pointer to the AFR service definition
 * @param[in]       usAfrIndex      Index within the AFR attribute array
 *
 * @return          eBTStatusSuccess - if service was successfully created
 *                  eBTStatusNoMem - if there was not enough heap memory
 *                                   to allocate the UUID
 */
static BTStatus_t _prvCreateDescriptorEntry( gattAttribute_t * xTiService,
                                             uint16_t *usTiIndex,
                                             uint16_t *usCurrentHandle,
                                             BTService_t * pxAfrService,
                                             uint16_t usAfrIndex )
{
    /* The pValue field of the
    * attribute holds the descriptor.
    * This is allocated from the heap as it must persist
    */
    CccdTable_t  *pxCCCDTable = NULL;

    /* The UUID of a char descriptor is SIG defined
     * as are its properties. Fill these out.
     */
    uint8_t **pucAttrVal;
    xTiService[ *usTiIndex ].type.len = _prvGetAfrUuidLen( pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristicDescr.xUuid.ucType );
    xTiService[ *usTiIndex ].type.uuid = (uint8_t *)&( pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristicDescr.xUuid.uu.uu128 );
    xTiService[ *usTiIndex ].permissions = _prvConvertAfrPermissions( pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristicDescr.xPermissions );
    xTiService[ *usTiIndex ].handle = 0;
    pucAttrVal = ( uint8_t ** )&xTiService[ *usTiIndex ].pValue;

    /* Allocate CCCD instance for each possible connection */
    if ( _prvIsCCCD( &(pxAfrService->pxBLEAttributes[ usAfrIndex ].xCharacteristicDescr) ) )
    {
        pxCCCDTable = (CccdTable_t  *)pvPortMalloc( sizeof(CccdTable_t) );
        if ( pxCCCDTable == NULL)
        {
            return ( eBTStatusNoMem );
        }
        *pucAttrVal = (uint8_t *)pxCCCDTable;

        /* This is a bit complicated, but the stack requires it.
         * The pValue member of gattAttribute_t for CCCDs points to what's
         * referred to as a CCCD table.
         *
         * The member in this CCCD table is a pointer to the actual CCCD array
         * We need multiple CCCD instances in the case of multiple connections
         *
         * In order to prevent heap fragmentation, we allocate both the table
         * and pointer to table in contiguous memory.
         *
         * This means in practice that the CCCD table pointer will point to
         * memory that directly proceeds it.
         */
        pxCCCDTable->pxTable = ( gattCharCfg_t * )&(pxCCCDTable->xTable);

        // Initialize Client Characteristic Configuration attributes
        GATTServApp_InitCharCfg( LINKDB_CONNHANDLE_INVALID,
                                 pxCCCDTable->pxTable );
    }

    /* Fill in handle in the buffer of handles */
    pxAfrService->pusHandlesBuffer[ usAfrIndex ] = (*usCurrentHandle)++;

    /* increment TI table counter
     */
    (*usTiIndex)++;

    return (eBTStatusSuccess);
}

/*-----------------------------------------------------------*/

/**
 * @brief      Send notification or indication
 *             This function is similar to and based on
 *             GATTServApp_ProcessCharCfg
 *             However, it will only allocate exactly how much memory is
 *             requested for the characteristic and will perform a deep copy
 *             of the variable from the input buffer to output without needing
 *             a local GATT read callback.
 *
 *             This function was written to avoid the limitation where
 *             the pValue is not set at the time of table creation
 *
 * @param[in]  usConnHandle    connection handle
 * @param      pxCharCfgTbl    Character config (CCCD) table allocated at service
 *                             creation time
 * @param      pucValue        Pointer to buffer of data to send in noti/ind
 *                             payload
 * @param[in]  usLen           Payload length in bytes
 * @param[in]  bAuthenticated  Is authentication required
 * @param      pxAttrTbl       Pointer to the attribute table that the CCCD
 *                             table belongs to
 * @param[in]  usNumAttrs      Number of attributes in pxAttrTbl
 * @param[in]  ucTaskId        ICall task entity where to send handle value
 *                             confirmations (for indications)
 *
 * @return     SUCCESS  - if notification/indication sent successfully
 *             bleNoResources - If allocation failed or pxCharCfgTbl is
 *                              configured incorrectly
 */
static bStatus_t _prvSendNotiInd( uint16_t usConnHandle,
                                  gattCharCfg_t *pxCharCfgTbl,
                                  uint8_t *pucValue,
                                  uint16_t usLen,
                                  uint8_t bAuthenticated,
                                  gattAttribute_t *pxAttrTbl,
                                  uint16_t usNumAttrs,
                                  uint8_t ucTaskId )
{
    bStatus_t status = SUCCESS;
    attHandleValueNoti_t xNoti;

    /* If the attribute value is longer than (ATT_MTU - 3) octets, then
     * only the first (ATT_MTU - 3) octets of this attributes value can
     * be sent in a notification.
     */
    xNoti.pValue = ( uint8_t * ) GATT_bm_alloc( usConnHandle,
                                                ATT_HANDLE_VALUE_NOTI,
                                                usLen,
                                                &xNoti.len );

    if( xNoti.pValue != NULL )
    {
        for ( uint8_t i = 0; i < linkDBNumConns; i++ )
        {
            gattCharCfg_t *pxItem = &(pxCharCfgTbl[i]);

            if ( ( pxItem->connHandle != LINKDB_CONNHANDLE_INVALID ) &&
                 ( pxItem->value != GATT_CFG_NO_OPERATION ) )
            {
                gattAttribute_t *pAttr;

                /* Find the characteristic value attribute */
                pAttr = GATTServApp_FindAttr( pxAttrTbl, usNumAttrs, pucValue );
                xNoti.handle = pAttr->handle;
                memcpy(xNoti.pValue, pucValue, usLen);
                if ( pAttr != NULL )
                {
                    if ( pxItem->value & GATT_CLIENT_CFG_NOTIFY )
                    {
                       status  = GATT_Notification( usConnHandle,
                                                    &xNoti,
                                                    bAuthenticated );
                    }

                    if ( pxItem->value & GATT_CLIENT_CFG_INDICATE )
                    {
                        status = GATT_Indication( usConnHandle,
                                                  (attHandleValueInd_t *)&xNoti,
                                                  bAuthenticated,
                                                  ucTaskId );
                    }

                    if ( status != SUCCESS )
                    {
                        GATT_bm_free( (gattMsg_t *)&xNoti,
                                      ATT_HANDLE_VALUE_NOTI );
                    }
                }
                else
                {
                    status = bleNoResources;
                }
            }
        }
    }
    else
    {
      status = bleNoResources;
    }

  return ( status );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTRegisterServer( BTUuid_t * pxUuid )
{
    BTStatus_t xStatus = eBTStatusSuccess;

    if( _xGattServerCallbacks.pxRegisterServerCb != NULL )
    {
        _xGattServerCallbacks.pxRegisterServerCb( eBTStatusSuccess,
                                                  ulGattServerIFhandle,
                                                  pxUuid );
    }
    return ( xStatus );
}


/*-----------------------------------------------------------*/

static BTStatus_t _prvBTUnregisterServer( uint8_t ucServerIf )
{
    BTStatus_t xStatus = eBTStatusSuccess;

    if( _xGattServerCallbacks.pxUnregisterServerCb != NULL )
    {
        _xGattServerCallbacks.pxUnregisterServerCb( eBTStatusSuccess,
                                                    ulGattServerIFhandle );
    }

    return ( xStatus );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTGattServerInit( const BTGattServerCallbacks_t * pxCallbacks )
{
    BTStatus_t xStatus = eBTStatusSuccess;

    if( bGattInitialized )
    {
        return eBTStatusSuccess;
    }

    if( pxCallbacks != NULL )
    {
        _xGattServerCallbacks = *pxCallbacks;

        /* Register to receive incoming ATT Indications/Notifications */
        GATT_RegisterForInd( _IotBleHalAsyncMq_GetEntity() );

        /* Register for GATT local events */
        GATT_RegisterForMsgs( _IotBleHalAsyncMq_GetEntity() );

        /* Create the GAP GATT service */
        GGS_AddService( GATT_ALL_SERVICES );
        /* Create the GAPServ App service */
        GATTServApp_AddService( GATT_ALL_SERVICES );
        bGattInitialized = true;
    }
    else
    {
        xStatus = eBTStatusParamInvalid;
    }
    return ( xStatus );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTConnect( uint8_t ucServerIf,
                                 const BTBdaddr_t * pxBdAddr,
                                 bool bIsDirect,
                                 BTTransport_t xTransport )
{
    return eBTStatusUnsupported;
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTDisconnect( uint8_t ucServerIf,
                                    const BTBdaddr_t * pxBdAddr,
                                    uint16_t usConnId )
{
    return eBTStatusUnsupported;
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTAddService( uint8_t ucServerIf,
                                    BTGattSrvcId_t * pxSrvcId,
                                    uint16_t usNumHandles )
{
    return ( eBTStatusUnsupported );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTAddIncludedService( uint8_t ucServerIf,
                                            uint16_t usServiceHandle,
                                            uint16_t usIncludedHandle )
{
    return ( eBTStatusUnsupported );
}

static BTStatus_t _prvAddServiceBlob( uint8_t ucServerIf,
                                      BTService_t * pxService )
{
    BTStatus_t xStatus = eBTStatusSuccess;
    bStatus_t  xStackStatus = SUCCESS;
    uint16_t usAfrIndex = 0;
    uint16_t usTiIndex = 0;
    uint16_t usNumAttributes = _prvConvertAfrNumAttributes( pxService );
    gattAttribute_t *xNewTiService = NULL;

    /* The empty parameter "" is intentionally added to work around an
     * issue with the icall_directAPI function mapping and GCC compiler
     */
    uint16_t usCurrentHandle = GATT_GetNextHandle("");

    xNewTiService = pvPortMalloc( sizeof( gattAttribute_t )*usNumAttributes );

    if( xNewTiService == NULL )
    {
        return ( eBTStatusNoMem );
    }

    /* Iterate through the Amazon table item by item, building up TI table
     * along the way
     */
    for( usAfrIndex = 0; usAfrIndex < pxService->xNumberOfAttributes; usAfrIndex++ )
    {
        switch( pxService->pxBLEAttributes[ usAfrIndex ].xAttributeType )
        {
            case eBTDbPrimaryService:
            case eBTDbSecondaryService:
            {
               xStatus = _prvCreateServiceEntry( xNewTiService,
                                                 &usTiIndex,
                                                 &usCurrentHandle,
                                                 pxService,
                                                 usAfrIndex );
                break;
            }
            case eBTDbIncludedService:
            {
                xStatus =  _prvCreateIncludeServiceEntry( xNewTiService,
                                                          &usTiIndex,
                                                          &usCurrentHandle,
                                                          pxService,
                                                          usAfrIndex );
                break;
            }

            case eBTDbCharacteristic:
            {
                xStatus = _prvCreateCharEntry( xNewTiService,
                                               &usTiIndex,
                                               &usCurrentHandle,
                                               pxService,
                                               usAfrIndex );
                break;
            }

            case eBTDbDescriptor:
            {
                xStatus = _prvCreateDescriptorEntry( xNewTiService,
                                                     &usTiIndex,
                                                     &usCurrentHandle,
                                                     pxService,
                                                     usAfrIndex );
                break;
            }

            default:
                break;
        }


        /* Error handling based on the results of the last switch case */
        if( xStatus != eBTStatusSuccess )
        {
            if( xNewTiService != NULL )
            {
                uint8_t i;
                // Free the new service
                for( i = 0; i < usNumAttributes; i++ )
                {
                    if( memcmp(xNewTiService[i].type.uuid, characterUUID, 2) == 0 )
                    {
                        // We need to skip another attribute
                        i++;
                    }
                    else
                    {
                        if( xNewTiService[i].pValue != NULL )
                        {
                            // Free value allocation
                            vPortFree(xNewTiService[i].pValue);
                        }
                    }
                }
                vPortFree(xNewTiService);
            }

            return ( xStatus );
        }
    }

    /* Now, we have iterated over the AFR service definition table and
     * converted it to a TI definition, commit it to the GATT server
     */
    xStackStatus = GATTServApp_RegisterService( xNewTiService,
                                                usNumAttributes,
                                                GATT_MAX_ENCRYPT_KEY_SIZE,
                                                &xServiceCBs );

    xStatus = _IotBleHalTypes_ConvertStatus( xStackStatus );

    /* Error handling if the registration wasn't a success */
    if( xStatus != eBTStatusSuccess )
    {
        if( xNewTiService != NULL )
        {
            uint8_t i;
            // Free the new service
            for( i = 0; i < usNumAttributes; i++ )
            {
                if( memcmp(xNewTiService[i].type.uuid, characterUUID, 2) == 0 )
                {
                    // We need to skip another attribute
                    i++;
                }
                else
                {
                    if( xNewTiService[i].pValue != NULL )
                    {
                        // Free value allocation
                        vPortFree(xNewTiService[i].pValue);
                    }
                }
            }
            vPortFree(xNewTiService);
        }

        return ( xStatus );
    }

    xStatus = _prvAddToServiceList(xNewTiService, usNumAttributes);
    if( xStatus != eBTStatusSuccess )
    {
       return xStatus;
    }

    return ( xStatus );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTAddCharacteristic( uint8_t ucServerIf,
                                           uint16_t usServiceHandle,
                                           BTUuid_t * pxUuid,
                                           BTCharProperties_t xProperties,
                                           BTCharPermissions_t xPermissions )
{
    return ( eBTStatusUnsupported );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvAddToServiceList(gattAttribute_t *newAtt, uint16_t numAtts)
{
    // create new node
    servicesList_t *pNewNode = NULL;

    pNewNode = (servicesList_t*)pvPortMalloc(sizeof(servicesList_t));

    if( pNewNode == NULL )
    {
        return eBTStatusFail;
    }

    // save the pointer and the num attribute
    pNewNode->pService = newAtt;
    pNewNode->numAttributes = numAtts;
    pNewNode->next = NULL;

    if( pServList == NULL )
    {
        pServList = pNewNode;
    }
    else
    {
        pNewNode->next = pServList;
        pServList = pNewNode;
    }

    return eBTStatusSuccess;
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTSetVal( BTGattResponse_t * pxValue )
{
    return ( eBTStatusUnsupported );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTAddDescriptor( uint8_t ucServerIf,
                                       uint16_t usServiceHandle,
                                       BTUuid_t * pxUuid,
                                       BTCharPermissions_t xPermissions )
{
    return ( eBTStatusUnsupported );
}

static BTStatus_t _prvBTStartService( uint8_t ucServerIf,
                                      uint16_t usServiceHandle,
                                      BTTransport_t xTransport )
{
    return ( eBTStatusUnsupported );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTStopService( uint8_t ucServerIf,
                                     uint16_t usServiceHandle )
{
    BTStatus_t xStatus = eBTStatusSuccess;

    /* It is not supported to stop a GATT service, so we just return success.
     */
    if( _xGattServerCallbacks.pxServiceStoppedCb )
    {
        _xGattServerCallbacks.pxServiceStoppedCb( eBTStatusSuccess,
                                                  ucServerIf,
                                                  usServiceHandle );
    }

    return ( xStatus );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTDeleteService( uint8_t ucServerIf,
                                       uint16_t usServiceHandle )
{
    BTStatus_t xStatus = eBTStatusSuccess;
    gattAttribute_t *p2pAttrs;
    bStatus_t xStackStatus = SUCCESS;

    xStackStatus = GATTServApp_DeregisterService( usServiceHandle, &p2pAttrs );
    xStatus = _IotBleHalTypes_ConvertStatus( xStackStatus );

    if( p2pAttrs )
    {
      vPortFree(p2pAttrs);
    }

    if( _xGattServerCallbacks.pxServiceDeletedCb )
    {
        _xGattServerCallbacks.pxServiceDeletedCb( xStatus,
                                                  ucServerIf,
                                                  usServiceHandle );
    }

    return ( xStatus );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTSendIndication( uint8_t ucServerIf,
                                        uint16_t usAttributeHandle,
                                        uint16_t usConnId,
                                        size_t xLen,
                                        uint8_t * pucValue,
                                        bool bConfirm )
{


    uint16_t usServiceHandle;
    uint16_t usNumSvcAttrs = 0;
    uint8_t **pucAttrVal;
    bStatus_t xStackStatus = SUCCESS;
    BTStatus_t xStatus = eBTStatusSuccess;

    /* Look up the attribute structure based on handle */
    gattAttribute_t * pxAttr = (gattAttribute_t *)GATT_FindHandle( usAttributeHandle, &usServiceHandle );
    if ( pxAttr == NULL )
    {
        return ( eBTStatusParamInvalid );
    }

    /* Set the pValue pointer of the attribute to match that of the buffer
     * That just came in. This because the stack will expect them to match later
     * inside _prvSendNotiInd
     *
     * Use the same double pointer trick to access the const pValue member as
     * used in service creation
     */
    pucAttrVal = ( uint8_t ** )&(pxAttr->pValue);
    *pucAttrVal = pucValue;

    /* Get the service declaration attribute so we can find the num attrs */
    gattAttribute_t * pxSvc = (gattAttribute_t *)GATT_FindHandle( usServiceHandle, NULL );
    if ( pxSvc == NULL )
    {
        return ( eBTStatusParamInvalid );
    }

    /* Get the number of attributes in the service where the CCCD belongs */
    usNumSvcAttrs = GATT_ServiceNumAttrs( usServiceHandle );

    gattCharCfg_t * pxCCCD = ( gattCharCfg_t * )(++pxAttr)->pValue;

    /* Actually do the heavy lifting to accomplish sending noti/ind */
    xStackStatus = _prvSendNotiInd( usConnId,
                                    GATT_CCC_TBL(pxCCCD),
                                    pucValue,
                                    xLen,
                                    false,
                                    pxSvc,
                                    usNumSvcAttrs,
                                    _IotBleHalAsyncMq_GetEntity() );

    xStatus = _IotBleHalTypes_ConvertStatus( xStackStatus );

    if( _xGattServerCallbacks.pxIndicationSentCb )
    {
        _xGattServerCallbacks.pxIndicationSentCb( usConnId,
                                                  xStatus );
    }

    return ( xStatus );
}

/*-----------------------------------------------------------*/

static BTStatus_t _prvBTSendResponse( uint16_t usConnId,
                                      uint32_t ulTransId,
                                      BTStatus_t xStatus,
                                      BTGattResponse_t * pxResponse )
{
    BTStatus_t xReturnStatus = eBTStatusSuccess;
    bStatus_t xStackStatus = SUCCESS;

    /* In this function we handle the application's response to the read
     * or write callback that was passed up from GATTServApp.
     * Now the application has some data ready for response, so we respond
     * manually. The ulTransId is the the ATT op code that was passed along in
     * the method parameter of the read/write attribute callbacks
     * This helps us to look up what we need to do here
     */
    switch ( ulTransId )
    {

        /* Handle read commands, the general pattern here is to allocate
         * A payload and copy in the application data before passing onto the
         * stack
         */
        case ATT_READ_REQ:
        {
            attReadRsp_t rsp;
            rsp.pValue = (uint8_t *)GATT_bm_alloc( usConnId,
                                                   ATT_READ_RSP,
                                                   (uint16_t) pxResponse->xAttrValue.xLen,
                                                   NULL );
            if( rsp.pValue )
            {
                rsp.len = (uint16_t) pxResponse->xAttrValue.xLen;
                memcpy( rsp.pValue, pxResponse->xAttrValue.pucValue, rsp.len );
                xStackStatus = ATT_ReadRsp( usConnId , &rsp );
                if( xStackStatus != SUCCESS )
                {
                    GATT_bm_free( (gattMsg_t * ) &rsp, ATT_READ_RSP );
                }
            }
            else
            {
                return ( eBTStatusNoMem );
            }

            break;
        }
        case ATT_READ_BLOB_REQ:
        {
            attReadBlobRsp_t rsp;
            rsp.pValue = (uint8_t *)GATT_bm_alloc( usConnId,
                                                   ATT_READ_BLOB_RSP,
                                                   (uint16_t) pxResponse->xAttrValue.xLen,
                                                   NULL );
            if( rsp.pValue )
            {
                rsp.len = (uint16_t) pxResponse->xAttrValue.xLen;
                memcpy( rsp.pValue, pxResponse->xAttrValue.pucValue, rsp.len );
                xStackStatus = ATT_ReadBlobRsp( usConnId , &rsp );
                if( xStackStatus != SUCCESS )
                {
                    GATT_bm_free( (gattMsg_t * ) &rsp, ATT_READ_BLOB_RSP );
                }
            }
            else
            {
                return ( eBTStatusNoMem );
            }
            break;
        }
        case ATT_READ_BY_TYPE_REQ:
        {
            xStackStatus = GATTServApp_ReadRsp( usConnId,
                                                pxResponse->xAttrValue.pucValue,
                                                (uint16_t) pxResponse->xAttrValue.xLen,
                                                pxResponse->usHandle );
            break;
        }

        /* Handle write commands manually */
        case ATT_WRITE_REQ:
        {
            xStackStatus = ATT_WriteRsp( usConnId );
            break;
        }

        case ATT_EXECUTE_WRITE_REQ:
        {
            xStackStatus = ATT_ExecuteWriteRsp( usConnId );
            break;
        }
        default:
            break;
    }

    xReturnStatus = _IotBleHalTypes_ConvertStatus( xStackStatus );

    if( _xGattServerCallbacks.pxResponseConfirmationCb != NULL )
    {
        _xGattServerCallbacks.pxResponseConfirmationCb( xStatus, pxResponse->usHandle );
    }

    return ( xReturnStatus );
}

/*-----------------------------------------------------------*/

const void * _prvBTGetGattServerInterface()
{
    return &xGATTserverInterface;
}

/*-----------------------------------------------------------*/

BTStatus_t xCleanGattTable( void )
{
    servicesList_t *pTemp = pServList;
    bStatus_t xStackStatus = SUCCESS;
    BTStatus_t xStatus = eBTStatusSuccess;
    gattAttribute_t *p2pAttrs;
    uint8_t i;

    while( pTemp != NULL )
    {
        uint16_t serviceHandle = GATT_SERVICE_HANDLE(pTemp->pService);

        // unregister all the services from the service list
        xStackStatus = GATTServApp_DeregisterService( serviceHandle, &p2pAttrs );
        xStatus = _IotBleHalTypes_ConvertStatus( xStackStatus );

        if( xStatus != eBTStatusSuccess )
        {
            return xStatus;
        }

        if( p2pAttrs )
        {
            // Free the attribute deleted from the GATT table
            for( i = 0; i < pTemp->numAttributes; i++ )
            {
                if( memcmp(p2pAttrs[i].type.uuid, characterUUID, 2) == 0 )
                {
                    // We need to skip another attribute
                    i++;
                }
                else
                {
                    if( p2pAttrs[i].pValue != NULL )
                    {
                        // Free value allocation
                        vPortFree(p2pAttrs[i].pValue);
                    }
                }
            }
            vPortFree(p2pAttrs);
            p2pAttrs = NULL;
        }

        pServList = pServList->next;
        if( pTemp != NULL )
        {
            // free the service list node
            vPortFree(pTemp);
        }
        pTemp = pServList;
    }

    return eBTStatusSuccess;
}

/*-----------------------------------------------------------*/
