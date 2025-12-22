/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== application.c ========
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
/* Driver configuration */
#include "ti_drivers_config.h"
#include "ti_wisunfan_config.h"

#include "rcp_lmac.h"
#include "mcp.h"


extern uint8_t timacTaskId;

/******************************************************************************
Function definitions
 *****************************************************************************/

/* Uncomment this macro so if MAC CFG and INIT should be done locally for test purposes. 
Keep this undefined for regular usage as Host wills ned commands over MT for CONF and INIT */
//#define TEST_RCP_LMAC_SELF_CONF_INIT
/* Uncomment this macro to send a test async packet; 
Keep this undefined for regular usage and spawning MT thread*/
//#define TEST_RCP_LMAC_TEST_ASYNC_PKT 

#ifdef TEST_RCP_LMAC_SELF_CONF_INIT
// #include "advanced_config.h"
// #include "nsconfig.h"
// #include "net_interface.h"
// #include "mac_hl_patch.h"
// #include "ns_types.h"
// #include "mac_common_defines.h"

#include "mac_assert.h"
#include "osal_port.h"
#include "macTask.h"
#include "timac_api.h"
#include "macs.h"
#include "macwrapper.h"
#include "mac_settings.h"

#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/knl/Task.h>
#else
/* POSIX Header files */
#include <pthread.h>
#include <FreeRTOS.h>
#include <semaphore.h>
#endif

// Test code
/* Network name; max 32 octets + terminating 0 */
#define MAX_NETWORK_NAME_SIZE 33
#define CHANNEL_BITMAP_SIZE 17
#define HWADDR_SIZE 8
#define SIZE_OF_NEIGH_LIST 32 // max value currently set in nanostack code

#define CONFIG_PAN_ID                   0xFFFF
#define CONFIG_CHANNEL_PAGE             9
#define CONFIG_OP_MODE_ID               OPERATING_MODE_1b
#define CONFIG_OP_MODE_CLASS            1
#define CONFIG_CCA_THRESHOLD            -83
#define CONFIG_REG_DOMAIN               0x01
#define CONFIG_UNICAST_CHANNEL_MASK       { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
                                          0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
                                          0xFF,0xFF,0xFF,0xFF,0x01 }
#define CONFIG_ASYNC_CHANNEL_MASK         { 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
                                          0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
                                          0xFF,0xFF,0xFF,0xFF,0x01 }
#define CONFIG_NETNAME                  "Wi-SUN Network"
#define CONFIG_BROADCAST_INTERVAL       1020
#define CONFIG_BROADCAST_DWELL_TIME     255
#define CONFIG_UNICAST_DWELL_TIME       255
#define CONFIG_TRANSMIT_POWER           0
#define CONFIG_CHANNEL_FUNCTION         2
#define CONFIG_UNICAST_FIXED_CHANNEL_NUM  0

typedef enum {
    MESH_DEVICE_TYPE_THREAD_ROUTER = 0,         /*<! Thread router */
    MESH_DEVICE_TYPE_THREAD_SLEEPY_END_DEVICE,  /*<! Thread Sleepy end device */
    MESH_DEVICE_TYPE_THREAD_MINIMAL_END_DEVICE, /*<! Thread minimal end device */
    MESH_DEVICE_TYPE_WISUN_ROUTER,              /*<! Wi-SUN router */
    MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER        /*<! Wi-SUN border router */
} mesh_device_type_t;

typedef struct configurable_props_s {
    int8_t ccaDefaultdBm;
    int8_t phyTxPower;
    uint8_t uc_channel_function;
    uint8_t uc_channel_list[CHANNEL_BITMAP_SIZE];
    uint16_t uc_fixed_channel;
    uint8_t uc_dwell_interval;
    uint8_t bc_channel_function;
    uint8_t bc_channel_list[CHANNEL_BITMAP_SIZE];
    uint16_t bc_fixed_channel;
    uint32_t bc_interval;
    uint8_t bc_dwell_interval;
    uint8_t async_channel_list[CHANNEL_BITMAP_SIZE];
    uint8_t excluded_channels[CHANNEL_BITMAP_SIZE];
    uint16_t pan_id;
    char network_name[MAX_NETWORK_NAME_SIZE];
    mesh_device_type_t wisun_device_type;
    uint32_t ch0_center_frequency; //in KHz
    uint16_t config_channel_spacing;
    uint8_t config_phy_id;
    uint8_t config_reg_domain;
    uint8_t operating_class;
    uint8_t operating_mode;
    
    uint8_t fan_support_version;
    uint8_t usie_chan_plan_selection;
    uint8_t bsie_chan_plan_selection;  
      
    uint8_t hwaddr[HWADDR_SIZE];
} configurable_props_t;


configurable_props_t cfg_props =
{ 
    .phyTxPower = CONFIG_TRANSMIT_POWER,
    .ccaDefaultdBm = CONFIG_CCA_THRESHOLD,
    .uc_channel_function = 0,
    .uc_fixed_channel = 0,
    .uc_dwell_interval = CONFIG_UNICAST_DWELL_TIME,
    .bc_channel_function = 0,
    .bc_fixed_channel = 0,
    .bc_interval = CONFIG_BROADCAST_INTERVAL,
    .bc_dwell_interval = CONFIG_BROADCAST_DWELL_TIME,
    .pan_id = CONFIG_PAN_ID,
    .network_name = CONFIG_NETNAME,
    .bc_channel_list = 0,
    .uc_channel_list = CONFIG_UNICAST_CHANNEL_MASK,
    .async_channel_list = CONFIG_ASYNC_CHANNEL_MASK,
    .wisun_device_type = MESH_DEVICE_TYPE_WISUN_BORDER_ROUTER,
    .ch0_center_frequency = CONFIG_CENTER_FREQ * 1000,
    .config_channel_spacing = CONFIG_CHANNEL_SPACING,
    .config_phy_id = CONFIG_PHY_ID,
    .config_reg_domain = CONFIG_REG_DOMAIN,
    .operating_class = CONFIG_OP_MODE_CLASS,
    .operating_mode = 0,
    .hwaddr = 0,
};

uint8_t deviceExtAddr[8]  = {0x00, 0x12, 0x4B, 0x00, 0x14, 0xF9, 0x12, 0x34}; //local to lmac side project

#ifdef TEST_RCP_LMAC_NO_MT
const uint8_t test_pa_data[100] = {
    0x01, 0xE3, 0xCD, 0xAB, 0x0E, 0x19, 0x53, 0x28, 0x00, 0x4B, 0x12, 0x00, 0x05, 0x15, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3F, 0x1F, 0xA0, 0x06, 0x88, 0xFF, 0xFF, 0x64, 0x10, 0x01, 0x01, 0x05,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x23, 0x0E, 0x05, 0x57, 0x69, 0x2D, 0x53, 0x55, 0x4E, 0x20, 0x4E,
    0x65, 0x74, 0x77, 0x6F, 0x72, 0x6B, 0x00, 0x00
};
#endif /* TEST_RCP_LMAC_NO_MT */

uint8_t fixed_key[16] = { 0xB9, 0x42, 0x70, 0x4F, 0xB4, 0xEC, 0x2D, 0xFD,
                          0x53, 0x02, 0x52, 0x4D, 0x08, 0x40, 0x51, 0x00 };

/*!
 Enables the Frequency hopping operation.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_enableFH(void)
{
    return (ApiMac_status_t) MAC_EnableFH();
}

static ApiMac_status_t mlmeSetFhReq(uint16_t pibAttribute, void *pValue)
{
    return (ApiMac_status_t) MAC_MlmeFHSetReq(pibAttribute, pValue);
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

ApiMac_status_t ApiMac_mlmeSetReqUint16(ApiMac_attribute_uint16_t pibAttribute,
                                        uint16_t value)
{
    return (ApiMac_status_t) MAC_MlmeSetReq(pibAttribute, (void *)&value);
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
 This function must be called once at system startup before any other
 function in the management API is called.

 Public function defined in api_mac.h
 */
ApiMac_status_t ApiMac_mlmeResetReq(bool setDefaultPib)
{
    return (ApiMac_status_t) MAC_MlmeResetReq(setDefaultPib);
}

extern MAC_RCP_Tasklet_DBG_s MacRcpTaskletDbg;
extern rcp_btie_debug_t rcp_btie_dbg;


static void test_timac_initialize()
{
    uint8_t excludeChannels[MAC_154G_CHANNEL_BITMAP_SIZ];
    uint8_t idx, sizeOfChannelMask;

    /* init debugt counts */
    memset(&MacRcpTaskletDbg, 0x0, sizeof(MacRcpTaskletDbg));
    memset(&rcp_btie_dbg, 0x0, sizeof(rcp_btie_dbg));
    rcp_btie_dbg.btieMinDelay = 0xffff;

    Task_sleep(10);

    /* Enable frequency hopping */
    ApiMac_enableFH();

    /* Reset the MAC */
    ApiMac_mlmeResetReq(true);

    timac_setup_Test_GPIO();

    //hard code key in lmac
    keyDescriptor_t *key_descriptor = &rcp_lmac_store.key_descriptors[0];
    memset(key_descriptor, 0, sizeof(*key_descriptor));
    memcpy(key_descriptor->key, fixed_key, sizeof(key_descriptor->key));

    MAC_MlmeSetReq(macExtendedAddress, (void *)deviceExtAddr);
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyCurrentDescriptorId, cfg_props.config_phy_id);
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_regDomain, cfg_props.config_reg_domain);
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_channelPage, (uint8_t)CONFIG_CHANNEL_PAGE);
    /* Set the transmit power */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyTransmitPowerSigned, (uint8_t)cfg_props.phyTxPower);
    ApiMac_mlmeSetReqBool(ApiMac_attribute_RxOnWhenIdle, true);

    ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_numNonSleepDevice, 1);

    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastDwellInterval, cfg_props.uc_dwell_interval);

    ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastChannelFunction, cfg_props.uc_channel_function);

    if(cfg_props.uc_channel_function == 0) //fixed channel
    {
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_unicastFixedChannel, cfg_props.uc_fixed_channel);
    }
    else //DH1CF
    {
        sizeOfChannelMask = sizeof(cfg_props.uc_channel_list)/sizeof(uint8_t);

        if(sizeOfChannelMask > MAC_154G_CHANNEL_BITMAP_SIZ)
        {
           sizeOfChannelMask = MAC_154G_CHANNEL_BITMAP_SIZ;
        }

        memset(excludeChannels, 0, MAC_154G_CHANNEL_BITMAP_SIZ);

        for(idx = 0; idx < sizeOfChannelMask; idx++)
        {
           excludeChannels[idx] = ~cfg_props.uc_channel_list[idx];
        }
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_unicastExcludedChannels,
                                excludeChannels);
    }

    if (cfg_props.ffd) {
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval, cfg_props.bc_dwell_interval);
        ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval, cfg_props.bc_interval);
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction, cfg_props.bc_channel_function);
        if(cfg_props.bc_channel_function == 0) //fixed channel
        {
            ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_broadcastFixedChannel, cfg_props.bc_fixed_channel);
        }
        else //DH1CF
        {
            sizeOfChannelMask = sizeof(cfg_props.bc_channel_list)/sizeof(uint8_t);
            if(sizeOfChannelMask > MAC_154G_CHANNEL_BITMAP_SIZ)
            {
            sizeOfChannelMask = MAC_154G_CHANNEL_BITMAP_SIZ;
            }

            memset(excludeChannels, 0, MAC_154G_CHANNEL_BITMAP_SIZ);

            for(idx = 0; idx < sizeOfChannelMask; idx++)
            {
            excludeChannels[idx] = ~cfg_props.bc_channel_list[idx];
            }
            ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels,
                                    excludeChannels);
        }
    } else {
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval, 0);
        ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval, 0);
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction, 0);
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_broadcastFixedChannel, 0);
        memset(excludeChannels, 0, MAC_154G_CHANNEL_BITMAP_SIZ);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels,
                                excludeChannels);
    }

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
#endif //TEST_RCP_LMAC_SELF_CONF_INIT

extern rcp_lmac_internal_t rcp_lmac_store;
/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
#ifdef TEST_RCP_LMAC_SELF_CONF_INIT

    // Initialize PIBs
    // test_timac_initialize();
    // memcpy(rcp_lmac_store.async_channel_list, cfg_props.async_channel_list, sizeof(rcp_lmac_store.async_channel_list));

    // // Start MAC
    // ApiMac_mlmeStartReq_t timac_start_req;
    // memset(&timac_start_req, 0, sizeof(timac_start_req));
    // timac_start_req.startTime = 0;
    // timac_start_req.panId = 0xABCD;
    // timac_start_req.beaconOrder = 15;
    // timac_start_req.superframeOrder = 15;
    // timac_start_req.panCoordinator = 1;

    // timac_start_req.phyID = 2;
    // timac_start_req.startFH = true;
    // timac_start_req.channelPage = MAC_CHANNEL_PAGE_9;
    // MAC_MlmeStartReq(&timac_start_req);
#endif //TEST_RCP_LMAC_SELF_CONF_INIT

#ifdef TEST_RCP_LMAC_TEST_ASYNC_PKT
    rcp_data_req_t test_data_req_async;

    // Form test PA packet (no security)
    memset(&test_data_req_async, 0, sizeof(rcp_data_req_t));
    test_data_req_async.req_type = RCP_DATA_REQ_ASYNC;
    test_data_req_async.msdu_handle = 0xAB;
    test_data_req_async.min_tx_frame_count = 100;
    test_data_req_async.frame_count_offset = 32;
    // security off, ignore sec field
    test_data_req_async.mdata_offset = 21;
    test_data_req_async.utie_offset = 16;
    test_data_req_async.btie_offset = 0;
    // Async, ignore fhnt_entry field
    test_data_req_async.data_len = 54;
    test_data_req_async.data_ptr = (uint8_t *) test_pa_data;


    // Send test PA frame
    handle_rcp_data_req(&test_data_req_async);
#else //plugin MT interface 
    /* Kick off co-processor application task */
    MCP_task(timacTaskId);
#endif //TEST_RCP_LMAC_TEST_ASYNC_PKT

    return NULL;
}
