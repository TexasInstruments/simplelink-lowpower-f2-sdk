/******************************************************************************

 @file icall_osal_rom_jt.h

 @brief Icall and Osal API directly map the function to function jump table

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated

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

#ifndef ICALL_OSAL_ROM_JT_H
#define ICALL_OSAL_ROM_JT_H

#ifndef OSAL_PORT2TIRTOS
#include "icall.h"
#include "osal.h"
#else
#include "osal_port.h"
#include "osal_port_timers.h"

#include <ti/drivers/dpl/ClockP.h>
#endif

#include "saddr.h"
#include "comdef.h"
#include "rom_jt_def_154.h"

/*
** ICALL OSAL API Proxy
** ROM-to ROM or ROM-to-Flash function
** if there is any patch function, replace the corresponding entries
*/

#ifdef TIMAC_ROM_IMAGE_BUILD

#ifndef OSAL_PORT2TIRTOS

#define ICALL_OSAL_API_BASE_INDEX                       (0)

#define MAP_ICall_enterCriticalSection                  ((ICall_CSState     (*)(void ))                               ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+0))
#define MAP_ICall_leaveCriticalSection                  ((void     (*)(ICall_CSState ))                               ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+1))
#define MAP_osal_mem_alloc                              ((void *    (*)(uint16_t ))                                   ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+2))
#define MAP_osal_mem_free                               ((void     (*)(void * ))                                      ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+3))
#define MAP_osal_memcmp                                 ((uint8     (*)(const void *,const void *,unsigned int ))     ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+4))
#define MAP_osal_memcpy                                 ((void *    (*)(void *, const void *, unsigned int ))         ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+5))
#define MAP_osal_memset                                 ((void *    (*)(void *, uint8, int ))                         ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+6))
#define MAP_osal_msg_allocate                           ((uint8 *    (*)(uint16 ))                                    ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+7))
#define MAP_osal_msg_deallocate                         ((uint8      (*)(uint8 * ))                                   ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+8))
#define MAP_osal_msg_dequeue                            ((void *    (*)(osal_msg_q_t * ))                             ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+9))
#define MAP_osal_msg_enqueue                            ((void *    (*)(osal_msg_q_t *, void *  ))                    ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+10))
#define MAP_osal_msg_enqueue_max                        ((uint8    (*)(osal_msg_q_t *, void *, uint8  ))              ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+11))
#define MAP_osal_msg_extract                            ((void    (*)(osal_msg_q_t *, void *,void * ))                ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+12))
#define MAP_osal_msg_push                               ((void    (*)(osal_msg_q_t *, void * ))                       ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+13))
#define MAP_osal_msg_receive                            ((uint8 *  (*)(uint8 ))                                       ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+14))
#define MAP_osal_msg_send                               ((uint8    (*)(uint8, uint8 * ))                              ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+15))
#define MAP_osal_pwrmgr_task_state                      ((uint8    (*)(uint8, uint8  ))                               ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+16))
#define MAP_osal_set_event                              ((uint8    (*)(uint8, uint16  ))                              ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+17))
#define MAP_osal_strlen                                 ((int      (*)(char *  ))                                     ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+18))
#define MAP_memcmp                                      ((int (*)(const void *, const void *, unsigned int ))         ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+19))
#define MAP_ICall_getTicks                              ((uint_fast32_t    (*)(void ))                                ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+20))
#define MAP_ICall_setTimer                              ((ICall_Errno    (*)(uint_fast32_t,MAP_ICall_TimerCback,void *,MAP_ICall_TimerID * )) \
                                                                                                                      ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+21))
#define MAP_ICall_stopTimer                             ((void  (*)(MAP_ICall_TimerID))                               ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+22))
#define MAP_sAddrCmp                                    ((bool  (*)(const sAddr_t *,const sAddr_t *))                 ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+23))
#define MAP_sAddrCpy                                    ((void  (*)(sAddr_t *,const sAddr_t *))                       ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+24))
#define MAP_sAddrExtCpy                                 ((void *  (*)(uint8 * ,const uint8 *))                        ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+25))
#define MAP_sAddrExtCmp                                 ((bool  (*)(uint8 * ,const uint8 *))                          ROM_ICALL_OSAL_JT_OFFSET(ICALL_OSAL_API_BASE_INDEX+26))

#define MAP_osal_GetSystemClock                         osal_GetSystemClock
#define MAP_osal_start_timerEx                          osal_start_timerEx
#define MAP_osal_start_reload_timer                     osal_start_reload_timer
#define MAP_osal_stop_timerEx                           osal_stop_timerEx
#define MAP_osal_get_timeoutEx                          osal_get_timeoutEx

#define MAP_osal_rand                                   osal_rand

#define MAP_ICall_getTickPeriod                         ICall_getTickPeriod
#define MAP_ICall_malloc                                ICall_malloc
#define MAP_ICall_free                                  ICall_free
#define MAP_ICall_allocMsg                              ICall_allocMsg
#define MAP_ICall_freeMsg                               ICall_freeMsg

#define MAP_osal_msg_find                               osal_msg_find

#define MAP_osal_build_uint16                           osal_build_uint16
#define MAP_osal_build_uint32                           osal_build_uint32
#define MAP_osal_buffer_uint32                          osal_buffer_uint32
#define MAP_osal_isbufset                               osal_isbufset
#define MAP_osal_offsetof                               osal_offsetof

#define MAP_OSAL_MSG_NEXT                               OSAL_MSG_NEXT
#define MAP_OSAL_MSG_Q_INIT                             OSAL_MSG_Q_INIT
#define MAP_OSAL_MSG_Q_EMPTY                            OSAL_MSG_Q_EMPTY
#define MAP_OSAL_MSG_Q_HEAD                             OSAL_MSG_Q_HEAD
#define MAP_OSAL_MSG_LEN                                OSAL_MSG_LEN
#define MAP_OSAL_MSG_ID                                 OSAL_MSG_ID

#define MAP_PWRMGR_HOLD                                 PWRMGR_HOLD
#define MAP_PWRMGR_CONSERVE                             PWRMGR_CONSERVE

#define MAP_OSAL_TIMERS_MAX_TIMEOUT                     OSAL_TIMERS_MAX_TIMEOUT
#define MAP_TASK_NO_TASK                                TASK_NO_TASK

#define MAP_ICall_TimerCback                            ICall_TimerCback
#define MAP_ICall_TimerID                               ICall_TimerID
#define MAP_osal_msg_q_t                                osal_msg_q_t
#define MAP_osal_msg_hdr_t                              osal_msg_hdr_t
#define MAP_osal_event_hdr_t                            osal_event_hdr_t
#define MAP_osal_revmemcpy                              osal_revmemcpy
#else // OSAL_PORT2TIRTOS
/* Map functions not used by ROM, so do not need to be exposed through JT */

#ifndef USE_DMM

#define ICall_EntityID              uint8_t
#define ICall_Semaphore             Semaphore_Handle

#endif

#define ICall_Errno uint8_t
#define ICall_CSState uint32_t

#define MAP_osal_mem_alloc                              OsalPort_malloc
#define MAP_osal_mem_free                               OsalPort_free
#define MAP_osal_memcmp                                 OsalPort_memcmp
#define MAP_osal_memcpy                                 memcpy
#define MAP_osal_revmemcpy                              OsalPort_revmemcpy
#define MAP_osal_memset                                 memset
#define MAP_osal_msg_allocate                           OsalPort_msgAllocate
#define MAP_osal_msg_deallocate                         OsalPort_msgDeallocate
#define MAP_osal_msg_dequeue                            OsalPort_msgDequeue
#define MAP_osal_msg_enqueue                            OsalPort_msgEnqueue
#define MAP_osal_msg_enqueue_max                        OsalPort_msgEnqueueMax
#define MAP_osal_msg_extract                            OsalPort_msgExtract
#define MAP_osal_msg_push                               OsalPort_msgPush
#define MAP_osal_msg_receive                            OsalPort_msgReceive
#define MAP_osal_msg_send                               OsalPort_msgSend
#define MAP_osal_msg_find                               OsalPort_msgFind

#define MAP_osal_pwrmgr_task_state                      OsalPort_pwrmgrTaskState
#define MAP_osal_set_event                              OsalPort_setEvent
#define MAP_osal_clear_event                            OsalPort_clearEvent
#define MAP_osal_strlen                                 strlen
#define MAP_memcmp                                      memcmp
#define MAP_osal_build_uint16                           OsalPort_buildUint16
#define MAP_osal_build_uint32                           OsalPort_buildUint32
#define MAP_osal_buffer_uint32                          OsalPort_bufferUint32
#define MAP_osal_isbufset                               OsalPort_isBufSet
#define MAP_osal_offsetof                               OsalPort_OFFSET_OF

#define MAP_osal_GetSystemClock()                       ((ClockP_getSystemTicks() * ClockP_getSystemTickPeriod()) / 1000)
#define MAP_osal_start_timerEx                          OsalPortTimers_startTimer
#define MAP_osal_start_reload_timer                     OsalPortTimers_startReloadTimer
#define MAP_osal_stop_timerEx                           OsalPortTimers_stopTimer
#define MAP_osal_get_timeoutEx                          OsalPortTimers_getTimerTimeout

#define MAP_osal_rand                                   OsalPort_rand

#define MAP_ICall_getTicks                              ClockP_getSystemTicks
#define MAP_ICall_setTimer                              OsalPort_setTimer
#define MAP_ICall_stopTimer                             OsalPort_stopTimer
#define MAP_ICall_getTickPeriod()                       ClockP_getSystemTickPeriod()

#define MAP_ICall_malloc                                OsalPort_malloc
#define MAP_ICall_free                                  OsalPort_free
#define MAP_ICall_allocMsg                              OsalPort_msgAllocate
#define MAP_ICall_freeMsg                               OsalPort_msgDeallocate

#define MAP_sAddrCmp                                    sAddrCmp
#define MAP_sAddrCpy                                    sAddrCpy
#define MAP_sAddrExtCpy                                 sAddrExtCpy
#define MAP_sAddrExtCmp                                 sAddrExtCmp

#define MAP_OSAL_MSG_NEXT                               OsalPort_MSG_NEXT
#define MAP_OSAL_MSG_Q_INIT                             OsalPort_MSG_Q_INIT
#define MAP_OSAL_MSG_Q_EMPTY                            OsalPort_MSG_Q_EMPTY
#define MAP_OSAL_MSG_Q_HEAD                             OsalPort_MSG_Q_HEAD
#define MAP_OSAL_MSG_LEN                                OsalPort_MSG_LEN
#define MAP_OSAL_MSG_ID                                 OsalPort_MSG_ID

#define MAP_PWRMGR_HOLD                                 OsalPort_PWR_HOLD
#define MAP_PWRMGR_CONSERVE                             OsalPort_PWR_CONSERVE

#define MAP_OSAL_TIMERS_MAX_TIMEOUT                     OsalPortTimers_TIMERS_MAX_TIMEOUT
#define MAP_TASK_NO_TASK                                OsalPort_TASK_NO_TASK

#define MAP_ICall_TimerCback                            OsalPort_TimerCback
#define MAP_ICall_TimerID                               OsalPort_TimerID
#define MAP_osal_msg_q_t                                OsalPort_MsgQ
#define MAP_osal_msg_hdr_t                              OsalPort_MsgHdr
#define MAP_osal_event_hdr_t                            OsalPort_EventHdr

#endif // OSAL_PORT2TIRTOS

#endif // TIMAC_ROM_IMAGE_BUILD

#endif // ICALL_OSAL_ROM_JT_H
