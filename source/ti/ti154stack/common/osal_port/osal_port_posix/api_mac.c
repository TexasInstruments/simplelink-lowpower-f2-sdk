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
#include <stdlib.h>

#include "osal_port.h"
#include "api_mac.h"
#include "macTask.h"
#include "mac_util.h"
#include "macs.h"
#include "macwrapper.h"

#include "mac.h"

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#ifdef MAC_DUTY_CYCLE_CHECKING
#include "mac_duty_cycle/mac_duty_cycle.h"
#endif

#ifdef IEEE_COEX_ENABLED
#include DeviceFamily_constructPath(driverlib/rf_ieee_coex.h)
#endif

#ifdef FREERTOS_SUPPORT
/* POSIX Header files */
#include <FreeRTOS.h>
/* debug */
#include "mac_assert.h"
#endif

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
#define MAKE_UINT16(low,high) (((low)&0x00FF)|(((high)&0x00FF)<<8))

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

#ifdef MCUBOOT_ENABLE
#define MCUBOOT_HEADER_VER_ADDR_OFFSET 20
#define MCUBOOT_VERSION_PTR ((&__MCUBOOT_HDR_BASE) + MCUBOOT_HEADER_VER_ADDR_OFFSET)
#endif

/******************************************************************************
 Structures
 *****************************************************************************/

/******************************************************************************
 Global variables
 *****************************************************************************/

/* Linker file created symbol for addressing MCUBoot header */
#ifdef MCUBOOT_ENABLE
extern uint8_t __MCUBOOT_HDR_BASE;
#endif

/*!
 The ApiMac_extAddr is the MAC's IEEE address, setup with the Chip's
 IEEE addresses in main.c
 */
ApiMac_sAddrExt_t ApiMac_extAddr;

bool ApiMac_gpOffsetToms = false;
bool ApiMac_mtDataReqTxOptionGp = false;

#ifdef IEEE_COEX_ENABLED
extern rfc_ieeeCoExConfig_t coexConfig;
/**
 * Note that updating this bitmap alone does not apply the configurations. This is for coex config state tracking only.
 * coexConfig must also be updated accordingly
 */
uint16_t ieeeCoexBitmap = 0;
#endif

#ifdef COPROCESSOR
/*! Transmit Delay for Green Power*/
extern uint8_t GP_Offset;
#endif

/******************************************************************************
 Local variables
 *****************************************************************************/
static ApiMac_subAttribute_t subAttributeReadId = ApiMac_subAttribute_stopScan;

/*! Semaphore used to post events to the application thread */
static sem_t appSemHandle;

/*! Storage for Events flags */
uint32_t appEvents = 0;

/*! MAC callback table, initialized to no callback table */
static ApiMac_callbacks_t *pMacCallbacks = (ApiMac_callbacks_t *) NULL;

uint8_t appTaskId;

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
void *ApiMac_init(uint8_t macTaskIdParam, bool enableFH)
{
    int retc;


    /* create semaphores for messages / events
     */
    retc = sem_init(&appSemHandle, 0, 0);
    if (retc != 0) {
        while (1);
    }

    appTaskId = OsalPort_registerTask(pthread_self(), &appSemHandle, &appEvents);

    /* Allocate message buffer space */
    macStackInitParams_t *pMsg = (macStackInitParams_t *)OsalPort_msgAllocate(
                    sizeof(macStackInitParams_t));

    if(pMsg != NULL)
    {
        /* Fill in the message content */
        pMsg->hdr.event = MAC_STACK_INIT_PARAMS;
        pMsg->hdr.status = 0;
        pMsg->srctaskid = appTaskId;
        pMsg->retransmit = 0;
        pMsg->pendingMsg = 0;
        pMsg->pMacCbackQueryRetransmit = NULL;
        pMsg->pMacCbackCheckPending = NULL;

        OsalPort_msgSend( macTaskIdParam, (uint8_t*)pMsg );
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

#ifdef IEEE_COEX_ENABLED
    ApiMac_mlmeEnableCoex(true);
#endif

    return (&appSemHandle);
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
 Process incoming messages from the MAC

 Public function defined in api_mac.h
 */
void ApiMac_processIncoming(void)
{
    macCbackEvent_t *pMsg;

    /* Wait for response message */
    if( sem_wait(&appSemHandle) == 0)
    {
        /* Retrieve the response message */
        if( (pMsg = (macCbackEvent_t*) OsalPort_msgReceive( appTaskId )) != NULL)
        {
            /* Process the message from the MAC stack */
            processIncomingICallMsg(pMsg);
        }

        if(pMsg != NULL)
        {
            OsalPort_msgDeallocate((uint8_t*)pMsg);
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
    return (ApiMac_status_t) MAC_McpsDataReq(pData);
}

/*!
 This function purges and discards a data request from the MAC
 data queue.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mcpsPurgeReq(uint8_t msduHandle)
{
    return (ApiMac_status_t) MAC_McpsPurgeReq(msduHandle);
}

/*!
 This function sends an associate request to a coordinator
 device.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeAssociateReq(ApiMac_mlmeAssociateReq_t *pData)
{

    return (ApiMac_status_t) MAC_MlmeAssociateReq(pData);
}

/*!
 This function sends an associate response to a device
 requesting to associate.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeAssociateRsp(ApiMac_mlmeAssociateRsp_t *pData)
{
    return (ApiMac_status_t) MAC_MlmeAssociateRsp(pData);
}

/*!
 This function is used by an associated device to notify the
 coordinator of its intent to leave the PAN.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeDisassociateReq(ApiMac_mlmeDisassociateReq_t *pData)
{
    return (ApiMac_status_t) MAC_MlmeDisassociateReq(pData);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqBool(ApiMac_attribute_bool_t pibAttribute,
bool *pValue)
{
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleEnabled)
    {
        *pValue = dcEnabled;
        return ApiMac_status_success;
    }
#endif
    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqUint8(ApiMac_attribute_uint8_t pibAttribute,
                                       uint8_t *pValue)
{
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleStatus)
    {
        *pValue = dcStatus;
        return ApiMac_status_success;
    }
#endif
    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqUint16(ApiMac_attribute_uint16_t pibAttribute,
                                        uint16_t *pValue)
{
    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqUint32(ApiMac_attribute_uint32_t pibAttribute,
                                        uint32_t *pValue)
{
#ifdef MAC_OVERRIDE_TX_DELAY
    if (pibAttribute == ApiMac_attribute_minTxOffTime)
    {
        *pValue = minTxOffTime;
        return ApiMac_status_success;
    }
#endif
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute >= ApiMac_attribute_dutyCycleLimited && pibAttribute <= ApiMac_attribute_dutyCyclePtr)
    {
        if (pibAttribute == ApiMac_attribute_dutyCycleLimited)
        {
            *pValue = dcLimited;
        }
        if (pibAttribute == ApiMac_attribute_dutyCycleCritical)
        {
            *pValue = dcCritical;
        }
        if (pibAttribute == ApiMac_attribute_dutyCycleRegulated)
        {
            *pValue = dcRegulated;
        }
        if (pibAttribute == ApiMac_attribute_dutyCycleUsed)
        {
            *pValue = dcTimeUsed;
        }
        if (pibAttribute == ApiMac_attribute_dutyCyclePtr)
        {
            *pValue = dcPtr;
        }
        return ApiMac_status_success;
    }
#endif
    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqArray(ApiMac_attribute_array_t pibAttribute,
                                       uint8_t *pValue)
{
    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
}

/*!
 * @brief       This direct execute function retrieves an attribute value from
 *              the MAC PIB.
 *
 * @param       pibAttribute - The attribute identifier
 * @param       pValue - pointer to the attribute value
 * @param       pLen - pointer to the read length - Max size of MAX_PARAM_BUF_SIZE_MAC
 *
 * @return      The status of the request
 */
ApiMac_status_t ApiMac_mlmeGetReqArrayLen(ApiMac_attribute_array_t pibAttribute,
                                          uint8_t *pValue,
                                          uint16_t *pLen)
{
    mcuboot_image_version_t mcuboot_ver;

    if (pibAttribute == ApiMac_attribute_advancedSettings)
    {
        if (subAttributeReadId == ApiMac_subAttribute_getVersions)
        {
            *pLen = sizeof(mcuboot_image_version_t) + sizeof(ti154stack_version_t) +
                    sizeof(ti154stack_core_version_t);
#ifdef MCUBOOT_ENABLE
            memcpy(&mcuboot_ver, MCUBOOT_VERSION_PTR, sizeof(mcuboot_image_version_t));
#else
            memset(&mcuboot_ver, 0, sizeof(mcuboot_image_version_t));
#endif
            /* Returned value will be in the following format:
             * 8 byte MCUBoot version <1B major, 1B minor, 2B revision, 4B build num> +
             * 4 byte stack version <1B major, 1B minor, 1B revision, 1B reserved> +
             * 2 byte core stack version <1B major, 1B minor> */
            memcpy(pValue, &mcuboot_ver, sizeof(mcuboot_ver));
            pValue += sizeof(mcuboot_image_version_t);
            memcpy(pValue, &ti154stack_version, sizeof(ti154stack_version_t));
            pValue += sizeof(ti154stack_version_t);
            memcpy(pValue, &ti154stack_core_version, sizeof(ti154stack_core_version_t));
        }
#ifdef MAC_RADIO_USE_CSF
        else if(subAttributeReadId == ApiMac_subAttribute_getMacStatistics)
        {
            extern macStatisticsStruct_t threadMacStats;

            macStatisticsStruct_t combinedStatsBuffer;

            memcpy(&combinedStatsBuffer, &macStatistics, sizeof(macStatisticsStruct_t));

            combinedStatsBuffer.pta_lo_pri_req += threadMacStats.pta_lo_pri_req;
            combinedStatsBuffer.pta_hi_pri_req += threadMacStats.pta_hi_pri_req;
            combinedStatsBuffer.pta_lo_pri_denied += threadMacStats.pta_lo_pri_denied;
            combinedStatsBuffer.pta_hi_pri_denied += threadMacStats.pta_hi_pri_denied;
            combinedStatsBuffer.cca_retries += threadMacStats.cca_retries;
            combinedStatsBuffer.cca_failures += threadMacStats.cca_failures;
            combinedStatsBuffer.mac_tx_ucast_retry += threadMacStats.mac_tx_ucast_retry;
            combinedStatsBuffer.mac_tx_ucast_fail += threadMacStats.mac_tx_ucast_fail;

            /* Calculate MAC+Thread  denied rate */
            combinedStatsBuffer.pta_denied_rate =
                (((macStatistics.pta_hi_pri_denied + macStatistics.pta_lo_pri_denied +
                threadMacStats.pta_hi_pri_denied + threadMacStats.pta_lo_pri_denied) * 100) /
                (macStatistics.pta_hi_pri_req + macStatistics.pta_lo_pri_req +
                threadMacStats.pta_hi_pri_req + threadMacStats.pta_lo_pri_req));

            *pLen = sizeof(macStatisticsStruct_t);
            memcpy(pValue, &combinedStatsBuffer, sizeof(macStatisticsStruct_t));
        }
#else
        else if(subAttributeReadId == ApiMac_subAttribute_getMacStatistics)
        {
            *pLen = sizeof(macStatisticsStruct_t);
            memcpy(pValue, &macStatistics, sizeof(macStatisticsStruct_t));
        }
#endif
        else
        {
            return ApiMac_status_unsupportedAttribute;
        }
        return ApiMac_status_success;
    }
    /* Duty Cycle PIB Variable */
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleBucket)
    {
        memcpy((DC_NODE_ENTRY_s *)pValue, dcBucket, sizeof(DC_NODE_ENTRY_s) * DUTY_CYCLE_BUCKETS);
        return ApiMac_status_success;
    }
#endif
#ifdef IEEE_COEX_ENABLED
    if (pibAttribute == ApiMac_coexAttribute_coexConfig)
    {
        if (NULL != pLen)
		{
        	*pLen = sizeof(ieeeCoexBitmap);
		}
        memcpy(pValue, &ieeeCoexBitmap, sizeof(ieeeCoexBitmap));
        return ApiMac_status_success;
    }
#endif
    if (NULL != pLen)
    {
        *pLen = (uint32_t)MAC_MlmeGetReqSize(pibAttribute);
    }

    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Coex PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetCoexReqStruct(
                ApiMac_coexAttribute_struct_t pibAttribute, void *pValue)
{
#ifdef IEEE_COEX_ENABLED
    ((coexMetricsStruct_t *)pValue)->dbgCoexGrants = coexMetricsStruct.dbgCoexGrants;
    ((coexMetricsStruct_t *)pValue)->dbgCoexRejects = coexMetricsStruct.dbgCoexRejects;
    ((coexMetricsStruct_t *)pValue)->dbgCoexContRejects = coexMetricsStruct.dbgCoexContRejects;
    ((coexMetricsStruct_t *)pValue)->dbgCoexMaxContRejects = coexMetricsStruct.dbgCoexMaxContRejects;
#endif
    return ApiMac_status_success;
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Statistics PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetMacStatsReqStruct(
                ApiMac_macStatistics_struct_t pibAttribute, void *pValue)
{
#ifdef IEEE_COEX_ENABLED
    ((macStatisticsStruct_t *)pValue)->pta_lo_pri_req = macStatistics.pta_lo_pri_req;
    ((macStatisticsStruct_t *)pValue)->pta_hi_pri_req = macStatistics.pta_hi_pri_req;
    ((macStatisticsStruct_t *)pValue)->pta_lo_pri_denied = macStatistics.pta_lo_pri_denied;
    ((macStatisticsStruct_t *)pValue)->pta_hi_pri_denied = macStatistics.pta_hi_pri_denied;
    ((macStatisticsStruct_t *)pValue)->pta_denied_rate = macStatistics.pta_denied_rate;
#endif
    ((macStatisticsStruct_t *)pValue)->cca_retries = macStatistics.cca_retries;
    ((macStatisticsStruct_t *)pValue)->cca_failures = macStatistics.cca_failures;
    ((macStatisticsStruct_t *)pValue)->mac_tx_ucast_retry = macStatistics.mac_tx_ucast_retry;
    ((macStatisticsStruct_t *)pValue)->mac_tx_ucast_fail = macStatistics.mac_tx_ucast_fail;
    return ApiMac_status_success;
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
    return (ApiMac_status_t) MAC_MlmeGetSecurityReq(pibAttribute, pValue);
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
    return (ApiMac_status_t) MAC_MlmeGetSecurityReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqArray(
                ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return (ApiMac_status_t) MAC_MlmeGetSecurityReq(pibAttribute, pValue);
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
        *pLen = (uint16_t)MAC_MlmeGetSecurityReqSize(pibAttribute);
    }

    return (ApiMac_status_t)MAC_MlmeGetSecurityReq(pibAttribute, pValue);
}

/*!
 This direct execute function retrieves an attribute value from
 the MAC Secutity PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetSecurityReqStruct(
                ApiMac_securityAttribute_struct_t pibAttribute, void *pValue)
{
    return (ApiMac_status_t) MAC_MlmeGetSecurityReq(pibAttribute, pValue);
}

/*!
 This function is called in response to an orphan notification
 from a peer device.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeOrphanRsp(ApiMac_mlmeOrphanRsp_t *pData)
{
    return (ApiMac_status_t) MAC_MlmeOrphanRsp(pData);
}

/*!
 This function is used to request pending data from the coordinator.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmePollReq(ApiMac_mlmePollReq_t *pData)
{
    return (ApiMac_status_t) MAC_MlmePollReq(pData);
}

/*!
 This function must be called once at system startup before any other
 function in the management API is called.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeResetReq(bool setDefaultPib)
{
    return (ApiMac_status_t) MAC_MlmeResetReq(setDefaultPib);
}

/*!
 This function initiates an energy detect, active, passive, or
 orphan scan on one or more channels.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeScanReq(ApiMac_mlmeScanReq_t *pData)
{
#if defined (FEATURE_BEACON_MODE) || defined (FEATURE_NON_BEACON_MODE)
    return (ApiMac_status_t) MAC_MlmeScanReq(pData);
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
#ifdef MAC_OVERRIDE_TX_DELAY
    if (pibAttribute == ApiMac_attribute_customMinTxOffEnabled)
    {
        customMinTxOffEnabled = value;
        return ApiMac_status_success;
    }
#endif
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleEnabled)
    {
        dcEnabled = value;
        return ApiMac_status_success;
    }
#endif
    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, (void *)&value);
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqUint8(ApiMac_attribute_uint8_t pibAttribute,
                                       uint8_t value)
{
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleStatus)
    {
        dcStatus = value;
        return ApiMac_status_success;
    }
#endif
    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, (void *)&value);
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqUint16(ApiMac_attribute_uint16_t pibAttribute,
                                        uint16_t value)
{
    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, (void *)&value);
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqUint32(ApiMac_attribute_uint32_t pibAttribute,
                                        uint32_t value)
{
#ifdef MAC_OVERRIDE_TX_DELAY
    if (pibAttribute == ApiMac_attribute_minTxOffTime)
    {
        minTxOffTime = value;
        return ApiMac_status_success;
    }
#endif
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute >= ApiMac_attribute_dutyCycleLimited && pibAttribute <= ApiMac_attribute_dutyCyclePtr)
    {
        if (pibAttribute == ApiMac_attribute_dutyCycleLimited)
        {
            dcLimited = value;
        }
        if (pibAttribute == ApiMac_attribute_dutyCycleCritical)
        {
            dcCritical = value;
        }
        if (pibAttribute == ApiMac_attribute_dutyCycleRegulated)
        {
            dcRegulated = value;
        }
        if (pibAttribute == ApiMac_attribute_dutyCycleUsed)
        {
            dcTimeUsed = value;
        }
        if (pibAttribute == ApiMac_attribute_dutyCyclePtr)
        {
            dcPtr = value;
        }
        return ApiMac_status_success;
    }
#endif
    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, (void *)&value);
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqArray(ApiMac_attribute_array_t pibAttribute,
                                       uint8_t *pValue)
{
    uint8_t readAddrMode;
    sAddr_t addr;
    uint16_t panID;
    uint8_t option;

#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleBucket)
    {
        memcpy(dcBucket, (DC_NODE_ENTRY_s *)pValue, sizeof(DC_NODE_ENTRY_s) * DUTY_CYCLE_BUCKETS);
        return ApiMac_status_success;
    }
#endif
    if (pibAttribute == ApiMac_attribute_advancedSettings)
    {
        uint8_t subAttribute = pValue[0];
        uint8_t *value = pValue + 1;
        if (subAttribute == ApiMac_subAttribute_setReadId)
        {
            subAttributeReadId = (ApiMac_subAttribute_t) *value;
        }
        else if (subAttribute == ApiMac_subAttribute_stopScan)
        {
            MAC_AbortScan();
        }
        else if (subAttribute == ApiMac_subAttribute_gpOffsetToms)
        {
            // Set units of gpOffset to ms
            ApiMac_gpOffsetToms = true;
        }
        else if (subAttribute == ApiMac_subAttribute_mtDataReqTxOptionGp)
        {
            // Allow mt macDataReq to have txOption MAC_TXOPTION_GREEN_PWR
            ApiMac_mtDataReqTxOptionGp = true;
        }
        else if (subAttribute == ApiMac_subAttribute_srcMatchAddEntry ||
                 subAttribute == ApiMac_subAttribute_srcMatchSetPend  ||
                 subAttribute == ApiMac_subAttribute_srcMatchDeleteEntry)
        {
            // Set addr (short or extended) parameter
            // Format: addrmode (1 byte) + address (2 bytes for short, 8 bytes for ext)
            readAddrMode = *value;
            addr.addrMode = readAddrMode;
            value += 1;
            if (readAddrMode == SADDR_MODE_SHORT)
            {
                memcpy(&addr.addr.shortAddr, value, sizeof(addr.addr.shortAddr));
                value += sizeof(addr.addr.shortAddr);

                // Set panid parameter
                memcpy(&panID, value, sizeof(panID));
                value += sizeof(panID);

            }
            else if (readAddrMode == SADDR_MODE_EXT)
            {
                memcpy(&addr.addr.extAddr, value, sizeof(addr.addr.extAddr));
                value += sizeof(addr.addr.extAddr);
                panID = 0; // Unused for ext addr
            }
            else
            {
                return ApiMac_status_unsupportedAttribute;
            }

            if (subAttribute == ApiMac_subAttribute_srcMatchAddEntry)
            {
                MAC_SrcMatchAddEntry (&addr, panID);
            }
            else if (subAttribute == ApiMac_subAttribute_srcMatchSetPend)
            {
                option = *value;
                MAC_SrcMatchSetPend (&addr, panID, option);
            }
            else if (subAttribute == ApiMac_subAttribute_srcMatchDeleteEntry)
            {
                MAC_SrcMatchDeleteEntry (&addr, panID);
            }
        }
        else if (subAttribute == ApiMac_subAttribute_srcMatchSetDefaultPend)
        {
            option = *value;
            MAC_SrcMatchSetDefaultPend(option);
        }
        else if (subAttribute == ApiMac_subAttribute_srcMatchSetManualAddressTracking)
        {
            option = *value;
            MAC_SrcMatchSetManualAddressTracking(option);
        }
#ifdef COPROCESSOR
        else if (subAttribute == ApiMac_subAttribute_setGpOffset)
        {
            GP_Offset = *value;
        }
#endif
        else if (subAttribute == ApiMac_subAttribute_setMacStatistics)
        {
            memcpy(&macStatistics, value, sizeof(macStatisticsStruct_t));
        }
        else
        {
            return ApiMac_status_unsupportedAttribute;
        }
        return ApiMac_status_success;
    }
#ifdef IEEE_COEX_ENABLED
    else if (pibAttribute == ApiMac_coexAttribute_coexConfig)
    {
        ApiMac_mlmeConfigCoex((uint16_t*)pValue);
        return ApiMac_status_success;
    }
#endif

    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, pValue);
}

/*!
 This direct execute function sets an attribute value from
 the MAC Coex PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetCoexReqStruct(
                ApiMac_coexAttribute_struct_t pibAttribute, void *pValue)
{
#ifdef IEEE_COEX_ENABLED
    coexMetricsStruct.dbgCoexGrants = ((coexMetricsStruct_t *)pValue)->dbgCoexGrants;
    coexMetricsStruct.dbgCoexRejects = ((coexMetricsStruct_t *)pValue)->dbgCoexRejects;
    coexMetricsStruct.dbgCoexContRejects = ((coexMetricsStruct_t *)pValue)->dbgCoexContRejects;
    coexMetricsStruct.dbgCoexMaxContRejects = ((coexMetricsStruct_t *)pValue)->dbgCoexMaxContRejects;
#endif
    return ApiMac_status_success;
}

/*!
 This direct execute function sets an attribute value from
 the MAC Statistics PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetMacStatsReqStruct(
            ApiMac_macStatistics_struct_t pibAttribute, void *pValue)
{
#ifdef IEEE_COEX_ENABLED
    macStatistics.pta_lo_pri_req = ((macStatisticsStruct_t *)pValue)->pta_lo_pri_req;
    macStatistics.pta_hi_pri_req = ((macStatisticsStruct_t *)pValue)->pta_hi_pri_req;
    macStatistics.pta_lo_pri_denied = ((macStatisticsStruct_t *)pValue)->pta_lo_pri_denied;
    macStatistics.pta_hi_pri_denied = ((macStatisticsStruct_t *)pValue)->pta_hi_pri_denied;
    macStatistics.pta_denied_rate = ((macStatisticsStruct_t *)pValue)->pta_denied_rate;
#endif
    macStatistics.cca_retries = ((macStatisticsStruct_t *)pValue)->cca_retries;
    macStatistics.cca_failures = ((macStatisticsStruct_t *)pValue)->cca_failures;
    macStatistics.mac_tx_ucast_retry = ((macStatisticsStruct_t *)pValue)->mac_tx_ucast_retry;
    macStatistics.mac_tx_ucast_fail = ((macStatisticsStruct_t *)pValue)->mac_tx_ucast_fail;
    return ApiMac_status_success;
}

/*!
  This is a function call to enable or disable coex in the MAC.
  This must be called with enabled set to true for coex to function.
*/
ApiMac_status_t ApiMac_mlmeEnableCoex(bool enabled)
{
    ieeeCoexEnabled = enabled;

#ifdef IEEE_COEX_ENABLED
    ieeeCoexBitmap = (enabled) ? (ieeeCoexBitmap | APIMAC_COEX_ENABLE) : ieeeCoexBitmap;
#endif

#ifdef IEEE_COEX_3_WIRE
    coexOverrideUseCases.ieeeConnEstabTx.defaultPriority = coexConfigIeee.ieeeInitiatorTx.priority;
    coexOverrideUseCases.ieeeConnEstabTx.assertRequestForRx = coexConfigIeee.ieeeInitiatorTx.request;

    coexOverrideUseCases.ieeeConnectedTx.defaultPriority = coexConfigIeee.ieeeConnectedTx.priority;
    coexOverrideUseCases.ieeeConnectedTx.assertRequestForRx = coexConfigIeee.ieeeConnectedTx.request;

    coexOverrideUseCases.ieeeConnEstabRx.defaultPriority = coexConfigIeee.ieeeInitiatorRx.priority;
    coexOverrideUseCases.ieeeConnEstabRx.assertRequestForRx = coexConfigIeee.ieeeInitiatorRx.request;

    coexOverrideUseCases.ieeeConnectedRx.defaultPriority = coexConfigIeee.ieeeConnectedRx.priority;
    coexOverrideUseCases.ieeeConnectedRx.assertRequestForRx = coexConfigIeee.ieeeConnectedRx.request;
    
    ieeeCoexBitmap = (coexConfigIeee.ieeeInitiatorTx.priority == RF_PriorityCoexHigh) ? (ieeeCoexBitmap | APIMAC_COEX_PRI_CE_TX) : ieeeCoexBitmap;
    ieeeCoexBitmap = (coexConfigIeee.ieeeInitiatorTx.request == RF_RequestCoexAssertRx) ? (ieeeCoexBitmap | APIMAC_COEX_RX_REQ_CE_TX) : ieeeCoexBitmap;

    ieeeCoexBitmap = (coexConfigIeee.ieeeConnectedTx.priority == RF_PriorityCoexHigh) ? (ieeeCoexBitmap | APIMAC_COEX_PRI_CONN_TX) : ieeeCoexBitmap;
    ieeeCoexBitmap = (coexConfigIeee.ieeeConnectedTx.request == RF_RequestCoexAssertRx) ? (ieeeCoexBitmap | APIMAC_COEX_RX_REQ_CONN_TX) : ieeeCoexBitmap;

    ieeeCoexBitmap = (coexConfigIeee.ieeeInitiatorRx.priority == RF_PriorityCoexHigh) ? (ieeeCoexBitmap | APIMAC_COEX_PRI_CE_RX) : ieeeCoexBitmap;
    ieeeCoexBitmap = (coexConfigIeee.ieeeInitiatorRx.request == RF_RequestCoexAssertRx) ? (ieeeCoexBitmap | APIMAC_COEX_RX_REQ_CE_RX) : ieeeCoexBitmap;

    ieeeCoexBitmap = (coexConfigIeee.ieeeConnectedRx.priority == RF_PriorityCoexHigh) ? (ieeeCoexBitmap | APIMAC_COEX_PRI_CONN_RX) : ieeeCoexBitmap;
    ieeeCoexBitmap = (coexConfigIeee.ieeeConnectedRx.request == RF_RequestCoexAssertRx) ? (ieeeCoexBitmap | APIMAC_COEX_RX_REQ_CONN_RX) : ieeeCoexBitmap;
#endif
    return ApiMac_status_success;
}

ApiMac_status_t ApiMac_mlmeConfigCoex(uint16_t* coexEnableBitmap)
{
    uint16_t panID;

#ifdef IEEE_COEX_ENABLED
    ieeeCoexBitmap = *coexEnableBitmap;
    coexConfig.coExEnable.bCoExEnable = ieeeCoexBitmap & APIMAC_COEX_ENABLE ? 1 : 0;
#endif
#ifdef IEEE_COEX_3_WIRE
    coexOverrideUseCases.ieeeConnEstabTx.defaultPriority        = (ieeeCoexBitmap & APIMAC_COEX_PRI_CE_TX) ? RF_PriorityCoexHigh : RF_PriorityCoexLow;
    coexOverrideUseCases.ieeeConnEstabTx.assertRequestForRx     = (ieeeCoexBitmap & APIMAC_COEX_RX_REQ_CE_TX) ? RF_RequestCoexAssertRx : RF_RequestCoexNoAssertRx;

    coexOverrideUseCases.ieeeConnectedTx.defaultPriority        = (ieeeCoexBitmap & APIMAC_COEX_PRI_CONN_TX) ? RF_PriorityCoexHigh : RF_PriorityCoexLow;
    coexOverrideUseCases.ieeeConnectedTx.assertRequestForRx     = (ieeeCoexBitmap & APIMAC_COEX_RX_REQ_CONN_TX) ? RF_RequestCoexAssertRx : RF_RequestCoexNoAssertRx;

    coexOverrideUseCases.ieeeConnEstabRx.defaultPriority        = (ieeeCoexBitmap & APIMAC_COEX_PRI_CE_RX) ? RF_PriorityCoexHigh : RF_PriorityCoexLow;
    coexOverrideUseCases.ieeeConnEstabRx.assertRequestForRx     = (ieeeCoexBitmap & APIMAC_COEX_RX_REQ_CE_RX) ? RF_RequestCoexAssertRx : RF_RequestCoexNoAssertRx;

    coexOverrideUseCases.ieeeConnectedRx.defaultPriority        = (ieeeCoexBitmap & APIMAC_COEX_PRI_CONN_RX) ? RF_PriorityCoexHigh : RF_PriorityCoexLow;
    coexOverrideUseCases.ieeeConnectedRx.assertRequestForRx     = (ieeeCoexBitmap & APIMAC_COEX_RX_REQ_CONN_RX) ? RF_RequestCoexAssertRx : RF_RequestCoexNoAssertRx;
#endif
    // Forcing RX command resubmit through panID update
    ApiMac_mlmeGetReqUint16(ApiMac_attribute_panId, &panID);
    MAC_MlmeSetReq(ApiMac_attribute_panId, &panID);

    return ApiMac_status_success;
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
    return (ApiMac_status_t) MAC_MlmeSetSecurityReq(pibAttribute, (void *)&value);
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqUint16(
                ApiMac_securityAttribute_uint16_t pibAttribute, uint16_t value)
{
    return (ApiMac_status_t) MAC_MlmeSetSecurityReq(pibAttribute, (void *)&value);
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqArray(
                ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return (ApiMac_status_t) MAC_MlmeSetSecurityReq(pibAttribute, (void *)pValue);
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqStruct(
                ApiMac_securityAttribute_struct_t pibAttribute, void *pValue)
{
    return (ApiMac_status_t) MAC_MlmeSetSecurityReq(pibAttribute, (void *)pValue);
}

/*!
 This function is called by a coordinator or PAN coordinator
 to start or reconfigure a network.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeStartReq(ApiMac_mlmeStartReq_t *pData)
{
    return (ApiMac_status_t) MAC_MlmeStartReq(pData);
}

/*!
 This function requests the MAC to synchronize with the
 coordinator by acquiring and optionally tracking its beacons.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSyncReq(ApiMac_mlmeSyncReq_t *pData)
{
    return (ApiMac_status_t) MAC_MlmeSyncReq(pData);
}

/*!
 This function returns a random byte from the MAC random number
 generator.

 Public function defined in api_mac.h
 */
uint8_t ApiMac_randomByte(void)
{
    return MAC_RandomByte();
}

/*!
 Update Device Table entry and PIB with new Pan Id.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_updatePanId(uint16_t panId)
{
    ApiMac_status_t ret = ApiMac_status_noResources;
#ifdef FEATURE_MAC_SECURITY
    macUpdatePanId(panId);
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
    MAC_SrcMatchEnable();
    return ApiMac_status_success;
}
#endif

/*!
 This functions handles the WiSUN async request.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeWSAsyncReq(ApiMac_mlmeWSAsyncReq_t* pData)
{
    return (ApiMac_status_t) MAC_MlmeWSAsyncReq(pData);
}

/*!
 This function start the Frequency hopping operation.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_startFH(void)
{
    return (ApiMac_status_t) MAC_StartFH();
}

/*!
 Enables the Frequency hopping operation.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_enableFH(void)
{
    return (ApiMac_status_t) MAC_EnableFH();
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
        OsalPort_free(pTmp);
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
    /* Initialize the structure */
    memset(pPBcapInfo, 0, sizeof(ApiMac_capabilityInfo_t));

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
    return (ApiMac_status_t) macWrapperAddDevice(pAddDevice);
}

/*!
 Removes MAC device table entries.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secDeleteDevice(ApiMac_sAddrExt_t *pExtAddr)
{
    return (ApiMac_status_t) macWrapperDeleteDevice(pExtAddr);
}

/*!
 Removes the key at the specified key Index and removes all MAC device table
 enteries associated with this key.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secDeleteKeyAndAssocDevices(uint8_t keyIndex)
{
    return (ApiMac_status_t) macWrapperDeleteKeyAndAssociatedDevices(keyIndex);
}

/*!
 Removes all MAC device table entries

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secDeleteAllDevices(void)
{
    return (ApiMac_status_t) macWrapperDeleteAllDevices();
}

/*!
 Reads the frame counter value associated with a MAC security key indexed
 by the designated key identifier and the default key source

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secGetDefaultSourceKey(uint8_t keyId,
                                              uint32_t *pFrameCounter)
{
    return (ApiMac_status_t) macWrapperGetDefaultSourceKey( keyId, (uint32*) pFrameCounter);
}

/*!
 Adds the MAC security key, adds the associated lookup list for the key,
 initializes the frame counter to the value provided

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_secAddKeyInitFrameCounter(
                ApiMac_secAddKeyInitFrameCounter_t *pInfo)
{
    return (ApiMac_status_t) macWrapperAddKeyInitFCtr(pInfo);
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
        *pLen = (uint16_t) MAC_MlmeFHGetReqSize(pibAttribute);
    }

    return (ApiMac_status_t) MAC_MlmeFHGetReq(pibAttribute, pValue);

}

/*!
 * @brief       This function process incoming ICall callback messages.
 *
 * @param       pMsg - pointer to the incoming message
 */
static uint16_t processIncomingICallMsg(macCbackEvent_t *pMsg)
{
    /* Only process message if callbacks are setup */
    if (pMacCallbacks != NULL)
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
                        OsalPort_free(pMsg->scanCnf.result.pEnergyDetect);
                    }
                }
                else
                {
                    if( pMsg->scanCnf.result.pPanDescriptor != NULL)
                    {
                        OsalPort_free(pMsg->scanCnf.result.pPanDescriptor);
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
        OsalPort_msgDeallocate((uint8_t*) pData);
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

        pTempIe = (ApiMac_payloadIeRec_t *)OsalPort_malloc(
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
            OsalPort_free(pIe);
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
    return (ApiMac_status_t) MAC_MlmeFHSetReq(pibAttribute, pValue);
}

#ifdef FREERTOS_SUPPORT

#define RF_CB_THREADSTACKSIZE 2000

pthread_t rfCdThreadHndl = NULL;

sem_t rfCbSemHandle;

uint32_t rxCount;
bool rxRec;
// rf queue variables
uint8_t macRfQueueSize;
macRfQueue_t macRfQueue[MAC_FREERTOS_RF_SLOTS];
uint8_t wRfPtr;
uint8_t rRfPtr;
uint8_t numRfCmdQueued;

void runRxCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    //pushing to rf queue

    macRfQueue[wRfPtr].rfCbHndl = h;
    macRfQueue[wRfPtr].rfCbCmdHndl = ch;
    macRfQueue[wRfPtr].rfCbEventMask = e;
    macRfQueue[wRfPtr].rfCb = macRxCb;
    wRfPtr = (wRfPtr + 1) % macRfQueueSize;
    numRfCmdQueued++;

    rxCount++;

    sem_post(&rfCbSemHandle);

}

void *rfCbThread(void *arg0)
{
    halIntState_t  s;
    while(true)
    {
        sem_wait(&rfCbSemHandle);
        MAC_ASSERT(numRfCmdQueued);

        macRfQueue[rRfPtr].rfCb(macRfQueue[rRfPtr].rfCbHndl,
                               macRfQueue[rRfPtr].rfCbCmdHndl,
                               macRfQueue[rRfPtr].rfCbEventMask);
        HAL_ENTER_CRITICAL_SECTION(s);
        numRfCmdQueued--;
        HAL_EXIT_CRITICAL_SECTION(s);
        rRfPtr = (rRfPtr + 1) % macRfQueueSize;
    }
}

void startRfCbThread(void)
{
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    rxCount = 0;
    rxRec = true;
    numRfCmdQueued = 0;

    // initialize the RF Queue
    wRfPtr = 0;
    rRfPtr = 0;
    macRfQueueSize = MAC_FREERTOS_RF_SLOTS;
    for (int i = 0; i < macRfQueueSize; i++)
    {
      memset(&macRfQueue[i], 0, sizeof(macRfQueue_t));

    }

    if(rfCdThreadHndl == NULL)
    {
        /* create semaphores
         */
        retc = sem_init(&rfCbSemHandle, 0, 0);
        if (retc != 0) {
            while (1);
        }

        /* Initialize the attributes structure with default values */
        pthread_attr_init(&attrs);

        /* Set priority, detach state, and stack size attributes */
        priParam.sched_priority = (configMAX_PRIORITIES - 1);
        retc = pthread_attr_setschedparam(&attrs, &priParam);
        retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
        retc |= pthread_attr_setstacksize(&attrs, RF_CB_THREADSTACKSIZE);
        if (retc != 0) {
            /* failed to set attributes */
            while (1) {}
        }

        retc = pthread_create(&rfCdThreadHndl, &attrs, rfCbThread, NULL);
        if (retc != 0) {
            /* pthread_create() failed */
            while (1) {}
        }
    }
}

#endif //FREERTOS_SUPPORT
