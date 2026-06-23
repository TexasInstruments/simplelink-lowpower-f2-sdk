/******************************************************************************

 @file  npi_hdlc_rxbuf.c

 @brief NPI HDLC RX Buffer and utilities

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2026, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

// ****************************************************************************
// includes
// ****************************************************************************
#include <string.h>

#include <ti_drivers_config.h>
#include "hal_types.h"
#include "inc/npi_config.h"
#include "inc/npi_tl.h"
#include "inc/npi_hdlc.h"

// ****************************************************************************
// defines
// ****************************************************************************
#define NPI_HDLC_RXBUF_RXHEAD_INC(x)   HDLC_RxBufHead += x;               \
    HDLC_RxBufTail %= NPI_HDLC_RX_BUF_SIZE;

#define NPI_HDLC_RXBUF_RXTAIL_INC(x)   HDLC_RxBufTail += x;               \
    HDLC_RxBufTail %= NPI_HDLC_RX_BUF_SIZE;

// ****************************************************************************
// typedefs
// ****************************************************************************

//*****************************************************************************
// globals
//*****************************************************************************

//Receive Buffer for all NPI messages
uint8  HDLC_RxBuf[NPI_HDLC_RX_BUF_SIZE];
static uint16 HDLC_RxBufHead = 0;
static uint16 HDLC_RxBufTail = 0;

//*****************************************************************************
// function prototypes
//*****************************************************************************

// -----------------------------------------------------------------------------
//! \brief      NPI_HDLC_RxBuf_Read
//!
//! \param[in]  len -
//!
//! \return     uint16 -
// -----------------------------------------------------------------------------
uint16 NPI_HDLC_RxBuf_Read(uint16 len)
{
    uint16 partialLen = 0;

    // Need to make two reads due to wrap around of circular buffer
    if ((len + HDLC_RxBufTail) > NPI_HDLC_RX_BUF_SIZE)
    {
        partialLen = NPI_HDLC_RX_BUF_SIZE - HDLC_RxBufTail;
        NPITL_readTL(&HDLC_RxBuf[HDLC_RxBufTail],partialLen);
        len -= partialLen;
        HDLC_RxBufTail = 0;
    }

    // Read remainder of data from Transport Layer
    NPITL_readTL(&HDLC_RxBuf[HDLC_RxBufTail],len);
    NPI_HDLC_RXBUF_RXTAIL_INC(len);

    // Return len to original size
    len += partialLen;

    return len;
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are unparsed in RxBuf
//!
//! \return     uint16 -
// -----------------------------------------------------------------------------
uint16 NPI_HDLC_RxBuf_GetRxBufCount(void)
{
    return ((HDLC_RxBufTail - HDLC_RxBufHead + NPI_HDLC_RX_BUF_SIZE) % NPI_HDLC_RX_BUF_SIZE);
}

// -----------------------------------------------------------------------------
//! \brief      Returns number of bytes that are available in RxBuf
//!
//! \return     uint16 -
// -----------------------------------------------------------------------------
uint16 NPI_HDLC_RxBuf_GetRxBufAvail(void)
{
    return (NPI_HDLC_RX_BUF_SIZE - NPI_HDLC_RxBuf_GetRxBufCount());
}

// -----------------------------------------------------------------------------
//! \brief      NPI_HDLC_RxBuf_ReadFromRxBuf
//!
//! \return     uint16 -
// -----------------------------------------------------------------------------
uint16 NPI_HDLC_RxBuf_ReadFromRxBuf(uint8_t *buf, uint16 len)
{
    uint16_t idx;
    for (idx = 0; idx < len; idx++)
    {
        *buf++ = HDLC_RxBuf[HDLC_RxBufHead];
        NPI_HDLC_RXBUF_RXHEAD_INC(1)
    }

    return len;
}

uint8_t * NPI_HDLC_RxBuf_GetHead(void)
{
    return &HDLC_RxBuf[HDLC_RxBufHead];
}

uint16 NPI_HDLC_RxBuf_GetProcLength(void)
{
    uint16 partialLen = 0;
    uint16_t rd_idx =0;
    uint16 len = NPI_HDLC_RxBuf_GetRxBufCount();

    // Need to make two reads due to wrap around of circular buffer
    if ((len + HDLC_RxBufHead) > NPI_HDLC_RX_BUF_SIZE)
    {
        partialLen = NPI_HDLC_RX_BUF_SIZE - HDLC_RxBufHead;
    }
    else
    {
        partialLen = len;
    }

    return partialLen;
}

void NPI_HDLC_RxBuf_UpdateHead(uint16_t length)
{
    // Update the HDLC RxBuf Head pointer
    HDLC_RxBufHead += length;
    HDLC_RxBufHead %= NPI_HDLC_RX_BUF_SIZE;
#if 0
    // Reset the tail pointer if it has caught up to the head
    if (HDLC_RxBufHead == HDLC_RxBufTail)
    {
        HDLC_RxBufTail = 0;
    }
#endif
}