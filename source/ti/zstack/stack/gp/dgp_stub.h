/**************************************************************************************************
  Filename:       cgp_stub.h
  Revised:        $Date: 2016-05-23 11:51:49 -0700 (Mon, 23 May 2016) $
  Revision:       $Revision: - $

  Description:    This file contains the common Green Power stub definitions.


  Copyright 2006-2014 Texas Instruments Incorporated.

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



#ifndef DGP_STUB_H
#define DGP_STUB_H


#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
 * INCLUDES
 */
#include "cgp_stub.h"

 /*********************************************************************
 * CONSTANTS
 */

 /*********************************************************************
 * MACROS
 */


#define GP_NONCE_SEC_CONTROL_OUTGOING_APP_ID_GP   0xC5
#define GP_NONCE_SEC_CONTROL                      0x05


#define GP_TRANSMIT_SCHEDULED_FRAME_EVENT     0x0001
#define GP_DUPLICATE_FILTERING_TIMEOUT_EVENT  0x0002
#define GP_CHANNEL_CONFIGURATION_TIMEOUT      0x0004
#define GP_PROXY_ALIAS_CONFLICT_TIMEOUT       0x0008

 /*********************************************************************
 * TYPEDEFS
 */


typedef void   (*GP_DataCnfGCB_t)(gp_DataCnf_t* gp_DataCnf);
typedef void   (*GP_endpointInitGCB_t)(void);
typedef void   (*GP_expireDuplicateFilteringGCB_t)(void);
typedef void   (*GP_stopCommissioningModeGCB_t)(void);
typedef void   (*GP_returnOperationalChannelGCB_t)(void);
typedef void   (*GP_DataIndGCB_t)(gp_DataInd_t* gp_DataInd);
typedef void   (*GP_SecReqGCB_t)(gp_SecReq_t* gp_SecReq);
typedef void   (*GP_CheckAnnouncedDevice_t) ( uint8_t *sinkIEEE, uint16_t sinkNwkAddr );

typedef struct
{
gpEventHdr_t hdr;
uint32_t       timestamp;         //Timestamp in backoff units
sAddr_t      srcAddr;
sAddr_t      dstAddr;
uint16_t       srcPanID;
uint16_t       dstPanID;
int8_t         Rssi;
uint8_t        LinkQuality;
uint8_t        SeqNumber;
uint8_t        mpduLen;
uint8_t        mpdu[1];            //This is a place holder for the buffer, its length depends on mpdu_len
}dgp_DataInd_t;


#define GP_NONCE_LENGTH   13
typedef struct
{
uint8_t  SourceAddr[Z_EXTADDR_LEN];
uint32_t FrameCounter;
uint8_t  securityControl;
}gp_AESNonce_t;



/*********************************************************************
 * GLOBAL VARIABLES
 */

/* Callbacks for GP endpoint */
extern GP_DataCnfGCB_t                   GP_DataCnfGCB;
extern GP_expireDuplicateFilteringGCB_t  GP_expireDuplicateFilteringGCB;
extern GP_stopCommissioningModeGCB_t     GP_stopCommissioningModeGCB;
extern GP_DataIndGCB_t                   GP_DataIndGCB;
extern GP_SecReqGCB_t                    GP_SecReqGCB;
extern GP_CheckAnnouncedDevice_t         GP_CheckAnnouncedDeviceGCB;

 /*********************************************************************
 * FUNCTION MACROS
 */


 /*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief       Handles GPDF to pass to application as GP SecReq to be validated.
 */
extern void dGP_DataInd(dgp_DataInd_t *dgp_DataInd);

/*
 * @brief       Notify the application about a GPDF being delivered
 */
extern void dGP_DataCnf(uint8_t GPmpduHandle);

/*
 * @brief   Find entry by handle in the list of dGP-DataInd pendings by GP-SecReq
 */
extern uint8_t* dGP_findHandle(uint8_t handle);

/*
 * @brief       Primitive by GP EndPoint to pass to dGP stub a request to send GPDF to a GPD
 */
extern bool GP_DataReq(gp_DataReq_t *gp_DataReq);

/*
 * @brief       Response to dGP stub from GP endpoint on how to process the GPDF
 */
extern void  GP_SecRsp(gp_SecRsp_t *gp_SecRsp);

#if (defined (USE_ICALL) || defined (OSAL_PORT2TIRTOS))
/*
 * @brief       Send a Green Power security response to Green Power Stub
 */
extern void sendGpSecRsp(gp_SecRsp_t *rsp);

/*
 * @brief       Resolves Address Conflict
 */
extern void GpResolveAddressConflict(uint16_t nwkAddr, bool conflict);

/*
 * @brief   Security and authentication processing function dedicated to GP.
 */
extern ZStatus_t gp_ccmStarKey(gpdID_t *gpdId, uint8_t keyType, uint8_t *pEncKey, uint8_t *pMic, uint8_t *pSecKey, uint32_t secFrameCounter, uint8_t encrypt);
#endif

/*
* @brief   Security and authentification processing function dedicated to GP.
*/
extern uint8_t gp_ccmStar(gp_DataInd_t *gpDataInd, uint8_t* key);

#ifdef __cplusplus
}
#endif


#endif /* DGP_STUB_H */


