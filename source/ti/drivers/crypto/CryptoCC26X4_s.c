/*
 * Copyright (c) 2022-2024, Texas Instruments Incorporated - http://www.ti.com
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

#include <stdint.h>

#include "CryptoCC26X4_s.h"
#include <third_party/tfm/interface/include/psa/service.h>
#include <psa_manifest/crypto_sp.h> /* Auto-generated header */

#ifdef TI_CRYPTO_ITS_INTEGRATION
    #include <third_party/tfm/secure_fw/partitions/internal_trusted_storage/tfm_internal_trusted_storage.h>
#endif

#include <ti/drivers/aescbc/AESCBCCC26X4_s.h>
#include <ti/drivers/aesccm/AESCCMCC26X4_s.h>
#include <ti/drivers/aescmac/AESCMACCC26X4_s.h>
#include <ti/drivers/aesctr/AESCTRCC26X4_s.h>
#include <ti/drivers/aesctrdrbg/AESCTRDRBGX4_s.h>
#include <ti/drivers/aesecb/AESECBCC26X4_s.h>
#include <ti/drivers/aesgcm/AESGCMCC26X4_s.h>
#include <ti/drivers/ecdh/ECDHCC26X4_s.h>
#include <ti/drivers/ecdsa/ECDSACC26X4_s.h>
#include <ti/drivers/ecjpake/ECJPAKECC26X4_s.h>
#include <ti/drivers/eddsa/EDDSACC26X4_s.h>
#include <ti/drivers/sha2/SHA2CC26X4_s.h>
#include <ti/drivers/trng/TRNGCC26X4_s.h>

#if (ENABLE_KEY_STORAGE == 1)
    #include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_s.h>
#endif

// #include <psa/PSA_s.h>

#include <ti/drivers/dpl/HwiP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ints.h)

static psa_msg_t msg;

#ifdef ENABLE_ITS_IPC_INTEGRATION
    #include "CryptoCC26X4_ITS_s.c"
#endif

/*
 *  ======== pka_irqn_flih ========
 */
psa_flih_result_t pka_irqn_flih(void)
{
    HwiP_dispatchInterrupt(INT_PKA_IRQ);

    return PSA_FLIH_NO_SIGNAL;
}

/*
 *  ======== cryptoresultavail_irqn_flih ========
 */
psa_flih_result_t cryptoresultavail_irqn_flih(void)
{
    HwiP_dispatchInterrupt(INT_CRYPTO_RESULT_AVAIL_IRQ);

    return PSA_FLIH_NO_SIGNAL;
}

/*
 *  ======== trng_irqn_flih ========
 */
psa_flih_result_t trng_irqn_flih(void)
{
    HwiP_dispatchInterrupt(INT_TRNG_IRQ);

    return PSA_FLIH_NO_SIGNAL;
}

/*
 *  ======== Crypto_s_handlePsaMsg ========
 */
static psa_status_t Crypto_s_handlePsaMsg(psa_msg_t *msg)
{
    int32_t msgTypeIndex;
    psa_status_t status = PSA_ERROR_PROGRAMMER_ERROR;

    msgTypeIndex = GET_CRYPTO_S_MSG_TYPE_INDEX(msg->type);

    switch (msgTypeIndex)
    {
        case CRYPTO_S_MSG_TYPE_INDEX_AESCBC:
            status = AESCBC_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_AESCCM:
            status = AESCCM_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_AESCMAC:
            status = AESCMAC_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_AESCTR:
            status = AESCTR_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_AESECB:
            status = AESECB_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_AESGCM:
            status = AESGCM_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_AESCTRDRBG:
            status = AESCTRDRBG_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_ECDH:
            status = ECDH_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_ECDSA:
            status = ECDSA_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_ECJPAKE:
            status = ECJPAKE_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_EDDSA:
            status = EDDSA_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_SHA2:
            status = SHA2_s_handlePsaMsg(msg);
            break;

        case CRYPTO_S_MSG_TYPE_INDEX_TRNG:
            status = TRNG_s_handlePsaMsg(msg);
            break;

#if (ENABLE_KEY_STORAGE == 1)
        case CRYPTO_S_MSG_TYPE_INDEX_KEYSTORE:
            status = KeyStore_s_handlePsaMsg(msg);
            break;

            // BQ - commented out as we should be using native PSA call support.
            // Delete once implementation finalized.
            // case CRYPTO_S_MSG_TYPE_INDEX_PSA:
            //     status = PSA_s_handlePsaMsg(msg);
            //     break;
#endif

        default:
            /* Unknown msg type - do nothing */
            break;
    }

    return status;
}

/*
 *  ======== Crypto_sp_main ========
 *  Crypto Secure Partition entry point
 */
void Crypto_sp_main(void)
{
    uint32_t signals;

#if defined(TI_CRYPTO_ITS_INTEGRATION) || defined(ENABLE_ITS_IPC_INTEGRATION)
    /* Initialize ITS */
    if (tfm_its_init() != PSA_SUCCESS)
    {
        psa_panic();
    }
#endif

    /* Initialize all secure crypto drivers except TRNG. TRNG init requires TRNG
     * HW to be powered ON first so must be done by non-secure world where power
     * is controlled.
     */
    AESCBC_s_init();
    AESCCM_s_init();
    AESCMAC_s_init();
    AESCTR_s_init();
    AESECB_s_init();
    AESGCM_s_init();

    AESCTRDRBG_s_init();

    ECDH_s_init();
    ECDSA_s_init();
    ECJPAKE_s_init();
    EDDSA_s_init();

    SHA2_s_init();

#if (ENABLE_KEY_STORAGE == 1)
    KeyStore_s_init();

    // BQ - commented out as we should be using native PSA call support.
    // Delete once implementation finalized.
    // PSA_s_init();
#endif

    /* Enable external interrupts for AES and TRNG. PKA interrupt is not enabled
     * here since it must be enabled by the PKA driver for proper operation.
     */
    psa_irq_enable(CryptoResultAvail_IRQn_SIGNAL);
    psa_irq_enable(TRNG_IRQn_SIGNAL);

    while (1)
    {
        signals = psa_wait(PSA_WAIT_ANY, PSA_BLOCK);

        if (signals & TI_CRYPTO_SERVICE_SIGNAL)
        {
            psa_get(TI_CRYPTO_SERVICE_SIGNAL, &msg);

            switch (msg.type)
            {
                case PSA_IPC_CONNECT:
                    /* This partition supports multiple simultaneous connections */
                    psa_reply(msg.handle, PSA_SUCCESS);
                    break;

                case PSA_IPC_DISCONNECT:
                    psa_reply(msg.handle, PSA_SUCCESS);
                    break;

                default:
                    if (msg.type >= PSA_IPC_CALL)
                    {
                        psa_reply(msg.handle, Crypto_s_handlePsaMsg(&msg));
                    }
                    else
                    {
                        psa_reply(msg.handle, PSA_ERROR_PROGRAMMER_ERROR);
                    }
                    break;
            }
        }
#ifdef ENABLE_ITS_IPC_INTEGRATION
        else if (signals & TFM_ITS_SET_SIGNAL)
        {
            its_signal_handle(TFM_ITS_SET_SIGNAL, tfm_its_set_ipc);
        }
        else if (signals & TFM_ITS_GET_SIGNAL)
        {
            its_signal_handle(TFM_ITS_GET_SIGNAL, tfm_its_get_ipc);
        }
        else if (signals & TFM_ITS_GET_INFO_SIGNAL)
        {
            its_signal_handle(TFM_ITS_GET_INFO_SIGNAL, tfm_its_get_info_ipc);
        }
        else if (signals & TFM_ITS_REMOVE_SIGNAL)
        {
            its_signal_handle(TFM_ITS_REMOVE_SIGNAL, tfm_its_remove_ipc);
        }
#endif /* ENABLE_ITS_IPC_INTEGRATION */
        else
        {
            psa_panic();
        }
    }

    return;
}
