/******************************************************************************

 @file api_mac.c

 @brief TIMAC 2.0 API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

/******************************************************************************
 Includes
 *****************************************************************************/

#include <string.h>

#include "icall.h"
#include "api_mac.h"
#include "macstack.h"
#include "mac_util.h"
#include "macs.h"
#include "macwrapper.h"


/*!
 This module is the ICall interface for the application and all ICall
 activity must go through this module, no ICall activity anywhere else.
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/*! Capability Information - Device is capable of becoming a PAN coordinator */
#define CAPABLE_PAN_COORD       0x01
/*! Capability Information - Device is an FFD  */
#define CAPABLE_FFD             0x02
/*!
 Capability Information - Device is mains powered rather than battery powered
 */
#define CAPABLE_MAINS_POWER     0x04
/*! Capability Information - Device has its receiver on when idle  */
#define CAPABLE_RX_ON_IDLE      0x08
/*!
 Capability Information - Device is capable of sending
 and receiving secured frames
 */
#define CAPABLE_SECURITY        0x40
/*!
 Capability Information - Request allocation of a short address in the
 associate procedure
 */
#define CAPABLE_ALLOC_ADDR      0x80

/*! Offset into the payload for the payload IEs */
#define PAYLOAD_IE_OFFSET                    0
/*! Offset into the IE for the subIE */
#define PAYLOAD_IE_SUBIE_OFFSET              0

/*! Macro to get the IE Type */
#define PAYLOAD_IE_TYPE(p) (((p)[PAYLOAD_IE_OFFSET+1] >> 7) & 0x01)

/*! Macro to get the IE Group ID */
#define PAYLOAD_IE_GROUP_ID(p) ((((uint8_t *)p)[PAYLOAD_IE_OFFSET+1] >> 3) & 0x0f)

/*! Macro to get the IE Content Length */
#define PAYLOAD_IE_CONTENT_LEN(p) ((((uint8_t *)p)[PAYLOAD_IE_OFFSET+0] & 0x00ff) +\
                ((((uint8_t *)p)[PAYLOAD_IE_OFFSET+1] & 0x0007) << 8))

/*! Type value for payload IE */
#define PAYLOAD_IE_TYPE_VAL 1
/*! Type value for payload IE */
#define PAYLOAD_IE_HEADER_LEN 2

/*! Macro to get the short subIE length */
#define PAYLOAD_IE_SHORT_SUBIE_LEN(p) ((p)[PAYLOAD_IE_SUBIE_OFFSET+0])

/*! Macro to get the long subIE length */
#define PAYLOAD_IE_LONG_SUBIE_LEN(p) (((p)[PAYLOAD_IE_OFFSET+0] & 0x00ff) + \
                              (((p)[PAYLOAD_IE_OFFSET+1] & 0x0007)  << 8))

/*! Macro to get the subIE type */
#define PAYLOAD_IE_SUBIE_TYPE(p) (((p)[PAYLOAD_IE_SUBIE_OFFSET+1] >> 7) & 0x01)

/*! Macro to get the subIE ID */
#define PAYLOAD_IE_SUBIE_ID(p) ((p)[PAYLOAD_IE_SUBIE_OFFSET+1] & 0x7f)

/*! subIE header length */
#define PAYLOAD_SUB_IE_HEADER_LEN 2

/*! Short subIE type */
#define PAYLOAD_SUB_ID_IE_TYPE_SHORT 0
/*! Long subIE type */
#define PAYLOAD_SUB_ID_IE_TYPE_LONG 1

/*! Short subIE header length */
#define PAYLOAD_SUB_ID_IE_SHORT_HEADER_LEN  2

/*! Payload IE SubIE Type Size */
#define PAYLOAD_IE_SUB_IE_TYPE_SIZE 1
/*! Payload IE SubIE Type Position */
#define PAYLOAD_IE_SUB_IE_TYPE_POSITION 15
/*! Payload IE SubIE ID Short Size */
#define PAYLOAD_IE_SUB_IE_ID_SHORT_SIZE 7
/*! Payload IE SubIE ID Short Position */
#define PAYLOAD_IE_SUB_IE_ID_SHORT_POSITION 8
/*! Payload IE SubIE Short Length Size */
#define PAYLOAD_IE_SUB_IE_LEN_SHORT_SIZE 8
/*! Payload IE SubIE Short Length Position */
#define PAYLOAD_IE_SUB_IE_LEN_SHORT_POSITION 0
/*! Payload IE SubIE ID Long Size */
#define PAYLOAD_IE_SUB_IE_ID_LONG_SIZE 4
/*! Payload IE SubIE ID Long Position */
#define PAYLOAD_IE_SUB_IE_SUB_ID_LONG_POSITION 11
/*! Payload IE SubIE ID Long Length Size */
#define PAYLOAD_IE_SUB_IE_LEN_LONG_SIZE 11
/*! Payload IE SubIE Long Length Position */
#define PAYLOAD_IE_SUB_IE_LEN_LONG_POSITION 0

/*! Unpack a field from a uint16_t */
#define IE_UNPACKING(var,size,position) (((uint16_t)(var)>>(position))\
                &(((uint16_t)1<<(size))-1))

/*! Make a uint16_t from 2 uint8_t */
#define MAKE_UINT16(low, high) (((low)&0x00FF)|(((high)&0x00FF)<<8))

/*! Get the SubIE type field (bool) */
#define GET_SUBIE_TYPE(ctl) (bool)(IE_UNPACKING(ctl,\
                 PAYLOAD_IE_SUB_IE_TYPE_SIZE, PAYLOAD_IE_SUB_IE_TYPE_POSITION))

/*! Get the SubIE Long ID  */
#define GET_SUBIE_ID_LONG(ctl) (uint8_t)(IE_UNPACKING(ctl,\
       PAYLOAD_IE_SUB_IE_ID_LONG_SIZE, PAYLOAD_IE_SUB_IE_SUB_ID_LONG_POSITION))

/*! Get the SubIE Long Length */
#define GET_SUBIE_LEN_LONG(ctl) (uint16_t)(IE_UNPACKING(ctl,\
         PAYLOAD_IE_SUB_IE_LEN_LONG_SIZE, PAYLOAD_IE_SUB_IE_LEN_LONG_POSITION))

/*! Get the SubIE Short ID  */
#define GET_SUBIE_ID_SHORT(ctl) (uint8_t)(IE_UNPACKING(ctl,\
         PAYLOAD_IE_SUB_IE_ID_SHORT_SIZE, PAYLOAD_IE_SUB_IE_ID_SHORT_POSITION))

/*! Get the SubIE Short Length */
#define GET_SUBIE_LEN_SHORT(ctl) (uint16_t)(IE_UNPACKING(ctl,\
       PAYLOAD_IE_SUB_IE_LEN_SHORT_SIZE, PAYLOAD_IE_SUB_IE_LEN_SHORT_POSITION))

/******************************************************************************
 Structures
 *****************************************************************************/

/******************************************************************************
 Global variables
 *****************************************************************************/

/*! ICall thread entity */
ICall_EntityID ApiMac_appEntity = 0;

/*!
 The ApiMac_extAddr is the MAC's IEEE address, setup with the Chip's
 IEEE addresses in main.c
 */
ApiMac_sAddrExt_t ApiMac_extAddr;

/******************************************************************************
 Local variables
 *****************************************************************************/
/*! Semaphore used to post events to the application thread */
STATIC ICall_Semaphore sem;

/*! MAC callback table, initialized to no callback table */
STATIC ApiMac_callbacks_t *pMacCallbacks = (ApiMac_callbacks_t *) NULL;

STATIC ICall_EntityID macEntityID;

#ifdef DBG_APP
uint16_t appDbg_event[20];
uint8_t appDbg_eidx = 0;
#endif
/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
static ApiMac_status_t mlmeGetFhReq(uint16_t pibAttribute, void *pValue,
                                    uint16_t *pLen);
static uint16_t processIncomingICallMsg(macCbackEvent_t *pMsg);
static void copyMacAddrToApiMacAddr(ApiMac_sAddr_t *pDst, sAddr_t *pSrc);
static void copyMacPanDescToApiMacPanDesc(ApiMac_panDesc_t *pDst,
                                          macPanDesc_t *pSrc);
static void processBeaconNotifyInd(macMlmeBeaconNotifyInd_t *pInd);
static void processScanCnf(macMlmeScanCnf_t *pCnf);
static void macMsgDeallocate(macEventHdr_t *pData);
static void copyDataInd(ApiMac_mcpsDataInd_t *pDst, macMcpsDataInd_t *pSrc);
void copyApiMacAddrToMacAddr(sAddr_t *pDst, ApiMac_sAddr_t *pSrc);
static ApiMac_status_t parsePayloadIEs(uint8_t *pContent, uint16_t contentLen,
                                       ApiMac_payloadIeRec_t **pList,
                                       bool group);
uint16_t convertTxOptions(ApiMac_txOptions_t txOptions);
static ApiMac_status_t mlmeSetFhReq(uint16_t pibAttribute, void *pValue);

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize this module.

 Public function defined in api_mac.h
 */
void *ApiMac_init(bool enableFH)
{
    /* Allocate message buffer space */
    macStackInitParams_t *pMsg = (macStackInitParams_t *)ICall_allocMsg(
                    sizeof(macStackInitParams_t));

    /* Register the current thread as an ICall dispatcher application
     * so that the application can send and receive messages.
     */
    ICall_registerApp(&ApiMac_appEntity, &sem);

    if(pMsg != NULL)
    {
        /* Fill in the message content */
        pMsg->hdr.event = MAC_STACK_INIT_PARAMS;
        pMsg->hdr.status = 0;
        pMsg->srctaskid = ApiMac_appEntity;
        pMsg->retransmit = 0;
        pMsg->pendingMsg = 0;
        pMsg->pMacCbackQueryRetransmit = NULL;
        pMsg->pMacCbackCheckPending = NULL;

        /* Send the message to ICALL_SERVICE_CLASS_TIMAC */
        ICall_sendServiceMsg(ApiMac_appEntity, ICALL_SERVICE_CLASS_TIMAC,
                             (ICALL_MSG_FORMAT_3RD_CHAR_TASK_ID),
                             pMsg);
    }

    /* Enable frequency hopping? */
    if(enableFH)
    {
        ApiMac_enableFH();
    }

    /* Reset the MAC */
    ApiMac_mlmeResetReq(true);

    /* Set the device IEEE address */
    ApiMac_mlmeSetReqArray(ApiMac_attribute_extendedAddress, ApiMac_extAddr);

    /*
     Save the MAC Stack ICall Entity ID to be used to check received
     messages.
     */
    macEntityID = ICall_searchServiceEntity(ICALL_SERVICE_CLASS_TIMAC);

    return (sem);
}

/*!
 Register for MAC callbacks.

 Public function defined in api_mac.h
 */
void ApiMac_registerCallbacks(ApiMac_callbacks_t *pCallbacks)
{
    /* Save the application's callback table */
    pMacCallbacks = pCallbacks;
}

/*!
 Register for MAC callbacks.

 * @brief       This function waits the incoming messages forever. Once message is received,
 *              1. it will fetch the message from queue
 *              2. if message destion is Api_Mac_appEntity, it will check if source of
 *                 message is macEntity
 *                 if source of message is macEntity, it will call processIncomingICallMsg
 *                 otherwise if the there is unprocessedCb, it will invoke this callback function
 *              3. After the message is processed, it will free this message.
 *
 */
void ApiMac_processIncoming(void)
{
    ICall_EntityID src;
    ICall_EntityID dest;
    macCbackEvent_t *pMsg;

    /* Wait for response message */
    if(ICall_wait(ICALL_TIMEOUT_FOREVER) == ICALL_ERRNO_SUCCESS)
    {
        /* Retrieve the response message */
        if(ICall_fetchMsg(&src, &dest, (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
        {
            if(dest == ApiMac_appEntity)
            {
                /* Process the message from the MAC stack */
                processIncomingICallMsg(pMsg);
            }

            if(pMsg != NULL)
            {
                ICall_freeMsg(pMsg);
            }
        }
    }
}

/*!
 This function sends application data to the MAC for
 transmission in a MAC data frame.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mcpsDataReq(ApiMac_mcpsDataReq_t *pData)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_McpsDataReq, pData));
}

/*!
 This function purges and discards a data request from the MAC
 data queue.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mcpsPurgeReq(uint8_t msduHandle)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_McpsPurgeReq,
                            msduHandle));
}

/*!
 This function sends an associate request to a coordinator
 device.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeAssociateReq(ApiMac_mlmeAssociateReq_t *pData)
{
    return ((ApiMac_status_t)
    icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                    (uint32_t)MAC_MlmeAssociateReq, pData));
}

/*!
 This function sends an associate response to a device
 requesting to associate.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeAssociateRsp(ApiMac_mlmeAssociateRsp_t *pData)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeAssociateRsp, pData));
}

/*!
 This function is used by an associated device to notify the
 coordinator of its intent to leave the PAN.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeDisassociateReq(ApiMac_mlmeDisassociateReq_t *pData)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeDisassociateReq, pData));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqBool(ApiMac_attribute_bool_t pibAttribute,
bool *pValue)
{
    return ((ApiMac_status_t) icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqUint8(ApiMac_attribute_uint8_t pibAttribute,
                                       uint8_t *pValue)
{
    return ((ApiMac_status_t) icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqUint16(ApiMac_attribute_uint16_t pibAttribute,
                                        uint16_t *pValue)
{
    return ((ApiMac_status_t) icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqUint32(ApiMac_attribute_uint32_t pibAttribute,
                                        uint32_t *pValue)
{
    return ((ApiMac_status_t) icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqArray(ApiMac_attribute_array_t pibAttribute,
                                       uint8_t *pValue)
{
    return ((ApiMac_status_t) icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetReq,
                                             pibAttribute, pValue));
}

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 * @param       pLen - pointer to the read length
 *
 * @return      The status of the request
 */
ApiMac_status_t ApiMac_mlmeGetReqArrayLen(ApiMac_attribute_array_t pibAttribute,
                                          uint8_t *pValue,
                                          uint16_t *pLen)
{
    if (NULL != pLen)
    {
        *pLen = icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                (uint32_t)MAC_MlmeGetReqSize,
                                pibAttribute);
    }
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC frequency Hopping PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetFhReqUint8(
                ApiMac_FHAttribute_uint8_t pibAttribute, uint8_t *pValue)
{
    return (mlmeGetFhReq((uint16_t)pibAttribute, (void *)pValue, NULL));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC frequency Hopping PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetFhReqUint16(
                ApiMac_FHAttribute_uint16_t pibAttribute, uint16_t *pValue)
{
    return (mlmeGetFhReq((uint16_t)pibAttribute, (void *)pValue, NULL));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC frequency Hopping PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetFhReqUint32(
                ApiMac_FHAttribute_uint32_t pibAttribute, uint32_t *pValue)
{
    return (mlmeGetFhReq((uint16_t)pibAttribute, (void *)pValue, NULL));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC frequency Hopping PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetFhReqArray(
                ApiMac_FHAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return (mlmeGetFhReq((uint16_t)pibAttribute, (void *)pValue, NULL));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC frequency Hopping PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetFhReqArrayLen(
                ApiMac_FHAttribute_array_t pibAttribute,
                uint8_t *pValue,
                uint16_t *pLen)
{
    return (mlmeGetFhReq((uint16_t)pibAttribute, (void *)pValue, pLen));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqUint8(
                ApiMac_securityAttribute_uint8_t pibAttribute, uint8_t *pValue)
{
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetSecurityReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqUint16(
                ApiMac_securityAttribute_uint16_t pibAttribute,
                uint16_t *pValue)
{
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetSecurityReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqArray(
                ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetSecurityReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqArrayLen(
                ApiMac_securityAttribute_array_t pibAttribute,
                uint8_t *pValue,
                uint16_t *pLen
                )
{
    if (NULL != pLen)
    {
        *pLen = icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                (uint32_t)MAC_MlmeGetSecurityReqSize,
                                pibAttribute);
    }

    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                             (uint32_t)MAC_MlmeGetSecurityReq,
                                             pibAttribute, pValue));
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqStruct(
                ApiMac_securityAttribute_struct_t pibAttribute, void *pValue)
{
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                            (uint32_t)MAC_MlmeGetSecurityReq,
                                             pibAttribute, pValue));
}

/*!
 This function is called in response to an orphan notification
 from a peer device.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeOrphanRsp(ApiMac_mlmeOrphanRsp_t *pData)
{
    return ((ApiMac_status_t)
    icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                    (uint32_t)MAC_MlmeOrphanRsp, pData));
}

/*!
 This function is used to request pending data from the coordinator.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmePollReq(ApiMac_mlmePollReq_t *pData)
{
    return ((ApiMac_status_t)
    icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                    (uint32_t)MAC_MlmePollReq, pData));
}

/*!
 This function must be called once at system startup before any other
 function in the management API is called.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeResetReq(bool setDefaultPib)
{
    return ((ApiMac_status_t)
    icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                    (uint32_t)MAC_MlmeResetReq, setDefaultPib));
}

/*!
 This function initiates an energy detect, active, passive, or
 orphan scan on one or more channels.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeScanReq(ApiMac_mlmeScanReq_t *pData)
{
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeScanReq, pData));
#else
    return (ApiMac_status_unsupported);
#endif /* FEATURE_BEACON_MODE || FEATURE_NON_BEACON_MODE */
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqBool(ApiMac_attribute_bool_t pibAttribute,
bool value)
{
    return ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                           (uint32_t)MAC_MlmeSetReq,
                           pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqUint8(ApiMac_attribute_uint8_t pibAttribute,
                                       uint8_t value)
{
    return ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                           (uint32_t)MAC_MlmeSetReq,
                           pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqUint16(ApiMac_attribute_uint16_t pibAttribute,
                                        uint16_t value)
{
    return ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                           (uint32_t)MAC_MlmeSetReq,
                           pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqUint32(ApiMac_attribute_uint32_t pibAttribute,
                                        uint32_t value)
{
    return ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                           (uint32_t)MAC_MlmeSetReq,
                           pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqArray(ApiMac_attribute_array_t pibAttribute,
                                       uint8_t *pValue)
{
    return ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                           (uint32_t)MAC_MlmeSetReq,
                           pibAttribute, pValue));
}

/*!
 This direct execute function sets a frequency hopping attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetFhReqUint8(
                ApiMac_FHAttribute_uint8_t pibAttribute, uint8_t value)
{
    return (mlmeSetFhReq((uint16_t)pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets a frequency hopping attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetFhReqUint16(
                ApiMac_FHAttribute_uint16_t pibAttribute, uint16_t value)
{
    return (mlmeSetFhReq((uint16_t)pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets a frequency hopping attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetFhReqUint32(
                ApiMac_FHAttribute_uint32_t pibAttribute, uint32_t value)
{
    return (mlmeSetFhReq((uint16_t)pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets a frequency hopping attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetFhReqArray(
                ApiMac_FHAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return (mlmeSetFhReq((uint16_t)pibAttribute, (void *)pValue));
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqUint8(
                ApiMac_securityAttribute_uint8_t pibAttribute, uint8_t value)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeSetSecurityReq,
                            pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqUint16(
                ApiMac_securityAttribute_uint16_t pibAttribute, uint16_t value)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeSetSecurityReq,
                            pibAttribute, (void *)&value));
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqArray(
                ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                           (uint32_t)MAC_MlmeSetSecurityReq,
                           pibAttribute, (void *)pValue));
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqStruct(
                ApiMac_securityAttribute_struct_t pibAttribute, void *pValue)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeSetSecurityReq,
                            pibAttribute, (void *)pValue));
}

/*!
 This function is called by a coordinator or PAN coordinator
 to start or reconfigure a network.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeStartReq(ApiMac_mlmeStartReq_t *pData)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeStartReq, pData));
}

/*!
 This function requests the MAC to synchronize with the
 coordinator by acquiring and optionally tracking its beacons.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSyncReq(ApiMac_mlmeSyncReq_t *pData)
{
    return ((ApiMac_status_t)
              icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                              (uint32_t)MAC_MlmeSyncReq, pData));
}

/*!
 This function returns a random byte from the MAC random number
 generator.

 Public function defined in api_mac.h
 */
uint8_t ApiMac_randomByte(void)
{
    return (icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_RandomByte));
}

/*!
 Update Device Table entry and PIB with new Pan Id.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_updatePanId(uint16_t panId)
{
    ApiMac_status_t ret = ApiMac_status_noResources;
#ifdef FEATURE_MAC_SECURITY
    ret = ((ApiMac_status_t)
           icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                    (uint32_t)macUpdatePanId, panId));
    ret = ApiMac_status_success;
#endif  /* FEATURE_MAC_SECURITY */
    return ret;
}

#if defined(COMBO_MAC) || defined(FREQ_2_4G)
/*!
 Enable source match for auto ack and pending.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_srcMatchEnable(void)
{
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_SrcMatchEnable));
}
#endif

/*!
 This functions handles the WiSUN async request.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeWSAsyncReq(ApiMac_mlmeWSAsyncReq_t* pData)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeWSAsyncReq, pData));
}

/*!
 This function start the Frequency hopping operation.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_startFH(void)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,(uint32_t)MAC_StartFH));
}

/*!
 Enables the Frequency hopping operation.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_enableFH(void)
{
    return ((ApiMac_status_t)
    icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,(uint32_t)MAC_EnableFH));
}

/*!
 Parses the payload information elements.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_parsePayloadGroupIEs(uint8_t *pPayload,
                                            uint16_t payloadLen,
                                            ApiMac_payloadIeRec_t **pList)
{
    return (parsePayloadIEs(pPayload, payloadLen, pList, true));
}

/*!
 Parses the payload Sub Information Elements.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_parsePayloadSubIEs(uint8_t *pContent,
                                          uint16_t contentLen,
                                          ApiMac_payloadIeRec_t **pList)
{
    return (parsePayloadIEs(pContent, contentLen, pList, false));
}

/*!
 Free memory allocated by ApiMac.

 Public function defined in api_mac.h
 */
void ApiMac_freeIEList(ApiMac_payloadIeRec_t *pList)
{
    /* Loop through the list */
    while(pList)
    {
        ApiMac_payloadIeRec_t *pTmp = pList;

        /* Move to the next item in the list */
        pList = pTmp->pNext;

        /* free the current item */
        ICall_free(pTmp);
    }
}

/*!
 Convert ApiMac_capabilityInfo_t data type to uint8_t capInfo

 Public function defined in api_mac.h
 */
uint8_t ApiMac_convertCapabilityInfo(ApiMac_capabilityInfo_t *pMsgcapInfo)
{
    uint8 capInfo = 0;

    if(pMsgcapInfo->panCoord)
    {
        capInfo |= CAPABLE_PAN_COORD;
    }

    if(pMsgcapInfo->ffd)
    {
        capInfo |= CAPABLE_FFD;
    }

    if(pMsgcapInfo->mainsPower)
    {
        capInfo |= CAPABLE_MAINS_POWER;
    }

    if(pMsgcapInfo->rxOnWhenIdle)
    {
        capInfo |= CAPABLE_RX_ON_IDLE;
    }

    if(pMsgcapInfo->security)
    {
        capInfo |= CAPABLE_SECURITY;
    }

    if(pMsgcapInfo->allocAddr)
    {
        capInfo |= CAPABLE_ALLOC_ADDR;
    }

    return (capInfo);
}

/*!
 Convert from bitmask byte to API MAC capInfo

 Public function defined in api_mac.h
 */
void ApiMac_buildMsgCapInfo(uint8_t cInfo, ApiMac_capabilityInfo_t *pPBcapInfo)
{
    if(cInfo & CAPABLE_PAN_COORD)
    {
        pPBcapInfo->panCoord = 1;
    }

    if(cInfo & CAPABLE_FFD)
    {
        pPBcapInfo->ffd = 1;
    }

    if(cInfo & CAPABLE_MAINS_POWER)
    {
        pPBcapInfo->mainsPower = 1;
    }

    if(cInfo & CAPABLE_RX_ON_IDLE)
    {
        pPBcapInfo->rxOnWhenIdle = 1;
    }

    if(cInfo & CAPABLE_SECURITY)
    {
        pPBcapInfo->security = 1;
    }

    if(cInfo & CAPABLE_ALLOC_ADDR)
    {
        pPBcapInfo->allocAddr = 1;
    }
}
#ifdef FEATURE_MAC_SECURITY
/*!
 Adds a new MAC device table entry.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secAddDevice(ApiMac_secAddDevice_t *pAddDevice)
{
    return ((ApiMac_status_t)
              icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                              (uint32_t)macWrapperAddDevice, pAddDevice));
}

/*!
 Removes MAC device table entries.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secDeleteDevice(ApiMac_sAddrExt_t *pExtAddr)
{
    return ((ApiMac_status_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                    (uint32_t)macWrapperDeleteDevice, pExtAddr));
}

/*!
 Removes the key at the specified key Index and default keySrc
 removes all MAC device table
 entries associated with it.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secDeleteKeyAndAssocDevices(uint8_t keyIndex)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)macWrapperDeleteKeyAndAssociatedDevices,
                            keyIndex));
}

/*!
 Removes all MAC device table entries

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secDeleteAllDevices(void)
{
    return ((ApiMac_status_t)
             icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                             (uint32_t)macWrapperDeleteAllDevices));
}

/*!
 Reads the frame counter value associated with a MAC security key indexed
 by the designated key identifier and the default key source

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secGetDefaultSourceKey(uint8_t keyId,
                                              uint32_t *pFrameCounter)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)macWrapperGetDefaultSourceKey,
                            keyId, pFrameCounter));
}

/*!
 Adds the MAC security key, adds the associated lookup list for the key,
 initializes the frame counter to the value provided

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secAddKeyInitFrameCounter(
                ApiMac_secAddKeyInitFrameCounter_t *pInfo)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)macWrapperAddKeyInitFCtr,
                            pInfo));
}
#endif
/******************************************************************************
 Local Functions
 *****************************************************************************/

/*!
 * @brief       Generic function to get an FH attribute
 *
 * @param       pibAttribute - attribute to get
 * @param       pValue - pointer to put the attribute value
 * @param       pLen - pointer to place to put length
 *
 * @return      status result
 */
static ApiMac_status_t mlmeGetFhReq(uint16_t pibAttribute, void *pValue,
                                    uint16_t *pLen)
{
    if (NULL != pLen )
    {
        *pLen = (uint16_t)icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                                (uint32_t)MAC_MlmeFHGetReqSize, pibAttribute);
    }

    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeFHGetReq, pibAttribute, pValue));
}

/*!
 * @brief       This function process incoming ICall callback messages.
 *
 * @param       pMsg - pointer to the incoming message
 */
static uint16_t processIncomingICallMsg(macCbackEvent_t *pMsg)
{
    /* Only process message if callbacks are setup */
    if(pMacCallbacks != NULL)
    {
#ifdef DBG_APP
      appDbg_event[appDbg_eidx] = (pMsg->hdr.event << 8)|pMsg->hdr.status;
      appDbg_eidx = (appDbg_eidx + 1) % 20;
#endif
        /* Determine the callback type */
        switch(pMsg->hdr.event)
        {
            case MAC_MLME_ASSOCIATE_IND:
                if(pMacCallbacks->pAssocIndCb)
                {
                    /* Indication structure */
                    ApiMac_mlmeAssociateInd_t ind;

                    /* Initialize the structure */
                    memset(&ind, 0, sizeof(ApiMac_mlmeAssociateInd_t));

                    /* copy the message to the indication structure */
                    memcpy(ind.deviceAddress, pMsg->associateInd.deviceAddress,
                           sizeof(ApiMac_sAddrExt_t));
                    ApiMac_buildMsgCapInfo(
                                    pMsg->associateInd.capabilityInformation,
                                    &(ind.capabilityInformation));
                    memcpy(&(ind.sec),&(pMsg->associateInd.sec),
                           sizeof(ApiMac_sec_t));

                    /* Initiate the callback */
                    pMacCallbacks->pAssocIndCb(&ind);
                }
                break;

            case MAC_MLME_ASSOCIATE_CNF:
                if(pMacCallbacks->pAssocCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mlmeAssociateCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mlmeAssociateCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status =
                        (ApiMac_assocStatus_t)pMsg->associateCnf.hdr
                                                                    .status;
                    cnf.assocShortAddress = (uint16_t)pMsg->associateCnf
                                    .assocShortAddress;
                    memcpy(&(cnf.sec),&(pMsg->associateCnf.sec),
                           sizeof(ApiMac_sec_t));

                    /* Initiate the callback */
                    pMacCallbacks->pAssocCnfCb(&cnf);
                }
                break;

            case MAC_MLME_DISASSOCIATE_IND:
                if(pMacCallbacks->pDisassociateIndCb)
                {
                    /* Indication structure */
                    ApiMac_mlmeDisassociateInd_t ind;

                    /* Initialize the structure */
                    memset(&ind, 0, sizeof(ApiMac_mlmeDisassociateInd_t));

                    /* copy the message to the indication structure */
                    memcpy(ind.deviceAddress,
                           pMsg->disassociateInd.deviceAddress,
                           sizeof(ApiMac_sAddrExt_t));
                    ind.disassociateReason = (ApiMac_disassocateReason_t)pMsg
                                    ->disassociateInd.disassociateReason;
                    memcpy(&(ind.sec),&(pMsg->disassociateInd.sec),
                           sizeof(ApiMac_sec_t));

                    /* Initiate the callback */
                    pMacCallbacks->pDisassociateIndCb(&ind);
                }
                break;

            case MAC_MLME_DISASSOCIATE_CNF:
                if(pMacCallbacks->pDisassociateCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mlmeDisassociateCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mlmeDisassociateCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status = (ApiMac_status_t)pMsg->disassociateCnf.hdr
                                    .status;
                    copyMacAddrToApiMacAddr(
                                    &(cnf.deviceAddress),
                                    &(pMsg->disassociateCnf.deviceAddress));
                    cnf.panId = (uint16_t)pMsg->disassociateCnf.panId;

                    /* Initiate the callback */
                    pMacCallbacks->pDisassociateCnfCb(&cnf);
                }
                break;

            case MAC_MLME_BEACON_NOTIFY_IND:
                if(pMacCallbacks->pBeaconNotifyIndCb)
                {
                    processBeaconNotifyInd(&(pMsg->beaconNotifyInd));
                }
                break;

            case MAC_MLME_ORPHAN_IND:
                if(pMacCallbacks->pOrphanIndCb)
                {
                    /* Indication structure */
                    ApiMac_mlmeOrphanInd_t ind;

                    /* Initialize the structure */
                    memset(&ind, 0, sizeof(ApiMac_mlmeOrphanInd_t));

                    /* copy the message to the indication structure */
                    memcpy(ind.orphanAddress, pMsg->orphanInd.orphanAddress,
                           sizeof(ApiMac_sAddrExt_t));
                    memcpy(&(ind.sec), &(pMsg->orphanInd.sec),
                           sizeof(ApiMac_sec_t));

                    /* Initiate the callback */
                    pMacCallbacks->pOrphanIndCb(&ind);
                }
                break;

            case MAC_MLME_SCAN_CNF:
                if(pMacCallbacks->pScanCnfCb)
                {
                    processScanCnf(&(pMsg->scanCnf));
                }


                if(pMsg->scanCnf.scanType == ApiMac_scantype_energyDetect)
                {
                    if( pMsg->scanCnf.result.pEnergyDetect != NULL)
                    {
                        ICall_free(pMsg->scanCnf.result.pEnergyDetect);
                    }
                }
                else
                {
                    if( pMsg->scanCnf.result.pPanDescriptor != NULL)
                    {
                        ICall_free(pMsg->scanCnf.result.pPanDescriptor);
                    }
                }

                break;

            case MAC_MLME_START_CNF:
                if(pMacCallbacks->pStartCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mlmeStartCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mlmeStartCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status = (ApiMac_status_t)pMsg->startCnf.hdr.status;

                    /* Initiate the callback */
                    pMacCallbacks->pStartCnfCb(&cnf);
                }
                break;

            case MAC_MLME_SYNC_LOSS_IND:
                if(pMacCallbacks->pSyncLossIndCb)
                {
                    /* Indication structure */
                    ApiMac_mlmeSyncLossInd_t ind;

                    /* Initialize the structure */
                    memset(&ind, 0, sizeof(ApiMac_mlmeSyncLossInd_t));

                    /* copy the message to the indication structure */
                    ind.reason = (ApiMac_status_t)pMsg->syncLossInd.hdr.status;
                    ind.panId = pMsg->syncLossInd.panId;
                    ind.logicalChannel = pMsg->syncLossInd.logicalChannel;
                    ind.channelPage = pMsg->syncLossInd.channelPage;
                    ind.phyID = pMsg->syncLossInd.phyID;
                    memcpy(&(ind.sec), &(pMsg->syncLossInd.sec),
                           sizeof(ApiMac_sec_t));

                    /* Initiate the callback */
                    pMacCallbacks->pSyncLossIndCb(&ind);
                }
                break;

            case MAC_MLME_POLL_CNF:
                if(pMacCallbacks->pPollCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mlmePollCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mlmePollCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status = (ApiMac_status_t)pMsg->pollCnf.hdr.status;
                    cnf.framePending = pMsg->pollCnf.framePending;

                    /* Initiate the callback */
                    pMacCallbacks->pPollCnfCb(&cnf);
                }
                break;

            case MAC_MLME_POLL_IND:
                if(pMacCallbacks->pPollIndCb)
                {
                    /* Indication structure */
                    ApiMac_mlmePollInd_t ind;

                    /* Initialize the structure */
                    memset(&ind, 0, sizeof(ApiMac_mlmePollInd_t));

                    /* copy the message to the indication structure */
                    copyMacAddrToApiMacAddr(&(ind.srcAddr),
                                            &(pMsg->pollInd.srcAddr));
                    ind.srcPanId = pMsg->pollInd.srcPanId;
                    ind.noRsp = pMsg->pollInd.noRsp;

                    /* Initiate the callback */
                    pMacCallbacks->pPollIndCb(&ind);
                }
                break;

            case MAC_MLME_COMM_STATUS_IND:
                if(pMacCallbacks->pCommStatusCb)
                {
                    /* Indication structure */
                    ApiMac_mlmeCommStatusInd_t ind;

                    /* Initialize the structure */
                    memset(&ind, 0, sizeof(ApiMac_mlmeCommStatusInd_t));

                    /* copy the message to the indication structure */
                    ind.status = (ApiMac_status_t)pMsg->hdr.status;
                    copyMacAddrToApiMacAddr(&(ind.srcAddr),
                                            &(pMsg->commStatusInd.srcAddr));
                    copyMacAddrToApiMacAddr(&(ind.dstAddr),
                                            &(pMsg->commStatusInd.dstAddr));
                    ind.panId = (uint16_t)pMsg->commStatusInd.panId;
                    ind.reason = (ApiMac_commStatusReason_t)pMsg->commStatusInd
                                    .reason;
                    memcpy(&(ind.sec),&(pMsg->commStatusInd.sec),
                           sizeof(ApiMac_sec_t));

                    /* Initiate the callback */
                    pMacCallbacks->pCommStatusCb(&ind);
                }
                break;

            case MAC_MCPS_DATA_CNF:
                if(pMacCallbacks->pDataCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mcpsDataCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mcpsDataCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status = (ApiMac_status_t)pMsg->dataCnf.hdr.status;
                    cnf.msduHandle = pMsg->dataCnf.msduHandle;
                    cnf.timestamp = pMsg->dataCnf.timestamp;
                    cnf.timestamp2 = pMsg->dataCnf.timestamp2;
                    cnf.retries = pMsg->dataCnf.retries;
                    cnf.mpduLinkQuality = pMsg->dataCnf.mpduLinkQuality;
                    cnf.correlation = pMsg->dataCnf.correlation;
                    cnf.rssi = pMsg->dataCnf.rssi;
                    cnf.frameCntr = pMsg->dataCnf.frameCntr;

                    /* Initiate the callback */
                    pMacCallbacks->pDataCnfCb(&cnf);
                }

                if(pMsg->dataCnf.pDataReq)
                {
                    /* Deallocate the original data request structure */
                    macMsgDeallocate((macEventHdr_t *)pMsg->dataCnf.pDataReq);
                }
                break;

            case MAC_MCPS_DATA_IND:
                if(pMacCallbacks->pDataIndCb)
                {
                    /* Indication structure */
                    ApiMac_mcpsDataInd_t ind;

                    /* copy structure to structure */
                    copyDataInd(&ind, &(pMsg->dataInd));

                    /* Initiate the callback */
                    pMacCallbacks->pDataIndCb(&ind);
                }
                break;

            case MAC_MCPS_PURGE_CNF:
                if(pMacCallbacks->pPurgeCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mcpsPurgeCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mcpsPurgeCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status = (ApiMac_status_t)pMsg->purgeCnf.hdr.status;
                    cnf.msduHandle = pMsg->purgeCnf.msduHandle;

                    /* Initiate the callback */
                    pMacCallbacks->pPurgeCnfCb(&cnf);
                }
                break;

            case MAC_MLME_WS_ASYNC_IND:
                if(pMacCallbacks->pWsAsyncIndCb)
                {
                    /* Indication structure */
                    ApiMac_mlmeWsAsyncInd_t ind;

                    /* copy structure to structure */
                    copyDataInd((ApiMac_mcpsDataInd_t *)&ind,
                                (macMcpsDataInd_t *)&(pMsg->asyncInd));

                    /* Initiate the callback */
                    pMacCallbacks->pWsAsyncIndCb(&ind);
                }
                break;

            case MAC_MLME_WS_ASYNC_CNF:
                if(pMacCallbacks->pWsAsyncCnfCb)
                {
                    /* Confirmation structure */
                    ApiMac_mlmeWsAsyncCnf_t cnf;

                    /* Initialize the structure */
                    memset(&cnf, 0, sizeof(ApiMac_mlmeWsAsyncCnf_t));

                    /* copy the message to the confirmation structure */
                    cnf.status = (ApiMac_status_t)pMsg->asyncCnf.hdr.status;

                    /* Initiate the callback */
                    pMacCallbacks->pWsAsyncCnfCb(&cnf);
                }
                break;

            case MAC_NCP_MT_MSG:
                if(pMacCallbacks->pUnprocessedCb)
                {
                    /* NPI MT MSG */
                    uint8_t *pNpiMtMsg;

                    pNpiMtMsg = (uint8_t *)pMsg + sizeof(macNpiMtMsg_t);

                    pMacCallbacks->pUnprocessedCb(0, 0,(void *)pNpiMtMsg);

                }
                break;

            default:
                break;
        }
    }
    return (0);
}

/*!
 * @brief       Copy the common address type from Mac Stack type to App type.
 *
 * @param       pDst - pointer to the application type
 * @param       pSrc - pointer to the mac stack type
 */
static void copyMacAddrToApiMacAddr(ApiMac_sAddr_t *pDst, sAddr_t *pSrc)
{
    /* Copy each element of the structure */
    pDst->addrMode = (ApiMac_addrType_t)pSrc->addrMode;
    if(pDst->addrMode == ApiMac_addrType_short)
    {
        pDst->addr.shortAddr = pSrc->addr.shortAddr;
    }
    else
    {
        memcpy(pDst->addr.extAddr, pSrc->addr.extAddr,
               sizeof(ApiMac_sAddrExt_t));
    }
}

/*!
 * @brief       Copy the common address type from Mac Stack type to App type.
 *
 * @param       pDst - pointer to the application type
 * @param       pSrc - pointer to the mac stack type
 */
static void copyMacPanDescToApiMacPanDesc(ApiMac_panDesc_t *pDst,
                                          macPanDesc_t *pSrc)
{
    /* Copy each element of the structure */
    copyMacAddrToApiMacAddr(&(pDst->coordAddress), &(pSrc->coordAddress));
    pDst->coordPanId = (uint16_t)pSrc->coordPanId;
    pDst->superframeSpec = (uint16_t)pSrc->superframeSpec;
    pDst->logicalChannel = (uint8_t)pSrc->logicalChannel;
    pDst->channelPage = (uint8_t)pSrc->channelPage;
    pDst->gtsPermit = (bool)pSrc->gtsPermit;
    pDst->linkQuality = (uint8_t)pSrc->linkQuality;
    pDst->timestamp = (uint32_t)pSrc->timestamp;
    pDst->securityFailure = (bool)pSrc->securityFailure;
    memcpy(&(pDst->sec), &(pSrc->sec), sizeof(ApiMac_sec_t));
}

/*!
 * @brief       Process the incoming Beacon Notification callback.
 *
 * @param       pInd - pointer MAC Beacon indication info
 */
static void processBeaconNotifyInd(macMlmeBeaconNotifyInd_t *pInd)
{
    /* Indication structure */
    ApiMac_mlmeBeaconNotifyInd_t ind;

    /* Initialize the structure */
    memset(&ind, 0, sizeof(ApiMac_mlmeBeaconNotifyInd_t));

    /* copy the message to the indication structure */
    ind.beaconType = (ApiMac_beaconType_t)pInd->beaconType;
    ind.bsn = pInd->bsn;

    if(ind.beaconType == ApiMac_beaconType_normal)
    {
        uint8_t *pAddrList;

        /* Fill in the PAN descriptor */
        if(pInd->info.beaconData.pPanDesc)
        {
            copyMacPanDescToApiMacPanDesc(&ind.panDesc,
                                          pInd->info.beaconData.pPanDesc);
        }

        /* Add the pending address lists for short address and extended address */
        pAddrList = pInd->info.beaconData.pAddrList;
        ind.beaconData.beacon.numPendShortAddr = MAC_PEND_NUM_SHORT(
                        pInd->info.beaconData.pendAddrSpec);
        ind.beaconData.beacon.numPendExtAddr = MAC_PEND_NUM_EXT(
                        pInd->info.beaconData.pendAddrSpec);
        if(ind.beaconData.beacon.numPendShortAddr)
        {
            ind.beaconData.beacon.pShortAddrList = (uint16_t *)pAddrList;
            pAddrList += ind.beaconData.beacon.numPendShortAddr * 2;
        }
        if(ind.beaconData.beacon.numPendExtAddr)
        {
            ind.beaconData.beacon.pExtAddrList = (uint8_t *)pAddrList;
        }

        /* Add the beacon payload */
        ind.beaconData.beacon.sduLength = pInd->info.beaconData.sduLength;
        ind.beaconData.beacon.pSdu = pInd->info.beaconData.pSdu;
    }
    else
    {
        /* Fill in the PAN descriptor */
        if(pInd->info.eBeaconData.pPanDesc)
        {
            copyMacPanDescToApiMacPanDesc(&ind.panDesc,
                                          pInd->info.eBeaconData.pPanDesc);
        }

        /* Must be an enhanced beacon */
        ind.beaconData.eBeacon.coexist.beaconOrder = pInd->info.eBeaconData
                        .coexist.beaconOrder;
        ind.beaconData.eBeacon.coexist.superFrameOrder = pInd->info.eBeaconData
                        .coexist.superFrameOrder;
        ind.beaconData.eBeacon.coexist.finalCapSlot = pInd->info.eBeaconData
                        .coexist.finalCapSlot;
        ind.beaconData.eBeacon.coexist.eBeaconOrder = pInd->info.eBeaconData
                        .coexist.eBeaconOrder;
        ind.beaconData.eBeacon.coexist.offsetTimeSlot = pInd->info.eBeaconData
                        .coexist.offsetTimeSlot;
        ind.beaconData.eBeacon.coexist.capBackOff = pInd->info.eBeaconData
                        .coexist.capBackOff;
        ind.beaconData.eBeacon.coexist.eBeaconOrderNBPAN = pInd->info
                        .eBeaconData.coexist.eBeaconOrderNBPAN;
    }

    /*
     * Initiate the callback, no need to check pMacCallbacks or the function
     * pointer for non-null, the calling function will check the function
     * pointer
     */
    pMacCallbacks->pBeaconNotifyIndCb(&ind);
}

/*!
 * @brief       Process the incoming Scan Confirm callback.
 *
 * @param       pCnf - pointer MAC Scan Confirm info
 */
static void processScanCnf(macMlmeScanCnf_t *pCnf)
{
    /* Confirmation structure */
    ApiMac_mlmeScanCnf_t cnf;

    /* Initialize the structure */
    memset(&cnf, 0, sizeof(ApiMac_mlmeScanCnf_t));

    /* copy the message to the confirmation structure */
    cnf.status = (ApiMac_status_t)pCnf->hdr.status;

    cnf.scanType = (ApiMac_scantype_t)pCnf->scanType;
    cnf.channelPage = pCnf->channelPage;
    cnf.phyId = pCnf->phyID;
    memcpy(cnf.unscannedChannels, pCnf->unscannedChannels,
    APIMAC_154G_CHANNEL_BITMAP_SIZ);
    cnf.resultListSize = pCnf->resultListSize;

    if(cnf.resultListSize)
    {
        if(cnf.scanType == ApiMac_scantype_energyDetect)
        {
            cnf.result.pEnergyDetect = pCnf->result.pEnergyDetect;

        }
        else
        {
            cnf.result.pPanDescriptor =
              (ApiMac_panDesc_t*)pCnf->result.pPanDescriptor;
        }
    }

    /*
     * Initiate the callback, no need to check pMacCallbacks or the function
     * pointer for non-null, the calling function will check the function
     * pointer
     */
    pMacCallbacks->pScanCnfCb(&cnf);
}

/*!
 * @brief       Deallocate message function, MAC will deallocate the message.
 *
 * @param       pData - pointer to message to deallocate.
 */
static void macMsgDeallocate(macEventHdr_t *pData)
{
    if(pData != NULL)
    {
        /* Fill in the message content */
        pData->event = MAC_MSG_DEALLOCATE;
        pData->status = 0;

        /* Send the message */
        ICall_sendServiceMsg(ApiMac_appEntity, ICALL_SERVICE_CLASS_TIMAC,
                             (ICALL_MSG_FORMAT_KEEP),
                             pData);
    }
}

/*!
 * @brief       Copy the MAC data indication to the API MAC data indication
 *
 * @param       pDst - pointer to the API MAC data indication
 * @param       pSrc - pointer to the MAC data indication
 */
static void copyDataInd(ApiMac_mcpsDataInd_t *pDst, macMcpsDataInd_t *pSrc)
{
    /* Initialize the structure */
    memset(pDst, 0, sizeof(ApiMac_mcpsDataInd_t));

    /* copy the message to the indication structure */
    copyMacAddrToApiMacAddr(&(pDst->srcAddr), &(pSrc->mac.srcAddr));
    copyMacAddrToApiMacAddr(&(pDst->dstAddr), &(pSrc->mac.dstAddr));
    pDst->timestamp = pSrc->mac.timestamp;
    pDst->timestamp2 = pSrc->mac.timestamp2;
    pDst->srcPanId = pSrc->mac.srcPanId;
    pDst->dstPanId = pSrc->mac.dstPanId;
    pDst->mpduLinkQuality = pSrc->mac.mpduLinkQuality;
    pDst->correlation = pSrc->mac.correlation;
    pDst->rssi = pSrc->mac.rssi;
    pDst->dsn = pSrc->mac.dsn;
    pDst->payloadIeLen = pSrc->mac.payloadIeLen;
    pDst->pPayloadIE = pSrc->mac.pPayloadIE;
    pDst->fhFrameType = (ApiMac_fhFrameType_t)pSrc->internal.fhFrameType;
    pDst->fhProtoDispatch = (ApiMac_fhDispatchType_t)pSrc->mac.fhProtoDispatch;
    pDst->frameCntr = (uint32_t)pSrc->mac.frameCntr;
    memcpy(&(pDst->sec), &(pSrc->sec), sizeof(ApiMac_sec_t));

    /* Copy the payload information */
    pDst->msdu.len = pSrc->msdu.len;
    pDst->msdu.p = pSrc->msdu.p;
}

/*!
 * @brief       Copy the common address type from App type to Mac Stack type.
 *
 * @param       pDst - pointer to the mac stack type
 * @param       pSrc - pointer to the application type
 */
void copyApiMacAddrToMacAddr(sAddr_t *pDst, ApiMac_sAddr_t *pSrc)
{
    /* Copy each element of the structure */
    pDst->addrMode = pSrc->addrMode;
    if(pSrc->addrMode == ApiMac_addrType_short)
    {
        pDst->addr.shortAddr = pSrc->addr.shortAddr;
    }
    else
    {
        memcpy(pDst->addr.extAddr, pSrc->addr.extAddr, sizeof(sAddrExt_t));
    }
}

/*!
 * @brief Parses the payload information element.
 *
 * @param pPayload - pointer to the buffer with the payload IEs.
 * @param payloadLen - length of the buffer with the payload IEs.
 * @param pList - pointer to point of place to allocated the link list.
 * @param group - true to check for termination IE.
 *
 * @return      ApiMac_status_t
 */
static ApiMac_status_t parsePayloadIEs(uint8_t *pContent, uint16_t contentLen,
                                       ApiMac_payloadIeRec_t **pList,
                                       bool group)
{
    ApiMac_payloadIeRec_t* pIe = (ApiMac_payloadIeRec_t*) NULL;
    ApiMac_payloadIeRec_t* pTempIe;
    uint16_t lenContent = 0;
    ApiMac_status_t status = ApiMac_status_success;

    if((pContent == NULL) || (contentLen == 0))
    {
        return (ApiMac_status_noData);
    }

    /* Initialize the list pointer */
    *pList = (ApiMac_payloadIeRec_t*) NULL;

    while(lenContent < contentLen)
    {
        uint16_t hdr;
        bool typeLong;
        uint8_t ieId;

        hdr = MAKE_UINT16(pContent[0], pContent[1]);
        pContent += PAYLOAD_IE_HEADER_LEN; /* Move past the header */

        typeLong = GET_SUBIE_TYPE(hdr);
        if(typeLong)
        {
            ieId = GET_SUBIE_ID_LONG(hdr);
        }
        else
        {
            ieId = GET_SUBIE_ID_SHORT(hdr);
        }

        if(group)
        {
            if(!typeLong)
            {
                /* Only long IE types when parsing Group IEs */
                status = ApiMac_status_unsupported;
                break;
            }

            if(ApiMac_payloadIEGroup_term == ieId)
            {
                /* Termination IE found */
                break;
            }
        }

        pTempIe = (ApiMac_payloadIeRec_t *)ICall_malloc(
                        sizeof(ApiMac_payloadIeRec_t));

        if(pTempIe)
        {
            memset(pTempIe, 0, sizeof(ApiMac_payloadIeRec_t));

            /* If nothing in the list, add the node first otherwise
             add it to the end of the list */
            if(*pList == NULL)
            {
                *pList = pTempIe;
            }
            else
            {
                /* pIe should point to the previous node,
                 since it was allocated in the previous iteration */
                pIe->pNext = pTempIe;
            }

            pIe = pTempIe;
            pTempIe = NULL;

            /* Fill in the IE information */
            pIe->item.ieTypeLong = typeLong;
            pIe->item.ieId = ieId;

            if(pIe->item.ieTypeLong)
            {
                pIe->item.ieContentLen = GET_SUBIE_LEN_LONG(hdr);
            }
            else
            {
                pIe->item.ieContentLen = GET_SUBIE_LEN_SHORT(hdr);
            }
            pIe->item.pIEContent = pContent;

            /* Update length and pointer */
            lenContent += PAYLOAD_IE_HEADER_LEN + pIe->item.ieContentLen;
            pContent += pIe->item.ieContentLen;
        }
        else
        {
            status = ApiMac_status_noResources;
            break;
        }
    }

    if((status != ApiMac_status_success) && (NULL != *pList))
    {
        /* not successful in parsing all header ie's, free the linked list */
        pIe = *pList;
        while(NULL != pIe)
        {
            pTempIe = pIe->pNext;
            ICall_free(pIe);
            pIe = pTempIe;
        }
        *pList = NULL;
    }

    return (status);
}

/*!
 * @brief       Convert API txOptions to bitmasked txOptions.
 *
 * @param       txOptions - tx options structure
 *
 * @return      bitmasked txoptions
 */
uint16_t convertTxOptions(ApiMac_txOptions_t txOptions)
{
    uint16_t retVal = 0;

    if(txOptions.ack == true)
    {
        retVal |= MAC_TXOPTION_ACK;
    }
    if(txOptions.indirect == true)
    {
        retVal |= MAC_TXOPTION_INDIRECT;
    }
    if(txOptions.pendingBit == true)
    {
        retVal |= MAC_TXOPTION_PEND_BIT;
    }
    if(txOptions.noRetransmits == true)
    {
        retVal |= MAC_TXOPTION_NO_RETRANS;
    }
    if(txOptions.noConfirm == true)
    {
        retVal |= MAC_TXOPTION_NO_CNF;
    }
    if(txOptions.useAltBE == true)
    {
        retVal |= MAC_TXOPTION_ALT_BE;
    }
    if(txOptions.usePowerAndChannel == true)
    {
        retVal |= MAC_TXOPTION_PWR_CHAN;
    }
    if(txOptions.useGreenPower == true)
    {
        retVal |= MAC_TXOPTION_GREEN_PWR;
    }
    return (retVal);
}

/*!
 * @brief       Generic function to set an FH attribute
 *
 * @param       pibAttribute - attribute to set
 * @param       pValue - pointer to the attribute value
 *
 * @return      status result
 */
static ApiMac_status_t mlmeSetFhReq(uint16_t pibAttribute, void *pValue)
{
    return ((ApiMac_status_t)
            icall_directAPI(ICALL_SERVICE_CLASS_TIMAC,
                            (uint32_t)MAC_MlmeFHSetReq,
                            pibAttribute, pValue));
}

