/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated - http://www.ti.com
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
#include <stdbool.h>

#include "CryptoCC26X4_ns.h"

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)

#include <ti/drivers/tfm/SecureCallback.h>

#include <psa/client.h>
#include <psa_manifest/sid.h>

#define STATUS_SUCCESS ((int_fast16_t)0)
#define STATUS_ERROR   ((int_fast16_t)-1)

/* Semaphore used to synchronize access to TF-M Crypto service */
SemaphoreP_Struct CryptoPSACC26X4_accessSemaphore;
/* Semaphore used to block the task when blocking return behavior is used */
SemaphoreP_Struct CryptoPSACC26X4_operationSemaphore;

static bool isInitialized = false;

static psa_handle_t sp_handle = PSA_NULL_HANDLE;

/*
 *  ======== CryptoPSACC26X4_setupCall ========
 */
static int_fast16_t CryptoPSACC26X4_setupCall(void)
{
    int_fast16_t status = STATUS_SUCCESS;

    if (!PSA_HANDLE_IS_VALID(sp_handle))
    {
        /* Connect to the Crypto Service */
        sp_handle = psa_connect(TI_CRYPTO_SERVICE_SID, TI_CRYPTO_SERVICE_VERSION);

        if (!PSA_HANDLE_IS_VALID(sp_handle))
        {
            status = (int_fast16_t)sp_handle;
        }
    }

    return (status);
}

/*
 *  ======== CryptoPSACC26X4_cleanupCall ========
 */
static void CryptoPSACC26X4_cleanupCall(psa_status_t psaStatus)
{
    if (psaStatus == PSA_ERROR_PROGRAMMER_ERROR)
    {
        /* When a PSA_ERROR_PROGRAMMER_ERROR occurs, the PSA connection must be
         * closed and re-established before normal operation can resume. The PSA
         * connection will be re-established during the next crypto function
         * call.
         */
        psa_close(sp_handle);

        sp_handle = PSA_NULL_HANDLE;
    }
}

/*
 *  ======== CryptoPSACC26X4_call ========
 */
psa_status_t CryptoPSACC26X4_call(int32_t type, psa_invec *invecs, psa_outvec *outvecs)
{
    psa_status_t psaStatus;
    uintptr_t key;

    if (CryptoPSACC26X4_setupCall() != 0)
    {
        return STATUS_ERROR;
    }

    /* Disable context switching */
    key = HwiP_disable();

    psaStatus = psa_call(sp_handle, type, invecs, 1, outvecs, 1);

    /* Reenable context switching */
    HwiP_restore(key);

    CryptoPSACC26X4_cleanupCall(psaStatus);

    return (psaStatus);
}

/*
 *  ======== CryptoPSACC26X4_init ========
 */
void CryptoPSACC26X4_init(void)
{
    uintptr_t key;

    key = HwiP_disable();

    if (!isInitialized)
    {
        SemaphoreP_constructBinary(&CryptoPSACC26X4_accessSemaphore, 1);
        SemaphoreP_constructBinary(&CryptoPSACC26X4_operationSemaphore, 0);

        /* Initialize SecureCB driver */
        SecureCallback_init();

        isInitialized = true;
    }

    HwiP_restore(key);
}

/*
 *  ======== CryptoPSACC26X4_acquireLock ========
 */
bool CryptoPSACC26X4_acquireLock(uint32_t timeout)
{
    SemaphoreP_Status resourceAcquired;

    /* Try and obtain access to the crypto module */
    resourceAcquired = SemaphoreP_pend(&CryptoPSACC26X4_accessSemaphore, timeout);

    return resourceAcquired == SemaphoreP_OK;
}
