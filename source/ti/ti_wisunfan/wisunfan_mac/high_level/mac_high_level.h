/******************************************************************************

 @file  mac_high_level.h

 @brief Contains interfaces shared between high and low level MAC.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2006-2025, Texas Instruments Incorporated

 All rights reserved not granted herein.
 Limited License.

 Texas Instruments Incorporated grants a world-wide, royalty-free,
 non-exclusive license under copyrights and patents it now or hereafter
 owns or controls to make, have made, use, import, offer to sell and sell
 ("Utilize") this software subject to the terms herein. With respect to the
 foregoing patent license, such license is granted solely to the extent that
 any such patent is necessary to Utilize the software alone. The patent
 license shall not apply to any combinations which include this software,
 other than combinations with devices manufactured by or for TI ("TI
 Devices"). No hardware patent is licensed hereunder.

 Redistributions must preserve existing copyright notices and reproduce
 this license (including the above copyright notice and the disclaimer and
 (if applicable) source code license limitations below) in the documentation
 and/or other materials provided with the distribution.

 Redistribution and use in binary form, without modification, are permitted
 provided that the following conditions are met:

   * No reverse engineering, decompilation, or disassembly of this software
     is permitted with respect to any software provided in binary form.
   * Any redistribution and use are licensed by TI for use only with TI Devices.
   * Nothing shall obligate TI to provide you with source code for the software
     licensed and provided to you in object code.

 If software source code is provided to you, modification and redistribution
 of the source code are permitted provided that the following conditions are
 met:

   * Any redistribution and use of the source code, including any resulting
     derivative works, are licensed by TI for use only with TI Devices.
   * Any redistribution and use of any object code compiled from the source
     code and any resulting derivative works, are licensed by TI for use
     only with TI Devices.

 Neither the name of Texas Instruments Incorporated nor the names of its
 suppliers may be used to endorse or promote products derived from this
 software without specific prior written permission.

 DISCLAIMER.

 THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/

#ifndef MAC_HIGH_LEVEL_H
#define MAC_HIGH_LEVEL_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "timac_api.h"
#include "mac_ie_parse.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */
#ifndef MAC_INTERNAL_API
#define MAC_INTERNAL_API
#endif

/* Call macPwrVote() only outside swi context */
#define MAC_PWR_VOTE_NO_SWI(x)        do { if (!bInSwi) MAP_macPwrVote(x, FALSE); } while (0);

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* RX flag masks */
#define MAC_RX_FLAG_VERSION           0x03    /* received frame's version */
#define MAC_RX_FLAG_ACK_PENDING       0x04    /* outgoing ACK has pending bit set */
#define MAC_RX_FLAG_SECURITY          0x08    /* received frame has security bit set */
#define MAC_RX_FLAG_PENDING           0x10    /* received frame has pending bit set */
#define MAC_RX_FLAG_ACK_REQUEST       0x20    /* received frame has ack request bit set */
#define MAC_RX_FLAG_INTRA_PAN         0x40    /* received frame has intra pan fcf bit set */
#define MAC_RX_FLAG_CRC_OK            0x80    /* received frame CRC OK bit */

/* Enhanced Beacon Request offsets for RX */
#define MAC_RX_EBR_EB_FILTER_IE_HEADER_LEN 0x02 /* received frame has 2 bytes
                                                 * for these - Type, ID and len
                                                 */

/* Enhanced Beacon Request payload bit position masks */
#define MAC_RX_EBR_PERMIT_JOINING_ON      0x01
#define MAC_RX_EBR_INCLUDE_LINK_QUALITY   0x02
#define MAC_RX_EBR_INCLUDE_PERCENT_FILTER 0x04

/* Enhanced Beacon Request End of IE List indication as per Table 4b of draft
 * for IEEE 802.15.4e
 *
 * Payload ID name space has terminator byte = 0x0F (ID) (bits 1,2,3,4)
 * Type of IE = 0x01
 */
#define MAC_RX_EBR_IE_PAYLOAD_LIST_TERMINATOR        0x1F
#define MAC_RX_EBR_IE_PAYLOAD_LIST_TERMINATOR_IE_LEN 0x03

/* FH Frame Types */
#define MAC_FH_FRAME_PAN_ADVERT         0x00    /* WiSUN PAN advertisement */
#define MAC_FH_FRAME_PAN_ADVERT_SOLICIT 0x01    /* WiSUN PAN advertisement solicit */
#define MAC_FH_FRAME_PAN_CONFIG         0x02    /* WiSUN PAN config */
#define MAC_FH_FRAME_PAN_CONFIG_SOLICIT 0x03    /* WiSUN PAN config solicit */
#define MAC_FH_FRAME_DATA               0x04    /* WiSUN Data frame */
#define MAC_FH_FRAME_ACK                0x05    /* WiSUN Ack frame */
#define MAC_FH_FRAME_EAPOL              0x06    /* WiSUN EAPOL frame */
#define MAC_FH_FRAME_INVALID            0xFF    /* Internal: WiSUN Invalid frame */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Structure for internal data tx */
typedef struct
{
  macEventHdr_t     hdr;
  sData_t           msdu;
  macTxIntData_t    internal;
  ApiMac_sec_t          sec;
} macTx_t;

struct _macHeaderIeInfo;
struct _macPayloadIeInfo;

/* Structure for internal data rx */
typedef struct _macRx
{
  macEventHdr_t       hdr;
  sData_t             msdu;
  macRxIntData_t      internal;
  ApiMac_sec_t        sec;
  macDataInd_t        mac;
  sData_t             mhr;
  uint8               payloadIePresent;
  uint8*              pHdrIes;
  uint16              headerIeLen;
  struct _macHeaderIeInfo*  pHeaderIes;
  struct _macPayloadIeInfo* pPayloadIes;
} macRx_t;

/* Function pointer for the 16 byte random seed callback */
typedef void (*macRNGFcn_t )(uint8* seed);

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* functions located in mac_data.c */
MAC_INTERNAL_API uint8 *macDataRxMemAlloc(uint16 len);
MAC_INTERNAL_API uint8 macDataRxMemFree(uint8 **pMsg);
MAC_INTERNAL_API uint16 macDataTxTimeAvailable(void);

/* functions located in mac_pwr.c */
MAC_INTERNAL_API void macPwrVote(bool pwrUp, bool bypassRAT);

/**************************************************************************************************
*/

#endif /* MAC_HIGH_LEVEL_H */
