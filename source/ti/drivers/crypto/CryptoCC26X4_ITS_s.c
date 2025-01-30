/*
 * Copyright (c) 2019-2024, Arm Limited. All rights reserved.
 * Copyright (c) 2024, Texas Instruments Incorporated - http://www.ti.com
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/* The following code is copied from tfm_its_req_mngr.c from TF-M v1.1 and
 * its_signal_handle() was modified to support TF-M v2.0.
 */

#include <stddef.h>

#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/interface/include/psa/storage_common.h>
#include <third_party/tfm/secure_fw/partitions/internal_trusted_storage/tfm_internal_trusted_storage.h>

typedef psa_status_t (*its_func_t)(void);

static psa_status_t tfm_its_set_ipc(void)
{
    psa_storage_uid_t uid;
    size_t data_length;
    psa_storage_create_flags_t create_flags;
    size_t num;

    if (msg.in_size[0] != sizeof(uid) || msg.in_size[2] != sizeof(create_flags))
    {
        /* The size of one of the arguments is incorrect */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    data_length = msg.in_size[1];

    num = psa_read(msg.handle, 0, &uid, sizeof(uid));
    if (num != sizeof(uid))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    num = psa_read(msg.handle, 2, &create_flags, sizeof(create_flags));
    if (num != sizeof(create_flags))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return tfm_its_set(msg.client_id, uid, data_length, create_flags);
}

static psa_status_t tfm_its_get_ipc(void)
{
    psa_storage_uid_t uid;
    size_t data_offset;
    size_t data_size;
    size_t data_length;
    size_t num;

    if (msg.in_size[0] != sizeof(uid) || msg.in_size[1] != sizeof(data_offset))
    {
        /* The size of one of the arguments is incorrect */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    data_size = msg.out_size[0];

    num = psa_read(msg.handle, 0, &uid, sizeof(uid));
    if (num != sizeof(uid))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    num = psa_read(msg.handle, 1, &data_offset, sizeof(data_offset));
    if (num != sizeof(data_offset))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return tfm_its_get(msg.client_id, uid, data_offset, data_size, &data_length);
}

static psa_status_t tfm_its_get_info_ipc(void)
{
    psa_status_t status;
    psa_storage_uid_t uid;
    struct psa_storage_info_t info;
    size_t num;

    if (msg.in_size[0] != sizeof(uid) || msg.out_size[0] != sizeof(info))
    {
        /* The size of one of the arguments is incorrect */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    num = psa_read(msg.handle, 0, &uid, sizeof(uid));
    if (num != sizeof(uid))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    status = tfm_its_get_info(msg.client_id, uid, &info);
    if (status == PSA_SUCCESS)
    {
        psa_write(msg.handle, 0, &info, sizeof(info));
    }

    return status;
}

static psa_status_t tfm_its_remove_ipc(void)
{
    psa_storage_uid_t uid;
    size_t num;

    if (msg.in_size[0] != sizeof(uid))
    {
        /* The input argument size is incorrect */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    num = psa_read(msg.handle, 0, &uid, sizeof(uid));
    if (num != sizeof(uid))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    return tfm_its_remove(msg.client_id, uid);
}

static void its_signal_handle(psa_signal_t signal, its_func_t pfn)
{
    psa_status_t status;

    status = psa_get(signal, &msg);
    if (status != PSA_SUCCESS)
    {
        return;
    }

    switch (msg.type)
    {
        case PSA_IPC_CONNECT:
            psa_reply(msg.handle, PSA_SUCCESS);
            break;

        case PSA_IPC_DISCONNECT:
            psa_reply(msg.handle, PSA_SUCCESS);
            break;

        default:
            if (msg.type >= PSA_IPC_CALL)
            {
                psa_reply(msg.handle, pfn());
            }
            else
            {
                psa_reply(msg.handle, PSA_ERROR_PROGRAMMER_ERROR);
            }
            break;
    }
}