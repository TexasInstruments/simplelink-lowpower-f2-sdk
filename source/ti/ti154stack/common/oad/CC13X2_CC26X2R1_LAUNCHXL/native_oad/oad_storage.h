/******************************************************************************

 @file oad_storage.h

 @brief OAD Storage Header

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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

#include <oad_image_header.h>

#ifndef __unix__
#include <ti_154stack_oad_config.h>
#endif

/*********************************************************************
 * CONSTANTS
 */
// OAD payload overheads
#define OADStorage_ATT_OVERHEAD                    3
#define OADStorage_BLK_NUM_HDR_SZ                  2 //4

#ifndef OAD_BLOCK_SIZE
#ifdef FREQ_2_4G
/* 2.4G defaults to a 64 byte block size */
#define OADStorage_BLOCK_SIZE              64 + OADStorage_BLK_NUM_HDR_SZ
#else
/* Sub1G defaults to a 128 byte block size */
#define OADStorage_BLOCK_SIZE              128 + OADStorage_BLK_NUM_HDR_SZ
#endif /* FREQ_2_4G */
#else
#define OADStorage_BLOCK_SIZE              OAD_BLOCK_SIZE + OADStorage_BLK_NUM_HDR_SZ
#endif /* OAD_BLOCK_SIZE */

 /// OADStorage_Status_t status codes
typedef enum {
    OADStorage_Status_Success, ///< Success
    OADStorage_Failed,         ///< Fail
    OADStorage_CrcError,       ///< Acknowledgment or Response Timed out
    OADStorage_FlashError,     ///< flash access error
    OADStorage_txError,        ///< Error code returned by block request tx
    OADStorage_Aborted,        ///< Canceled by application
    OADStorage_Rejected,       ///< OAD request rejected by application
} OADStorage_Status_t;

/* Image Identify Payload */
typedef struct __attribute__((packed))
{
    uint8_t   imgID[8];       //!< User-defined Image Identification bytes. */
    uint8_t   bimVer;         //!< BIM version */
    uint8_t   metaVer;        //!< Metadata version */
    uint8_t   imgCpStat;      //!< Image copy status bytes */
    uint8_t   crcStat;        //!< CRC status */
    uint8_t   imgType;        //!< Image Type */
    uint8_t   imgNo;          //!< Image number of 'image type' */
    uint32_t  len;            //!< Image length in bytes
    uint8_t   softVer[4];     //!< Software version of the image */
    uint8_t   isDeltaImg;     //!< Indicates if payload is delta image  */
    uint8_t   toadMetaVer;    //!< Turbo OAD header version  */
    uint8_t   toadVer;        //!< Turbo OAD version  */
    uint8_t   memoryCfg;      //!< Flash configuration used */
    uint32_t  oldImgCrc;      //!< CRC of the current app image  */
    uint32_t  newImgLen;      //!< Length of the new app image  */
} OADStorage_imgIdentifyPld_t;

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */


/*********************************************************************
 * FUNCTIONS
 */

/*********************************************************************
 * @fn      OADStorage_init
 *
 * @brief   Initialise the OAD Target Profile.
 *
 * @param   None.
 *
 * @return  None.
 */
extern void OADStorage_init(void);

/*********************************************************************
 * @fn      OADStorage_imgIdentifyRead
 *
* @brief   Read Image header and return number of blocks.
 *
 * @param   imageType   - image type indicating which image to read
 * @param   pImgHdr     - pointer to image header data
 *
 * @return  Total Blocks if image accepted, 0 if Image invalid
 */
uint16_t OADStorage_imgIdentifyRead(uint8_t imageType, OADStorage_imgIdentifyPld_t* pImgId);

/*********************************************************************
 * @fn      OADStorage_imgIdentifyWrite
 *
 * @brief   Process the Image Identify Write.  Determine from the received OAD
 *          Image Header if the Downloaded Image should be acquired.
 *
 * @param   pValue     - pointer to data to be written
 *
  * @return  Total Blocks if image accepted, 0 if Image rejected
 */
extern uint16_t OADStorage_imgIdentifyWrite(uint8_t *pValue);

/*********************************************************************
 * @fn      OADStorage_imgBlockRead
 *
 * @brief   Read Image Block.
 *
 * @param   blockNum   - block number to be written
 * @param   pBlockData - pointer for data to be read
 *
 * @return  none
 */
extern OADStorage_Status_t OADStorage_imgBlockRead(uint16_t blockNum, uint8_t *pBlockData);

/*********************************************************************
 * @fn      OADStorage_imgInfoRead
 *
 * @brief   Read an Image info.
 *
 * @param   pimgInfo - pointer for data to be read
 *
 * @return  none
 */
extern void OADStorage_imgInfoRead(uint8_t *pimgInfo);

/*********************************************************************
 * @fn      OADStorage_imgBlockWrite
 *
 * @brief   Write Image Block.
 *
 * @param   blockNum   - block number to be written
 * @param   pBlockData - pointer to data to be written
 * @param   len        - length of the block
 *
 * @return  status
 */
extern OADStorage_Status_t OADStorage_imgBlockWrite(uint32_t blockNum, uint8_t *pBlockData, uint16_t len);

/*********************************************************************
 * @fn      OADStorage_imgDataWrite
 *
 * @brief   Write image data.
 *
 * @param   addrOffset  - starting address offset to write block data
 * @param   pBlockData  - pointer to data to be written
 * @param   blockOffset - offset in the block data pointer to start writing data
 * @param   len         - length of the block
 *
 * @return  status
 */
extern OADStorage_Status_t OADStorage_imgDataWrite(uint32_t addrOffset, uint8_t *pBlockData, uint32_t blockOffset, uint32_t len);

/*********************************************************************
 * @fn      OADStorage_eraseImgPage
 *
 * @brief   Erases an Image page. Note this is only needed if an image
 *          page has been corrupted typically OADStorage_imgBlockWrite
 *          pre-erase all pages
 *
 * @param  none
 *
 * @return  OADStorage_Status_t
 */
extern OADStorage_Status_t OADStorage_eraseImgPage(uint32_t page);

/*********************************************************************
 * @fn      OADStorage_imgFinalise
 *
 * @brief   Process the Image Block Write.
 *
 * @param  none
 *
 * @return  status
 */
extern OADStorage_Status_t OADStorage_imgFinalise(void);

/*********************************************************************
 * @fn      OADStorage_createFactoryImageBackup
 *
 * @brief   This function creates factory image backup of current running image
 *
 * @param   None
 *
 * @return  status  OADStorage_Status_Success/OADStorage_FlashError
 *
 */
extern uint8_t OADStorage_createFactoryImageBackup(void);
/*********************************************************************
 * @fn      OADStorage_checkFactoryImage
 *
 * @brief   This function check if the valid factory image exists on external
 *          flash
 *
 * @param   None
 *
 * @return  TRUE If factory image exists on external flash, else FALSE
 *
 */
extern bool OADStorage_checkFactoryImage(void);

/*********************************************************************
 * @fn      OADStorage_close
 *
 * @brief   Releases the resource required for OAD stoarage.
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
