/******************************************************************************

 @file  ble_adapter.c

 @brief Describe the purpose and contents of the file.

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

/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

/**
 * @file ble_adapter.c
 */

/* BLE HAL related files */
#include <bt_hal_manager_adapter_ble.h>
#include <bt_hal_manager.h>
#include <bt_hal_gatt_server.h>

/* SID realated include */
#include <sid_pal_ble_adapter_ifc.h>
#include <sid_ble_config_ifc.h>
#include <sid_error.h>

#include "FreeRTOS.h"

/* C standard library includes */
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------------------------*/

/*********************************************************************
 * CONSTANTS
 */
#define DEFAULT_MIN_CONN_INTERVAL 16
#define DEFAULT_MAX_CONN_INTERVAL 60
#define BLE_NOTIFY_LENGTH 2
#define BLE_NOTIFICATION_ENABLED 1
#define DEFAULT_SCAN_INTERVAL   256
#define DEFAULT_SCAN_WINDOW     150 // Currently Support not implemented in TI PAL

/*********************************************************************
 * TYPEDEFS
 */

void (*ti_ble_adapter_adv_terminated_cb)(void) = NULL;
extern void setTIPALBleAdvTimeout(uint16_t ulAdvTimeOut);
static uint32_t last_set_adv_interval = 0;

/* To handle cases where a stopADV may be called
after a startADV is called & device is still in fast interval mode */
static bool enableSlowIntvAdv = true;

// BLE interfaces
struct ble_hal_ifc {
    uint8_t adapter_if;
    uint8_t server_if;
    BTInterface_t* bt_interface;
    BTBleAdapter_t* bt_ble_adapter_if;
    BTGattServerInterface_t* gatt_server_if;
};

struct sid_pal_ble_adapter_ctx {
    const sid_ble_config_t *cfg;
    const sid_pal_ble_adapter_callbacks_t *callback;
    struct ble_hal_ifc ifc;
    BTService_t *service_table;
    uint16_t mtu_size;
    BTGattAdvertismentParams_t adv_config;
    bool is_connected;
    uint16_t conn_id;
    uint8_t bt_addr[BLE_ADDR_MAX_LEN];  // Peer's BD address
};

static struct sid_pal_ble_adapter_ctx ctx;
static BTCallbacks_t bt_manager_cb;
static BTBleAdapterCallbacks_t bt_ble_adapter_cb;

static void sid_pal_set_adv_param(uint32_t adv_interval, uint32_t adv_timeout)
{
    ctx.ifc.bt_ble_adapter_if->pxSetScanParameters(ctx.ifc.adapter_if,
                                                   adv_interval,
                                                   DEFAULT_SCAN_WINDOW);
    setTIPALBleAdvTimeout(adv_timeout);
    last_set_adv_interval = adv_interval;
}

static void ble_connection_cb(uint16_t conn_id,
                              uint8_t server_if,
                              bool connected,
                              BTBdaddr_t* bt_addr)
{
    if (bt_addr == NULL) {
        return;
    }

    ctx.conn_id = conn_id;
    ctx.is_connected = connected;
    memcpy(&ctx.bt_addr, &bt_addr->ucAddress, BLE_ADDR_MAX_LEN);
    ctx.callback->conn_callback(ctx.is_connected, ctx.bt_addr);
}

static void ble_indication_sent_cb(uint16_t conn_id, BTStatus_t status)
{
    ctx.callback->ind_callback((status == eBTStatusSuccess));
}

static void ble_mtu_changed_cb(uint16_t us_connection_id, uint16_t us_mtu)
{
    ctx.mtu_size = us_mtu;
    ctx.callback->mtu_callback(ctx.mtu_size);
}

// Saving server interface
static void ble_register_server_cb(BTStatus_t status,
                                   uint8_t server_if,
                                   BTUuid_t* app_uuid)
{
    if (status == eBTStatusSuccess) {
        ctx.ifc.server_if = server_if;
    }
}

static void ble_request_write_cb(uint16_t conn_id,
                                 uint32_t trans_id,
                                BTBdaddr_t* bt_addr,
                                uint16_t attr_handle,
                                uint16_t offset,
                                size_t length,
                                bool need_resp,
                                bool is_prep,
                                uint8_t* data)
{
    if (data == NULL) {
        return;
    }

    sid_ble_cfg_service_identifier_t id;

    for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
        id = ctx.cfg->profile[i].service.type;
        for (uint8_t idx = 1; idx < ctx.service_table[i].xNumberOfAttributes; idx++) {
            if (ctx.service_table[i].pusHandlesBuffer[idx] == attr_handle) {
                if (ctx.service_table[i].pxBLEAttributes[idx].xAttributeType == eBTDbDescriptor &&
                    length == BLE_NOTIFY_LENGTH ) {
                    uint16_t notif_data;
                    memcpy(&notif_data, data, sizeof(notif_data));
                    ctx.callback->notify_callback(id, (notif_data == BLE_NOTIFICATION_ENABLED));
                }  else if (ctx.service_table[i].pxBLEAttributes[idx].xAttributeType == eBTDbCharacteristic) {
                    ctx.callback->data_callback(id, data, length);
                }
                return;
            }
        }
    }
}


static void ble_unregister_server_cb(BTStatus_t status, uint8_t server_if)
{
    if (status != eBTStatusSuccess) {
        return;
    }

    ctx.ifc.server_if = 0;
}

static void ble_adapter_free_services(void)
{
    uint8_t i = 0;

    if (ctx.service_table != NULL) {
        for (i = 0; i < ctx.cfg->num_profile; i++) {
            if (ctx.service_table[i].pusHandlesBuffer != NULL) {
                vPortFree(ctx.service_table[i].pusHandlesBuffer);
                ctx.service_table[i].pusHandlesBuffer = NULL;
            }

            if (ctx.service_table[i].pxBLEAttributes != NULL) {
                vPortFree(ctx.service_table[i].pxBLEAttributes);
                ctx.service_table[i].pxBLEAttributes = NULL;
            }
        }
    }

    if (ctx.service_table != NULL) {
        vPortFree(ctx.service_table);
        ctx.service_table = NULL;
    }
}

static void ble_convert_to_hal_uuid(BTUuid_t* hal_uuid, const sid_ble_cfg_uuid_info_t* uuid)
{
    if (!hal_uuid || !uuid) {
        return;
    }

    memset(hal_uuid, 0, sizeof(BTUuid_t));
    switch (uuid->type) {
        case UUID_TYPE_16: {
            uint16_t* p_val = &(hal_uuid->uu.uu16);
            hal_uuid->ucType = eBTuuidType16;
            *p_val = uuid->uu[1] | (uuid->uu[0] << 8);
            break;
        }

        case UUID_TYPE_32: {
            uint32_t* p_val = &(hal_uuid->uu.uu32);
            hal_uuid->ucType = eBTuuidType32;
            *p_val = uuid->uu[3] | (uuid->uu[2] << 8) | (uuid->uu[1] << 16) | (uuid->uu[0] << 24);
            break;
        }

        case UUID_TYPE_128: {
            hal_uuid->ucType = eBTuuidType128;
            memcpy(hal_uuid->uu.uu128, uuid->uu, BLE_UUID_MAX_LEN);
            break;
        }
    }
}

static sid_error_t ble_copy_characteristics(BTService_t* srvc,
                                            const sid_ble_cfg_characteristics_t* char_rec,
                                            uint8_t index)
{
    if (!srvc || !char_rec ) {
        return SID_ERROR_INVALID_ARGS;
    }

    BTCharProperties_t prop = 0;
    BTCharPermissions_t perm = 0;

    srvc->pxBLEAttributes[index].xAttributeType = eBTDbCharacteristic;

    if (char_rec->properties.is_read) {
        prop |= eBTPropRead;
    }
    if (char_rec->properties.is_write) {
        prop |= eBTPropWrite;
    }
    if (char_rec->properties.is_notify) {
        prop |= eBTPropNotify;
    }
    if (char_rec->properties.is_write_no_resp) {
        prop |= eBTPropWriteNoResponse;
    }
    if (char_rec->perm.is_none) {
        perm |= eBTPermNone;
    }
    if (char_rec->perm.is_read) {
        perm |= eBTPermRead;
    }
    if (char_rec->perm.is_write) {
        perm |= eBTPermWrite;
    }

    srvc->pxBLEAttributes[index].xCharacteristic.xProperties = prop;
    srvc->pxBLEAttributes[index].xCharacteristic.xPermissions= perm;
    ble_convert_to_hal_uuid(&srvc->pxBLEAttributes[index].xCharacteristic.xUuid,
                            &char_rec->id);

    return SID_ERROR_NONE;
}

static sid_error_t ble_copy_descriptor(BTService_t* srvc,
                                       const sid_ble_cfg_descriptor_t* desc_rec,
                                       uint8_t index)
{
    if (!srvc || !desc_rec) {
        return SID_ERROR_INVALID_ARGS;
    }

    BTCharPermissions_t perm = 0;

    srvc->pxBLEAttributes[index].xAttributeType = eBTDbDescriptor;

    if (desc_rec->perm.is_none) {
        perm |= eBTPermNone;
    }
    if (desc_rec->perm.is_read) {
        perm |= eBTPermRead;
    }
    if (desc_rec->perm.is_write) {
        perm |= eBTPermWrite;
    }

    srvc->pxBLEAttributes[index].xCharacteristicDescr.xPermissions = perm;
    ble_convert_to_hal_uuid(&srvc->pxBLEAttributes[index].xCharacteristicDescr.xUuid,
                            &desc_rec->id);

    return SID_ERROR_NONE;
}

// .start_service
static sid_error_t ble_adapter_start_service(void)
{
    if (ctx.cfg->num_profile == 0) {
        return SID_ERROR_INVALID_ARGS;
    }

    sid_error_t status = SID_ERROR_NONE;
    uint8_t i = 0,j = 0;

    ctx.service_table = (BTService_t*)pvPortMalloc((ctx.cfg->num_profile)*sizeof(BTService_t));
    if (ctx.service_table == NULL) {
        ble_adapter_free_services();
        return SID_ERROR_OOM;
    }

    for (i = 0; i < ctx.cfg->num_profile; i++) {
        // Account for service UUID attribute
        size_t num_attributes = 1 + ctx.cfg->profile[i].desc_count + ctx.cfg->profile[i].char_count;
        uint8_t handle_idx = 0;

        ctx.service_table[i].pxBLEAttributes = (BTAttribute_t*)pvPortMalloc(num_attributes*sizeof(BTAttribute_t));
        if (ctx.service_table[i].pxBLEAttributes == NULL) {
            ble_adapter_free_services();
            return SID_ERROR_OOM;
        }

        ctx.service_table[i].pusHandlesBuffer = (uint16_t*)pvPortMalloc(num_attributes*sizeof(uint16_t));
        if (ctx.service_table[i].pusHandlesBuffer == NULL) {
            ble_adapter_free_services();
            return SID_ERROR_OOM;
        }

        // Fill up Service properties
        ctx.service_table[i].ucInstId = 0;
        ctx.service_table[i].xType = eBTServiceTypePrimary;
        ctx.service_table[i].xNumberOfAttributes = num_attributes;
        ctx.service_table[i].pxBLEAttributes[0].xAttributeType = eBTDbPrimaryService;

        ble_convert_to_hal_uuid(&ctx.service_table[i].pxBLEAttributes[0].xServiceUUID,
                                &ctx.cfg->profile[i].service.id);

        // Fill up Characteristic properties
        for (j = 0; j < ctx.cfg->profile[i].char_count; j++) {
            handle_idx++;
            status = ble_copy_characteristics(&ctx.service_table[i],
                                              &ctx.cfg->profile[i].characteristic[j],
                                              handle_idx);

            if (status != SID_ERROR_NONE) {
                ble_adapter_free_services();
                return status;
            }
        }

        // Fill up Descriptor properties
        for (j = 0; j < ctx.cfg->profile[i].desc_count; j++) {
            handle_idx++;
            status = ble_copy_descriptor(&ctx.service_table[i],
                                   &ctx.cfg->profile[i].desc[j],
                                   handle_idx);

            if (status != SID_ERROR_NONE) {
                ble_adapter_free_services();
                return status;
            }
        }

        status = ctx.ifc.gatt_server_if->pxAddServiceBlob(ctx.ifc.server_if, &ctx.service_table[i]);
        if (status != SID_ERROR_NONE) {
            ble_adapter_free_services();
            return status;
        }
    }

    return status;
}

//.set_callback
static sid_error_t ble_adapter_set_callback(const sid_pal_ble_adapter_callbacks_t *cb)
{
    if (!cb) {
        return SID_ERROR_NULL_POINTER;
    }

    if (!cb->data_callback   ||
        !cb->notify_callback ||
        !cb->conn_callback   ||
        !cb->ind_callback    ||
        !cb->mtu_callback    ||
        !cb->adv_start_callback) {
        return SID_ERROR_INVALID_ARGS;
    }

    ctx.callback = cb;
    return SID_ERROR_NONE;
}

// .set_adv_data
static sid_error_t ble_adapter_set_adv_data(uint8_t *data, uint8_t length)
{
    BTStatus_t status = eBTStatusSuccess;
    BTUuid_t service_uuid;
    bool found = false;

    for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
        if (ctx.cfg->adv_param.type == ctx.cfg->profile[i].service.type) {
            ble_convert_to_hal_uuid(&service_uuid, &ctx.cfg->profile[i].service.id);
            found = true;
            break;
        }
    }

    if (!found) {
        return SID_ERROR_INCOMPATIBLE_PARAMS;
    }

    // Save the original value
    bool setScanFlag = ctx.adv_config.bSetScanRsp;

    // Change it in order to update the advertising data first
    ctx.adv_config.bSetScanRsp = false;
    ctx.adv_config.ucNameType = BTGattAdvNameNone;

    // Update the advertising data
    status = ctx.ifc.bt_ble_adapter_if->pxSetAdvData(ctx.ifc.adapter_if,
                                                     &ctx.adv_config,
                                                     length,
                                                     (char*)data,
                                                     0,
                                                     NULL,
                                                     &service_uuid,
                                                     1);

    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    // Change it in order to update the scan response data
    ctx.adv_config.bSetScanRsp = true;
    ctx.adv_config.ucNameType = BTGattAdvNameComplete;

    // Update the scan response data
    status = ctx.ifc.bt_ble_adapter_if->pxSetAdvData(ctx.ifc.adapter_if,
                                                     &ctx.adv_config,
                                                     0,
                                                     NULL,
                                                     0,
                                                     NULL,
                                                     NULL,
                                                     0);

    // Return ctx.adv_config.bSetScanRsp to previous status
    ctx.adv_config.bSetScanRsp = setScanFlag;

    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

// .start_adv
static sid_error_t ble_adapter_start_advertisement(void)
{
    BTStatus_t status = eBTStatusSuccess;
    sid_pal_set_adv_param(ctx.cfg->adv_param.fast_interval, ctx.cfg->adv_param.fast_timeout);
    enableSlowIntvAdv = true;

    status = ctx.ifc.bt_ble_adapter_if->pxStartAdv(ctx.ifc.adapter_if);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

// .stop_adv
static sid_error_t ble_adapter_stop_advertisement(void)
{
    BTStatus_t status = eBTStatusSuccess;

    if(last_set_adv_interval == ctx.cfg->adv_param.fast_interval)
    {
        enableSlowIntvAdv = false;
    }

    status = ctx.ifc.bt_ble_adapter_if->pxStopAdv(ctx.ifc.adapter_if);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

// .send
static sid_error_t ble_adapter_send_data(sid_ble_cfg_service_identifier_t id, uint8_t *data, uint16_t length)
{
    if (!ctx.is_connected) {
        return SID_ERROR_PORT_NOT_OPEN;
    }

    if ( !data || !length || length > ctx.mtu_size ) {
        return SID_ERROR_INVALID_ARGS;
    }

    BTStatus_t status = eBTStatusSuccess;
    uint16_t handle;
    bool found = false;

    for (uint8_t i = 0; i < ctx.cfg->num_profile; i++) {
        if (id == ctx.cfg->profile[i].service.type) {
            for (uint8_t idx = 1; idx < ctx.service_table[i].xNumberOfAttributes; idx++) {
                if (ctx.service_table[i].pxBLEAttributes[idx].xAttributeType == eBTDbCharacteristic &&
                    ctx.service_table[i].pxBLEAttributes[idx].xCharacteristic.xProperties & eBTPropNotify) {
                    handle = ctx.service_table[i].pusHandlesBuffer[idx];
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            break;
        }
    }

    if (found) {
        status = ctx.ifc.gatt_server_if->pxSendIndication(ctx.ifc.server_if,
                                                          handle,
                                                          ctx.conn_id,
                                                          length,
                                                          data,
                                                          false);

        if (status != eBTStatusSuccess) {
            return SID_ERROR_GENERIC;
        }
    } else {
        return SID_ERROR_INVALID_ARGS;
    }

    return SID_ERROR_NONE;
}

// .disconnect
static sid_error_t ble_adapter_disconnect(void)
{
    if (!ctx.is_connected) {
        return SID_ERROR_NONE;
    }

    BTStatus_t status = eBTStatusSuccess;

    status = ctx.ifc.bt_ble_adapter_if->pxDisconnect(ctx.ifc.adapter_if,
                                                     (const BTBdaddr_t *)ctx.bt_addr,
                                                     ctx.conn_id);

    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

// .deinit
static sid_error_t ble_adapter_deinit(void)
{
    BTStatus_t status = eBTStatusSuccess;

    // This the connection and stops the advertising if enabled
    status = ctx.ifc.bt_interface->pxDisable();
    if (status == eBTStatusSuccess) {
        // Continue the cleanup
        status = ctx.ifc.gatt_server_if->pxUnregisterServer(ctx.ifc.server_if);
        if (status != eBTStatusSuccess) {
            return SID_ERROR_GENERIC;
        }

        // Destroy the advertising and release frees the adv and the scan response data
        status = ctx.ifc.bt_interface->pxBtManagerCleanup();
        if (status != eBTStatusSuccess) {
            return SID_ERROR_GENERIC;
        }

        ble_adapter_free_services();
    } else {
        return SID_ERROR_GENERIC;
    }

    return SID_ERROR_NONE;
}

static BTGattServerCallbacks_t bt_gatt_svr_cb = {
    .pxRegisterServerCb       = ble_register_server_cb,
    .pxUnregisterServerCb     = ble_unregister_server_cb,
    .pxConnectionCb           = ble_connection_cb,
    .pxRequestWriteCb         = ble_request_write_cb,
    .pxIndicationSentCb       = ble_indication_sent_cb,
    .pxMtuChangedCb           = ble_mtu_changed_cb,
};

void sid_pal_ble_adapter_handle_adv_terminated_cb()
{
    if(last_set_adv_interval == ctx.cfg->adv_param.fast_interval &&
       enableSlowIntvAdv == true)
    {
        // Update ADV Parameters
        sid_pal_set_adv_param(ctx.cfg->adv_param.slow_interval, ctx.cfg->adv_param.slow_timeout);
        ctx.ifc.bt_ble_adapter_if->pxStartAdv(ctx.ifc.adapter_if);
    }
    else
    {
        /* this is part of slow interval cb. Reset to fast interval */
        sid_pal_set_adv_param(ctx.cfg->adv_param.fast_interval, ctx.cfg->adv_param.fast_timeout);
    }
}

static sid_error_t ble_adapter_init(const sid_ble_config_t *cfg)
{
    if (!cfg) {
        return SID_ERROR_INVALID_ARGS;
    }

    BTStatus_t status = eBTStatusSuccess;
    BTProperty_t prop;
    ctx.cfg = cfg;

    /* add an external pointer reference for internal pal cb */
    ti_ble_adapter_adv_terminated_cb = sid_pal_ble_adapter_handle_adv_terminated_cb;

    /* Get BT HAL interface */
    ctx.ifc.bt_interface = (BTInterface_t*)BTGetBluetoothInterface();
    if (ctx.ifc.bt_interface == NULL) {
        return SID_ERROR_GENERIC;
    }

    /* Initialize BT manager callbacks */
    status = ctx.ifc.bt_interface->pxBtManagerInit(&bt_manager_cb);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    /* Enable Bluetooth Module */
    status = ctx.ifc.bt_interface->pxEnable(0);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    ctx.ifc.bt_ble_adapter_if = (BTBleAdapter_t*)ctx.ifc.bt_interface->pxGetLeAdapter();
    if (ctx.ifc.bt_ble_adapter_if == NULL) {
        return SID_ERROR_GENERIC;
    }

    status = ctx.ifc.bt_ble_adapter_if->pxBleAdapterInit(&bt_ble_adapter_cb);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    // Set default advertisement parameters
    status = ctx.ifc.bt_ble_adapter_if->pxSetScanParameters(ctx.ifc.adapter_if,
                                                   ctx.cfg->adv_param.fast_interval,
                                                   DEFAULT_SCAN_WINDOW);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }
    setTIPALBleAdvTimeout(ctx.cfg->adv_param.fast_timeout);

    last_set_adv_interval = ctx.cfg->adv_param.fast_interval;

    /* Get BT GATT interface */
    ctx.ifc.gatt_server_if = (BTGattServerInterface_t*)ctx.ifc.bt_ble_adapter_if->ppvGetGattServerInterface();
    if (ctx.ifc.gatt_server_if == NULL) {
        return SID_ERROR_GENERIC;
    }

    status = ctx.ifc.gatt_server_if->pxGattServerInit(&bt_gatt_svr_cb);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    status = ctx.ifc.gatt_server_if->pxRegisterServer(NULL);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    // Set Name
    prop.xType = eBTpropertyBdname;
    prop.xLen = strlen(ctx.cfg->name);
    prop.pvVal = (void *)(ctx.cfg->name);

    // Set device property
    status = ctx.ifc.bt_interface->pxSetDeviceProperty(&prop);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    // Set MTU property
    prop.xType = eBTpropertyLocalMTUSize;
    prop.xLen = sizeof(ctx.cfg->mtu);
    prop.pvVal = (void*)(&ctx.cfg->mtu);

    // Set device property
    status = ctx.ifc.bt_interface->pxSetDeviceProperty(&prop);
    if (status != eBTStatusSuccess) {
        return SID_ERROR_GENERIC;
    }

    // Build BTGattAdvertismentParams_t
    memset(&ctx.adv_config, 0, sizeof(BTGattAdvertismentParams_t));

    ctx.adv_config.usAdvertisingEventProperties = BTAdvInd;
    ctx.adv_config.ucNameType = BTGattAdvNameComplete;
    ctx.adv_config.xAddrType = BTAddrTypePublic;

    // ctx.adv_config.ucTimeout parameter is deprecated

    if (cfg->is_conn_available) {
        ctx.adv_config.ulMinInterval = cfg->conn_param.min_conn_interval;
        ctx.adv_config.ulMaxInterval = cfg->conn_param.max_conn_interval;
    } else {
        ctx.adv_config.ulMinInterval = DEFAULT_MIN_CONN_INTERVAL;
        ctx.adv_config.ulMaxInterval = DEFAULT_MAX_CONN_INTERVAL;
    }

    return SID_ERROR_NONE;
}

static struct sid_pal_ble_adapter_interface ble_adapter_ifc = {
    .init = ble_adapter_init,
    .start_service = ble_adapter_start_service,
    .set_adv_data = ble_adapter_set_adv_data,
    .start_adv = ble_adapter_start_advertisement,
    .stop_adv = ble_adapter_stop_advertisement,
    .send = ble_adapter_send_data,
    .set_callback = ble_adapter_set_callback,
    .disconnect = ble_adapter_disconnect,
    .deinit = ble_adapter_deinit,
};

// links the struct to the commands
sid_error_t sid_pal_ble_adapter_create(sid_pal_ble_adapter_interface_t *handle)
{
    if (!handle) {
        return SID_ERROR_INVALID_ARGS;
    }

    *handle = &ble_adapter_ifc;
    return SID_ERROR_NONE;
}
