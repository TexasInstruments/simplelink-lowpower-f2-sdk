/***************************************************************************************************
  Filename:       MT_GP.c
  Revised:        $Date: 2016-06-21 01:06:52 -0700 (Thu, 21 July 2016) $
  Revision:       $Revision:  $

  Description:    MonitorTest functions GP interface.

  Copyright 2007-2013 Texas Instruments Incorporated.

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

 ***************************************************************************************************/

/***************************************************************************************************
 * INCLUDES
 ***************************************************************************************************/


#include "zcomdef.h"
#include "mt.h"
#include "mt_gp.h"


/***************************************************************************************************
* CONST
***************************************************************************************************/
#define GP_DATA_REQ_PAYLOAD_LEN_POS   17
#define GP_DATA_REQ_APP_ID_POS         2

#define SEC_KEY_LEN                   16

/***************************************************************************************************
* LOCAL FUNCTIONs
***************************************************************************************************/

#if defined (MT_GP_CB_FUNC)
static void MT_GpDataReq(uint8_t* pBuf);
static void MT_GpSecRsp(uint8_t* pBuf);

void MT_GPDataCnf(gp_DataCnf_t* gp_DataCnf);
void MT_GPSecReq(gp_SecReq_t* gp_SecReq);
void MT_GPDataInd(gp_DataInd_t* gp_DataInd);



/***************************************************************************************************
* External variables
***************************************************************************************************/


uint8_t MT_GpCommandProcessing(uint8_t *pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;

  switch (pBuf[MT_RPC_POS_CMD1])
  {
//GP_UPDATE
    case MT_GP_DATA_REQ:
      MT_GpDataReq(pBuf);
    break;
    case MT_GP_SEC_RSP:
      MT_GpSecRsp(pBuf);
    break;


  }
  return status;
}


 /***************************************************************************************************
 * @fn      MT_GpDataReq
 *
 * @brief   GP data request interface
 *
 * @param   pBuf - gp data request
 *
 * @return  void
 ***************************************************************************************************/
 static void MT_GpDataReq(uint8_t* pBuf)
{
  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  gp_DataReq_t *gp_DataReq;
  uint8_t  payloadLen;


  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  //Get the payload length
  payloadLen = pBuf[GP_DATA_REQ_PAYLOAD_LEN_POS];

  gp_DataReq = (gp_DataReq_t*)OsalPort_msgAllocate(sizeof(gp_DataReq_t) + payloadLen);

  //No memory
  if(gp_DataReq == NULL)
  {
    retValue = FAILURE;
  }
  //Invalid application ID
  if( (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_GPID) && (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_IEEE ) )
  {
    retValue = INVALIDPARAMETER;
  }
  //Return fail/InvalidParameter
  if(retValue)
  {
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_GP), cmdId, 1, &retValue);
    return;
  }


  gp_DataReq->Action = *pBuf++;
  gp_DataReq->TxOptions = *pBuf++;
  gp_DataReq->gpdID.appID = *pBuf++;
  if(gp_DataReq->gpdID.appID == GP_APP_ID_GPID)
  {
    gp_DataReq->gpdID.id.srcID = OsalPort_buildUint32(pBuf,4);
    pBuf += sizeof(uint32_t) + Z_EXTADDR_LEN;
  }
  else
  {
    pBuf += sizeof(uint32_t);
    OsalPort_memcpy(gp_DataReq->gpdID.id.gpdExtAddr,pBuf,Z_EXTADDR_LEN);
    pBuf += Z_EXTADDR_LEN;
  }

  gp_DataReq->EndPoint = *pBuf++;
  gp_DataReq->GPDCmmdId = *pBuf++;
  gp_DataReq->GPDasduLength = *pBuf++;
  OsalPort_memcpy(gp_DataReq->GPDasdu,pBuf,payloadLen);
  pBuf += payloadLen;
  gp_DataReq->GPEPhandle = *pBuf++;
  gp_DataReq->gpTxQueueEntryLifeTime[2] = *pBuf++;
  gp_DataReq->gpTxQueueEntryLifeTime[1] = *pBuf++;
  gp_DataReq->gpTxQueueEntryLifeTime[0] = *pBuf++;

  gp_DataReq->hdr.event = GP_DATA_REQ;
  gp_DataReq->hdr.status = 0;

  OsalPort_msgSend(gp_TaskID,(uint8_t*)gp_DataReq);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_GP), cmdId, 1, &retValue);
}



/***************************************************************************************************
* @fn      MT_GpSecRsp
*
* @brief   GP Sec Response interface
*
* @param   pBuf - gp sec rsp
*
* @return  void
***************************************************************************************************/

void MT_GpSecRsp(uint8_t *pBuf)
{

  uint8_t retValue = ZSuccess;
  uint8_t cmdId;
  gp_SecRsp_t *gp_SecRsp;

  /* parse header */
  cmdId = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  gp_SecRsp = (gp_SecRsp_t*)OsalPort_msgAllocate(sizeof(gp_SecRsp_t));

    //No memory
  if(gp_SecRsp == NULL)
  {
    retValue = FAILURE;
  }
  //Invalid application ID
  if( (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_GPID) && (pBuf[GP_DATA_REQ_APP_ID_POS] != GP_APP_ID_IEEE ) )
  {
    retValue = INVALIDPARAMETER;
  }
  //Return fail/InvalidParameter
  if(retValue)
  {
    /* Build and send back the response */
    MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_GP), cmdId, 1, &retValue);
    return;
  }

  gp_SecRsp->Status = *pBuf++;
  gp_SecRsp->dGPStubHandle = *pBuf++;
  gp_SecRsp->gpdID.appID = *pBuf++;
  if(gp_SecRsp->gpdID.appID == GP_APP_ID_GPID)
  {
    gp_SecRsp->gpdID.id.srcID = OsalPort_buildUint32(pBuf,4);
    pBuf += sizeof(uint32_t) + Z_EXTADDR_LEN;
  }
  else
  {
    pBuf += sizeof(uint32_t);
    OsalPort_memcpy(gp_SecRsp->gpdID.id.gpdExtAddr,pBuf,Z_EXTADDR_LEN);
    pBuf += Z_EXTADDR_LEN;
  }
  gp_SecRsp->EndPoint = *pBuf++;
  gp_SecRsp->gp_SecData.GPDFSecLvl = *pBuf++;
  gp_SecRsp->gp_SecData.GPDFKeyType = *pBuf++;
  OsalPort_memcpy(gp_SecRsp->GPDKey,pBuf,SEC_KEY_LEN);
  pBuf += SEC_KEY_LEN;
  gp_SecRsp->gp_SecData.GPDSecFrameCounter = OsalPort_buildUint32(pBuf,4);

  gp_SecRsp->hdr.event = GP_SEC_RSP;
  gp_SecRsp->hdr.status = 0;

  OsalPort_msgSend(gp_TaskID,(uint8_t*)gp_SecRsp);

  /* Build and send back the response */
  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_SRSP | (uint8_t)MT_RPC_SYS_GP), cmdId, 1, &retValue);
}

/***************************************************************************************************
* @fn      MT_GPDataInd
*
* @brief   Send GP Data Ind to Host Processor
*
* @param   gp_DataInd
*
* @return  void
***************************************************************************************************/
void MT_GPDataInd(gp_DataInd_t* gp_DataInd)
{
  uint8_t *pBuf = NULL;
  uint8_t *tempBuf = NULL;
  uint8_t bufLen = sizeof(gp_DataInd_t) + gp_DataInd->GPDasduLength - 1;

  pBuf = OsalPort_malloc(bufLen);

  if(pBuf == NULL)
  {
    return;
  }

  tempBuf = pBuf;

  *pBuf++ = gp_DataInd->status;
  *pBuf++ = gp_DataInd->Rssi;
  *pBuf++ = gp_DataInd->LinkQuality;
  *pBuf++ = gp_DataInd->SeqNumber;
  *pBuf++ = gp_DataInd->srcAddr.addrMode;
  *pBuf++ = LO_UINT16(gp_DataInd->srcPanID);
  *pBuf++ = HI_UINT16(gp_DataInd->srcPanID);

   pBuf   = osal_cpyExtAddr( pBuf,gp_DataInd->srcAddr.addr.extAddr);
  *pBuf++ = gp_DataInd->frameType;
  *pBuf++ = gp_DataInd->appID;
  *pBuf++ = gp_DataInd->GPDFSecLvl;
  *pBuf++ = gp_DataInd->GPDFKeyType;
  *pBuf++ = gp_DataInd->AutoCommissioning;
  *pBuf++ = gp_DataInd->RxAfterTx;

  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_DataInd->SrcId);
  *pBuf++ = gp_DataInd->EndPoint;
  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_DataInd->GPDSecFrameCounter);
  *pBuf++ = gp_DataInd->GPDCmmdID;
  *pBuf++ = gp_DataInd->GPDasduLength;
  OsalPort_memcpy(pBuf,gp_DataInd->GPDasdu,gp_DataInd->GPDasduLength);
  pBuf += gp_DataInd->GPDasduLength;
  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_DataInd->MIC);

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_GP), MT_GP_DATA_IND, 37 + gp_DataInd->GPDasduLength - 1, tempBuf);

  OsalPort_free(tempBuf);

}




/***************************************************************************************************
* @fn      MT_GPDataCnf
*
* @brief   Send GP Data Cnf to Host Processor
*
* @param   gp_DataCnf
*
* @return  void
***************************************************************************************************/
void MT_GPDataCnf(gp_DataCnf_t* gp_DataCnf)
{
  uint8_t buf[2];
  uint8_t *pBuf = buf;

  *pBuf++ = gp_DataCnf->status;
  *pBuf++ = gp_DataCnf->GPEPhandle;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_GP), MT_GP_DATA_CNF, 2, buf);
}

/***************************************************************************************************
* @fn      MT_GPSecReq
*
* @brief   Send GP Sec Req to Host Processor
*
* @param   gp_SecReq
*
* @return  void
***************************************************************************************************/
void MT_GPSecReq(gp_SecReq_t* gp_SecReq)
{
  uint8_t buf[21];
  uint8_t *pBuf = buf;


  *pBuf++ = gp_SecReq->gpdID.appID;

  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_SecReq->gpdID.id.srcID);
  pBuf   = osal_cpyExtAddr( pBuf,gp_SecReq->gpdID.id.gpdExtAddr);
  *pBuf++ = gp_SecReq->EndPoint;
  *pBuf++ = gp_SecReq->gp_SecData.GPDFSecLvl;
  *pBuf++ = gp_SecReq->gp_SecData.GPDFKeyType;

  UINT32_TO_BUF_LITTLE_ENDIAN(pBuf,gp_SecReq->gp_SecData.GPDSecFrameCounter);

  *pBuf++ = gp_SecReq->dGPStubHandle;

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_GP), MT_GP_SEC_REQ, 21, buf);
}



#endif

