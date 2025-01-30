/******************************************************************************

 @file  bim_dual_onchip_main.c

 @brief This module contains the definitions for the main functionality of a
        Boot Image Manager for Dual on chip OAD.

 Group: CMCU
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

/*******************************************************************************
 *                                          Includes
 */
#include <stdint.h>
#include <string.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/flash.h)
#include DeviceFamily_constructPath(driverlib/watchdog.h)
#include DeviceFamily_constructPath(inc/hw_prcm.h)

#include "ti/common/cc26xx/crc/crc32.h"
#include "ti/common/cc26xx/flash_interface/flash_interface.h"
#include "ti/common/cc26xx/bim/bim_util.h"
#include "ti/common/cc26xx/oad/oad_image_header.h"

#ifdef __IAR_SYSTEMS_ICC__
#include <intrinsics.h>
#endif

#if defined(DEBUG_BIM) || defined(BIM_BLINK_LED_NO_VALID_IMAGE)
#include DeviceFamily_constructPath(driverlib/gpio.h)
#include "ti/common/flash/no_rtos/extFlash/bsp.h"
#include "ti/common/cc26xx/debug/led_debug.h"
#endif

#include "ti/common/cc26xx/ecc/sign_util.h"
#if defined(DeviceFamily_CC26X2) || defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC13X2X7) || defined(DeviceFamily_CC26X2X7)
#include "ti/common/cc26xx/sha2/sha2_driverlib.h"
#else
#include DeviceFamily_constructPath(driverlib/rom_sha256.h)
#endif /* DeviceFamily_CC26X2 || DeviceFamily_CC13X2 || DeviceFamily_CC13X2X7 || DeviceFamily_CC26X2X7 */

#if defined(DEBUG_BIM)
    // DUAL Image BIM is considered a strictly production ready variant. DEBUG_BIM cannot be used as
    // it would skip key security steps.
    #warning "WARNING: DUAL Image BIM is considered a strictly production ready variant. DEBUG_BIM cannot be used as it would skip key security steps."
#endif
/*******************************************************************************
 *                                          Constants
 */

/* Customer to update these as per their images */
#define IMAGE_1_HDR_START_PAGE_NUM (0)
#define IMAGE_2_HDR_START_PAGE_NUM (23)
#if(IMAGE_2_HDR_START_PAGE_NUM <= IMAGE_1_HDR_START_PAGE_NUM)
    #error "Error: Ensure Image 2 Header starts at a higher flash page as compared to Image 1 Header!"
#endif

/* Customer to update these as per their images */
/* In the current example:*/
/* Flash pages 0 to 20 is slot 1 */
/* Flash pages 21 & 22 are being used as shared NV between both the images */
/* Flash pages 23 to 42 is slot 2 */
/* Flash page 43 is BIM + CCFG */
/* Total flash pages on CC26x2, CC13x2 is 44 pages */
#define IMAGE_1_START_FLASH_PAGE_NUM (IMAGE_1_HDR_START_PAGE_NUM)
#define IMAGE_1_END_FLASH_PAGE_NUM   (20)
#define IMAGE_2_START_FLASH_PAGE_NUM (IMAGE_2_HDR_START_PAGE_NUM)
#define IMAGE_2_END_FLASH_PAGE_NUM   (42)

#if(IMAGE_2_START_FLASH_PAGE_NUM <= IMAGE_1_START_FLASH_PAGE_NUM)
    #error "Error: Ensure Image 2 starts at a higher flash page as compared to Image 1!"
#endif
#if(IMAGE_1_END_FLASH_PAGE_NUM <= IMAGE_1_START_FLASH_PAGE_NUM)
    #error "Error: Incorrect image 1: end flash page number is less than start flash page number!"
#endif
#if(IMAGE_2_END_FLASH_PAGE_NUM <= IMAGE_2_START_FLASH_PAGE_NUM)
    #error "Error: Incorrect image 2: end flash page number is less than start flash page number!"
#endif
#if(IMAGE_2_END_FLASH_PAGE_NUM <= IMAGE_1_END_FLASH_PAGE_NUM)
    #error "Error: Ensure Image 2 ends at a higher flash page as compared to Image 1!"
#endif
#if(IMAGE_2_END_FLASH_PAGE_NUM > 42)
    #error "Error: CC13x2/26x2 devices have only 44 flash pages & 43th flash page is reserved for BIM + CCFG !"
#endif

#define IMG_TYPE_NO_IMAGE_PRESENT           (uint8_t)(0x00)
#define IMG_TYPE_IMAGE_PRESENT              (uint8_t)(0x01)
#define IMG_TYPE_INVALID_APPSTACKLIB_IMG    (uint8_t)(0x00)
#define IMG_TYPE_VALID_APPSTACKLIB_IMG      (uint8_t)(0x02)

#define IMG_PRESENT_MASK  (uint8_t)(0x01)
#define IMG_VALIDITY_MASK (uint8_t)(0x02)

#define BIM_ONCHIP_MAX_NUM_SEARCHES (2)

#define SHA_BUF_SZ                      EFL_PAGE_SIZE

#define SUCCESS                         0
#define FAIL                           -1

/*******************************************************************************
 * LOCAL VARIABLES
 */

#ifndef DEBUG_BIM
static uint32_t intFlashPageSize;       /* Size of internal flash page */
#endif

uint8_t img1_status = IMG_TYPE_NO_IMAGE_PRESENT;
uint8_t img2_status = IMG_TYPE_NO_IMAGE_PRESENT;

#if defined(__IAR_SYSTEMS_ICC__)
__no_init uint8_t shaBuf[SHA_BUF_SZ];
#elif defined(__TI_COMPILER_VERSION__) || defined(__clang__)
uint8_t shaBuf[SHA_BUF_SZ];
#endif

/* Cert element stored in flash where public keys in Little Endian format*/
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION(_secureCertElement, ".cert_element")
#pragma RETAIN(_secureCertElement)
const certElement_t _secureCertElement =
#elif __clang__
const certElement_t _secureCertElement __attribute__((section(".cert_element"))) =
#elif  defined(__IAR_SYSTEMS_ICC__)
#pragma location=".cert_element"
const certElement_t _secureCertElement @ ".cert_element" =
#endif
{
    .version    = SECURE_SIGN_TYPE,
    .len        = SECURE_CERT_LENGTH,
    .options    = SECURE_CERT_OPTIONS,
    .signerInfo = {0xb0,0x17,0x7d,0x51,0x1d,0xec,0x10,0x8b},
    .certPayload.eccKey.pubKeyX = {0xd8,0x51,0xbc,0xa2,0xed,0x3d,0x9e,0x19,0xb7,0x33,0xa5,0x2f,0x33,0xda,0x05,0x40,
                                    0x4d,0x13,0x76,0x50,0x3d,0x88,0xdf,0x5c,0xd0,0xe2,0xf2,0x58,0x30,0x53,0xc4,0x2a},
    .certPayload.eccKey.pubKeyY = {0xb9,0x2a,0xbe,0xef,0x66,0x5f,0xec,0xcf,0x56,0x16,0xcc,0x36,0xef,0x2d,0xc9,0x5e,
                                    0x46,0x2b,0x7c,0x3b,0x09,0xc1,0x99,0x56,0xd9,0xaf,0x95,0x81,0x63,0x23,0x7b,0xe7}
 };

uint32_t eccWorkzone[SECURE_FW_ECC_NIST_P256_WORKZONE_LEN_IN_BYTES + SECURE_FW_ECC_BUF_TOTAL_LEN(SECURE_FW_ECC_NIST_P256_KEY_LEN_IN_BYTES)*5] = {0};
uint8_t headerBuf[HDR_LEN_WITH_SECURITY_INFO];

/*******************************************************************************
 * EXTERN FUNCTIONS
 */


/*******************************************************************************
 * LOCAL FUNCTIONS
 */
static void Bim_findImage(uint8_t flashPageNum, uint8_t imgType);

inline static void BIM_updateImgStatus(uint8_t flashPageNum, uint8_t status);
static void Bim_UpdateExecValidImg();
static void Bim_BootIntoImg(imgHdr_t new_image, imgHdr_t old_image, uint8_t newImgFlashPgNum);
#ifdef BIM_ERASE_INVALID_IMAGE
static uint8_t Bim_EraseOnchipFlashPages(uint8_t startPage, uint8_t endPage);
#else
static void Bim_InvalidateFlashPageImage(uint8_t flashPageNum);
inline static void Bim_updateVerifStatus(uint8_t flashPageNum, uint8_t secStatus);
#endif //BIM_ERASE_INVALID_IMAGE
static bool    Bim_checkForSecSegmnt(uint32_t iflStartAddr, uint32_t imgLen);

#ifndef DEBUG_BIM
static uint8_t Bim_verifyImage(uint32_t iflStartAddr);
#endif

// Function which is used to verify the authenticity of the OAD commands
int8_t Bim_payloadVerify(uint8_t ver, uint32_t cntr, uint32_t payloadlen,
                          uint8_t  *dataPayload, uint8_t *signPayload,
                          ecdsaSigVerifyBuf_t *ecdsaSigVerifyBuf);

// Creating a section for the function pointer, so that it can be easily accessed by OAD application(s)
#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION(_fnPtr, ".fnPtr")
#pragma RETAIN(_fnPtr)
const uint32_t _fnPtr = (uint32_t)&Bim_payloadVerify;
#elif  defined(__IAR_SYSTEMS_ICC__)
#pragma location=".fnPtr"
const uint32_t _fnPtr @ ".fnPtr" = (uint32_t)&Bim_payloadVerify;
#endif // #ifdef __TI_COMPILER_VERSION__

/*******************************************************************************
 * @fn         Bim_checkForSecSegmnt
*
*  @brief      Check for Security Segment. Reads through the headers in the .bin
*              file. If a security header is found the function checks to see if
*              the header has a populated segment.
*
*  @param       iFlStrAddr - The start address in internal flash of the binary image
*  @param       imgLen - Length of the image over which presence of security segment
*               is searched
*  @return      0  - valid security segment not found
*  @return      1  - valid security segment found
*
*/
static bool Bim_checkForSecSegmnt(uint32_t iFlStrAddr, uint32_t imgLen)
{
    bool securityFound = false;
    uint8_t endOfSegment = 0;
    uint8_t segmentType = DEFAULT_STATE;
    uint32_t segmentLength = 0;
    uint32_t searchAddr =  iFlStrAddr+OAD_IMG_HDR_LEN;

    while(!endOfSegment)
    {
        readFlash(searchAddr, &segmentType, 1);

        if(segmentType == IMG_SECURITY_SEG_ID)
        {
            /* In this version of BIM, the security header will ALWAYS be present
               But the payload will sometimes not be there. If this finds the
               header, the payload is also checked for existence. */
            searchAddr += SIG_OFFSET;
            uint32_t sigVal = 0;
            readFlash(searchAddr, (uint8_t *)&sigVal, sizeof(uint32_t));

            if(sigVal != 0) //Indicates the presence of a signature
            {
                endOfSegment = 1;
                securityFound = true;
            }
            else
            {
                break;
            }
        }
        else
        {
            searchAddr += SEG_LEN_OFFSET;
            if((searchAddr + sizeof(uint32_t)) > (iFlStrAddr + imgLen))
            {
                break;
            }
            //extFlashRead(searchAddr, sizeof(uint32_t), (uint8_t *)&segmentLength);
            readFlash(searchAddr, (uint8_t *)&segmentLength, sizeof(uint32_t));

            searchAddr += (segmentLength - SEG_LEN_OFFSET);
            if((searchAddr) > (iFlStrAddr + imgLen))
            {
                break;
            }
        }
    }

    return securityFound;
}//end of function

#ifndef DEBUG_BIM
/*******************************************************************************
 * @fn      Bim_verifyImage
 *
 * @brief   Verifies the image stored on internal flash using ECDSA-SHA256
 *
 * @param   iflStartAddr - internal flash address of the image to be verified.
 *
 * @return  Zero when successful. Non-zero, otherwise..
 */
static uint8_t Bim_verifyImage(uint32_t iflStartAddr)
{
    uint8_t verifyStatus = (uint8_t)FAIL;

    /* clear the ECC work zone Buffer */
    uint32_t *eccPayloadWorkzone = eccWorkzone;
    memset(eccPayloadWorkzone, 0, sizeof(eccWorkzone));

    /* Read in the header to get the image signature */
    readFlash(iflStartAddr, headerBuf, HDR_LEN_WITH_SECURITY_INFO);

    /* First verify signerInfo */
    verifyStatus = verifyCertElement(&headerBuf[SEG_SIGERINFO_OFFSET]);
    if(verifyStatus != SUCCESS)
    {
        return verifyStatus;
    }

    // Get the hash of the image
    uint8_t *finalHash;

    finalHash = computeSha2Hash(iflStartAddr, shaBuf, SHA_BUF_SZ, false);

    if(NULL == finalHash)
    {
        verifyStatus = (uint8_t)FAIL;
        return verifyStatus;
    }

    // Verify the hash
    // Create temp buffer used for ECDSA sign verify, it should 6*ECDSA_KEY_LEN
    uint8_t tempWorkzone[ECDSA_SHA_TEMPWORKZONE_LEN];
    memset(tempWorkzone, 0, ECDSA_SHA_TEMPWORKZONE_LEN);

    verifyStatus = bimVerifyImage_ecc(_secureCertElement.certPayload.eccKey.pubKeyX,
                                      _secureCertElement.certPayload.eccKey.pubKeyY,
                                       finalHash,
                                       &headerBuf[SEG_SIGNR_OFFSET],
                                       &headerBuf[SEG_SIGNS_OFFSET],
                                       eccWorkzone,
                                       tempWorkzone);

    if(verifyStatus == SECURE_FW_ECC_STATUS_VALID_SIGNATURE)
    {
        verifyStatus = SUCCESS;
    }
    else
    {
        verifyStatus = (uint8_t)FAIL;
    }
    return verifyStatus;

}//end of function

#endif // DEBUG_BIM

/*******************************************************************************
 * @fn      Bim_payloadVerify
 *
 * @brief   Function in BIM to verify the payload of an OTA command.
 *
 * @param   ver - version of the security algorithm
 *          cntr - time-stamp /counter value use to verify the payload
 *          payloadlen - payload length in bytes
 *          dataPayload - pointer to data payload to be verified
 *          signPayload - pointer to sign payload
 *          eccPayloadWorkzone - pointer to the workzone used to verify the command
 *          shaPayloadWorkzone - pointer to the workzone used to generate a hash of the command
 *
 *
 * @return  Zero when successful. Non-zero, otherwise..
 */
int8_t Bim_payloadVerify(uint8_t ver, uint32_t cntr, uint32_t payloadlen,
                         uint8_t  *dataPayload, uint8_t *signPayload,
                         ecdsaSigVerifyBuf_t *ecdsaSigVerifyBuf)
{
    signPld_ECDSA_P256_t *signPld = (signPld_ECDSA_P256_t*)signPayload;

    uint8_t *sig1 = signPld->signature;
    uint8_t *sig2 = &signPld->signature[32];
    int8_t status = FAIL;
    int8_t verifyStatus = FAIL;
    uint8_t *finalHash = ecdsaSigVerifyBuf->tempWorkzone;

    memset(ecdsaSigVerifyBuf->tempWorkzone, 0, sizeof(ECDSA_SHA_TEMPWORKZONE_LEN));

    if (ver == 1)
    {
#if defined(DeviceFamily_CC26X2) || defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC13X2X7) || defined(DeviceFamily_CC26X2X7)
        SHA2_open();
        SHA2_addData(dataPayload, payloadlen);
        SHA2_finalize(finalHash);
        SHA2_close();
#else
        SHA256_Workzone sha256_workzone;
        SHA256_init(&sha256_workzone);
        SHA256_full(&sha256_workzone, finalHash, dataPayload, payloadlen);
#endif /* DeviceFamily_CC26X2 || DeviceFamily_CC13X2 || DeviceFamily_CC13X2X7 || DeviceFamily_CC26X2X7 */

        // First verify signerInfo
        verifyStatus = verifyCertElement(signPld->signerInfo);
        if(verifyStatus != SUCCESS)
        {
            return FAIL;
        }

        verifyStatus = bimVerifyImage_ecc(_secureCertElement.certPayload.eccKey.pubKeyX,
                                          _secureCertElement.certPayload.eccKey.pubKeyY,
                                           finalHash, sig1, sig2,
                                           ecdsaSigVerifyBuf->eccWorkzone,
                                           (ecdsaSigVerifyBuf->tempWorkzone + ECDSA_KEY_LEN));
        if(verifyStatus == (int8_t)SECURE_FW_ECC_STATUS_VALID_SIGNATURE)
        {
            status = SUCCESS;
        }
    }
    return status;
}//end of function


/*******************************************************************************
 * @fn     Bim_findImage
 *
 * @brief  Performs various checks on the image, if present at
 *         flash page number. The image status variables are
 *         updated based on checks performed.
 *
 * @param  flashPageNum - flash page number
 *         imgType      - image type to be searched
 *
 * @return None.
 */
static void Bim_findImage(uint8_t flashPageNum, uint8_t imgType)
{
    imgHdr_t imgHdr;
    uint8_t imgStatus;

    /* Read flash to find OAD image identification value */
    readFlashPg(flashPageNum, 0, &imgHdr.fixedHdr.imgID[0], OAD_IMG_ID_LEN);

    /* Check imageID bytes */
    if ((imgIDCheck(&(imgHdr.fixedHdr)) != true))
    {
        imgStatus = IMG_TYPE_NO_IMAGE_PRESENT;
        BIM_updateImgStatus(flashPageNum, imgStatus);
        /* return so that same process can be repeated*/
        return;
    }
    else //valid OAD image ID value is found
    {
        /* Read whole of fixed header in the image header */
        readFlashPg(flashPageNum, 0, (uint8_t *)&imgHdr, OAD_IMG_HDR_LEN);

#ifdef BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE
        /* if commit flag is COMMIT_REJECTED: invalidate the image immediately */
        if(imgType != imgHdr.fixedHdr.imgType || (evenBitCount(imgHdr.fixedHdr.imgVld) == false) || COMMIT_REJECTED == imgHdr.fixedHdr.commitFlag)
#else
        if(imgType != imgHdr.fixedHdr.imgType || (evenBitCount(imgHdr.fixedHdr.imgVld) == false))
#endif
        {

            imgStatus = (IMG_TYPE_IMAGE_PRESENT | IMG_TYPE_INVALID_APPSTACKLIB_IMG);
            BIM_updateImgStatus(flashPageNum, imgStatus);
            /* didn't find the image type we are looking for */
            /* Or the image we found is considered 'invalid' */
            /* return so that same process can be repeated */
            return;
        }

        /* Image type matched: proceed with further checks */

        /* check BIM and Metadata version and CRC status (if need be). If DEBUG_BIM is enabled
         * skip the crc checking and updating the crc status- as crc wouldn't have been
         * calculated at the first place */

        if( (imgHdr.fixedHdr.bimVer != BIM_VER  || imgHdr.fixedHdr.metaVer != META_VER) /* Invalid metadata version */
#ifndef DEBUG_BIM
              ||
           (imgHdr.fixedHdr.crcStat == 0xFC)  /* Invalid CRC */
#endif
          )
        {
            imgStatus = (IMG_TYPE_IMAGE_PRESENT | IMG_TYPE_INVALID_APPSTACKLIB_IMG);
            BIM_updateImgStatus(flashPageNum, imgStatus);
            /* return so that same process can be repeated*/
            return;
        }
        else if(imgHdr.fixedHdr.crcStat == 0xFF) /* CRC not calculated yet */
        {
#ifndef DEBUG_BIM

            /* Calculate the CRC over the data buffer and update status */
            uint32_t crc32 = 0;
            uint8_t  crcstat = CRC_VALID;
            crc32 = CRC32_calc(flashPageNum, intFlashPageSize, 0, imgHdr.fixedHdr.len, false);

            /* Check if calculated CRC matched with the image header */
            if (crc32 != imgHdr.fixedHdr.crc32)
            {
                /* Update CRC status */
                crcstat = CRC_INVALID;
                writeFlashPg(flashPageNum, CRC_STAT_OFFSET, (uint8_t *)&crcstat, 1);

                imgStatus = (IMG_TYPE_IMAGE_PRESENT | IMG_TYPE_INVALID_APPSTACKLIB_IMG);
                BIM_updateImgStatus(flashPageNum, imgStatus);
                /* return so that same process can be repeated */
                return;

            } /* if (crc32 != imgHdr.crc32) */

            /* if we have come here: CRC check has passed */
            /* Update CRC status */
            writeFlashPg(flashPageNum, CRC_STAT_OFFSET, (uint8_t *)&crcstat, 1);
#endif
        } /* else if(imgHdr.crcStat == 0xFF) */

        /* populate the start address of the image in the internal flash */
        uint32_t iFlStrAddr = FLASH_ADDRESS(flashPageNum, 0);

        /* verify that start address + length of image is less than the slot boundary */
        if (((iFlStrAddr == IMAGE_1_START_FLASH_PAGE_NUM * INTFLASH_PAGE_SIZE) && /* if slot 1 image */
             ((iFlStrAddr + imgHdr.fixedHdr.len) > IMAGE_2_START_FLASH_PAGE_NUM * INTFLASH_PAGE_SIZE)) ||
            ((iFlStrAddr != IMAGE_1_START_FLASH_PAGE_NUM * INTFLASH_PAGE_SIZE) && /* if slot 2 image */
             ((iFlStrAddr + imgHdr.fixedHdr.len) > MAX_ONCHIP_FLASH_PAGES * INTFLASH_PAGE_SIZE)))
        {
            imgStatus = (IMG_TYPE_IMAGE_PRESENT | IMG_TYPE_INVALID_APPSTACKLIB_IMG);
            BIM_updateImgStatus(flashPageNum, imgStatus);
            /* return so that same process can be repeated */
            return;
        }

        /* Begin security checks */
        int8_t signVrfyStatus = FAIL;

        uint8_t securityPresence = false;

        /* check if security segment is present or not in the image */
        securityPresence = Bim_checkForSecSegmnt(iFlStrAddr, imgHdr.fixedHdr.len);

        if(securityPresence)
        {
            /* Calculate the SHA256 of the image */
            uint8_t readSecurityByte;

            /* Read in the header to check if the signature has already been denied */
            readFlashPg(flashPageNum, SEC_VERIF_STAT_OFFSET, &readSecurityByte, 1);

            if(readSecurityByte != VERIFY_FAIL)
            {
#ifndef DEBUG_BIM
                signVrfyStatus = Bim_verifyImage(iFlStrAddr);
                /* If the signature is invalid, update the sign verification status */
                if((uint8_t)signVrfyStatus != SUCCESS)
                {
                    readSecurityByte = VERIFY_FAIL;
                    writeFlashPg(flashPageNum, SEC_VERIF_STAT_OFFSET, &readSecurityByte, 1);
                }
#else
                signVrfyStatus = SUCCESS;
#endif
            }
        } /* if(securityPresence) */

        /*
         * sign verification has failed or
         * didn't find the security segment in the first place or
         * iFlStrAddr is outside of the authenticated flash space
         */
        if((uint8_t)signVrfyStatus != SUCCESS ||
                (imgHdr.fixedHdr.prgEntry < iFlStrAddr) ||
                (imgHdr.fixedHdr.prgEntry > (iFlStrAddr + imgHdr.fixedHdr.len)))
        {
            imgStatus = (IMG_TYPE_IMAGE_PRESENT | IMG_TYPE_INVALID_APPSTACKLIB_IMG);
            BIM_updateImgStatus(flashPageNum, imgStatus);
            /* return so that same process can be repeated */
            return;
        }

        /*if we get here, its highly likely we found a valid image */
        imgStatus = (IMG_TYPE_IMAGE_PRESENT | IMG_TYPE_VALID_APPSTACKLIB_IMG);
        BIM_updateImgStatus(flashPageNum, imgStatus);
    }//valid imageID found

    return;
}

/*******************************************************************************
 * @fn     BIM_updateImgStatus
 *
 * @brief  Updates the imgX_status variable based on if
 *         the image being checked is in slot 1 or slot 2.
 *         These global variables will be later consumed in
 *         Bim_UpdateExecValidImg() function.
 *
 * @param  flashPageNum - flash page number
 *
 * @return None.
 */
inline static void BIM_updateImgStatus(uint8_t flashPageNum, uint8_t status)
{
    if( IMAGE_1_HDR_START_PAGE_NUM == flashPageNum ) /* 1st slot */
    {
        img1_status = status;
    }
    else /* 2nd slot */
    {
        img2_status = status;
    }
}

/*******************************************************************************
 * @fn     Bim_updateVerifStatus
 *
 * @brief  Based on the img_status fields populated by Bim_findImage
 *         & anti-roll back check, decide which image to boot to &
 *         update the verifStatus fields in the security header
 *         of the images appropriately.
 *         Also, Jump into the decided upon image.
 *
 * @param  flashPageNum - flash page where the image header starts.
 * @param  secStatus - value to be written into verifStatus field.
 *
 * @return None.
 */
inline static void Bim_updateVerifStatus(uint8_t flashPageNum, uint8_t secStatus)
{
    writeFlashPg(flashPageNum, SEC_VERIF_STAT_OFFSET,  (uint8_t *)&secStatus, 1);
}

#ifdef BIM_ERASE_INVALID_IMAGE
/*******************************************************************************
 * @fn     Bim_EraseOnchipFlashPages
 *
 * @brief  It Erases the onchip flash pages.
 *
 * @param  startPage - Starting page on on-chip flash to be erased
 * @param  endPage  - End page on on-chip flash to be erased
 * @param  pageSize  - flash page size
 *
 * @return - SUCCESS on successful erasure else
 *           FAIL
 */
static uint8_t Bim_EraseOnchipFlashPages(uint8_t startPage, uint8_t endPage)
{
    int8_t status = SUCCESS;

    // Erase the correct amount of pages
    for(uint8_t page = startPage; page <= endPage; ++page)
    {
        uint8_t flashStat = eraseFlashPg(page);
        if(flashStat == FLASH_FAILURE)
        {
            // If we fail to pre-erase, then halt the process
            status = FAIL;
            break;
        }
    }
    return status;
}
#endif // BIM_ERASE_INVALID_IMAGE

/*******************************************************************************
 * @fn     Bim_InvalidateFlashPageImage
 *
 * @brief  Given BIM_ERASE_INVALID_IMAGE feature, we either update its
 *         status to VERIFY_FAIL or completely erase it from flash.
 *
 * @param  flashPageNum - flash page where the invalid image header starts.
 *
 * @return None.
 */
static void Bim_InvalidateFlashPageImage(uint8_t flashPageNum)
{
#ifdef BIM_ERASE_INVALID_IMAGE
    uint8_t flashPageEnd = IMAGE_1_END_FLASH_PAGE_NUM;
    if( IMAGE_2_HDR_START_PAGE_NUM == flashPageNum )
    {
        flashPageEnd = IMAGE_2_END_FLASH_PAGE_NUM;
    }
    Bim_EraseOnchipFlashPages(flashPageNum, flashPageEnd);
#else
    Bim_updateVerifStatus(flashPageNum, VERIFY_FAIL);
#endif
}

/*******************************************************************************
 * @fn     Bim_BootIntoImg
 *
 * @brief  Given two VALID images, Bim_BootIntoImg() decides which to boot into based on:
 *         1. security version, baseline BIM always checks this
 *         2. Image version, if BIM_VERIFY_VERSION_IMAGE enabled
 *         Also, restricted rollback, if BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE enabled
 *
 * @param  new_image        - new image's imgHdr_t
 * @param  old_image        - old image's imgHdr_t
 * @param  newImgFlashPgNum - the new image's flash page number
 *
 * @return None.
 */
static void Bim_BootIntoImg(imgHdr_t new_image, imgHdr_t old_image, uint8_t newImgFlashPgNum)
{
    uint8_t secVer_new = 0, secVer_old = 0;
#ifdef BIM_VERIFY_VERSION_IMAGE
    uint32_t softVer_new = 0, softVer_old = 0;
#endif
    uint8_t oldImgFlashPgNum = IMAGE_2_HDR_START_PAGE_NUM;

    secVer_new = new_image.secInfoSeg.secVer;
    secVer_old = old_image.secInfoSeg.secVer;

#ifdef BIM_VERIFY_VERSION_IMAGE
    softVer_new = new_image.fixedHdr.softVer;
    softVer_old = old_image.fixedHdr.softVer;
#endif

#ifdef BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE
    uint8_t cmtFlg_new = new_image.fixedHdr.commitFlag;
    uint8_t cmtFlg_old = old_image.fixedHdr.commitFlag;
#endif

    if( IMAGE_2_HDR_START_PAGE_NUM == newImgFlashPgNum )
    {
        oldImgFlashPgNum = IMAGE_1_HDR_START_PAGE_NUM;
    }

    if(secVer_new > secVer_old)
    {
#ifdef BIM_VERIFY_VERSION_IMAGE
        if(softVer_new <= softVer_old) {
            /* new image has a lower or similar software version while having a higher security version */
            /* reject new image and boot into existing one */
            /* VERIFY_FAIL or erase new image and update the old image to VERIFY_PASS_CURRENT */
            Bim_InvalidateFlashPageImage(newImgFlashPgNum);
            Bim_updateVerifStatus(oldImgFlashPgNum, VERIFY_PASS_CURRENT);
            /* Do not switch to new image */
            jumpToPrgEntry((uint32_t*)old_image.fixedHdr.prgEntry);
        }
#endif

#ifdef BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE
        /* when BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE feature is enabled, we apply slightly different rules to the old image */
        /* if commit flag is COMMIT_REJECTED: this image would be invalidated early on in the code at the beginning of */
        /* Bim_findImage() and we would not get here*/
        /* if commit flag is COMMIT_ACCEPTED or COMMIT_PENDING: we update the status of the old image to VERIFY_PASS_NOT_CURRENT */
        /* Effectively marking both images as passing to give the app a "grace period" in which it can either commit or reject itself */
        /* In the case of a rejection, we will revert back this image as it is marked as not current and can be booted into */
        Bim_updateVerifStatus(oldImgFlashPgNum, VERIFY_PASS_NOT_CURRENT);
#else
        Bim_InvalidateFlashPageImage(oldImgFlashPgNum);
#endif
        Bim_updateVerifStatus(newImgFlashPgNum, VERIFY_PASS_CURRENT);
        jumpToPrgEntry((uint32_t*)new_image.fixedHdr.prgEntry);
    } // secVer_new > secVer_old
    else if( secVer_new < secVer_old)
    {
        /* reject new image and boot into existing one */
        /* VERIFY_FAIL or erase new image and update the old image to VERIFY_PASS_CURRENT */
        Bim_InvalidateFlashPageImage(newImgFlashPgNum);
        Bim_updateVerifStatus(oldImgFlashPgNum, VERIFY_PASS_CURRENT);
        /* Do not switch to new image */
        jumpToPrgEntry((uint32_t*)old_image.fixedHdr.prgEntry);
    } // secVer_new < secVer_old
    else /* same sec version */
    {
#ifdef BIM_VERIFY_VERSION_IMAGE
        if(softVer_new <= softVer_old) {
            /* new image has a lower software verion while having a similar security version in which case it is invalid or */
            /* new image has the same software and security versions in which case it is the same image */
            /* either case reject new image and boot into existing image*/
            /* VERIFY_FAIL or erase new image and update the old image to VERIFY_PASS_CURRENT */
            Bim_InvalidateFlashPageImage(newImgFlashPgNum);
            /* Do not switch to new image */
            Bim_updateVerifStatus(oldImgFlashPgNum, VERIFY_PASS_CURRENT);
            jumpToPrgEntry((uint32_t*)old_image.fixedHdr.prgEntry);
        }
#ifndef BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE
        else {
            Bim_InvalidateFlashPageImage(oldImgFlashPgNum);
            Bim_updateVerifStatus(newImgFlashPgNum, VERIFY_PASS_CURRENT);
            jumpToPrgEntry((uint32_t*)new_image.fixedHdr.prgEntry);
        }
#endif
#endif // BIM_VERIFY_VERSION_IMAGE not defined
        /* when BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE feature is enabled, we apply slightly different rules to the old image */
        /* if commit flag is COMMIT_REJECTED: this image would be invalidated early on in the code at the beginning of */
        /* Bim_findImage() and we would not get here*/
        /* if commit flag is COMMIT_ACCEPTED or COMMIT_PENDING: we update the status of the old image to VERIFY_PASS_NOT_CURRENT */
        /* Effectively marking both images as passing to give the app a "grace period" in which it can either commit or reject itself */
        /* In the case of a rejection, we will revert back this image as it is marked as not current and can be booted into */
        Bim_updateVerifStatus(oldImgFlashPgNum,  VERIFY_PASS_NOT_CURRENT);
        Bim_updateVerifStatus(newImgFlashPgNum, VERIFY_PASS_CURRENT);
        jumpToPrgEntry((uint32_t*)new_image.fixedHdr.prgEntry);
    } // secVer_new == secVer_old
}

/*******************************************************************************
 * @fn     Bim_UpdateExecValidImg
 *
 * @brief  Based on the img_status fields populated by Bim_findImage
 *         & anti-roll back check, decide which image to boot to &
 *         update the verifStatus fields in the security header
 *         of the images appropriately.
 *         Also, Jump into the decided upon image.
 *
 * @param  None.
 *
 * @return None.
 */
static void Bim_UpdateExecValidImg()
{
    /* Declare local variables used in the main function */
    imgHdr_t imgHdr_1, imgHdr_2;
    uint8_t verifStat_1 = 0, verifStat_2 = 0;
    uint8_t isPresImg_1 = img1_status & IMG_PRESENT_MASK;
    uint8_t isPresImg_2 = img2_status & IMG_PRESENT_MASK;
    uint8_t isValidImg_1 = img1_status & IMG_VALIDITY_MASK;
    uint8_t isValidImg_2 = img2_status & IMG_VALIDITY_MASK;

    if((isValidImg_1) && !(isValidImg_2)) /* first image is valid & 2nd image is not valid */
    {
        /* mark the verifStatus fields accordingly */
        if(isPresImg_2)
        {
            Bim_InvalidateFlashPageImage(IMAGE_2_HDR_START_PAGE_NUM);
        }
        /*
         * In the case BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE feature is defined,
         * image 1 might have been marked as VERIFY_PASS_NOT_CURRENT
         * And this action will not revert it back to VERIFY_PASS_CURRENT
         */
        Bim_updateVerifStatus(IMAGE_1_HDR_START_PAGE_NUM, VERIFY_PASS_CURRENT);
        readFlashPg(IMAGE_1_HDR_START_PAGE_NUM, 0, (uint8_t *)&imgHdr_1, OAD_IMG_HDR_LEN);
        /* jump to the image */
        jumpToPrgEntry((uint32_t*)imgHdr_1.fixedHdr.prgEntry);
    }
    else if(!(isValidImg_1) && (isValidImg_2))
    {
        /* mark the verifStatus fields accordingly */
        if(isPresImg_1)
        {
            Bim_InvalidateFlashPageImage(IMAGE_1_HDR_START_PAGE_NUM);
        }
        /*
         * In the case BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE feature is defined,
         * image 1 might have been marked as VERIFY_PASS_NOT_CURRENT
         * And this action will not revert it back to VERIFY_PASS_CURRENT
         */
        Bim_updateVerifStatus(IMAGE_2_HDR_START_PAGE_NUM, VERIFY_PASS_CURRENT);
        /* jump to the image */
        readFlashPg(IMAGE_2_HDR_START_PAGE_NUM, 0, (uint8_t *)&imgHdr_2, OAD_IMG_HDR_LEN);
        jumpToPrgEntry((uint32_t*)imgHdr_2.fixedHdr.prgEntry);
    }
    else if((isValidImg_1) && (isValidImg_2)) /* both the images are valid so far */
    {
        readFlashPg(IMAGE_1_HDR_START_PAGE_NUM, 0, (uint8_t *)&imgHdr_1, (OAD_IMG_HDR_LEN + sizeof(securityInfoSeg_t)));
        readFlashPg(IMAGE_2_HDR_START_PAGE_NUM, 0, (uint8_t *)&imgHdr_2, (OAD_IMG_HDR_LEN + sizeof(securityInfoSeg_t)));

        verifStat_1 = imgHdr_1.secInfoSeg.verifStat;
        verifStat_2 = imgHdr_2.secInfoSeg.verifStat;

        /* image 1 is new */
        if((DEFAULT_STATE == verifStat_1) && (DEFAULT_STATE != verifStat_2))
        {
            Bim_BootIntoImg(imgHdr_1, imgHdr_2, IMAGE_1_HDR_START_PAGE_NUM);
        } /* end of if image 1 is new */
        /* image 2 is new */
        else if((DEFAULT_STATE != verifStat_1) && (DEFAULT_STATE == verifStat_2))
        {
            Bim_BootIntoImg(imgHdr_2, imgHdr_1, IMAGE_2_HDR_START_PAGE_NUM);
        } /* end of if image 2 is new */
        /* neither of the images are new */
        else if((DEFAULT_STATE != verifStat_1) & (DEFAULT_STATE != verifStat_2))
        {
            /* boot into image that has been marked as the current image previously */
            if((VERIFY_PASS_CURRENT == imgHdr_1.secInfoSeg.verifStat) &&
                (VERIFY_PASS_NOT_CURRENT == imgHdr_2.secInfoSeg.verifStat))
            {
#ifdef BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE
                uint8_t cmtFlg_1 = imgHdr_1.fixedHdr.commitFlag;
                if(COMMIT_ACCEPTED == cmtFlg_1) {
                    Bim_InvalidateFlashPageImage(IMAGE_2_HDR_START_PAGE_NUM);
                }
#endif
                jumpToPrgEntry((uint32_t*)imgHdr_1.fixedHdr.prgEntry);
            }
            else if((VERIFY_PASS_CURRENT == imgHdr_2.secInfoSeg.verifStat) &&
                    (VERIFY_PASS_NOT_CURRENT == imgHdr_1.secInfoSeg.verifStat))
            {
#ifdef BIM_RESTRICTED_ROLLBACK_VERIFY_COMMIT_IMAGE
                uint8_t cmtFlg_2 = imgHdr_2.fixedHdr.commitFlag;
                if(COMMIT_ACCEPTED == cmtFlg_2) {
                    Bim_InvalidateFlashPageImage(IMAGE_1_HDR_START_PAGE_NUM);
                }
#endif
                jumpToPrgEntry((uint32_t*)imgHdr_2.fixedHdr.prgEntry);
            }
            /* else: any other case: something is wrong */

        } /* if neither of the images are new */
    } /* both the images are valid */

} /* end of function Bim_UpdateExecValidImg */


/*******************************************************************************
 * @fn          main
 *
 * @brief       C-code main function.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
int main(void)
{
#ifdef __IAR_SYSTEMS_ICC__
  __set_CONTROL(0);
#endif

#ifndef DEBUG_BIM
    /* Read and populate the static variable intFlashPageSize */
    intFlashPageSize = FlashSectorSizeGet();
#endif
    uint8_t flashPgNum;

    /* Check if image is present in slot 1, if yes, do checks on it */
    flashPgNum = IMAGE_1_HDR_START_PAGE_NUM;
    Bim_findImage(flashPgNum, OAD_IMG_TYPE_APPSTACKLIB);

    /* Check if image is present in slot 2, if yes, do checks on it */
    flashPgNum = IMAGE_2_HDR_START_PAGE_NUM;
    Bim_findImage(flashPgNum, OAD_IMG_TYPE_APPSTACKLIB);

    /* once here, do checks on which image to boot to & boot into it */
    Bim_UpdateExecValidImg();

    /* If we get here, that means there is an Issue: No valid image found */
#if defined(DEBUG_BIM) || defined(BIM_BLINK_LED_NO_VALID_IMAGE)

    powerUpGpio();
    while(1)
    {
        lightRedLed();
    }

#else /* ifdef DEBUG_BIM */
    /* Set the device to the lowest power state. Does not return. */
    setLowPowerMode();

    return(0);
#endif

} /* end of main function */

/**************************************************************************************************
*/
