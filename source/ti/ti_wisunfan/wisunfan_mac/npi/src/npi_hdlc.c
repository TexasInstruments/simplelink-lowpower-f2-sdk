/******************************************************************************

 @file  npi_hdlc.c

 @brief NPI Transport Layer Module for HDLC

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
#ifndef FREERTOS_SUPPORT
#include <xdc/std.h>
#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4) || defined(DeviceFamily_CC26X3)
#include <ti/sysbios/family/arm/v8m/Hwi.h>
#else
#include <ti/sysbios/family/arm/m3/Hwi.h>
#endif
#endif
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "mt.h"

#include <ti_drivers_config.h>
#include "hal_types.h"

#include "inc/npi_hdlc.h"

extern void NPITask_HDLC_transportRX_Done_CallBack(int size);

static const uint16_t kHdlcCrcCheckValue = 0xf0b8;
static const uint16_t kHdlcCrcResetValue = 0xffff;

HDLC_Dbg_s hdlc_dbg;

static uint16_t hdlc_crc16(uint16_t aFcs, uint8_t aByte)
{
#if 1
    // CRC-16/CCITT, CRC-16/CCITT-TRUE, CRC-CCITT
    // width=16 poly=0x1021 init=0x0000 refin=true refout=true xorout=0x0000 check=0x2189 name="KERMIT"
    // http://reveng.sourceforge.net/crc-catalogue/16.htm#crc.cat.kermit
    static const uint16_t sFcsTable[256] = {
        0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5,
        0xe97e, 0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb, 0xae52,
        0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3,
        0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
        0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9,
        0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 0x5285, 0x430c, 0x7197, 0x601e,
        0x14a1, 0x0528, 0x37b3, 0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 0x6306, 0x728f,
        0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
        0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862,
        0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb,
        0x4e64, 0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1, 0x0948,
        0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
        0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226,
        0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785, 0xe51e, 0xf497,
        0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704,
        0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
        0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb,
        0x0e70, 0x1ff9, 0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c,
        0x3de3, 0x2c6a, 0x1ef1, 0x0f78};
    return (aFcs >> 8) ^ sFcsTable[(aFcs ^ aByte) & 0xff];
#else
    // CRC-16/CCITT-FALSE, same CRC as 802.15.4
    // width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1 name="CRC-16/CCITT-FALSE"
    // http://reveng.sourceforge.net/crc-catalogue/16.htm#crc.cat.crc-16-ccitt-false
    aFcs = (uint16_t)((aFcs >> 8) | (aFcs << 8));
    aFcs ^= aByte;
    aFcs ^= ((aFcs & 0xff) >> 4);
    aFcs ^= (aFcs << 12);
    aFcs ^= ((aFcs & 0xff) << 5);
    return aFcs;
#endif
}

static bool hdlc_byte_needs_escape(uint8_t byte)
{
    switch (byte)
    {
    case HDLC_BYTE_SPECIAL:
    case HDLC_BYTE_ESC:
    case HDLC_BYTE_FLAG:
    case HDLC_BYTE_XOFF:
    case HDLC_BYTE_XON:
        return true;
    default:
        return false;
    }
}

uint8_t *add_NLI_byte(const uint8_t *inputBuf, uint16_t inputFrameLen, uint8_t nli, uint16_t *outputBufLen)
{
    uint8_t *outputBuf = NULL;

    if (inputFrameLen > 0)
    {
         *outputBufLen = inputFrameLen + 1;

    	outputBuf = (uint8_t*)MAP_ICall_malloc(*outputBufLen);
    	if (NULL != outputBuf)
    	{
    		outputBuf[0] = (NLI_FLAG_FIELD | (nli << 4));
    		memcpy(outputBuf + 1, inputBuf, inputFrameLen);
    	}
    	else
    	{
    		*outputBufLen = 0;
    	}
	}

    return (outputBuf);
}



uint16_t hdlc_encode(const uint8_t *inputBuf, uint16_t inputFrameLen, uint8_t *outputBuf, uint16_t out_max)
{
    uint16_t escaped_frame_len = 0;

    hdlc_dbg.num_tx_hdlc_pkt++;

    if ((NULL != outputBuf) && (inputFrameLen > 0))
    {
        uint8_t  c;
        uint16_t fcs = kHdlcCrcResetValue;
        uint16_t i;

        outputBuf[escaped_frame_len++] = HDLC_BYTE_FLAG;

        for (i =  0 ; i < inputFrameLen; i++)
        {
            c   = inputBuf[i];
            fcs = hdlc_crc16(fcs, c);
            if (hdlc_byte_needs_escape(c))
            {
                outputBuf[escaped_frame_len++] = HDLC_BYTE_ESC;
                outputBuf[escaped_frame_len++] = c ^ HDLC_ESCAPE_XFORM;
            }
            else
            {
                outputBuf[escaped_frame_len++] = c;
            }

            if (escaped_frame_len >= out_max)
            {
                // Buffer overflow
                hdlc_dbg.num_tx_buffer_overrun++;
                return 0;
            }
        }

        fcs ^= 0xFFFF;

        c = fcs & 0xFF;
        // make outbuffer has enough space
        if ( (escaped_frame_len+2) >= out_max)
        {
            // Buffer overflow
            hdlc_dbg.num_tx_buffer_overrun++;
            return 0;
        }

        if (hdlc_byte_needs_escape(c))
        {
            outputBuf[escaped_frame_len++] = HDLC_BYTE_ESC;
            outputBuf[escaped_frame_len++] = c ^ HDLC_ESCAPE_XFORM;
        }
        else
        {
            outputBuf[escaped_frame_len++] = c;
        }

        c = (fcs >> 8) & 0xFF;
        if (hdlc_byte_needs_escape(c))
        {
            outputBuf[escaped_frame_len++] = HDLC_BYTE_ESC;
            outputBuf[escaped_frame_len++] = c ^ HDLC_ESCAPE_XFORM;
        }
        else
        {
            outputBuf[escaped_frame_len++] = c;
        }

        outputBuf[escaped_frame_len++] = HDLC_BYTE_FLAG;

    }
    hdlc_dbg.num_tx_bytes += escaped_frame_len;
    return escaped_frame_len;
}

HDLC_STATE_e mState = HDLC_State_NO_SYNC;
uint16_t mDecodedLength;
uint16_t mFcs;
// HDLC RX decoding working buffer
// This buffer is used to store the decoded HDLC frame
uint8_t hdlc_buf_output[MAX_FRAME_SIZE+2];

void hdlc_decode(const uint8_t *aData, uint16_t aLength)
{
    while (aLength--)
    {
        uint8_t byte = *aData++;
        hdlc_dbg.num_rx_bytes++;
        switch (mState)
        {
        case HDLC_State_NO_SYNC:
            if (byte == HDLC_BYTE_FLAG)
            {
                mState          = HDLC_State_SYNC;
                mDecodedLength  = 0;
                mFcs            = kHdlcCrcResetValue;
            }

            break;

        case HDLC_State_SYNC:
            switch (byte)
            {
            case HDLC_BYTE_ESC:
                mState = HDLC_State_ESCAPED;
                break;

            case HDLC_BYTE_FLAG:
                /* find the paired HDLC_BYTE_FLAG */
                if (mDecodedLength == 0 )
                {   /*  input packet is BYTE_FLAG + BYTE_FLAG
                        stay in the same SYNC STATE
                    */
                   mDecodedLength  = 0;
                   mFcs            = kHdlcCrcResetValue;
                   //mState          = HDLC_State_NO_SYNC;

                   hdlc_dbg.num_rx_hdlc_pkt_flag_by_flag++;
                }
                else
                {
                    // check if the fcs is good
                    if ( mFcs == kHdlcCrcCheckValue )
                    {   // remove the FCS from output
                        mDecodedLength -= HDLC_FCS_SIZE;

                        hdlc_dbg.num_rx_hdlc_pkt++;
                        //* copy the whole packet to further processing
                        NPITask_HDLC_transportRX_Done_CallBack(mDecodedLength);
                    }
                    else
                    {   //CRC is bad, drop this packet
                        hdlc_dbg.num_rx_hdlc_crc_bad++;
                    }
                    mState          = HDLC_State_NO_SYNC;
                }

                mDecodedLength  = 0;
                mFcs            = kHdlcCrcResetValue;

                break;

            default:
                if ( mDecodedLength < MAX_FRAME_SIZE)
                {
                    mFcs = hdlc_crc16(mFcs, byte);
                    hdlc_buf_output[mDecodedLength++] = byte;
                }
                else
                {   /*  there is an error, packet length is too big
                        drop this packet
                    */
                    mDecodedLength  = 0;
                    mFcs            = kHdlcCrcResetValue;
                    mState          = HDLC_State_NO_SYNC;
                    hdlc_dbg.num_rx_sync_state_len_too_big++;
                }

                break;
            }

            break;

        case HDLC_State_ESCAPED:
            if ( mDecodedLength < MAX_FRAME_SIZE)
            {
                byte ^= 0x20;
                mFcs = hdlc_crc16(mFcs, byte);
                hdlc_buf_output[mDecodedLength++] = byte;
                /* change back to SYNC state */
                mState = HDLC_State_SYNC;
            }
            else
            {
                /* there is an error, packet legth is too big */

                mDecodedLength  = 0;
                mFcs            = kHdlcCrcResetValue;
                mState          = HDLC_State_NO_SYNC;
                hdlc_dbg.num_rx_esc_state_len_too_big++;
            }

            break;
        }
    }
}
