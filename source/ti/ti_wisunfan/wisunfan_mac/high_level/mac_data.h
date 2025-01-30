/******************************************************************************

 @file  mac_data.h

 @brief Internal interface file for the MAC data module.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2005-2025, Texas Instruments Incorporated

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

#ifndef MAC_DATA_H
#define MAC_DATA_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "mac_high_level.h"
#include "mac_main.h"

#if defined(TIMAC_ROM_IMAGE_BUILD)
#include "icall_osal_rom_jt.h"
#else
#include "icall_osal_map_direct.h"
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */
#ifdef FEATURE_WISUN_SUPPORT
/* Internal frame type values. */
#define MAC_INTERNAL_GTS_REQ                 0x03
#define MAC_INTERNAL_ENH_ACK                 0x04
#define MAC_INTERNAL_EDFE_EFRM               0x05
#define MAC_INTERNAL_BEACON                  0x00
#define MAC_INTERNAL_DATA                    0x01
#define MAC_INTERNAL_ASSOC_REQ               0x02
#define MAC_INTERNAL_ASSOC_RSP               0x03
#define MAC_INTERNAL_DISASSOC_NOTIF          0x04
#define MAC_INTERNAL_DATA_REQ                0x05
#define MAC_INTERNAL_PAN_CONFLICT            0x06
#define MAC_INTERNAL_ORPHAN_NOTIF            0x07
#define MAC_INTERNAL_BEACON_REQ              0x08
#define MAC_INTERNAL_COORD_REALIGN           0x09
#define MAC_INTERNAL_COEXIST_EB_REQ          0x0B
#define MAC_INTERNAL_ENHANCED_BEACON_REQ     0x0C
#else
/* Internal frame type values. */
#define MAC_INTERNAL_BEACON                  0x00
#define MAC_INTERNAL_DATA                    0x01
#define MAC_INTERNAL_ASSOC_REQ               0x02
#define MAC_INTERNAL_ASSOC_RSP               0x03
#define MAC_INTERNAL_DISASSOC_NOTIF          0x04
#define MAC_INTERNAL_DATA_REQ                0x05
#define MAC_INTERNAL_PAN_CONFLICT            0x06
#define MAC_INTERNAL_ORPHAN_NOTIF            0x07
#define MAC_INTERNAL_BEACON_REQ              0x08
#define MAC_INTERNAL_COORD_REALIGN           0x09
#define MAC_INTERNAL_GTS_REQ                 0x0A
#define MAC_INTERNAL_COEXIST_EB_REQ          0x0B
#define MAC_INTERNAL_ENHANCED_BEACON_REQ     0x0C
#define MAC_INTERNAL_ENH_ACK                 0x0D
#define MAC_INTERNAL_EDFE_EFRM               0x0E
#endif

#define MAC_INTERNAL_DATA_UNICAST            0x15
#define MAC_INTERNAL_DATA_BROADCAST          0x25
#define MAC_INTERNAL_ASYNC                   0x35
#define MAC_INTERNAL_DATA_EDFE               0x45
#define MAC_INTERNAL_DATA_EDFE_IFRM          0x46
#define MAC_INTERNAL_DATA_EDFE_RFRM          0x47
#define MAC_INTERNAL_DATA_EDFE_FFRM          0x48
#define MAC_INTERNAL_DATA_EDFE_IDFRM         0x49
#define MAC_INTERNAL_DATA_EDFE_RDFRM         0x4a
/* Internal zero length data frame */
#define MAC_INTERNAL_ZERO_DATA        MAC_INTERNAL_DATA

/* Additional byte used by low level in tx buffer */
#define MAC_TX_OFFSET_LEN             1

/* Enhanced Beacon filter IE */
#define MAC_ATTRIB_ID_INCLUDE_MPMIE          0x40  /* MPM IE Attribute ID bit */
#define MAC_ATTRIB_ID_LEN_1_BYTE_VAL         0x08  /* Attribute ID field length value for 1 byte */


/* length of an ack frame in bytes */
#define MAC_LEN_ACK_FRAME           (MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN + MAC_FCS4_FIELD_LEN)
#define MAC_LEN_ACK_IEEE_FRAME      (MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN + MAC_FCS2_FIELD_LEN)
#define MAC_LEN_ACK_FRAME_FH        49

/* EDFE */
#define MAC_EDFE_MAX_RXFC           255
#define MAC_EDFE_MAX_FRAME          250
/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Data info type */
typedef struct
{
  MAP_osal_msg_q_t        txQueue;          /* transmit data queue */
  MAP_osal_msg_q_t        rxQueue;          /* receive data queue */
  uint16              duration;         /* duration of current frame */
  uint8               indirectCount;    /* number of indirect frames in tx queue */
  uint8               directCount;      /* number of direct frames in tx queue */
  uint8               rxCount;          /* number of frames in rx queue */
} macData_t;

/* general purpose data handling function type */
typedef void (*macDataTxFcn_t)(macTx_t *pMsg);

/* critical beacon handling function */
typedef void (*macRxBeaconCritical_t)(macRx_t *pBuf);

/* beacon tx complete function */
typedef void (*macTxBeaconComplete_t)(uint8 status);

/* tx frame retransmit function */
typedef void (*macTxFrameRetransmit_t)(void);

/* tx frame check schedule function */
typedef uint8 (*macDataCheckSched_t)(void);

/* tx frame check tx time function */
typedef uint16 (*macDataCheckTxTime_t)(void);

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */
#ifndef ENHACK_MALLOC
extern uint8 enhancedAckBuf[250];
#endif
/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

MAC_INTERNAL_API void macDataReset(void);
MAC_INTERNAL_API macTx_t *macAllocTxBuffer(uint8 cmd, ApiMac_sec_t *sec);
MAC_INTERNAL_API uint16 macFrameDuration(uint16 len, uint16 txOptions);
MAC_INTERNAL_API void macDataRetransmit(void);
MAC_INTERNAL_API uint8 macBuildDataFrame(macEvent_t *pEvent);
MAC_INTERNAL_API uint8 macCheckSched(void);
MAC_INTERNAL_API void macSetSched(macTx_t *pMsg);
MAC_INTERNAL_API void macDataTxComplete(macTx_t *pMsg);
MAC_INTERNAL_API void macDataTxSend(void);
MAC_INTERNAL_API void macDataTxEnqueue(macTx_t *pMsg);
MAC_INTERNAL_API void macDataSend(macEvent_t *pEvent);
MAC_INTERNAL_API void macApiDataReq(macEvent_t *pEvent);
MAC_INTERNAL_API void macDataRxInd(macEvent_t *pEvent);
MAC_INTERNAL_API uint8_t macCheckIndPacket(macRx_t  *pRxBuf);
#if defined(COMBO_MAC) || defined(FREQ_2_4G)
MAC_INTERNAL_API void macAutoPendMaintainSrcMatchTable(macTx_t *pCurr);
MAC_INTERNAL_API void macAutoPendAddSrcMatchTableEntry(macTx_t *pMsg);
#endif
/**************************************************************************************************
*/

#endif /* MAC_DATA_H */
