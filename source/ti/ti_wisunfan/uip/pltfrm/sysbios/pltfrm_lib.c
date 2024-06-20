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
 *  ====================== pltfrm_lib.c =============================================
 *  SYSBIOS related OS functions. 
 */

#include "pltfrm_lib.h"

void pltfrm_debug_hex(int b)
{
#ifdef __SYS_BIOS__
    System_printf("%.2x\n", b);
#if PLTFRM_DEBUG_ALWAYS_FLUSH
    System_flush();
#endif
#endif
}

void pltfrm_debug_dec(long b)
{
#ifdef __SYS_BIOS__
    System_printf("%d\n", b);
#if PLTFRM_DEBUG_ALWAYS_FLUSH
    System_flush();
#endif
#endif
}

void pltfrm_debug_flush()
{
    System_flush();
}

//*-------------------------------------------------*//

void pltfrm_byte_memcpy_swapped(void* destination, int dest_len, void* source,
                                int src_len)
{
    int i;
    memset(destination, 0, dest_len); //init to zeros.
    for (i = 0; i < src_len; i++)
    {
        ((char*) destination)[i] = ((char*) source)[(src_len - 1) - i];
    }
}

//*-------------------------------------------------*//

Void clock_FuncPtr(UArg params)
{
    pltfrm_timer_t *t = (pltfrm_timer_t*) params;
    t->func(t->params);
}

void pltfrm_timer_init(pltfrm_timer_t* timer)
{
    timer->handle = NULL;
    timer->func = NULL;
    timer->params = NULL;
}

Clock_Params clkParams;

void pltfrm_timer_create(pltfrm_timer_t* timer, pltfrm_timer_function_t func,
                         void* func_params, unsigned int timeout)
{

    Clock_Params_init(&clkParams);
    clkParams.period = 0;
    clkParams.startFlag = FALSE;

    timer->func = func;
    timer->params = func_params;
    timer->handle = Clock_create(clock_FuncPtr, 0, &clkParams, NULL); //TODO: check for error?
    Clock_setFunc(((Clock_Handle) timer->handle), clock_FuncPtr, (UArg) timer);
}

void pltfrm_timer_delete(pltfrm_timer_t* timer)
{
    Clock_Handle handle = (Clock_Handle) timer->handle;

    if (handle == NULL)
        return;

    Clock_delete(&handle);
    timer->handle = NULL;
}
