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
#else
/* POSIX Header files */
#include <pthread.h>
#include <FreeRTOS.h>
#include <semaphore.h>
#endif

#include "fh_map_direct.h"
#include "fh_pib.h"
#include "fh_nt.h"
#include "fh_api.h"

#include "ti_drivers_config.h"

#include <ti/drivers/GPIO.h>
#include <driverlib/ioc.h>

/* Using NanoStack VPIE parsing function */
#include "6LoWPAN/MAC/mac_ie_lib.h"
#include "6LoWPAN/ws/ws_common_defines.h"
#include "6LoWPAN/ws/ws_ie_lib.h"

//#define TIMAC_MPL_RETRY_BASEON_RX_DOWNSTREAM
//#define MAC_MPL_BACKTOBACK_TX
//#define ENABLE_GPIO_MPL

/*
 * for CC1312R7 use the following GPIO
 */
#define FH_UNICAST_GPIO             IOID_18
#define HOST_RX_GPIO                IOID_19
#define MAC_BROADCAST_GPIO          IOID_20
#define FH_BROADCAST_GPIO           IOID_21

#define RF_RX_GPIO                  IOID_23
#define RF_TX_GPIO                  IOID_24

void timac_setup_Test_GPIO(void);
void timac_Set_MAC_BROADCAST_GPIO(uint8_t st);

#ifdef MAC_DUTY_CYCLE_CHECKING
#include "mac_duty_cycle/mac_duty_cycle.h"
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

uint8_t enableVPIE = 0;
uint8_t detectVPIE = 0;
bool disableBCRequeue = false;
#ifdef FEATURE_DISABLE_FRAME_COUNT
bool disableFrameCountChecking = true;
#else
bool disableFrameCountChecking = false;
#endif

volatile MAC_VPIE_STATUS_t vpieStatus;
timac_rx_MPL_Data_Callback *pRxMPLCallback=NULL;
MAC_MPL_Handler_t macMplHnd;

/*!
 The ApiMac_extAddr is the MAC's IEEE address, setup with the Chip's
 IEEE addresses in main.c
 */
ApiMac_sAddrExt_t ApiMac_extAddr;
extern uint8_t timacTaskId;
extern sem_t event_thread_sem_handle;

extern configurable_props_t cfg_props;
extern int8_t eventOS_event_timer_request(uint8_t event_id, uint8_t event_type, int8_t tasklet_id, uint32_t time);

//fake pib variable to hold reg domain info
extern uint8_t regDomain;
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

#ifdef WISUN_TEST_METRICS
MAC_Perf_Data MacPerfData;
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

    timac_setup_Test_GPIO();

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
    if (pibAttribute == ApiMac_attribute_regDomain)
    {
        *pValue = regDomain;
        return ApiMac_status_success;
    }
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
 * @param       pLen - pointer to the read length
 *
 * @return      The status of the request
 */
ApiMac_status_t ApiMac_mlmeGetReqArrayLen(ApiMac_attribute_array_t pibAttribute,
                                          uint8_t *pValue,
                                          uint16_t *pLen)
{
    /* Duty Cycle PIB Variable */
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleBucket)
    {
        memcpy((DC_NODE_ENTRY_s *)pValue, dcBucket, sizeof(DC_NODE_ENTRY_s) * DUTY_CYCLE_BUCKETS);
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
    if (pibAttribute == ApiMac_attribute_regDomain)
    {
        regDomain = value;
        return ApiMac_status_success;
    }
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
#ifdef MAC_DUTY_CYCLE_CHECKING
    if (pibAttribute == ApiMac_attribute_dutyCycleBucket)
    {
        memcpy(dcBucket, (DC_NODE_ENTRY_s *)pValue, sizeof(DC_NODE_ENTRY_s) * DUTY_CYCLE_BUCKETS);
        return ApiMac_status_success;
    }
#endif
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

#else

ApiMac_status_t ApiMac_mlmeSetSecurityReqUint8(
                ApiMac_securityAttribute_uint8_t pibAttribute, uint8_t value)
{
    return ApiMac_status_unsupported;
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqUint16(
                ApiMac_securityAttribute_uint16_t pibAttribute, uint16_t value)
{
    return ApiMac_status_unsupported;
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqArray(
                ApiMac_securityAttribute_array_t pibAttribute, uint8_t *pValue)
{
    return ApiMac_status_unsupported;
}

/*!
 This direct execute function sets an attribute value
 in the MAC Security PIB.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeSetSecurityReqStruct(
                ApiMac_securityAttribute_struct_t pibAttribute, void *pValue)
{
    return ApiMac_status_unsupported;
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
#ifdef WISUN_TEST_METRICS
        MacPerfData.num_tx_unicast++;
#endif
    }
    else
    {
        tdataReq->txOptions.noRetransmits = true;
#ifdef DBG_APP
        mcpsDbg.data[5]++;
#endif

#ifdef WISUN_TEST_METRICS
        MacPerfData.num_tx_broadcast++;
#endif
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

uint16 timacDataInd_CheckVPIE(mcps_data_ind_t *mdataInd,macMcpsDataInd_t *tdataInd,uint8_t *vpieType,uint8_t *vp_len)
{
    uint8*    pPayloadIE;
    uint16    payloadIeLen;
    ws_vp_ie_t vp_ie;

    uint16    mpl_payload_test =0;
    bool      st;
    uint8_t  *pVpie,data,msgID;

    pPayloadIE      = tdataInd->mac.pPayloadIE;
    payloadIeLen    = tdataInd->mac.payloadIeLen;

    st = ws_wp_nested_vp_get(pPayloadIE, payloadIeLen, &vp_ie);
    if (st)
    {
        uint16_t len;
        vpieStatus.num_rx_vpie++;

        pVpie = vp_ie.ptrContent;
        len   = vp_ie.length;

        // save the VPIE content
        if (len > FH_VP_IE_LEN)
            len = FH_VP_IE_LEN;

        *vp_len = len;

        OsalPort_memcpy(FH_hnd.rxVPIE.payload,pVpie,len);

        /* VPIE Format
         *      Vendor ID : one byte
         *      msgID ID  : one byte
         *      msgContent:
         */

        data = *pVpie++;

        if (data == WISUN_VENDOR_ID_TI)
        {
            /* TI defined payload IE is detected */
            msgID = *pVpie++;

            /* save the VPIE type */
            *vpieType = msgID;

            if (msgID == VPIE_MSG_ID_PANID)
            {   /* VPIE with panID */
                detectVPIE = 1;
                vpieStatus.num_rx_vpie_panid++;

                /* save the RX panid */
                FH_hnd.rxVPIE.panID = *pVpie++;
                FH_hnd.rxVPIE.panID += (*pVpie++)<<8;

                /* update the local MAC PAN ID */
                pMacPib->panId = FH_hnd.rxVPIE.panID;
                MAP_macRadioSetPanID(pMacPib->panId);

                /* update the mdataInd Src and Dst Panid */
                mdataInd->DstPANId = FH_hnd.rxVPIE.panID;
                mdataInd->SrcPANId = FH_hnd.rxVPIE.panID;
            }
            else if (msgID == VPIE_MSG_ID_MPL_PAYLOAD)
            {   /* VPIE with MPL payload */
                vpieStatus.num_rx_vpie_mpl++;
            }

            /* there is TI VPIE in the payload IE */
            return 1;
        }
    }

    /* there is no VPIE in the payload */
    return 0;

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
        if (enableVPIE == 1)
        {
            uint16_t panid;

            tdataReq->includeFhIEs |= MAC_FH_VP_IE;
            MAC_MlmeGetReq(ApiMac_attribute_panId, &panid);
            /* setup VPIE for Pan ID */
            tdataReq->Vpie_length = timacSet_TX_VPIE_PanID (panid);
            tdataReq->Vpie_type = VPIE_MSG_ID_PANID;
            tdataReq->pVpie_data = FH_hnd.txVPIE.payload;
        }
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

void timacInsertVPIE(ApiMac_mcpsDataReq_t *tdataReq)
{
    if (tdataReq->includeFhIEs & MAC_FH_US_IE )
    {   /* data request is unicat packet */
        if (enableVPIE == 1)
        {
            uint16_t panid;

            tdataReq->includeFhIEs |= MAC_FH_VP_IE;
            MAC_MlmeGetReq(ApiMac_attribute_panId, &panid);

            tdataReq->Vpie_type = VPIE_MSG_ID_PANID;    /* PAN ID */
            tdataReq->Vpie_length = timacSet_TX_VPIE_PanID (panid);
            tdataReq->pVpie_data = FH_hnd.txVPIE.payload;
        }
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
    tr_info("Set MAC PAN-ID 0x%x:",panId);
    ApiMac_mlmeSetReqUint16(ApiMac_attribute_panId, panId);
}

void timacSetTrackParent(uint8_t* eui64)
{
    tr_info("Set MAC Track Parent eui64: %s", trace_array(eui64, 8));
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

    buffer->device_decription_table_size = NANOSTACK_DEVICE_TABLE_ENTRIES;
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

#ifdef FEATURE_MAC_SECURITY
void mac_delete_Device_FH_NT_entry(const mlme_device_descriptor_t *device_desc, deviceIndexDesc_t *pDevice)
{
    uint16_t deviceIndex;
    deviceDescriptor_t *pDeviceTable;
    sAddrExt_t *pDeviceEUI;
    FHAPI_status status;
    mlme_device_descriptor_t AllFFDev;

    if  ((enableVPIE == 1) || (detectVPIE==1 ) )
    {   /* in VPIE PANID mode , don't delete entry */
        return;
    }

    deviceIndex = pDevice->deviceIndex;

    /* check if the deviceDesc is all 0xFF
     *
     */
    MAP_osal_memset(&AllFFDev,0xFF,sizeof(mlme_device_descriptor_t));

    if (MAP_osal_memcmp(device_desc, &AllFFDev, sizeof(mlme_device_descriptor_t)))
    {   /* Nanostack want to delete this device
           from device index to get the EUI
        */
        uint8_t macAddr[8];

        pDeviceTable = ((deviceDescriptor_t *)pMacSecurityPib->macDeviceTable) + deviceIndex;
        pDeviceEUI = &(pDeviceTable->extAddress);

        memcpy(macAddr,pDeviceEUI,8);
        // call FH API to delete this device from entry
        status = FHNT_deleteEntry(pDeviceEUI);

        tr_info("Delete FH entry with EUI %s with status(%d)",trace_array(macAddr, 8),status);
    }
}
#endif

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
                        /*
                         * try to delete the FH NT entry
                         */
                        mac_delete_Device_FH_NT_entry(device_desc,&deviceDesc);
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

uint8_t tx_packet_num=0;

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

#ifdef WISUN_TEST_METRICS
        MacPerfData.num_asynchReq[asynch_req->message_type]++;
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

        tx_packet_num++;
        status = MAC_McpsDataReq(&timacDataReq);

        if(timacDataReq.txOptions.ack)
        {
            uint8_t macAddr[8];
            memcpy(macAddr,timacDataReq.dstAddr.addr.extAddr,8);
            tr_debug("\n MAC MCPs Data Req : unicast to Dst %s TxPkt(0x%x), st(%d)",trace_array(macAddr, 8),tx_packet_num,status);
        }
        else
        {
            tr_debug("\n MAC MCPs Data Req : broadcast TxPkt(0x%x), st(%d)",tx_packet_num,status);
            /*
             * toggle the GPIO to high
             */
            timac_Set_MAC_BROADCAST_GPIO(1);
        }

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
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_regDomain, cfg_props.config_reg_domain);
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

#ifdef MAC_DUTY_CYCLE_CHECKING
    ApiMac_mlmeSetReqBool(ApiMac_attribute_dutyCycleEnabled, true);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleRegulated,
                            DUTY_CYCLE_MEAS_PERIOD*MAC_DUTY_CYCLE_THRESHOLD/100);

    /* Critical and limited duty cycle modes unused, set to max
     * value to avoid entering state */
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleCritical,
                            UINT32_MAX);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleLimited,
                            UINT32_MAX);
#endif

#ifdef MAC_OVERRIDE_TX_DELAY
    ApiMac_mlmeSetReqBool(ApiMac_attribute_customMinTxOffEnabled, true);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_minTxOffTime,
                            (uint32_t)MAC_CONFIG_MIN_TX_OFF);
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
    uint8_t vpIEType,vpIELen;

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

#ifdef WISUN_TEST_METRICS
                MacPerfData.num_asynchInd[pMsg->asyncInd.internal.fhFrameType]++;
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

#ifdef WISUN_TEST_METRICS
                // save the cnf status
                if (pMsg->hdr.status == ApiMac_status_success )
                {
                    MacPerfData.num_tx_conf_ok++;
                }
                else if (pMsg->hdr.status == FHAPI_STATUS_ERR_NO_ENTRY_IN_THE_NEIGHBOR)
                {
                    MacPerfData.num_tx_conf_no_entry++;
                }
                else if (pMsg->hdr.status == ApiMac_status_channelAccessFailure)
                {
                    MacPerfData.num_tx_conf_chan_busy++;
                }
                else if (pMsg->hdr.status == ApiMac_status_noAck)
                {
                    MacPerfData.num_tx_conf_no_ack++;
                }
                else
                {
                    MacPerfData.num_tx_conf_other++;
                }
#endif
                if (pMsg->ackCnf.ackOption == 0)
                {
                    tr_info("\n MAC MCPs Data Cnf status broadcast %u TxPkt(0x%x): ",pMsg->hdr.status,tx_packet_num);
                    /*
                     * set GPIO to zero
                     */
                    timac_Set_MAC_BROADCAST_GPIO(0);
                }
                else
                {
                    tr_info("\n MAC MCPs Data Cnf status Unicast %u TxPkt(0x%x): ",pMsg->hdr.status,tx_packet_num);
                }
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

                if ( (pMsg->ackCnf.priority == MAC_DATA_EXPEDITE_FORWARD) && (pMsg->ackCnf.ackOption == 0))
                {
                    vpieStatus.num_tx_conf++;
                    /* don't send to Application */
                }
                else
                {
                    mbed_mac_api->data_conf_ext_cb(mbed_mac_api, &data_conf, &data_conf_ie);
                }
                /* free up the buffer holding the enhanced ACK packet here */
                /* refer also to comments mac_rx.c file rxFcsIsr() function */
                /* Deallocate the original data request structure */
                MAP_mac_msg_deallocate((uint8_t**)&pMsg->ackCnf.pPayloadIE);
            }
            break;

        case MAC_MCPS_DATA_IND:
            if(mbed_mac_api->data_ind_ext_cb)
            {
                uint16_t vpIEStatus;
#ifdef DBG_APP
                mcpsDbg.data[3]++;
#endif

#ifdef WISUN_TEST_METRICS
                MacPerfData.num_rxInd++;
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

                tr_info("\n MAC MCPs Data Ind Src %s srcPanID %u:", trace_array(data_ind.SrcAddr, 8), data_ind.SrcPANId);

                vpIEStatus = timacDataInd_CheckVPIE(&data_ind,&pMsg->dataInd,&vpIEType,&vpIELen);
                /* check if the srcPanID match the MACPib pan ID */
                if ( (macPib.panId !=0xFFFF) && (macPib.panId == data_ind.SrcPANId) )
                {
                    if ( (vpIEStatus) && (vpIEType == VPIE_MSG_ID_MPL_PAYLOAD) )
                    {
                        uint8_t st;
                        // invoke MAC MPL handler
                        st = timac_handle_RX_MPL(vpIELen);

                    }
                    else
                    {
                        mbed_mac_api->data_ind_ext_cb(mbed_mac_api, &data_ind, &ie_ext);
                    }
                }
                else
                {   /* the packet is from different PAN, need to drop */
                    vpieStatus.err_num_rx_from_different_pan++;
                    tr_cmdline(">>> Data Ind srcPanID %u: my PanID(%u)", data_ind.SrcPANId,macPib.panId);
                }
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
uint32_t timac_one_second_timeout = 1000;

uint32_t timac_TX_MPL_retry_timeout= 150;

void timac_GetBC_Slot_BFIO(uint16_t *slot, uint32_t *bfio);

#define TIMAC_NANO_STACK_ONE_SECOND
/*!
 * @brief       TI MAC event handler (tasklet)
 *
 * @param       event:  dispatched event
 *
 *
 * @return      None
 */

volatile uint8_t mpl_user_test_data[6];

void timac_set_mpl_test (uint8_t st)
{
    macMplHnd.start_mpl_test = st;
}

uint8_t timac_get_mpl_test(void)
{
    return macMplHnd.start_mpl_test;
}

void timac_tasklet_event_handler(arm_event_s *event)
{
    switch (event->event_type) {
        case TIMAC_EVENT_INIT:
            //tr_debug("TIMAC Tasklet Generated");
#ifdef TIMAC_NANO_STACK_ONE_SECOND
            /* start the one second timer*/
            eventOS_event_timer_request(0, TIMAC_EVENT_TIMERR, timac_event_handler, timac_one_second_timeout);
#endif
            break;
        case TIMAC_EVENT_CALLBACK:
            //tr_debug("TIMAC Tasklet Callback event");
            eventOS_dispatch_timac_event();
            break;
#ifdef TIMAC_NANO_STACK_ONE_SECOND
        case TIMAC_EVENT_TIMERR:
            /* start the same timer event */
            eventOS_event_timer_request(0, TIMAC_EVENT_TIMERR, timac_event_handler, timac_one_second_timeout);

            if (macMplHnd.firstFlagTimeoutCnt > 0)
            {
                macMplHnd.firstFlagTimeoutCnt--;

                if (macMplHnd.firstFlagTimeoutCnt == 0)
                {   // reset the flag
                    timac_resetMpl_Seq();
                }
            }

            if (macMplHnd.start_mpl_test)
            {
                uint8_t st;
                uint16_t slotIdx;
                uint32_t bfio;

                // toggle LED
                GPIO_toggle(CONFIG_GPIO_GLED);

                /* construct the MPL user test payload */
                /* using slot-idx + BFIO for timing measurement*/
                timac_GetBC_Slot_BFIO(&slotIdx, &bfio);
                mpl_user_test_data[0] = slotIdx;
                mpl_user_test_data[1] = slotIdx>>8;

                mpl_user_test_data[2] = bfio;
                mpl_user_test_data[3] = bfio>>8;
                mpl_user_test_data[4] = bfio>>16;
                mpl_user_test_data[5] = bfio>>24;

                tr_cmdline("TxSeq(%d Tx(%d,%d)) ",macMplHnd.mpl_seq,slotIdx,bfio);
                //invoke send MPL API
                st = timac_Send_MPL((uint8_t *)mpl_user_test_data, sizeof(mpl_user_test_data));

            }
            break;
#endif

#ifdef TIMAC_MPL_RETRY_BASEON_RX_DOWNSTREAM
        case TIMAC_EVENT_TX_MPL_RETRY_TIMER:
            /* MAC TX MPL retry timer */
            timac_handle_TX_MPL_Retry(event->event_id);

            break;
#endif
        default:
            break;

    }
}

void timac_rx_MPL_test_callback(uint8 *pdata,uint8 len,MAC_MPL_Header_t *pRxHeader);

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

    /* register the call back function */
    timac_register_MPLCallBack(&timac_rx_MPL_test_callback);

    /* init MAC MPL handler */
    timac_InitMPL(&macMplHnd);
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
    }
}

uint16_t timacSet_TX_VPIE_PanID (uint16_t network_panid)
{
    uint16_t index;

    /* prefill the BE pattern */
    memset(FH_hnd.txVPIE.payload, 0xBE,FH_VP_IE_LEN);

    /*
     * Vendor-ID + msgID (PAN ID), panID (2 bytes)
     */

    index =0;
    FH_hnd.txVPIE.payload[index++] = WISUN_VENDOR_ID_TI;

    FH_hnd.txVPIE.payload[index++] = VPIE_MSG_ID_PANID;
    // network PAN ID
    FH_hnd.txVPIE.payload[index++] = network_panid;
    FH_hnd.txVPIE.payload[index++] = (network_panid)>>8;

    // terminator
    //FH_hnd.txVPIE.payload[index++] = VPIE_MSG_ID_TERMINATOR;

    return index;
}

void timac_LogEvent(uint32_t eventID,uint32_t timestamp,uint32_t st1,uint32_t st2,uint32_t st3,uint32_t st4)
{
    uint8_t addr[8];

    if (eventID == MAC_EVENT_LOG_ID_TX_BROADCAST_CHAN )
    {
        tr_info("MAC Broadcast TX  slot(%d) BFIO(%d) chan:%u backoffDur:%u status: %u", st3,st4, (st1 & 0xFFFF), st1>>16, st2);
    }
    else if (eventID == MAC_EVENT_LOG_ID_TX_UNICAST_CHAN )
    {
        tr_info("MAC Unicast TX slot(%d) BFIO(%d) chan:%u backoffDur:%u status:%u", st3, st4, (st1 & 0xFFFF), st1>>16, st2);
    }
    else if (eventID == MAC_EVENT_LOG_ID_START_BS_PANID)
    {

        memcpy(addr, &st2, 4);
        memcpy(addr + 4, &st3, 4);

        tr_info("BS changed, slot(%d) BFIO(%d), by PAN ID:%u, EUI: %x%x%x%x",(st1>>16) ,st4, (st1 & 0xFFFF), addr[4], addr[5], addr[6], addr[7]);
    }
    else if (eventID == MAC_EVENT_LOG_ID_RX_SYNC_ERROR)
    {
        /* MAC RX CB Sync error */
        tr_info("MAC RX-CB Sync Error, e-hig (%x) e-low(%x) error-cnt(%d),RFStatus(%x)",st1,st2,st3,st4);
    }
    else if (eventID == MAC_EVENT_LOG_ID_RX_PKT_BC_SLOT_BFIO)
    {
        /* RX packet BC slot and BFIO, Phy channel) */
        tr_info("MAC RX: BC slot (%d) BFIO (%d), ARQ(%d) PHY-Chan(%d)",st1,st2,st4,st3);
    }
#ifdef FH_DEBUG_NT_SIZE
    else if (eventID == MAC_EVENT_LOG_ID_FH_NT)
    {
        memcpy(addr, &st2, 4);
        memcpy(addr + 4, &st3, 4);

        if (st1 == MAC_EVENT_LOG_ID_FH_NT_CREATE_SUCCESS)
        {
            tr_info("FH NT: create entry success, EUI: %x%x%x%x%x%x%x%x  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_CREATE_REPLACE)
        {
            tr_info("FH NT: replace old entry, EUI: %x%x%x%x%x%x%x%x  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_CREATE_FAIL)
        {
            tr_info("FH NT: create entry failed, EUI: %x%x%x%x%x%x%x%x  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_PUT_SUCCESS)
        {
            tr_info("FH NT: put entry success, EUI: %x%x%x%x%x%x%x%x  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_PUT_FAIL)
        {
            tr_info("FH NT: put entry fail, no entry found, EUI: %x%x%x%x%x%x%x%x  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_HIE_FAIL_NO_ENTRY)
        {
            tr_info("FH NT: HIE UTIE failed, entry not found, EUI: %x%x%x%x%x%x%x%x  ", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_HIE_FAIL_LEN)
        {
            tr_info("FH NT: HIE UTIE failed, invalid len ");
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_PIE_FAIL_FORMAT)
        {
            tr_info("FH NT: PIE USIE failed, invalid format ");
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_PIE_FAIL_CREATE)
        {
            tr_info("FH NT: PIE USIE failed, could not create entry ");
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_CREATE_NUM_NODE)
        {
            tr_info("FH NT: create entry start. Curr num entries: %d, max num entries: %d ", st2, st3);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_DELETE_ENTRY)
        {
            tr_info("FH NT: delete FhType (%d) entry EUI: %x%x%x%x%x%x%x%x  ", st4,
                    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_CREATE_JOIN_NUM_NODE)
        {
            tr_warn("FH NT: create join. Join entries: %d, Nonsleepy entries: %d ", st2, st3);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_DELETE_NUM_NODE)
        {
            tr_warn("FH NT: delete. Join entries: %d, Nonsleepy entries: %d ", st2, st3);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_DELETE_JOIN_NUM_NODE)
        {
            tr_warn("FH NT: delete join. Join entries: %d, Nonsleepy entries: %d ", st2, st3);
        }
        else if (st1 == MAC_EVENT_LOG_ID_FH_NT_RESTORE_NUM_NODE)
        {
            tr_warn("FH NT: restore. Join entries: %d, Nonsleepy entries: %d ", st2, st3);
        }
    }
#endif
    else
    {
        //tr_info("CSMA computed TX chan:%u status: %u", chanNum, status);
    }
}

void timac_BootstrapCallback(uint8_t state)
{
    tr_info("====== MAC Bootstrap CB State (%d)", state);
    FH_hnd.joinState = state;
}

void timac_BootstrapCallbackMode(uint8_t BR_node)
{
    /* save node' role */
    macMplHnd.BRNode = BR_node;
}

#ifdef WISUN_TEST_METRICS
extern FHNT_HND_s FHNT_hnd;
/*!
 * @brief       TI MAC Get MAC Debug counts
 *
 * @param       pMacDebugCnts:  pointer to MAC debug counts to be filled
 *
 *
 * @return      None
 */
void timac_getMACDebugCounts(MAC_debug_s *pMacDebugCnts)
{
    if (pMacDebugCnts)
    {
        pMacDebugCnts->max_device_table_size = MAX_DEVICE_TABLE_ENTRIES;
        pMacDebugCnts->fhnt_curr_num_node = FHNT_hnd.numNonSleepNode;
    }
}
/*!
 * @brief       TI MAC Get MAC Performance data
 *
 * @param       pmacPerfData:  pointer to MAC Performance data to be filled
 *
 *
 * @return      None
 */
void timac_getMACPerfData(MAC_Perf_Data *pMacPerfData)
{
    if (pMacPerfData)
    {
        memcpy(pMacPerfData, &MacPerfData, sizeof(MAC_Perf_Data));
    }
}
#endif

void timac_GetBC_Slot_BFIO(uint16_t *slot, uint32_t *bfio)
{
    FHAPI_status st;
    st = FHUTIL_getCurBfio(bfio, slot);
}

void timac_setup_Test_GPIO(void)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_setConfig(FH_UNICAST_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(HOST_RX_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(MAC_BROADCAST_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(FH_BROADCAST_GPIO, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    // Map LNA PIN RFC_GPO0
    IOCPortConfigureSet(RF_RX_GPIO, IOC_PORT_RFC_GPO0,IOC_IOMODE_NORMAL);

    // Map TX start pin RFC_GPO3
    IOCPortConfigureSet(RF_TX_GPIO, IOC_PORT_RFC_GPO3,IOC_IOMODE_NORMAL);
#endif
}

void timac_Set_FH_Broadcast_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(FH_BROADCAST_GPIO,st);
#endif
}

void timac_Set_FH_UNICAST_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(FH_UNICAST_GPIO,st);
#endif
}

void timac_Set_HOST_RX_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(HOST_RX_GPIO,st);
#endif
}

void timac_Set_MAC_BROADCAST_GPIO(uint8_t st)
{
#ifdef ENABLE_GPIO_MPL
    GPIO_write(MAC_BROADCAST_GPIO,st);
#endif
}

void timac_register_MPLCallBack(timac_rx_MPL_Data_Callback *pCallback)
{
    pRxMPLCallback = pCallback;
}

void timac_InitMPL(MAC_MPL_Handler_t *pMacMplHnd)
{
    memset(pMacMplHnd, 0x00,sizeof(MAC_MPL_Handler_t));
    /* set the default TX Hop Limit */
    pMacMplHnd->mpl_hop_limit = TIMAC_MPL_HOP_LIMIT;

    /* set up the initail seq to 1 */
    pMacMplHnd->mpl_seq = 1;
}

void timac_resetMpl_Seq(void)
{
    /* reset the first_rx_mpl flag
     * so that we treat we did not receive the initial packet
     */
    macMplHnd.first_rx_mpl = 0;
    tr_debug("reset MAC MPL first rx flag");
}

/*
 * Public API:
 * It is used to send the MPL data by BR
 * pBuf: pointer to user payload buffer
 * len:  buffer size (max packet length is 8 bytes)
 * Limitation: This API is only used by BR
 */
uint8_t timac_Send_MPL(uint8 *pBuf,uint8 len)
{
    uint8_t st,pktIdx, i,numBacktoBack;
    MAC_MPL_Header_t *ptxMPL_Header;
    MAC_MPL_RETRY_PKT_s *pRetryPkt;

    ptxMPL_Header = &macMplHnd.txMPL_Header;

    /* update the MAC MPL data header */
    ptxMPL_Header->hopLimit = macMplHnd.mpl_hop_limit;
    ptxMPL_Header->seq      = macMplHnd.mpl_seq++;
    MAP_sAddrExtCpy(ptxMPL_Header->orignator, macPib.extendedAddress.addr.extAddr);

    /* set up the user data payload */
    if (len > TIMAC_MAX_MPL_USER_PAYLOAD)
    {
        return 1;
    }
    ptxMPL_Header->pData = pBuf;
    ptxMPL_Header->len   = len;

    /* save the user payloader and  MPL header
       copy user payloader to retry queue
       save the header
   */
    pktIdx = macMplHnd.txQueIdx;
    pRetryPkt = &macMplHnd.txQueue[pktIdx];

    memcpy(pRetryPkt->txPayload,pBuf,len);
    memcpy(&(pRetryPkt->txMPL_header),ptxMPL_Header,sizeof(MAC_MPL_Header_t));
    /* update the buffer pointer */
    pRetryPkt->txMPL_header.pData = pRetryPkt->txPayload;
    /* set up the retry count */
    pRetryPkt->RetryCnt = TIMAC_MAX_MPL_RETRY_CNT;

    /* update the index */
    macMplHnd.txQueIdx++;
    if (macMplHnd.txQueIdx == TIMAC_MPL_TX_QUEUE_SIZE)
        macMplHnd.txQueIdx =0;

#ifdef MAC_MPL_BACKTOBACK_TX
    numBacktoBack = 2;
#else
    numBacktoBack = 1;
#endif

    for (i = 0; i<numBacktoBack;i++)
    {
        //invoke MPL data request
        st = timac_MPL_DataReq(ptxMPL_Header);
        if (st)
        {
            vpieStatus.err_num_tx++;
        }
    }
#ifdef TIMAC_MPL_RETRY_BASEON_RX_DOWNSTREAM
    /* start the TX MPL retry timer
     * event_id to TX packet index in the queue
     * when RETRY timeout, we will use this index to find the corresponding packet
     * */
    eventOS_event_timer_request(pktIdx, TIMAC_EVENT_TX_MPL_RETRY_TIMER, timac_event_handler, timac_TX_MPL_retry_timeout);
#endif
    return st;
}

uint8_t timac_Is_Seq_In_RxSeq(uint8_t txSeq)
{
    uint8_t i;

    for (i=0;i<RX_MPL_SEQ_QUEUE_SIZE;i++)
    {
        if (macMplHnd.rxSeqQueue[i] == txSeq)
        {
            /* found */
            return 1;
        }
    }

    return 0;

}

#ifdef TIMAC_MPL_RETRY_BASEON_RX_DOWNSTREAM
uint8_t timac_Is_Detected_DownStream_RxSeq(uint8_t Seq,uint16_t hopLimitBitmap)
{
    uint8_t i;
    MAC_MPL_RX_Header_s *pRxHeader;

    /* is the rxSeq in the queue  */
    for (i=0;i<RX_MPL_SEQ_QUEUE_SIZE;i++)
    {
        pRxHeader = &(macMplHnd.rxHeader[i]);

        if  ( (pRxHeader->seq == Seq ) && ( (pRxHeader->hopLimitBitMap) & hopLimitBitmap))
        {   /* found the seq from downstream */
            return 1;
        }
    }

    /* not found */
    return 0;
}

uint8_t timac_handle_TX_MPL_Retry(uint8_t pktIdx)
{
    uint8_t st, txSeq,hopLimit;
    uint16_t hopLimitBitmap;

    MAC_MPL_RETRY_PKT_s *pRetryPkt;

    pRetryPkt = &macMplHnd.txQueue[pktIdx];

    txSeq       = pRetryPkt->txMPL_header.seq;
    hopLimit    = pRetryPkt->txMPL_header.hopLimit;

    /* when downstream forward this packet, the hopLimit will decrement by one */
    hopLimit = hopLimit -1;
    hopLimitBitmap = 1 << hopLimit;

    /* check if the Received seq queue has this seq */
    //st = timac_Is_Seq_In_RxSeq(txSeq);
    st = timac_Is_Detected_DownStream_RxSeq(txSeq,hopLimitBitmap);
    if (st == 1)
    {   /* we don't need to retry because this packet is trasnmit by one node */

        /* reset the rerty cnt to zero */
        pRetryPkt->RetryCnt = 0;
        //tr_cmdline("+++ No Need to retry, TX-Seq (%d)  is in RxSeq Que",txSeq);
        vpieStatus.num_tx_no_need_retry++;
        return 1;
    }

    /* check if retry count is zero */
    if (pRetryPkt->RetryCnt > 0)
    {
        pRetryPkt->RetryCnt--;

        /* retransmit this packet */
        //invoke MPL data request
        vpieStatus.num_tx_retry++;

        st = timac_MPL_DataReq(&(pRetryPkt->txMPL_header));
        if (st)
        {
            vpieStatus.err_num_tx_retry_fail++;
            return 0;
        }

        tr_cmdline("+++ TxSeq(%d) retry-Cnt(%d)",txSeq,pRetryPkt->RetryCnt);
        /* start the TX MPL retry timer
         * event_id to TX packet index in the queue
         * when RETRY timeout, we will use this index to find the corresponding packet
         * */
        eventOS_event_timer_request(pktIdx, TIMAC_EVENT_TX_MPL_RETRY_TIMER, timac_event_handler, timac_TX_MPL_retry_timeout);

    }
    else
    {   /* no more rerty */
        vpieStatus.err_num_retry_limit++;
        tr_cmdline("+++ TxSeq(%d) reach retry-Cnt limit",txSeq);
    }

    return 1;

}
#endif

uint16_t timac_Set_TX_VPIE_MPL_Payload (MAC_MPL_Header_t *pMPL)
{
    uint16_t index;

    /* prefill the BE pattern */
    memset(FH_hnd.txVPIE.payload, 0xBE,FH_VP_IE_LEN);

    index =0;
    FH_hnd.txVPIE.payload[index++] = WISUN_VENDOR_ID_TI;

    /*
     * msgID        :  (MPL_PAYLOAD), one byte
     * HopLimt      :  one byte
     * Seq          :  one byte
     * originator   :  8 byte
     * MPL payloader
     */
    FH_hnd.txVPIE.payload[index++] = VPIE_MSG_ID_MPL_PAYLOAD;

    // HopLimit
    FH_hnd.txVPIE.payload[index++] = pMPL->hopLimit;
    // seq
    FH_hnd.txVPIE.payload[index++] = pMPL->seq;
    // originator
    memcpy(&FH_hnd.txVPIE.payload[index],pMPL->orignator,8);
    index+=8;

    // copy user payload
    memcpy(&FH_hnd.txVPIE.payload[index],pMPL->pData,pMPL->len);
    index += pMPL->len;
    // terminator (remove the terminator)
    //FH_hnd.txVPIE.payload[index++] = VPIE_MSG_ID_TERMINATOR;

    return index;
}

ApiMac_mcpsDataReq_t mpl_mcpsDataReq;

/* internal API */
uint8_t timac_MPL_DataReq(MAC_MPL_Header_t *pMplHeader)
{
    uint16_t panId;
    ApiMac_status_t st;

    /*
     * prefill the Data Request data structure
     */
    memset(&mpl_mcpsDataReq,0x00,sizeof(mpl_mcpsDataReq));

    /* dstAddr default is None */

    /* dstPanID */
    MAC_MlmeGetReq(MAC_PAN_ID, &panId);
    mpl_mcpsDataReq.dstPanId = panId;

    /* set up MAC source address */
    mpl_mcpsDataReq.srcAddrMode = ApiMac_addrType_extended;

    mpl_mcpsDataReq.msduHandle = macMplHnd.mpl_msduHandle++;

    /* all other options are zero, except no_retrans */
    mpl_mcpsDataReq.txOptions.noRetransmits = 1;

    /*pIEList is None */
    mpl_mcpsDataReq.pIEList = NULL;
    mpl_mcpsDataReq.payloadIELen = 0;

    /* UT/BT IE, US/BS/VP IE, no MPX IE */
    mpl_mcpsDataReq.includeFhIEs = MAC_FH_UT_IE  | MAC_FH_BT_IE |
                                   //MAC_FH_US_IE  | MAC_FH_BS_IE |
                                   MAC_FH_VP_IE;

    /* no user payload in MAC*/
    mpl_mcpsDataReq.msdu.p = NULL;
    mpl_mcpsDataReq.msdu.len =0;

    /* MAC security
     * key source is zero
     * MAC_SEC_LEVEL_ENC_MIC_64    0x06
     *  ===AES encryption and MIC-64 authentication are used
     */
    mpl_mcpsDataReq.sec.securityLevel = MAC_SEC_LEVEL_ENC_MIC_64;
    //mpl_mcpsDataReq.sec.securityLevel = 0;
    mpl_mcpsDataReq.sec.keyIdMode = 1;
    mpl_mcpsDataReq.sec.keyIndex  = 1;

    /* set priority to highest */
    mpl_mcpsDataReq.priority = MAC_DATA_EXPEDITE_FORWARD;

    vpieStatus.num_tx_vpie++;
    vpieStatus.num_tx_vpie_mpl++;

    /* set up VPIE */
    mpl_mcpsDataReq.Vpie_length = timac_Set_TX_VPIE_MPL_Payload(pMplHeader);

    mpl_mcpsDataReq.Vpie_type = VPIE_MSG_ID_MPL_PAYLOAD;
    mpl_mcpsDataReq.pVpie_data = FH_hnd.txVPIE.payload;

    // set the GPIO to high
    timac_Set_MAC_BROADCAST_GPIO(1);

    /* check if the FH is in Async state */
    if (FHAPI_IsAsyncState())
    {   // call Async Stop
        vpieStatus.num_async_state_when_tx++;

        FHAPI_stopAsync();
    }

    /* call Datareq API */
    st = ApiMac_mcpsDataReq(&mpl_mcpsDataReq);
    tr_debug("\n timac_MPL_DataReq Data Req : st(%d)",st);
    return st;
}

/* internal API
 *
 */
void timac_update_rxInfo(MAC_MPL_Header_t *prxMPL_header)
{
    uint8_t idx;
    uint8_t rxSeq,hopLimit;
    uint8_t i;
    MAC_MPL_RX_Header_s *pRxHeader;

    /* update the prev_seq */
    vpieStatus.prev_seq = prxMPL_header->seq;

    rxSeq =  prxMPL_header->seq;
    hopLimit = prxMPL_header->hopLimit;

    /* is the rxSeq in the queue  */
    for (i=0;i<RX_MPL_SEQ_QUEUE_SIZE;i++)
    {
        pRxHeader = &(macMplHnd.rxHeader[i]);

        if (pRxHeader->seq == rxSeq )
        {   /* rxSeq is in rx header queue, set hoplimit bitmap */
            pRxHeader->hopLimitBitMap |= (1<<hopLimit);
            return;
        }
    }

    /* add the RX inform */
    idx = macMplHnd.rxHeaderIdx;
    pRxHeader = &(macMplHnd.rxHeader[idx]);
    pRxHeader->seq = rxSeq;
    pRxHeader->hopLimitBitMap = (1<<hopLimit);

    /* update the idx */
    idx = idx +1;
    if (idx == RX_MPL_SEQ_QUEUE_SIZE )
        idx =0;

    macMplHnd.rxHeaderIdx = idx;
}

/* internal API
 *
 */
uint8_t timac_handle_RX_MPL(uint8 len)
{
    uint16_t index;
    uint8_t st=0;
    uint8_t rxSeqIdx;
    MAC_MPL_Header_t *ptxForwardHeader;
    MAC_MPL_Header_t *prxMPL_header;

    ptxForwardHeader = &macMplHnd.txForwardHeader;
    prxMPL_header    = &macMplHnd.rxMPL_header;

    macMplHnd.num_rx_mpl++;

    /* update the RX MPL timestamp */
    vpieStatus.prev_rx_timestamp =  MAP_ICall_getTicks();

    /* from rx MPL payload to retrieve MPL header */
    index = 2 ;
    /* Vendor ID    :
     * msgID        :  (MPL_PAYLOAD), one byte
     * HopLimt      :  one byte
     * Seq          :  one byte
     * originator   :  8 byte
     * MPL payloader
     */
    prxMPL_header->hopLimit = FH_hnd.rxVPIE.payload[index++];
    prxMPL_header->seq      = FH_hnd.rxVPIE.payload[index++];

    memcpy(prxMPL_header->orignator,&FH_hnd.rxVPIE.payload[index],8);
    index +=8;

    /* get the payload len (no terminator) */
    prxMPL_header->pData = &FH_hnd.rxVPIE.payload[index];
    prxMPL_header->len = len - index;

    /* set the first flag timeout */
    macMplHnd.firstFlagTimeoutCnt = TIMAC_FIRST_FLAG_TIMEOUT_CNT;

    /* check the originator address if it matches local address */
    if (memcmp(prxMPL_header->pData, macPib.extendedAddress.addr.extAddr,8) == 0 )
    {   /* BR received the MPL */
        vpieStatus.num_match_originator++;

        /* save the seq info */
        timac_update_rxInfo(prxMPL_header);

        /* BR should not forward any received MPL
         * we may use this message to see if we need to resend
         */
        return st;
    }

    /* check seq for packet duplication */
    if (macMplHnd.first_rx_mpl == 0)
    {   //* this is the first RX MPL packet
        macMplHnd.first_rx_mpl = 1;
    }
    else
    {
        uint8_t minSeq;
        int8 diff,diff1;

        if (vpieStatus.prev_seq == prxMPL_header->seq)
        {
            vpieStatus.num_seq_duplicatd++;

            /* don't forward */
            return st;
        }
        diff = prxMPL_header->seq - vpieStatus.prev_seq;

        if (diff < 0)
        {
            /* received is old MPL packet */
            minSeq = vpieStatus.prev_seq - RX_MPL_SEQ_QUEUE_SIZE;
            diff1 = prxMPL_header->seq - minSeq;

            if (diff1 < 0)
            {   /* received MPL is not expected range */
                vpieStatus.num_seq_not_in_window++;

                tr_cmdline("== rx-seq(%u) prev(%u) not in window",prxMPL_header->seq,vpieStatus.prev_seq);
                /* drop this MPL */
                return st;
            }
            else
            {   /* received MPL is expected range
                   may be need to check if this packet is sent to App
                */
                vpieStatus.num_seq_in_window++;
                tr_cmdline("=== rx-seq(%u) prev(%u) in window",prxMPL_header->seq,vpieStatus.prev_seq);
                return st;
            }
        }
        /* may check if the gap is more than one
         * the RX  seq may be received
         * */

    }

    /* save the seq info */
    timac_update_rxInfo(prxMPL_header);

    /* decrement the hop limit */
    if (prxMPL_header->hopLimit <=1 )
    {
        vpieStatus.num_terminate++;

        /* invoke user call back */
        if (pRxMPLCallback)
        {
            macMplHnd.total_rx_callback++;
            (*pRxMPLCallback)(prxMPL_header->pData,prxMPL_header->len,prxMPL_header);
        }
        return st;
    }

    /* start to forward the received MPL packet
     * decrement the hop limit
     * */
    ptxForwardHeader->hopLimit = prxMPL_header->hopLimit -1;

    ptxForwardHeader->seq = prxMPL_header->seq;
    memcpy (ptxForwardHeader->orignator,prxMPL_header->orignator,8);
    ptxForwardHeader->pData = prxMPL_header->pData;
    ptxForwardHeader->len   = prxMPL_header->len;

    vpieStatus.num_forward++;

    /* make sure the node is not BR, only RN will forward the MPL packet */
    if (macMplHnd.BRNode == 0)
    {
        st = timac_MPL_DataReq(ptxForwardHeader);
        if (st)
        {
            vpieStatus.err_num_forward++;
        }
    }
    else
    {   vpieStatus.err_num_br_forward++;
        tr_cmdline("*** BR can not forward rx-seq(%u) prev(%u) in window",prxMPL_header->seq,vpieStatus.prev_seq);
    }

    /* invoke user callback */
    if (pRxMPLCallback)
    {
        macMplHnd.total_rx_callback++;
        (*pRxMPLCallback)(prxMPL_header->pData,prxMPL_header->len,prxMPL_header);
    }
    return st;
}

uint32 mpl_delay;
void timac_rx_MPL_test_callback(uint8 *pdata,uint8 len,MAC_MPL_Header_t *pRxHeader)
{
    uint16_t tx_slot,rx_slot;
    uint32_t tx_bfio,rx_bfio;
    uint32 macBcInterval;
    uint8 idx=0;

    /* get current timestamp in term of slot and bfio */
    timac_GetBC_Slot_BFIO(&rx_slot, &rx_bfio);

    // retrieve the TX slot and bfio from the RX payload

    tx_slot = pdata[idx++];
    tx_slot += (pdata[idx++] <<8);
    tx_bfio = pdata[idx++];
    tx_bfio += (pdata[idx++]<<8);
    tx_bfio += (pdata[idx++]<<16);
    tx_bfio += (pdata[idx++]<<24);

    MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);
    mpl_delay = (rx_slot - tx_slot ) * macBcInterval + rx_bfio - tx_bfio;

    tr_cmdline("seq(%u) D(%d) RX (%d, %u), TX (%d,%u) hop(%d) Len(%d)",pRxHeader->seq, mpl_delay,rx_slot,rx_bfio,tx_slot,tx_bfio,
               pRxHeader->hopLimit,pRxHeader->len);
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

    /* Unblock the task by releasing the semaphore. */
    sem_post(&rfCbSemHandle);
}

void *rfCbThread(void *arg0)
{
    while(true)
    {
        sem_wait(&rfCbSemHandle);
        if (numRfCmdQueued)
        {
            macRfQueue[rRfPtr].rfCb(macRfQueue[rRfPtr].rfCbHndl,
                                   macRfQueue[rRfPtr].rfCbCmdHndl,
                                   macRfQueue[rRfPtr].rfCbEventMask);
            portENTER_CRITICAL();
            numRfCmdQueued--;
            rRfPtr = (rRfPtr + 1) % macRfQueueSize;
            portEXIT_CRITICAL();
        }
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
            while (1) {}
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
