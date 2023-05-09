/******************************************************************************

 @file  storage_kv.c

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

#include <sid_pal_storage_kv_ifc.h>
#include <sid_pal_storage_kv_internal_group_ids.h>
#include <storage_kv_keys.h>
#include <driverlib/aon_batmon.h>
#include <nvocmp.h>
#include <string.h>

static NVINTF_nvFuncts_t nv_fps;
static bool storage_kv_init_done = false;

static void low_voltage_cb(uint32_t voltage)
{
    // put here anything you want to do when low voltage detected
    // just leave at it is if you want nothing
    // write and erase will not happen in low voltage
}

sid_error_t sid_pal_storage_kv_init()
{
    if (storage_kv_init_done) {
        return SID_ERROR_NONE;
    }

    NVOCMP_loadApiPtrsExt(&nv_fps);
    NVOCMP_setLowVoltageCb(&low_voltage_cb);
    nv_fps.initNV(NULL);

    storage_kv_init_done = true;

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void *p_data, uint32_t len)
{
    NVINTF_itemID_t id;
    uint8_t status;

    if (!storage_kv_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    id.systemID = NVINTF_SYSID_SIDWK;
    id.itemID = group;
    id.subID = key;

    status = nv_fps.readItem(id, 0, len, p_data);

    if (status == NVINTF_NOTFOUND) {
        return SID_ERROR_NOT_FOUND;
    } else if (status) {
        return SID_ERROR_STORAGE_READ_FAIL;
    } else {
        return SID_ERROR_NONE;
    }
}

sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t * p_len)
{
    NVINTF_itemID_t id;

    if (!storage_kv_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    id.systemID = NVINTF_SYSID_SIDWK;
    id.itemID = group;
    id.subID = key;

    *p_len = nv_fps.getItemLen(id);

    if (!(*p_len)) {
            return SID_ERROR_NOT_FOUND;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const * p_data, uint32_t len)
{
    NVINTF_itemID_t id;
    uint8_t status;

    if (!storage_kv_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    id.systemID = NVINTF_SYSID_SIDWK;
    id.itemID = group;
    id.subID = key;

    status = nv_fps.writeItem(id, len, (void *)p_data);

    if (status) {
        return SID_ERROR_STORAGE_WRITE_FAIL;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key)
{
    NVINTF_itemID_t id;
    uint8_t status;

    if (!storage_kv_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    id.systemID = NVINTF_SYSID_SIDWK;
    id.itemID = group;
    id.subID = key;

    status = nv_fps.deleteItem(id);

    if (status) {
        return SID_ERROR_STORAGE_ERASE_FAIL;
    }

    return SID_ERROR_NONE;
}

sid_error_t sid_pal_storage_kv_group_delete(uint16_t group)
{
    NVINTF_nvProxy_t nv_proxy = {0};
    uint8_t status = 0;
    int32_t key = 0;

    if (!storage_kv_init_done) {
        return SID_ERROR_UNINITIALIZED;
    }

    nv_proxy.sysid = NVINTF_SYSID_SIDWK;
    nv_proxy.itemid = group;
    nv_proxy.flag = NVINTF_DOSTART | NVINTF_DOITMID | NVINTF_DODELETE;

    if (nv_fps.lockNV) {
        key = nv_fps.lockNV();
    }

    while (!status) {
        status = nv_fps.doNext(&nv_proxy);
    }

    if (nv_fps.unlockNV) {
        nv_fps.unlockNV(key);
    }

    if (status != NVINTF_NOTFOUND) {
        return SID_ERROR_STORAGE_ERASE_FAIL;
    }

    return SID_ERROR_NONE;
}
