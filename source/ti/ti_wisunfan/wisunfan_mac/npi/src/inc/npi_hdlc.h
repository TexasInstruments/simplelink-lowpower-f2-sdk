/******************************************************************************

 @file  npi_hdlc.h

 @brief NPI Transport Layer Module for UART

 Group: WCS, LPC, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated
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
#ifndef NPI_HDLC_H
#define NPI_HDLC_H

// use OSAL malloc function
#include "osal_port.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ****************************************************************************
// includes
// ****************************************************************************

// ****************************************************************************
// defines
// ****************************************************************************

#define NPI_HDLC_RX_BUF_SIZE (4096)

#define MAX_FRAME_SIZE  (2048)
#define HDLC_FCS_SIZE   (2)

#define HDLC_BYTE_FLAG 0x7E
#define HDLC_BYTE_ESC 0x7D
#define HDLC_BYTE_XON 0x11
#define HDLC_BYTE_XOFF 0x13
#define HDLC_BYTE_SPECIAL 0xF8
#define HDLC_ESCAPE_XFORM 0x20
#define NLI_FLAG_FIELD 0x40

// HDLC Decode state
typedef enum HDLC_STATE_ {
    HDLC_State_NO_SYNC          = 0 ,
    HDLC_State_SYNC                 ,
    HDLC_State_ESCAPED
} HDLC_STATE_e;

typedef struct __hdlc_debug_
{
    uint16_t num_tx_bytes;
    uint16_t num_rx_bytes;
    uint16_t num_tx_hdlc_pkt;
    uint16_t num_rx_hdlc_pkt;
    uint16_t num_rx_hdlc_crc_bad;
    uint16_t num_rx_hdlc_pkt_flag_by_flag;
    uint16_t num_rx_sync_state_len_too_big;
    uint16_t num_rx_esc_state_len_too_big;
    uint16_t num_tx_buffer_overrun;
    uint16_t num_rx_buffer_overflow;

} HDLC_Dbg_s;


// ****************************************************************************
// typedefs
// ****************************************************************************


//*****************************************************************************
// globals
//*****************************************************************************

//*****************************************************************************
// function prototypes
//*****************************************************************************
uint8_t *add_NLI_byte(const uint8_t *inputBuf, uint16_t inputFrameLen, uint8_t nli, uint16_t *outputBufLen);
uint16_t hdlc_encode(const uint8_t *inputBuf, uint16_t inputFrameLen, uint8_t *outputBuf, uint16_t out_max);
void hdlc_decode(const uint8_t *aData, uint16_t aLength);


uint16 NPI_HDLC_RxBuf_Read(uint16 len);
uint16 NPI_HDLC_RxBuf_GetRxBufCount(void);
uint16 NPI_HDLC_RxBuf_GetRxBufAvail(void);
uint16 NPI_HDLC_RxBuf_ReadFromRxBuf(uint8_t *buf, uint16 len);


uint8_t * NPI_HDLC_RxBuf_GetHead(void);
uint16_t NPI_HDLC_RxBuf_GetProcLength(void);
void NPI_HDLC_RxBuf_UpdateHead(uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* NPI_TL_UART_H */
