/*
 * Copyright (c) 2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "nsconfig.h"
#include <string.h>
#include "ns_types.h"
#include "ns_list.h"
#include "ns_trace.h"
#include "nsdynmemLIB.h"
#include "common_functions.h"
#include "6LoWPAN/ws/ws_config.h"
#include "NWK_INTERFACE/Include/protocol.h"
#include "Security/protocols/sec_prot_certs.h"
#include "Security/protocols/sec_prot_keys.h"
#include "RPL/rpl_nvm_store.h"
#include "RPL/rpl_nvm_data.h"
#ifndef NV_RESTORE
#include "Service_Libs/utils/ns_file.h"
#include "ns_file_system.h"
#endif //NV_RESTORE

#ifdef HAVE_RPL

#define TRACE_GROUP "rpnv"

#define RPL_FIELD_NOT_SET            0   // Field is not present
#define RPL_FIELD_SET                1   // Field is present

uint8_t rpl_nvm_buffer[RPL_NVM_DEFAULT_BUFFER_SIZE];             /**< Buffer for RPL NVM read and write operations */

nvm_tlv_t *rpl_get_nvm_tlv()
{
    return (nvm_tlv_t *) &rpl_nvm_buffer;
}

void rpl_nvm_store_info_tlv_create(nw_info_nvm_tlv_t *tlv_entry, uint8_t dodag_version, uint8_t dtsn)
{
    int len;
    tlv_entry->tag = RPL_INFO_TAG;
    tlv_entry->len = RPL_INFO_LEN;

    uint8_t *tlv = (uint8_t *) &tlv_entry->data[0];

    *tlv = dodag_version;
    tlv++;

    *tlv = dtsn;
    tlv++;


    int8_t status = rpl_nvm_store_tlv_file_write(RPL_INFO_FILE, (nvm_tlv_t *) tlv_entry);

    if (status != 0)
    {
        return;
    }

    tr_debug("NVM RPL_INFO write DODAG Version %i and DTSN %i to NV", dodag_version, dtsn);
}

int8_t rpl_nvm_store_info_tlv_read(nw_info_nvm_tlv_t *tlv_entry, uint8_t *dodag_version, uint8_t *dtsn)
{
    if (!tlv_entry || !dodag_version) {
        return -1;
    }

    if (tlv_entry->tag != RPL_INFO_TAG || tlv_entry->len != RPL_INFO_LEN) {
        return -1;
    }

    int8_t status = rpl_nvm_store_tlv_file_read(RPL_INFO_FILE, (nvm_tlv_t *) tlv_entry);

    if (status != 0)
    {
        return status;
    }

    uint8_t *tlv = (uint8_t *) &tlv_entry->data[0];

    *dodag_version = *tlv;
    tlv++;

    *dtsn = *tlv;
    tlv++;

    tr_debug("NVM RPL_INFO read DODAG Version %i DTSN %i", *dodag_version, *dtsn);

    return status;
}

#endif /* HAVE_WS */