/******************************************************************************

 @file sm_toolbox.h

 @brief TI 15.4 Security Manager, Generate derived keying material

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
#ifndef SM_TOOLBOX_H
#define SM_TOOLBOX_H

#ifdef FEATURE_SECURE_COMMISSIONING
/******************************************************************************
 Includes
 *****************************************************************************/

#include "ti_154stack_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Definitions
 *****************************************************************************/
//#define SM_TEST
//#define SM_TEST_AESECB
//#define SM_TEST_SUBKEY
//#define SM_TEST_CMAC
//#define SM_TEST_F1
//#define SM_TEST_F2
//#define SM_TEST_F3



/******************************************************************************
 Function Prototypes
 *****************************************************************************/
/*********************************************************************
 * @fn          SM_AESECB_init
 *
 * @brief       Initialize this module for ECDH operation.
 */
void SM_AESECB_init(void);

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
uint8_t SM_f1( uint8_t *U, uint8_t *V, uint8_t *X, uint8_t Z, uint8_t *pOut );


/*********************************************************************
 * @fn          SM_f2
 *
 * @brief       Generate derived keying material for the device key and keys for the
 *              commitment function f3.
 *
 * @param       W      - DHKey. 256 bits.
 * @param       N1     - Initiator's Random Number. 128 bits.
 * @param       N2     - Responder's Random Number. 128 bits.
 * @param       A1     - Local (Initiator) IEEE address. 64 bits.
 * @param       A2     - Remote (Responder) IEEE address. 64 bits.
 * @param       pOut   - pointer to output buffer.  2x128 bits.  the lower 128
 *                       bits of output is the ConKey in big endiant order. the
 *                       upper 128 bits of the output is the Device Key in big endian
 *                       order.
 *
 * @return     status - SM_SUCCESS if successful
 *                      SM_INVALID_PARAM if input parameters not allocated.
 *                      SM_MEMALLOC_ERROR if memory allocation failed.
 */
uint8_t SM_f2( uint8_t *W, uint8_t *N1, uint8_t *N2, uint8_t *A1, uint8_t *A2, uint8_t *pOut );



/*********************************************************************
 * @fn          SM_f3
 *
 * @brief       Generate check values for device key.
 *
 * @param       W      - ConKey output of f2. 128 bits.
 * @param       N1     - Local device's Random Number. 128 bits.
 * @param       N2     - Remote device's Random Number. 128 bits.
 * @param       R      - 128 bits.
 * @param       IOCap  - I/O capabilities. 24 bits.
 * @param       A1     - this device's address. 56 bits.
 * @param       A2     - other device's address. 56 bits.
 * @param       pOut - pointer to output buffer. 128 bits.
 *
 * @return     status - SM_SUCCESS if successful
 *                      SM_INVALID_PARAM if input parameters not allocated.
 *                      SM_MEMALLOC_ERROR if memory allocation failed.
 */
uint8_t SM_f3( uint8_t *W, uint8_t *N1, uint8_t *N2, uint8_t *R, uint8_t * IOCap, uint8_t *A1, uint8_t *A2, uint8_t *pOut );




#ifdef __cplusplus
}
#endif
#endif /*FEATURE_SECURE_COMMISSIONING*/
#endif /* SM_ECC_H */

