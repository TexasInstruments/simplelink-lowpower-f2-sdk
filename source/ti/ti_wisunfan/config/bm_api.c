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
 *  ====================== bm_api.c =============================================
 *  
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "bm_api.h"
#include "lib/memb.h"
#include "middleware.h"

#include <string.h>

MEMB(BM_BUF, BM_BUF_t, BM_NUM_BUFFER);

#ifdef MW_DEBUG
extern MW_Debug_t mwDbg;
#endif

/**************************************************************************************************//**
 *
 * @brief       This function will disable the interrupt and allow caller enter critical
 *              section.
 *
 * @param[in]
 *
 *
 * @return      current interrupt status. This value will be used by SpinUnlock function call
 ***************************************************************************************************/
uint32_t SpinLock(void)
{
    int32_t intSt;
    intSt = Hwi_disable();

    return intSt;
}

/**************************************************************************************************//**
 *
 * @brief       This function will restore the interrupts which are disable by the previous SpinLock
 *              function call.and allow caller enter critical
 *
 *
 * @param[in]   st -- interrupt status (from previous SpinLock
 *
 * @return      None
 ***************************************************************************************************/
void SpinUnlock(uint32_t st)
{
    Hwi_restore(st);
}

/**************************************************************************************************//**
 *
 * @brief       This function initializes the BM internal data structure.  It must be called
 *              once when the software system is started and before any other function in the
 *              buffer manager is called. The number of data buffer blocks is statically configured.
 *
 * @param[in]
 *
 * @return      None.
 ***************************************************************************************************/
void BM_init(void)
{
    memb_init(&BM_BUF);
}

/**************************************************************************************************//**
 *
 * @brief       This function is used to allocate one buffer from buffer manager.  If the
 *              allocation is success, the buffer manager will return a pointer to BM_BUF_t
 *              data structure. If there is no buffer left in the buffer manager pool, it will
 *              return NULL. It is caller responsibility to check the returned value after
 *              calling this API.
 *
 * @param[in]
 *
 *
 * @return      The pointer to BM_BUF_t data structure. If the allocation operation is failure,
 *              the returned value will be NULL
 ***************************************************************************************************/
BM_BUF_t* BM_alloc(void)
{
    BM_BUF_t* pbuf;
    uint32_t int_st;

    // enter critical section
    int_st = SpinLock();
    pbuf = memb_alloc(&BM_BUF);

    // exit critical section
    SpinUnlock(int_st);

    if (!pbuf)
    {
        return NULL;
    }
#ifdef MW_DEBUG
    mwDbg.cntBmAlloc++;
#endif

    // set data structure to default value
    pbuf->datalen = 0;
    pbuf->dataoffset = 0;

    return pbuf;
}

/**************************************************************************************************//**
 *
 * @brief       This function is used to free the previously allocated buffer and returned
 *              the buffer back to buffer manager. After this function call, the previously
 *              allocated buffer is no longer valid.
 *
 * @param[in]   pbuf: previous allocated BM_BUF_t pointer
 *
 * @return      BM_SUCCESS  if the free operation is success.
 *              BM_ERROR_POINTER  if the input buffer pointer has some problems.
 ***************************************************************************************************/
BM_STATUS_t BM_free(BM_BUF_t* pbuf)
{
    int rtn;
    uint32_t int_st;

    // enter critical section
    int_st = SpinLock();

    rtn = memb_free(&BM_BUF, pbuf);

    // exit critical section
    SpinUnlock(int_st);

    if (rtn == -1)
    {
#ifdef MW_DEBUG
        mwDbg.cntBmFreeError++;
#endif
        return BM_ERROR_POINTER;
    }
#ifdef MW_DEBUG
    mwDbg.cntBmFree++;
#endif

    return BM_SUCCESS;
}

/**************************************************************************************************//**
 *
 * @brief       This function is used to reset the buffer data structure to default value.
 *
 * @param[in]
 *
 * @return      The pointer to BM_BUF_t data structure. If the allocation operation is failure,
 *              the returned value will be NULL
 ***************************************************************************************************/
BM_STATUS_t BM_reset(BM_BUF_t *pbuf)
{
    // set data structure to default value
    pbuf->datalen = 0;
    pbuf->dataoffset = 0;

    return BM_SUCCESS;
}

/**************************************************************************************************//**
 *
 * @brief       This function is used to copy data to BM buffer from other memory location.
 *              The data is copied to buffer memory which starts from dataptr. If the specified
 *              data size is greater than the size which BM buffer can support. It will only
 *              copy up to its capacity and return the number of copied data bytes.
 *
 *
 * @param[in]   pbuf: BM_BUF_t pointer
 *              pSrc: pointer to source data memory
 *              len: number of bytes from the source memory
 *
 *
 * @return      The number of byte actually copied.
 ***************************************************************************************************/
uint8_t BM_copyfrom(BM_BUF_t* pbuf, void *pSrc, const uint16_t len)
{
    uint16_t copy_len;
    uint8_t *pDstAddr;

    copy_len = BM_BUFFER_SIZE - pbuf->dataoffset;

    if (copy_len >= len)
    {
        copy_len = len;
    }
    pDstAddr = (uint8_t *) pbuf->buf_aligned;
    pDstAddr += pbuf->dataoffset;

    memcpy(pDstAddr, pSrc, copy_len);

    // set the data length field
    pbuf->datalen = copy_len;

    // return number of bytes copied
    return copy_len;
}

/**************************************************************************************************//**
 *
 * @brief       This function is used to copy data from BM buffer to other memory location.
 *              The data is copied from BM buffer memory which starts from dataptr. We assume
 *              user specified memory has enough memory to hold the BM buffer's data. It is caller's
 *              responsibility to provide memory with enough space.
 *
 * @param[in]   pbuf: BM_BUF_t pointer
 *              pDst: pointer to destination data memory
 *
 *
 * @return      The number of byte actually copied.
 ***************************************************************************************************/
uint8_t BM_copyto(BM_BUF_t* pbuf, void *pDst)
{
    uint16_t copy_len;
    uint8_t *pSrcAddr;

    copy_len = pbuf->datalen;

    // source address in uint8
    pSrcAddr = (uint8_t *) pbuf->buf_aligned;
    pSrcAddr += pbuf->dataoffset;

    memcpy(pDst, pSrcAddr, copy_len);

    // return number of bytes copied
    return copy_len;
}

/**************************************************************************************************//**
 *
 * @brief       This function sets the BM buffer data length. It is help function and
 *              implemented as inline function.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *              len: number of bytes in data length
 *
 *
 * @return      BM_SUCCESS  if the  operation is success.
 *              BM_ERROR_INVALID_DATA_LEN if the user specified data length is
 *              beyond the BM buffer length.
 ***************************************************************************************************/
BM_STATUS_t BM_setDataLen(BM_BUF_t *pBuf, const uint8_t len)
{
    // check make sure there is enough space
    if ((BM_BUFFER_SIZE - pBuf->dataoffset) >= len)
    {
        pBuf->datalen = len;
        return BM_SUCCESS;
    }

    return BM_ERROR_INVALID_DATA_LEN;
}

/**************************************************************************************************//**
 *
 * @brief       This function returns the data length of the buffer. It is help function and
 *              implemented as inline function.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *
 * @return      number of bytes in data length
 ***************************************************************************************************/
uint16_t BM_getDataLen(BM_BUF_t *pBuf)
{
    return pBuf->datalen;
}

/**************************************************************************************************//**
 *
 * @brief       This function sets the data offset of BM buffer. It is help function and
 *              implemented as inline function.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *              of:fset data offset specified by user
 *
 * @return      BM_SUCCESS  if the  operation is success.
 *              BM_ERROR_INVALID_DATA_OFFSET  if the user specified data offset is beyond
 *              the buffer length.
 ***************************************************************************************************/
BM_STATUS_t BM_setDataOffset(BM_BUF_t *pBuf, const uint8_t offset)
{
    // check make sure there is enough space
    if ( BM_BUFFER_SIZE >= offset)
    {
        pBuf->dataoffset = offset;
        return BM_SUCCESS;
    }

    return BM_ERROR_INVALID_DATA_OFFSET;
}

/**************************************************************************************************//**
 *
 * @brief       This function returns the data offset of the buffer. It is help function and
 *              implemented as inline function.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *
 * @return      data offset
 ***************************************************************************************************/
uint8_t BM_getDataOffset(BM_BUF_t *pBuf)
{
    return pBuf->dataoffset;
}

/**************************************************************************************************//**
 *
 * @brief       This function returns the buffer pointer  (bufptr) of BM buffer.
 *              It is help function and implemented as inline function.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *
 * @return      : pointer to BM buffer
 ***************************************************************************************************/
uint8_t * BM_getBufPtr(BM_BUF_t *pBuf)
{
    return (uint8_t *) (&pBuf->buf_aligned[0]);
}

/**************************************************************************************************//**
 *
 * @brief       This function returns the data pointer (databufptr) of BM buffer.
 *              It is help function and implemented as inline function.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *
 *
 * @return      : The databufptr in the BM buffer data structure.
 ***************************************************************************************************/
uint8_t * BM_getDataPtr(BM_BUF_t *pBuf)
{
    return ((uint8_t *) (&pBuf->buf_aligned) + pBuf->dataoffset);
}

/**************************************************************************************************//**
 *
 * @brief       This function will decrement the BM buffer's data offset and increase the data
 *              length accordingly.  It is help function and implemented as inline function.
 *              If the decrement length is greater than the current data offset, this function
 *              will return error.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *              len: the decrement length for BM buffer's data offset
 *
 * @return      BM_SUCCESS if this operation is success.
 *              BM_ERROR_DEC_LENGTH if the decrement length is greater than current
 *              BM buffer's data offset.
 ***************************************************************************************************/
BM_STATUS_t BM_decDataOffset(BM_BUF_t *pBuf, const uint8_t len)
{
    if (pBuf->dataoffset > len)
    {
        pBuf->dataoffset -= len;
        pBuf->datalen += len;
        return BM_SUCCESS;
    }

    return BM_ERROR_DEC_LENGTH;
}

/**************************************************************************************************//**
 *
 * @brief       This function will increment the BM buffer's data offset and decrease the data
 *              length accordingly.  It is help function and implemented as inline function.
 *              If the increment length is greater than the current data length, this function
 *              will return error.
 *
 * @param[in]   pBuf: BM_BUF_t pointer
 *              len: the incrementt length for BM buffer's data offset
 *
 * @return      BM_SUCCESS if this operation is success.
 *              BM_ERROR_INC_LENGTH if the increment length is greater than BM buffer's
 *              data length.
 ***************************************************************************************************/
BM_STATUS_t BM_incDataOffset(BM_BUF_t *pBuf, const uint8_t len)
{
    if (pBuf->datalen >= len)
    {
        pBuf->dataoffset += len;
        pBuf->datalen -= len;
        return BM_SUCCESS;
    }

    return BM_ERROR_INC_LENGTH;
}

/**************************************************************************************************//**
 *
 * @brief       This function is help function. It will copy the packbuf data to BM buffer.
 *
 * @param[in]
 *
 * @return      BM buffer pointer which stores the packetbuf data. if BM buffer is not available,
 *              it will be NULL
 ***************************************************************************************************/
BM_BUF_t * BM_new_from_packetbuf(void)
{
    BM_BUF_t *pBuf;

    pBuf = BM_alloc();

    if (pBuf != NULL)
    {
        pBuf->datalen = packetbuf_copyto(pBuf->buf_aligned);

        if (pBuf->datalen == 0)
        {
            BM_free(pBuf);
            return NULL;
        }

    }

    return pBuf;
}

