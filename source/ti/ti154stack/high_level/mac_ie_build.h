/******************************************************************************

 @file  mac_ie_build.h

 @brief This file includes typedefs and functions required for building header
        and payload IE.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated

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

#ifndef MAC_IE_BUILD_H
#define MAC_IE_BUILD_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_high_level.h"
#include "mac_api.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* Payload IE byte 2 start position for individual fields */
#define MAC_PAYLOAD_GID_POS                       3 /* Group ID start Position */
#define MAC_PAYLOAD_TYPE_POS                      7 /* Type start Position */

/* First byte value of Payload Termination IE */
#define MAC_PAYLOAD_IE_TERM_FIRST_BYTE              0x00
/* Second byte value of Payload Termination IE */
#define MAC_PAYLOAD_IE_TERM_SECOND_BYTE_PAYIE       0xF8


/* Coexistence Specification IE, byte 1 */
#define MAC_COEXIST_IE_BEACON_ORDER_POS           0
#define MAC_COEXIST_IE_SUPERFRAME_ORDER_POS       4

/* Coexistence Specification IE, byte 2 */
#define MAC_COEXIST_IE_FINAL_CAP_SLOT_POS         0
#define MAC_COEXIST_IE_ENHANCED_BEACON_ORDER_POS  4

/* Coexistence Specification IE, byte 3 */
#define MAC_COEXIST_IE_OFFSET_TIME_SLOT_POS       0
#define MAC_COEXIST_IE_CAP_BACKOFF_OFFSET_POS     4

/* Internal -- use to indicate end of payload ie's */
#define MAC_PAYLOAD_IE_TERM                       0xFF

/* Internal -- use to indicate end of sub ie's */
#define MAC_PAYLOAD_SUB_IE_TERM                   0xFF

/* Internal -- use to indicate end of header ie's */
#define MAC_HEADER_IE_TERM                        0xFF

/* Coexistence IE content length */
#define MAC_PAYLOAD_MLME_SUB_IE_COEXIST_LEN         5

/* ------------------------------------------------------------------------------------------------
 *                                           Functions
 * ------------------------------------------------------------------------------------------------
 */

/**
 * @brief This function builds and inserts header ies in the
 *        transmit buffer.
 *
 * @param pBuf           pointer to the structure containing the
 *                       transmit message buffer.
 * @param pHeaderIes     pointer to the list of header ie's that
 *                       needs to be built and inserted in the
 *                       transmit buffer.
 *
 * @return uint8         returns MAC_SUCCESS if successful.
 *                       MAC_NO_DATA if it fails to build the
 *                       requested IE.
 */
MAC_INTERNAL_API uint8
macBuildHeaderIes( macTx_t* pBuf, headerIeInfo_t *pHeaderIes );

/**
 * @brief This function builds and inserts payload ies in the
 *        transmit buffer.
 *
 * @param pBuf           pointer to the structure containing the
 *                       transmit message buffer.
 * @param pPayloadIeInfo pointer to the list of payload ie's
 *                       that needs to be built and inserted in
 *                       the transmit buffer.
 * @param includeTermIe  if TRUE, payload termination IE will be included else not.
 * @param ieIdOnly       if TRUE no IE content will be populated and the length
 *                       field will be set to zero.
 *
 * @return uint8         returns MAC_SUCCESS if successful.
 *                       MAC_NO_RESOURCES if it fails to build the
 *                       requested IE.
 */
MAC_INTERNAL_API uint8
macBuildPayloadIes( macTx_t* pBuf, payloadIeInfo_t *pPayloadIeInfo, bool includeTermIe, bool ieIdOnly );

/**
 * @brief Builds the MLME payload co-existence sub IE.
 *
 * @param pBuf      pointer to the structure containing the transmitpBuf
 *                  message buffer.
 * @param ieIdOnly  if TRUE no IE content will be populated and the length
 *                  field will be set to zero.
 *
 * @return uint16 returns the length of the IE if successful,
 *         otherwise zero.
 */
uint16 macBuildPayloadCoexistIe( macTx_t* pBuf, bool ieIdOnly );

#endif /* MAC_IE_BUILD_H */
