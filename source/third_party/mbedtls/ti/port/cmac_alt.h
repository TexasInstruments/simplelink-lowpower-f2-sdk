/******************************************************************************
 Copyright (c) 2022-2023, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 *****************************************************************************/

/*!
 *  @file       cmac_alt.h
 *
 *  @brief      Hardware accelerated mbedtls aescmac implementation
 *
 *  # General
 *
 *  # Limitations
 *
 */

#ifndef MBEDTLS_CMAC_ALT_H
#define MBEDTLS_CMAC_ALT_H

#include "mbedtls/build_info.h"

#if defined(MBEDTLS_CMAC_ALT)

#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/aescmac/AESCMACCC26XX.h>
typedef AESCMACCC26XX_Object AESCMAC_Object;
typedef AESCMACCC26XX_HWAttrs AESCMAC_HWAttrs;

/**
 * \brief
 */
struct mbedtls_cmac_context_t
{
    int mode;                    /*!<  operation to perform */
    CryptoKey cryptoKey;         /*!<  input to the crypto driver  */
    uint8_t keyMaterial[32];     /*!<  storage for the key   */
    AESCMAC_Handle handle;       /*!<  Platform AESCMAC handle  */
    AESCMAC_Config cmacConfig;   /*!<  structure containing AESCMAC driver specific implementation  */
    AESCMAC_Object cmacObject;   /*!<  Pointer to a driver specific data object */
    AESCMAC_Operation operation; /*!<  Platform AESCMAC operation - required for segmented operations */
};

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_CMAC_ALT */

#endif /* MBEDTLS_CMAC_ALT_H */
