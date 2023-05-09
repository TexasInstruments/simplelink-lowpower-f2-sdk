/******************************************************************************

 @file  ti_itm.c

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include <ti_itm.h>
#include <ti_itm_internal.h>

#include <sid_time_types.h>
#include <sid_pal_uptime_ifc.h>

#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/HwiP.h>

#include <stdio.h>
#include <ioc.h>


/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

#define TI_LOG_HIGHTLIGHT_ISR   0

#define ITM_STIM_PORT_8(x)      (*(volatile uint8_t *) ITM_STIM_PORT((x)))

#define ITM_PORT 0

#define TI_SEMAPHORE_WAIT_FOREVER   ~(0)
#define TI_MAX_LOGBUF_LEN           128

// 2K buffer to hold logs
#define DEFERRED_LOG_BUF_LEN    2048

#define VT100_COLOR_ERROR       "\x1b[31m"
#define VT100_COLOR_WARNING     "\x1b[33m"
#define VT100_COLOR_INFO        "\x1b[39m"
#define VT100_COLOR_DEBUG       "\x1b[90m"
#define VT100_COLOR_DEMO        "\x1b[39m"

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static int deferred_log_ptr = 0;
static char deferred_log[DEFERRED_LOG_BUF_LEN];
static bool defer_logging = false;
static char logBuffer[TI_MAX_LOGBUF_LEN];
static char cCode[NO_OF_ITM_LOG_LEVELS][6] = { {VT100_COLOR_ERROR}, {VT100_COLOR_WARNING},
                                               {VT100_COLOR_INFO},  {VT100_COLOR_DEBUG} };
static char sCode[NO_OF_ITM_LOG_LEVELS][8] = { {"error"}, {"warning"}, {"info"}, {"debug"} };

static SemaphoreP_Handle logMutex;

/* ------------------------------------------------------------------------------------------------
 *                                           Functions
 * ------------------------------------------------------------------------------------------------
 */

// should be called under critical section
static void queue_chars(const uint8_t data, const uint8_t port)
{
    // if buffer is full, logs will be dropped until it is drained
    if (deferred_log_ptr < DEFERRED_LOG_BUF_LEN) {
        deferred_log[deferred_log_ptr++] = data;
    }
}

// should be called under critical section
static void queue_array(const char *buffer, uint16_t count)
{
    for (int i = 0; i < count; i++) {
        queue_chars(*buffer++, ITM_PORT);
    }
}

void ITM_logPrint(uint8_t level, const char *group, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    ITM_vprint(level, group, fmt, ap);
    va_end(ap);
}

uint16_t ITM_hexDump(uint8_t level, const char *group, const void *pData, uint16_t len)
{
    char const digit[16] = "0123456789abcdef";
    char * pd = (char *)pData;
    char u8data;
    char data;
    int count = 0;
    uint16_t i;
    uint16_t j;
    uint32_t key;

    if (!logMutex || SemaphoreP_pend(logMutex, TI_SEMAPHORE_WAIT_FOREVER)) {
        return 0;
    }
    key = HwiP_disable();
#if TI_LOG_WITH_TIMESTAMP
    struct sid_timespec time;
    sid_pal_uptime_now(&time);
    count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "[%08lu%03lu] ", time.tv_sec, time.tv_nsec/SID_TIME_NSEC_PER_MSEC);
    queue_array(logBuffer, count);
#endif

#if TI_LOG_HIGHTLIGHT_ISR
    if (HwiP_inISR()) {
        count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "%s<%s> ", cCode[!level ? 0 : 1], sCode[level]);
    }
    else
#endif
    {
        count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "%s<%s> ", cCode[level], sCode[level]);
    }

    queue_array(logBuffer, count);

    for (i = 0; i < len; i++) {
        u8data = *(pd + i);
        data = digit[(u8data >> 4) & 0x0F];
        queue_chars(data, ITM_PORT);
        data = digit[(u8data >> 0) & 0x0F];
        queue_chars(data, ITM_PORT);
        if ((i != len - 1) && (i % 16 == 15)) {
            queue_chars('\n', ITM_PORT);
            queue_chars('\r', ITM_PORT);
            for (j = 0; j < count - 5; j++) {
                queue_chars(' ', ITM_PORT);
            }
        } else if (i % 8 == 7) {
            queue_chars(' ', ITM_PORT);
            queue_chars(' ', ITM_PORT);
            queue_chars(' ', ITM_PORT);
            queue_chars(' ', ITM_PORT);
        } else {
            queue_chars(' ', ITM_PORT);
        }
    }

    count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "\x1b[0m\n\r");
    queue_array(logBuffer, count);
    HwiP_restore(key);
    SemaphoreP_post(logMutex);

    if (!defer_logging) {
        ITM_flushModule();
    }

    return len;
}

uint16_t ITM_vprint(uint8_t level, const char *group, const char *fmt, va_list ap)
{
    int count = 0;
    uint16_t len = 0;
    uint32_t key;

    if (!logMutex || SemaphoreP_pend(logMutex, TI_SEMAPHORE_WAIT_FOREVER)) {
        return 0;
    }
    key = HwiP_disable();
#if TI_LOG_WITH_TIMESTAMP
    struct sid_timespec time;
    sid_pal_uptime_now(&time);
    count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "[%08lu%03lu] ", time.tv_sec, time.tv_nsec/SID_TIME_NSEC_PER_MSEC);
    queue_array(logBuffer, count);
#endif

#if TI_LOG_HIGHTLIGHT_ISR
    if (HwiP_inISR()) {
        count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "%s<%s> ", cCode[!level ? 0 : 1], sCode[level]);
    }
    else
#endif
    {
        count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "%s<%s> ", cCode[level], sCode[level]);
    }
    queue_array(logBuffer, count);

    count = vsnprintf(logBuffer, TI_MAX_LOGBUF_LEN, fmt, ap);
    queue_array(logBuffer, count);
    len = count;

    count = snprintf(logBuffer, TI_MAX_LOGBUF_LEN, "\x1b[0m\n\r");
    queue_array(logBuffer, count);

    HwiP_restore(key);
    SemaphoreP_post(logMutex);

    if (!defer_logging) {
        ITM_flushModule();
    }

    return len;
}

void ITM_enableModule(void)
{
    // Enable ITM module
    ITM_TCR |= ITM_TCR_ENABLE_ITM;
}

void ITM_disableModule(void)
{
    // Disable ITM module
    ITM_TCR &= ~ITM_TCR_ENABLE_ITM;
}

void ITM_getLibraryVersion(uint32_t *majorVersion, uint32_t *minorVersion)
{
    if (majorVersion && minorVersion) {
        *majorVersion = ITM_LIB_MAJOR_VER;
        *minorVersion = ITM_LIB_MINOR_VER;
    }
}

static void flush_logs(void)
{
    uint16_t key = HwiP_disable();
    uint32_t log_len = deferred_log_ptr;
    HwiP_restore(key);

    for (int i = 0; i < log_len; i++) {
        while(0 == ITM_STIM_PORT_8(ITM_PORT));
        ITM_STIM_PORT_8(ITM_PORT) = deferred_log[i];
    }

    key = HwiP_disable();
    // if any logs were added while we were printing the buffer
    // go ahead and print while interrupts are disabled to flush
    // the entire buffer.
    if (deferred_log_ptr > log_len) {
        for (int i = log_len; i < deferred_log_ptr; i++) {
            while(0 == ITM_STIM_PORT_8(ITM_PORT));
            ITM_STIM_PORT_8(ITM_PORT) = deferred_log[i];
        }
    }

    deferred_log_ptr = 0;
    HwiP_restore(key);
}

void ITM_flushModule(void)
{
    flush_logs();

    // Wait till the ITM Busy status has been cleared
    while (ITM_TCR & ITM_TCR_BUSY) {
        __asm (" NOP");
    }
}

void ITM_initModule(const ITM_config *itm_config)
{
    /* create a Semaphore object to use as a resource lock, initial count 1 */
    logMutex = SemaphoreP_createBinary(1);

    if (!logMutex) {
        return;
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
    if (itm_config->system_clock) {
        uint32_t prescalar = itm_config->system_clock / itm_config->baud_rate;

        // Offset with current prescalar value
        uint32_t diff1 = itm_config->system_clock - (prescalar * itm_config->baud_rate);
        // Offset with prescalar+1 value
        uint32_t diff2 = ((prescalar+1) * itm_config->baud_rate) - itm_config->system_clock;

        if (diff2 < diff1)  prescalar++;
        // Program prescalar value as (prescalar factor - 1)
        TPIU_ACPR = (prescalar - 1);
    }

    // Disable formatter
    TPIU_FFCR  = 0;

    // Unlock DWT
    DWT_LAR    = CS_LAR_UNLOCK;

    // Configure SWO Traces
#if defined(DeviceFamily_CC26X2X7)
    IOCPortConfigureSet(IOID_3, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#else
    IOCPortConfigureSet(IOID_13, IOC_PORT_MCU_SWV, IOC_STD_OUTPUT);
#endif

    // Enable ITM module
    ITM_TCR |= ITM_TCR_ENABLE_ITM;

    defer_logging = itm_config->defer_logging;
}

