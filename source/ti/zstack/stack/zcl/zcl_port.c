/******************************************************************************
   Filename:       zcl_port.c
   Revised:        $Date: 2015-02-12 12:55:11 -0800 (Thu, 12 Feb 2015) $
   Revision:       $Revision: 42532 $

   Description:    This file contains the ZCL porting layer.


  Copyright (c) 2019, Texas Instruments Incorporated
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
******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <xdc/std.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

#include <ti/sysbios/BIOS.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <string.h>
#include <inc/hw_ints.h>

#include "zstackapi.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_port.h"

#include "bdb_reporting.h"

#include "rom_jt_154.h"

#include "zglobals.h"

#include "ti_zstack_config.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define EXT_ADDR_LEN 8

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
    uint8_t entity;
    endPointDesc_t *pEpDesc;
#if defined (ZCL_SCENES)
    zclGeneral_Scene_t scene;
#endif
} zclPort_entityEPDesc_t;

#if defined (ZCL_SCENES)
// Scene NV types
typedef struct zclGenSceneNVItem
{
    uint8_t endpoint;
    zclGeneral_Scene_t scene;
}zclGenSceneNVItem_t;
#endif

typedef struct
{
    void *next;
    uint8_t endpoint;
    zclport_pFnZclHandleExternal pfn;
} zclHandleExternalList_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static NVINTF_nvFuncts_t *pfnZclPortNV = NULL;
#if defined (ZCL_SCENES)
static uint16_t zclSceneNVID = ZCL_PORT_SCENE_TABLE_NV_ID;
#endif

#if defined (ZCL_GROUPS)
static aps_Group_t foundGrp;
#endif

static zclPort_entityEPDesc_t entityEPDescs[MAX_SUPPORTED_ENDPOINTS] = {0};

static zstack_sysNwkInfoReadRsp_t nwkInfo =
{
    0xFFFE,
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    zstack_DevState_HOLD,
    0xFFFF,
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    0xFFFE,
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
    { 0, 0, 0 },
    0
};

#if defined (ZCL_SCENES)
static uint8_t lastFindSceneEndpoint = 0xFF;
#endif

// Function pointer for applications to ZCL Handle External
static zclHandleExternalList_t *zclHandleExternalList = NULL;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void convertTxOptions(zstack_TransOptions_t *pOptions, uint8_t options);
endPointDesc_t *zcl_afFindEndPointDesc(uint8_t EndPoint);
uint8_t zclPortFindEntity(uint8_t EndPoint);
/*********************************************************************
* PUBLIC FUNCTIONS
*********************************************************************/

/*********************************************************************
* API Functions
*********************************************************************/

/**
 * Register an AF Endpoint.  This is needed by the ZCL code
 * to find an AF endpoint descriptor.
 *
 * Public function defined in zcl_port.h
 */
bool zclport_registerEndpoint(uint8_t entity, endPointDesc_t *pEpDesc)
{
    uint8_t x;

    // Register endpoint that does not exist already
    if( zcl_afFindEndPointDesc(pEpDesc->endPoint) == NULL )
    {
        for(x = 0; x < MAX_SUPPORTED_ENDPOINTS; x++)
        {
            if(entityEPDescs[x].pEpDesc == NULL)
            {
                zstack_afRegisterReq_t regReq = {0};
                zstack_SimpleDescriptor_t simpleDesc;

                // Save information to local table
                entityEPDescs[x].entity = entity;
                entityEPDescs[x].pEpDesc = pEpDesc;

                // Register an endpoint with the stack thread
                simpleDesc.endpoint = pEpDesc->endPoint;
                simpleDesc.profileID = pEpDesc->simpleDesc->AppProfId;
                simpleDesc.deviceID = pEpDesc->simpleDesc->AppDeviceId;
                simpleDesc.deviceVer = pEpDesc->simpleDesc->AppDevVer;
                simpleDesc.n_inputClusters = pEpDesc->simpleDesc->AppNumInClusters;
                simpleDesc.pInputClusters = pEpDesc->simpleDesc->pAppInClusterList;
                simpleDesc.n_outputClusters =
                    pEpDesc->simpleDesc->AppNumOutClusters;
                simpleDesc.pOutputClusters =
                    pEpDesc->simpleDesc->pAppOutClusterList;
                regReq.endpoint = pEpDesc->endPoint;
                regReq.pSimpleDesc = &simpleDesc;
                regReq.latencyReq = zstack_NetworkLatency_NO_LATENCY_REQS;
                (void)Zstackapi_AfRegisterReq(entity, &regReq);

                return(true);
            }
        }
    }

    return(false);
}

/**
 * Call to register the functions pointers for the NV driver.
 *
 * Public function defined in zcl_port.h
 */
void zclport_registerNV(NVINTF_nvFuncts_t *pfnNV, uint16_t sceneNVID)
{
    pfnZclPortNV = pfnNV;
#if defined (ZCL_SCENES)
    zclSceneNVID = sceneNVID;
#endif
}

/**
 * Call to register a function pointer to handle zcl_HandleExternal() messages.
 *
 * Public function defined in zcl_port.h
 */
bool zclport_registerZclHandleExternal( uint8_t endpoint, zclport_pFnZclHandleExternal pfn)
{
    zclHandleExternalList_t *find = zclHandleExternalList;
    zclHandleExternalList_t *tail = NULL;
    if( zcl_afFindEndPointDesc(endpoint) != NULL )
    {
        // Find matching endpoint or tail
        while( find != NULL )
        {
            if( find->endpoint == endpoint)
            {
                find->pfn = pfn;
                return true;
            }
            if( find->next == NULL )
            {
                tail = find;
            }
            find = find->next;
        }

        // Add new item
        zclHandleExternalList_t *newItem = zcl_mem_alloc( sizeof(zclHandleExternalList_t) );
        if(newItem)
        {
            newItem->next = NULL;
            newItem->endpoint = endpoint;
            newItem->pfn = pfn;
            if( zclHandleExternalList == NULL )
            {
                zclHandleExternalList = newItem;
            }
            else
            {
                tail->next = newItem;
            }
            return true;
        }
    }
    return false;
}


/**
 * Call to get Device Information.
 *
 * Public function defined in zcl_port.h
 */
zstack_sysNwkInfoReadRsp_t *zclport_getDeviceInfo(uint8_t entity)
{
    Zstackapi_sysNwkInfoReadReq(entity, &nwkInfo);
    return(&nwkInfo);
}

/**
 * Determines if the device is already part of a network by asking the
 * stack thread.
 *
 * Public function defined in zcl_port.h
 */
bool zclport_isAlreadyPartOfNetwork(uint8_t entity)
{
    zstack_sysConfigReadReq_t readReq = {0};
    zstack_sysConfigReadRsp_t readRsp = {0};

    // Ask if the device is already part of a network
    readReq.devPartOfNetwork = true;

    (void)Zstackapi_sysConfigReadReq(entity, &readReq, &readRsp);

    return(readRsp.devPartOfNetwork);
}

/**
 * If the NV item does not already exist, it is created and
 * initialized with the data passed to the function, if any.
 *
 * Public function defined in zcl_port.h
 */
uint8_t zclport_initializeNVItem(uint16_t id, uint16_t subId, uint16_t len,
                                 void *buf)
{
    if(pfnZclPortNV && pfnZclPortNV->createItem)
    {
        uint32_t nvLen = 0;
        NVINTF_itemID_t nvId;

        nvId.systemID = NVINTF_SYSID_APP;
        nvId.itemID = (uint16_t)id;
        nvId.subID = (uint16_t)subId;

        if(pfnZclPortNV->getItemLen)
        {
            nvLen = pfnZclPortNV->getItemLen(nvId);
        }

        if(nvLen == len)
        {
            // Already exists and length is good
            return(SUCCESS);
        }

        if(pfnZclPortNV->createItem(nvId, len, buf) == NVINTF_FAILURE)
        {
            // Operation failed
            return(NV_OPER_FAILED);
        }
    }

    // NV was created
    return(NV_ITEM_UNINIT);
}

/**
 * Write a data item to NV. Function can write an entire item to NV or
 * an element of an item by indexing into the item with an offset.
 *
 * Public function defined in zcl_port.h
 */
uint8_t zclport_writeNV(uint16_t id, uint16_t subId,
                        uint16_t len,
                        void *buf)
{
    uint8_t rtrn = SUCCESS;

    if(pfnZclPortNV && pfnZclPortNV->writeItem)
    {
        uint32_t nvLen = 0;
        NVINTF_itemID_t nvId;

        nvId.systemID = NVINTF_SYSID_APP;
        nvId.itemID = (uint16_t)id;
        nvId.subID = (uint16_t)subId;

        if(pfnZclPortNV->getItemLen)
        {
            nvLen = pfnZclPortNV->getItemLen(nvId);
        }

        if(nvLen > 0)
        {
            if(pfnZclPortNV->writeItem(nvId, len, buf)
               == NVINTF_FAILURE)
            {
                rtrn = NV_OPER_FAILED;
            }
        }
        else
        {
            rtrn = NV_ITEM_UNINIT;
        }
    }

    return rtrn;
}

/**
 * Read data from NV. This function can be used to read an entire item
 * from NV or an element of an item by indexing into the item with an
 * offset. Read data is copied into buf.
 *
 * Public function defined in zcl_port.h
 */
uint8_t zclport_readNV(uint16_t id, uint16_t subId, uint16_t ndx, uint16_t len,
                       void *buf)
{
    uint8_t ret = SUCCESS;

    if(pfnZclPortNV && pfnZclPortNV->readItem)
    {
        NVINTF_itemID_t nvId;

        nvId.systemID = NVINTF_SYSID_APP;
        nvId.itemID = (uint16_t)id;
        nvId.subID = (uint16_t)subId;

        if(pfnZclPortNV->readItem(nvId, ndx, len, buf) == NVINTF_FAILURE)
        {
            ret = NV_OPER_FAILED;
        }
    }

    return(ret);
}

/********************************************************************
 * @fn      zclport_deleteNV
 *
 * @brief   Delete item from NV. This function will fail if the length
 *          parameter does not match the length of the item in NV.
 *
 * @param   id  - Valid NV item Id.
 * @param   subId - Valid NV item sub Id.
 * @param   len - Length of item to delete.
 *
 * @return  SUCCESS if item was deleted,
 *          NV_ITEM_UNINIT if item did not exist in NV,
 *          NV_BAD_ITEM_LEN if length parameter not correct,
 *          NV_OPER_FAILED if attempted deletion failed.
 */
uint8_t zclport_deleteNV(uint16_t id, uint16_t subId, uint16_t len)
{
  uint8_t ret = SUCCESS;

  if (pfnZclPortNV && pfnZclPortNV->deleteItem)
  {
    uint32_t nvLen = 0;
    NVINTF_itemID_t nvId;

    nvId.systemID = NVINTF_SYSID_APP;
    nvId.itemID = (uint16_t)id;
    nvId.subID = (uint16_t)subId;

    if(pfnZclPortNV->getItemLen)
    {
      nvLen = pfnZclPortNV->getItemLen(nvId);
    }

    if(nvLen == 0)
    {
      ret = NV_ITEM_UNINIT;
    }
    else if(nvLen != len)
    {
      ret = NV_BAD_ITEM_LEN;
    }
    else if(pfnZclPortNV->deleteItem(nvId) == NVINTF_FAILURE)
    {
      ret = NV_OPER_FAILED;
    }
  }

  return(ret);
}

/*********************************************************************
 * @fn      zcl_HandleExternal
 *
 * @brief   Callback function to handle messages externally
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  TRUE
 */
uint8_t zcl_HandleExternal(zclIncoming_t *pInMsg)
{

#ifdef BDB_REPORTING
    zclIncomingMsg_t *pCmd;

    pCmd = (zclIncomingMsg_t *)OsalPort_msgAllocate( sizeof ( zclIncomingMsg_t ) );
    if ( pCmd != NULL )
    {
      // fill in the message
      pCmd->hdr.event = ZCL_INCOMING_MSG;
      pCmd->zclHdr    = pInMsg->hdr;
      pCmd->clusterId = pInMsg->msg->clusterId;
      pCmd->srcAddr   = pInMsg->msg->srcAddr;
      pCmd->endPoint  = pInMsg->msg->endPoint;
      pCmd->attrCmd   = pInMsg->attrCmd;

      if(pCmd->zclHdr.commandID == ZCL_CMD_CONFIG_REPORT)
      {
        zstack_bdbProcessInConfigReportReq_t Req = {0};
        Req.pZclIncommingMsg = pCmd;

        Zstackapi_bdbProcessInConfigReportCmd(zclPortFindEntity(pCmd->endPoint),&Req);
        OsalPort_msgDeallocate((uint8_t*)pCmd);
        return TRUE;
      }
      if(pCmd->zclHdr.commandID == ZCL_CMD_READ_REPORT_CFG)
      {
        zstack_bdbProcessInReadReportCfgReq_t Req = {0};
        Req.pZclIncommingMsg = pCmd;

        Zstackapi_bdbProcessInReadReportCfgCmd(zclPortFindEntity(pCmd->endPoint),&Req);

        OsalPort_msgDeallocate((uint8_t*)pCmd);
        return TRUE;
      }
      OsalPort_msgDeallocate((uint8_t*)pCmd);
    }

#endif

    // Did the application register to handle this message
    zclHandleExternalList_t *find = zclHandleExternalList;
    while( find )
    {
        // Find matching endpoint with defined handle
        if( ( find->endpoint == pInMsg->msg->endPoint ) && ( find->pfn ) )
        {
            // Let the application handle it
            return(find->pfn(pInMsg));
        }
        find = find->next;
    }
    return(TRUE);
}

/*********************************************************************
 * @fn          zcl_mem_alloc
 *
 * @brief       Abstraction function to allocate memory
 *
 * @param       size - size in bytes needed
 *
 * @return      pointer to allocated buffer, NULL if nothing allocated.
 */
void *zcl_mem_alloc(uint16_t size)
{
    return( (void *)OsalPort_malloc(size) );
}

/*********************************************************************
 * @fn      zcl_memset
 *
 * @brief   Abstract function to memset
 *
 * @param   dest - buffer to set
 * @param   value - value to set into memory
 * @param   len - length to set in memory
 *
 * @return  pointer to buffer after set
 */
void *zcl_memset(void *dest, uint8_t value, int len)
{
    return( (void *)memset(dest, value, len) );
}

/*********************************************************************
 * @fn      zcl_memcpy
 *
 * @brief   Generic memory copy.
 *
 *   Note: This function differs from the standard OsalPort_memcpy(), since
 *         it returns the pointer to the next destination uint8_t. The
 *         standard OsalPort_memcpy() returns the original destination address.
 *
 * @param   dst - pointer to destination memory
 * @param   src - pointer to source memory
 * @param   len - length to copy
 *
 * @return  pointer to buffer after set
 */
void *zcl_memcpy(void *dst, void *src, unsigned int len)
{
    uint8_t *pDst = dst;
    uint8_t *pSrc = src;

    while(len--)
    {
        *pDst++ = *pSrc++;
    }

    return(pDst);
}

/*********************************************************************
 * @fn      zcl_memcmp
 *
 * @brief
 *
 *   Generic memory compare.
 *
 *    *   Note: This function differs from the standard memcmp(), since
 *         it returns the TRUE to when memory is the same and FALSE when
 *         the memory is different.
 *
 * @param   src1 - source 1 address
 * @param   src2 - source 2 address
 * @param   len - number of bytes to compare
 *
 * @return  TRUE - same, FALSE - different
 */
uint8_t zcl_memcmp(const void *src1, const void *src2, unsigned int len)
{
  const uint8_t *pSrc1;
  const uint8_t *pSrc2;

  pSrc1 = src1;
  pSrc2 = src2;

  while ( len-- )
  {
    if( *pSrc1++ != *pSrc2++ )
      return FALSE;
  }
  return TRUE;
}

/*********************************************************************
 * @fn      zcl_cpyExtAddr
 *
 * @brief   Copy an extended address.
 *
 * @param   dst - pointer to destination memory
 * @param   src - pointer to source memory
 *
 * @return  pointer to buffer after set
 */
void *zcl_cpyExtAddr(uint8_t *pDest, const uint8_t *pSrc)
{
    return zcl_memcpy( (void *)pDest, (void *)pSrc, EXT_ADDR_LEN );
}

/*********************************************************************
 * @fn      zcl_mem_free
 *
 * @brief   Abstract function to free allocated memory
 *
 * @param   ptr - pointer to allocated memory
 */
void zcl_mem_free(void *ptr)
{
  if(ptr != NULL)
  {
    OsalPort_free(ptr);
  }
}

/*********************************************************************
 * @fn      zcl_buffer_uint32
 *
 * @brief   Abstract function to break a uin32 into a buffer
 *
 * @param   buf - pointer to destination memory
 * @param   val - value to break
 *
 * @return  pointer to buffer after set
 */
uint8_t *zcl_buffer_uint32(uint8_t *buf, uint32_t val)
{
    *buf++ = BREAK_UINT32(val, 0);
    *buf++ = BREAK_UINT32(val, 1);
    *buf++ = BREAK_UINT32(val, 2);
    *buf++ = BREAK_UINT32(val, 3);

    return buf;
}

/*********************************************************************
 * @fn      zcl_build_uint32
 *
 * @brief   Abstract function to build a uint32_t from an array of bytes
 *
 * @param   swapped - array of bytes
 * @param   len - length of array
 *
 * @return  uint32_t value
 */
uint32_t zcl_build_uint32(uint8_t *swapped, uint8_t len)
{
    if(len == 2)
    {
        return( BUILD_UINT32(swapped[0], swapped[1], 0L, 0L) );
    }
    else if(len == 3)
    {
        return( BUILD_UINT32(swapped[0], swapped[1], swapped[2], 0L) );
    }
    else if(len == 4)
    {
        return( BUILD_UINT32(swapped[0], swapped[1], swapped[2], swapped[3]) );
    }
    else
    {
        return( (uint32_t)swapped[0] );
    }
}

/*********************************************************************
 * @fn      zclPortFind
 *
 * @brief   Find the endpoint descriptor from endpoint
 *
 * @param   EndPoint - endpoint
 *
 * @return  pointer to found endpoint descriptor, NULL if not found
 */
zclPort_entityEPDesc_t *zclPortFind(uint8_t EndPoint)
{
    uint8_t x;

    for(x = 0; x < MAX_SUPPORTED_ENDPOINTS; x++)
    {
        if( entityEPDescs[x].pEpDesc
            && (EndPoint == entityEPDescs[x].pEpDesc->endPoint) )
        {
            return(&entityEPDescs[x]);
        }
    }

    return( (zclPort_entityEPDesc_t *)NULL );
}

/*********************************************************************
 * @fn      zclPortFindEntity
 *
 * @brief   Find the Task ID from endpoint
 *
 * @param   EndPoint - endpoint
 *
 * @return  Task ID
 */
uint8_t zclPortFindEntity(uint8_t EndPoint)
{
    uint8_t x;

    for(x = 0; x < MAX_SUPPORTED_ENDPOINTS; x++)
    {
        if( (entityEPDescs[x].pEpDesc)
            && (EndPoint == entityEPDescs[x].pEpDesc->endPoint) )
        {
            return(entityEPDescs[x].entity);
        }
    }

    return 0U;
}

/*********************************************************************
 * @fn      afFindEndPointDesc
 *
 * @brief   Find the endpoint descriptor from endpoint
 *
 * @param   EndPoint - endpoint
 *
 * @return  pointer to found endpoint descriptor, NULL if not found
 */
endPointDesc_t *zcl_afFindEndPointDesc(uint8_t EndPoint)
{
    uint8_t x;

    for(x = 0; x < MAX_SUPPORTED_ENDPOINTS; x++)
    {
        if( (entityEPDescs[x].pEpDesc)
            && (EndPoint == entityEPDescs[x].pEpDesc->endPoint) )
        {
            return(entityEPDescs[x].pEpDesc);
        }
    }

    return( (endPointDesc_t *)NULL );
}

/*********************************************************************
 * @fn      zcl_AF_DataRequest
 *
 * @brief   Common functionality for invoking APSDE_DataReq() for both
 *          SendMulti and MSG-Send.
 *
 *          NOTE: this is a conversion function
 *
 * input parameters
 *
 * @param  *dstAddr - Full ZB destination address: Nwk Addr + End Point.
 * @param  *srcEP - Origination (i.e. respond to or ack to) End Point Descr.
 * @param   cID - A valid cluster ID as specified by the Profile.
 * @param   bufLen - Number of bytes of data pointed to by next param.
 * @param  *buf - A pointer to the data bytes to send.
 * @param  *transID - A pointer to a byte which can be modified and which will
 *                    be used as the transaction sequence number of the msg.
 * @param   options - Valid bit mask of Tx options.
 * @param   radius - Normally set to AF_DEFAULT_RADIUS.
 *
 * output parameters
 *
 * @param  *transID - Incremented by one if the return value is success.
 *
 * @return  afStatus_t - See previous definition of afStatus_... types.
 */
afStatus_t zcl_AF_DataRequest(afAddrType_t *dstAddr, endPointDesc_t *srcEP,
                          uint16_t cID, uint16_t bufLen, uint8_t *buf,
                          uint8_t *transID, uint8_t options,
                          uint8_t radius)
{
    afStatus_t status;
    zstack_afDataReq_t req;

    memset(&req, 0, sizeof(zstack_afDataReq_t));

    req.dstAddr.addrMode = (zstack_AFAddrMode)dstAddr->addrMode;
    if(req.dstAddr.addrMode == zstack_AFAddrMode_EXT)
    {
        OsalPort_memcpy(req.dstAddr.addr.extAddr, dstAddr->addr.extAddr, EXT_ADDR_LEN);
    }
    else if(req.dstAddr.addrMode != zstack_AFAddrMode_NONE)
    {
        req.dstAddr.addr.shortAddr = dstAddr->addr.shortAddr;
    }
    req.dstAddr.endpoint = dstAddr->endPoint;
    req.dstAddr.panID = dstAddr->panId;

    convertTxOptions(&(req.options), options);

    req.srcEndpoint = srcEP->endPoint;

    req.clusterID = cID;

    req.transID = transID;

    req.radius = radius;

    req.n_payload = bufLen;
    req.pPayload = buf;

    status = Zstackapi_AfDataReq(zclPortFindEntity(srcEP->endPoint), &req);

    return(status);
}

/******************************************************************************
* @fn          convertTxOptions
*
* @brief       Convert uint8_t txOptions into PB TransOptions data type
*
* @param       pOptions - TransOptions pointer
* @param       options - txOptions
*
* @return      none
******************************************************************************/
static void convertTxOptions(zstack_TransOptions_t *pOptions, uint8_t options)
{
    if(options & AF_WILDCARD_PROFILEID)
    {
        pOptions->wildcardProfileID = TRUE;
    }

    if(options & AF_ACK_REQUEST)
    {
        pOptions->ackRequest = TRUE;
    }

    if(options & AF_LIMIT_CONCENTRATOR)
    {
        pOptions->limitConcentrator = TRUE;
    }

    if(options & AF_SUPRESS_ROUTE_DISC_NETWORK)
    {
        pOptions->suppressRouteDisc = TRUE;
    }

    if(options & AF_EN_SECURITY)
    {
        pOptions->apsSecurity = TRUE;
    }

    if(options & AF_SKIP_ROUTING)
    {
        pOptions->skipRouting = TRUE;
    }
}

#if defined (ZCL_GROUPS)
/*********************************************************************
* APS Interface messages
*********************************************************************/

/*********************************************************************
 * @fn      zclport_aps_RemoveGroup
 *
 * @brief   Remove a group with endpoint and groupID
 *
 * @param   endpoint -
 * @param   groupID - ID to look forw group
 *
 * @return  TRUE if removed, FALSE if not found
 */
uint8_t zclport_aps_RemoveGroup(uint8_t endpoint, uint16_t groupID)
{
    uint8_t status;
    zstack_apsRemoveGroup_t req;

    req.endpoint = endpoint;
    req.groupID = groupID;

    status = Zstackapi_ApsRemoveGroupReq(zclPortFindEntity(endpoint), &req);

    return(status);
}

/*********************************************************************
 * @fn      zclport_aps_RemoveAllGroup
 *
 * @brief   Remove a groups with an endpoint
 *
 * @param   endpoint -
 * @param   groupID - ID to look for group
 *
 * @return  none
 */
void zclport_aps_RemoveAllGroup(uint8_t endpoint)
{
    zstack_apsRemoveAllGroups_t req;

    req.endpoint = endpoint;

    Zstackapi_ApsRemoveAllGroupsReq(zclPortFindEntity(endpoint), &req);
}

/*********************************************************************
 * @fn      zclport_aps_FindAllGroupsForEndpoint
 *
 * @brief   Find all the groups with endpoint
 *
 * @param   endpoint - endpoint to look for
 * @param   groupList - List to hold group IDs (should hold
 *                      APS_MAX_GROUPS entries)
 *
 * @return  number of groups copied to groupList
 */
uint8_t zclport_aps_FindAllGroupsForEndpoint(uint8_t endpoint, uint16_t *groupList)
{
    zstack_apsFindAllGroupsReq_t req;
    zstack_apsFindAllGroupsRsp_t rsp = {0};

    req.endpoint = endpoint;

    if( (groupList)
        && (Zstackapi_ApsFindAllGroupsReq(
                zclPortFindEntity(endpoint),
                &req, &rsp) == zstack_ZStatusValues_ZSuccess) )
    {
        uint8_t x;
        for(x = 0; x < rsp.numGroups; x++)
        {
            groupList[x] = rsp.pGroupList[x];
        }

        if(rsp.pGroupList)
        {
            OsalPort_free(rsp.pGroupList);
        }
    }

    return( (uint8_t)rsp.numGroups );
}

/*********************************************************************
 * @fn      zclport_aps_FindGroup
 *
 * @brief   Find a group with endpoint and groupID
 *
 * @param   endpoint -
 * @param   groupID - ID to look forw group
 *
 * @return  a pointer to the group information, NULL if not found
 */
aps_Group_t *zclport_aps_FindGroup(uint8_t endpoint, uint16_t groupID)
{
    aps_Group_t *pFound = (aps_Group_t *)NULL;
    zstack_apsFindGroupReq_t req;
    zstack_apsFindGroupRsp_t rsp;

    req.endpoint = endpoint;
    req.groupID = groupID;

    if(Zstackapi_ApsFindGroupReq(zclPortFindEntity(endpoint),
                                 &req, &rsp) == zstack_ZStatusValues_ZSuccess)
    {
        memset( &foundGrp, 0, sizeof(aps_Group_t) );
        foundGrp.ID = rsp.groupID;
        if(rsp.pName)
        {
            if(rsp.n_name <= APS_GROUP_NAME_LEN)
            {
                OsalPort_memcpy(foundGrp.name, rsp.pName, rsp.n_name);
            }
            OsalPort_free(rsp.pName);
        }

        pFound = &foundGrp;
    }

    return(pFound);
}

/*********************************************************************
 * @fn      zclport_aps_AddGroup
 *
 * @brief   Add a group for an endpoint
 *
 * @param   endpoint -
 * @param   group - new group
 *
 * @return  ZStatus_t
 */
ZStatus_t zclport_aps_AddGroup(uint8_t endpoint, aps_Group_t *group)
{
    uint8_t status = zstack_ZStatusValues_ZFailure;
    zstack_apsAddGroup_t req;

    memset( &req, 0, sizeof(zstack_apsAddGroup_t) );

    req.endpoint = endpoint;

    if(group)
    {
        uint8_t len;

        req.groupID = group->ID;
        len = strlen( (const char *)group->name );
        if(len)
        {
            req.n_name = len;
            req.pName = group->name;
        }

        status = Zstackapi_ApsAddGroupReq(zclPortFindEntity(endpoint), &req);
    }

    return(status);
}

/*********************************************************************
 * @fn      zclport_aps_CountAllGroups
 *
 * @brief   Count the total number of groups
 *
 * @param   none
 *
 * @return  number of groups
 */
uint8_t zclport_aps_CountAllGroups(void)
{
    // Slight cheat, don't know the endpoint - use first entity
    return( Zstackapi_ApsCountAllGroupsReq(entityEPDescs[0].entity) );
}
#endif // defined(ZCL_GROUPS)


#if defined (ZCL_SCENES)

/*********************************************************************
 * @fn      sceneRecEmpty
 *
 * @brief   Checks for an empty record
 *
 * @param   pNvItem - pointer to scene NV record
 *
 * @return  true if all bytes where 0xFF, false otherwise
 */
static bool sceneRecEmpty(zclGenSceneNVItem_t *pNvItem)
{
    uint8_t *pBuf = (uint8_t *)pNvItem;
    uint16_t x;

    for(x = 0; x < sizeof(zclGenSceneNVItem_t); x++)
    {
        if(*pBuf++ != 0xFF)
        {
            return(false);
        }
    }
    return(true);
}

/*********************************************************************
 * @fn      zclGeneral_ScenesInit
 *
 * @brief   Initialize the Scenes Table
 */
void zclGeneral_ScenesInit(void)
{
    uint16_t x;
    zclGenSceneNVItem_t temp;

    memset(&temp, 0xFF, sizeof(zclGenSceneNVItem_t));
    for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
    {
        zclport_initializeNVItem(zclSceneNVID, x,
                                 sizeof(zclGenSceneNVItem_t), &temp);
    }
}

/*********************************************************************
 * @fn      zclGeneral_RemoveAllScenes
 *
 * @brief   Remove all scenes for an endpoint and groupID
 *
 * @param   endpoint - endpoint to filter with
 * @param   groupID - group ID looking for
 */
void zclGeneral_RemoveAllScenes(uint8_t endpoint, uint16_t groupID)
{
    uint16_t x;
    zclGenSceneNVItem_t nvItem;

    for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
    {
        if(zclport_readNV(zclSceneNVID, x, 0,
                          sizeof(zclGenSceneNVItem_t), &nvItem) == SUCCESS)
        {
            if( (sceneRecEmpty(&nvItem) == false)
                && ( (nvItem.endpoint == endpoint) || (endpoint == 0xFF) )
                && (nvItem.scene.groupID == groupID) )
            {
                // Remove the item by setting it all to 0xFF
                memset( &nvItem, 0xFF, sizeof(zclGenSceneNVItem_t) );
                zclport_writeNV(zclSceneNVID, x,
                                sizeof(zclGenSceneNVItem_t), &nvItem);
            }
        }
    }
}

/*********************************************************************
 * @fn      zclGeneral_RemoveScene
 *
 * @brief   Remove a scene
 *
 * @param   endpoint - endpoint to filter with
 * @param   groupID - group ID looking for
 * @param   sceneID - scene ID
 *
 * @return  TRUE if removed, FALSE if not found
 */
uint8_t zclGeneral_RemoveScene(uint8_t endpoint, uint16_t groupID, uint8_t sceneID)
{
    uint16_t x;
    zclGenSceneNVItem_t nvItem;

    for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
    {
        if(zclport_readNV(zclSceneNVID, x, 0,
                          sizeof(zclGenSceneNVItem_t), &nvItem) == SUCCESS)
        {
            if( (sceneRecEmpty(&nvItem) == false)
                && ( (nvItem.endpoint == endpoint) || (endpoint == 0xFF) )
                && (nvItem.scene.groupID == groupID)
                && (nvItem.scene.ID == sceneID) )
            {
                // Remove the item by setting it all to 0xFF
                memset( &nvItem, 0xFF, sizeof(zclGenSceneNVItem_t) );
                if(zclport_writeNV(zclSceneNVID, x,
                                   sizeof(zclGenSceneNVItem_t),
                                   &nvItem) == SUCCESS)
                {
                    return(TRUE);
                }
                else
                {
                    return(FALSE);
                }
            }
        }
    }
    return(FALSE);
}

/*********************************************************************
 * @fn      zclGeneral_FindScene
 *
 * @brief   Find a scene with endpoint and sceneID
 *
 * @param   endpoint - endpoint filter to find scene
 * @param   groupID - what group the scene belongs to
 * @param   sceneID - ID to look for scene
 *
 * @return  a pointer to the scene information, NULL if not found
 */
zclGeneral_Scene_t *zclGeneral_FindScene(uint8_t endpoint, uint16_t groupID,
                                         uint8_t sceneID)
{
    uint16_t x;
    zclGenSceneNVItem_t nvItem;
    zclPort_entityEPDesc_t *pEPDesc = zclPortFind(endpoint);

    if(pEPDesc != NULL)
    {
        for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
        {
            if(zclport_readNV(zclSceneNVID, x, 0,
                              sizeof(zclGenSceneNVItem_t),
                              &nvItem) == SUCCESS)
            {
                if( (sceneRecEmpty(&nvItem) == false)
                    && ( (nvItem.endpoint == endpoint) || (endpoint == 0xFF) )
                    && (nvItem.scene.groupID == groupID)
                    && (nvItem.scene.ID == sceneID) )
                {
                    lastFindSceneEndpoint = endpoint;

                    // Copy to a temp area
                    OsalPort_memcpy( &(pEPDesc->scene), &(nvItem.scene),
                            sizeof(zclGeneral_Scene_t) );

                    return( &(pEPDesc->scene) );
                }
            }
        }
    }

    return( (zclGeneral_Scene_t *)NULL );
}

/*********************************************************************
 * @fn      zclGeneral_AddScene
 *
 * @brief   Add a scene for an endpoint
 *
 * @param   endpoint -
 * @param   scene - new scene item
 *
 * @return  ZStatus_t
 */
ZStatus_t zclGeneral_AddScene(uint8_t endpoint, zclGeneral_Scene_t *scene)
{
    uint16_t x;
    zclGenSceneNVItem_t nvItem;

    // See if the item exists already
    for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
    {
        if(zclport_readNV(zclSceneNVID, x, 0,
                          sizeof(zclGenSceneNVItem_t), &nvItem) == SUCCESS)
        {
            if( (sceneRecEmpty(&nvItem) == false)
                && (nvItem.endpoint == endpoint)
                && (nvItem.scene.groupID == scene->groupID)
                && (nvItem.scene.ID == scene->ID) )
            {
                break;
            }
        }
    }

    // Find an empty slot
    if(x == ZCL_GENERAL_MAX_SCENES)
    {
        for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
        {
            if(zclport_readNV(zclSceneNVID, x, 0,
                              sizeof(zclGenSceneNVItem_t),
                              &nvItem) == SUCCESS)
            {
                if( sceneRecEmpty(&nvItem) )
                {
                    break;
                }
            }
        }
    }

    if(x == ZCL_GENERAL_MAX_SCENES)
    {
        return(ZFailure);
    }

    // Item found or empty slot found
    nvItem.endpoint = endpoint;
    OsalPort_memcpy( &(nvItem.scene), scene, sizeof(zclGeneral_Scene_t) );

    if(zclport_writeNV(zclSceneNVID, x,
                       sizeof(zclGenSceneNVItem_t), &nvItem) == SUCCESS)
    {
        return(ZSuccess);
    }
    else
    {
        return(ZFailure);
    }
}

/*********************************************************************
 * @fn      zclGeneral_CountAllScenes
 *
 * @brief   Count the number of scenes
 *
 * @return  number of scenes found
 */
uint8_t zclGeneral_CountAllScenes(void)
{
    uint16_t x;
    zclGenSceneNVItem_t nvItem;
    uint8_t cnt = 0;

    for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
    {
        if(zclport_readNV(zclSceneNVID, x, 0,
                          sizeof(zclGenSceneNVItem_t), &nvItem) == SUCCESS)
        {
            if(sceneRecEmpty(&nvItem) == false)
            {
                cnt++;
            }
        }
    }

    return(cnt);
}

/*********************************************************************
 * @fn      zclGeneral_FindAllScenesForGroup
 *
 * @brief   Get all the scenes with groupID
 *
 * @param   endpoint - endpoint to filter with
 * @param   groupID - group ID looking for
 *
 * @return  number of scenes found
 */
uint8_t zclGeneral_FindAllScenesForGroup(uint8_t endpoint, uint16_t groupID,
                                       uint8_t *sceneList)
{
    uint16_t x;
    zclGenSceneNVItem_t nvItem;
    uint8_t cnt = 0;

    for(x = 0; x < ZCL_GENERAL_MAX_SCENES; x++)
    {
        if(zclport_readNV(zclSceneNVID, x, 0,
                          sizeof(zclGenSceneNVItem_t), &nvItem) == SUCCESS)
        {
            if( (sceneRecEmpty(&nvItem) == false)
                && (nvItem.endpoint == endpoint) &&
                (nvItem.scene.groupID == groupID) )
            {
                sceneList[cnt++] = nvItem.scene.ID;
            }
        }
    }
    return(cnt);
}

/*********************************************************************
 * @fn      zclGeneral_ScenesSave
 *
 * @brief   Save the Scenes Table - Something has changed.
 *          This function is only called if zclGeneral_FindScene()
 *          was called and the found information was changed.
 */
void zclGeneral_ScenesSave(void)
{
    if(lastFindSceneEndpoint != 0xFF)
    {
        zclPort_entityEPDesc_t *pEPDesc = zclPortFind(lastFindSceneEndpoint);
        if(pEPDesc)
        {
            zclGeneral_AddScene( lastFindSceneEndpoint, &(pEPDesc->scene) );
            lastFindSceneEndpoint = 0xFF;
        }
    }
}
#endif // ZCL_SCENES

/*********************************************************************
*********************************************************************/

