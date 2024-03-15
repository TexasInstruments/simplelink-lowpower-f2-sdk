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
 * include this software, other than combinations with devices manufactured by or for TI (TI Devices).
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
 * THIS SOFTWARE IS PROVIDED BY TI AND TIS LICENSORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TIS LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 *  ====================== bm_api.h =============================================
 *  
 */

#ifndef BM_API_H
#define BM_API_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#if defined (__IAR_SYSTEMS_ICC__) || defined(__TI_COMPILER_VERSION__)
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <stdio.h>
#include <string.h>
#endif

#include <ti/sysbios/hal/Hwi.h>

#include "net/packetbuf.h"
#include "ulpsmacaddr.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Defines
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */
#ifdef MAX_PACKET_TEST
#define BM_NUM_BUFFER           2
#define BM_BUFFER_SIZE          (2000)
#else
#if IS_ROOT
#ifdef ENABLE_HCT
#define BM_NUM_BUFFER           10
#else
#define BM_NUM_BUFFER           10       //FIXME
#endif
#elif IS_INTERMEDIATE
#define BM_NUM_BUFFER           10       //FIXME
#else //LEAF
#define BM_NUM_BUFFER           10
#endif

#define BM_BUFFER_SIZE          (128)
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* BM API status */
typedef enum
{
    BM_SUCCESS = 0,
    BM_ERROR_POINTER = 1,
    BM_ERROR_INVALID_DATA_LEN = 2,
    BM_ERROR_INVALID_DATA_OFFSET = 3,
    BM_ERROR_DEC_LENGTH = 4,
    BM_ERROR_INC_LENGTH = 5,

    BM_STATUS_MAX

} BM_STATUS_t;

typedef struct BM_BUF_t
{
    uint16_t datalen;          // data length in the buffer
    uint16_t dataoffset;       // data starting address in the buffer
    uint16_t buf_aligned[(BM_BUFFER_SIZE) / 2];
} BM_BUF_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

// we will use SYS/RTOS function
uint32_t SpinLock(void);
void SpinUnlock(uint32_t st);

void BM_init(void);
BM_BUF_t* BM_alloc(void);
BM_STATUS_t BM_free(BM_BUF_t* pbuf);
BM_STATUS_t BM_reset(BM_BUF_t *pBuf);

uint8_t BM_copyfrom(BM_BUF_t* pbuf, void *pSrc, const uint16_t len);
uint8_t BM_copyto(BM_BUF_t* pbuf, void *pDst);

BM_STATUS_t BM_decDataOffset(BM_BUF_t *pBuf, const uint8_t len);
BM_STATUS_t BM_incDataOffset(BM_BUF_t *pBuf, uint8_t len);

// help functions
inline BM_STATUS_t BM_setDataLen(BM_BUF_t *pBuf, const uint8_t len);
inline uint16_t BM_getDataLen(BM_BUF_t *pBuf);
inline BM_STATUS_t BM_setDataOffset(BM_BUF_t *pBuf, const uint8_t offset);
inline uint8_t BM_getDataOffset(BM_BUF_t *pBuf);

inline uint8_t * BM_getBufPtr(BM_BUF_t *pBuf);
inline uint8_t * BM_getDataPtr(BM_BUF_t *pBuf);

BM_BUF_t * BM_new_from_packetbuf(void);

/**************************************************************************************************
 */

#endif /* BM_API_H */

