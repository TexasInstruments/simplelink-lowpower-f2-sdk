/*
 * Copyright (c) 2022 Texas Instruments Incorporated - http://www.ti.com
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

#include <psa/PSA_s.h>
#include <psa/ti_psa_crypto.h>

#include <third_party/tfm/interface/include/psa/crypto.h>
#include <third_party/tfm/interface/include/psa/error.h>
#include <third_party/tfm/interface/include/psa/service.h>
#include <third_party/tfm/secure_fw/spm/core/spm.h>

#include <third_party/tfm/secure_fw/spm/include/utilities.h> /* tfm_core_panic() */

/* Back-end psa_hash_operation_t */
extern psa_hash_operation_t ti_psa_crypto_hashOperation;

/*
 *  ======== PSA_s_signHash ========
 */
static inline psa_status_t PSA_s_signHash(psa_msg_t *msg)
{
    psa_status_t status;
    PSA_s_SignHashMsg signMsg;

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* NS clients are not supported */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if ((msg->in_size[0] != sizeof(signMsg)) || (msg->out_size[0] != 0))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &signMsg, sizeof(signMsg));

    status = ti_psa_sign_hash(signMsg.handle,
                              signMsg.alg,
                              signMsg.hash,
                              signMsg.hash_length,
                              signMsg.signature,
                              signMsg.signature_size,
                              signMsg.signature_length);

    return status;
}

/*
 *  ======== PSA_s_signHash ========
 */
static inline psa_status_t PSA_s_hashSetup(psa_msg_t *msg)
{
    psa_status_t status;
    PSA_s_HashSetupMsg setupMsg;

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* NS clients are not supported */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if ((msg->in_size[0] != sizeof(setupMsg)) || (msg->out_size[0] != 0))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &setupMsg, sizeof(setupMsg));

    status = ti_psa_hash_setup(&ti_psa_crypto_hashOperation, setupMsg.alg);

    return status;
}

static inline psa_status_t PSA_s_hashUpdate(psa_msg_t *msg)
{
    psa_status_t status;
    PSA_s_HashUpdateMsg updateMsg;

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* NS clients are not supported */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if ((msg->in_size[0] != sizeof(updateMsg)) || (msg->out_size[0] != 0))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &updateMsg, sizeof(updateMsg));

    status = ti_psa_hash_update(&ti_psa_crypto_hashOperation, updateMsg.input, updateMsg.input_length);

    return status;
}

static inline psa_status_t PSA_s_hashFinish(psa_msg_t *msg)
{
    psa_status_t status;
    PSA_s_HashFinishMsg finishMsg;

    if (TFM_CLIENT_ID_IS_NS(msg->client_id))
    {
        /* NS clients are not supported */
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    if ((msg->in_size[0] != sizeof(finishMsg)) || (msg->out_size[0] != 0))
    {
        return PSA_ERROR_PROGRAMMER_ERROR;
    }

    psa_read(msg->handle, 0, &finishMsg, sizeof(finishMsg));

    status = ti_psa_hash_finish(&ti_psa_crypto_hashOperation,
                                finishMsg.hash,
                                finishMsg.hash_size,
                                finishMsg.hash_length);

    return status;
}

/*
 *  ======== PSA_s_handlePsaMsg ========
 */
psa_status_t PSA_s_handlePsaMsg(psa_msg_t *msg)
{
    psa_status_t status = PSA_SUCCESS;

    switch (msg->type)
    {
        case PSA_S_MSG_TYPE_SIGN_HASH:
            status = PSA_s_signHash(msg);
            break;

        case PSA_S_MSG_TYPE_HASH_SETUP:
            status = PSA_s_hashSetup(msg);
            break;

        case PSA_S_MSG_TYPE_HASH_UPDATE:
            status = PSA_s_hashUpdate(msg);
            break;

        case PSA_S_MSG_TYPE_HASH_FINISH:
            status = PSA_s_hashFinish(msg);
            break;

        default:
            /* Unknown msg type */
            status = PSA_ERROR_PROGRAMMER_ERROR;
            break;
    }

    return status;
}

/*
 *  ======== PSA_s_init ========
 */
void PSA_s_init(void)
{
    if (ti_psa_crypto_init() != PSA_SUCCESS)
    {
        tfm_core_panic();
    }
}