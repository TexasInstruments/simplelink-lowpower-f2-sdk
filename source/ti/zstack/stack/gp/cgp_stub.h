/**************************************************************************************************
  Filename:       cGP_stub.h
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



#ifndef CGP_STUB_H
#define CGP_STUB_H


#ifdef __cplusplus
extern "C"
{
#endif



/*********************************************************************
 * INCLUDES
 */

#include "zcomdef.h"
#include "zmac.h"
#include "ssp.h"



/*********************************************************************
 * MACROS
 */

#define GP_ZIGBEE_PROTOCOL_VER    0x03

//GP Nwk FrameControl
//Masks
#define GP_NWK_FRAME_CTRL_EXT_MASK      0x80
#define GP_AUTO_COMM_MASK               0x40
#define GP_ZIGBEE_PROTOCOL_VERSION_MASK 0x3C
#define GP_FRAME_TYPE_MASK              0x03
//Possitions
#define GP_NWK_FRAME_CTRL_EXT_POS       7
#define GP_AUTO_COMM_POS                6
#define GP_ZIGBEE_PROTOCOL_VERSION_POS  2
#define GP_FRAME_TYPE_POS               0

#define GP_NWK_FRAME_TYPE_MASK          0x03
#define GP_NWK_FRAME_TYPE_POS           0
#define GP_NWK_FRAME_TYPE_MAINTENANCE   1
#define GP_NWK_FRAME_TYPE_DATA          0


//GP ExtendedNwkFrameControl
#define GP_EXT_NWK_APP_ID_MASK           0x07
#define GP_EXT_NWK_APP_ID_POS            0
//defined by application
#define GP_EXT_NWK_APP_SEC_LVL_MASK      0x18
#define GP_EXT_NWK_APP_SEC_LVL_POS       3
#define GP_EXT_NWK_APP_SEC_KEY_TYPE_MASK 0x20
#define GP_EXT_NWK_APP_SEC_KEY_TYPE_POS  5
#define GP_EXT_NWK_APP_RX_AFTER_TX_MASK  0x40
#define GP_EXT_NWK_APP_RX_AFTER_TX_POS   6
#define GP_EXT_NWK_APP_DIRECTION_MASK    0x80
#define GP_EXT_NWK_APP_DIRECTION_POS     7


#define GP_SRC_ID_UNSPECIFIED           0
#define GP_SRC_ID_RESERVED_RANGE_LOW    0xFFFFFFF9
#define GP_SRC_ID_RESERVED_RANGE_HIGH   0xFFFFFFFE

#define GP_APP_ID_GPID                  0
#define GP_APP_ID_LPED                  1
#define GP_APP_ID_IEEE                  2

#define GP_ENDPOINT_RESERVED_RANGE_LOW  0xF1
#define GP_ENDPOINT_RESERVED_RANGE_HIGH 0xFE
#define GP_ENDPOINT_ALL                 0xFF

#define GP_SECURITY_LVL_NO_SEC             0
#define GP_SECURITY_LVL_RESERVED           1
#define GP_SECURITY_LVL_4FC_4MIC           2
#define GP_SECURITY_LVL_4FC_4MIC_ENCRYPT   3

#define GP_SECURITY_MIC_SIZE               4

#define GP_CMD_ID_F0                       0xF0
#define GP_CMD_ID_FF                       0xFF


//TxOptions for GP-Data.Request
#define GP_OPT_USE_TX_QUEUE_MASK                0x01
#define GP_OPT_USE_CSMA_CA_MASK                 0x02
#define GP_OPT_USE_MAC_ACK_MASK                 0x04
#define GP_OPT_TX_FRAME_TYPE_MAINTENANCE_MASK   0x10
#define GP_OPT_TX_FRAME_TYPE_DATA_MASK          0x00
#define GP_OPT_TX_ON_MATCHING_ENDPOINT_MASK     0x20


//TxOptions for cGP-Data.Request
#define CGP_OPT_USE_CSMA_CA_MASK                0x01
#define CGP_OPT_USE_MAC_ACK_MASK                0x02


#define GP_OPT_GPDF_TYPE_FOR_TX_MASK            0x18
#define GP_OPT_GPDF_TYPE_FOR_TX_POS             3

 /*********************************************************************
 * CONSTANTS
 */



 /*********************************************************************
 * TYPEDEFS
 */

enum
{
GP_MAC_MCPS_DATA_CNF, //Related to Common GP stub
GP_MAC_MCPS_DATA_IND, //Related to Common GP stub
DGP_DATA_IND,      //Related to dedicated GP stub
DLPED_DATA_IND,    //Related to DLPE  not supported
GP_DATA_IND,       //Related to GP endpoint
GP_DATA_REQ,       //Related to GP endpoint
GP_DATA_CNF,       //Related to GP endpoint
GP_SEC_REQ,        //Related to GP endpoint
GP_SEC_RSP,        //Related to GP endpoint
};


enum
{
GP_DATA_CNF_TX_QUEUE_FULL,
GP_DATA_CNF_ENTRY_REPLACED,
GP_DATA_CNF_ENTRY_ADDED,
GP_DATA_CNF_ENTRY_EXPIRED,
GP_DATA_CNF_ENTRY_REMOVED,
GP_DATA_CNF_GPDF_SENDING_FINALIZED,
};

enum
{
GP_SEC_RSP_DROP_FRAME,
GP_SEC_RSP_MATCH,
GP_SEC_RSP_PASS_UNPROCESSED,
GP_SEC_RSP_TX_THEN_DROP,
GP_SEC_RSP_ERROR,
GP_SEC_RSP_NO_ENTRY,
};

enum
{
GP_DATA_IND_STATUS_SECURITY_SUCCESS,
GP_DATA_IND_STATUS_NO_SECURITY,
GP_DATA_IND_STATUS_COUNTER_FAILURE,
GP_DATA_IND_STATUS_AUTH_FAILURE,
GP_DATA_IND_STATUS_UNPROCESSED
};


/* GP event header type */
typedef struct
{
  uint8_t   event;              /* MAC event */
  uint8_t   status;             /* MAC status */
} gpEventHdr_t;



typedef struct
{
uint8_t                  dGPStubHandle;
struct gp_DataInd_tag  *next;
uint32_t                 timeout;
}gp_DataIndSecReq_t;

typedef struct gp_DataInd_tag
{
gpEventHdr_t        hdr;
gp_DataIndSecReq_t  SecReqHandling;
uint32_t              timestamp;
uint8_t               status;
int8_t                Rssi;
uint8_t               LinkQuality;
uint8_t               SeqNumber;
sAddr_t             srcAddr;
uint16_t              srcPanID;
uint8_t               frameType;
uint8_t               appID;
uint8_t               GPDFSecLvl;
uint8_t               GPDFKeyType;
bool                AutoCommissioning;
bool                RxAfterTx;
uint32_t              SrcId;
uint8_t               EndPoint;
uint32_t              GPDSecFrameCounter;
uint8_t               GPDCmmdID;
uint32_t              MIC;
uint8_t               GPDasduLength;
uint8_t               GPDasdu[1];         //This is a place holder for the buffer, the length depends on GPDasduLength
}gp_DataInd_t;


typedef struct
{
uint8_t        appID;
union
  {
    uint32_t       srcID;
    uint8_t        gpdExtAddr[Z_EXTADDR_LEN];
  }id;
}gpdID_t;


typedef struct
{
gpEventHdr_t hdr;
bool         Action;
uint8_t        TxOptions;
gpdID_t      gpdID;
uint8_t        EndPoint;
uint8_t        GPDCmmdId;
uint8_t        GPEPhandle;
uint8_t        gpTxQueueEntryLifeTime[3];
uint8_t        GPDasduLength;
uint8_t        GPDasdu[1];         //This is a place holder for the buffer, the length depends on GPDasduLength
}gp_DataReq_t;

typedef struct
{
gpEventHdr_t   hdr;
uint32_t         timestamp;
ZMacDataReq_t  ZMacDataReq;
}gp_ZMacDataReq_t;

typedef struct
{
  gp_DataReq_t  *gp_DataReq;
}
gp_DataReqPending_t;


typedef struct
{
gpEventHdr_t hdr;
uint8_t        status;
uint8_t        GPEPhandle;
}gp_DataCnf_t;

typedef struct
{
gpEventHdr_t hdr;
uint8_t        status;
uint8_t        MPDUhandle;
}cgp_DataCnf_t;


typedef struct
{
uint8_t        GPDFSecLvl;
uint8_t        GPDFKeyType;
uint32_t       GPDSecFrameCounter;
}gp_SecData_t;


typedef struct
{
gpEventHdr_t hdr;
gpdID_t      gpdID;
uint8_t        EndPoint;
gp_SecData_t gp_SecData;
uint8_t        dGPStubHandle;
}gp_SecReq_t;


typedef struct
{
gpEventHdr_t  hdr;
uint8_t         Status;
uint8_t         dGPStubHandle;
gpdID_t       gpdID;
uint8_t         EndPoint;
gp_SecData_t  gp_SecData;
uint8_t         GPDKey[SEC_KEY_LEN];
}gp_SecRsp_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */

extern byte gp_TaskID;

/*********************************************************************
 * FUNCTION MACROS
 */



/*********************************************************************
 * FUNCTIONS
 */

/*
 * @brief This function reports the results CGP Data request being
 *        handled by the cGP stub
 */

extern void CGP_DataCnf(uint8_t Status, uint8_t GPmpduHandle);

/*
 * @brief Allows dGP or dLPED stub send GPDF to GPD or LPED
 */

extern ZStatus_t CGP_DataReq(uint8_t TxOptions, zAddrType_t *dstAddr, uint8_t mpdu_len, uint8_t*  mpdu, uint8_t mpduHandle, uint32_t timestamp);


/*
 * @brief       Funtion to parse GPDF to pass to dedicated Stub (dGP)
 */
extern void cGP_processMCPSDataInd(macMcpsDataInd_t *pData);

/*
 * @brief Sends GPDF in the calculated window to the GPD.
 */
extern void gp_SendScheduledMacDataReq(void);

#ifdef __cplusplus
}
#endif


#endif /* CGP_STUB_H */










