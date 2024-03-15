/**************************************************************************************************
  Filename:       mt_mac.h
  Revised:        $Date: 2013-06-11 11:14:41 -0700 (Tue, 11 Jun 2013) $
  Revision:       $Revision: 34520 $

  Description:    MonitorTest functions for the MAC layer.


  Copyright 2004-2013 Texas Instruments Incorporated.

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
**************************************************************************************************/

#ifndef MT_MAC_H
#define MT_MAC_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "zcomdef.h"
#include "zmac.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#if defined (MT_MAC_CB_FUNC)
  //MAC Callback subscription IDs
  #define CB_ID_NWK_SYNC_LOSS_IND         0x0001
  #define CB_ID_NWK_ASSOCIATE_IND         0x0002
  #define CB_ID_NWK_ASSOCIATE_CNF         0x0004
  #define CB_ID_NWK_BEACON_NOTIFY_IND     0x0008
  #define CB_ID_NWK_DATA_CNF              0x0010
  #define CB_ID_NWK_DATA_IND              0x0020
  #define CB_ID_NWK_DISASSOCIATE_IND      0x0040
  #define CB_ID_NWK_DISASSOCIATE_CNF      0x0080
  #define CB_ID_NWK_PURGE_CNF             0x0100
  #define CB_ID_NWK_ORPHAN_IND            0x0400
  #define CB_ID_NWK_POLL_CNF              0x0800
  #define CB_ID_NWK_SCAN_CNF              0x1000
  #define CB_ID_NWK_COMM_STATUS_IND       0x2000
  #define CB_ID_NWK_START_CNF             0x4000
  #define CB_ID_NWK_RX_ENABLE_CNF         0x8000
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint16_t _macCallbackSub;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
/*********************************************************************
 * LOCAL FUNCTIONS
 */

#ifdef MT_MAC_FUNC

/*
 * MonitorTest function handling MAC commands
 */
extern uint8_t MT_MacCommandProcessing( uint8_t *pBuf );

#endif   /*MAC Command Processing in MT*/


#if defined ( MT_MAC_CB_FUNC )

/*
 *  Process the callback subscription for nwk_associate_ind
 */
extern void nwk_MTCallbackSubNwkAssociateInd( ZMacAssociateInd_t *param );

/*
 *  Process the callback subscription for nwk_associate_cnf
 */
extern void nwk_MTCallbackSubNwkAssociateCnf( ZMacAssociateCnf_t *param );

/*
 *  Process the callback subscription for nwk_data_cnf
 */
extern void nwk_MTCallbackSubNwkDataCnf( ZMacDataCnf_t *param );

/*
 *  Process the callback subscription for nwk_data_ind
 */
extern void nwk_MTCallbackSubNwkDataInd( ZMacDataInd_t *param );


/*
 * Process the callback subscription for nwk_disassociate_ind
 */
extern void nwk_MTCallbackSubNwkDisassociateInd( ZMacDisassociateInd_t *param );

/*
 *  Process the callback subscription for nwk_disassociate_cnf
 */
extern void nwk_MTCallbackSubNwkDisassociateCnf( ZMacDisassociateCnf_t *param );

/*
 *  Process the callback subscription for nwk_poll_ind
 */
extern void nwk_MTCallbackSubNwkPollInd( ZMacPollInd_t *param );

/*
 *  Process the callback subscription for nwk_orphan_ind
 */
extern void nwk_MTCallbackSubNwkOrphanInd( ZMacOrphanInd_t *param );

/*
 *  Process the callback subscription for nwk_poll_cnf
 */
extern void nwk_MTCallbackSubNwkPollCnf( byte Status );

/*
 *  Process the callback subscription for nwk_scan_cnf
 */
extern void nwk_MTCallbackSubNwkScanCnf( ZMacScanCnf_t *param );

/*
 *  Process the callback subscription for nwk_start_cnf
 */
extern void nwk_MTCallbackSubNwkStartCnf( uint8_t Status );

/*
 *  Process the callback subscription for nwk_syncloss_ind
 */
extern void nwk_MTCallbackSubNwkSyncLossInd( ZMacSyncLossInd_t *param );

/*
 *  Process the callback subscription for nwk_Rx_Enable_cnf
 */
extern void nwk_MTCallbackSubNwkRxEnableCnf ( byte Status );

/*
 *  Process the callback subscription for nwk_Comm_Status_ind
 */
extern void nwk_MTCallbackSubCommStatusInd ( ZMacCommStatusInd_t *param );

/*
 *  Process the callback subscription for nwk_Purge_cnf
 */
extern void nwk_MTCallbackSubNwkPurgeCnf( ZMacPurgeCnf_t *param );

/*
 *  Process the callback subscription for nwk_Beacon_Notify_ind
 */
extern void nwk_MTCallbackSubNwkBeaconNotifyInd ( ZMacBeaconNotifyInd_t *param );
#endif

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MT_MAC_H */
