/******************************************************************************

 @file  rom_init.c

 @brief This file contains the externs for BLE Controller and OSAL ROM
        initialization.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2017-2025, Texas Instruments Incorporated

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
#include "bcomdef.h"
#include "hal_types.h"
#include <inc/hw_types.h>
#include "rom_jt.h"
#include "onboard.h"
#include "osal.h"
#include "osal_memory.h"
#include "osal_bufmgr.h"
#include "osal_cbtimer.h"
#include "hal_assert.h"
#include "hci_tl.h"
#include "mb.h"
#include "rf_hal.h"
#include "ll_config.h"
#include "ll_common.h"
#include "icall.h"
#include <ti/drivers/rf/RF.h>
#ifndef CC33xx
#include "osal_pwrmgr.h"
#include "osal_list.h"
#include "hal_trng_wrapper.h"
#include "ecc_rom.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/vims.h)
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(inc/hw_sysctl.h)
#include DeviceFamily_constructPath(inc/hw_ioc.h)
#endif // !CC33xx
#ifndef CONTROLLER_ONLY
#include "linkdb.h"
#include "l2cap.h"
#include "att.h"
#include "gatt.h"
#include "gattservapp.h"
#include "gatt_uuid.h"
#include "gap.h"
#include "gap_internal.h"
#include "sm.h"
#include "linkdb_internal.h"
#include "gap_advertiser_internal.h"
#include "gap_advertiser.h"
#include "gap_scanner_internal.h"
#include "gap_scanner.h"
#include "gap_initiator.h"
#include "ll_sdaa.h"

#endif // !CONTROLLER_ONLY

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

// Function pointers used by the Flash software (provided by RTOS).
extern ICall_Dispatcher ICall_dispatcher;
extern ICall_EnterCS    ICall_enterCriticalSection;
extern ICall_LeaveCS    ICall_leaveCriticalSection;

#if defined( ENABLE_ROM_CHECKSUM_CHECK )
// Checksum start/end address and value for ROM and Common ROM.
extern const uint32 __checksum_begin;
extern const uint32 __checksum_end;

// calculate checksum routine
extern uint16 slow_crc16( uint16 crc, uint8 *pAddr, uint32 len );
extern uint8  validChecksum( const uint32 *beginAddr, const uint32 *endAddr );
#endif // ENABLE_ROM_CHECKSUM_CHECK

// callback for radio driver events
extern void rfCallback( RF_Handle, RF_CmdHandle, RF_EventMask );
extern void rfPUpCallback( RF_Handle, RF_CmdHandle, RF_EventMask );

// Jump Table Function Externs: Needed to access internal system functions.
extern void ll_eccInit(void);
extern void ll_GenerateDHKey(void);
extern void ll_ReadLocalP256PublicKey(void);
extern void attSendMsg(void);
extern void attSendRspMsg(void);
extern void gattClientHandleConnStatusCB(void);
extern void gattClientHandleTimerCB(void);
extern void gattClientNotifyTxCB(void);
extern void gattClientProcessMsgCB(void);
extern void gattFindClientInfo(void);
extern void gattFindServerInfo(void);
extern void gattFindService(void);
extern void gattGetPayload(void);
extern void gattGetServerStatus(void);
extern void gattNotifyEvent(void);
extern void gattParseReq(void);
extern void gattProcessExchangeMTUReq(void);
extern void gattProcessExecuteWriteReq(void);
extern void gattProcessFindByTypeValueReq(void);
extern void gattProcessFindInfoReq(void);
extern void gattProcessReadByGrpTypeReq(void);
extern void gattProcessReadByTypeReq(void);
extern void gattProcessReadReq(void);
extern void gattProcessReq(void);
extern void gattProcessRxData(void);
extern void gattProcessOSALMsg(void);
extern void gattProcessWriteReq(void);
extern void gattProcessReadMultiReq(void);
extern void gattRegisterClient(void);
extern void gattRegisterServer(void);
extern void gattResetServerInfo(void);
extern void gattSendFlowCtrlEvt(void);
extern void gattServerHandleConnStatusCB(void);
extern void gattServerHandleTimerCB(void);
extern void gattServerNotifyTxCB(void);
extern void gattServerProcessMsgCB(void);
extern void gattServerStartTimer(void);
extern void gattServiceLastHandle(void);
extern void gattStartTimer(void);
extern void gattStopTimer(void);
extern void gattStoreServerInfo(void);
extern void gattClientStartTimer(void);
extern void gattProcessMultiReqs(void);
extern void gattResetClientInfo(void);
extern void gattProcessFindInfo(void);
extern void gattProcessFindByTypeValue(void);
extern void gattProcessReadByType(void);
extern void gattProcessReadLong(void);
extern void gattProcessReadByGrpType(void);
extern void gattProcessReliableWrites(void);
extern void gattProcessWriteLong(void);
extern void gattWrite(void);
extern void gattWriteLong(void);
extern void gattPrepareWriteReq(void);
extern void gattStoreClientInfo(void);
extern void gattReadByGrpType(void);
extern void gattFindByTypeValue(void);
extern void gattReadByType(void);
extern void gattFindInfo(void);
extern void gattRead(void);
extern void gattReadLong(void);
extern void gattGetClientStatus(void);
extern void gattServApp_FindServiceCBs(void);
extern void gattServApp_ProcessExchangeMTUReq(void);
extern void gattServApp_ProcessExecuteWriteReq(void);
extern void gattServApp_ProcessFindByTypeValueReq(void);
extern void gattServApp_ProcessPrepareWriteReq(void);
extern void gattServApp_ProcessReadBlobReq(void);
extern void gattServApp_ProcessReadByTypeReq(void);
extern void gattServApp_ProcessReadByGrpTypeReq(void);
extern void gattServApp_ProcessReadMultiReq(void);
extern void gattServApp_ProcessReadReq(void);
extern void gattServApp_ProcessWriteReq(void);
extern void smProcessHCIBLEEventCode(void);
extern void smProcessHCIBLEMetaEventCode(void);
extern void smProcessOSALMsg(void);
extern void generate_subkey(void);
extern void leftshift_onebit(void);
extern void padding(void);
extern void smAuthReqToUint8(void);
extern void smEncrypt(void);
extern void smEncryptLocal(void);
extern void smGenerateRandBuf(void);
extern void smStartRspTimer(void);
extern void smUint8ToAuthReq(void);
extern void sm_c1new(void);
extern void sm_CMAC(void);
extern void sm_f4(void);
extern void sm_f5(void);
extern void sm_f6(void);
extern void sm_g2(void);
extern void sm_s1(void);
extern void sm_xor(void);
extern void xor_128(void);
extern void smDetermineIOCaps(void);
extern void smDetermineKeySize(void);
extern void smEndPairing(void);
extern void smFreePairingParams(void);
extern void smF5Wrapper(void);
extern void smGenerateAddrInput(void);
extern void smGenerateAddrInput_sPatch(void);
extern void smGenerateConfirm(void);
extern void smGenerateDHKeyCheck(void);
extern void smGeneratePairingReqRsp(void);
extern void smGenerateRandMsg(void);
extern void smGetECCKeys(void);
extern void smIncrementEccKeyRecycleCount(void);
extern void smLinkCheck(void);
extern void smNextPairingState(void);
extern void smOobSCAuthentication(void);
extern void smPairingSendEncInfo(void);
extern void smPairingSendIdentityAddrInfo(void);
extern void smPairingSendIdentityInfo(void);
extern void smPairingSendCentralID(void);
extern void smPairingSendSigningInfo(void);
extern void smProcessDataMsg(void);
extern void smProcessEncryptChange(void);
extern void smProcessPairingReq(void);
extern void smRegisterResponder(void);
extern void smSavePairInfo(void);
extern void smSaveRemotePublicKeys(void);
extern void smSendDHKeyCheck(void);
extern void smSendFailAndEnd(void);
extern void smSendFailureEvt(void);
extern void smSetPairingReqRsp(void);
extern void smSendPublicKeys(void);
extern void smStartEncryption(void);
extern void smTimedOut(void);
extern void sm_allocateSCParameters(void);
extern void sm_computeDHKey(void);
extern void sm_c1(void);
extern void smpProcessIncoming(void);
extern void smFinishPublicKeyExchange(void);
extern void smResponderAuthStageTwo(void);
extern void smpResponderProcessEncryptionInformation(void);
extern void smpResponderProcessIdentityAddrInfo(void);
extern void smpResponderProcessIdentityInfo(void);
extern void smpResponderProcessCentralID(void);
extern void smpResponderProcessPairingConfirm(void);
extern void smpResponderProcessPairingDHKeyCheck(void);
extern void smpResponderProcessPairingPublicKey(void);
extern void smpResponderProcessPairingRandom(void);
extern void smpResponderProcessPairingReq(void);
extern void smpResponderProcessSigningInfo(void);
extern void smpBuildEncInfo(void);
extern void smpBuildIdentityAddrInfo(void);
extern void smpBuildIdentityInfo(void);
extern void smpBuildCentralID(void);
extern void smpBuildPairingConfirm(void);
extern void smpBuildPairingDHKeyCheck(void);
extern void smpBuildPairingFailed(void);
extern void smpBuildPairingPublicKey(void);
extern void smpBuildPairingRandom(void);
extern void smpBuildPairingReq(void);
extern void smpBuildPairingReqRsp(void);
extern void smpBuildPairingRsp(void);
extern void smpBuildSecurityReq(void);
extern void smpBuildSigningInfo(void);
extern void smpParseEncInfo(void);
extern void smpParseIdentityAddrInfo(void);
extern void smpParseIdentityInfo(void);
extern void smpParseKeypressNoti(void);
extern void smpParseCentralID(void);
extern void smpParsePairingConfirm(void);
extern void smpParsePairingDHKeyCheck(void);
extern void smpParsePairingFailed(void);
extern void smpParsePairingPublicKey(void);
extern void smStopRspTimer(void);
extern void smpParsePairingRandom(void);
extern void smpParsePairingReq(void);
extern void smpParseSecurityReq(void);
extern void smpParseSigningInfo(void);
extern void smSendSMMsg(void);
extern void smpInitiatorProcessPairingRsp(void);
extern void smpInitiatorProcessPairingPubKey(void);
extern void smpInitiatorProcessPairingDHKeyCheck(void);
extern void smpInitiatorProcessPairingConfirm(void);
extern void smpInitiatorProcessPairingRandom(void);
extern void smpInitiatorProcessEncryptionInformation(void);
extern void smpInitiatorProcessCentralID(void);
extern void smpInitiatorProcessIdentityInfo(void);
extern void smpInitiatorProcessIdentityAddrInfo(void);
extern void smpInitiatorProcessSigningInfo(void);
extern void smInitiatorAuthStageTwo(void);
extern uint8 smpInitiatorContProcessPairingPubKey(void);
extern void setupInitiatorKeys(void);
extern void smInitiatorSendNextKeyInfo(void);
extern void smpResponderProcessIncoming(void);
extern void smResponderSendNextKeyInfo(void);
extern void smpResponderSendPairRspEvent(void);
extern void smResponderProcessLTKReq(void);
extern void smRegisterInitiator(void);
extern void smEncLTK(void);
extern void smpInitiatorProcessIncoming(void);
extern void gapScan_filterDiscMode(void);
extern void gapScan_discardAdvRptSession(void);
extern void gapScan_sendSessionEndEvt(void);
extern void gapScan_defragAdvRpt(void);
extern void gapScan_saveRptAndNotify(void);
extern void gapScan_processAdvRptCb(void);
extern void gapScan_processStartEndCb(void);
extern void gapScan_processErrorCb(void);
extern void gapInit_connect_internal(void);
extern void gapInit_sendConnCancelledEvt(void);
extern void gapInit_initiatingEnd(void);
extern uint8  l2capSendNextSegment(void);
extern uint8  l2capReassembleSegment(uint16 connHandle, void *pPkt );
extern uint8  l2capParseConnectRsp( void *pCmd, uint8 *pData, uint16 len );
extern uint8  l2capParseDisconnectReq( void *pCmd, uint8 *pData, uint16 len );
extern uint8  l2capParseDisconnectRsp( void *pCmd, uint8 *pData, uint16 len );
extern uint16 l2capBuildDisconnectRsp( uint8 *pBuf, uint8 *pData );
extern void   l2capProcessConnectReq( uint16 connHandle, uint8 id, void *pConnReq );
extern void   l2capGetCoChannelInfo( void *pCoC, void *pInfo );
extern void   l2capNotifyChannelEstEvt( void *pChannel, uint8 status, uint16 result );
extern void  *l2capFindRemoteCID( uint16 connHandle, uint16 CID );
extern void   l2capNotifyChannelTermEvt( void *pChannel, uint8 status, uint16 reason );
extern void  *l2capFindLocalCID( uint16 CID );
extern void   l2capDisconnectChannel( void *pChannel, uint16 reason );
/*******************************************************************************
 * PROTOTYPES
 */

void ROM_Spinlock( void );
uint32 FLASH_EmptyFunc( void ) { return 0; }

/*******************************************************************************
 * MACROS
 */

// ICall function pointers and R2F/R2R flash JT pointers for ROM
#define pICallRomDispatch      (uint32 *)(&RAM_BASE_ADDR+0)
#define pICallRomEnterCS       (uint32 *)(&RAM_BASE_ADDR+1)
#define pICallRomLeaveCS       (uint32 *)(&RAM_BASE_ADDR+2)
#define pROM_JT                (uint32 *)(&RAM_BASE_ADDR+3)

// Runtime Init code for Common ROM
#define RT_Init_ROM ((RT_Init_fp)&ROM_BASE_ADDR)

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
#include "ll.h"
#include "ll_ae.h"
#include "ll_enc.h"
#include "ll_al.h"
#include "ll_timer_drift.h"
#include "ll_rat.h"
#include "ll_privacy.h"

/*******************************************************************************
 * @fn          BLE ROM Spinlock
 *
 * @brief       This routine is used to trap indexing errors in R2R JT.
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
void ROM_Spinlock( void )
{
  volatile uint8 i = 1;

  while(i);
}

#if defined(__GNUC__) && !defined(__clang__)
__attribute__((optimize("O0")))
#endif
#if defined ( FLASH_ROM_BUILD )
/*******************************************************************************
 * @fn          BLE ROM Initialization
 *
 * @brief       This routine initializes the BLE Controller ROM software. First,
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
#if defined( ENABLE_ROM_CHECKSUM_CHECK )
  volatile uint8 i;

  // verify the Controller ROM image
  i = validChecksum(&__checksum_begin, &__checksum_end);

  // trap a checksum failure - what now?
  while( !i );
#endif // ENABLE_ROM_CHECKSUM_CHECK

  /*
  ** Controller ROM
  */

  // execute the ROM C runtime initialization
  // Note: This is the ROM's C Runtime initialization, not the flash's, which
  //       has already taken place.
  RT_Init_ROM();

  // initialize ICall function pointers for ROM
  // Note: The address of these functions is determined by the Application, and
  //       is passed to the Stack image via startup_entry.
  *pICallRomDispatch = (uint32)ICall_dispatcher;
  *pICallRomEnterCS  = (uint32)ICall_enterCriticalSection;
  *pICallRomLeaveCS  = (uint32)ICall_leaveCriticalSection;

  // initialize RAM pointer to ROM Flash JT for ROM code
  *pROM_JT = (uint32)ROM_Flash_JT;

  return;
}
#endif // FLASH_ROM_BUILD

/*******************************************************************************
 * PROTOTYPES
 */
extern uint8 llLastCmdDoneEventHandleConnectRequest( advSet_t *pAdvSet );
extern uint8 llRxEntryDoneEventHandleConnectRequest( advSet_t *pAdvSet, uint8 *PeerA, uint8 PeerAdd, uint8 chSel );
extern uint8 llRxIgnoreEventHandleConnectRequest( advSet_t *pAdvSet, uint8 *PeerA, uint8 PeerAdd, uint8 chSel );
extern uint8 llAbortEventHandleStateAdv( uint8 preempted );
extern uint8 llLastCmdDoneEventHandleStateAdv( void );
extern uint8 llTxDoneEventHandleStateAdv( void );
extern uint8 llRxIgnoreEventHandleStateAdv( void );
extern uint8 llRxEmptyEventHandleStateAdv( void );
extern uint8 llRxEntryDoneEventHandleStateAdv( void );
extern uint8 llAbortEventHandleStateScan( uint8 preempted );
extern uint8 llLastCmdDoneEventHandleStateScan( void );
extern uint8 llRxIgnoreEventHandleStateScan( void );
extern uint8 llRxIgnoreEventHandleConnectResponse( uint8 *OwnA, uint8 OwnAdd, uint8 *PeerA, uint8 PeerAdd );
extern uint8 llAbortEventHandleStateInit( uint8 preempted );
extern uint8 llLastCmdDoneEventHandleStateInit( void );
extern uint8 llRxIgnoreEventHandleStateInit( void );
extern uint8 llRxEntryDoneEventHandleStateInit( void );
extern uint8 llAbortEventHandleStatePeripheral( uint8 preempted );
extern uint8 llLastCmdDoneEventHandleStatePeripheral( void );
extern uint8 llAbortEventHandleStateCentral( uint8 preempted );
extern uint8 llLastCmdDoneEventHandleStateCentral( void );
extern uint8 llRxEntryDoneEventHandleStateConnection( uint8 crcError );
extern uint8 llLastCmdDoneEventHandleStateTest( void );
extern uint8 llRxEntryDoneEventHandleStateTest( void );
extern void llSetTaskInit( uint8 startType, taskInfo_t *nextSecTask, void *nextSecCommand, void *nextConnCmd );
extern void llSetTaskScan( uint8 startType, taskInfo_t *nextSecTask, void *nextSecCommand, void *nextConnCmd );
extern void llSetTaskAdv( uint8 startType, void *nextSecCmd );
extern void llSetTaskCentral( uint8 connId, void *nextConnCmd );
extern void llSetTaskPeripheral( uint8 connId, void *nextConnCmd );
extern void llSetTaskPeriodicAdv( void );
extern void llSetTaskPeriodicScan( void );
extern taskInfo_t *llSelectTaskAdv( uint8 secTaskID, uint32 timeGap );
extern taskInfo_t *llSelectTaskInit( uint8 secTaskID, uint32 timeGap );
extern taskInfo_t *llSelectTaskScan( uint8 secTaskID, uint32 timeGap );
extern taskInfo_t *llSelectTaskPeriodicScan( uint8 secTaskID, uint32 timeGap );
extern taskInfo_t *llSelectTaskPeriodicAdv( uint8 secTaskID, uint32 timeGap );
extern void LL_TxEntryDoneCback( void );
extern uint8 llCheckIsSecTaskCollideWithPrimTaskInLsto( taskInfo_t *secTask,uint32 timeGap,uint16 selectedConnId);
extern llStatus_t llPostProcessExtendedAdv( advSet_t *pAdvSet );
extern uint8 llTxDoneEventHandleStateExtAdv( advSet_t *pAdvSet );
extern void llSetupExtendedAdvData( advSet_t *pAdvSet );
extern uint8 llSetExtendedAdvReport(aeExtAdvRptEvt_t *extAdvRpt, uint8 *pPkt, uint16 evtType,uint8 extHdrFlgs, uint8 pHdr, uint8 dataLen, uint8 **pSyncInfo,uint8 *secPhy, uint8 *pChannelIndex);

/*******************************************************************************
 * INIT_CFG and SCAN_CFG hooks
 */
void MAP_llProcessCentralConnectionCreated(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llProcessCentralConnectionCreated();
#endif
}

void MAP_llProcessPeripheralConnectionCreated(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  llProcessPeripheralConnectionCreated();
#endif
}

void MAP_llProcessScanTimeout(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  llProcessScanTimeout();
#endif
}

void MAP_llProcessConnectionEstablishFailed(uint8 role, uint8 reason)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  llProcessConnectionEstablishFailed(role,reason);
#endif
}

void MAP_llProcessAdvAddrResolutionTimeout(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  llProcessAdvAddrResolutionTimeout();
#endif
}

uint8 MAP_llAbortEventHandleStateAdv( uint8 preempted )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llAbortEventHandleStateAdv(preempted);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleStateAdv( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llLastCmdDoneEventHandleStateAdv();
#else
  return 0;
#endif
}

uint8 MAP_llTxDoneEventHandleStateAdv( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llTxDoneEventHandleStateAdv();
#else
  return 0;
#endif
}

uint8 MAP_llRxIgnoreEventHandleStateAdv( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llRxIgnoreEventHandleStateAdv();
#else
  return 0;
#endif
}

uint8 MAP_llRxEmptyEventHandleStateAdv( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llRxEmptyEventHandleStateAdv();
#else
  return 0;
#endif
}

uint8 MAP_llRxEntryDoneEventHandleStateAdv( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llRxEntryDoneEventHandleStateAdv();
#else
  return 0;
#endif
}

uint8 MAP_llAbortEventHandleStateScan( uint8 preempted )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  return llAbortEventHandleStateScan(preempted);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleStateScan( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  return llLastCmdDoneEventHandleStateScan();
#else
  return 0;
#endif
}

uint8 MAP_llRxIgnoreEventHandleStateScan( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  return llRxIgnoreEventHandleStateScan();
#else
  return 0;
#endif
}

uint8 MAP_llRxIgnoreEventHandleConnectResponse( uint8 *OwnA, uint8 OwnAdd, uint8 *PeerA, uint8 PeerAdd )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llRxIgnoreEventHandleConnectResponse(OwnA,OwnAdd,PeerA,PeerAdd);
#else
  return 0;
#endif
}

uint8 MAP_llAbortEventHandleStateInit( uint8 preempted )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llAbortEventHandleStateInit(preempted);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleStateInit( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llLastCmdDoneEventHandleStateInit();
#else
  return 0;
#endif
}

uint8 MAP_llRxIgnoreEventHandleStateInit( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llRxIgnoreEventHandleStateInit();
#else
  return 0;
#endif
}

uint8 MAP_llRxEntryDoneEventHandleStateInit( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llRxEntryDoneEventHandleStateInit();
#else
  return 0;
#endif
}

uint8 MAP_llAbortEventHandleStatePeripheral( uint8 preempted )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  return llAbortEventHandleStatePeripheral(preempted);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleStatePeripheral( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  return llLastCmdDoneEventHandleStatePeripheral();
#else
  return 0;
#endif
}

uint8 MAP_llAbortEventHandleStateCentral( uint8 preempted )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llAbortEventHandleStateCentral(preempted);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleStateCentral( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llLastCmdDoneEventHandleStateCentral();
#else
  return 0;
#endif
}

uint8 MAP_llRxEntryDoneEventHandleStateConnection( uint8 crcError )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  return llRxEntryDoneEventHandleStateConnection(crcError);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleStateTest( void )
{
#if 1
  return llLastCmdDoneEventHandleStateTest();
#else
  return 0;
#endif
}

uint8 MAP_llRxEntryDoneEventHandleStateTest( void )
{
#if 1
  return llRxEntryDoneEventHandleStateTest();
#else
  return 0;
#endif
}

void MAP_llProcessCentralControlPacket(void *connPtr, uint8 *pPkt)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llProcessCentralControlPacket(connPtr, pPkt);
#endif
}

void MAP_llProcessPeripheralControlPacket(void *connPtr, uint8 *pPkt)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  llProcessPeripheralControlPacket(connPtr, pPkt);
#endif
}

void MAP_llSetTaskInit( uint8 startType, void *nextSecTask, void *nextSecCmd, void *nextConnCmd )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llSetTaskInit(startType, nextSecTask, nextSecCmd,nextConnCmd);
#endif
}

void MAP_llSetTaskScan( uint8 startType, void *nextSecTask, void *nextSecCmd, void *nextConnCmd )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  llSetTaskScan(startType, nextSecTask, nextSecCmd, nextConnCmd);
#endif
}

void MAP_llSetTaskPeriodicScan( void )
{
#ifdef USE_PERIODIC_SCAN
  llSetTaskPeriodicScan();
#endif
}

void MAP_llSetTaskAdv( uint8 startType, void *nextSecCmd )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  llSetTaskAdv(startType, nextSecCmd);
#endif
}

void MAP_llSetTaskPeriodicAdv( void )
{
#ifdef USE_PERIODIC_ADV
  llSetTaskPeriodicAdv();
#endif
}

void MAP_llSetTaskCentral( uint8 connId, void *nextConnCmd )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llSetTaskCentral(connId, nextConnCmd);
#endif
}

void MAP_llSetTaskPeripheral( uint8 connId, void *nextConnCmd )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  llSetTaskPeripheral(connId, nextConnCmd);
#endif
}

void *MAP_llSelectTaskAdv( uint8 secTaskID, uint32 timeGap )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return llSelectTaskAdv(secTaskID,timeGap);
#else
  return NULL;
#endif
}

void *MAP_llSelectTaskInit( uint8 secTaskID, uint32 timeGap )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return llSelectTaskInit(secTaskID,timeGap);
#else
  return NULL;
#endif
}

void *MAP_llSelectTaskScan( uint8 secTaskID, uint32 timeGap )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  return llSelectTaskScan(secTaskID,timeGap);
#else
  return NULL;
#endif
}

void *MAP_llSelectTaskPeriodicScan( uint8 secTaskID, uint32 timeGap )
{
#ifdef USE_PERIODIC_SCAN
  return llSelectTaskPeriodicScan(secTaskID,timeGap);
#else
  return NULL;
#endif
}

void *MAP_llSelectTaskPeriodicAdv( uint8 secTaskID, uint32 timeGap )
{
#if ( HOST_CONFIG & ( PERIPHERAL_CFG | BROADCASTER_CFG ) ) && defined ( USE_PERIODIC_ADV )
  return llSelectTaskPeriodicAdv(secTaskID,timeGap);
#else
  return NULL;
#endif
}

uint8 MAP_LE_ClearAdvSets(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return LE_ClearAdvSets();
#else
  return 0;
#endif
}

uint8 MAP_LL_ConnActive(uint16 connId)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  return LL_ConnActive(connId);
#else
  return 0;
#endif
}

uint8 MAP_LL_CountAdvSets( uint8 type )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return LL_CountAdvSets(type);
#else
  return 0;
#endif
}

void *MAP_LL_SearchAdvSet( uint8 handle )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return LL_GetAdvSet( handle, LE_SEARCH_ADV_SET );
#else
  return NULL;
#endif
}

uint8 MAP_llCheckPeripheralTerminate( uint8 connId )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  return llCheckPeripheralTerminate(connId);
#else
  return 0;
#endif
}

uint8 MAP_llGetNextConn( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  return llGetNextConn();
#else
  return 0;
#endif
}

void *MAP_llDataGetConnPtr( uint8 connId )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  return llDataGetConnPtr(connId);
#else
  return NULL;
#endif
}

void MAP_llConnCleanup( void *connPtr )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  llConnCleanup(connPtr);
#endif
}

void MAP_llReleaseAllConnId( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  llReleaseAllConnId();
#endif
}

void MAP_LL_TxEntryDoneCback( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  LL_TxEntryDoneCback();
#endif
}

uint8 MAP_llCompareSecondaryPrimaryTasksQoSParam( uint8 qosParamType,
                                                  void *secTask,
                                                  void *primConnPtr )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG | SCAN_CFG))
  return llCompareSecondaryPrimaryTasksQoSParam(qosParamType,secTask,primConnPtr);
#else
  return 0;
#endif
}

uint8 MAP_llCheckIsSecTaskCollideWithPrimTaskInLsto( void *secTask,
                                                     uint32 timeGap,
                                                     uint16 selectedConnId )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG | SCAN_CFG))
  return llCheckIsSecTaskCollideWithPrimTaskInLsto(secTask,timeGap,selectedConnId);
#else
  return 0;
#endif
}

void MAP_llSetupConn( uint8 connId )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llSetupConn(connId);
#endif
}

void MAP_llProcessExtScanRxFIFO( void )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  llProcessExtScanRxFIFO();
#endif
}

void MAP_llAlignToNextEvent( void *connPtr )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  llAlignToNextEvent(connPtr);
#endif
}

uint8 MAP_LL_SetSecAdvChanMap( uint8 *chanMap )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  return LL_SetSecAdvChanMap(chanMap);
#else
  return 0;
#endif
}

uint8 MAP_LL_ChanMapUpdate( uint8 *chanMap, uint16 connID )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  return LL_ChanMapUpdate(chanMap, connID);
#else
  return 0;
#endif
}

uint8 MAP_llLastCmdDoneEventHandleConnectRequest( void *pAdvSet )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  return llLastCmdDoneEventHandleConnectRequest(pAdvSet);
#else
  return 0;
#endif
}

uint8 MAP_llRxEntryDoneEventHandleConnectRequest( void *pAdvSet, uint8 *PeerA, uint8 PeerAdd, uint8 chSel )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  return llRxEntryDoneEventHandleConnectRequest(pAdvSet,PeerA,PeerAdd,chSel);
#else
  return 0;
#endif
}

uint8 MAP_llRxIgnoreEventHandleConnectRequest( void *pAdvSet, uint8 *PeerA, uint8 PeerAdd, uint8 chSel )
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  return llRxIgnoreEventHandleConnectRequest(pAdvSet,PeerA,PeerAdd,chSel);
#else
  return 0;
#endif
}

uint8 MAP_llConnExists( uint8 *peerAddr, uint8  peerAddrType)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_CONN_CFG | INIT_CFG))
  return llConnExists(peerAddr,peerAddrType);
#else
  return 0;
#endif
}

/*******************************************************************************
 * RTLS hooks
 */
extern uint8 llGetCteInfo( uint8 id, void *ptr );
extern uint8 RTLSSrv_processHciEvent(uint16_t hciEvt, uint16_t hciEvtSz, uint8_t *pEvtData);
extern uint8 RTLSSrv_processPeriodicAdvEvent(void *pMsg);
extern uint8 LL_DirectCteTestTxTest( uint8 txChan,uint8 payloadLen, uint8 payloadType,
                                     uint8 txPhy, uint8 cteLength, uint8 cteType,
                                     uint8 length, uint8 *pAntenna);
extern uint8 LL_DirectCteTestRxTest( uint8 rxChan, uint8 rxPhy, uint8 modIndex,
                                     uint8 expectedCteLength, uint8 expectedCteType,
                                     uint8 slotDurations, uint8 length, uint8 *pAntenna);

uint8 MAP_LL_DirectCteTestTxTest( uint8 txChan,
                                  uint8 payloadLen,
                                  uint8 payloadType,
                                  uint8 txPhy,
                                  uint8 cteLength,
                                  uint8 cteType,
                                  uint8 length,
                                  uint8 *pAntenna)
{
#ifdef RTLS_CTE_TEST
  return LL_DirectCteTestTxTest( txChan,
                                 payloadLen,
                                 payloadType,
                                 txPhy,
                                 cteLength,
                                 cteType,
                                 length,
                                 pAntenna);
#else
  return (LL_STATUS_SUCCESS);
#endif
}

uint8 MAP_LL_DirectCteTestRxTest( uint8 rxChan,
                                  uint8 rxPhy,
                                  uint8 modIndex,
                                  uint8 expectedCteLength,
                                  uint8 expectedCteType,
                                  uint8 slotDurations,
                                  uint8 length,
                                  uint8 *pAntenna)
{
#ifdef RTLS_CTE_TEST
  return LL_DirectCteTestRxTest( rxChan,
                                 rxPhy,
                                 modIndex,
                                 expectedCteLength,
                                 expectedCteType,
                                 slotDurations,
                                 length,
                                 pAntenna);
#else
  return (LL_STATUS_SUCCESS);
#endif
}

uint8 MAP_LL_EnhancedCteTxTest( uint8 txChan,
                                     uint8 payloadLen,
                                     uint8 payloadType,
                                     uint8 txPhy,
                                     uint8 cteLength,
                                     uint8 cteType,
                                     uint8 length,
                                     uint8 *pAntenna)
{
#ifdef RTLS_CTE_TEST
  return LL_EnhancedCteTxTest (txChan,
                               payloadLen,
                               payloadType,
                               txPhy,
                               cteLength,
                               cteType,
                               length,
                               pAntenna);
#else
  return (LL_STATUS_ERROR_COMMAND_DISALLOWED);
#endif
}

uint8 MAP_LL_EnhancedCteRxTest( uint8 rxChan,
                                     uint8 rxPhy,
                                     uint8 modIndex,
                                     uint8 expectedCteLength,
                                     uint8 expectedCteType,
                                     uint8 slotDurations,
                                     uint8 length,
                                     uint8 *pAntenna)
{
#ifdef RTLS_CTE_TEST
  return LL_EnhancedCteRxTest (rxChan,
                               rxPhy,
                               modIndex,
                               expectedCteLength,
                               expectedCteType,
                               slotDurations,
                               length,
                               pAntenna);
#else
  return (LL_STATUS_ERROR_COMMAND_DISALLOWED);
#endif
}

void MAP_llSetRfReportAodPackets( void )
{
#ifdef RTLS_CTE_TEST
  llSetRfReportAodPackets();
#endif
}

uint8 MAP_llGetCteInfo( uint8 id, void *ptr )
{
#ifdef RTLS_CTE
  return llGetCteInfo(id, ptr);
#else
  return 1;
#endif
}

uint8 MAP_RTLSSrv_processHciEvent(uint16_t hciEvt, uint16_t hciEvtSz, uint8_t *pEvtData)
{
#ifdef RTLS_CTE
  return RTLSSrv_processHciEvent(hciEvt, hciEvtSz, pEvtData);
#else
  return 1; //safeToDealloc
#endif
}

uint8 MAP_LL_SetConnectionCteReceiveParams( uint16 connHandle, uint8 samplingEnable,
                                            uint8 slotDurations, uint8 length, uint8 *pAntenna )
{
#ifdef RTLS_CTE
  return LL_SetConnectionCteReceiveParams( connHandle, samplingEnable, slotDurations, length, pAntenna );
#else
  return 1;
#endif
}

uint8 MAP_LL_SetConnectionCteTransmitParams( uint16 connHandle, uint8  types,
                                             uint8 length, uint8 *pAntenna )
{
#ifdef RTLS_CTE
  return LL_SetConnectionCteTransmitParams( connHandle, types, length, pAntenna );
#else
  return 1;
#endif
}

uint8 MAP_LL_SetConnectionCteRequestEnable( uint16 connHandle, uint8 enable,
                                             uint16 interval, uint8 length, uint8 type )
{
#ifdef RTLS_CTE
  return LL_SetConnectionCteRequestEnable( connHandle, enable, interval, length, type );
#else
  return 1;
#endif
}

uint8 MAP_LL_SetConnectionCteResponseEnable( uint16 connHandle, uint8 enable )
{
#ifdef RTLS_CTE
  return LL_SetConnectionCteResponseEnable( connHandle, enable );
#else
  return 1;
#endif
}

uint8 MAP_LL_ReadAntennaInformation( uint8 *sampleRates, uint8 *maxNumOfAntennas,
                                     uint8 *maxSwitchPatternLen, uint8 *maxCteLen)
{
#ifdef RTLS_CTE
  return LL_ReadAntennaInformation( sampleRates, maxNumOfAntennas, maxSwitchPatternLen, maxCteLen );
#else
  return 1;
#endif
}

void MAP_llUpdateCteState( void *connPtr )
{
#ifdef RTLS_CTE
  llUpdateCteState(connPtr);
#endif
}

uint8 MAP_llSetupCte( void *connPtr, uint8 req)
{
#ifdef RTLS_CTE
  return llSetupCte( connPtr, req );
#else
  return 0;
#endif
}

uint8 MAP_llFreeCteSamplesEntryQueue( void )
{
#ifdef RTLS_CTE
  return llFreeCteSamplesEntryQueue();
#else
  return 0;
#endif
}

uint8 MAP_LL_EXT_SetLocationingAccuracy( uint16 handle, uint8  sampleRate1M, uint8  sampleSize1M,
                                         uint8  sampleRate2M, uint8  sampleSize2M, uint8  sampleCtrl)
{
#ifdef RTLS_CTE
  return LL_EXT_SetLocationingAccuracy( handle, sampleRate1M, sampleSize1M,
                                        sampleRate2M, sampleSize2M, sampleCtrl);
#else
  return 1;
#endif
}

/*******************************************************************************
 * DMM hooks
 */
extern uint32_t LL_AbortedCback( uint8 preempted );

void MAP_llDmmSetThreshold( uint8 state, uint8 handle, uint8 reset )
{
#ifdef USE_DMM
  llDmmSetThreshold(state, handle, reset);
#endif
}

uint32 MAP_llDmmGetActivityIndex( uint16 cmdNum )
{
#ifdef USE_DMM
  return llDmmGetActivityIndex(cmdNum);
#else
  return 0;
#endif
}

uint8 MAP_llDmmSetAdvHandle( uint8 handle, uint8 clear )
{
#ifdef USE_DMM
  return llDmmSetAdvHandle(handle, clear);
#else
  return 0;
#endif
}

void MAP_llDmmDynamicFree( void )
{
#ifdef USE_DMM
  llDmmDynamicFree();
#endif
}

uint8 MAP_llDmmDynamicAlloc( void )
{
#ifdef USE_DMM
  return llDmmDynamicAlloc();
#else
  return 0;
#endif
}

uint8 MAP_llSetStarvationMode(uint16 connId, uint8 setOnOffValue)
{
#ifdef USE_DMM
  return LL_INACTIVE_CONNECTIONS;
#else
  return llSetStarvationMode(connId, setOnOffValue);
#endif
}

uint8 MAP_llGetStarvationMode(uint16 connId)
{
#ifdef USE_DMM
  return LL_INACTIVE_CONNECTIONS;
#else
  return llGetStarvationMode(connId);
#endif
}

void MAP_llCentral_TaskEnd(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llCentral_TaskEnd();
#endif
  return;
}

void MAP_llPeripheral_TaskEnd(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & ADV_CONN_CFG)
  llPeripheral_TaskEnd();
#endif
  return;
}

void MAP_llInitFeatureSet( void )
{
  llInitFeatureSet();

#ifndef USE_AE
  llRemoveFromFeatureSet(1, LL_FEATURE_EXTENDED_ADVERTISING);
#endif // !USE_AE

#if !defined(USE_PERIODIC_ADV) && !defined(USE_PERIODIC_SCAN)
  llRemoveFromFeatureSet(1, LL_FEATURE_PERIODIC_ADVERTISING);
#endif // !USE_PERIODIC_ADV && !USE_PERIODIC_SCAN

#ifndef USE_PERIODIC_ADV
  llRemoveFromFeatureSet(2, LL_FEATURE_CONNECTIONLESS_CTE_TRANSMITTER);
#endif //USE_PERIODIC_ADV

#ifndef USE_PERIODIC_SCAN
  llRemoveFromFeatureSet(2, LL_FEATURE_CONNECTIONLESS_CTE_RECEIVER);
#endif //USE_PERIODIC_SCAN

#ifndef RTLS_CTE
  llRemoveFromFeatureSet(2, LL_FEATURE_CONNECTION_CTE_REQUEST);
  llRemoveFromFeatureSet(2, LL_FEATURE_CONNECTION_CTE_RESPONSE);
  llRemoveFromFeatureSet(2, LL_FEATURE_ANTENNA_SWITCHING_DURING_CTE_RX);
  llRemoveFromFeatureSet(2, LL_FEATURE_RECEIVING_CTE);
  llRemoveFromFeatureSet(2, LL_FEATURE_CONNECTIONLESS_CTE_TRANSMITTER);
  llRemoveFromFeatureSet(2, LL_FEATURE_CONNECTIONLESS_CTE_RECEIVER);
#endif // RTLS_CTE
}

#ifndef CC23X0
/*******************************************************************************
 * Coex hooks
 */
#ifdef USE_COEX
extern llCoexParams_t llCoexSetParams(uint16 cmdNum, RF_ScheduleCmdParams *pCmdParams);
extern void llCoexInit(uint8 enable);
extern void llCoexUpdateCounters(uint8 grant);
#endif

void MAP_llCoexSetParams(uint16 cmdNum, void *pCmdParams)
{
#ifdef USE_COEX
  llCoexSetParams(cmdNum,pCmdParams);
#endif
}

void MAP_llCoexInit(uint8 enable)
{
#ifdef USE_COEX
  llCoexInit(enable);
#endif
}

void MAP_llCoexUpdateCounters(uint8 grant)
{
#ifdef USE_COEX
  llCoexUpdateCounters(grant);
#endif
}

uint8 MAP_LL_EXT_CoexEnable(uint8 enable)
{
#ifdef USE_COEX
  return LL_EXT_CoexEnable(enable);
#else
  return 1;
#endif
}

/*******************************************************************************
 * Osprey Coex hooks
 */
void MAP_llCoexSetHighPriorityGroup(uint16 cmdNum)
{
#ifdef CC33xx
#ifdef OSPREY_COEX
  llCoexSetHighPriorityGroup(cmdNum);
#endif
#endif
}

/*******************************************************************************
 * Periodic Adv hooks
 */
uint8 MAP_LE_SetPeriodicAdvParams( uint8 advHandle,
                                   uint16 periodicAdvIntervalMin,
                                   uint16 periodicAdvIntervalMax,
                                   uint16 periodicAdvProp )
{
#ifdef USE_PERIODIC_ADV
  return LE_SetPeriodicAdvParams(advHandle,
                                 periodicAdvIntervalMin,
                                 periodicAdvIntervalMax,
                                 periodicAdvProp);
#else
  return 1;
#endif
}

uint8 MAP_LE_SetPeriodicAdvData( uint8 advHandle, uint8 operation,
                                 uint8 dataLength, uint8 *data )
{
#ifdef USE_PERIODIC_ADV
  return LE_SetPeriodicAdvData( advHandle,operation,dataLength,data );
#else
  return 1;
#endif
}

uint8 MAP_LE_SetPeriodicAdvEnable( uint8 enable,uint8 advHandle )
{
#ifdef USE_PERIODIC_ADV
  return LE_SetPeriodicAdvEnable( enable, advHandle);
#else
  return 1;
#endif
}

uint8 MAP_LE_SetConnectionlessCteTransmitParams( uint8 advHandle, uint8 cteLen, uint8 cteType,
                                                 uint8 cteCount, uint8 length, uint8 *pAntenna )
{
#if defined ( USE_PERIODIC_ADV ) && defined ( RTLS_CTE )
  return LE_SetConnectionlessCteTransmitParams( advHandle, cteLen, cteType, cteCount, length, pAntenna );
#else
  return 1;
#endif
}

uint8 MAP_LE_SetConnectionlessCteTransmitEnable( uint8 advHandle, uint8 enable )
{
#if defined ( USE_PERIODIC_ADV ) && defined ( RTLS_CTE )
  return LE_SetConnectionlessCteTransmitEnable( advHandle, enable);
#else
  return 1;
#endif
}

void *MAP_llGetPeriodicAdv( uint8 handle )
{
#ifdef USE_PERIODIC_ADV
  return llGetPeriodicAdv( handle );
#else
  return NULL;
#endif
}

void MAP_llUpdatePeriodicAdvChainPacket( void )
{
#ifdef USE_PERIODIC_ADV
  llUpdatePeriodicAdvChainPacket();
#endif
}

void MAP_llSetPeriodicAdvChmapUpdate( uint8 set )
{
#ifdef USE_PERIODIC_ADV
  llSetPeriodicAdvChmapUpdate( set );
#endif
}

void MAP_llPeriodicAdv_PostProcess( void )
{
#ifdef USE_PERIODIC_ADV
  llPeriodicAdv_PostProcess();
#endif
}

uint8 MAP_llTrigPeriodicAdv( void *pAdvSet, void *pPeriodicAdv )
{
#ifdef USE_PERIODIC_ADV
  return llTrigPeriodicAdv( pAdvSet, pPeriodicAdv);
#else
  return 0;
#endif
}

uint8 MAP_llSetupPeriodicAdv( void *pAdvSet )
{
#if defined ( USE_PERIODIC_ADV ) && (!defined( DeviceFamily_CC13X4 ) || !defined( DeviceFamily_CC26X4 ))
  return llSetupPeriodicAdv( pAdvSet );
#else
  return 0;
#endif
}

void MAP_llEndPeriodicAdvTask( void *pPeriodicAdv )
{
#ifdef USE_PERIODIC_ADV
  llEndPeriodicAdvTask( pPeriodicAdv );
#endif
}

void *MAP_llFindNextPeriodicAdv( void )
{
#ifdef USE_PERIODIC_ADV
  return llFindNextPeriodicAdv();
#else
  return NULL;
#endif
}

void MAP_llSetPeriodicSyncInfo( void *pAdvSet, uint8 *pBuf )
{
#ifdef USE_PERIODIC_ADV
  llSetPeriodicSyncInfo(pAdvSet,pBuf);
#endif
}

void *MAP_llGetCurrentPeriodicAdv( void )
{
#ifdef USE_PERIODIC_ADV
  return llGetCurrentPeriodicAdv();
#else
  return NULL;
#endif
}

uint8 MAP_gapAdv_periodicAdvCmdCompleteCBs( void *pMsg )
{
#if ( HOST_CONFIG & ( PERIPHERAL_CFG | BROADCASTER_CFG ) ) && defined(USE_PERIODIC_ADV)
  return gapAdv_periodicAdvCmdCompleteCBs(pMsg);
#else
  return TRUE;
#endif
}

void MAP_llClearPeriodicAdvSets( void )
{
#ifdef USE_PERIODIC_ADV
  llClearPeriodicAdvSets();
#endif // USE_PERIODIC_ADV
}

/*******************************************************************************
 * Periodic Scan hooks
 */
extern void llProcessPeriodicScanSyncInfo( uint8 *pPkt, aeExtAdvRptEvt_t *advEvent, uint32 timeStamp, uint8 phy );
extern ble5OpCmd_t *llFindNextPeriodicScan( void );
extern void llUpdateExtScanAcceptSyncInfo( void );

uint8 MAP_LE_PeriodicAdvCreateSync( uint8  options, uint8  advSID, uint8  advAddrType, uint8  *advAddress,
                                    uint16 skip, uint16 syncTimeout, uint8  syncCteType )
{
#ifdef USE_PERIODIC_SCAN
  return LE_PeriodicAdvCreateSync( options, advSID, advAddrType, advAddress, skip,syncTimeout,syncCteType);
#else
  return 1;
#endif
}

uint8 MAP_LE_PeriodicAdvCreateSyncCancel( void )
{
#ifdef USE_PERIODIC_SCAN
  return LE_PeriodicAdvCreateSyncCancel();
#else
  return 1;
#endif
}

uint8 MAP_LE_PeriodicAdvTerminateSync( uint16 syncHandle )
{
#ifdef USE_PERIODIC_SCAN
  return LE_PeriodicAdvTerminateSync( syncHandle );
#else
  return 1;
#endif
}

uint8 MAP_LE_AddDeviceToPeriodicAdvList( uint8 advAddrType, uint8 *advAddress, uint8 advSID )
{
#ifdef USE_PERIODIC_SCAN
  return LE_AddDeviceToPeriodicAdvList( advAddrType, advAddress, advSID );
#else
  return 1;
#endif
}

uint8 MAP_LE_RemoveDeviceFromPeriodicAdvList( uint8 advAddrType, uint8 *advAddress, uint8 advSID )
{
#ifdef USE_PERIODIC_SCAN
  return LE_RemoveDeviceFromPeriodicAdvList( advAddrType, advAddress, advSID);
#else
  return 1;
#endif
}

uint8 MAP_LE_ClearPeriodicAdvList( void )
{
#ifdef USE_PERIODIC_SCAN
  return LE_ClearPeriodicAdvList();
#else
  return 1;
#endif
}

uint8 MAP_LE_ReadPeriodicAdvListSize( uint8 *listSize )
{
#ifdef USE_PERIODIC_SCAN
  return LE_ReadPeriodicAdvListSize( listSize );
#else
  return 1;
#endif
}

uint8 MAP_LE_SetPeriodicAdvReceiveEnable( uint16 syncHandle, uint8  enable )
{
#ifdef USE_PERIODIC_SCAN
  return LE_SetPeriodicAdvReceiveEnable( syncHandle, enable);
#else
  return 1;
#endif
}

uint8 MAP_LE_SetConnectionlessIqSamplingEnable( uint16 syncHandle, uint8 samplingEnable,
                                                uint8 slotDurations, uint8 maxSampledCtes,
                                                uint8 length, uint8 *pAntenna )
{
#if defined ( USE_PERIODIC_SCAN ) && defined ( RTLS_CTE )
  return LE_SetConnectionlessIqSamplingEnable( syncHandle, samplingEnable, slotDurations, maxSampledCtes, length, pAntenna );
#else
  return 1;
#endif
}

void MAP_llProcessPeriodicScanSyncInfo( uint8 *pPkt, void *advEvent, uint32 timeStamp, uint8 phy )
{
#ifdef USE_PERIODIC_SCAN
  llProcessPeriodicScanSyncInfo( pPkt, advEvent, timeStamp, phy );
#endif
}

void MAP_llEndPeriodicScanTask( void *pPeriodicScan )
{
#ifdef USE_PERIODIC_SCAN
  llEndPeriodicScanTask( pPeriodicScan );
#endif
}

void MAP_llPeriodicScan_PostProcess( void )
{
#ifdef USE_PERIODIC_SCAN
  llPeriodicScan_PostProcess();
#endif
}

void MAP_llProcessPeriodicScanRxFIFO( void )
{
#ifdef USE_PERIODIC_SCAN
  llProcessPeriodicScanRxFIFO();
#endif
}

void *MAP_llFindNextPeriodicScan( void )
{
#ifdef USE_PERIODIC_SCAN
  return llFindNextPeriodicScan();
#else
  return NULL;
#endif
}

void MAP_llTerminatePeriodicScan( void )
{
#ifdef USE_PERIODIC_SCAN
  llTerminatePeriodicScan();
#endif
}

void *MAP_llGetCurrentPeriodicScan( uint8 state )
{
#ifdef USE_PERIODIC_SCAN
  return llGetCurrentPeriodicScan(state);
#else
  return NULL;
#endif
}

void *MAP_llGetPeriodicScan( uint16 handle )
{
#ifdef USE_PERIODIC_SCAN
  return llGetPeriodicScan(handle);
#else
  return NULL;
#endif
}

uint8 MAP_llGetPeriodicScanCteTasks( void )
{
#ifdef USE_PERIODIC_SCAN
  return llGetPeriodicScanCteTasks();
#else
  return 0;
#endif
}

uint8_t MAP_gapScan_periodicAdvCmdCompleteCBs( void *pMsg )
{
#ifdef USE_PERIODIC_SCAN
  #ifdef USE_PERIODIC_RTLS
    hciEvt_CmdComplete_t *pEvt = (hciEvt_CmdComplete_t *)pMsg;
    return RTLSSrv_processHciEvent(pEvt->cmdOpcode, sizeof(pEvt->pReturnParam), pEvt->pReturnParam);
  #else
    return gapScan_periodicAdvCmdCompleteCBs(pMsg);
  #endif
#else
  return TRUE;
#endif
}

uint8_t MAP_gapScan_periodicAdvCmdStatusCBs( void *pMsg )
{
#ifdef USE_PERIODIC_SCAN
  #ifdef USE_PERIODIC_RTLS
    hciEvt_CommandStatus_t *pEvt = (hciEvt_CommandStatus_t *)pMsg;
    return RTLSSrv_processHciEvent(pEvt->cmdOpcode, sizeof(pEvt->cmdStatus), &pEvt->cmdStatus);
 #else
    return gapScan_periodicAdvCmdStatusCBs(pMsg);
 #endif
#else
  return TRUE;
#endif
}

uint8_t MAP_gapScan_processBLEPeriodicAdvCBs( void *pMsg )
{
#ifdef USE_PERIODIC_SCAN
  #ifdef USE_PERIODIC_RTLS
    return RTLSSrv_processPeriodicAdvEvent(pMsg);
  #else
    return gapScan_processBLEPeriodicAdvCBs(pMsg);
  #endif
#else
  return TRUE;
#endif
}

void MAP_llClearPeriodicScanSets( void )
{
#ifdef USE_PERIODIC_SCAN
  llClearPeriodicScanSets();
#endif
}

void MAP_llUpdateExtScanAcceptSyncInfo( void )
{
#ifdef USE_PERIODIC_SCAN
  llUpdateExtScanAcceptSyncInfo();
#endif
}
#endif //!CC23X0
/*******************************************************************************
 * Extended Advertising hooks
 */
uint8 MAP_llSetExtendedAdvParams( void *pAdvSet, void *pCmdParams )
{
#ifdef USE_AE
  return llSetExtendedAdvParams(pAdvSet,pCmdParams);
#else
  return 1;
#endif
}

uint8 MAP_llSetupExtAdv( void *pAdvSet )
{
#ifdef USE_AE
  return llSetupExtAdv(pAdvSet);
#else
  return 1;
#endif
}

llStatus_t MAP_llPostProcessExtendedAdv( void *pAdvSet )
{
#ifdef USE_AE
  return llPostProcessExtendedAdv(pAdvSet);
#else
  return 0;
#endif
}

uint8 MAP_llTxDoneEventHandleStateExtAdv( void *pAdvSet )
{
#ifdef USE_AE
  return llTxDoneEventHandleStateExtAdv(pAdvSet);
#else
  return 0;
#endif
}

void MAP_llSetupExtendedAdvData( void *pAdvSet )
{
#ifdef USE_AE
  llSetupExtendedAdvData(pAdvSet);
#endif
}

uint8 MAP_llSetExtendedAdvReport(void *extAdvRpt,
                                 uint8 *pPkt,
                                 uint16 evtType,
                                 uint8 extHdrFlgs,
                                 uint8 pHdr,
                                 uint8 dataLen,
                                 uint8 **pSyncInfo,
                                 uint8 *secPhy,
                                 uint8 *pChannelIndex)
{
#ifdef USE_AE
  return llSetExtendedAdvReport(extAdvRpt,pPkt,evtType,extHdrFlgs,
                                pHdr,dataLen,pSyncInfo,secPhy,pChannelIndex);
#else
  return 0;
#endif
}

/**
* These hooks created to change the call to the relevant HCI command
* instead of calling the controller directly.
* This is needed to support the relevant command complete events
* that are passed to the application when using BLE3_CMD
* compilation flag
*/
uint8_t LE_SetExtAdvData_hook( void * pMsg )
{
#ifdef BLE3_CMD
  return HCI_LE_SetExtAdvData(pMsg);
#else
  return LE_SetExtAdvData(pMsg);
#endif
}
uint8_t LE_SetExtScanRspData_hook( void * pMsg)
{
#ifdef BLE3_CMD
  return HCI_LE_SetExtScanRspData(pMsg);
#else
  return LE_SetExtScanRspData(pMsg);
#endif
}

uint8_t LE_SetExtAdvEnable_hook( void * pMsg)
{
#ifdef BLE3_CMD
  return HCI_LE_SetAdvStatus(pMsg);
#else
  return LE_SetExtAdvEnable(pMsg);
#endif
}


uint8 MAP_gapAdv_handleAdvHciCmdComplete( void *pMsg )
{
#ifdef BLE3_CMD
  return gapAdv_handleAdvHciCmdComplete(pMsg);
#else
  return TRUE;
#endif
}

/*******************************************************************************
 * Health check
 */
int8 MAP_llHealthCheck( void )
{
#ifdef USE_HEALTH_CHECK
  return llHealthCheck();
#else
  return 1;
#endif
}

void MAP_llHealthUpdate(uint8 state)
{
#ifdef USE_HEALTH_CHECK
  llHealthUpdate(state);
#endif
}

void MAP_llHealthSetThreshold(uint32 connTime, uint32 scanTime, uint32 initTime, uint32 advTime)
{
#ifdef USE_HEALTH_CHECK
  llHealthSetThreshold(connTime, scanTime, initTime, advTime);
#endif
}

/*******************************************************************************
 * Check legacy command status
 */
#ifdef LEGACY_CMD
extern uint8_t checkLegacyHCICmdStatus(uint16_t opcode);
#endif

uint8_t MAP_checkLegacyHCICmdStatus(uint16_t opcode)
{
#ifdef LEGACY_CMD
  return checkLegacyHCICmdStatus(opcode);
#else
  return FALSE;
#endif
}

/*******************************************************************************
 * Check auto feature exchange status
 */
uint8_t MAP_checkAutoFeatureExchangeStatus(void)
{
#ifdef DISABLE_AUTO_FEATURE_REQ
  return FALSE;
#else
  return TRUE;
#endif
}

/*******************************************************************************
 * Check vendor specific events status
 */
uint8_t MAP_checkVsEventsStatus(void)
{
#ifdef DISABLE_VS_EVENTS
  return FALSE;
#else
  return TRUE;
#endif
}

/*******************************************************************************
 * Scan Optimization
 */
uint8 MAP_llAddExtAlAndSetIgnBit(void *extAdvRpt, uint8 ignoreBit)
{
#if defined(SCAN_OPTIMIZATION) && !defined(USE_RCL)
  return llAddExtAlAndSetIgnBit((aeExtAdvRptEvt_t *)extAdvRpt, ignoreBit);
#endif
  return ignoreBit;
}

uint8 MAP_llFlushIgnoredRxEntry(uint8 ignoreBit)
{
#if defined(SCAN_OPTIMIZATION) && !defined(USE_RCL)
  return llFlushIgnoredRxEntry(ignoreBit);
#endif
  return FALSE;
}

void MAP_llSetRxCfg(void)
{
#if defined(SCAN_OPTIMIZATION) && !defined(USE_RCL)
  llSetRxCfg();
#endif
}

/*******************************************************************************
 * Link time configuration functions
 */

// (CENTRAL_CFG | OBSERVER_CFG) functions
uint8 MAP_gapScan_init(void)
{
#if ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )
  return gapScan_init();
#else
  return LL_STATUS_SUCCESS;
#endif
}
uint8 MAP_SM_InitiatorInit(void)
{
#if ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )
  return SM_InitiatorInit();
#else
  return LL_STATUS_SUCCESS;
#endif
}
void MAP_gap_CentConnRegister(void)
{
#if ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )
  gap_CentConnRegister();
#endif
}
void MAP_gapScan_processSessionEndEvt(void* pSession, uint8_t status)
{
#if ( HOST_CONFIG & ( CENTRAL_CFG | OBSERVER_CFG ) )
  gapScan_processSessionEndEvt( pSession, status);
#endif
}

// (PERIPHERAL_CFG | BROADCASTER_CFG) functions
uint8 MAP_gapAdv_init(void)
{
#if ( HOST_CONFIG & ( PERIPHERAL_CFG | BROADCASTER_CFG ) )
  return gapAdv_init();
#else
  return LL_STATUS_SUCCESS;
#endif
}
uint8 MAP_SM_ResponderInit(void)
{
#if ( HOST_CONFIG & ( PERIPHERAL_CFG ) )
  return SM_ResponderInit();
#else
  return LL_STATUS_SUCCESS;
#endif
}
void MAP_gap_PeriConnRegister(void)
{
#if ( HOST_CONFIG & ( PERIPHERAL_CFG ) )
  gap_PeriConnRegister();
#endif
}

// (ADV_CONN_CFG) functions
void MAP_llExtAdv_PostProcess(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & (ADV_NCONN_CFG | ADV_CONN_CFG))
  llExtAdv_PostProcess();
#endif
}

// (SCAN_CFG) functions
void MAP_llExtScan_PostProcess(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & SCAN_CFG)
  llExtScan_PostProcess();
#endif
}

// (INIT_CFG) functions
void MAP_llExtInit_PostProcess(void)
{
#if defined(CTRL_CONFIG) && (CTRL_CONFIG & INIT_CFG)
  llExtInit_PostProcess();
#endif
}

// (L2CAP_COC_CFG) functions
uint8  MAP_l2capSendNextSegment(void)
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capSendNextSegment();
#else
  return ( FALSE );
#endif
}
uint8  MAP_l2capReassembleSegment(uint16 connHandle, void *pPkt )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capReassembleSegment( connHandle, pPkt );
#else
  return ( TRUE );
#endif
}
uint8  MAP_L2CAP_ParseConnectReq( void *pCmd, uint8 *pData, uint16 len )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return L2CAP_ParseConnectReq( pCmd, pData, len );
#else
  return ( FAILURE );
#endif
}
uint8  MAP_l2capParseConnectRsp( void *pCmd, uint8 *pData, uint16 len )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capParseConnectRsp( pCmd, pData, len );
#else
  return ( FAILURE );
#endif
}
uint8  MAP_L2CAP_ParseFlowCtrlCredit( void *pCmd, uint8 *pData, uint16 len )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return L2CAP_ParseFlowCtrlCredit( pCmd, pData, len );
#else
  return ( FAILURE );
#endif
}
uint8  MAP_l2capParseDisconnectReq( void *pCmd, uint8 *pData, uint16 len )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capParseDisconnectReq( pCmd, pData, len );
#else
  return ( FAILURE );
#endif
}
uint8  MAP_l2capParseDisconnectRsp( void *pCmd, uint8 *pData, uint16 len )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capParseDisconnectRsp( pCmd, pData, len );
#else
  return ( FAILURE );
#endif
}
uint8  MAP_L2CAP_DisconnectReq( uint16 CID )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return L2CAP_DisconnectReq( CID );
#else
  return ( INVALIDPARAMETER );
#endif
}
uint16 MAP_l2capBuildDisconnectRsp( uint8 *pBuf, uint8 *pData )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capBuildDisconnectRsp( pBuf, pData );
#else
  return ( FAILURE );
#endif
}
void   MAP_l2capProcessConnectReq( uint16 connHandle, uint8 id, void *pConnReq )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  l2capProcessConnectReq( connHandle, id, pConnReq );
#endif
}
void   MAP_l2capGetCoChannelInfo( void *pCoC, void *pInfo )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  l2capGetCoChannelInfo( pCoC, pInfo );
#endif
}
void   MAP_l2capNotifyChannelEstEvt( void *pChannel, uint8 status, uint16 result )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  l2capNotifyChannelEstEvt( pChannel, status, result );
#endif
}
void  *MAP_l2capFindRemoteCID( uint16 connHandle, uint16 CID )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capFindRemoteCID( connHandle, CID );
#else
  return ( NULL );
#endif
}
void   MAP_l2capNotifyChannelTermEvt( void *pChannel, uint8 status, uint16 reason )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  l2capNotifyChannelTermEvt( pChannel, status, reason );
#endif
}
void  *MAP_l2capFindLocalCID( uint16 CID )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  return l2capFindLocalCID( CID );
#else
  return ( NULL );
#endif
}
void   MAP_l2capDisconnectChannel( void *pChannel, uint16 reason )
{
#if defined (BLE_V41_FEATURES) && (BLE_V41_FEATURES & L2CAP_COC_CFG)
  l2capDisconnectChannel( pChannel, reason );
#endif
}

// (CENTRAL_CFG) functions
uint8 MAP_gapIsInitiating( void )
{
#if (HOST_CONFIG & CENTRAL_CFG)
  return gapIsInitiating();
#else
  return ( FALSE );
#endif
}
uint8 MAP_GapInit_cancelConnect( void )
{
#if (HOST_CONFIG & CENTRAL_CFG)
  return MAP_GapInit_cancelConnect();
#else
  return ( bleIncorrectMode );
#endif
}
uint8 MAP_smpInitiatorContProcessPairingPubKey( void )
{
#if (HOST_CONFIG & CENTRAL_CFG)
  return smpInitiatorContProcessPairingPubKey();
#else
  return LL_STATUS_ERROR_INVALID_PARAMS;
#endif
}
void MAP_gapInit_initiatingEnd( void )
{
#if (HOST_CONFIG & CENTRAL_CFG)
  gapInit_initiatingEnd();
#endif
}
void MAP_gapInit_sendConnCancelledEvt( void )
{
#if (HOST_CONFIG & CENTRAL_CFG)
  gapInit_sendConnCancelledEvt();
#endif
}
/*******************************************************************************
* SDAA module
*/
extern void  llHandleSDAALastCmdDone( void );
extern uint8 llHandleSDAAControlTX( void *nextConnPtr,
                                   void *secTask,
                                   uint8 startTaskType);

void MAP_LL_SDAA_Init( void )
{
#ifdef SDAA_ENABLE
 LL_SDAA_Init();
#endif
}
void MAP_LL_SDAA_RecordTxUsage( uint16 numOfBytes,
                                uint8 phyType,
                                uint8 power,
                                uint8 channel)
{
#ifdef SDAA_ENABLE
 LL_SDAA_RecordTxUsage( numOfBytes,  phyType,  power,  channel );
#endif
}

void MAP_LL_SDAA_HandleSDAALastCmdDone()
{
#ifdef SDAA_ENABLE
  llHandleSDAALastCmdDone();
#endif
}

void MAP_LL_SDAA_AddDwtRecord( uint32 dwT,
                               uint8 task,
                               uint8 index)
{
#ifdef SDAA_ENABLE
 LL_SDAA_AddDwtRecord( dwT, task, index );
#endif
}

void MAP_LL_SDAA_SampleRXWindow( void )
{
#ifdef SDAA_ENABLE
 LL_SDAA_SampleRXWindow();
#endif
}

uint16 MAP_LL_SDAA_GetRXWindowDuration( void )
{
#ifdef SDAA_ENABLE
 return LL_SDAA_GetRXWindowDuration();
#else
 return 0;
#endif
}

void MAP_LL_SDAA_SetChannelInSample( uint8 channel )
{
#ifdef SDAA_ENABLE
 LL_SDAA_SetChannelInSample(channel);
#endif
}

uint8 MAP_llSDAASetupRXWindowCmd(void)
{
#ifdef SDAA_ENABLE
   return llSDAASetupRXWindowCmd();
#else
   return LL_STATUS_SUCCESS;
#endif
}

uint8 MAP_llHandleSDAAControlTX(void            *nextConnPtr,
                               void            *secTask,
                               uint8           startTaskType)
{
#ifdef SDAA_ENABLE
   return llHandleSDAAControlTX(nextConnPtr,
                                secTask,
                                startTaskType);
#else
   return startTaskType;
#endif
}

/*******************************************************************************
 */
