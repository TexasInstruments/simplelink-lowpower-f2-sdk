/******************************************************************************

 @file sm_toolbox.c

 @brief TI 15.4 Security Manager
This file contains functions to generate derived keying material 
      
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

/******************************************************************************
 Includes
 *****************************************************************************/
#include "ti_154stack_features.h"

#ifdef FEATURE_SECURE_COMMISSIONING
#include <string.h>
#include <stdint.h>

#include "mac_util.h"
#include "ti_154stack_config.h"
#include "sm_toolbox.h"
#include "sm_ti154.h"
#include "mac_api.h"
#include "icall_osal_rom_jt.h"
#include "crypto_mac_api.h"	

#include "osal_port.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/******************************************************************************
 Structures
 *****************************************************************************/


/******************************************************************************
 Global variables
 *****************************************************************************/
#define SM_F1_INPUT_LEN         (SM_ECC_KEYLEN * 2) + 1 // 65 bytes
#define SM_ADDRESS_LEN          7
#define SM_F2_INPUT_LEN         53
#define SM_F3_INPUT_LEN         65
#define SM_F3_IOCAP_LEN         3





/******************************************************************************
 Local variables
 *****************************************************************************/
/* AESECB Test Vector */
#if defined(SM_TEST)

uint8_t  keyMaterial[SM_KEY_LENGTH] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                                       0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
#endif

#if defined(SM_TEST)&& defined(SM_TEST_SUBKEY)
uint8_t  Testplaintext[SM_AES_BLOCK_LEN]  = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                                                   0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

const uint8_t  CipherTextVector[SM_AES_BLOCK_LEN]  = {0x7d, 0xf7, 0x6b, 0x0c, 0x1a, 0xb8, 0x99, 0xb3,
                                                      0x3e, 0x42, 0xf0, 0x47, 0xb9,0x1b, 0x54, 0x6f};

const uint8_t  K1[SM_AES_BLOCK_LEN]  = {0xfb, 0xee, 0xd6, 0x18, 0x35, 0x71, 0x33, 0x66,
                                                      0x7c, 0x85, 0xe0, 0x8f, 0x72,0x36, 0xa8, 0xde};

const uint8_t  K2[SM_AES_BLOCK_LEN]  = {0xf7, 0xdd, 0xac, 0x30, 0x6a, 0xe2, 0x66, 0xcc,
                                                      0xf9, 0x0b, 0xc1, 0x1e, 0xe4,0x6d, 0x51, 0x3b};

#endif

#if defined(SM_TEST)&& defined(SM_TEST_CMAC)
unsigned char M[64] =
{
  0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
  0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
  0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c,
  0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
  0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11,
  0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
  0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17,
  0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10
};

uint8_t CMAC[SM_AES_BLOCK_LEN];

#endif

// Secure Connections test vectors and known outputs
// These
#if defined(SM_TEST_F1)
  // Little endian
  uint32_t testU[] = {0x0E359DE6, 0xCC030148, 0xACF4FDDB, 0xEFF49111, 0xE9F9A5B9, 0x5E2C83A7, 0xF297BE2C, 0x20B003D2};

  uint32_t testV[] = {0xF47FC5FD, 0x6B4FDD49, 0xF19D7CFB, 0x59CB9AC2, 0xEED4E72A, 0x900AFCFB, 0x32F6BB9A, 0x55188B3D};

  uint32_t testX[] = {0x712BAEAB, 0xFFFFB2EC, 0xD177733E, 0xD5CB8454};

  uint8_t testZ = 0;

  // Big endian
  uint32_t F1_OUTPUT_VECTOR[] = { 0xF2C916F1, 0x07A9BD1C, 0xF1EDA1BE, 0xA974872D };

  uint8_t F1_output[SM_KEY_LENGTH];

#endif //SM_TEST_F1

#if defined(SM_TEST_F2)
  // Little endian
  uint32 testW[] = {0x73bfa698, 0x868d34f3, 0xb4f866f1, 0x99796b13, 0x0a397d9b, 0x341010a6, 0x57c8ad05, 0xec0234a3};

  uint32 testkeyID = 0x62746c65;
  uint32 testN1[] = {0x712baeab, 0xffffb2ec, 0xd177733e, 0xd5cb8454};
  uint32 testN2[] = {0xff3dc4cf, 0x216583f7, 0x25a75f6e, 0xa6e8e7cc};
  uint8_t testA1[7] = {0xCE, 0xBF, 0x37, 0x37, 0x12, 0x56, 0x00};
  uint8_t testA2[7] = {0xC1, 0xCF, 0x2D, 0x70, 0x13, 0xA7, 0x00};

  // Big endian
  uint32 testT[] = {0x3c128f20, 0xde883288, 0x97624bdb, 0x8dac6989};
  uint32 F2_DKEY_AES_CMAC[] = {0x69867911, 0x69d7cd23, 0x980522b5, 0x94750a38};
  uint32 F2_CONKEY_AES_CMAC[] = {0x2965f176, 0xa1084a02, 0xfd3f6a20, 0xce636e20};

  uint8_t F2_output [SM_KEY_LENGTH *2];
  uint32_t temp[4];
#endif //SM_TEST_F2

#if defined(SM_TEST_F3)
  // Little endian
  uint32 testN1[] = {0x712baeab, 0xffffb2ec, 0xd177733e, 0xd5cb8454};
  uint32 testN2[] = {0xff3dc4cf, 0x216583f7, 0x25a75f6e, 0xa6e8e7cc};
  uint32 testConKey[] = {0xce636e20, 0xfd3f6a20, 0xa1084a02, 0x2965f176};
  uint32 testR[] = {0x0c2d0fc8, 0x08da42d2, 0xb453bb54, 0x12a3343b};
  uint8_t testAu[] = {0x02, 0x01, 0x01};
  uint8_t testA1[7] = {0xCE, 0xBF, 0x37, 0x37, 0x12, 0x56, 0x00};
  uint8_t testA2[7] = {0xC1, 0xCF, 0x2D, 0x70, 0x13, 0xA7, 0x00};

  // Big endian
  uint32 F3_AES_CMAC[] = {0xe3c47398, 0x9cd0e8c5, 0xd26c0b09, 0xda958f61};
  uint8_t F3_output[SM_KEY_LENGTH];
#endif //SM_TEST_F3


// SM Encryption related definitions
CONST uint8_t const_Rb_ti154[SM_KEY_LENGTH] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87
};

extern AESECB_Handle       AESECB_handle;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
/*********************************************************************
 * @fn          SM_CMAC
 *
 * @brief       An implementation of the CMAC algorithm.
 *
 * NOTE:  All of the input and output buffers are MSByte first.
 *        The calling function must take care of the byte order, before
 *        and after calling this function
 *
 * @param       pK - key
 * @param       pM - data
 * @param       mLen - data length
 * @param       pMac - pointer to result
 * @param       macLen - result length (up to 16 bytes)
 *
 * @return      uint8_t
 */
uint8_t SM_CMAC( uint8_t *pK, uint8_t *pM, uint16_t mLen, uint8_t *pMac, uint8_t macLen );

uint8_t SM_generate_subkey( uint8_t *pKey, uint8_t *pSubK1, uint8_t *pSubK2 );

void SM_leftshift_onebit( uint8_t *pInp, uint8_t *pOutp );

void SM_xor_128( uint8_t *pA, CONST uint8_t *pB, uint8_t *pOutcome );

void SM_padding ( uint8_t *pLastb, uint8_t *pPad, uint8_t length );

uint8_t SM_AESECB_encrypt(uint8_t *key, uint8_t *ciphertext, uint8_t *plaintext);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*********************************************************************
 * @fn          SM_f1
 *
 * @brief       Generate confirm values during the pairing process
 *
 * @param       U     - Public Key, local devices x-coordinate. 256 bits.
 * @param       V     - Public Key, remote device's x-coordinate.
 * @param       X     - The key. 128 bits.
 * @param       Z     - passkey, 8 bits
 *                      PASSKEY_TEST_SIZE = PASSKEY_BIT => Z = 1 LSB, MSB =1
 *                      PASSKEY_TEST_SIZE =PASSKEY_ONEDIGIT => Z = 4 LSB, MSB =0
 * @param      pOut - pointer to output buffer. 128 bits.
 *
 * @return     status - SM_SUCCESS if successful
 *                      SM_INVALID_PARAM if input parameters not allocated.
 *                      SM_MEMALLOC_ERROR if memory allocation failed.
 */
uint8_t SM_f1( uint8_t *U, uint8_t *V, uint8_t *X, uint8_t Z, uint8_t *pOut )
{

#if defined (SM_TEST)

#if defined(SM_TEST_AESECB)

    uint8_t CText[SM_AES_BLOCK_LEN];
    uint8_t status;

    status = SM_AESECB_encrypt(keyMaterial, Testplaintext, CText);
#endif

#if defined (SM_TEST_SUBKEY)
    uint8_t K1[SM_KEY_LENGTH], K2[SM_KEY_LENGTH];

    SM_generate_subkey(keyMaterial, K1, K2);
#endif

#if defined (SM_TEST_CMAC)

    SM_CMAC(keyMaterial, M, 0,CMAC, SM_AES_BLOCK_LEN );

    SM_CMAC(keyMaterial, M, 16,CMAC, SM_AES_BLOCK_LEN );

    SM_CMAC(keyMaterial, M, 40,CMAC, SM_AES_BLOCK_LEN );

    SM_CMAC(keyMaterial, M, 64,CMAC, SM_AES_BLOCK_LEN );

#endif

#else
    // (U || V || Z) is the input message m to AES-CMACx.
    // the LSB of Z is the LSB of m, the MSB of U is the MSB of m.
    // HOWEVER, m is MSB so the MSB of U is offset 0 of m. and the LSB of Z if the last octet of m.
    // m is of length 256*2 + 8 bits = 65 bytes.
    uint8_t *ptr;
    uint8_t *input;
    uint8_t *output;
    uint8_t *key;

#if defined(SM_TEST_F1)
    U = (uint8_t *)testU;
    V = (uint8_t *)testV;
    X = (uint8_t *)testX;
    Z = testZ;
    pOut = F1_output;

#endif //SM_TEST_F1

    // Parameter check.
    if ( U == NULL || V == NULL || X == NULL || pOut == NULL )
    {
        return SM_INVALID_PARAM;
    }



    // Allocate buffers.
    // input message
    if ( !( input = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_F1_INPUT_LEN ) ) )
    {
        return SM_MEMALLOC_ERROR;
    }
    // output
    if ( !( output = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH ) ) )
    {
        // Free buffer.
        OsalPort_free(input);

        return SM_MEMALLOC_ERROR;
    }
    // key to AES
    if ( !( key = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH ) ) )
    {
        // Free buffers.
        OsalPort_free(input);
        OsalPort_free(output);

        return SM_MEMALLOC_ERROR;
    }

    // clear buffers
    memset( input, 0x00, SM_F1_INPUT_LEN );
    memset( output, 0x00, SM_KEY_LENGTH );

    // Set ptr to start of input buffer
    ptr = input;

    // Copy in U (256 bits), in reverse order.
    OsalPort_revmemcpy( ptr, U, SM_ECC_KEYLEN );

    // Increment ptr
    ptr = &input[SM_ECC_KEYLEN];

    // next is V (256 bits), in reverse order.
    OsalPort_revmemcpy( ptr, V, SM_ECC_KEYLEN );

    // Increment ptr
    ptr += SM_ECC_KEYLEN;

    // Last byte is Z.
    OsalPort_revmemcpy( ptr, &Z, 1 );

    //Reverse the key
    OsalPort_revmemcpy( key, X, SM_KEY_LENGTH );

    SM_CMAC( key, input, SM_F1_INPUT_LEN , output, SM_KEY_LENGTH );

#if defined(SM_TEST_F1)
    // compare the output
    OsalPort_memcmp( output, F1_OUTPUT_VECTOR, SM_KEY_LENGTH );
    // passed the test on 7/29
#endif //SM_TEST_F1

  // reverse output into pOut.
  OsalPort_revmemcpy( pOut, output, SM_KEY_LENGTH );

  // Free allocated memory
  OsalPort_free( input );
  OsalPort_free( output );
  OsalPort_free( key );

  return SM_SUCCESS;

#endif // defined(SM_TEST_CMAC)
}


/*********************************************************************
 * @fn          SM_f2
 *
 * @brief       Generate derived keying material for the device key and keys for the
 *              commitment function f3.
 *
 * @param       W      - DHKey. 256 bits.
 * @param       N1     - Initiator's Random Number. 128 bits.
 * @param       N2     - Responder's Random Number. 128 bits.
 * @param       A1     - Local (Initiator) IEEE address. 56 bits (MSB).
 * @param       A2     - Remote (Responder) IEEE address. 56 bits (MSB).
 * @param       pOut   - pointer to output buffer.  2x128 bits.
 *                       lower index (128 bits) = Device Key in little endian
 *                       upper index (128 bits) = ConKey in little endian
 *
 * @return     status - SM_SUCCESS if successful
 *                      SM_INVALID_PARAM if input parameters not allocated.
 *                      SM_MEMALLOC_ERROR if memory allocation failed.
 */
uint8_t SM_f2( uint8_t *W, uint8_t *N1, uint8_t *N2, uint8_t *A1, uint8_t *A2, uint8_t *pOut )
{
    uint8_t counter = 0;
    uint16_t length = 0x0100;
    uint8_t *ptr;
    uint8_t *T;
    uint8_t *input;
    uint8_t *output;
    uint8_t *key;

    // SALT = 0x6C88_8391_AAF5_A538_6037_0BDB_5A60_83BE
    const uint16_t salt[8] = {0x83BE, 0x5A60, 0x0BDB, 0x6037, 0xA538, 0xAAF5, 0x8391, 0x6C88};

    // ASCII "btle" in LSB order ('b' is the most significant octet)
    const uint32 keyID   = 0x62746C65;

#if defined(SM_TEST_F2)
    W = (uint8_t *)testW;
    //T - don't test. instead make sure it is generated correctly and matches test output.
    //keyID - same as testKeyID
    N1 = (uint8_t *)testN1;
    N2 = (uint8_t *)testN2;
    A1 = (uint8_t *)testA1;
    A2 = (uint8_t *)testA2;
    //Length is the same
    pOut = F2_output;
#endif //SM_TEST_F2

    if ( W == NULL || N1 == NULL || N2 == NULL || A1 == NULL || A2 == NULL ||
       pOut == NULL )
    {
        return SM_INVALID_PARAM;
    }



    // Allocate buffers.
    if ( !( input = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_F2_INPUT_LEN ) ) )
    {
        return SM_MEMALLOC_ERROR;
    }

    if ( !( output = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH * 2 ) ) )
    {
        // Free buffer.
        OsalPort_free( input );

        return SM_MEMALLOC_ERROR;
    }

    if ( !( key = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH ) ) )
    {
        // Free buffers.
        OsalPort_free( input );
        OsalPort_free( output );

        return SM_MEMALLOC_ERROR;
    }

    if ( !( T = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH ) ) )
    {
        // Free buffers.
        OsalPort_free( input );
        OsalPort_free( output );
        OsalPort_free( key );

        return SM_MEMALLOC_ERROR;
    }

    // clear output buffer
    memset( output, 0x00, SM_KEY_LENGTH * 2 );

    /* part 1: generate T with key salt and message W. */
    // Reverse salt into key.
    OsalPort_revmemcpy( key, (uint8_t *)salt, SM_KEY_LENGTH );

    // reverse dubu-ah.
    OsalPort_revmemcpy( input, W, SM_KEY_LENGTH * 2 );

    // generate T
    SM_CMAC( (uint8_t *)key, input, SM_KEY_LENGTH * 2, T, SM_KEY_LENGTH );

#if defined(SM_TEST_F2)
    // See if T matches.
    OsalPort_memcpy( temp, T, SM_KEY_LENGTH );
    OsalPort_memcmp( testT, temp, SM_KEY_LENGTH );
#endif //SM_TEST_F2

    /* part 2: use generated key 'T' to generate LTK. */

    // Set ptr to input.
    ptr = input;

    // Store counter (0) and increment ptr, then increment counter.
    *ptr++ = counter++;

    // keyID is next.
    OsalPort_revmemcpy( ptr, (uint8_t *)&keyID, sizeof (keyID) );

    // Increment ptr.
    ptr += sizeof ( keyID );

    // N1 is next, in reverse order.
    OsalPort_revmemcpy( ptr, N1, SM_KEY_LENGTH );

    // Increment ptr.
    ptr += SM_KEY_LENGTH;

    // N2 is next, in reverse order.
    OsalPort_revmemcpy( ptr, N2, SM_KEY_LENGTH );

    // Increment ptr.
    ptr += SM_KEY_LENGTH;

    // A1 is next, in reverse order
    OsalPort_revmemcpy( ptr, A1, SM_ADDRESS_LEN );

    // Increment ptr.
    ptr += SM_ADDRESS_LEN;

    // A2 is next, in reverse order
    OsalPort_revmemcpy( ptr, A2, SM_ADDRESS_LEN );

    // Increment ptr.
    ptr += SM_ADDRESS_LEN;

    // Last is length, in reverse order
    OsalPort_revmemcpy( ptr, (uint8_t *)&length, sizeof (length) );

    /*
     * Generate the ConKey.
     * "The ConKey is the most significant 128 bits (Counter = 0) of f2." Meaning
     * that it is the lower 128 bits of the output buffer, in big-endian order.
     */
    SM_CMAC( T, input, SM_F2_INPUT_LEN, output, SM_KEY_LENGTH );

    /* part 3: use generated key 't' to generate MacKey. */

    // update m with the incremented counter value (1).
    *input = counter;

    /*
     * Generate the Device Key.
     * "The Device Key is the least significant 128 bits (Counter = 1) of f2." Meaning
     * that it is the upper 128 bits of the output buffer, in big-endian order.
     */
    SM_CMAC( T, input, SM_F2_INPUT_LEN, &output[SM_KEY_LENGTH], SM_KEY_LENGTH );

#if defined(SM_TEST_F2)
    // test MacKey output
    OsalPort_memcpy( temp, output, SM_KEY_LENGTH );
    OsalPort_memcmp((uint8_t *)F2_CONKEY_AES_CMAC, temp, SM_KEY_LENGTH );

    OsalPort_memcpy( temp, &output[SM_KEY_LENGTH], SM_KEY_LENGTH );
    // test LTK output
    OsalPort_memcmp( (uint8_t *)F2_DKEY_AES_CMAC, temp, SM_KEY_LENGTH );
#endif //SM_TEST_F2

    /*
     * reverse copy output into pOut
     * This makes the lower 128 bits the LTK in little endian order.
     * This makes the upper 128 bits the MacKey in little endian order.
     */
    OsalPort_revmemcpy( pOut, output, SM_KEY_LENGTH * 2 );

    // Free allocated memory
    OsalPort_free( input );
    OsalPort_free( output );
    OsalPort_free( key );
    OsalPort_free( T );

  return SM_SUCCESS;
}

/*********************************************************************
 * @fn          SM_f3
 *
 * @brief       Generate check values for device key.
 *
 * @param       W      - ConKey output of f2. 128 bits.
 * @param       N1     - Local device's Random Number. 128 bits.
 * @param       N2     - Remote device's Random Number. 128 bits.
 * @param       R      - 128 bits.
 * @param       Au  - Sensor Device Short Address | 0x800000. 24 bits.
 * @param       A1     - Local (Initiator) IEEE address. 56 bits (MSB).
 * @param       A2     - Remote (Initiator) IEEE address. 56 bits (MSB).
 * @param       pOut - pointer to output buffer. 128 bits.
 *
 * @return     status - SM_SUCCESS if successful
 *                      SM_INVALID_PARAM if input parameters not allocated.
 *                      SM_MEMALLOC_ERROR if memory allocation failed.
 */
uint8_t SM_f3( uint8_t *W, uint8_t *N1, uint8_t *N2, uint8_t *R, uint8_t * Au, uint8_t *A1, uint8_t *A2, uint8_t *pOut )
{
    uint8_t *ptr;
    uint8_t *input;
    uint8_t *output;
    uint8_t *key;

#if defined(SM_TEST_F3)
    W = (uint8_t *)testConKey;
    N1 = (uint8_t *)testN1;
    N2 = (uint8_t *)testN2;
    R = (uint8_t *)testR;
    Au = testAu;
    A1 = (uint8_t *)testA1;
    A2 = (uint8_t *)testA2;
    pOut =F3_output;
#endif //SM_TEST_F3

    // Parameter check
    if ( W == NULL || N1 == NULL || N2 == NULL || R == NULL || Au == NULL ||
            A1 == NULL || A2 == NULL || pOut == NULL )
    {
        return SM_INVALID_PARAM;
    }



    // Allocate buffers.
    if ( !( input = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_F3_INPUT_LEN ) ) )
    {
        return SM_MEMALLOC_ERROR;
    }

    if ( !( output = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH ) ) )
    {
        // Free buffer.
        OsalPort_free( input );

        return SM_MEMALLOC_ERROR;
    }

    if ( !( key = (uint8_t *)OsalPort_malloc( sizeof ( uint8_t ) * SM_KEY_LENGTH ) ) )
    {
        // Free buffers.
        OsalPort_free( input );
        OsalPort_free( output );

        return SM_MEMALLOC_ERROR;
    }

    // Set ptr to start of input buffer.
    ptr = input;

    // N1 goes in input buffer first, in reverse order.
    OsalPort_revmemcpy( ptr, N1, SM_KEY_LENGTH );

    // Increment ptr.
    ptr += SM_KEY_LENGTH;

    // N2 is next, in reverse order.
    OsalPort_revmemcpy( ptr, N2, SM_KEY_LENGTH );

    // Increment ptr.
    ptr += SM_KEY_LENGTH;

    // R is next, in reverse order.
    OsalPort_revmemcpy( ptr, R, SM_KEY_LENGTH );

    // Increment ptr.
    ptr += SM_KEY_LENGTH;

    // Au is next, in reverse order.
    OsalPort_revmemcpy( ptr, Au, SM_F3_IOCAP_LEN );

    // Increment ptr.
    ptr += SM_F3_IOCAP_LEN;

    // A1 is next, in reverse order
    OsalPort_revmemcpy( ptr, A1, SM_ADDRESS_LEN );

    // Increment ptr.
    ptr += SM_ADDRESS_LEN;

    // A2 is last, in reverse order
    OsalPort_revmemcpy( ptr, A2, SM_ADDRESS_LEN );

    // Reverse copy W into key
    OsalPort_revmemcpy( key, W, SM_KEY_LENGTH );

    // generate output.
    SM_CMAC( key, input, SM_F3_INPUT_LEN, output, SM_KEY_LENGTH );

#if defined(SM_TEST_F3)
    // Test output against F3_AES_CMAC
    OsalPort_memcmp( (uint8_t *)F3_AES_CMAC, output, SM_KEY_LENGTH );
#endif //SM_TEST_F3

    // copy output into pOut
    OsalPort_revmemcpy( pOut, output, SM_KEY_LENGTH );

    // Free allocated memory
    OsalPort_free( input );
    OsalPort_free( output );
    OsalPort_free( key );

    return SM_SUCCESS;
}

/******************************************************************************
 Local Functions
 *****************************************************************************/

/*********************************************************************
 * @fn          SM_CMAC
 *
 * @brief       An implementation of the CMAC algorithm.
 *
 * NOTE:  All of the input and output buffers are MSByte first.
 *        The calling function must take care of the byte order, before
 *        and after calling this function
 *
 * @param       pK - key
 * @param       pM - data
 * @param       mLen - data length
 * @param       pMac - pointer to result
 * @param       macLen - result length (up to 16 bytes)
 *
 * @return      uint8_t
 */

uint8_t SM_CMAC( uint8_t *pK, uint8_t *pM, uint16_t mLen, uint8_t *pMac, uint8_t macLen ){

    int16 n, i, flag;
    uint8_t *pK1;
    uint8_t *pK2;
    uint8_t stat;

    // Allocate RAM needed
    pK1 = OsalPort_malloc( SM_KEY_LENGTH );
    pK2 = OsalPort_malloc( SM_KEY_LENGTH );

    // Make sure they allocated
    if ( (pK1 != NULL) && (pK2 != NULL) )
    {
        stat = SM_generate_subkey( pK, pK1, pK2 );
        if ( stat == SM_SUCCESS )
        {
            uint8_t  *pX;
            uint8_t  *pM_last;

            pX = OsalPort_malloc( SM_KEY_LENGTH );
            pM_last = OsalPort_malloc( SM_KEY_LENGTH );

            if ( (pX != NULL) && (pM_last != NULL) )
            {
                uint16_t  mIdx;

                n = (mLen + 15)/16;  /* n is number of rounds */

                if ( n == 0 )
                {
                    n = 1;
                    flag = 0;
                }
                else
                {
                    if ( (mLen%16) == 0 ) /* last block is a complete block */
                    {
                        flag = 1;
                    }
                    else /* last block is not complete block */
                    {
                        flag = 0;
                    }
                }

                mIdx = (uint16_t)(16*(n-1));

                if ( flag ) /* last block is complete block */
                {
                    SM_xor_128( &pM[mIdx], pK1, pM_last );
                }
                else
                {
                    uint8_t padded[SM_KEY_LENGTH];
                    SM_padding( &pM[mIdx], padded, mLen%16 );
                    SM_xor_128( padded, pK2, pM_last );
                }

                VOID memset( pX, 0, SM_KEY_LENGTH );

                {
                    uint8_t Y[SM_KEY_LENGTH];

                    for ( i = 0; (i < (n-1)) && (stat == SM_SUCCESS); i++ )
                    {
                        mIdx = (uint16_t)(16*i);
                        SM_xor_128( pX, &pM[mIdx], Y ); /* Y := Mi (+) X  */
                        stat = SM_AESECB_encrypt( pK, Y, pX );
                    }

                    if ( stat == SM_SUCCESS )
                    {
                        SM_xor_128( pX, pM_last, Y );
                        stat = SM_AESECB_encrypt( pK, Y, pX );

                        // T = MSB[Tlen]( Cn ); T = mac, Tlen = macLen, Cn = X
                        VOID OsalPort_memcpy( pMac, pX, macLen );
                    }
                }
            }
            else
            {
                stat = SM_MEMALLOC_ERROR;
            }

            if ( pX )
            {
                OsalPort_free( pX );
            }

            if ( pM_last )
            {
                OsalPort_free( pM_last );
            }
        }
    }
    else
    {
        stat = SM_MEMALLOC_ERROR;
    }

    if ( pK1 )
    {
        OsalPort_free( pK1 );
    }

    if ( pK2 )
    {
        OsalPort_free( pK2 );
    }

    return ( stat );
}




/*********************************************************************
 * @fn          SM_generate_subkey
 *
 * @brief       Generate CMAC Subkeys.  Only call from SM_CMAC.
 *
 * @param       pKey - key
 * @param       pSubK1 - sub key 1
 * @param       pSubK2 - sub key 2
 *
 * @return      uint8_t
 */
uint8_t SM_generate_subkey( uint8_t *pKey, uint8_t *pSubK1, uint8_t *pSubK2 )
{
    uint8_t *pL;
    uint8_t *pTmp;
    uint8_t stat;

    pL = OsalPort_malloc( SM_KEY_LENGTH );
    pTmp = OsalPort_malloc( SM_KEY_LENGTH );

    if ( (pL != NULL) && (pTmp != NULL) )
    {
        memset( pTmp, 0, SM_KEY_LENGTH );

        stat = SM_AESECB_encrypt( pKey, pTmp, pL );
        if ( stat == SM_SUCCESS )
        {
            if ( (pL[0] & 0x80) == 0 ) /* If MSB(L) = 0, then K1 = L << 1 */
            {
                SM_leftshift_onebit( pL, pSubK1 );
            }
            else /* Else pK1 = ( pL << 1 ) (+) Rb */
            {
                SM_leftshift_onebit( pL, pTmp );
                SM_xor_128( pTmp, const_Rb_ti154, pSubK1 );
            }

            if ( (pSubK1[0] & 0x80) == 0 )
            {
                SM_leftshift_onebit( pSubK1, pSubK2 );
            }
            else
            {
                SM_leftshift_onebit( pSubK1, pTmp );
                SM_xor_128( pTmp, const_Rb_ti154, pSubK2 );
            }
        }
    }
    else
    {
        stat = SM_MEMALLOC_ERROR;
    }

    if ( pL )
    {
        OsalPort_free( pL );
    }
    if ( pTmp )
    {
        OsalPort_free( pTmp );
    }

    return ( stat );
}

/*********************************************************************
 * @fn          SM_leftshift_onebit
 *
 * @brief       Left shift 128 bits
 *
 * @param       pInp - input buffer
 * @param       pOutp - output buffer
 *
 * @return      None
 */
void SM_leftshift_onebit( uint8_t *pInp, uint8_t *pOutp )
{
  uint8_t overflow = 0;

  for ( int8_t i = 15; i >= 0; i-- )
  {
    pOutp[i] = pInp[i] << 1;
    pOutp[i] |= overflow;
    overflow = (pInp[i] & 0x80) ? 1 : 0;
  }
}


/*********************************************************************
 * @fn          SM_xor_128
 *
 * @brief       XOR 128 bits - outcome = a XOR B
 *              This function doesn't XOR in place.
 *
 * @param       pA - var 1
 * @param       pB - var 2
 * @param       pOutcome - outcome
 *
 * @return      None
 */
void SM_xor_128( uint8_t *pA, CONST uint8_t *pB, uint8_t *pOutcome )
{
  for ( uint8_t i = 0; i < SM_KEY_LENGTH; i++ )
  {
    pOutcome[i] = pA[i] ^ pB[i];
  }
}


/*********************************************************************
 * @fn          SM_padding
 *
 * @brief       add padding to "pad", first copy lastb, then on the last
 *              byte of the "lastb" set to 0x80, the rest of the
 *              buffer set to 0.
 *
 * @param       pLastb - previous buffer (last block)
 * @param       pPad - new buffer
 * @param       length - length non-padding
 *
 * @return      None
 */
void SM_padding ( uint8_t *pLastb, uint8_t *pPad, uint8_t length )
{
  /* original last block */
  for ( uint8_t j = 0; j < SM_KEY_LENGTH; j++ )
  {
    if ( j < length )
    {
      pPad[j] = pLastb[j];
    }
    else if ( j == length )
    {
      pPad[j] = 0x80;    // mark the last non-pad byte
    }
    else
    {
      pPad[j] = 0x00;
    }
  }
}

/******************************************************************************
 Local Functions - AES ECB interface
 *****************************************************************************/
/*******************************************************************************
 * @fn          SM_AESECB_encrypt
 *
 * @brief        Initialize this module for AESECB operation.
 */
void SM_AESECB_init(void) {

    AESECB_init();
    /* Open AESCCM_open */
    AESECB_handle = AESECB_open(0, NULL);
    if (!AESECB_handle) {
        /* AESECM_open_open() failed */
        while(1);
    }

}

/*******************************************************************************
 * @fn          SM_AESECB_encrypt
 *
 * @brief       This function takes a key, ciphertext, and generates plaintext
 *              by AES128 decryption (ECB). Note: key length and plaintext length must be 16 bytes!
 *
 * input parameters
 * @param       key        - pointer, The 128 bit key to be used for encyption.
 * @param       plaintext - pointer, The 128 bit cipher text to be encypted.
 *
 * output parameters
 * @param       ciphertext  - pointer, The 128 bit plain text after encyption.
 *
 * @return      uint8_t
 */
uint8_t SM_AESECB_encrypt(uint8_t *key, uint8_t *plaintext, uint8_t *ciphertext ) {

    /* CryptoKey storage */
    CryptoKey           cryptoKey;
    int_fast16_t        AESstatus;
    uint8_t status;

    /* Initialize the key structure */
    AESstatus = CryptoKeyPlaintext_initKey(&cryptoKey, (uint8_t*) key, SM_KEY_LENGTH);
    if (AESstatus == AESECB_STATUS_SUCCESS)
    {
        AESECB_Operation operationOneStepEncrypt;

        AESECB_Operation_init(&operationOneStepEncrypt);
        operationOneStepEncrypt.key            = &cryptoKey;
        operationOneStepEncrypt.input          = (uint8_t*)plaintext;
        operationOneStepEncrypt.output         = ciphertext;
        operationOneStepEncrypt.inputLength    = SM_AES_BLOCK_LEN;

        AESstatus = AESECB_oneStepEncrypt(AESECB_handle, &operationOneStepEncrypt);
    }
    status = (AESstatus == AESECB_STATUS_SUCCESS) ?
                SM_SUCCESS : SM_ENCRYPT_ERROR;

    return status;

}

#endif /*FEATURE_SECURE_COMMISSIONING*/
