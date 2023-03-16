/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
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

#ifndef CRYPTO_HELPER_H
#define CRYPTO_HELPER_H

#include <ti/drivers/cryptoutils/ecc/ECCParams.h>
#include <ti/drivers/SHA2.h>

/*
 * Error mapping functions. These functions take as input
 * SL driver error codes and maps them to nearest PSA error
 * code.
 */
psa_status_t map_SHA2Error_to_PSA_error(int_fast16_t status);
psa_status_t map_EDDSAError_to_PSA_error(int_fast16_t status);
psa_status_t map_ECDSAError_to_PSA_error(int_fast16_t status);
psa_status_t map_ECDHError_to_PSA_error(int_fast16_t status);
psa_status_t map_TRNGError_to_PSA_error(int_fast16_t status);
psa_status_t map_AESError_to_PSA_error(int_fast16_t status);

/*
 * Mapping functions between MBEDTLS keytype values
 * and the corresponding MBEDCRYPTO keytype values.
 * For ECC keys, keyBits is required to map from the curve
 * families used by MBEDTLS and the curve types used by MBEDCRYPTO.
 */
psa_key_type_t map_PSAKeyType_to_KeyStoreKeyType(psa_key_type_t keyType, size_t keyBits);
psa_key_type_t map_KeyStoreKeyType_to_PSAKeyType(psa_key_type_t keyType);

/*
 * Mapping function from MBEDTLS alg values to their corresponding
 * MBEDCRYPTO alg values. Some of the mappings are awkward because
 * the SL drivers expect those alg values.
 */
psa_algorithm_t map_PSAKeyAlg_to_KeyStoreKeyAlg(psa_algorithm_t alg);

/*
 * Convert to and from the keyID format expected
 * by the SL KeyStore API.
 */
psa_key_id_t toKeyID(psa_key_file_id_t keystoreKeyID);
psa_key_file_id_t toKeyStoreKeyID(psa_key_id_t keyID);

/*
 * Maps PSA ECC key types to their corresponding ECCParams_CurveParams
 * struct. Assumes that the keyType is some form of ECC key type.
 * Returns NULL if could not find a mapping.
 */
const ECCParams_CurveParams *map_keyTypeToECCParams(psa_key_type_t keyType, size_t keyBits);

/*
 * Maps PSA hash alg values to their corresponding SHA2_HashType
 * value. Assumes that the alg some form of SHA2. Caller must verify
 * this.
 */
SHA2_HashType map_hashAlgToHashType(psa_algorithm_t alg);

#endif