/*
 * Copyright (c) 2010-2014 Texas Instruments Incorporated
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
 * include this software, other than combinations with devices manufactured by or for TI (“TI Devices”).
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
 * THIS SOFTWARE IS PROVIDED BY TI AND TI’S LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI’S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== ulpsmacaddr.c =============================================
 *  
 */

#include <string.h>

#include "ulpsmacaddr.h"

const ulpsmacaddr_t ulpsmacaddr_null = { { 0, 0, 0, 0, 0, 0, 0, 0 } };
const ulpsmacaddr_t ulpsmacaddr_broadcast = { { 0xff, 0xff, 0xff, 0xff, 0xff,
                                                0xff, 0xff, 0xff } };

const uint16_t ulpsmacaddr_null_short = 0x0;

ulpsmacaddr_t ulpsmacaddr_long_node;
uint16_t ulpsmacaddr_short_node;

/*---------------------------------------------------------------------------*/
inline void ulpsmacaddr_swap_order(ulpsmacaddr_t *addr)
{
    uint8_t i;
    uint8_t temp[ULPSMACADDR_LONG_SIZE];

    for (i = 0; i < ULPSMACADDR_LONG_SIZE; i++)
    {
        temp[i] = addr->u8[ULPSMACADDR_LONG_SIZE - 1 - i];
    }

    for (i = 0; i < ULPSMACADDR_LONG_SIZE; i++)
    {
        addr->u8[i] = temp[i];
    }
}

inline void ulpsmacaddr_long_copy(ulpsmacaddr_t *dest, const ulpsmacaddr_t *src)
{
    uint8_t i;
    for (i = 0; i < ULPSMACADDR_LONG_SIZE; i++)
    {
        dest->u8[i] = src->u8[i];
    }
}

/*---------------------------------------------------------------------------*/
inline void ulpsmacaddr_short_copy(ulpsmacaddr_t *dest,
                                   const ulpsmacaddr_t *src)
{
    uint8_t i;
    for (i = 0; i < ULPSMACADDR_SHORT_SIZE; i++)
    {
        dest->u8[i] = src->u8[i];
    }
}

/*---------------------------------------------------------------------------*/
inline void ulpsmacaddr_copy(uint8_t addr_mode, ulpsmacaddr_t *dest,
                             const ulpsmacaddr_t *src)
{
    uint8_t i, cp_size;

    switch (addr_mode)
    {
    case ULPSMACADDR_MODE_LONG:
        cp_size = ULPSMACADDR_LONG_SIZE;
        break;
    case ULPSMACADDR_MODE_SHORT:
        cp_size = ULPSMACADDR_SHORT_SIZE;
        break;
    default:
        cp_size = 0;
        break;
    }

    for (i = 0; i < cp_size; i++)
    {
        dest->u8[i] = src->u8[i];
    }
}

/*---------------------------------------------------------------------------*/
inline int ulpsmacaddr_long_cmp(const ulpsmacaddr_t *addr1,
                                const ulpsmacaddr_t *addr2)
{
    uint8_t i;
    for (i = 0; i < ULPSMACADDR_LONG_SIZE; i++)
    {
        if (addr1->u8[i] != addr2->u8[i])
        {
            return 0;
        }
    }

    // same addr
    return 1;
}

/*---------------------------------------------------------------------------*/
inline int ulpsmacaddr_short_cmp(const ulpsmacaddr_t *addr1,
                                 const ulpsmacaddr_t *addr2)
{
    uint8_t i;
    for (i = 0; i < ULPSMACADDR_SHORT_SIZE; i++)
    {
        if (addr1->u8[i] != addr2->u8[i])
        {
            return 0;
        }
    }

    // same addr
    return 1;
}

/*---------------------------------------------------------------------------*/
inline int ulpsmacaddr_cmp(uint8_t addr_mode, const ulpsmacaddr_t *addr1,
                           const ulpsmacaddr_t *addr2)
{
    uint8_t i, cmp_size;

    switch (addr_mode)
    {
    case ULPSMACADDR_MODE_LONG:
        cmp_size = ULPSMACADDR_LONG_SIZE;
        break;
    case ULPSMACADDR_MODE_SHORT:
        cmp_size = ULPSMACADDR_SHORT_SIZE;
        break;
    default:
        cmp_size = 0;
        break;
    }

    for (i = 0; i < cmp_size; i++)
    {
        if (addr1->u8[i] != addr2->u8[i])
        {
            return 0;
        }
    }

    // same addr
    return 1;
}
/*---------------------------------------------------------------------------*/

/**************************************************************************************************
 * @fn          ulpsmacaddr_copy_swapped
 *
 * @brief       Copy source address to destination address from the end to start
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void ulpsmacaddr_copy_swapped(uint8_t *addr_dest, uint8_t dest_len,
                              uint8_t *addr_src, uint8_t src_len)
{
    uint8_t i;
    memset(addr_dest, 0, dest_len);
    for (i = 0; i < src_len; i++)
    {
        addr_dest[i] = addr_src[(src_len - 1) - i];
    }
}
/** @} */
