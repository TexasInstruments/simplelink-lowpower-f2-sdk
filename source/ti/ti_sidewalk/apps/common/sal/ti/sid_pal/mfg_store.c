/******************************************************************************

 @file  mfg_store.c

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
 * conditions set forth in the accompanying LICENSE.txt file. This file is a
 * Modifiable File, as defined in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#include <sid_pal_mfg_store_ifc.h>
#include <sid_pal_log_ifc.h>
#include <sid_endian.h>

#include <inc/hw_memmap.h>
#include <inc/hw_fcfg1.h>
#include <ti/drivers/NVS.h>

#include <string.h>
#include <stdint.h>
#include <stddef.h>

#define MFG_WORD_SIZE       4
#define CONFIG_MFG_ID       1

struct __attribute__((__packed__)) sid_pal_mfg_store_t {
    uint8_t devid[SID_PAL_MFG_STORE_DEVID_SIZE];
    uint8_t version[SID_PAL_MFG_STORE_VERSION_SIZE];
    uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE];
    uint8_t smsn[SID_PAL_MFG_STORE_SMSN_SIZE];
    uint8_t app_pub_ed25519[SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE];
    uint8_t device_priv_ed25519[SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE];
    uint8_t device_pub_ed25519[SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIZE];
    uint8_t device_pub_ed25519_signature[SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_SIZE];
    uint8_t device_priv_p256r1[SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE];
    uint8_t device_pub_p256r1[SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIZE];
    uint8_t device_pub_p256r1_signature[SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_SIZE];
    uint8_t dak_pub_ed25519[SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIZE];
    uint8_t dak_pub_ed25519_signature[SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_SIZE];
    uint8_t dak_ed25519_serial[SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_SIZE];
    uint8_t dak_pub_p256r1[SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIZE];
    uint8_t dak_pub_p256r1_signature[SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_SIZE];
    uint8_t dak_p256r1_serial[SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_SIZE];
    uint8_t product_pub_ed25519[SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIZE];
    uint8_t product_pub_ed25519_signature[SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_SIZE];
    uint8_t product_ed25519_serial[SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_SIZE];
    uint8_t product_pub_p256r1[SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIZE];
    uint8_t product_pub_p256r1_signature[SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_SIZE];
    uint8_t product_p256r1_serial[SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_SIZE];
    uint8_t man_pub_ed25519[SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIZE];
    uint8_t man_pub_ed25519_signature[SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_SIZE];
    uint8_t man_ed25519_serial[SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_SIZE];
    uint8_t man_pub_p256r1[SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIZE];
    uint8_t man_pub_p256r1_signature[SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_SIZE];
    uint8_t man_p256r1_serial[SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_SIZE];
    uint8_t sw_pub_ed25519[SID_PAL_MFG_STORE_SW_PUB_ED25519_SIZE];
    uint8_t sw_pub_ed25519_signature[SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_SIZE];
    uint8_t sw_ed25519_serial[SID_PAL_MFG_STORE_SW_ED25519_SERIAL_SIZE];
    uint8_t sw_pub_p256r1[SID_PAL_MFG_STORE_SW_PUB_P256R1_SIZE];
    uint8_t sw_pub_p256r1_signature[SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_SIZE];
    uint8_t sw_p256r1_serial[SID_PAL_MFG_STORE_SW_P256R1_SERIAL_SIZE];
    uint8_t amzn_pub_ed25519[SID_PAL_MFG_STORE_AMZN_PUB_ED25519_SIZE];
    uint8_t amzn_pub_p256r1[SID_PAL_MFG_STORE_AMZN_PUB_P256R1_SIZE];
    uint8_t apid[SID_PAL_MFG_STORE_APID_SIZE];
};

struct sid_pal_mfg_store_table_t {
    sid_pal_mfg_store_value_t id;
    size_t offset;
};

static uint32_t default_app_value_to_offset(int value)
{
    SID_PAL_LOG_ERROR("No support for app value to offset");
    return SID_PAL_MFG_STORE_INVALID_OFFSET;
}

static NVS_Handle mfg_store_handle;
static NVS_Attrs mfg_store_region_attrs;
static bool mfg_store_init_done = false;
static sid_pal_mfg_store_app_value_to_offset_t app_value_to_offset = default_app_value_to_offset;

struct sid_pal_mfg_store_table_t sid_pal_mfg_store_app_id_to_offset_table[] = {
    {SID_PAL_MFG_STORE_DEVID,                           offsetof(struct sid_pal_mfg_store_t, devid)},
    {SID_PAL_MFG_STORE_VERSION,                         offsetof(struct sid_pal_mfg_store_t, version)},
    {SID_PAL_MFG_STORE_SERIAL_NUM,                      offsetof(struct sid_pal_mfg_store_t, serial_num)},
    {SID_PAL_MFG_STORE_SMSN,                            offsetof(struct sid_pal_mfg_store_t, smsn)},
    {SID_PAL_MFG_STORE_APP_PUB_ED25519,                 offsetof(struct sid_pal_mfg_store_t, app_pub_ed25519)},
    {SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519,             offsetof(struct sid_pal_mfg_store_t, device_priv_ed25519)},
    {SID_PAL_MFG_STORE_DEVICE_PUB_ED25519,              offsetof(struct sid_pal_mfg_store_t, device_pub_ed25519)},
    {SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,    offsetof(struct sid_pal_mfg_store_t, device_pub_ed25519_signature)},
    {SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1,              offsetof(struct sid_pal_mfg_store_t, device_priv_p256r1)},
    {SID_PAL_MFG_STORE_DEVICE_PUB_P256R1,               offsetof(struct sid_pal_mfg_store_t, device_pub_p256r1)},
    {SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,     offsetof(struct sid_pal_mfg_store_t, device_pub_p256r1_signature)},
    {SID_PAL_MFG_STORE_DAK_PUB_ED25519,                 offsetof(struct sid_pal_mfg_store_t, dak_pub_ed25519)},
    {SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE,       offsetof(struct sid_pal_mfg_store_t, dak_pub_ed25519_signature)},
    {SID_PAL_MFG_STORE_DAK_ED25519_SERIAL,              offsetof(struct sid_pal_mfg_store_t, dak_ed25519_serial)},
    {SID_PAL_MFG_STORE_DAK_PUB_P256R1,                  offsetof(struct sid_pal_mfg_store_t, dak_pub_p256r1)},
    {SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE,        offsetof(struct sid_pal_mfg_store_t, dak_pub_p256r1_signature)},
    {SID_PAL_MFG_STORE_DAK_P256R1_SERIAL,               offsetof(struct sid_pal_mfg_store_t, dak_p256r1_serial)},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519,             offsetof(struct sid_pal_mfg_store_t, product_pub_ed25519)},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE,   offsetof(struct sid_pal_mfg_store_t, product_pub_ed25519_signature)},
    {SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL,          offsetof(struct sid_pal_mfg_store_t, product_ed25519_serial)},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1,              offsetof(struct sid_pal_mfg_store_t, product_pub_p256r1)},
    {SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE,    offsetof(struct sid_pal_mfg_store_t, product_pub_p256r1_signature)},
    {SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL,           offsetof(struct sid_pal_mfg_store_t, product_p256r1_serial)},
    {SID_PAL_MFG_STORE_MAN_PUB_ED25519,                 offsetof(struct sid_pal_mfg_store_t, man_pub_ed25519)},
    {SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE,       offsetof(struct sid_pal_mfg_store_t, man_pub_ed25519_signature)},
    {SID_PAL_MFG_STORE_MAN_ED25519_SERIAL,              offsetof(struct sid_pal_mfg_store_t, man_ed25519_serial)},
    {SID_PAL_MFG_STORE_MAN_PUB_P256R1,                  offsetof(struct sid_pal_mfg_store_t, man_pub_p256r1)},
    {SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE,        offsetof(struct sid_pal_mfg_store_t, man_pub_p256r1_signature)},
    {SID_PAL_MFG_STORE_MAN_P256R1_SERIAL,               offsetof(struct sid_pal_mfg_store_t, man_p256r1_serial)},
    {SID_PAL_MFG_STORE_SW_PUB_ED25519,                  offsetof(struct sid_pal_mfg_store_t, sw_pub_ed25519)},
    {SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE,        offsetof(struct sid_pal_mfg_store_t, sw_pub_ed25519_signature)},
    {SID_PAL_MFG_STORE_SW_ED25519_SERIAL,               offsetof(struct sid_pal_mfg_store_t, sw_ed25519_serial)},
    {SID_PAL_MFG_STORE_SW_PUB_P256R1,                   offsetof(struct sid_pal_mfg_store_t, sw_pub_p256r1)},
    {SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE,         offsetof(struct sid_pal_mfg_store_t, sw_pub_p256r1_signature)},
    {SID_PAL_MFG_STORE_SW_P256R1_SERIAL,                offsetof(struct sid_pal_mfg_store_t, sw_p256r1_serial)},
    {SID_PAL_MFG_STORE_AMZN_PUB_ED25519,                offsetof(struct sid_pal_mfg_store_t, amzn_pub_ed25519)},
    {SID_PAL_MFG_STORE_AMZN_PUB_P256R1,                 offsetof(struct sid_pal_mfg_store_t, amzn_pub_p256r1)},
    {SID_PAL_MFG_STORE_APID,                            offsetof(struct sid_pal_mfg_store_t, apid)},
};

static size_t sid_pal_get_offset(sid_pal_mfg_store_value_t id)
{
    const size_t table_count = sizeof(sid_pal_mfg_store_app_id_to_offset_table)/sizeof(sid_pal_mfg_store_app_id_to_offset_table[0]);
    size_t offset = 0;

    for (uint16_t i = 0; i < table_count; i++) {
        if (id == sid_pal_mfg_store_app_id_to_offset_table[i].id) {
            offset = sid_pal_mfg_store_app_id_to_offset_table[i].offset;
            goto ret;
        }
    }

    if (id < 0 || id >= SID_PAL_MFG_STORE_CORE_VALUE_MAX) {
        // This is not a core value. Search for this value among those provided by the application.
        uint32_t custom_offset = app_value_to_offset(id);
        if (custom_offset != SID_PAL_MFG_STORE_INVALID_OFFSET) {
            offset = (size_t)custom_offset;
            goto ret;
        }
    }

    SID_PAL_LOG_ERROR("No TI manufacturing store offset for: %d", id);
    return mfg_store_region_attrs.regionSize;

ret:
    return offset < mfg_store_region_attrs.regionSize ? offset : mfg_store_region_attrs.regionSize;
}

void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region)
{
    if (mfg_store_init_done) {
        return;
    }

    NVS_Params params;

    NVS_init();
    NVS_Params_init(&params);
    mfg_store_handle = NVS_open(CONFIG_MFG_ID, &params);

    if (!mfg_store_handle) {
        SID_PAL_LOG_ERROR("mfg_store open failed.");
        return;
    }

    NVS_getAttrs(mfg_store_handle, &mfg_store_region_attrs);
    if ((mfg_store_region_attrs.regionBase != (uint32_t *)mfg_store_region.addr_start) ||
        (mfg_store_region_attrs.regionSize != (mfg_store_region.addr_end - mfg_store_region.addr_start))) {
        SID_PAL_LOG_ERROR("mfg_store region is wrong.");
        return;
    }

    app_value_to_offset = mfg_store_region.app_value_to_offset ? mfg_store_region.app_value_to_offset : default_app_value_to_offset;

    mfg_store_init_done = true;
}

int32_t sid_pal_mfg_store_write(int value, const uint8_t *buffer, uint8_t length)
{
#if defined (HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS
    int_fast16_t result;

    if (!mfg_store_init_done) {
        return -1;
    }

    size_t offset = sid_pal_get_offset(value);

    if(offset != mfg_store_region_attrs.regionSize) {
        result = NVS_write(mfg_store_handle, offset, (void *)buffer, length, NVS_WRITE_POST_VERIFY);

        if (result) {
            return -1;
        }
    }

    return 0;
#else
    return -1;
#endif
}

void sid_pal_mfg_store_read(int value, uint8_t *buffer, uint8_t length)
{
    if (!mfg_store_init_done) {
        return;
    }

    size_t offset = sid_pal_get_offset(value);

    if(offset != mfg_store_region_attrs.regionSize) {
        NVS_read(mfg_store_handle, offset, buffer, length);
    }
}

int32_t sid_pal_mfg_store_erase(void)
{
#if defined (HALO_ENABLE_DIAGNOSTICS) && HALO_ENABLE_DIAGNOSTICS
    int_fast16_t result;

    if (!mfg_store_init_done) {
        return -1;
    }

    result = NVS_erase(mfg_store_handle, 0, mfg_store_region_attrs.sectorSize);

    if (result) {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

uint32_t sid_pal_mfg_store_get_version(void)
{
    uint32_t version;

    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_VERSION,
                           (uint8_t *)&version, SID_PAL_MFG_STORE_VERSION_SIZE);
    // Assuming that we keep this behavior for both 1P & 3P
    return version;
}

bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE])
{
    bool ret = false;
    uint8_t buffer[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_DEVID,
                            buffer, SID_PAL_MFG_STORE_DEVID_SIZE);

    static const uint8_t UNSET_DEV_ID[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if(memcmp(buffer, UNSET_DEV_ID, SID_PAL_MFG_STORE_DEVID_SIZE) == 0) {
        uint8_t mac_addr[4];
        memcpy(mac_addr, (uint8_t *)(FCFG1_BASE + FCFG1_O_MAC_15_4_0), 4);
        buffer[0] = 0xBF;
        buffer[1] = 0xFF;
        buffer[2] = 0xFF;
        buffer[3] = mac_addr[1];
        buffer[4] = mac_addr[0];
    } else {
        ret = true;
    }

    memcpy(dev_id, buffer, SID_PAL_MFG_STORE_DEVID_SIZE);
    return ret;
}

bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE])
{
    uint32_t buffer[(SID_PAL_MFG_STORE_SERIAL_NUM_SIZE + (MFG_WORD_SIZE - 1))/ MFG_WORD_SIZE];

    sid_pal_mfg_store_read(SID_PAL_MFG_STORE_SERIAL_NUM,
                            (uint8_t*)buffer, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

    static const uint8_t UNSET_SERIAL_NUM[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                               0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    if(memcmp(buffer, UNSET_SERIAL_NUM, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE) == 0) {
        return false;
    }

    memcpy(serial_num, buffer, SID_PAL_MFG_STORE_SERIAL_NUM_SIZE);

    return true;
}
