/******************************************************************************

 @file  ti_itm.h

 @brief Describe the purpose and contents of the file.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************

 Copyright (c) 2023, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

#ifndef TI_ITM_H_
#define TI_ITM_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* ------------------------------------------------------------------------------------------------
 *                                              Includes
 * ------------------------------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

#ifdef SID_PAL_LOG_ENABLED
#define TI_LOG_ENABLED          SID_PAL_LOG_ENABLED
#define ITM_LOG_ERROR           SID_PAL_LOG_SEVERITY_ERROR
#define ITM_LOG_WARNING         SID_PAL_LOG_SEVERITY_WARNING
#define ITM_LOG_INFO            SID_PAL_LOG_SEVERITY_INFO
#define ITM_LOG_DEBUG           SID_PAL_LOG_SEVERITY_DEBUG
#define NO_OF_ITM_LOG_LEVELS    4
#ifndef SID_PAL_LOG_LEVEL
#define TI_LOG_LEVEL  SID_PAL_LOG_LEVEL
#else
#define TI_LOG_LEVEL            ITM_LOG_INFO
#endif
#else
#define TI_LOG_ENABLED          1
#define ITM_LOG_ERROR           0
#define ITM_LOG_WARNING         1
#define ITM_LOG_INFO            2
#define ITM_LOG_DEBUG           3
#define NO_OF_ITM_LOG_LEVELS    4
#ifndef SID_PAL_LOG_LEVEL
#define TI_LOG_LEVEL  SID_PAL_LOG_LEVEL
#else
#define TI_LOG_LEVEL            ITM_LOG_INFO
#endif
#endif

#if TI_LOG_ENABLED
#define TI_LOG(level, fmt_, ...)                                                        \
    do {                                                                                \
        if (level <= TI_LOG_LEVEL)  {                                                   \
            ITM_logPrint(level, NULL, fmt_, ##__VA_ARGS__);                             \
        }                                                                               \
    } while(0)

#define TI_HEXDUMP(level, pData, len)                                                   \
    do {                                                                                \
        if (level <= TI_LOG_LEVEL)  {                                                   \
            ITM_hexDump(level, NULL, pData, len);                                       \
        }                                                                               \
    } while(0)
#else
#define TI_LOG(level_, fmt_, ...)
#define TI_HEXDUMP(level, pData, len)
#endif

/* Logging helpers to simplify logging APIs */
#define TI_LOG_ERROR(fmt_, ...)                 TI_LOG(ITM_LOG_ERROR,   fmt_, ##__VA_ARGS__)
#define TI_LOG_WARNING(fmt_, ...)               TI_LOG(ITM_LOG_WARNING,   fmt_, ##__VA_ARGS__)
#define TI_LOG_INFO(fmt_, ...)                  TI_LOG(ITM_LOG_INFO,   fmt_, ##__VA_ARGS__)
#define TI_LOG_DEBUG(fmt_, ...)                 TI_LOG(ITM_LOG_DEBUG,   fmt_, ##__VA_ARGS__)

#define TI_LOG_HEXDUMP_ERROR(pData, len)        TI_HEXDUMP(ITM_LOG_ERROR, pData, len)
#define TI_LOG_HEXDUMP_WARNING(pData, len)      TI_HEXDUMP(ITM_LOG_WARNING, pData, len)
#define TI_LOG_HEXDUMP_INFO(pData, len)         TI_HEXDUMP(ITM_LOG_INFO, pData, len)
#define TI_LOG_HEXDUMP_DEBUG(pData, len)        TI_HEXDUMP(ITM_LOG_DEBUG, pData, len)

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */
typedef enum
{
    ITM_9600 = 9600,
    ITM_57600 = 57600,
    ITM_115200 = 115200,
    ITM_230400 = 230400,
    ITM_460800 = 460800,
    ITM_500000 = 500000,
    ITM_576000 = 576000,
    ITM_921600 = 921600,
    ITM_1000000 = 1000000,
    ITM_1152000 = 1152000,
    ITM_1500000 = 1500000,
    ITM_2000000 = 2000000,
    ITM_2500000 = 2500000,
    ITM_3000000 = 3000000,
    ITM_3500000 = 3500000,
    ITM_4000000 = 4000000,
    ITM_6000000 = 6000000
} ITM_baudrate;


typedef struct _ITM_config_
{
    uint_fast32_t system_clock;
    ITM_baudrate baud_rate;
    bool defer_logging;
} ITM_config;


typedef enum
{
    ITM_TS_DIV_NONE = 0,
    ITM_TS_DIV_4 = 1,
    ITM_TS_DIV_16 = 2,
    ITM_TS_DIV_64 = 3
} ITM_tsPrescale;

typedef enum
{
    ITM_SYNC_NONE = 0,
    ITM_SYNC_16M_CYCLES = 1,
    ITM_SYNC_64M_CYCLES = 2,
    ITM_SYNC_256M_CYCLES = 3
} ITM_syncPacketRate;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */
//*****************************************************************************
//
//! Print through the ITM module.
//!
//! \param level Debug level.
//! \param group Debug group.
//! \param fmt String to be sent to ITM.
//!
//! This function will print though the ITM/TPIU module for UART SWO mode.
//!
//! \return Number of bytes to print.
//
//*****************************************************************************
void ITM_logPrint(uint8_t level, const char *group, const char *fmt, ...);

//*****************************************************************************
//
//! Print through the ITM module.
//!
//! \param level Debug level.
//! \param group Debug group.
//! \param fmt String to be sent to ITM.
//! \param ap Argument list.
//!
//! This function will print though the ITM/TPIU module for UART SWO mode.
//!
//! \return Number of bytes to print.
//
//*****************************************************************************
uint16_t ITM_vprint(uint8_t level, const char *group, const char *fmt, va_list ap);

//*****************************************************************************
//
//! Hex Dump through the ITM module.
//!
//! \param level Debug level.
//! \param group Debug group.
//! \param pData Pointer of data array to be sent to ITM.
//! \param len Length of Data.
//!
//! This function will print though the ITM/TPIU module for UART SWO mode.
//!
//! \return Number of bytes to print.
//
//*****************************************************************************
uint16_t ITM_hexDump(uint8_t level, const char *group, const void *pData, uint16_t len);

//*****************************************************************************
//
//! Initialize the ITM module.
//!
//! \param itm_config provides configuration parameters.
//!
//! This function will initialize the ITM/TPIU module for UART SWO mode.
//!
//! \return None.
//
//*****************************************************************************
void ITM_initModule(const ITM_config *itm_config);

//*****************************************************************************
//
//! Enable ITM module.
//!
//! \param None.
//!
//! This function will enable ITM module for trace generation.
//!
//! \return None.
//
//*****************************************************************************
void ITM_enableModule(void);

//*****************************************************************************
//
//! Disable ITM module.
//!
//! \param None.
//!
//! This function will disable trace generation by ITM module.
//!
//! \return None.
//
//*****************************************************************************
void ITM_disableModule(void);

//*****************************************************************************
//
//! Enable sync packet generation.
//!
//! \param majorVersion Major version.
//! \param majorVersion Minor version.
//!
//! This function returns the library version.
//!
//! \return None.
//
//*****************************************************************************
void ITM_getLibraryVersion(uint32_t *majorVersion, uint32_t *minorVersion);

//*****************************************************************************
//
//! Flush ITM module.
//!
//! \param None.
//!
//! This function will wait till data is flushed from the ITM. This API must be
//! called before core enters sleep.
//!
//! \return None.
//
//*****************************************************************************
void ITM_flushModule(void);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif /* TI_ITM_H_ */
