/******************************************************************************

 @file oad_storage.h

 @brief OAD Storage Header

 Group: CMCU, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2022, Texas Instruments Incorporated
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
#ifndef OADStorage_H
#define OADStorage_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
 * CONSTANTS
 */
/// OADStorage_Status_t status codes
typedef enum {
   OADStorage_Status_Success, ///< Success
   OADStorage_Failed,         ///< Fail
   OADStorage_CrcError,       ///< Acknowledgment or Response Timed out
   OADStorage_FlashError,     ///< flash access error
   OADStorage_Aborted,        ///< Canceled by application
   OADStorage_Rejected,       ///< OAD request rejected by application
} OADStorage_Status_t;

/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      OADStorage_init
 *
 * @brief   Initialize OAD flash storage interface.
 *
 * @param   None.
 *
 * @return  None.
 */
extern void OADStorage_init(void);

/*********************************************************************
 * @fn      OADStorage_imgBlockWrite
 *
 * @brief   Write Image Block.
 *
 * @param   blockNum   - block number to be written
 * @param   blockLen   - length of block (full block size)
 * @param   pBlockData - pointer to data to be written
 * @param   dataLen    - length of data to be written (may be <= blockLen)
 *
 *
 * @return  status
 */
extern OADStorage_Status_t OADStorage_imgBlockWrite(uint32_t blockNum, uint16_t blockLen, uint8_t *pBlockData, uint16_t dataLen);

/*********************************************************************
 * @fn      OADStorage_eraseImg
 *
 * @brief   Erases the required number of flash pages in preparation
 *          for a new image.
 *
 * @param   image_len Length of flash to be erased to prepare for the
 *                    new image.
 *
 * @return  OADStorage_Status_t
 */
extern OADStorage_Status_t OADStorage_eraseImg(uint32_t imageLen);

/*********************************************************************
 * @fn      OADStorage_close
 *
 * @brief   Releases the OAD flash storage interface.
 *
 * @param  none
 *
 * @return none
 */
extern void OADStorage_close(void);
/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OADStorage_H */
