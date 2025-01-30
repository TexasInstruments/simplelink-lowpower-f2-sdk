/******************************************************************************

 @file  mt_rpc.h

 @brief Definitions for the MT RPC Transport Protocol

 Group: WCS LPC
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
#ifndef MTRPC_H
#define MTRPC_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 General RPC constants and definitions
 *****************************************************************************/
/*! Length of MT header for general format RPC frame */
#define MTRPC_FRAME_HDR_SZ  3

/*!
 Position of fields in the general format RPC frame:
   1st byte is length of the data field
   2nd byte is command type/sub-system
   3rd byte is sub-system command ID
   4th+ bytes are data block (if any)
 */
/*! General RPC frame header data length byte position */
#define MTRPC_POS_LEN   0
/*! General RPC frame header command type/sub-system byte position */
#define MTRPC_POS_CMD0  1
/*! General RPC frame header sub-system command ID byte position */
#define MTRPC_POS_CMD1  2
/*! General RPC frame header data block byte position */
#define MTRPC_POS_DAT0  3

/*!
 Maximum length of data in the general frame format. The upper limit is 255
 because of the 1-byte length protocol. The operational limit is lower for
 code size and ram savings so that the uart driver can use 256 byte rx/tx
 queues: (MTRPC_DATA_MAX + MTRPC_FRAME_HDR_SZ + MTUART_FRAME_OVHD) < 256
 */
/*! General RPC frame maximum data block size */
#define MTRPC_DATA_MAX  250

/*!
 The CMD0 frame header byte is composed of two bit fields:
   3 MSB's (bits 05-07) contain the command type
   5 LSB's (bits 00-04) contain the command sub-system ID
 */
/*! General RPC frame command type bit mask */
#define MTRPC_CMD_TYPE_MASK   0xE0
/*! General RPC frame command sub-system bit mask */
#define MTRPC_SUBSYSTEM_MASK  0x1F

/*! General RPC format status: no error */
#define MTRPC_SUCCESS         0
/*! General RPC format status: invalid sub-system */
#define MTRPC_ERR_SUBSYSTEM   1
/*! General RPC format status: invalid command ID */
#define MTRPC_ERR_COMMAND_ID  2
/*! General RPC format status: invalid parameter */
#define MTRPC_ERR_PARAMETER   3
/*! General RPC format status: invalid length */
#define MTRPC_ERR_LENGTH      4
/*! General RPC format status: unsupported extended type */
#define MTRPC_ERR_EXTTYPE     5
/*! General RPC format status: memory allocation failed */
#define MTRPC_ERR_NOMEMORY    6

/******************************************************************************
 Extended RPC constants and definitions
 *****************************************************************************/
/*! Length of extended header for fragmentation ACK frame */
#define MTRPC_FRAG_ACK_SZ  3
/*! Length of extended header for fragmentation RPC frame */
#define MTRPC_FRAG_HDR_SZ  4

/*!
 Position of fields in the extended format RPC header:
   1st byte is version/stackId bit fields
   2nd byte is fragmentation block index
   3rd-4th bytes are full data packet length
 */
/*! Extended RPC frame header version/stackId position */
#define MTRPC_POS_VERS  0
/*! Extended RPC frame header fragmentation block index position */
#define MTRPC_POS_FBLK  1
/*! Extended RPC frame header fragmentation ACK status position */
#define MTRPC_POS_ACKS  2
/*! Extended RPC frame header fragmentation packet length position */
#define MTRPC_POS_FLEN  2
/*! Extended RPC frame header fragmentation data block byte position */
#define MTRPC_POS_DATX  4

/*!
 Maximum length of data in outgoing fragment frame. The upper limit is set
 for same reasons as MTRPC_DATA_MAX, with MTRPC_FRAG_HDR_SZ subtracted. It
 may be advantageous to use smaller fragments to reduce occurances of memory
 allocation failures, so MTRPC_FRAG_MAX can be specified on the command-line
 */
/*! Extended RPC maximum fragmentation data block size */
#if !defined(MTRPC_FRAG_MAX)
#define MTRPC_FRAG_MAX  (MTRPC_DATA_MAX - MTRPC_FRAG_HDR_SZ)
#endif

/*!
 The VERS frame header byte is composed of two bit fields:
   5 MSB's (bits 03-07) contain the extended frame version number
   3 LSB's (bits 00-02) contain the extended frame stack ID number
 */
/*! Extended RPC frame extended version number bit mask */
#define MTRPC_EXTVERS_MASK  0xF8
/*! Extended RPC frame extended stackId number bit mask */
#define MTRPC_STACKID_MASK  0x07

/*! Extended RPC format status: no error */
#define MTRPC_EXT_SUCCESS    0
/*! Extended RPC resend last frame */
#define MTRPC_EXT_RESEND     1
/*! Extended RPC unsupported Stack ID  */
#define MTRPC_EXT_BADSTACK   2
/*! Extended RPC fragmentation block out-of-order */
#define MTRPC_EXT_BADBLOCK   3
/*! Extended RPC fragmentation bad block length */
#define MTRPC_EXT_BADLENGTH  4
/*! Extended RPC memory allocation error */
#define MTRPC_EXT_NOMEMORY   5
/*! Extended RPC fragmentation process is done */
#define MTRPC_EXT_FRAGDONE   6
/*! Extended RPC fragmentation process aborted */
#define MTRPC_EXT_FRAGABORT  7
/*! Extended RPC fragmentation unknown ACK  */
#define MTRPC_EXT_BADACK     8

/******************************************************************************
 Typedefs
 *****************************************************************************/
/*! General header command codes, shifted to bits 05-07 position */
typedef enum
{
    /*! RPC poll command code */
    MTRPC_CMD_POLL = 0x00,
    /*! RPC synchronous request command code */
    MTRPC_CMD_SREQ = 0x20,
    /*! RCP asynchrounous request command code */
    MTRPC_CMD_AREQ = 0x40,
    /*! RPC synchronous response code */
    MTRPC_CMD_SRSP = 0x60,
    /*! RPC extended header indicator */
    MTRPC_CMD_EXTN = 0x80
} MtRpc_CmdType_t;

typedef enum
{
    /*! RPC sub-system code */
    MTRPC_SYS_RES0,
    /*! MTSYS sub-system code */
    MTRPC_SYS_SYS,
    /*! MTMAC sub-system code */
    MTRPC_SYS_MAC,
    /*! Reserved sub-system code */
    MTRPC_SYS_RESERVED03,
    /*! Reserved sub-system code */
    MTRPC_SYS_RESERVED04,
    /*! Reserved sub-system code */
    MTRPC_SYS_RESERVED05,
    /*! Reserved sub-system code */
    MTRPC_SYS_RESERVED06,
    /*!  MTUTIL sub-system code */
    MTRPC_SYS_UTIL,
    /*! Reserved sub-system code */
    MTRPC_SYS_RESERVED08,
    /*! MTAPP sub-system code */
    MTRPC_SYS_APP,
    /*! Maximum sub-system code, must be last in list */
    MTRPC_SYS_MAX
} MtRpc_SysType_t;

/*! Extended header version codes, shifted to bits 03-07 position */
typedef enum
{
    /*! Extended RPC not used */
    MTRPC_EXT_NOTUSED = 0x00,
    /*! Extended RPC StackID frame */
    MTRPC_EXT_STACKID = 0x08,
    /*! Extended RPC Fragmentation data frame */
    MTRPC_EXT_FRAGBLK = 0x10,
    /*! Extended RPC Fragmentation ACK frame */
    MTRPC_EXT_FRAGACK = 0x18,
    /*! Extended RPC Status frame */
    MTRPC_EXT_FRAGSTS = 0x20
} MtRpc_ExtType_t;

#ifdef __cplusplus
};
#endif

#endif /* MTRPC_H */
