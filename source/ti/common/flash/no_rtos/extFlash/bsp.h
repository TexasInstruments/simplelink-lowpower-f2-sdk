/******************************************************************************

 @file  bsp.h

 @brief Board support package header file for CC2650 LaunchPad

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
#ifndef __BSP_H__
#define __BSP_H__


/******************************************************************************
* If building with a C++ compiler, make all of the definitions in this header
* have a C binding.
******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
* INCLUDES
*/
#include <stdint.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_sysctl.h)
#include DeviceFamily_constructPath(inc/hw_ioc.h)

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
#include DeviceFamily_constructPath(driverlib/ioc.h)
#endif
#include DeviceFamily_constructPath(driverlib/gpio.h)

/******************************************************************************
* DEFINES
*/

#if !defined(DeviceFamily_CC23X0R5) && !defined(DeviceFamily_CC23X0R53) && !defined(DeviceFamily_CC23X0R2) && !defined(DeviceFamily_CC23X0R22)
// Board LED defines
#define BSP_IOID_LED_1          IOID_6
#define BSP_IOID_LED_2          IOID_7

// Board key defines
#define BSP_IOID_KEY_LEFT       IOID_13
#define BSP_IOID_KEY_RIGHT      IOID_14
#endif

// Board external flash defines
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X2_CC26X2) || \
    (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X1_CC26X1)

#define BSP_IOID_FLASH_CS       IOID_20
#define BSP_SPI_MOSI            IOID_9
#define BSP_SPI_MISO            IOID_8
#define BSP_SPI_CLK_FLASH       IOID_10

#elif (DeviceFamily_PARENT == DeviceFamily_PARENT_CC13X4_CC26X3_CC26X4)

#if defined(DeviceFamily_CC26X4)

#define BSP_IOID_FLASH_CS       IOID_20
#define BSP_SPI_MOSI            IOID_9
#define BSP_SPI_MISO            IOID_8
#define BSP_SPI_CLK_FLASH       IOID_10

#else

#define BSP_IOID_FLASH_CS       IOID_38
#define BSP_SPI_MOSI            IOID_36
#define BSP_SPI_MISO            IOID_37
#define BSP_SPI_CLK_FLASH       IOID_39

#endif

#elif defined(DeviceFamily_CC23X0R5) || defined(DeviceFamily_CC23X0R53)

#define BSP_IOID_FLASH_CS       6
#define BSP_SPI_MOSI            13
#define BSP_SPI_MISO            12
#define BSP_SPI_CLK_FLASH       18

#elif defined(DeviceFamily_CC23X0R2) || defined(DeviceFamily_CC23X0R22)

#define BSP_IOID_FLASH_CS       11
#define BSP_SPI_MOSI            13
#define BSP_SPI_MISO            21
#define BSP_SPI_CLK_FLASH       24

#else

#error "Invalid Device Family"

#endif


/******************************************************************************
* Mark the end of the C bindings section for C++ compilers.
******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif /* #ifndef __BSP_H__ */
