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
#include "Security/protocols/sec_prot_certs.h"
#include "Security/protocols/sec_prot_keys.h"
#include "RPL/rpl_nvm_store.h"
#include "RPL/rpl_nvm_data.h"
#ifdef NV_RESTORE
#include "nvintf.h"
#include "nvocmp.h"
#else
#include "Service_Libs/utils/ns_file.h"
#include "ns_file_system.h"
#endif

#ifdef HAVE_RPL

#define TRACE_GROUP "rpnv"

#ifdef NV_RESTORE

extern NVINTF_nvFuncts_t *pNV;


int8_t rpl_nvm_store_tlv_file_write(const char *file, nvm_tlv_t *tlv)
{
    int8_t status = RPL_NVM_FILE_WRITE_ERROR;

    tr_info(" File name passed = %s", file);

    if((pNV != NULL) && (pNV->writeItem != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_WISUN;
        id.itemID = tlv->tag;
        id.subID = 0;

        /* Write the NV item */
        if(NVINTF_SUCCESS == pNV->writeItem(id, tlv->len, (tlv+1)))
        {
            tr_info("Successfully written item = %d to NV", tlv->tag);
            status = RPL_NVM_FILE_SUCCESS;
        }
        else
        {
            tr_info("Failed to Write NV item with tag = %d", tlv->tag);
        }
    }

    return(status);
}

int8_t rpl_nvm_store_tlv_file_read(const char *file, nvm_tlv_t *tlv)
{
    int8_t status = RPL_NVM_FILE_READ_ERROR;

    tr_info(" File name passed = %s", file);

    if((pNV != NULL) && (pNV->readItem != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_WISUN;
        id.itemID = tlv->tag;
        id.subID = 0;

        /* Read Network Information from NV */
        if(pNV->readItem(id, 0, tlv->len, (tlv+1)) == NVINTF_SUCCESS)
        {
        tr_info("Successfully read item = %d from NV", tlv->tag);
        status = RPL_NVM_FILE_SUCCESS;
        }
        else
        {
            tr_info("Failed to read NV item with tag = %d", tlv->tag);
        }
    }

    return(status);
}

int8_t rpl_nvm_store_tlv_file_remove(const char *file)
{
    int8_t status = RPL_NVM_FILE_READ_ERROR;
    uint16_t tag = 0;
    uint16_t subID = 0;
    NVINTF_itemID_t id;

    tr_info(" File name passed = %s", file);

    /* map file name to tag value */
    if(0 == strcmp(file, RPL_INFO_FILE))
    {
        tag = RPL_INFO_TAG;
    }

    if(tag != 0)
    {
        //clear item from NV
        id.systemID = NVINTF_SYSID_WISUN;
        id.itemID = tag;
        id.subID = subID;
        if(pNV == NULL)
        {
            // this case is triggered if NV_RESTORE is disabled in app
            status = RPL_NVM_FILE_SUCCESS;
        }
        else if(pNV->deleteItem(id) == NVINTF_SUCCESS)
        {
            tr_info("Cleared NV item with tag = %d", tag);
            status = RPL_NVM_FILE_SUCCESS;
        }
        else
        {
            tr_info("Failed to clear NV item with tag = %d", tag);
        }
    }

    return(status);
}

#else //NV_RESTORE
int8_t rpl_nvm_store_tlv_file_write(const char *file, nvm_tlv_t *tlv)
{
    return -1; // NV_RESTORE is disabled
}

int8_t rpl_nvm_store_tlv_file_read(const char *file, nvm_tlv_t *tlv)
{
    return -1; // NV_RESTORE is disabled
}

int8_t rpl_nvm_store_tlv_file_remove(const char *file)
{
    return -1; // NV_RESTORE is disabled
}

#endif //NV_RESTORE

#endif /* HAVE_RPL */

