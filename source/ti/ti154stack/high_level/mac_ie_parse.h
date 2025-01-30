/******************************************************************************

 @file  mac_ie_parse.h

 @brief This file includes typedefs and functions required for MAC header and
        payload IE parsing.

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

#ifndef MAC_IE_PARSE_H
#define MAC_IE_PARSE_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "mac_high_level.h"

/* ------------------------------------------------------------------------------------------------
 *                                          Constants
 * ------------------------------------------------------------------------------------------------
 */
/**
 * Header IEs
 */
/* Header IE header Length -- 2 bytes */
#define MAC_HEADER_IE_HEADER_LEN                 2
/* Type value for header IE */
#define MAC_HEADER_IE_TYPE_VAL                   0

/* IE to terminate list of IEs with no payload IE */
#define MAC_INTERNAL_HEADER_IE_TERM_NO_PAYIE     0x7f
/* IE to terminate list of IEs followed by payload IE */
#define MAC_INTERNAL_HEADER_IE_TERM_WITH_PAYIE   0x7e

/* First byte value of Header Termination IE */
#define MAC_HEADER_IE_TERM_SECOND_BYTE           0x3F
/* Second byte value of Header Termination IE with Payload IE  */
#define MAC_HEADER_IE_TERM_FIRST_BYTE_PAYIE      0x00
/* Second byte value of Header Termination IE without Payload IE  */
#define MAC_HEADER_IE_TERM_FIRST_BYTE_NO_PAYIE   0x80

/* WiSUN header IE, element ID */
#define MAC_HEADER_IE_WISUN                      0x2A

/**
 * Payload IEs
 */
/* Type value for payload IE */
#define MAC_PAYLOAD_IE_TYPE_VAL                  1
/* Type value for payload IE */
#define MAC_PAYLOAD_IE_HEADER_LEN                2


/* Payload IE Group IDs */
#define MAC_PAYLOAD_IE_ESDU_GID                  0x00   /* Payload ESDU IE Group ID*/
#define MAC_PAYLOAD_IE_MLME_GID                  0x01   /* Payload MLME IE Group ID*/
#define MAC_PAYLOAD_IE_WISUN_GID                 0x04   /* Payload MLME IE Group ID*/
#define MAC_PAYLOAD_IE_TERM_GID                  0x0f   /* Payload Termination IE Group ID*/

/* MLME Sub IEs -- short format */
#define MAC_PAYLOAD_MLME_SUB_IE_EBEACON_FILTER   0x1E   /* Enhanced beacon filter IE */
#define MAC_PAYLOAD_MLME_SUB_IE_COEXIST          0x21   /* Coexistence IE */
#define MAC_PAYLOAD_MLME_SUB_IE_SUN_DEV_CAP      0x22   /* SUN Device capabilities IE */
#define MAC_PAYLOAD_MLME_SUB_IE_SUN_FSK_GEN_PHY  0x23   /* SUN FSK Generic PHY IE */
#define MAC_PAYLOAD_MLME_SUB_IE_MODE_SWITCH      0x24   /* Mode Switch Parameter IE */

/* Payload IE's SubIe Header length */
#define MAC_PAYLOAD_SUB_IE_HEADER_LEN    2
/* Payload IE's SubIe Type short value  */
#define MAC_PAYLOAD_SUB_IE_ID_TYPE_SHORT 0
/* Payload IE's SubIe Type Long value */
#define MAC_PAYLOAD_SUB_IE_ID_TYPE_LONG  1


/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
/* Macros for parsing Header IE header fields */
#define MAC_HEADER_IE_OFFSET                     0
#define MAC_HEADER_IE_CONTENT_LEN(p)             ((p)[MAC_HEADER_IE_OFFSET+0] & 0x7f)
#define MAC_HEADER_IE_ELEMENT_ID(p)              ((((p)[MAC_HEADER_IE_OFFSET+0] >> 7) & 0x01) + \
                                                  (((p)[MAC_HEADER_IE_OFFSET+1] << 1) & 0xfe))
#define MAC_HEADER_IE_TYPE(p)                    (((p)[MAC_HEADER_IE_OFFSET+1] >> 7) & 0x01)

/* Macros for parsing Payload IE header fields */
#define MAC_PAYLOAD_IE_OFFSET                    0
#define MAC_PAYLOAD_IE_SUBIE_OFFSET              0
#define MAC_PAYLOAD_IE_TYPE(p)                   (((p)[MAC_PAYLOAD_IE_OFFSET+1] >> 7) & 0x01)
#define MAC_PAYLOAD_IE_GROUP_ID(p)               (((p)[MAC_PAYLOAD_IE_OFFSET+1] >> 3) & 0x0f)
#define MAC_PAYLOAD_IE_CONTENT_LEN(p)            (((p)[MAC_PAYLOAD_IE_OFFSET+0] & 0x00ff) + \
                                                  (((p)[MAC_PAYLOAD_IE_OFFSET+1] & 0x0007)  \
                                                    << 8))

/* Macros for parsing SubIe of Payload IEs */
#define MAC_PAYLOAD_SHORT_SUBIE_LEN(p)           ((p)[MAC_PAYLOAD_IE_SUBIE_OFFSET+0])
#define MAC_PAYLOAD_LONG_SUBIE_LEN(p)            (((p)[MAC_PAYLOAD_IE_OFFSET+0] & 0x00ff) + \
                                                   (((p)[MAC_PAYLOAD_IE_OFFSET+1] & 0x0007)  \
                                                    << 8))
#define MAC_PAYLOAD_SUBIE_TYPE(p)                (((p)[MAC_PAYLOAD_IE_SUBIE_OFFSET+1] >> 7) \
                                                  & 0x01)
#define MAC_PAYLOAD_SHORT_SUBIE_ID(p)            ((p)[MAC_PAYLOAD_IE_SUBIE_OFFSET+1] & 0x7f)
#define MAC_PAYLOAD_LONG_SUBIE_ID(p)             (((p)[MAC_PAYLOAD_IE_OFFSET+1] >> 3) & 0x0f)


/*--------------------------------------------------------------------------------------------------
*           Typedef
*---------------------------------------------------------------------------------------------------
*/

/* Type definition for mac header information element */
typedef struct _macHeaderIeInfo
{
  struct _macHeaderIeInfo *pNext; /* pointer to the next element */
  uint8  ieType;                  /* 1 bit, Type (always short form, value 0) */
  uint8  ieElementId;             /* 8 bits, Element ID */
  uint8  ieContentLen;            /* 7 bits, IE Content Length */
  uint8* ieContentPtr;            /* max size 127 bytes */

} macHeaderIeInfo_t;

/* Type definition for the Payload information element */
typedef struct _macPayloadIeInfo
{
  struct _macPayloadIeInfo *pNext;          /* pointer to the next element */
  uint8                    ieType;          /* 1 bit, type of payload IE */
  uint8                    ieGroupId;       /* 7 bits, group id */
  uint16                   ieContentLen;    /* max 16 bits, IE Content Length */
  uint8*                   ieContentPtr;    /* max size 2047 bytes */

} macPayloadIeInfo_t;


struct _macRx;
/*--------------------------------------------------------------------------------------------------
*           Functions
*---------------------------------------------------------------------------------------------------
*/

/*!
 * @brief Parses the header information element.
 *        This function creates the link list of the header IEs.
 *        The caller is responsible to release the memory for
 *        the same. It also updates the payload pointer and the
 *        length.
 *
 * @param pMsg pointer to the macRx_t structure containing the
 *             payload data.
 *
 * @return returns MAC_SUCCESS if successful in parsing the
 *         payload ie's. MAC_NO_DATA if it does not find any
 *         payload ie's. MAC_NO_RESOURCES if memory allocation
 *         fails.
 */
uint8 macParseHeaderIes(struct _macRx *pMsg);

/*!
 * @brief Parses the payload information element.
 *        This function creates the link list of the Payload IE.
 *        The caller is responsible to release the memory for
 *        the same. It also updates the payload pointer and the
 *        length.
 *
 * @param pMsg pointer to the macRx_t structure containing the
 *             payload data.
 *
 * @return returns MAC_SUCCESS if successful in parsing the
 *         payload ie's. MAC_NO_DATA if it does not find any
 *         payload ie's. MAC_NO_RESOURCES if memory allocation
 *         fails.
 */
uint8 macParsePayloadIes(struct _macRx *pMsg );

/*!
 * @brief Updates the header IE pointer if header IE is present in macRx_t
 *        structure with the start position of header IE.
 *        It also updates the headerIeLen fields.
 *        Call this function only if the payload IE to precess exists.
 *
 * @param pMsg pointer to the macRx_t containing the incoming decrypted message.
 * @return None
 */
void
macUpdateHeaderIEPtr(struct _macRx  *pMsg);

/*!
 * @brief Updates the payload IE pointer if payload IE's are present in macRx_t
 *        structure with the start position of payload IE in the buffer.
 *        It also updates the payloadIeLen fields.
 *
 * @param pMsg pointer to the macRx_t containing the incoming decrypted message.
 * @return None
 */
void
macUpdatePayloadIEPtr(struct _macRx *pMsg);

/*!
 * @brief Cycles through the MLME IE content to find the Sub-IE indicated by the
 *        param mlmeSubIeType. If found set the value of the field pointed by
 *        pIePresent value to 1 and if the content of the IE is present returns
 *        the pointer to the content.
 *
 * @param pPayloadIe    pointer to the buffer containing the MLME IE content.
 * @param mlmeSubIeType Sub-IE to find within the MLME IE content.
 * @param pIePresent    pointer to the IE present field which is updated by this
 *                      function.
 *
 * @return uint8*       pointer to the Sub-IE content if present otherwise NULL.
 */
uint8* macMlmeSubIePresent( macPayloadIeInfo_t* pPayloadIe, uint8 mlmeSubIeType,
                            uint8* pIePresent );

/*!
 * @brief Frees the memory associated with each element in the link list
 * pointed by the pList
 *
 * @param pList pointer to the payload IE information element.
 *
 * @return None
 */
void macIeFreeList(macPayloadIeInfo_t* pList);

/*!
 * @brief Sets the value of the location pointed by the pIePresent parameter
 *        to 1 and returns the pointer to the start of the coexistence IE content
 *        if the coexistence is found in the list pointed by pPayloadIE param.
 *
 * @param pPayloadIe pointer to the payload IE list.
 * @param pIePresent pointer to the location of the iePresent field.
 *
 * @return uint8* pointer to the start of coexistence IE content if available
 *                otherwise NULL.
 */
uint8* macIeCoexistIePresent( macPayloadIeInfo_t* pPayloadIe, uint8* pIePresent );

#endif /* MAC_IE_PARSE_H */

