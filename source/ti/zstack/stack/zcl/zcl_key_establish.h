/**************************************************************************************************
  Filename:       zcl_key_establish.h
  Revised:        $Date: 2014-11-06 23:59:26 -0800 (Thu, 06 Nov 2014) $
  Revision:       $Revision: 41038 $

  Description:    This file contains the ZCL (Smart Energy) Key Establishment definitions.


  Copyright (c) 2019, Texas Instruments Incorporated
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
**************************************************************************************************/
#ifndef ZCL_KEY_ESTABLISH_H
#define ZCL_KEY_ESTABLISH_H

#ifdef __cplusplus
extern "C"
{
#endif


/**************************************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "rom_jt_154.h"
#include "zcl.h"
#include "ti_zstack_config.h"


/**************************************************************************************************
 * CONSTANTS
 */

// KE task endpoint
#define ZCL_KE_ENDPOINT  10

// ZCL Attribute constants
#define ATTRID_KE_SUITE  0x0000
#define ZCL_KE_SUITE_1   0x0001
#define ZCL_KE_SUITE_2   0x0002

// ZCL_KE_NOTIFY_STATUS
#define ZCL_KE_NOTIFY_SUCCESS         0x00 // Key establishment successful
#define ZCL_KE_NOTIFY_TIMEOUT         0x01 // Timeout
#define ZCL_KE_NOTIFY_TERMINATE_RCVD  0x02 // Terminate command recieved from partner
#define ZCL_KE_NOTIFY_TERMINATE_SENT  0x03 // Terminate command sent to partner
#define ZCL_KE_NOTIFY_BUSY            0x04 // Client/server connections busy OR no resources
#define ZCL_KE_NOTIFY_NO_EXT_ADDR     0x05 // Partner extended address not found
#define ZCL_KE_NOTIFY_BAD_SUITE       0x06 // Suite not supported on device
#define ZCL_KE_NOTIFY_NO_CERTS        0x07 // No certs installed
#define ZCL_KE_NOTIFY_NO_EP_MATCH     0x08 // Partner's Match_Desc_rsp has no CBKE endpoint
#define ZCL_KE_NOTIFY_NO_SUITE_MATCH  0x09 // Partner's supported suites do not match device's

// ZCL_KE_TERMINATE_ERROR
#define ZCL_KE_TERMINATE_ERROR_NONE          0x00
#define ZCL_KE_TERMINATE_UNKNOWN_ISSUER      0x01
#define ZCL_KE_TERMINATE_BAD_KEY_CONFIRM     0x02
#define ZCL_KE_TERMINATE_BAD_MESSAGE         0x03
#define ZCL_KE_TERMINATE_NO_RESOURCES        0x04
#define ZCL_KE_TERMINATE_UNSUPPORTED_SUITE   0x05
#define ZCL_KE_TERMINATE_INVALID_CERTIFICATE 0x06


/**************************************************************************************************
 * TYPEDEFS
 */

// ZCL_KEY_ESTABLISH_IND message payload
typedef struct
{
  OsalPort_EventHdr hdr; //hdr::status -- see ZCL_KE_NOTIFY_STATUS
  uint16_t partnerNwkAddr;
  uint8_t terminateError; // see ZCL_KE_TERMINATE_ERROR
  uint16_t suites; // only valid if terminateError set
  uint8_t waitTime; // only valid if terminateError set
} zclKE_StatusInd_t;


/**************************************************************************************************
 * PUBLIC FUNCTIONS
 */

/**************************************************************************************************
 * @fn      zclKE_HdlGeneralCmd
 *
 * @brief   Handle general cluster commands in ZCL_STANDALONE mode.
 *
 * @param   pInMsg - incoming message to process
 *
 * @return  void
 */
extern void zclKE_HdlGeneralCmd( zclIncoming_t *pInMsg );

/**************************************************************************************************
 * @fn      zclKE_ECDSASignGetLen
 *
 * @brief   Returns length required for zclKE_ECDSASign "pOutBuf" field.
 *
 * @param   suite - selected security suite
 *
 * @return  uint8_t - length for zclKE_ECDSASign "pOutBuf" field
 */
extern uint8_t zclKE_ECDSASignGetLen( uint16_t suite );

/**************************************************************************************************
 * @fn      zclKE_ECDSASign
 *
 * @brief   Creates an ECDSA signature of a message digest.
 *
 * @param   suite - selected security suite
 * @param   pInBuf - input buffer
 * @param   inBufLen - input buffer length
 * @param   pOutBuf - output buffer ( length == zclKE_ECDSASignGetLen )
 *
 * @return  ZStatus_t - status
 */
extern ZStatus_t zclKE_ECDSASign( uint16_t suite, uint8_t *pInBuf, uint8_t inBufLen, uint8_t *pOutBuf );

/**************************************************************************************************
 * @fn      zclKE_Start
 *
 * @brief   Start key establishment with selected partner at the nwkAddr.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   partnerNwkAddr - partner network address
 * @param   transSeqNum - starting transaction sequence number
 *
 * @return  ZStatus_t - status
 */
extern ZStatus_t zclKE_Start( uint8_t taskID, uint16_t partnerNwkAddr, uint8_t transSeqNum );

/**************************************************************************************************
 * @fn      zclKE_StartDirect
 *
 * @brief   Start key establishment directly with partner at the pPartnerAddr.
 *
 * @param   taskID - OSAL task ID of requesting task
 * @param   pPartnerAddr - valid partner short address and end point
 * @param   transSeqNum - starting transaction sequence number
 * @param   suite - selected security suite
 *
 * @return  ZStatus_t - status
 */
extern ZStatus_t zclKE_StartDirect( uint8_t taskID, afAddrType_t *pPartnerAddr,
                                    uint8_t transSeqNum, uint16_t suite );

/**************************************************************************************************
 * @fn      zclKE_Init
 *
 * @brief   Initialization function for the application.
 *
 * @param   taskID - OSAL task ID
 *
 * @return  void
 */
void zclKE_Init( uint8_t taskID );

/**************************************************************************************************
 * @fn      zclKE_ProcessEvent
 *
 * @brief   Process all events for the task.
 *
 * @param   taskID - OSAL task ID
 * @param   events - OSAL event mask
 *
 * @return  uint16_t - OSAL events not process
 */
extern uint16_t zclKE_ProcessEvent( uint8_t taskID, uint16_t events );


/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_KEY_ESTABLISH_H */
