/*
 * Copyright (c) 2014 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free,
 * non-exclusive license under copyrights and patents it now or hereafter
 * owns or controls to make, have made, use, import, offer to sell and sell ("Utilize")
 * this software subject to the terms herein.  With respect to the foregoing patent
 *license, such license is granted  solely to the extent that any such patent is necessary
 * to Utilize the software alone.  The patent license shall not apply to any combinations which
 * include this software, other than combinations with devices manufactured by or for TI (â€œTI Devicesâ€�).
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license (including the
 * above copyright notice and the disclaimer and (if applicable) source code license limitations below)
 * in the documentation and/or other materials provided with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided that the following
 * conditions are met:
 *
 *       * No reverse engineering, decompilation, or disassembly of this software is permitted with respect to any
 *     software provided in binary form.
 *       * any redistribution and use are licensed by TI for use only with TI Devices.
 *       * Nothing shall obligate TI to provide you with source code for the software licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the source code are permitted
 * provided that the following conditions are met:
 *
 *   * any redistribution and use of the source code, including any resulting derivative works, are licensed by
 *     TI for use only with TI Devices.
 *   * any redistribution and use of any object code compiled from the source code and any resulting derivative
 *     works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers may be used to endorse or
 * promote products derived from this software without specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TIâ€™S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TIâ€™S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== pltfrm_lib.h =============================================
 *  SYSBIOS related OS functions. 
 */

#ifndef PLTFRM_LIB_H_
#define PLTFRM_LIB_H_

#include "plat-conf.h"

#include <xdc/std.h>

#include <xdc/runtime/Log.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Timestamp.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Semaphore.h>

#include <ti/sysbios/BIOS.h>
#include <xdc/cfg/global.h>

#include <ti/sysbios/knl/Task.h>

#include <stdint.h>
#include <stdarg.h>

#define __access_byte(ptr,idx)	(((char*)ptr)[idx])

#include <stdio.h>
#include <string.h>

//DEBUGGING MESSAGES

#define pltfrm_debug(...)       System_printf(__VA_ARGS__)

#define pltfrm_debug_address(addr) uip_debug_ipaddr_print(addr)

#define pltfrm_debug_lladdr(lladdr) uip_debug_lladdr_print(lladdr)

#define PLTFRM_DEBUG_ALWAYS_FLUSH	1 // 0: if we do not want to flush each message debugged (use IP6_flush() for flushing). In other case, each message is directly flushed

/******************************************************************************
 *  FUNCTION NAME: pltfrm_debug_init
 *
 *  DESCRIPTION:
 *        This function initializes the debugging method if needed.
 *
 *  Return value:      none
 *
 *  Input Parameters:	none
 *
 *  Output Parameters: none
 *
 *  Functions Called:
 *
 *******************************************************************************/
void pltfrm_debug_init();

/******************************************************************************
 *  FUNCTION NAME: pltfrm_debug_hex
 *
 *  DESCRIPTION:
 *        This function prints a value in hexadecimal mode
 *
 *  Return value:      none
 *
 *  Input Parameters:
 *  		int b: the value to print
 *
 *  Output Parameters: none
 *
 *  Functions Called:
 *  		System_printf (for SYS_BIOS)
 *
 *******************************************************************************/
void pltfrm_debug_hex(int b);

/******************************************************************************
 *  FUNCTION NAME: pltfrm_debug_dec
 *
 *  DESCRIPTION:
 *        This function prints a value in decimal mode
 *
 *  Return value:      none
 *
 *  Input Parameters:
 *  		long b: the value to print
 *
 *  Output Parameters: none
 *
 *  Functions Called:
 *  		System_printf (for SYS_BIOS)
 *
 *******************************************************************************/
void pltfrm_debug_dec(long b);

/******************************************************************************
 *  FUNCTION NAME: pltfrm_debug_flush
 *
 *  DESCRIPTION:
 *        This function flushes the buffer that contains the debugging messages to the console
 *
 *  Return value:      	none
 *
 *  Input Parameters:	none
 *
 *  Output Parameters: 	none
 *
 *  Functions Called:
 *  		System_flush (for SYS_BIOS)
 *
 *******************************************************************************/
void pltfrm_debug_flush(void);

//BYTE ACCESS:

//IMPORTANT: All these functions will be defined as macros for performance purposes (better footprint)

#define pltfrm_byte_memcpy(dst,offset_dst_bytes,src,offset_src_bytes,num_bytes)     memcpy(((char*)(dst))+(offset_dst_bytes),((char*)(src))+(offset_src_bytes),num_bytes)

#define pltfrm_byte_memset(dst,offset_dst_bytes,value,num_bytes)        memset(((char*)(dst))+(offset_dst_bytes),value,num_bytes)

#define pltfrm_byte_memcmp(ptr1,offset_ptr1_bytes,ptr2,offset_ptr2_bytes,num_bytes)    memcmp(((char*)(ptr1))+(offset_ptr1_bytes),((char*)(ptr2))+(offset_ptr2_bytes),num_bytes)

#define pltfrm_byte_set(ptr,offset_bytes,value)         ((char*)ptr)[offset_bytes]=(value)

#define pltfrm_byte_get(ptr,offset_bytes)               ((char*)ptr)[offset_bytes]

//Initializes detination with dest_len zeros and then copies src_len bytes from source to destination starting from the end of source, to the beginning of destination (dest_len must be higher than src_len)
void pltfrm_byte_memcpy_swapped(void* destination, int dest_len, void* source,
                                int src_len);

//TIMESTAMPS

#define pltfrm_getTimeStamp() (Clock_getTicks())

//RANDOM NUMBERS

#define pltfrm_initRandom(seed)

#define pltfrm_getRandom()      HalTRNG_GetTRNG()

//*--------------------------------------------*//
//TIMERS

typedef void (*pltfrm_timer_function_t)(void*);

typedef struct pltfrm_timer
{
    void* handle;
    pltfrm_timer_function_t func;
    void* params;
} pltfrm_timer_t;

//Init timer structure
void pltfrm_timer_init(pltfrm_timer_t* timer);

//Reserve memory and create the timer with the corresponding function and parameters and timeout
void pltfrm_timer_create(pltfrm_timer_t* timer, pltfrm_timer_function_t func,
                         void* func_params, unsigned int timeout);

//Evaluate if the timer is already created (pltfrm_timer_create was called)
#define pltfrm_timer_isCreated(timer)   (((Clock_Handle)(timer)->handle) != NULL)

//Set timeout and start timer
#define pltfrm_timer_restart(timer,timeout)   do { Clock_stop((Clock_Handle)(timer)->handle); \
                                              Clock_setTimeout((Clock_Handle)(timer)->handle,timeout); \
                                              Clock_start((Clock_Handle)(timer)->handle); \
                                              } while (0)

//Stop the timer from executing
#define pltfrm_timer_stop(timer)        Clock_stop((Clock_Handle)(timer)->handle)

//Evaluate if the timer is running
#define pltfrm_timer_isActive(timer)    Clock_isActive((Clock_Handle)(timer)->handle)

//Delete the platform timer (free memory is necessary)
void pltfrm_timer_delete(pltfrm_timer_t* timer);

#endif /* PLTFRM_UTIL_H_ */
