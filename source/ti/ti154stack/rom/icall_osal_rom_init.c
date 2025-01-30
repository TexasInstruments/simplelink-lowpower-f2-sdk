/******************************************************************************

 @file  icall_osal_rom_init.c

 @brief This file contains the externs for ROM API initialization.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated

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

/*******************************************************************************
 * INCLUDES
 */

#ifndef OSAL_PORT2TIRTOS
#include "icall.h"
#include "osal.h"
#include "osal_pwrmgr.h"
#else
#include "osal_port.h"
#define ICall_Errno uint8_t
#define ICall_CSState uint32_t
#endif

#include "string.h"
#include "saddr.h"

#include "rom_jt_def_154.h"

#include <ti/drivers/dpl/ClockP.h>

/*******************************************************************************
 * EXTERNS
 */

extern ICall_CSState ICall_enterCSImpl(void);
extern void ICall_leaveCSImpl(ICall_CSState key);


/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// ROM Flash Jump Table

#if defined(USE_ICALL) && !defined(USE_DMM)
#if defined __TI_COMPILER_VERSION || defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(ICALL_OSAL_ROM_Flash_JT, 4)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__ ((aligned (4)))
#else
#pragma data_alignment=4
#endif

const uint32 ICALL_OSAL_ROM_Flash_JT[] =
{
     /* function pointer */
     (uint32)&ICall_enterCSImpl,                                        // ROM_ICALL_OSAL_JT_OFFSET[0]
     (uint32)&ICall_leaveCSImpl,                                        // ROM_ICALL_OSAL_JT_OFFSET[1]

     (uint32)&osal_mem_alloc,                                           // ROM_ICALL_OSAL_JT_OFFSET[2]
     (uint32)&osal_mem_free,                                            // ROM_ICALL_OSAL_JT_OFFSET[3]
     (uint32)&osal_memcmp,                                              // ROM_ICALL_OSAL_JT_OFFSET[4]
     (uint32)&osal_memcpy,                                              // ROM_ICALL_OSAL_JT_OFFSET[5]
     (uint32)&osal_memset,                                              // ROM_ICALL_OSAL_JT_OFFSET[6]

     (uint32)&osal_msg_allocate,                                        // ROM_ICALL_OSAL_JT_OFFSET[7]
     (uint32)&osal_msg_deallocate,                                      // ROM_ICALL_OSAL_JT_OFFSET[8]
     (uint32)&osal_msg_dequeue,                                         // ROM_ICALL_OSAL_JT_OFFSET[9]
     (uint32)&osal_msg_enqueue,                                          // ROM_ICALL_OSAL_JT_OFFSET[10]
     (uint32)&osal_msg_enqueue_max,                                     // ROM_ICALL_OSAL_JT_OFFSET[11]

     (uint32)&osal_msg_extract,                                         // ROM_ICALL_OSAL_JT_OFFSET[12]
     (uint32)&osal_msg_push,                                          	// ROM_ICALL_OSAL_JT_OFFSET[13]
     (uint32)&osal_msg_receive,                                         // ROM_ICALL_OSAL_JT_OFFSET[14]
     (uint32)&osal_msg_send,                                          	// ROM_ICALL_OSAL_JT_OFFSET[15]
     (uint32)&osal_pwrmgr_task_state,                                   // ROM_ICALL_OSAL_JT_OFFSET[16]
     (uint32)&osal_set_event,                                          	// ROM_ICALL_OSAL_JT_OFFSET[17]
     (uint32)&osal_strlen,                                              // ROM_ICALL_OSAL_JT_OFFSET[18]
     (uint32)&memcmp,                                          			// ROM_ICALL_OSAL_JT_OFFSET[19]

     (uint32)&ICall_getTicks,                                          	// ROM_ICALL_OSAL_JT_OFFSET[20]
     (uint32)&ICall_setTimer,                                          	// ROM_ICALL_OSAL_JT_OFFSET[21]
     (uint32)&ICall_stopTimer,                                          // ROM_ICALL_OSAL_JT_OFFSET[22]

     (uint32)&sAddrCmp,                                             // ROM_ICALL_OSAL_JT_OFFSET[23]
     (uint32)&sAddrCpy,                                             // ROM_ICALL_OSAL_JT_OFFSET[24]
     (uint32)&sAddrExtCpy,                                          // ROM_ICALL_OSAL_JT_OFFSET[25]
     (uint32)&sAddrExtCmp,                                          // ROM_ICALL_OSAL_JT_OFFSET[26]

};
#else
const uint32 ICALL_OSAL_ROM_Flash_JT[] =
{
     /* function pointer */
     (uint32)&OsalPort_enterCS,                                        // ROM_ICALL_OSAL_JT_OFFSET[0]
     (uint32)&OsalPort_leaveCS,                                        // ROM_ICALL_OSAL_JT_OFFSET[1]

#ifdef MALLOC_DEBUG
     (uint32)&OsalPort_malloc_dbg,                                           // ROM_ICALL_OSAL_JT_OFFSET[2]
     (uint32)&OsalPort_free_dbg,                                            // ROM_ICALL_OSAL_JT_OFFSET[3]
#else
      (uint32)&OsalPort_malloc,                                           // ROM_ICALL_OSAL_JT_OFFSET[2]
      (uint32)&OsalPort_free,                                            // ROM_ICALL_OSAL_JT_OFFSET[3]
#endif
     (uint32)&OsalPort_memcmp,                                              // ROM_ICALL_OSAL_JT_OFFSET[4]
     (uint32)&memcpy,                                              // ROM_ICALL_OSAL_JT_OFFSET[5]
     (uint32)&memset,                                              // ROM_ICALL_OSAL_JT_OFFSET[6]

#ifdef MALLOC_DEBUG
     (uint32)&OsalPort_msgAllocate_dbg,                                        // ROM_ICALL_OSAL_JT_OFFSET[7]
     (uint32)&OsalPort_msgDeallocate_dbg,                                      // ROM_ICALL_OSAL_JT_OFFSET[8]
#else
      (uint32)&OsalPort_msgAllocate,                                        // ROM_ICALL_OSAL_JT_OFFSET[7]
      (uint32)&OsalPort_msgDeallocate,                                      // ROM_ICALL_OSAL_JT_OFFSET[8]
#endif
     (uint32)&OsalPort_msgDequeue,                                         // ROM_ICALL_OSAL_JT_OFFSET[9]
     (uint32)&OsalPort_msgEnqueue,                                          // ROM_ICALL_OSAL_JT_OFFSET[10]
     (uint32)&OsalPort_msgEnqueueMax,                                     // ROM_ICALL_OSAL_JT_OFFSET[11]

     (uint32)&OsalPort_msgExtract,                                         // ROM_ICALL_OSAL_JT_OFFSET[12]
     (uint32)&OsalPort_msgPush,                                            // ROM_ICALL_OSAL_JT_OFFSET[13]
     (uint32)&OsalPort_msgReceive,                                         // ROM_ICALL_OSAL_JT_OFFSET[14]
     (uint32)&OsalPort_msgSend,                                            // ROM_ICALL_OSAL_JT_OFFSET[15]
     (uint32)&OsalPort_pwrmgrTaskState,                                   // ROM_ICALL_OSAL_JT_OFFSET[16]
     (uint32)&OsalPort_setEvent,                                           // ROM_ICALL_OSAL_JT_OFFSET[17]
     (uint32)&strlen,                                              // ROM_ICALL_OSAL_JT_OFFSET[18]
     (uint32)&memcmp,                                                   // ROM_ICALL_OSAL_JT_OFFSET[19]

     (uint32)&ClockP_getSystemTicks,                                           // ROM_ICALL_OSAL_JT_OFFSET[20]
     (uint32)&OsalPort_setTimer,                                           // ROM_ICALL_OSAL_JT_OFFSET[21]
     (uint32)&OsalPort_stopTimer,                                          // ROM_ICALL_OSAL_JT_OFFSET[22]

     (uint32)&sAddrCmp,                                             // ROM_ICALL_OSAL_JT_OFFSET[23]
     (uint32)&sAddrCpy,                                             // ROM_ICALL_OSAL_JT_OFFSET[24]
     (uint32)&sAddrExtCpy,                                          // ROM_ICALL_OSAL_JT_OFFSET[25]
     (uint32)&sAddrExtCmp,                                          // ROM_ICALL_OSAL_JT_OFFSET[26]
};

#endif


void Icall_Osal_ROM_Init(void)
{
    /* assign the Icall Osal ROM JT table */
    RAM_MAC_BASE_ADDR[ROM_RAM_ICALL_OSAL_TABLE_INDEX] = (uint32)(ICALL_OSAL_ROM_Flash_JT);
}

