/******************************************************************************

 @file  mt_mac.h

 @brief Monitor/Test command/response definitions for MAC subsystem

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2025, Texas Instruments Incorporated
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
#ifndef MTMAC_H
#define MTMAC_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "api_mac.h"

#include "mt.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Global Variables
 *****************************************************************************/

/*! Transmit Delay for Green Power*/
extern uint8_t GP_Offset;

/******************************************************************************
 Public Function Prototypes
 *****************************************************************************/
/*!
 * @brief   Process incoming MT MAC command messages
 *
 * @param   pMpb - pointer to incoming message parameter block
 *
 * @return  error - command processing error status
 */
extern uint8_t MtMac_commandProcessing(Mt_mpb_t *pMpb);

/*!
 * @brief   Process MAC_ASSOCIATE_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_AssociateCnf(ApiMac_mlmeAssociateCnf_t *pCnf);

/*!
 * @brief   Process MAC_ASSOCIATE_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_AssociateInd(ApiMac_mlmeAssociateInd_t *pInd);

/*!
 * @brief   Process MAC_ASYNC_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_AsyncCnf(ApiMac_mlmeWsAsyncCnf_t *pCnf);

/*!
 * @brief   Process MAC_ASYNC_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_AsyncInd(ApiMac_mlmeWsAsyncInd_t *pInd);

/*!
 * @brief   Process MAC_BEACON_NOTIFY_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_BeaconNotifyInd(ApiMac_mlmeBeaconNotifyInd_t *pInd);

/*!
 * @brief   Process MAC_COMM_STATUS_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_CommStatusInd(ApiMac_mlmeCommStatusInd_t *pInd);

/*!
 * @brief   Process MAC_DATA_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_DataCnf(ApiMac_mcpsDataCnf_t *pCnf);

/*!
 * @brief   Process MAC_DATA_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_DataInd(ApiMac_mcpsDataInd_t *pInd);

/*!
 * @brief   Process MAC_DISASSOCIATE_IND callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_DisassociateCnf(ApiMac_mlmeDisassociateCnf_t *pCnf);

/*!
 * @brief   Process MAC_DISASSOCIATE_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_DisassociateInd(ApiMac_mlmeDisassociateInd_t *pInd);

/*!
 * @brief   Process MAC_ORPHAN_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_OrphanInd(ApiMac_mlmeOrphanInd_t *pInd);

/*!
 * @brief   Process MAC_POLL_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_PollCnf(ApiMac_mlmePollCnf_t *pCnf);

/*!
 * @brief   Process MAC_POLL_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_PollInd(ApiMac_mlmePollInd_t *pInd);

/*!
 * @brief   Process MAC_PURGE_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_PurgeCnf(ApiMac_mcpsPurgeCnf_t *pCnf);

/*!
 * @brief   Process MAC_SCAN_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_ScanCnf(ApiMac_mlmeScanCnf_t *pCnf);

/*!
 * @brief   Update enabled MT MAC callbacks
 *
 * @param   cbBits - bit mask of affected callbacks
 *
 * @return  Current MT MAC callback enabled bit mask
 */
extern uint32_t MtMac_setCallbacks(uint32_t cbBits);

/*!
 * @brief   Process MAC_START_CNF callback issued by MAC
 *
 * @param   pCnf - pointer to incoming callback data structure
 */
extern void MtMac_StartCnf(ApiMac_mlmeStartCnf_t *pCnf);

/*!
 * @brief   Process MAC_SYNC_LOSS_IND callback issued by MAC
 *
 * @param   pInd - pointer to incoming callback data structure
 */
extern void MtMac_SyncLossInd(ApiMac_mlmeSyncLossInd_t *pInd);

#ifdef __cplusplus
}
#endif

#endif /* MTMAC_H */
