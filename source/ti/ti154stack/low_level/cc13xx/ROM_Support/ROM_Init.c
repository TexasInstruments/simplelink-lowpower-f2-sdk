/******************************************************************************

 @file  ROM_Init.c

 @brief This file contains the entry point for the BLE ROM.

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2009-2025, Texas Instruments Incorporated

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

/*******************************************************************************
 * INCLUDES
 */

#include "hal_types.h"
#include "hw_types.h"
#include "ROM_Init.h"
//
#include "OSAL.h"
#include "OSAL_Memory.h"
#include "OSAL_PwrMgr.h"
#include "osal_bufmgr.h"
#include "OSAL_Tasks.h"

#include "interrupt.h"
#include "hal_assert.h"

// MAC MCU
#include "mac_mcu.h"

// Low Level MAC
#include "mac_low_level.h"
#include "mac_mem.h"

// High Level MAC
#include "mac_main.h"
#include "mac_data.h"
#include "mac_timer.h"   
#include "mac_device.h"
#include "mac_coord.h"
#include "mac_mgmt.h"
#include "mac_beacon_coord.h"
#include "mac_pwr.h"
#include "mac_scan.h"
#include "mac_device.h"
#include "mac_beacon_device.h"
#include "mac_beacon.h"
#include "mac_security.h"
#include "mac_pib.h"
#include "mac_security_pib.h"

extern const macCfg_t macCfg;
extern void macAssociateCnf(uint8 status, uint16 addr);
extern void macBeaconBattLifeCallback(uint8 param);
extern uint8 macBeaconCheckStartTime(macEvent_t *pEvent);
extern void macBeaconPeriodCallback(uint8 param);
extern void macBeaconSetPrepareTime(void);
extern void macBeaconStopTrack(void);
extern void macBeaconSyncLoss(void);
extern void macBeaconTxCallback(uint8 param);
extern uint8 macBlacklistChecking( keyDescriptor_t *pKeyDescriptor,
                                   uint8 *deviceLookupData,
                                   uint8 deviceLookupSize,
                                   deviceDescriptor_t **ppDeviceDescriptor,
                                   keyDeviceDescriptor_t **ppKeyDeviceDescriptor );
extern void macBroadcastPendCallback(uint8 param);
extern uint8 macBuildAssociateReq(macEvent_t *pEvent);
extern uint8 *macBuildPendAddr(uint8 *p, uint8 maxPending, bool *pBroadcast);
extern uint8 macCcmStarInverseTransform( uint8    *pKey,
                                         uint32   frameCounter,
                                         uint8    securityLevel,
                                         uint8    *pAData,
                                         uint16   aDataLen,
                                         uint8    *pCData,
                                         uint16   cDataLen,
                                         uint8    *pExtAddr );
extern uint8 macCheckPendAddr(uint8 pendAddrSpec, uint8 *pAddrList);
extern void macDataTxDelayCallback(uint8 param);
extern uint8 macDeviceDescriptorLookup( deviceDescriptor_t *deviceDescriptor,
                                        uint8 *deviceLookupData,
                                        uint8 deviceLookupSize );
extern uint8 macIncomingFrameSecurityMaterialRetrieval( macRx_t *pMsg,
                                                        keyDescriptor_t **ppKeyDescriptor,
                                                        deviceDescriptor_t **ppDeviceDescriptor,
                                                        keyDeviceDescriptor_t **ppKeyDeviceDescriptor);
extern uint8 macIncomingKeyUsagePolicyChecking( keyDescriptor_t *keyDescriptor,
                                                uint8 frameType,
                                                uint8 cmdFrameId );
extern void macIncomingNonSlottedTx(void);
extern uint8 macIncomingSecurityLevelChecking( uint8 securityLevel,
                                               uint8 frameType,
                                               uint8 cmdFrameId );
extern void macIndirectMark(macTx_t *pMsg);
extern void macIndirectRequeueFrame(macTx_t *pMsg);
extern bool macIndirectSend(sAddr_t *pAddr, uint16 panId);
extern void macIndirectTxFrame(macTx_t *pMsg);
extern uint8 macKeyDescriptorLookup( uint8 *pLookupData, uint8 lookupDataSize, keyDescriptor_t **ppKeyDescriptor );
extern void macMgmtReset(void);
extern uint8 macOutgoingFrameKeyRetrieval( macTx_t  *pBuf,
                                           sAddr_t  *pDstAddr,
                                           uint16   dstPanId,
                                           uint8    **ppKey,
                                           uint32   **ppFrameCounter );
extern uint8 macPendAddrLen(uint8 *maxPending);
extern void macPollCnf(uint8 status);
extern void macRxBeaconCritical(macRx_t *pBuf);
extern void macScanCnfInit(macMlmeScanCnf_t *pScanCnf, macEvent_t *pEvent);
extern void macStartBeaconPrepareCallback(uint8 param);
extern void macSyncTimeoutCallback(uint8 param);
extern void macTimerAddTimer(macTimer_t *pTimer, macTimerHeader_t *pList);
extern void macTimerRecalcUnaligned(int32 adjust, macTimer_t *pTimer);
extern uint8 macTimerRemoveTimer(macTimer_t *pTimer, macTimerHeader_t *pList);
extern void macTimerUpdateBackoffTimer(void);
extern void macTrackPeriodCallback(uint8 param);
extern void macTrackStartCallback(uint8 param);
extern void macTrackTimeoutCallback(uint8 param);
extern uint8 MAC_MlmeGetReqSize( uint8 pibAttribute );
extern uint8 MAC_MlmeGetSecurityReqSize( uint8 pibAttribute );


// Security
#include "zaesccm_api.h"

// CC26XX
#include "hw_sysctl.h"
#include "ioc.h"
#include "hw_ioc.h"
#include "mb.h"

#ifdef OSAL_PORT2TIRTOS
#include <ICall.h>
#endif

/*******************************************************************************
 * EXTERNS
 */

// RAM address containing a pointer to the R2F flash jump table base address.
// Note: This linker imported symbol is treated as a variable by the compiler.

// ROM base address where the ROM's C runtime routine is expected.
extern uint32 ROM_BASE_ADDR;

// RAM base address of a table a pointers that are used by ROM and which are
// initialized by ROM_Init.
extern uint32 RAM_BASE_ADDR;

// Checksum start/end address and value for ROM and Common ROM.
extern const uint32 __checksum_begin;
extern const uint32 __checksum_end;

// Function pointers used by the Flash software (provided by RTOS).
extern ICall_Dispatcher ICall_dispatcher;
extern ICall_EnterCS    ICall_enterCriticalSection;
extern ICall_LeaveCS    ICall_leaveCriticalSection;

// calculate checksum routine
extern uint16 slow_crc16( uint16 crc, uint8 *pAddr, uint32 len );
extern uint8  validChecksum( const uint32 *beginAddr, const uint32 *endAddr );

/*******************************************************************************
 * PROTOTYPES
 */

/*******************************************************************************
 * MACROS
 */

// ICall function pointers and R2F/R2R flash JT pointers for ROM
#define icallRomDispatchPtr    (uint32 *)(&RAM_BASE_ADDR+0)
#define icallRomEnterCSPtr     (uint32 *)(&RAM_BASE_ADDR+1)
#define icallRomLeaveCSPtr     (uint32 *)(&RAM_BASE_ADDR+2)
#define r2fRomPtr              (uint32 *)(&RAM_BASE_ADDR+3)
#define r2rRomPtr              (uint32 *)(&RAM_BASE_ADDR+4)

// Runtime Init code for Common ROM
#define RT_Init_ROM ((RT_Init_fp)&ROM_BASE_ADDR)

// Map IntDisable and IntEnable to driverLib ROM or Flash
#ifdef DRIVERLIB_NOROM
#define R2F_IntDisable          IntDisable
#define R2F_IntEnable           IntEnable 
#else
#define R2F_IntDisable          0x10001a1b
#define R2F_IntEnable           0x100019b7
#endif

/*******************************************************************************
 * CONSTANTS
 */

/*******************************************************************************
 * TYPEDEFS
 */

typedef void (*RT_Init_fp)(void);

/*******************************************************************************
 * LOCAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL VARIABLES
 */

// ROM-to-Flash Flash Jump Table
// Note: Any change here must accompany a change to R2F_FlashJT.h defines!

#pragma data_alignment=4
const uint32 R2F_Flash_JT[] =
{
  // ROM-to-Flash Functions
  (uint32)osal_memcpy,
  (uint32)IntMasterEnable,
  (uint32)IntMasterDisable,
  (uint32)osal_bm_free,
  (uint32)osal_mem_alloc,
  (uint32)osal_mem_free,
  (uint32)R2F_IntDisable,
  (uint32)R2F_IntEnable,
  (uint32)macBackoffTimerCount,
  (uint32)macBackoffTimerSetTrigger,
  (uint32)macBackoffTimerCancelTrigger,
  (uint32)halAssertHandler,
  (uint32)macBackoffTimerSetRollover,
  (uint32)macBackoffTimerSetCount,
  (uint32)macBackoffTimerRealign,
  (uint32)macBackoffTimerGetTrigger,
  (uint32)osal_memcmp, 
  (uint32)zaesccmDecryptAuth,
  (uint32)zaesccmAuthEncrypt,
  (uint32)macRadioStartScan,
  (uint32)&macRxEnable,
  (uint32)MAC_CbackEvent,
  (uint32)macRadioStopScan,
  (uint32)macRxDisable,
  (uint32)macSleep,
  (uint32)macSleepWakeUp,
  (uint32)macRadioRandomByte,
  (uint32)macRadioSetPanID,
  (uint32)macRadioSetShortAddr,
  (uint32)macRadioSetChannel,
  (uint32)macRadioSetIEEEAddr,
  (uint32)macRadioSetTxPower,
  (uint32)macLowLevelReset,
  (uint32)macRadioSetPanCoordinator,
  (uint32)macLowLevelResume,
  (uint32)macLowLevelYield,
  (uint32)macLowLevelInit,
  (uint32)osal_msg_receive,
  (uint32)osal_set_event,
  (uint32)osal_msg_allocate,
  (uint32)osal_msg_send,
  (uint32)osal_msg_dequeue,
  (uint32)&macTasksEvents,
  (uint32)macRxSoftEnable,
  (uint32)MAC_CbackCheckPending,
  (uint32)osal_msg_enqueue,
  (uint32)MAC_CbackQueryRetransmit,
  (uint32)macTxFrameRetransmit,
  (uint32)osal_msg_extract,
  (uint32)macTxFrame,
  (uint32)osal_msg_enqueue_max,
  (uint32)&macCfg,
  (uint32)osal_memset,
  (uint32)osal_msg_deallocate,
  (uint32)osal_msg_push,
  (uint32)macRxHardDisable,
  (uint32)&macTxSlottedDelay,
  (uint32)macMcuRandomByte,
  (uint32)macCanSleep,
  (uint32)osal_pwrmgr_task_state, 
  (uint32)macLowLevelDiags,
  (uint32)MAC_SrcMatchDeleteEntry,
  (uint32)MAC_SrcMatchAddEntry,    
  (uint32)MAC_SrcMatchAckAllPending, 
  (uint32)MAC_SrcMatchCheckAllPending,
};

#include "mac_api.h"

// ROM-to-ROM Flash Jump Table
// Note: Any change here must accompany a change to R2R_FlashJT.h defines!

#pragma data_alignment=4
#pragma diag_suppress=Pa128,Pe767
const uint32 R2R_Flash_JT[] =
{
  // MAC ROM-to-ROM Functions
  (uint32)MAC_Init,                                      // R2R_JT_OFFSET(0))
  (uint32)MAC_InitBeaconCoord,                           // R2R_JT_OFFSET(1))
  (uint32)MAC_InitBeaconDevice,                          // R2R_JT_OFFSET(2))
  (uint32)MAC_InitCoord,                                 // R2R_JT_OFFSET(3))                            
  (uint32)MAC_InitDevice,                                // R2R_JT_OFFSET(4))          
  (uint32)MAC_McpsDataAlloc,                             // R2R_JT_OFFSET(5))       
  (uint32)MAC_McpsDataReq,                               // R2R_JT_OFFSET(6))         
  (uint32)MAC_McpsPurgeReq,                              // R2R_JT_OFFSET(7))        
  (uint32)MAC_MlmeAssociateReq,                          // R2R_JT_OFFSET(8))    
  (uint32)MAC_MlmeAssociateRsp,                          // R2R_JT_OFFSET(9))    
  (uint32)MAC_MlmeDisassociateReq,                       // R2R_JT_OFFSET(10))
  (uint32)MAC_MlmeGetPointerSecurityReq,                 // R2R_JT_OFFSET(11))
  (uint32)MAC_MlmeGetReq,                                // R2R_JT_OFFSET(12))
  (uint32)MAC_MlmeGetReqSize,                            // R2R_JT_OFFSET(13))
  (uint32)MAC_MlmeGetSecurityReq,                        // R2R_JT_OFFSET(14))
  (uint32)MAC_MlmeGetSecurityReqSize,                    // R2R_JT_OFFSET(15)) 
  (uint32)MAC_MlmeOrphanRsp,                             // R2R_JT_OFFSET(16))       
  (uint32)MAC_MlmePollReq,                               // R2R_JT_OFFSET(17))         
  (uint32)MAC_MlmeResetReq,                              // R2R_JT_OFFSET(18))        
  (uint32)MAC_MlmeScanReq,                               // R2R_JT_OFFSET(19))
  (uint32)MAC_MlmeSetReq,                                // R2R_JT_OFFSET(20))
  (uint32)MAC_MlmeSetSecurityReq,                        // R2R_JT_OFFSET(21)) 
  (uint32)MAC_MlmeStartReq,                              // R2R_JT_OFFSET(22))        
  (uint32)MAC_MlmeSyncReq,                               // R2R_JT_OFFSET(23))         
  (uint32)MAC_PwrMode,                                   // R2R_JT_OFFSET(24))             
  (uint32)MAC_PwrNextTimeout,                            // R2R_JT_OFFSET(25))      
  (uint32)MAC_PwrOffReq,                                 // R2R_JT_OFFSET(26))           
  (uint32)MAC_PwrOnReq,                                  // R2R_JT_OFFSET(27))            
  (uint32)MAC_RandomByte,                                // R2R_JT_OFFSET(28))          
  (uint32)MAC_ResumeReq,                                 // R2R_JT_OFFSET(29))           
  (uint32)MAC_YieldReq,                                  // R2R_JT_OFFSET(30))            
  (uint32)macAllocTxBuffer,                              // R2R_JT_OFFSET(31))
  (uint32)macApiAssociateReq,                            // R2R_JT_OFFSET(32))
  (uint32)macApiAssociateRsp,                            // R2R_JT_OFFSET(33)) 
  (uint32)macApiBadState,                                // R2R_JT_OFFSET(34)) 
  (uint32)macApiBeaconStartReq,                          // R2R_JT_OFFSET(35))
  (uint32)macApiDataReq,                                 // R2R_JT_OFFSET(36)) 
  (uint32)macApiDisassociateReq,                         // R2R_JT_OFFSET(37)) 
  (uint32)macApiOrphanRsp,                               // R2R_JT_OFFSET(38))
  (uint32)macApiPending,                                 // R2R_JT_OFFSET(39))
  (uint32)macApiPollReq,                                 // R2R_JT_OFFSET(40))
  (uint32)macApiPurgeReq,                                // R2R_JT_OFFSET(41))
  (uint32)macApiPwrOnReq,                                // R2R_JT_OFFSET(42))
  (uint32)macApiScanReq,                                 // R2R_JT_OFFSET(43))
  (uint32)macApiStartReq,                                // R2R_JT_OFFSET(44)) 
  (uint32)macApiSyncReq,                                 // R2R_JT_OFFSET(45)) 
  (uint32)macApiUnsupported,                             // R2R_JT_OFFSET(46)) 
  (uint32)macAssocDataReq,                               // R2R_JT_OFFSET(47))
  (uint32)macAssocDataReqComplete,                       // R2R_JT_OFFSET(48))
  (uint32)macAssocDataRxInd,                             // R2R_JT_OFFSET(49)) 
  (uint32)macAssocFailed,                                // R2R_JT_OFFSET(50)) 
  (uint32)macAssocFrameResponseTimeout,                  // R2R_JT_OFFSET(51))
  (uint32)macAssocRxDisassoc,                            // R2R_JT_OFFSET(52))
  (uint32)macAssociateCnf,                               // R2R_JT_OFFSET(53))
  (uint32)macAutoPendAddSrcMatchTableEntry,              // R2R_JT_OFFSET(54))
  (uint32)macAutoPendMaintainSrcMatchTable,              // R2R_JT_OFFSET(55))
  (uint32)macAutoPoll,                                   // R2R_JT_OFFSET(56))
  (uint32)macBackoffTimerRolloverCallback,               // R2R_JT_OFFSET(57))
  (uint32)macBackoffTimerTriggerCallback,                // R2R_JT_OFFSET(58))
  (uint32)macBeaconBattLifeCallback,                     // R2R_JT_OFFSET(59))
  (uint32)macBeaconCheckSched,                           // R2R_JT_OFFSET(60)) 
  (uint32)macBeaconCheckStartTime,                       // R2R_JT_OFFSET(61)) 
  (uint32)macBeaconCheckTxTime,                          // R2R_JT_OFFSET(62)) 
  (uint32)macBeaconClearIndirect,                        // R2R_JT_OFFSET(63))
  (uint32)macBeaconInit,                                 // R2R_JT_OFFSET(64))
  (uint32)macBeaconPeriodCallback,                       // R2R_JT_OFFSET(65))
  (uint32)macBeaconPrepareCallback,                      // R2R_JT_OFFSET(66)) 
  (uint32)macBeaconRequeue,                              // R2R_JT_OFFSET(67))
  (uint32)macBeaconReset,                                // R2R_JT_OFFSET(68))
  (uint32)macBeaconSchedRequested,                       // R2R_JT_OFFSET(69))
  (uint32)macBeaconSetPrepareTime,                       // R2R_JT_OFFSET(70))
  (uint32)macBeaconSetSched,                             // R2R_JT_OFFSET(71))
  (uint32)macBeaconSetupBroadcast,                       // R2R_JT_OFFSET(72)) 
  (uint32)macBeaconSetupCap,                             // R2R_JT_OFFSET(73))
  (uint32)macBeaconStartContinue,                        // R2R_JT_OFFSET(74))
  (uint32)macBeaconStartFrameResponseTimer,              // R2R_JT_OFFSET(75)) 
  (uint32)macBeaconStopTrack,                            // R2R_JT_OFFSET(76))
  (uint32)macBeaconSyncLoss,                             // R2R_JT_OFFSET(77)) 
  (uint32)macBeaconTxCallback,                           // R2R_JT_OFFSET(78))
  (uint32)macBlacklistChecking,                          // R2R_JT_OFFSET(79))
  (uint32)macBroadcastPendCallback,                      // R2R_JT_OFFSET(80))
  (uint32)macBuildAssociateReq,                          // R2R_JT_OFFSET(81))
  (uint32)macBuildAssociateRsp,                          // R2R_JT_OFFSET(82))
  (uint32)macBuildBeacon,                                // R2R_JT_OFFSET(83)) 
  (uint32)macBuildBeaconNotifyInd,                       // R2R_JT_OFFSET(84))
  (uint32)macBuildCommonReq,                             // R2R_JT_OFFSET(85))
  (uint32)macBuildDataFrame,                             // R2R_JT_OFFSET(86))
  (uint32)macBuildDisassociateReq,                       // R2R_JT_OFFSET(87))
  (uint32)macBuildEnhanceBeaconReq,                      // R2R_JT_OFFSET(88)) 
  (uint32)macBuildHeader,                                // R2R_JT_OFFSET(89))
  (uint32)macBuildPendAddr,                              // R2R_JT_OFFSET(90))
  (uint32)macBuildRealign,                               // R2R_JT_OFFSET(91))
  (uint32)macCbackForEvent,                              // R2R_JT_OFFSET(92))
  (uint32)macCcmStarInverseTransform,                    // R2R_JT_OFFSET(93)) 
  (uint32)macCcmStarTransform,                           // R2R_JT_OFFSET(94))
  (uint32)macCheckPendAddr,                              // R2R_JT_OFFSET(95))
  (uint32)macCheckSched,                                 // R2R_JT_OFFSET(96))
  (uint32)macCommStatusInd,                              // R2R_JT_OFFSET(97)) 
  (uint32)macConflictSyncLossInd,                        // R2R_JT_OFFSET(98))
  (uint32)macCoordAddrCmp,                               // R2R_JT_OFFSET(99))
  (uint32)macCoordDestAddrCmp,                           // R2R_JT_OFFSET(100))
  (uint32)macCoordReset,                                 // R2R_JT_OFFSET(101)) 
  (uint32)macDataReset,                                  // R2R_JT_OFFSET(102)) 
  (uint32)macDataRxInd,                                  // R2R_JT_OFFSET(103)) 
  (uint32)macDataRxMemAlloc,                             // R2R_JT_OFFSET(104))  
  (uint32)macDataRxMemFree,                              // R2R_JT_OFFSET(105))  
  (uint32)macDataSend,                                   // R2R_JT_OFFSET(106))  
  (uint32)macDataTxComplete,                             // R2R_JT_OFFSET(107))  
  (uint32)macDataTxDelayCallback,                        // R2R_JT_OFFSET(108))  
  (uint32)macDataTxEnqueue,                              // R2R_JT_OFFSET(109))  
  (uint32)macDataTxSend,                                 // R2R_JT_OFFSET(110))  
  (uint32)macDataTxTimeAvailable,                        // R2R_JT_OFFSET(111))  
  (uint32)macDefaultAction,                              // R2R_JT_OFFSET(112))  
  (uint32)macDestAddrCmp,                                // R2R_JT_OFFSET(113))  
  (uint32)macDestSAddrCmp,                               // R2R_JT_OFFSET(114)) 
  (uint32)macDeviceDescriptorLookup,                     // R2R_JT_OFFSET(115)) 
  (uint32)macDeviceReset,                                // R2R_JT_OFFSET(116))  
  (uint32)macDisassocComplete,                           // R2R_JT_OFFSET(117))  
  (uint32)macEventLoop,                                  // R2R_JT_OFFSET(118))  
  (uint32)macExecute,                                    // R2R_JT_OFFSET(119))  
  (uint32)macFrameDuration,                              // R2R_JT_OFFSET(120))  
  (uint32)macGetCoordAddress,                            // R2R_JT_OFFSET(121))  
  (uint32)macGetMyAddrMode,                              // R2R_JT_OFFSET(122)) 
  (uint32)macIncomingFrameSecurity,                      // R2R_JT_OFFSET(123))
  (uint32)macIncomingFrameSecurityMaterialRetrieval,     // R2R_JT_OFFSET(124))
  (uint32)macIncomingKeyUsagePolicyChecking,             // R2R_JT_OFFSET(125))
  (uint32)macIncomingNonSlottedTx,                       // R2R_JT_OFFSET(126))
  (uint32)macIncomingSecurityLevelChecking,              // R2R_JT_OFFSET(127)) 
  (uint32)macIndirectExpire,                             // R2R_JT_OFFSET(128))  
  (uint32)macIndirectMark,                               // R2R_JT_OFFSET(129)) 
  (uint32)macIndirectRequeueFrame,                       // R2R_JT_OFFSET(130)) 
  (uint32)macIndirectSend,                               // R2R_JT_OFFSET(131))  
  (uint32)macIndirectTxFrame,                            // R2R_JT_OFFSET(132))  
  (uint32)macKeyDescriptorLookup,                        // R2R_JT_OFFSET(133))  
  (uint32)macMainReserve,                                // R2R_JT_OFFSET(134)) 
  (uint32)macMainReset,                                  // R2R_JT_OFFSET(135))
  (uint32)macMemReadRam,                                 // R2R_JT_OFFSET(136))
  (uint32)macMemReadRamByte,                             // R2R_JT_OFFSET(137))
  (uint32)macMemWriteRam,                                // R2R_JT_OFFSET(138)) 
  (uint32)macMgmtReset,                                  // R2R_JT_OFFSET(139))  
  (uint32)macNoAction,                                   // R2R_JT_OFFSET(140)) 
  (uint32)macOutgoingFrameKeyRetrieval,                  // R2R_JT_OFFSET(141))
  (uint32)macOutgoingFrameSecurity,                      // R2R_JT_OFFSET(142))
  (uint32)macOutgoingNonSlottedTx,                       // R2R_JT_OFFSET(143)) 
  (uint32)macPanConflictComplete,                        // R2R_JT_OFFSET(144))  
  (uint32)macPendAddrLen,                                // R2R_JT_OFFSET(145)) 
  (uint32)macPibIndex,                                   // R2R_JT_OFFSET(146))
  (uint32)macPibReset,                                   // R2R_JT_OFFSET(147)) 
  (uint32)macPollCnf,                                    // R2R_JT_OFFSET(148))  
  (uint32)macPollDataReqComplete,                        // R2R_JT_OFFSET(149))  
  (uint32)macPollDataRxInd,                              // R2R_JT_OFFSET(150)) 
  (uint32)macPollFrameResponseTimeout,                   // R2R_JT_OFFSET(151)) 
  (uint32)macPollRxAssocRsp,                             // R2R_JT_OFFSET(152))  
  (uint32)macPollRxDisassoc,                             // R2R_JT_OFFSET(153))  
  (uint32)macPwrReset,                                   // R2R_JT_OFFSET(154))  
  (uint32)macPwrVote,                                    // R2R_JT_OFFSET(155))  
  (uint32)macRxAssocRsp,                                 // R2R_JT_OFFSET(156))  
  (uint32)macRxAssociateReq,                             // R2R_JT_OFFSET(157))  
  (uint32)macRxBeacon,                                   // R2R_JT_OFFSET(158))  
  (uint32)macRxBeaconCritical,                           // R2R_JT_OFFSET(159))  
  (uint32)macRxBeaconReq,                                // R2R_JT_OFFSET(160)) 
  (uint32)macRxCheckMACPendingCallback,                  // R2R_JT_OFFSET(161))
  (uint32)macRxCheckPendingCallback,                     // R2R_JT_OFFSET(162)) 
  (uint32)macRxCompleteCallback,                         // R2R_JT_OFFSET(163))  
  (uint32)macRxCoordRealign,                             // R2R_JT_OFFSET(164))  
  (uint32)macRxDataReq,                                  // R2R_JT_OFFSET(165))  
  (uint32)macRxDisassoc,                                 // R2R_JT_OFFSET(166))  
  (uint32)macRxEnhancedBeaconReq,                        // R2R_JT_OFFSET(167))  
  (uint32)macRxOrphan,                                   // R2R_JT_OFFSET(168))  
  (uint32)macRxPanConflict,                              // R2R_JT_OFFSET(169))  
  (uint32)macScanCnfInit,                                // R2R_JT_OFFSET(170))  
  (uint32)macScanComplete,                               // R2R_JT_OFFSET(171)) 
  (uint32)macScanFailedInProgress,                       // R2R_JT_OFFSET(172)) 
  (uint32)macScanNextChan,                               // R2R_JT_OFFSET(173))  
  (uint32)macScanRxBeacon,                               // R2R_JT_OFFSET(174))  
  (uint32)macScanRxCoordRealign,                         // R2R_JT_OFFSET(175))  
  (uint32)macScanStartTimer,                             // R2R_JT_OFFSET(176)) 
  (uint32)macSecCpy,                                     // R2R_JT_OFFSET(177))
  (uint32)macSecurityPibIndex,                           // R2R_JT_OFFSET(178))
  (uint32)macSecurityPibReset,                           // R2R_JT_OFFSET(179)) 
  (uint32)macSendDataMsg,                                // R2R_JT_OFFSET(180))  
  (uint32)macSendMsg,                                    // R2R_JT_OFFSET(181))  
  (uint32)macSetEvent,                                   // R2R_JT_OFFSET(182))  
  (uint32)macSetSched,                                   // R2R_JT_OFFSET(183)) 
  (uint32)macStartBeaconPrepareCallback,                 // R2R_JT_OFFSET(184)) 
  (uint32)macStartBegin,                                 // R2R_JT_OFFSET(185)) 
  (uint32)macStartBroadcastPendTimer,                    // R2R_JT_OFFSET(186)) 
  (uint32)macStartComplete,                              // R2R_JT_OFFSET(187))  
  (uint32)macStartContinue,                              // R2R_JT_OFFSET(188)) 
  (uint32)macStartFrameResponseTimer,                    // R2R_JT_OFFSET(189)) 
  (uint32)macStartResponseTimer,                         // R2R_JT_OFFSET(190))  
  (uint32)macStartSetParams,                             // R2R_JT_OFFSET(191))  
  (uint32)macStateIdle,                                  // R2R_JT_OFFSET(192))  
  (uint32)macStateIdleOrPolling,                         // R2R_JT_OFFSET(193))  
  (uint32)macStateScanning,                              // R2R_JT_OFFSET(194))  
  (uint32)macSyncTimeoutCallback,                        // R2R_JT_OFFSET(195))  
  (uint32)macTaskInit,                                   // R2R_JT_OFFSET(196))  
  (uint32)macTimer,                                      // R2R_JT_OFFSET(197))  
  (uint32)macTimerAddTimer,                              // R2R_JT_OFFSET(198))  
  (uint32)macTimerAligned,                               // R2R_JT_OFFSET(199))  
  (uint32)macTimerCancel,                                // R2R_JT_OFFSET(200))  
  (uint32)macTimerGetTime,                               // R2R_JT_OFFSET(201))  
  (uint32)macTimerInit,                                  // R2R_JT_OFFSET(202))  
  (uint32)macTimerRealign,                               // R2R_JT_OFFSET(203)) 
  (uint32)macTimerRecalcUnaligned,                       // R2R_JT_OFFSET(204)) 
  (uint32)macTimerRemoveTimer,                           // R2R_JT_OFFSET(205))  
  (uint32)macTimerSetRollover,                           // R2R_JT_OFFSET(206))  
  (uint32)macTimerStart,                                 // R2R_JT_OFFSET(207))  
  (uint32)macTimerSyncRollover,                          // R2R_JT_OFFSET(208)) 
  (uint32)macTimerUpdateBackoffTimer,                    // R2R_JT_OFFSET(209)) 
  (uint32)macTrackPeriodCallback,                        // R2R_JT_OFFSET(210))  
  (uint32)macTrackStartCallback,                         // R2R_JT_OFFSET(211)) 
  (uint32)macTrackTimeoutCallback,                       // R2R_JT_OFFSET(212))
  (uint32)macTxBeaconCompleteCallback,                   // R2R_JT_OFFSET(213)) 
  (uint32)macTxCompleteCallback,                         // R2R_JT_OFFSET(214))  
  (uint32)macUpdatePanId,                                // R2R_JT_OFFSET(215))  
  (uint32)mac_msg_deallocate,                            // R2R_JT_OFFSET(216))  
  (uint32)sAddrCmp,                                      // R2R_JT_OFFSET(217)) 
  (uint32)sAddrCpy,                                      // R2R_JT_OFFSET(218)) 
  (uint32)sAddrExtCmp,                                   // R2R_JT_OFFSET(219))  
  (uint32)sAddrExtCpy,                                   // R2R_JT_OFFSET(220)) 
  (uint32)sAddrIden,                                     // R2R_JT_OFFSET(221))
};




/*******************************************************************************
 * @fn          BLE ROM Initialization
 *
 * @brief       This routine initializes the TIMAC ROM software. First,
 *              the image's CRC is verified. Next, its C runtime is initialized.
 *              Then the ICall function pointers for dispatch, and enter/leave
 *              critical section are initialized. Finally, the location of the
 *              R2F and R2R flash jump tables are initialized.
 *
 * input parameters
 *
 * @param       None.
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void ROM_Init( void )
{
  volatile uint8 i;

  // verify the Controller ROM image
  i = validChecksum(&__checksum_begin, &__checksum_end);

  // trap a checksum failure - what now?
  while( !i );

  /*
  ** Controller ROM
  */

  // execute the ROM C runtime initializion
  // Note: This is the ROM's C Runtime initialization, not the flash's, which
  //       has already taken place.
  RT_Init_ROM();

  // initialize ICall function pointers for ROM
  // Note: The address of these functions is determined by the Application, and
  //       is passed to the Stack image via startup_entry.
  *icallRomDispatchPtr = (uint32)ICall_dispatcher;
  *icallRomEnterCSPtr  = (uint32)ICall_enterCriticalSection;
  *icallRomLeaveCSPtr  = (uint32)ICall_leaveCriticalSection;

  // initialize RAM pointer to R2F Flash JT for ROM code
  *r2fRomPtr = (uint32)R2F_Flash_JT;

  // initialize RAM pointer to R2R Flash JT for ROM code
  *r2rRomPtr = (uint32)R2R_Flash_JT;

  return;
}

/*******************************************************************************
 */

