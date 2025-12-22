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

#ifndef RPL_NVM_STORE_H_
#define RPL_NVM_STORE_H_

#include "stdint.h"

/*
 * RPL non-volatile memory (NVM) storage module. Module is used
 * to write and read RPL NVM data TLVs to/from filesystem.
 *
 */

#define RPL_NVM_FILE_SUCCESS            0
#define RPL_NVM_FILE_READ_ERROR        -1
#define RPL_NVM_FILE_WRITE_ERROR       -2
#define RPL_NVM_FILE_VERSION_WRONG     -3
#define RPL_NVM_FILE_CANNOT_OPEN       -4
#define RPL_NVM_FILE_ROOT_PATH_INVALID -5
#define RPL_NVM_FILE_PARAMETER_INVALID -6
#define RPL_NVM_FILE_REMOVE_ERROR      -7

typedef struct nvm_tlv {
    uint16_t tag;                       /**< Unique tag */
    uint16_t len;                       /**< Number of the bytes after the length field */
} nvm_tlv_t;

typedef struct {
    nvm_tlv_t nvm_tlv;                  /**< NVM TLV */
    uint32_t reference_restart_cnt;     /**< Reference re-start counter set when file is created) */
    uint64_t reference_time;            /**< Reference time used for timers (set when file is created) */
} key_storage_nvm_tlv_entry_t;

// tag + length
#define NVM_TLV_FIXED_LEN               sizeof(nvm_tlv_t)

/**
 * rpl_nvm_store_tlv_file_write write a list of TLVs to file
 *
 * \param file file name
 * \param tlv TLV
 *
 * \return < 0 failure
 * \return >= 0 success
 *
 */
int8_t rpl_nvm_store_tlv_file_write(const char *file, nvm_tlv_t *tlv);

/**
 * rpl_nvm_store_tlv_file_read read a list of TLVs from file
 *
 * \param file file name
 * \param tlv TLV
 *
 * \return < 0 failure
 * \return >= 0 success
 *
 */
int8_t rpl_nvm_store_tlv_file_read(const char *file, nvm_tlv_t *tlv);

/**
 * rpl_nvm_store_tlv_file_remove delete a file
 *
 * \param file file name
 *
 * \return < 0 failure
 * \return >= 0 success
 *
 */
int8_t rpl_nvm_store_tlv_file_remove(const char *file);

#endif /* RPL_NVM_STORE_H_ */
