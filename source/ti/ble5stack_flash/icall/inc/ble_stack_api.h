/******************************************************************************

@file  ble_stack_api.h

@brief This file contains the BLE stack wrapper above ICall

Group: WCS, BTS
Target Device: cc13xx_cc26xx

******************************************************************************

 Copyright (c) 2013-2025, Texas Instruments Incorporated
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

#ifndef BLE_STACK_API_H
#define BLE_STACK_API_H

/*********************************************************************
 * INCLUDES
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <icall.h>
#include <bcomdef.h>
#include <icall_ble_api.h>

/*********************************************************************
* TYPEDEFS
*/
typedef ICall_EntityID          bleStack_entityId_t;
typedef ICall_Hdr               bleStack_msgHdt_t;
typedef ICall_Errno             bleStack_errno_t;

typedef struct {
  /**
   * Bits 0 to 4 indicate connectable, scannable, directed, scan response, and
   * legacy respectively
   */
  uint8_t  evtType;
  /// Public, random, public ID, random ID, or anonymous
  uint8_t  addrType;
  /// Address of the advertising device
  uint8_t  addr[B_ADDR_LEN];
  /// PHY of the primary advertising channel
  uint8_t  primPhy;
  /// PHY of the secondary advertising channel
  uint8_t  secPhy;
  /// SID (0x00-0x0f) of the advertising PDU. 0xFF means no ADI field in the PDU
  uint8_t  advSid;
  /// -127 dBm <= TX power <= 126 dBm
  int8_t   txPower;
  /// -127 dBm <= RSSI <= 20 dBm
  int8_t   rssi;
  /// Type of TargetA address in the directed advertising PDU
  uint8_t  directAddrType;
  /// TargetA address
  uint8_t  directAddr[B_ADDR_LEN];
  /// Periodic advertising interval. 0 means no periodic advertising.
  uint16_t periodicAdvInt;
  /// Length of the data
  uint16_t dataLen;
  /// Pointer to advertising or scan response data
  uint8_t  *pData;
} bleStk_GapScan_Evt_AdvRpt_t;

typedef union
{
    uint8_t advHandle;
    GapAdv_setTerm_t pSetTerm;
    GapAdv_scanReqReceived_t pScanReqRcv;
    GapAdv_truncData_t pTruncData;
} GapAdv_data_t;

// Structures/unions data types declarations
typedef union
{
    GapScan_Evt_End_t pScanDis;
    bleStk_GapScan_Evt_AdvRpt_t pAdvReport;
} GapScan_data_t;

typedef void (*pfnBleStkAdvCB_t) (uint32_t event,
                                  GapAdv_data_t *pBuf,
                                  uint32_t *arg);
typedef void (*bleStk_pfnGapScanCB_t) (uint32_t event,
                                       GapScan_data_t *pBuf,
                                     uint32_t *arg);

/*********************************************************************
 * MACROS
 */
#define bleStack_malloc         ICall_malloc
#define bleStack_free           ICall_free
#define bleStack_mallocMsg      ICall_mallocMsg
#define bleStack_freeMsg        ICall_freeMsg
#define BLE_STACK_ERRNO_SUCCESS ICALL_ERRNO_SUCCESS

/*********************************************************************
 * FUNCTIONS
 */
#ifdef ICALL_NO_APP_EVENTS
bleStack_errno_t bleStack_register(uint8_t *selfEntity, appCallback_t appCallback);
#endif

extern void      bleStack_createTasks();

// Stack Init
extern bStatus_t bleStack_initGap(uint8_t role, ICall_EntityID appSelfEntity, bleStk_pfnGapScanCB_t scanCallback, uint16_t paramUpdateDecision);
extern bStatus_t bleStack_initGapBond(gapBondParams_t *pGapBondParams, void *bleApp_bondMgrCBs);
extern bStatus_t bleStack_initGatt(uint8_t role, ICall_EntityID appSelfEntity, uint8_t *pAttDeviceName);

// Advertisement
extern bStatus_t bleStk_initAdvSet(pfnBleStkAdvCB_t advCallback, uint8_t *advHandle,
                                          GapAdv_eventMaskFlags_t eventMask,
                                          GapAdv_params_t *advParams,
                                          uint16_t advDataLen ,uint8_t *advData,
                                          uint16_t scanRespDataLen, uint8_t *scanRespData);
extern void bleStk_getDevAddr(uint8_t wantIA, uint8_t *pAddr);

// Scan
extern bStatus_t bleStk_scanInit(bleStk_pfnGapScanCB_t bleStk_bleApp_scanCB,
                                       GapScan_EventMask_t eventMask,
                                       uint8_t primPhys, uint8_t scanType, uint16_t scanInterval, uint16_t scanWindow,
                                       uint16_t advReportFields, uint8_t defaultScanPhy, uint8_t scanDupFilter,
                                       uint16_t scanFilterPduType,
                                       uint16_t scanMinConnInterval, uint16_t scanMaxConnInterval);
status_t bleStk_GapScan_registerCb(bleStk_pfnGapScanCB_t cb, uint32_t * arg);

#ifdef __cplusplus
}
#endif

#endif /* BLE_STACK_API_H */
