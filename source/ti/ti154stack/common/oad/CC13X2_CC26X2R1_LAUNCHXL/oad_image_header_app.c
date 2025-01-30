/******************************************************************************

 @file  oad_image_header_app.c

 @brief OAD image header definition file.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2014-2025, Texas Instruments Incorporated
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
 * INCLUDES
 */

#include <stddef.h>
#include <oad_image_header_app.h>

/*******************************************************************************
 * EXTERNS
 */

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */
#ifdef FREERTOS_SUPPORT
#define VECTOR_TB_SIZE       0xd8 //!< Interrupt vector table entry size */
#else
#define VECTOR_TB_SIZE       0x40 //!< Interrupt vector table entry size */
#endif
#ifndef STACK_LIBRARY
  #define BOUNDARY_SEG_LEN   0x18 //!< Length of the boundary segment */
#endif

#define SOFTWARE_VER            {'0', '0', '0', '1'}

#if defined(DeviceFamily_CC26X2) || defined(DeviceFamily_CC26X2X7) || \
    defined(DeviceFamily_CC26X1) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3)
#define    OAD_WIRELESS_TECH_TIMAC  OAD_WIRELESS_TECH_TIMAC_2_4G     //!< Wireless protocol type BLE/TI-MAC/ZIGBEE etc. */
#elif defined(DeviceFamily_CC13X2) || defined(DeviceFamily_CC13X2X7) || defined(DeviceFamily_CC13X1) || defined(DeviceFamily_CC13X4)
#define    OAD_WIRELESS_TECH_TIMAC  OAD_WIRELESS_TECH_TIMAC_SUBG     //!< Wireless protocol type BLE/TI-MAC/ZIGBEE etc. */
#endif

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */
/*
 * NV Page Setting:
 * This define is used ensure the stack is built with a compatible NV setting
 * Note: this restriction does not apply to the stack library configuration
 * for off-chip OAD
 */

extern const uint32_t  RAM_END;

#if defined HAL_IMAGE_A
extern const uint8_t  ENTRY_END;
extern const uint8_t  ENTRY_START;
#endif

#if defined MCUBOOT
//grab header from beginning of image
const mcuboot_image_header_t* _mcuBootHdr = (mcuboot_image_header_t *) &_PRIMARY_SLOT_BASE;
#endif

#ifdef __IAR_SYSTEMS_ICC__
#pragma section = "ROSTK"
#pragma section = "RWDATA"
#pragma section = "ENTRY_FLASH"
#pragma section = ".intvec"
#endif


#if defined(__TI_COMPILER_VERSION__) || defined(__clang__)
/* This symbol is created by the linker file */
extern uint8_t prgEntryAddr;
extern uint8_t flashEndAddr;
#endif /* __TI_COMPILER_VERSION__ || __clang__ */

#ifdef __TI_COMPILER_VERSION__
#pragma DATA_SECTION(_imgHdr, ".image_header")
#pragma RETAIN(_imgHdr)
const imgHdr_t _imgHdr =
#elif  defined(__IAR_SYSTEMS_ICC__)
#pragma location=".img_hdr"
const imgHdr_t _imgHdr @ ".img_hdr" =
#elif defined(__clang__)
const imgHdr_t _imgHdr __attribute__((section( ".image_header"))) __attribute__((used)) =
#endif
{
  {
    .imgID = OAD_IMG_ID_VAL,
    .crc32 = DEFAULT_CRC,
    .bimVer = BIM_VER,
    .metaVer = META_VER,                   //!< Metadata version */
    .techType = OAD_WIRELESS_TECH_TIMAC,    //!< Wireless protocol type BLE/TI-MAC/ZIGBEE etc. */
    .imgCpStat = DEFAULT_STATE,            //!< Image copy status bytes */
    .crcStat = DEFAULT_STATE,              //!< CRC status */
    .imgNo = 0x1,                          //!< Image number of 'image type' */
    .imgVld = 0xFFFFFFFF,                  //!< In indicates if the current image in valid 0xff - valid, 0x00 invalid image */
#ifdef MCUBOOT
    .len = (uint32_t)&_SLOT_SIZE,          //!< Image length in bytes. */
#else
    .len = INVALID_LEN,                    //!< Image length in bytes. */
#endif
    .softVer = SOFTWARE_VER,               //!< Software version of the image */
    .hdrLen = offsetof(imgHdr_t, fixedHdr.rfu) + sizeof(((imgHdr_t){0}).fixedHdr.rfu),
                                            //!< Total length of the image header */
    .rfu = 0xFFFF,                         //!< reserved bytes */
#if defined(__TI_COMPILER_VERSION__) || defined(__clang__)
#ifdef MCUBOOT
    .prgEntry = (uint32_t)&_PRIMARY_SLOT_BASE,
    .imgEndAddr = (uint32_t)&_IMG_END_ADDR,
#else
    .prgEntry = (uint32_t)&prgEntryAddr,
    .imgEndAddr = (uint32_t)&flashEndAddr,
#endif //MCUBOOT
#elif  defined(__IAR_SYSTEMS_ICC__)
    .prgEntry   = (uint32_t)(__section_begin(".intvec")), //!< Program entry address */
    .imgEndAddr = (uint32_t)(__section_end("ROSTK")),
#endif
  #if defined HAL_IMAGE_A
    .imgType =  OAD_IMG_TYPE_PERSISTENT_APP,
  #else
    .imgType = OAD_IMG_TYPE_APPSTACKLIB,
  #endif

  },

#if (defined(SECURITY))
  {
    .segTypeSecure    = IMG_SECURITY_SEG_ID,
    .wirelessTech     = OAD_WIRELESS_TECH_TIMAC,
    .verifStat        = DEFAULT_STATE,
    .secSegLen        = 0x55,
    .secVer           = SECURITY_VER,                     /* Image payload and length */
    .secTimestamp     = 0x0,                              /* Security timestamp */
    .secSignerInfo    = 0x0,
    .eccSign.sign_r   = 0x0,
    .eccSign.sign_s   = 0x0,
  },
#endif

#if (defined(FEATURE_TOAD))
  // Delta image information segment
  {
   .segTypeImg      = IMG_DELTA_SEG_ID,
   .wirelessTech    = OAD_WIRELESS_TECH_TIMAC,
   .rfu             = DEFAULT_STATE,
   .len             = DELTA_SEG_LEN,
   .isDeltaImg      = 0x0,
   .toadMetaVer     = TOAD_META_VER,
   .toadVer         = TOAD_VER,
   .memoryCfg       = TOAD_MEMORY_CFG,
   .oldImgCrc       = DEFAULT_CRC,
   .newImgLen       = INVALID_LEN,
  },
#endif

  // Image payload segment initialization
   {
     .segTypeImg = IMG_PAYLOAD_SEG_ID,

     .wirelessTech = OAD_WIRELESS_TECH_TIMAC,

     .rfu = DEFAULT_STATE,
#ifdef MCUBOOT
     .startAddr = (uint32_t)&_PRIMARY_SLOT_BASE,
#else
     .startAddr = (uint32_t)&(_imgHdr.fixedHdr.imgID),
#endif //MCUBOOT
   }
 };
