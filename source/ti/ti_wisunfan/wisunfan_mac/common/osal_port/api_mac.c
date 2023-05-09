/******************************************************************************

 @file api_mac.c

 @brief TIMAC 2.0 API

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2023, Texas Instruments Incorporated
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
#include "mbed_config_app.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <inc/hw_ccfg.h>
#include <inc/hw_ccfg_simple_struct.h>
#include <inc/hw_memmap.h>
#include <inc/hw_fcfg1.h>
#include <inc/hw_types.h>
#include "advanced_config.h"
#include "nsconfig.h"
#include "net_interface.h"
#include "mac_hl_patch.h"
#include "ns_types.h"
#include "mac_common_defines.h"

#include "6LoWPAN/MAC/mpx_api.h"
#include "6LoWPAN/ws/ws_llc.h"
#include "6LoWPAN/ws/ws_ie_lib.h"
#include "NWK_INTERFACE/Include/protocol_timer.h"
#include "NWK_INTERFACE/Include/protocol.h"
#include "MAC/rf_driver_storage.h"

#include "mac_assert.h"
#include "osal_port.h"
#include "timac_ns_interface.h"
#include "api_mac.h"
#include "macTask.h"
#include "timac_api.h"
#include "macs.h"
#include "macwrapper.h"
#include "nsdynmemLIB.h"
#include "application.h"
#include "mac_settings.h"
#define TRACE_GROUP "MRsH"
#include "ns_trace.h"
#include "eventOS_event.h"

#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/knl/Task.h>
#endif
/*!
 This module is the ICall interface for the application and all ICall
 activity must go through this module, no ICall activity anywhere else.
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
#ifdef DBG_OSAL
extern struct osal_debug osalDbg;
#endif

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

#define MAC_ADDR_LEN        8

#define MAC_UNDEFINED_TASKID    0xFF

#define IE_HDR_LEN          2
/******************************************************************************
 Structures
 *****************************************************************************/
typedef struct mac_internal_s {
    mac_api_t *mac_api;
    arm_device_driver_list_s *dev_driver;
    arm_device_driver_list_s *virtual_driver;
    //Move define inside MAC (now in protocol_abstract.h)
    struct protocol_interface_rf_mac_setup *setup;
    uint8_t device_table_size;
    uint8_t key_description_size;
    //linked list link
} mac_internal_t;
/******************************************************************************
 Global variables
 *****************************************************************************/
/*!
 The ApiMac_extAddr is the MAC's IEEE address, setup with the Chip's
 IEEE addresses in main.c
 */
ApiMac_sAddrExt_t ApiMac_extAddr;
extern uint8_t timacTaskId;
extern sem_t event_thread_sem_handle;

extern configurable_props_t cfg_props;
/******************************************************************************
 Local variables
 *****************************************************************************/
/*! Semaphore used to post events to the application thread */
static sem_t appSemHandle;

/*! Storage for Events flags */
static uint32_t appEvents = 0;
static uint8_t stackTaskId;
static uint8_t appTaskId = MAC_UNDEFINED_TASKID;
static struct mac_api_s *mbed_mac_api = NULL;
static fhss_api_t *fhss_api = NULL;
static uint8_t deviceExtAddr[8];

#ifdef DBG_APP
uint16_t appDbg_event[20];
uint8_t appDbg_eidx = 0;
struct {
    uint16_t asynchReq[4];
    uint16_t asynchInd[4];
    uint32_t data[6];
} mcpsDbg;
#endif
/*****************************************************************************
 Local Function Prototypes
 *****************************************************************************/
static ApiMac_status_t mlmeGetFhReq(uint16_t pibAttribute, void *pValue, uint16_t *pLen);
static ApiMac_status_t mlmeSetFhReq(uint16_t pibAttribute, void *pValue);
static uint16_t processIncomingICallMsg(macCbackEvent_t *pMsg);

static int8_t ns_sw_mac_initialize(mac_api_t *api, mcps_data_confirm *mcps_data_conf_cb,
                                   mcps_data_indication *mcps_data_ind_cb, mcps_purge_confirm *purge_conf_cb,
                                   mlme_confirm *mlme_conf_callback, mlme_indication *mlme_ind_callback, int8_t parent_id);
static int8_t ns_sw_mac_api_enable_mcps_ext(mac_api_t *api, mcps_data_indication_ext *data_ind_cb,
                                            mcps_data_confirm_ext *data_cnf_cb, mcps_ack_data_req_ext *ack_data_req_cb);
static int8_t ns_sw_mac_api_enable_edfe_ext(mac_api_t *api, mcps_edfe_handler *edfe_ind_cb);
static void mlme_req(const mac_api_t *api, mlme_primitive id, void *data);
static void mcps_req(const mac_api_t *api, const mcps_data_req_t *data);
static uint8_t mcps_req_ext(const mac_api_t *api, void *data, ns_ie_iovec_t *iovec, wh_ie_sub_list_t *ie_header_mask, wp_nested_ie_sub_list_t *nested_wp_id, uint8_t *gtkhash);
static uint8_t purge_req(const mac_api_t *api, const mcps_purge_t *data);
static int8_t macext_mac64_address_set(const mac_api_t *api, const uint8_t *mac64);
static int8_t macext_mac64_address_get(const mac_api_t *api, mac_extended_address_type type, uint8_t *mac64_buf);

static int8_t sw_mac_storage_decription_sizes_get(const mac_api_t *api, mac_description_storage_size_t *buffer);
static void setGtkhash(uint8_t *gtkhash);

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize this module.

 Public function defined in api_mac.h
 */
void *ApiMac_init(uint8_t macTaskIdParam, bool enableFH)
{
    stackTaskId = macTaskIdParam;
    int                 retc;

    /* create semaphores for messages / events
     */
    retc = sem_init(&appSemHandle, 0, 0);
    if (retc != 0) {
        while (1);
    }

    appTaskId = OsalPort_registerTask(pthread_self(), &appSemHandle, &appEvents);

    /* Allocate message buffer space */
    macStackInitParams_t *pMsg = (macStackInitParams_t *)OsalPort_msgAllocate(sizeof(macStackInitParams_t));

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

        OsalPort_msgSend( stackTaskId, (uint8_t*)pMsg );
    }

    /* Let MAC task consume the message */
#ifndef FREERTOS_SUPPORT
    Task_sleep(10);
#endif

    /* Enable frequency hopping? */
    if(enableFH)
    {
        ApiMac_enableFH();
    }

    /* Reset the MAC */
    ApiMac_mlmeResetReq(true);
    return (&appSemHandle);
}

/*!
 Register for MAC callbacks.

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
 This direct execute function retrieves an attribute value from
 the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeGetReqBool(ApiMac_attribute_bool_t pibAttribute,
bool *pValue)
{
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
        *pLen = (uint32_t)MAC_MlmeGetReqSize(pibAttribute);
    }

    return (ApiMac_status_t) MAC_MlmeGetReq(pibAttribute, pValue);
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
 This function must be called once at system startup before any other
 function in the management API is called.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeResetReq(bool setDefaultPib)
{
    return (ApiMac_status_t) MAC_MlmeResetReq(setDefaultPib);
}

/*!
 This direct execute function sets an attribute value
 in the MAC PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetReqBool(ApiMac_attribute_bool_t pibAttribute,
bool value)
{
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
    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, pValue);
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
#ifdef FEATURE_MAC_SECURITY

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
#endif

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


void ccfg_read_mac_addr(uint8_t *mac_addr)
{
    uint8_t invalidExtAddr[] =
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint32_t read_mac_addr;
    uint8_t tmp_mac_addr[MAC_ADDR_LEN];

    read_mac_addr = (( HWREG(
            CCFG_BASE + CCFG_O_IEEE_MAC_0 ) &
            CCFG_IEEE_MAC_0_ADDR_M ) >>
            CCFG_IEEE_MAC_0_ADDR_S );
    memcpy(tmp_mac_addr, (uint8_t *)&read_mac_addr, (MAC_ADDR_LEN / 2));

    read_mac_addr = (( HWREG(
            CCFG_BASE + CCFG_O_IEEE_MAC_1 ) &
            CCFG_IEEE_MAC_1_ADDR_M ) >>
            CCFG_IEEE_MAC_1_ADDR_S );
    memcpy(tmp_mac_addr + (MAC_ADDR_LEN / 2), (uint8_t *)&read_mac_addr,
           (MAC_ADDR_LEN / 2));

    /* Check to see if the CCFG IEEE is valid */
    if(memcmp(tmp_mac_addr, invalidExtAddr, MAC_ADDR_LEN) == 0)
    {
        /* No, it isn't valid.  Get the Primary IEEE Address */
        memcpy(tmp_mac_addr, (uint8_t *)(FCFG1_BASE + FCFG1_O_MAC_15_4_0),
               (MAC_ADDR_LEN));
    }

    sAddrExtRevCpy(mac_addr, tmp_mac_addr);
}

void timacExtaddressRegister()
{
#ifdef WRITABLE_HWADDR
    unsigned char null_hw_address[8] = {0};

    if (memcmp(&cfg_props.hwaddr, null_hw_address, 8) == 0)
    {
        ccfg_read_mac_addr(&cfg_props.hwaddr[0]);
    }

    memcpy(deviceExtAddr, cfg_props.hwaddr, sizeof(deviceExtAddr));
#else
    /* Copy the extended address from the CCFG area */
    ccfg_read_mac_addr(deviceExtAddr);
#endif
}

void timacConvertDataReq(mcps_data_req_t *mdataReq, ApiMac_mcpsDataReq_t *tdataReq)
{
    tdataReq->srcAddrMode = (mdataReq->SrcAddrMode == 3 ? ApiMac_addrType_extended : ApiMac_addrType_none);
    tdataReq->dstAddr.addrMode = (mdataReq->DstAddrMode  == 3 ? ApiMac_addrType_extended : ApiMac_addrType_none);
    tdataReq->dstPanId = mdataReq->DstPANId;
    if(mdataReq->DstAddrMode == 3)
    {
        memcpy(tdataReq->dstAddr.addr.extAddr, mdataReq->DstAddr, 8);
    }
    tdataReq->msdu.len = mdataReq->msduLength;
    tdataReq->msdu.p = mdataReq->msdu;
    tdataReq->msduHandle = mdataReq->msduHandle;
    if(mdataReq->TxAckReq)
    {
        tdataReq->txOptions.ack = true;
    }
    else
    {
        tdataReq->txOptions.noRetransmits = true;
    }
    tdataReq->txOptions.indirect = mdataReq->InDirectTx;
    tdataReq->txOptions.pendingBit = mdataReq->PendingBit;
    memcpy(&tdataReq->sec, &mdataReq->Key, sizeof(ApiMac_sec_t));
}

void timacConvertDataCnf(mcps_data_conf_t *mdataCnf, macMcpsDataCnf_t *tdataCnf)
{
    mdataCnf->msduHandle = tdataCnf->msduHandle;
    mdataCnf->status = tdataCnf->hdr.status;
    mdataCnf->timestamp = tdataCnf->timestamp;
    mdataCnf->tx_retries = tdataCnf->retries;
    mdataCnf->cca_retries = 0;
}

void timacConvertDataInd(mcps_data_ind_t *mdataInd, macMcpsDataInd_t *tdataInd)
{
    mdataInd->SrcAddrMode = (tdataInd->mac.srcAddr.addrMode == 3 ? 3 : 0);
    mdataInd->SrcPANId = tdataInd->mac.srcPanId;
    if(tdataInd->mac.srcAddr.addrMode == 3)
    {
        memcpy(mdataInd->SrcAddr, tdataInd->mac.srcAddr.addr.extAddr, 8);
    }
    mdataInd->DstAddrMode = (tdataInd->mac.dstAddr.addrMode == 3 ? 3 : 0);
    mdataInd->DstPANId = tdataInd->mac.dstPanId;
    if(tdataInd->mac.dstAddr.addrMode == 3)
    {
        memcpy(mdataInd->DstAddr, tdataInd->mac.dstAddr.addr.extAddr, 8);
    }
    mdataInd->mpduLinkQuality = tdataInd->mac.mpduLinkQuality;
    mdataInd->signal_dbm = tdataInd->mac.rssi;
    mdataInd->timestamp = tdataInd->mac.timestamp;
    /* currently seq no of 0 is used to represent seqNoSuppression */
    mdataInd->DSN_suppressed = (tdataInd->mac.dsn == 0 ? true : false);
    mdataInd->DSN = tdataInd->mac.dsn;
    memcpy(&mdataInd->Key, &tdataInd->sec, sizeof(ApiMac_sec_t));
    mdataInd->msduLength = tdataInd->msdu.len;
    mdataInd->msdu_ptr = tdataInd->msdu.p;
}

void timacConvertIncludeIes(wh_ie_sub_list_t *wh_mask, wp_nested_ie_sub_list_t *wp_mask, ApiMac_mcpsDataReq_t *tdataReq, bool mpxIe)
{
    if(mpxIe)
    {
        tdataReq->includeFhIEs |= MAC_FH_MPX_IE;
    }
    if(wh_mask->utt_ie)
    {
        tdataReq->includeFhIEs |= MAC_FH_UT_IE;
    }
    if(wh_mask->bt_ie)
    {
        tdataReq->includeFhIEs |= MAC_FH_BT_IE;
    }
    if(wh_mask->ea_ie)
    {
        tdataReq->includeFhIEs |= MAC_FH_EA_IE;
    }
    if(wp_mask->us_ie)
    {
        tdataReq->includeFhIEs |= MAC_FH_US_IE;
    }
    if(wp_mask->bs_ie)
    {
        tdataReq->includeFhIEs |= MAC_FH_BS_IE;
    }
    if(wp_mask->pan_ie)
    {
        tdataReq->includeFhIEs |= MAC_FH_US_IE;
    }
}

void timacConvertKeyDescriptor(const mlme_key_descriptor_entry_t *mKeyDesc, keyDescriptor_t *tKeyDesc)
{
    uint16_t i;

    if(mKeyDesc->KeyIdLookupListEntries)
    {
        tKeyDesc->keyIdLookupList = OsalPort_malloc(mKeyDesc->KeyIdLookupListEntries * sizeof(keyIdLookupDescriptor_t));
        MAC_ASSERT(tKeyDesc->keyIdLookupList != NULL );
        for(i = 0; i < mKeyDesc->KeyIdLookupListEntries; i++)
        {
            memcpy((tKeyDesc->keyIdLookupList+i)->lookupData, (mKeyDesc->KeyIdLookupList+i)->LookupData, MAC_MAX_KEY_LOOKUP_LEN);
            (tKeyDesc->keyIdLookupList+i)->lookupDataSize = (mKeyDesc->KeyIdLookupList+i)->LookupDataSize;
        }
        tKeyDesc->keyIdLookupEntries = mKeyDesc->KeyIdLookupListEntries;
    }

    if(mKeyDesc->KeyDeviceListEntries)
    {
        tKeyDesc->keyDeviceList = OsalPort_malloc(mKeyDesc->KeyDeviceListEntries * sizeof(keyDeviceDescriptor_t));
        MAC_ASSERT(tKeyDesc->keyDeviceList != NULL );
        for(i = 0; i < mKeyDesc->KeyDeviceListEntries; i++)
        {
            (tKeyDesc->keyDeviceList+i)->deviceDescriptorHandle = (mKeyDesc->KeyDeviceList+i)->DeviceDescriptorHandle;
            (tKeyDesc->keyDeviceList+i)->uniqueDevice = (mKeyDesc->KeyDeviceList+i)->UniqueDevice;
            (tKeyDesc->keyDeviceList+i)->blackListed = (mKeyDesc->KeyDeviceList+i)->Blacklisted;
        }
        tKeyDesc->keyDeviceListEntries = mKeyDesc->KeyDeviceListEntries;
    }

    if(mKeyDesc->KeyUsageListEntries)
    {
        tKeyDesc->keyUsageList = OsalPort_malloc(mKeyDesc->KeyUsageListEntries * sizeof(keyUsageDescriptor_t));
        MAC_ASSERT(tKeyDesc->keyUsageList != NULL );
        for(i = 0; i < mKeyDesc->KeyUsageListEntries; i++)
        {
            (tKeyDesc->keyUsageList+i)->frameType = (mKeyDesc->KeyUsageList+i)->FrameType;
            (tKeyDesc->keyUsageList+i)->cmdFrameId = (mKeyDesc->KeyUsageList+i)->CommandFrameIdentifier;
        }
        tKeyDesc->keyUsageListEntries = mKeyDesc->KeyUsageListEntries;
    }
    memcpy(tKeyDesc->key, mKeyDesc->Key, MAC_KEY_MAX_LEN);
}

void timacFreeKeyDescriptor(keyDescriptor_t *tKeyDesc)
{
    if(tKeyDesc->keyIdLookupEntries)
    {
        OsalPort_free(tKeyDesc->keyIdLookupList);
    }

    if(tKeyDesc->keyDeviceListEntries)
    {
        OsalPort_free(tKeyDesc->keyDeviceList);
    }

    if(tKeyDesc->keyUsageListEntries)
    {
        OsalPort_free(tKeyDesc->keyUsageList);
    }
}

void timacConvertDeviceDescriptor(const mlme_device_descriptor_t *mDevDesc, deviceDescriptor_t *tDevDesc)
{
    uint8_t i;

    tDevDesc->panID = mDevDesc->PANId;
    tDevDesc->shortAddress = mDevDesc->ShortAddress;
    memcpy(tDevDesc->extAddress, mDevDesc->ExtAddress, 8);
    tDevDesc->exempt = mDevDesc->Exempt;
    for (i = 0; i < MAX_KEY_TABLE_ENTRIES; i++)
    {
        tDevDesc->frameCounter[i] = mDevDesc->FrameCounter;
    }
}

void timacStorePanInformation(ws_pan_information_t *panInfo)
{
#ifdef WISUN_CERT_CONFIG
    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_panSize, 8192);
#else
    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_panSize, panInfo->pan_size);
#endif
    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_routingCost, panInfo->routing_cost);
    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_useParentBSIE, panInfo->use_parent_bs);
    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_routingMethod, panInfo->rpl_routing_method);
    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_fanTPSVersion, panInfo->version);
}

void timacSetPanId(uint16_t panId)
{
    ApiMac_mlmeSetReqUint16(ApiMac_attribute_panId, panId);
}

void timacSetTrackParent(uint8_t* eui64)
{
    ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_trackParentEUI, eui64);
}

void timacStorePanVersionInformation(uint16_t panVersion)
{
    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_panVersion, panVersion);
}

void mcps_data_confirm_handler(const mac_api_t *api, const mcps_data_conf_t *data)
{
    (void) api;
    (void) data;
    MAC_ASSERT(0);
    return;
}

void mcps_data_indication_handler(const mac_api_t *api, const mcps_data_ind_t *data_ind)
{
    (void) api;
    (void) data_ind;
    MAC_ASSERT(0);
    return;
}

void mcps_purge_confirm_handler(const mac_api_t *api, mcps_purge_conf_t *data)
{
    (void) api;
    (void) data;
    MAC_ASSERT(0);
    return;
}

static void stop_bootstrap_timer(protocol_interface_info_entry_t *info_entry)
{
    if (info_entry->bootsrap_mode == ARM_NWK_BOOTSRAP_MODE_6LoWPAN_BORDER_ROUTER) {
        protocol_timer_stop(PROTOCOL_TIMER_BOOTSTRAP_TIM);
    }
}

void mlme_confirm_handler(const mac_api_t *api, mlme_primitive id, const void *data)
{
    protocol_interface_info_entry_t *info_entry = protocol_stack_interface_info_get_by_id(api->parent_id);
    if (!info_entry) {
        return;
    }
    switch (id) {
        case MLME_START: {
            stop_bootstrap_timer(info_entry);
            break;
        }
        case MLME_GET:
        case MLME_SET:
        case MLME_RESET:
        case MLME_SCAN:
        case MLME_ASSOCIATE:
        case MLME_DISASSOCIATE:
        case MLME_SYNC:
        case MLME_POLL:
        case MLME_RX_ENABLE:
        case MLME_GTS:
        case MLME_BEACON_NOTIFY:
        case MLME_ORPHAN:
        case MLME_COMM_STATUS:
        case MLME_SYNC_LOSS:
        default:
            break;
    }
}

void mlme_indication_handler(const mac_api_t *api, mlme_primitive id, const void *data)
{
    (void) api;
    (void) id;
    (void) data;
    MAC_ASSERT(0);
    return;
}

static int8_t sw_mac_storage_decription_sizes_get(const mac_api_t *api, mac_description_storage_size_t *buffer)
{
    if (!api || !buffer) {
        return -1;
    }

    buffer->device_decription_table_size = 32;
    buffer->key_description_table_size = 4;
    buffer->key_lookup_size = 1;
    buffer->key_usage_size = 3;
    return 0;
}

mac_api_t *ns_sw_mac_create(int8_t rf_driver_id, mac_description_storage_size_t *storage_sizes)
{
    (void) rf_driver_id;
    (void ) storage_sizes;

    mac_api_t *this = (mac_api_t *)ns_dyn_mem_alloc(sizeof(mac_api_t));
    if (!this) {
        return NULL;
    }
    memset(this, 0, sizeof(mac_api_t));
    this->parent_id = -1;
    this->mac_initialize = &ns_sw_mac_initialize;
    this->mac_mcps_extension_enable = &ns_sw_mac_api_enable_mcps_ext;
    this->mac_mcps_edfe_enable = &ns_sw_mac_api_enable_edfe_ext;
    this->mlme_req = &mlme_req;
    this->mcps_data_req = &mcps_req;
    this->mcps_data_req_ext = &mcps_req_ext;
    this->mcps_purge_req = &purge_req;
    this->mac64_get = &macext_mac64_address_get;
    this->mac64_set = &macext_mac64_address_set;
    this->mac_storage_sizes_get = &sw_mac_storage_decription_sizes_get;
    this->phyMTU = MAC_MAX_TX_MTU_SIZE;

    return this;
}

int8_t ns_sw_mac_enable_frame_counter_per_key(struct mac_api_s *mac_api_s, bool enable_feature)
{
    (void) mac_api_s;
    (void) enable_feature;
    return 0;
}

int ns_sw_mac_fhss_register(mac_api_t *mac_api, fhss_api_t *fhss_api)
{
    (void) mac_api;
    (void) fhss_api;
    return 0;
}

int ns_sw_mac_fhss_unregister(mac_api_t *mac_api)
{
    (void) mac_api;
    (void) fhss_api;
    return 0;
}

int ns_sw_mac_statistics_start(struct mac_api_s *mac_api, struct mac_statistics_s *mac_statistics)
{
    (void) mac_api;
    (void) mac_statistics;
    return 0;
}

struct fhss_api *ns_sw_mac_get_fhss_api(struct mac_api_s *mac_api)
{
    (void) mac_api;
    return fhss_api;
}

static int8_t ns_sw_mac_initialize(mac_api_t *api, mcps_data_confirm *mcps_data_conf_cb,
                                   mcps_data_indication *mcps_data_ind_cb, mcps_purge_confirm *mcps_purge_conf_cb,
                                   mlme_confirm *mlme_conf_callback, mlme_indication *mlme_ind_callback, int8_t parent_id)
{
    api->data_conf_cb = mcps_data_conf_cb;
    api->data_ind_cb = mcps_data_ind_cb;
    api->purge_conf_cb = mcps_purge_conf_cb;
    api->mlme_conf_cb = mlme_conf_callback;
    api->mlme_ind_cb = mlme_ind_callback;
    api->parent_id = parent_id;
    timac_initialize(api);
    return 0;
}

static int8_t ns_sw_mac_api_enable_mcps_ext(mac_api_t *api, mcps_data_indication_ext *data_ind_cb,
                                            mcps_data_confirm_ext *data_cnf_cb, mcps_ack_data_req_ext *ack_data_req_cb)
{
    if (!api) {
        return -1;
    }

    api->data_conf_ext_cb = data_cnf_cb;
    api->data_ind_ext_cb = data_ind_cb;
    api->enhanced_ack_data_req_cb = ack_data_req_cb;
    return 0;
}

static int8_t ns_sw_mac_api_enable_edfe_ext(mac_api_t *api, mcps_edfe_handler *edfe_ind_cb)
{
    if (!api) {
        return -1;
    }

    api->edfe_ind_cb = edfe_ind_cb;

    return 0;
}

void mlme_req(const mac_api_t *api, mlme_primitive id, void *data)
{
    uint8_t ret = MAC_UNSUPPORTED;
#ifdef FEATURE_MAC_SECURITY
    deviceIndexDesc_t deviceDesc;
    keyIndexDesc_t keyDesc;
    macSecurityGet_t reqData;
    uint8_t zeroExt[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t oneExt[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
#endif

    switch (id) {
        case MLME_GET: {
            mlme_get_t *dat = (mlme_get_t *) data;
#ifdef FEATURE_MAC_SECURITY
            if(dat->attr == macFrameCounter)
            {
                reqData.keyIndex = dat->attr_index;
                reqData.data = dat->value_pointer;
                ret = MAC_MlmeGetSecurityReq(dat->attr, (void *)&reqData);
                if (ret == MAC_SUCCESS)
                {
                    memcpy(dat->value_pointer, reqData.data, sizeof(uint32_t));
                }
            }
#endif
            dat->status = ret;
            break;
        }
        case MLME_SET: {
            mlme_set_t *dat = (mlme_set_t *)data;
            const mlme_device_descriptor_t *device_desc = (const mlme_device_descriptor_t *)dat->value_pointer;
            const mlme_key_descriptor_entry_t *key_desc = (const mlme_key_descriptor_entry_t *)dat->value_pointer;
            if(dat->attr < mac802_15_4Mode)
            {
                if(((dat->attr >= macKeyTable) && (dat->attr <= macDeviceTableFrameCount)) ||
                        (dat->attr == macDeviceTableFrameCount))
                {
#ifdef FEATURE_MAC_SECURITY
                    if(dat->attr == macDeviceTable)
                    {
                        memset(&deviceDesc, 0, sizeof(deviceIndexDesc_t));
                        timacConvertDeviceDescriptor(device_desc, &deviceDesc.deviceDesc);
                        deviceDesc.deviceIndex = dat->attr_index;
                        ret = MAC_MlmeSetSecurityReq(macDeviceTable, (void *)&deviceDesc);
                    }
                    else if(dat->attr == macKeyTable)
                    {
                        memset(&keyDesc, 0, sizeof(keyIndexDesc_t));
                        timacConvertKeyDescriptor(key_desc, &keyDesc.keyDesc);
                        keyDesc.keyIndex = dat->attr_index;
                        ret = MAC_MlmeSetSecurityReq(macKeyTable, (void *)&keyDesc);
                        timacFreeKeyDescriptor(&keyDesc.keyDesc);
                    }
                    // RX frame counter case, updated on boot and rekey
                    else if(dat->attr == macDeviceTableFrameCount)
                    {
                        uint16_t i;
                        deviceDescriptor_t *device_table;
                        mlme_device_frame_count_t frameCountAndIndex;
                        memcpy(&frameCountAndIndex, dat->value_pointer, sizeof(mlme_device_frame_count_t));
                        ret = MAC_MlmeGetPointerSecurityReq(macDeviceTable, (void **)&device_table);
                        if (ret == MAC_SUCCESS)
                        {
                            for (i = 0; i < MAX_DEVICE_TABLE_ENTRIES; i++)
                            {
                                device_table[i].frameCounter[frameCountAndIndex.frameCountIndex] =
                                        frameCountAndIndex.frameCount;
                            }
                        }
                    }
                    // TX frame counter case, updated on boot from NV
                    else if(dat->attr == macFrameCounter)
                    {
                        keyIndexDesc_t keyIndexDescriptor = {0};
                        keyIndexDescriptor.keyIndex = dat->attr_index;
                        keyIndexDescriptor.keyDesc.frameCounter = *((uint32_t *) dat->value_pointer);
                        ret = MAC_MlmeSetSecurityReq(dat->attr, (void *) &keyIndexDescriptor);
                    }
                    else
                    {
                        ret = MAC_MlmeSetSecurityReq(dat->attr, (void *)dat->value_pointer);
                    }
#endif
                }
                else
                {
                    if((dat->attr == macExtendedAddress) || (dat->attr == macPanCoordExtendedAddress))
                    {
                        uint8_t addr[8];
                        memcpy(addr, dat->value_pointer, 8);
                        ret = MAC_MlmeSetReq(dat->attr, (void *)addr);
                    }
                    else
                    {
                        ret = MAC_MlmeSetReq(dat->attr, (void *)dat->value_pointer);
                    }
                }
            }
            dat->status = ret;
            break;
        }
        case MLME_START: {
            ApiMac_mlmeStartReq_t *dat = (ApiMac_mlmeStartReq_t *)data;
            dat->phyID = cfg_props.config_phy_id;
            dat->startFH = true;
            dat->channelPage = MAC_CHANNEL_PAGE_9;
            MAC_MlmeStartReq(dat);
            break;
        }
        case MLME_RESET: {
            MAC_MlmeResetReq(true);
            break;
        }
        case MLME_SCAN:
        case MLME_ASSOCIATE:
        case MLME_DISASSOCIATE:
        case MLME_SYNC:
        case MLME_POLL:
        case MLME_RX_ENABLE:
        case MLME_GTS:
        default:
            break;
    }
}

void mcps_req(const mac_api_t *api, const mcps_data_req_t *data)
{
    (void) api;
    (void) data;
    MAC_ASSERT(0);
    return;
}

uint8_t mcps_req_ext(const mac_api_t *api, void *data, ns_ie_iovec_t *iovec, wh_ie_sub_list_t *ie_header_mask, wp_nested_ie_sub_list_t *nested_wp_id, uint8_t *gtkhash)
{
    (void) api;
    ApiMac_mlmeWSAsyncReq_t asyncReq;
    ApiMac_mcpsDataReq_t timacDataReq;
    uint8_t status = 0;

    if(!iovec)
    {
        asynch_request_t *asynch_req = (asynch_request_t *)data;
#ifdef DBG_APP
        mcpsDbg.asynchReq[asynch_req->message_type]++;
#endif
        if(asynch_req->wp_requested_nested_ie_list.gtkhash_ie && gtkhash)
        {
            setGtkhash(gtkhash);
        }
        asyncReq.frameType = (ApiMac_wisunAsyncFrame_t)asynch_req->message_type;
        asyncReq.operation = (ApiMac_wisunAsycnOperation_t)MAC_WS_OPER_ASYNC_START;
        memcpy(asyncReq.channels, asynch_req->channel_list.channel_mask, 17);
        memcpy(&asyncReq.sec, &asynch_req->security, sizeof(mlme_security_t));
        status = MAC_MlmeWSAsyncReq(&asyncReq);
    }
    else
    {
        mcps_data_req_t *data_req = (mcps_data_req_t *)data;
#ifdef DBG_APP
        mcpsDbg.data[0]++;
#endif
        memset(&timacDataReq, 0, sizeof(ApiMac_mcpsDataReq_t));
        timacConvertDataReq(data_req, &timacDataReq);
        timacDataReq.pIEList = iovec->ieBase;
        timacDataReq.payloadIELen = iovec->iovLen;
        timacConvertIncludeIes(ie_header_mask, nested_wp_id, &timacDataReq, true);
        tr_info("\n MAC MCPs Data Req");
        status = MAC_McpsDataReq(&timacDataReq);
    }

#ifdef DBG_OSAL
    if(status)
    {
        osalDbg.mac_data_req_fail_cnt++;
    }
#endif

    return status;
}

static uint8_t purge_req(const mac_api_t *api, const mcps_purge_t *data)
{
    (void) api;
    (void) data;
    /* To Do : implement purge here ?
    MAC_ASSERT(0);
    */
    return 0;
}

static int8_t macext_mac64_address_set(const mac_api_t *api, const uint8_t *mac64)
{
    if (!mac64 || !api) {
        return -1;
    }

    mlme_set_t set_req;
    set_req.attr = macExtendedAddress;
    set_req.attr_index = 0;
    set_req.value_pointer = mac64;
    set_req.value_size = 8;
    api->mlme_req(api, MLME_SET, &set_req);
    return 0;
}


static int8_t macext_mac64_address_get(const mac_api_t *api, mac_extended_address_type type, uint8_t *mac64_buf)
{
    if (!mac64_buf || !api) {
        return -1;
    }
    uint8_t *ptr;
    switch (type) {
        case MAC_EXTENDED_READ_ONLY:
        case MAC_EXTENDED_DYNAMIC:
            ptr = deviceExtAddr;
            break;
        default:
            return -1;
    }
    memcpy(mac64_buf, ptr, 8);
    return 0;
}

uint32_t ns_sw_mac_read_current_timestamp(struct mac_api_s *mac_api)
{
    // get current time in microseconds to compare with utt_rx_timestamp in us
    uint32_t currentTime   = MAP_ICall_getTicks() * ClockP_getSystemTickPeriod();
    return currentTime;
}

static void setGtkhash(uint8_t *gtkhash)
{
#define GTKHASH_SIZE        8
    if(gtkhash)
    {
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk0Hash, (uint8_t *)gtkhash);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk1Hash, (uint8_t *)gtkhash + GTKHASH_SIZE);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk2Hash, (uint8_t *)gtkhash + (GTKHASH_SIZE * 2));
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk3Hash, (uint8_t *)gtkhash + (GTKHASH_SIZE * 3));
    }
}

void timac_initialize(struct mac_api_s *api)
{
    /* Initialize the MAC */
    ApiMac_init(timacTaskId, true);
    mbed_mac_api = api;

    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyCurrentDescriptorId, cfg_props.config_phy_id);
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_channelPage, (uint8_t)CONFIG_CHANNEL_PAGE);
    /* Set the transmit power */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyTransmitPowerSigned, (uint8_t)cfg_props.phyTxPower);
    ApiMac_mlmeSetReqBool(ApiMac_attribute_RxOnWhenIdle, true);

    ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_netName, (uint8_t*)&cfg_props.network_name[0]);
    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_numNonSleepDevice, FH_NUM_NON_SLEEPY_HOPPING_NEIGHBORS);

    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastDwellInterval, cfg_props.uc_dwell_interval);

    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastChannelFunction, cfg_props.uc_channel_function);

    if(cfg_props.uc_channel_function == 0) //fixed channel
    {
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_unicastFixedChannel, cfg_props.uc_fixed_channel);
    }
    else //DH1CF
    {
        uint8_t excludeChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
        uint8_t idx, sizeOfChannelMask;

        sizeOfChannelMask = sizeof(cfg_props.uc_channel_list)/sizeof(uint8_t);

        if(sizeOfChannelMask > APIMAC_154G_CHANNEL_BITMAP_SIZ)
        {
           sizeOfChannelMask = APIMAC_154G_CHANNEL_BITMAP_SIZ;
        }

        memset(excludeChannels, 0, APIMAC_154G_CHANNEL_BITMAP_SIZ);

        for(idx = 0; idx < sizeOfChannelMask; idx++)
        {
           excludeChannels[idx] = ~cfg_props.uc_channel_list[idx];
        }
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_unicastExcludedChannels,
                                excludeChannels);
    }


#ifdef FEATURE_FULL_FUNCTION_DEVICE

    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval, cfg_props.bc_dwell_interval);
    ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval, cfg_props.bc_interval);
    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction, cfg_props.bc_channel_function);
    if(cfg_props.bc_channel_function == 0) //fixed channel
    {
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_broadcastFixedChannel, cfg_props.bc_fixed_channel);
    }
    else //DH1CF
    {
        uint8_t excludeChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
        uint8_t idx, sizeOfChannelMask;

        sizeOfChannelMask = sizeof(cfg_props.bc_channel_list)/sizeof(uint8_t);
        if(sizeOfChannelMask > APIMAC_154G_CHANNEL_BITMAP_SIZ)
        {
           sizeOfChannelMask = APIMAC_154G_CHANNEL_BITMAP_SIZ;
        }

        memset(excludeChannels, 0, APIMAC_154G_CHANNEL_BITMAP_SIZ);

        for(idx = 0; idx < sizeOfChannelMask; idx++)
        {
           excludeChannels[idx] = ~cfg_props.bc_channel_list[idx];
        }
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels,
                                excludeChannels);
    }

#else
    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval, 0);
    ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval, 0);
    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction, 0);
    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_broadcastFixedChannel, 0);
    uint8_t excludeChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
    memset(excludeChannels, 0, APIMAC_154G_CHANNEL_BITMAP_SIZ);
    ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels,
                            excludeChannels);
#endif

}


/*!
 * @brief       This function process incoming ICall callback messages.
 *
 * @param       pMsg - pointer to the incoming message
 */
static uint16_t processIncomingICallMsg(macCbackEvent_t *pMsg)
{
    mlme_start_conf_t conf;
    mcps_data_conf_t data_conf;
    mcps_data_conf_payload_t data_conf_ie;
    mcps_data_ind_t data_ind;
    mcps_data_ie_list_t ie_ext;
    ws_pan_information_t pan_information;
    uint16_t pan_version;

    /* Only process message if callbacks are setup */
    if((!pMsg) || (pMsg->hdr.status == MAC_BAD_STATE) || (!mbed_mac_api))
    {
        return 0;
    }

#ifdef DBG_APP
      appDbg_event[appDbg_eidx] = (pMsg->hdr.event << 8)|pMsg->hdr.status;
      appDbg_eidx = (appDbg_eidx + 1) % 20;
#endif
    /* Determine the callback type */
    switch(pMsg->hdr.event)
    {
        case MAC_MLME_START_CNF:
            if(mbed_mac_api->mlme_conf_cb)
            {
                conf.status = MLME_SUCCESS;
                mbed_mac_api->mlme_conf_cb(mbed_mac_api, MLME_START, &conf);
            }
            break;
        case MAC_MLME_WS_ASYNC_CNF:
            if(mbed_mac_api->data_conf_ext_cb)
            {
                mbed_mac_api->data_conf_ext_cb(mbed_mac_api, (mcps_data_conf_t *)pMsg, NULL);
            }
            break;
        case MAC_MLME_WS_ASYNC_IND:
            if(mbed_mac_api->data_ind_ext_cb)
            {
#ifdef DBG_APP
                mcpsDbg.asynchInd[pMsg->asyncInd.internal.fhFrameType]++;
#endif
                if(pMsg->asyncInd.internal.fhFrameType == MAC_WS_ASYNC_PAN_ADVERT)
                {
                    if (!ws_wp_nested_pan_read(pMsg->asyncInd.mac.pPayloadIE + IE_HDR_LEN, pMsg->asyncInd.mac.payloadIeLen - IE_HDR_LEN, &pan_information)) {
                        return (1);
                    }
                }
                if(pMsg->asyncInd.internal.fhFrameType == MAC_WS_ASYNC_PAN_CONFIG)
                {
                    if (!ws_wp_nested_pan_version_read(pMsg->asyncInd.mac.pPayloadIE + IE_HDR_LEN, pMsg->asyncInd.mac.payloadIeLen - IE_HDR_LEN, &pan_version)) {
                        return (1);
                    }
                }
                memset(&data_ind, 0, sizeof(mcps_data_ind_t));
                timacConvertDataInd(&data_ind, &pMsg->asyncInd);
                ie_ext.headerIeList = pMsg->asyncInd.mac.pHeaderIE;
                ie_ext.headerIeListLength = pMsg->asyncInd.mac.headerIeLen;
                ie_ext.payloadIeList =  pMsg->asyncInd.mac.pPayloadIE;
                ie_ext.payloadIeListLength = pMsg->asyncInd.mac.payloadIeLen;
                mbed_mac_api->data_ind_ext_cb(mbed_mac_api, &data_ind, &ie_ext);
            }
            break;
        case MAC_MLME_COMM_STATUS_IND:
#ifdef DBG_APP
            mcpsDbg.data[4]++;
#endif
            break;
        case MAC_MCPS_DATA_CNF:
            if(mbed_mac_api->data_conf_ext_cb)
            {
#ifdef DBG_APP
                if(!pMsg->hdr.status)
                {
                    mcpsDbg.data[1]++;
                }
                else
                {
                    mcpsDbg.data[2]++;
                }
#endif
                memset(&data_conf, 0, sizeof(mcps_data_conf_t));
                memset(&data_conf_ie, 0, sizeof(mcps_data_conf_payload_t));
                timacConvertDataCnf(&data_conf, (macMcpsDataCnf_t *)&pMsg->ackCnf);
                data_conf_ie.headerIeList = pMsg->ackCnf.pHeaderIE;
                data_conf_ie.headerIeListLength = pMsg->ackCnf.headerIeLen;
                data_conf_ie.payloadIeList = pMsg->ackCnf.pPayloadIE;//technically this is holding the pointer to pRxBuf Now
                data_conf_ie.payloadIeListLength = pMsg->ackCnf.payloadIeLen;
                data_conf_ie.payloadPtr = NULL;
                data_conf_ie.payloadLength = 0;
                if(pMsg->ackCnf.pDataReq)
                {
                    /* Deallocate the original data request structure */
                    OsalPort_msgDeallocate((uint8_t*)pMsg->ackCnf.pDataReq);
                }
                mbed_mac_api->data_conf_ext_cb(mbed_mac_api, &data_conf, &data_conf_ie);

                /* free up the buffer holding the enhanced ACK packet here */
                /* refer also to comments mac_rx.c file rxFcsIsr() function */
                /* Deallocate the original data request structure */
                MAP_mac_msg_deallocate((uint8_t**)&pMsg->ackCnf.pPayloadIE);
            }
            break;

        case MAC_MCPS_DATA_IND:
            if(mbed_mac_api->data_ind_ext_cb)
            {
#ifdef DBG_APP
                mcpsDbg.data[3]++;
#endif
                memset(&data_ind, 0, sizeof(mcps_data_ind_t));
                timacConvertDataInd(&data_ind, &pMsg->dataInd);
                ie_ext.headerIeList = pMsg->dataInd.mac.pHeaderIE;
                ie_ext.headerIeListLength = pMsg->dataInd.mac.headerIeLen;
                ie_ext.payloadIeList =  pMsg->dataInd.mac.pPayloadIE;
                ie_ext.payloadIeListLength = pMsg->dataInd.mac.payloadIeLen;
                //Swap compressed address to broadcast when dst Address is elided
                if (data_ind.DstAddrMode == MAC_ADDR_MODE_NONE) {
                    data_ind.DstAddrMode = MAC_ADDR_MODE_16_BIT;
                    data_ind.DstAddr[0] = 0xff;
                    data_ind.DstAddr[1] = 0xff;
                }

                if((data_ind.SrcPANId != 0xffff) && (data_ind.DstPANId == 0))
                {
                    data_ind.DstPANId = data_ind.SrcPANId;
                }

                mbed_mac_api->data_ind_ext_cb(mbed_mac_api, &data_ind, &ie_ext);
            }
            break;

        case MAC_MCPS_PURGE_CNF:
            break;

        default:
            break;
    }

    return (0);
}

/*!
 Register for MAC callbacks.

 Public function defined in api_mac.h
 */
void eventOS_dispatch_timac_event(void)
{
    macCbackEvent_t *pMsg;
    if(appTaskId != MAC_UNDEFINED_TASKID)
    {
        /* Retrieve the response message */
        pMsg = (macCbackEvent_t*) OsalPort_msgReceive( appTaskId );
        if(pMsg != NULL)
        {
            /* Process the message from the MAC stack */
            processIncomingICallMsg(pMsg);
            OsalPort_msgDeallocate((uint8_t*)pMsg);
        }
    }
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

int8_t timac_event_handler = -1;

/*!
 * @brief       TI MAC event handler (tasklet)
 *
 * @param       event:  dispatched event
 *
 *
 * @return      None
 */
void timac_tasklet_event_handler(arm_event_s *event)
{
    switch (event->event_type) {
        case TIMAC_EVENT_INIT:
            //tr_debug("TIMAC Tasklet Generated");
            break;
        case TIMAC_EVENT_CALLBACK:
            //tr_debug("TIMAC Tasklet Callback event");
            eventOS_dispatch_timac_event();
            break;
        default:
            break;

    }
}

/*!
 * @brief       initialize the TIMAC tasklet
 *
 * @param       none
 *
 *
 * @return      none
 */
void timac_tasklet_init(void)
{
    if (timac_event_handler == -1) {
        // save the timac_event handle for later use in timacSignalEventLoop
        timac_event_handler = eventOS_event_handler_create(&timac_tasklet_event_handler, TIMAC_EVENT_INIT);
    }

}

/*!
 * send the TIMAC event to event loop
 */
void timacSignalEventLoop(void)
{
    uint8_t status;
    //post an event to timac tasklet with ARM_LIB_LOW_PRIORITY_EVENT
    arm_event_s event = {
           .sender = 0,
           .receiver = timac_event_handler,
           .priority = ARM_LIB_LOW_PRIORITY_EVENT,
           .event_type = TIMAC_EVENT_CALLBACK,
           .event_id = 0,
           .event_data = 0
       };

    status = eventOS_event_send(&event);
    if (status)
    {   // can't send event
        /* To Do : implement purge here ?
            MAC_ASSERT(0);
        */
        //tr_debug("TIMAC event send fails");
    }
}
