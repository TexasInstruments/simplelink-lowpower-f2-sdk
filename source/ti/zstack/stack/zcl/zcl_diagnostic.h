/**************************************************************************************************
  Filename:       zcl_diagnostic.h
  Revised:        $Date: 2014-03-13 11:21:25 -0700 (Thu, 13 Mar 2014) $
  Revision:       $Revision: 37667 $

  Description:    This file contains the ZCL Diagnostic definitions.


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

#ifndef ZCL_DIAGNOSTIC_H
#define ZCL_DIAGNOSTIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef ZCL_DIAGNOSTIC

/******************************************************************************
 * INCLUDES
 */
#include "zcl.h"
#include "ti_zstack_config.h"


/******************************************************************************
 * CONSTANTS
 */

/***************************************/
/***  Diagnostic Cluster Attributes ***/
/***************************************/

// Server Attributes
#define ATTRID_DIAGNOSTIC_NUMBER_OF_RESETS                            0x0000  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES                    0x0001  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_MAC_RX_BCAST                                0x0100  // O, R, uint32_t
#define ATTRID_DIAGNOSTIC_MAC_TX_BCAST                                0x0101  // O, R, uint32_t
#define ATTRID_DIAGNOSTIC_MAC_RX_UCAST                                0x0102  // O, R, uint32_t
#define ATTRID_DIAGNOSTIC_MAC_TX_UCAST                                0x0103  // O, R, uint32_t
#define ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY                          0x0104  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL                           0x0105  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_RX_BCAST                                0x0106  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_TX_BCAST                                0x0107  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_RX_UCAST                                0x0108  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_TX_UCAST_SUCCESS                        0x0109  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_TX_UCAST_RETRY                          0x010A  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_TX_UCAST_FAIL                           0x010B  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_ROUTE_DISC_INITIATED                        0x010C  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_NEIGHBOR_ADDED                              0x010D  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_NEIGHBOR_REMOVED                            0x010E  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_NEIGHBOR_STALE                              0x010F  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_JOIN_INDICATION                             0x0110  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_CHILD_MOVED                                 0x0111  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_NWK_FC_FAILURE                              0x0112  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_FC_FAILURE                              0x0113  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_UNAUTHORIZED_KEY                        0x0114  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_NWK_DECRYPT_FAILURES                        0x0115  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_APS_DECRYPT_FAILURES                        0x0116  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES             0x0117  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_RELAYED_UCAST                               0x0118  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED              0x0119  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT                  0x011A  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT      0x011B  // O, R, uint16_t
#define ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI                            0x011C  // O, R, uint8_t
#define ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI                           0x011D  // O, R, int8_t

// Server Attribute Defaults
#define ATTR_DEFAULT_DIAGNOSTIC_NUMBER_OF_RESETS                            0
#define ATTR_DEFAULT_DIAGNOSTIC_PERSISTENT_MEMORY_WRITES                    0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_RX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_RX_UCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_UCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_UCAST_RETRY                          0
#define ATTR_DEFAULT_DIAGNOSTIC_MAC_TX_UCAST_FAIL                           0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_RX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_BCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_RX_UCAST                                0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_UCAST_SUCCESS                        0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_UCAST_RETRY                          0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_TX_UCAST_FAIL                           0
#define ATTR_DEFAULT_DIAGNOSTIC_ROUTE_DISC_INITIATED                        0
#define ATTR_DEFAULT_DIAGNOSTIC_NEIGHBOR_ADDED                              0
#define ATTR_DEFAULT_DIAGNOSTIC_NEIGHBOR_REMOVED                            0
#define ATTR_DEFAULT_DIAGNOSTIC_NEIGHBOR_STALE                              0
#define ATTR_DEFAULT_DIAGNOSTIC_JOIN_INDICATION                             0
#define ATTR_DEFAULT_DIAGNOSTIC_CHILD_MOVED                                 0
#define ATTR_DEFAULT_DIAGNOSTIC_NWK_FC_FAILURE                              0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_FC_FAILURE                              0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_UNAUTHORIZED_KEY                        0
#define ATTR_DEFAULT_DIAGNOSTIC_NWK_DECRYPT_FAILURES                        0
#define ATTR_DEFAULT_DIAGNOSTIC_APS_DECRYPT_FAILURES                        0
#define ATTR_DEFAULT_DIAGNOSTIC_PACKET_BUFFER_ALLOCATE_FAILURES             0
#define ATTR_DEFAULT_DIAGNOSTIC_RELAYED_UCAST                               0
#define ATTR_DEFAULT_DIAGNOSTIC_PHY_TO_MAC_QUEUE_LIMIT_REACHED              0
#define ATTR_DEFAULT_DIAGNOSTIC_PACKET_VALIDATE_DROP_COUNT                  0
#define ATTR_DEFAULT_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT      0
#define ATTR_DEFAULT_DIAGNOSTIC_LAST_MESSAGE_LQI                            0
#define ATTR_DEFAULT_DIAGNOSTIC_LAST_MESSAGE_RSSI                           0

/*******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * FUNCTION MACROS
 */

/******************************************************************************
 * VARIABLES
 */

/******************************************************************************
 * FUNCTIONS
 */
extern uint8_t zclDiagnostic_InitStats( void );

extern uint32_t zclDiagnostic_ClearStats( bool clearNV );

extern ZStatus_t zclDiagnostic_GetStatsAttr( uint16_t attributeId, uint32_t *attrValue, uint16_t *dataLen );

extern ZStatus_t zclDiagnostic_ReadWriteAttrCB( uint16_t clusterId, uint16_t attrId, uint8_t oper,
                                                uint8_t *pValue, uint16_t *pLen );

extern uint8_t zclDiagnostic_RestoreStatsFromNV( void );

extern uint32_t zclDiagnostic_SaveStatsToNV( void );

/*
 * Register for callbacks from this cluster library
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif // ZCL_DIAGNOSTIC

#endif /* ZCL_DIAGNOSTIC_H */
