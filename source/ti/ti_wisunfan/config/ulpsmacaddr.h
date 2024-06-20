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
 *  ====================== ulpsmacaddr.h =============================================
 *  
 */

#ifndef __ULPSMACADDR_H__
#define __ULPSMACADDR_H__

#if defined (__IAR_SYSTEMS_ICC__) || defined(__TI_COMPILER_VERSION__)
#include <stdint.h>
#endif

#define ULPSMACADDR_MODE_NULL   0x00
#define ULPSMACADDR_MODE_SHORT  0x02
#define ULPSMACADDR_MODE_LONG   0x03

/*---------------------------------------------------------------------------*/
#define ULPSMACADDR_LONG_SIZE   8    /* long address size */
#define ULPSMACADDR_SHORT_SIZE  2    /* short address size */
#define ULPSMACADDR_PANID_SIZE  2    /* pan id size */

/*---------------------------------------------------------------------------*/
typedef union
{
    uint8_t u8[ULPSMACADDR_LONG_SIZE];
} ulpsmacaddr_t;

typedef uint16_t ulpsmacaddr_panid_t;

/*---------------------------------------------------------------------------*/
extern const ulpsmacaddr_t ulpsmacaddr_null;
extern const ulpsmacaddr_t ulpsmacaddr_broadcast;

extern ulpsmacaddr_t ulpsmacaddr_long_node;
extern uint16_t ulpsmacaddr_short_node;

extern const uint16_t ulpsmacaddr_null_short;

/*---------------------------------------------------------------------------*/
static inline uint16_t ulpsmacaddr_to_short(const ulpsmacaddr_t* addr)
{
    return ((addr->u8[1] << 8) + addr->u8[0]);
}

/*---------------------------------------------------------------------------*/
static inline void ulpsmacaddr_from_short(ulpsmacaddr_t* addr,
                                          const uint16_t short_addr)
{
    addr->u8[0] = short_addr & 0xff;
    addr->u8[1] = (short_addr >> 8) & 0xff;
}

/*---------------------------------------------------------------------------*/
static inline uint8_t ulpsmacaddr_addr_mode(ulpsmacaddr_t* addr)
{
    uint8_t addr_mode;

    if ((addr->u8[2] == 0x00) && (addr->u8[3] == 0x00) && (addr->u8[4] == 0x00)
            && (addr->u8[5] == 0x00) && (addr->u8[6] == 0x00)
            && (addr->u8[7] == 0x00))
    {

        if ((addr->u8[0] == 0x00) && (addr->u8[1] == 0x00))
        {
            addr_mode = ULPSMACADDR_MODE_NULL;
        }
        else
        {
            addr_mode = ULPSMACADDR_MODE_SHORT;
        }

    }
    else
    {
        addr_mode = ULPSMACADDR_MODE_LONG;
    }

    return addr_mode;
}

extern inline void
ulpsmacaddr_swap_order(ulpsmacaddr_t *addr);
/*---------------------------------------------------------------------------*/
extern inline void
ulpsmacaddr_long_copy(ulpsmacaddr_t *dest, const ulpsmacaddr_t *from);
/*---------------------------------------------------------------------------*/
extern inline void
ulpsmacaddr_short_copy(ulpsmacaddr_t *dest, const ulpsmacaddr_t *from);
/*---------------------------------------------------------------------------*/
extern inline void
ulpsmacaddr_copy(uint8_t addr_mode, ulpsmacaddr_t *dest,
                 const ulpsmacaddr_t *from);
/*---------------------------------------------------------------------------*/
extern inline int
ulpsmacaddr_long_cmp(const ulpsmacaddr_t *addr1, const ulpsmacaddr_t *addr2);
/*---------------------------------------------------------------------------*/
extern inline int
ulpsmacaddr_short_cmp(const ulpsmacaddr_t *addr1, const ulpsmacaddr_t *addr2);
/*---------------------------------------------------------------------------*/
extern inline int
ulpsmacaddr_cmp(uint8_t addr_mode, const ulpsmacaddr_t *addr1,
                const ulpsmacaddr_t *addr2);
/*---------------------------------------------------------------------------*/
extern void ulpsmacaddr_copy_swapped(uint8_t *addr_dest, uint8_t dest_len,
                                     uint8_t *addr_src, uint8_t src_len);

#endif /* __ULPSMACADDR_H__ */
/** @} */
/** @} */
