/******************************************************************************

 @file sm_ecc.h

 @brief TI 15.4 Security Manager, ECDH Event for P-256 and Diffie-Hellman keys

 Group: LPRF
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2025, Texas Instruments Incorporated
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

 ******************************************************************************
 
 
 *****************************************************************************/
#ifndef SM_ECC_TI154_H
#define SM_ECC_TI154_H
#ifdef FEATURE_SECURE_COMMISSIONING
/******************************************************************************
 Includes
 *****************************************************************************/

#include "ti_154stack_config.h"
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/ECDH.h>

#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* Size of Private key material in bytes */
#define SM_ECC_PRIVATE_KEY_SIZE (32)

/* Size of public key material in bytes */
/* Add one for the extra formatting byte at the front */
#define SM_ECC_PUBLIC_KEY_SIZE (2 * SM_ECC_PRIVATE_KEY_SIZE + 1)

/******************************************************************************
 Structures & Function pointers
 *****************************************************************************/

/*! SM ECC Security Device Descriptor */
typedef struct _sm_ecc_secdevicedescriptor
{
    /*! Local Private key */
	CryptoKey localPrivateKey;

    /*! Local Public key */
	CryptoKey localPublicKey;

    /* Local private key material storage*/
    uint8_t localPrivateKeyMaterial[SM_ECC_PRIVATE_KEY_SIZE];

    /* Local public key material storage */
    uint8_t localPublicKeyMaterial[SM_ECC_PUBLIC_KEY_SIZE];

    /* Foreign public key material storage, obtained from over the 15.4 network */
    uint8_t foreignPublicKeyMaterial[SM_ECC_PUBLIC_KEY_SIZE];

    /* Shared secret key material storage, obtained from over the 15.4 network */
    uint8_t sharedSecretKeyMaterial[SM_ECC_PUBLIC_KEY_SIZE];

    /*! ECDH derived shared secret */
	CryptoKey sharedSecretKey;
} SM_ECC_securityDeviceDescriptor_t;



/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief       Initialize this module for ECDH operation.
 */
extern void SM_ECC_init(void);

/*!
 * @brief       Generate a local key pair for ECDH.
 *              <BR>
 *              This function will take the application's MAC device descriptor
 *              structure, security information and a parameter to start the key
 *              refreshment process.
 *
 * @param       secDevice - pointer to an empty security device structure to
 *                          populate with the newly generated local public
 *                          and public keys.
 */
extern bool SM_ECC_genLocalKeyPair(SM_ECC_securityDeviceDescriptor_t *secDevice);


/*!
 * @brief       Register Security manager callback functions
 *              refreshment process.
 *
 * @pre         SM_ECC_genLocalKeyPair must be called first to populate the local
 *              public/private key-pair. 
 * @param       secDevice - pointer to the previously populated security device
 *                          with information for the local public/private keys in
 *                          addition to the foreign public key information.
 *    
 *                          This function will populate the security descriptor
 *                          newly generated shared secret key.
 */
extern bool SM_ECC_genSharedSecretKey(SM_ECC_securityDeviceDescriptor_t *secDevice);

/*!
 * @brief       Function to generate random number like private key material etc.
 *
 * @pre         None
 *
 * @param       buf - pointer to the array to hold the generated random number.
 * @param       keyLen - Length of the random number to be generated in Bytes.
 */
extern void SM_ECC_genRandomPrivateKeyMaterial(uint8_t *buf, uint8_t keyLen);

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif
#endif /*FEATURE_SECURE_COMMISSIONING*/
#endif /* SM_ECC_H */

