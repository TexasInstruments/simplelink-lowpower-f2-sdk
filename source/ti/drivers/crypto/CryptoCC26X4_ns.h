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
/** ============================================================================
 *  @file       CryptoCC26X4_ns.h
 *
 *  @brief      Shared resources to arbitrate access to Crypto engines on Secure/Nonsecure devices
 *
 */

#ifndef ti_drivers_crypto_CryptoCC26X4_ns__include
#define ti_drivers_crypto_CryptoCC26X4_ns__include

#include <stdint.h>

#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)

#include <psa/client.h>

/*! @cond NODOC */

extern SemaphoreP_Struct CryptoPSACC26X4_accessSemaphore;
extern SemaphoreP_Struct CryptoPSACC26X4_operationSemaphore;

/*!
 *  @brief   Non-secure client wrapper to make PSA calls to secure service.
 *           Disables context switching around psa_call()
 *
 *  @pre     CryptoPSACC26X4_init() must be called first
 *
 *  @param type      Secure message type
 *  @param invecs    Input vector pointer to secure message struct
 *  @param outvecs   Output vector pointer to storage for return value
 *  @return          Returns 0 on success, or -129 on PSA programmer error
 */
psa_status_t CryptoPSACC26X4_call(int32_t type, psa_invec *invecs, psa_outvec *outvecs);

/*!
 *  @brief   Initialization function constructs semaphores and inits SecureCB driver
 */
void CryptoPSACC26X4_init(void);

/*!
 *  @brief   Get the CryptoPSACC26X4_accessSemaphore (callback and blocking modes)
 *           Returns bool False on timeout.
 *
 *  @pre     CryptoPSACC26X4_init() must be called first
 *
 *  @param timeout   Semaphore timeout value
 *  @return          Returns True on success, False on timeout
 */
bool CryptoPSACC26X4_acquireLock(uint32_t timeout);

/*!
 *  @brief   Post the CryptoPSACC26X4_accessSemaphore (callback and blocking modes)
 *
 *  @pre     CryptoPSACC26X4_init() must be called first
 */
__STATIC_INLINE void CryptoPSACC26X4_releaseLock(void)
{
    SemaphoreP_post(&CryptoPSACC26X4_accessSemaphore);
}

/*! @endcond */

#endif /* ti_drivers_crypto_CryptoCC26X4_ns__include */
