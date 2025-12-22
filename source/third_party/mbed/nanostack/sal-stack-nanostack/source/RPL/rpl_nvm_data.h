/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
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

#ifndef RPL_NVM_DATA_H_
#define RPL_NVM_DATA_H_

#include "stdint.h"

/*
 * Port access entity non-volatile memory (NVM) data module. Module is used
 * to create and parse RPL NVM data TLVs.
 *
 */

// file names
#define RPL_INFO_FILE                       "rpl_info"

// This tag will be used as item ID with TI NVS driver APIs. Needs to not conflict with existing PAE item ids
#define RPL_INFO_TAG                        11

// DODAG Version Number (1) + DTSN (1)
#define RPL_INFO_LEN                        1 + 1

#define RPL_NVM_DEFAULT_BUFFER_SIZE         sizeof(nvm_tlv_t) + RPL_INFO_LEN

typedef struct nw_info_nvm_tlv {
    uint16_t tag;                               /**< Unique tag */
    uint16_t len;                               /**< Number of the bytes after the length field */
    uint8_t data[RPL_INFO_LEN];                 /**< Data */
} nw_info_nvm_tlv_t;

/**
 * rpl_get_nvm_tlv get TLV entry pointing to buffer
 *
 * \return TLV entry
 *
 */
nvm_tlv_t *rpl_get_nvm_tlv();

/**
 * rpl_nvm_store_info_tlv_create create NVM network info TLV
 *
 * \param tlv_entry TLV
 * \param dodag_version DODAG Version
 * \param dtsn DTSN Value
 *
 * \return TLV entry or NULL
 *
 */
void rpl_nvm_store_info_tlv_create(nw_info_nvm_tlv_t *tlv_entry, uint8_t dodag_version, uint8_t dtsn);

/**
 * rpl_nvm_store_info_tlv_read read from NVM network info TLV
 *
 * \param tlv_entry TLV
 * \param dodag_version DODAG Version
 * \param dtsn DTSN Value
 *
 * \return < 0 failure
 * \return >= 0 success
 *
 */
int8_t rpl_nvm_store_info_tlv_read(nw_info_nvm_tlv_t *tlv_entry, uint8_t *dodag_version, uint8_t *dtsn);

#endif /* RPL_NVM_DATA_H_ */
