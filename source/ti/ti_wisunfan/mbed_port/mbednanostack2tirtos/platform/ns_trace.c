/******************************************************************************

 @file  ns_trace.c

 @brief Provides ITM module functions

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2011-2025, Texas Instruments Incorporated

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

#include <ti/drivers/dpl/SystemP.h>

#include <semaphore.h>

#include "ns_trace.h"
#include "itm_private.h"

#include "mbed-client-libservice/ip6string.h"
#include "ti_radio_config.h"

#include <stdint.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include <ioc.h>

#define ITM_STIM_PORT_8(x)    (*(volatile uint8_t *) ITM_STIM_PORT((x)))
#define ITM_STIM_PORT_32(x)   (*(volatile uint32_t *) ITM_STIM_PORT((x)))

#define ITM_PORT 0

#define VT100_COLOR_ERROR "\x1b[31m"
#define VT100_COLOR_WARN  "\x1b[33m"
#define VT100_COLOR_INFO  "\x1b[39m"
#define VT100_COLOR_DEBUG "\x1b[90m"
#define VT100_COLOR_DEMO  "\x1b[39m"
#define VT100_RESET_TERM  "\x1b[0m\n\r"

#define DEFAULT_TRACE_TMP_LINE_LEN  128

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
    uint_fast32_t systemClock;
    ITM_baudrate  baudRate;
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

char tmpStr[DEFAULT_TRACE_TMP_LINE_LEN];

static sem_t ns_trace_mutex_handle;
char ns_buf[256];

void ns_trace_init(void)
{
    ITM_config itm_config =
    {
      48000000,
      ITM_115200
    };
    int retc;

    retc = sem_init(&ns_trace_mutex_handle, 0, 1);
    if (retc != 0) {
        while (1);
    }

    // Disable module
    SCS_DEMCR &= (~SCS_DEMCR_TRCEN);
    ITM_TCR   = 0x00000000;

    // Enable trace
    SCS_DEMCR |= SCS_DEMCR_TRCEN;

    // Unlock and Setup TPIU for SWO UART mode
    TPIU_LAR   = CS_LAR_UNLOCK;
    TPIU_SPPR  = TPIU_SPPR_SWO_UART;
    TPIU_CSPSR = TPIU_CSPSR_PIN_1;

    // Unlock and enable all ITM stimulus ports with default settings
    ITM_LAR    = CS_LAR_UNLOCK;
    ITM_TER    = ITM_TER_ENABLE_ALL;
    ITM_TPR    = ITM_TPR_ENABLE_USER_ALL;

    // Setup Baud rate
    if (itm_config.systemClock)
    {
        uint32_t prescalar = itm_config.systemClock / itm_config.baudRate;

        // Offset with current prescalar value
        uint32_t diff1 = itm_config.systemClock - (prescalar * itm_config.baudRate);
        // Offset with prescalar+1 value
        uint32_t diff2 = ((prescalar+1) * itm_config.baudRate) - itm_config.systemClock;

        if (diff2 < diff1)  prescalar++;
        // Program prescalar value as (prescalar factor - 1)
        TPIU_ACPR = (prescalar - 1);
    }

    // Disable formatter
    TPIU_FFCR  = 0;

    // Unlock DWT
    DWT_LAR    = CS_LAR_UNLOCK;

    // Configure SWO Traces
#ifdef  WISUN_NCP_ENABLE
    // bring trace output on UART1 TX pin
#if defined(LP_CC1312R7) || defined(LAUNCHXL_CC1312R1)
    IOCPortConfigureSet(IOID_11, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#elif defined(LP_EM_CC1314R10) || defined(LP_CC1314R10_RGZ) \
    || defined(LP_EM_CC1354P10_1) || defined(LP_CC1354P10_1_RGZ) || defined(LP_CC1354R10_RGZ) \
    || defined(LP_EM_CC1354P10_6) || defined(LP_CC1354P10_6_RGZ)
    IOCPortConfigureSet(IOID_21, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#else //1312
    IOCPortConfigureSet(IOID_16, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#endif //1312
#else //WISUN_NCP_ENABLE
#if defined(LP_CC1312R7) || defined(LAUNCHXL_CC1312R1) || defined(LP_EM_CC1314R10) || defined(LP_CC1314R10_RGZ) \
    || defined(LP_EM_CC1354P10_1) || defined(LP_CC1354P10_1_RGZ) || defined(LP_CC1354R10_RGZ) \
    || defined(LP_EM_CC1354P10_6) || defined(LP_CC1354P10_6_RGZ)
    IOCPortConfigureSet(IOID_3, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#else // 1312
    // bring trace output on UART0 TX pin
    IOCPortConfigureSet(IOID_13, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#endif // 1312
#endif //WISUN_NCP_ENABLE
    ns_enable_module();
    //ns_enable_exception_trace();

}

void ns_trace_printf(uint8_t dlevel, const char *grp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ns_trace_vprintf(dlevel, grp, fmt, ap);
    va_end(ap);
}

void ns_trace_vprintf(uint8_t dlevel, const char *grp, const char *fmt, va_list ap)
{
    sem_wait(&ns_trace_mutex_handle);
    int len_written = 0, total_len =0, remaining_len;
    char *pBuf;

    /*
     * This function assumes tirtos cfg file redirects System_printf to ns_put_char_blocking:
     *
     * var System = xdc.useModule("xdc.runtime.System");
     * var SysCallback = xdc.useModule('xdc.runtime.SysCallback');
     * System.SupportProxy = SysCallback;
     * SysCallback.putchFxn = "&ns_put_char_blocking";
     */
    pBuf = ns_buf;

    switch (dlevel) {
        case (TRACE_LEVEL_ERROR):
            len_written = SystemP_snprintf(ns_buf, sizeof(ns_buf), "%s[ERR ][%-4s]: ", VT100_COLOR_ERROR, grp);
            break;
        case (TRACE_LEVEL_WARN):
            len_written = SystemP_snprintf(ns_buf, sizeof(ns_buf), "%s[ERR ][%-4s]: ", VT100_COLOR_WARN, grp);
            break;
        case (TRACE_LEVEL_INFO):
            len_written = SystemP_snprintf(ns_buf, sizeof(ns_buf), "%s[INFO][%-4s]: ", VT100_COLOR_INFO, grp);
            break;
        case (TRACE_LEVEL_DEBUG):
            len_written = SystemP_snprintf(ns_buf, sizeof(ns_buf), "%s[DBG ][%-4s]: ", VT100_COLOR_DEBUG, grp);
            break;
        default:
            len_written = SystemP_snprintf(ns_buf, sizeof(ns_buf), "                ", grp);
            break;
    }
    total_len += len_written;
    remaining_len = sizeof(ns_buf) - total_len;

    // update the buf position
    pBuf += len_written;
    len_written = SystemP_vsnprintf(pBuf, remaining_len, fmt, ap);
    if (len_written > (remaining_len - sizeof(VT100_RESET_TERM)))
    {
        len_written = remaining_len - sizeof(VT100_RESET_TERM);
    }

    total_len += len_written;
    remaining_len = sizeof(ns_buf) - total_len;

    // update the buf position
    pBuf += len_written;
    len_written = SystemP_snprintf(pBuf, remaining_len, VT100_RESET_TERM, grp);

    total_len += len_written;
    remaining_len = sizeof(ns_buf) - total_len;

    // update the buf position
    pBuf += len_written;

    for(int x = 0; x < total_len; x++)
    {
        ns_put_char_blocking(ns_buf[x]);
    }
    sem_post(&ns_trace_mutex_handle);
}

void ns_enable_module(void)
{
    // Enable ITM module
    ITM_TCR |= ITM_TCR_ENABLE_ITM;
}

void ns_disable_module(void)
{
    // Disable ITM module
    ITM_TCR &= ~ITM_TCR_ENABLE_ITM;
}

void ns_put_char_blocking(const char ch)
{
    while(0 == ITM_STIM_PORT_8(ITM_PORT));
    ITM_STIM_PORT_8(ITM_PORT) = ch;
}

void ns_enable_exception_trace(void)
{
    DWT_CTRL |= DWT_CTRL_ENABLE_EXC_TRC;
    ITM_TCR  |= ITM_TCR_ENABLE_DWT_TX;
}

void ns_enable_ps_sampling(void)
{
    // Clear the PC Sampling and Cycle Event bits
    DWT_CTRL &= ~(DWT_CTRL_ENABLE_PC_SAMP | DWT_CTRL_ENABLE_CYC_EVT);
    // Setup Sampling interval
    DWT_CTRL |= DWT_CTRL_CYC_CNT_1024;
    ITM_TCR  |= ITM_TCR_ENABLE_DWT_TX;
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_CNT;
    // Enable PC sampling event
    DWT_CTRL |= DWT_CTRL_ENABLE_PC_SAMP;
}

bool ns_enable_data_trace(const uint32_t *variable)
{
    uint_least8_t numDwtComp = (DWT_CTRL & DWT_CTRL_MASK_NUM_COMP) >> DWT_CTRL_SHIFT_NUM_COMP;
    uint_least8_t dwtIndex = 0;
    bool dwtAvailable = false;
    for (dwtIndex = 0; dwtIndex < numDwtComp ; dwtIndex++)
    {
        if (0 == DWT_FUNC(dwtIndex))
        {
            DWT_COMP(dwtIndex) = (uint32_t)variable;
            DWT_MASK(dwtIndex) = 0x0;
            DWT_FUNC(dwtIndex) = (DWT_FUNC_DATA_SIZE_32 | DWT_FUNC_ENABLE_ADDR_OFFSET | DWT_FUNC_ENABLE_COMP_RW);
            dwtAvailable = true;
        }
    }
    return dwtAvailable;
}

void ns_enable_cycle_counter(void)
{
    // Clear the PC Sampling and Cycle Event bits
    DWT_CTRL &= ~(DWT_CTRL_ENABLE_PC_SAMP | DWT_CTRL_ENABLE_CYC_EVT);
    // Setup Sampling interval
    DWT_CTRL |= DWT_CTRL_CYC_CNT_1024;
    ITM_TCR  |= ITM_TCR_ENABLE_DWT_TX;
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_CNT;
    // Enable Cycle Count event
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_EVT;
}

void ns_enable_timing(ITM_tsPrescale tsPrescale)
{
    // Set timestamp prescalar enable timestamp generation
    ITM_TCR   |= ((tsPrescale << ITM_TCR_TS_PRESCALE_SHIFT) & ITM_TCR_TS_PRESCALE_MASK);
    ITM_TCR   |= (ITM_TCR_ENABLE_TS);
}

void ns_enable_sync_packets(ITM_syncPacketRate syncPacketRate)
{
    // Clear sync packet rate
    DWT_CTRL &= ~(DWT_CTRL_MASK_SYNCTAP);
    // Set sync packet rate
    DWT_CTRL |= ((syncPacketRate << DWT_CTRL_SHIFT_SYNCTAP) & DWT_CTRL_MASK_SYNCTAP);
    // Enable sync packet generation
    DWT_CTRL |= DWT_CTRL_ENABLE_CYC_CNT;
    ITM_TCR  |= (ITM_TCR_ENABLE_SYNC);
}

void ns_flush_module(void)
{
    // Wait till the ITM Busy status has been cleared
    while (ITM_TCR & ITM_TCR_BUSY)
    {
        asm (" NOP");
    }
}

char *ns_trace_ipv6(const void *addr_ptr)
{
    if (addr_ptr == NULL) {
        return "<null>";
    }
    tmpStr[0] = 0;
    ip6tos(addr_ptr, tmpStr);
    return tmpStr;
}

char *ns_trace_ipv6_prefix(const uint8_t *prefix, uint8_t prefix_len)
{
    if ((prefix_len != 0 && prefix == NULL) || prefix_len > 128) {
        return "<err>";
    }

    ip6_prefix_tos(prefix, prefix_len, tmpStr);

    return tmpStr;
}

char *ns_trace_array(const uint8_t *buf, uint16_t len)
{
    int i;
    if (len == 0) {
        return "";
    }
    if (buf == NULL) {
        return "<null>";
    }

    const uint8_t *ptr = buf;
    char *pOutput = tmpStr;
    // zero tmpbuf to Null
    memset (pOutput, 0x0,DEFAULT_TRACE_TMP_LINE_LEN);

    char overflow = 0;
    for (i = 0; i < len; i++) {
        int retval = SystemP_snprintf(pOutput, DEFAULT_TRACE_TMP_LINE_LEN, "%02x:", *ptr++);
        if (retval <= 0 || retval > DEFAULT_TRACE_TMP_LINE_LEN) {
            overflow = 1;
            break;
        }
        // move pOutput to next position
        pOutput += retval;

    }

    if (overflow) {
        // replace last character as 'star',
        // which indicate buffer len is not enough
        tmpStr[DEFAULT_TRACE_TMP_LINE_LEN - 1] = '*';
    } else {
        //null to replace last ':' character
        tmpStr[DEFAULT_TRACE_TMP_LINE_LEN - 1] = 0;
    }

    return tmpStr;
}

char *ns_trace_array16(const uint16_t *buf, uint16_t len)
{
    int i;
    if (len == 0) {
        return "";
    }
    if (buf == NULL) {
        return "<null>";
    }

    const uint16_t *ptr = buf;
    char *pOutput = tmpStr;
    // zero tmpbuf to Null
    memset (pOutput, 0x0,DEFAULT_TRACE_TMP_LINE_LEN);

    char overflow = 0;
    for (i = 0; i < len; i++) {
        int retval = SystemP_snprintf(pOutput, DEFAULT_TRACE_TMP_LINE_LEN, "%04x: ", *ptr++);
        if (retval <= 0 || retval > DEFAULT_TRACE_TMP_LINE_LEN) {
            overflow = 1;
            break;
        }
        // move pOutput to next position
        pOutput += retval;

    }

    if (overflow) {
        // replace last character as 'star',
        // which indicate buffer len is not enough
        tmpStr[DEFAULT_TRACE_TMP_LINE_LEN - 1] = '*';
    } else {
        //null to replace last ':' character
        tmpStr[DEFAULT_TRACE_TMP_LINE_LEN - 1] = 0;
    }

    return tmpStr;

}
