/**************************************************************************************************
  Filename:       MT_OTA.c
  Revised:        $Date: 2013-07-18 12:30:24 -0700 (Thu, 18 Jul 2013) $
  Revision:       $Revision: 34729 $

  Description:    MonitorTest functions for the ZCL OTA Upgrade.


  Copyright 2004-2007 Texas Instruments Incorporated.

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

#ifdef MT_OTA_FUNC

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "zcomdef.h"
#include "rom_jt_154.h"
#include "mt.h"
#include "mt_ota.h"
#include "zcl_ota.h"
#include "npi_config.h"
#include "npi_data.h"

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/**************************************************************************************************
 * GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 * LOCAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 * LOCAL FUNCTIONS
 **************************************************************************************************/

/***************************************************************************************************
 * @fn      MT_OtaRegister
 *
 * @brief   Called to set the task to receive callbacks from the MT OTA.
 *
 * @param   taskId - task identifier
 *
 * @return  void
 ***************************************************************************************************/
void MT_OtaRegister(uint8_t taskId)
{
}

/***************************************************************************************************
 * @fn      MT_OtaCommandProcessing
 *
 * @brief   Process all the MT OTA commands that are issued by the OTA Console tool
 *
 * @param   pBuf - pointer to the msg buffer
 *
 *          | LEN  | CMD0  | CMD1  |  DATA  |
 *          |  1   |   1   |   1   |  0-255 |
 *
 * @return  status
 ***************************************************************************************************/
uint8_t MT_OtaCommandProcessing(uint8_t* pBuf)
{
  uint8_t status = MT_RPC_SUCCESS;
  uint8_t len;
  OTA_MtMsg_t *pMsg;
  uint8_t cmd = pBuf[MT_RPC_POS_CMD1];
  static uint8_t ZStackServiceTaskID = 0xFF;

  if(ZStackServiceTaskID == 0xFF)
  {
      extern uint8_t ZStackTask_getServiceTaskID(void);
      ZStackServiceTaskID = ZStackTask_getServiceTaskID();
  }

    if (cmd == MT_OTA_FILE_READ_RSP || cmd == MT_OTA_NEXT_IMG_RSP)
    {
      len = pBuf[MT_RPC_POS_LEN];
      pMsg = (OTA_MtMsg_t*) OsalPort_msgAllocate(len + sizeof(OTA_MtMsg_t));

      if (pMsg)
      {
        pMsg->hdr.event = MT_SYS_OTA_MSG;
        pMsg->cmd = cmd;
        pMsg->len = len;
        OsalPort_memcpy(pMsg->data, &pBuf[MT_RPC_POS_DAT0], len);

        endPointDesc_t *epDesc;

        /* Look up the endpoint */
        epDesc = afFindEndPointDesc( ZCL_OTA_ENDPOINT );
        if( epDesc != NULL )
        {
          OsalPort_msgSend( ZStackServiceTaskID, (uint8_t *)pMsg );
        }
      }
    }
    else
    {
        status = MT_RPC_ERR_COMMAND_ID;
    }

  return status;
}

/***************************************************************************************************
 * @fn      MT_OtaFileReadReq
 *
 * @brief   Requests a block of a file be read from the remote.
 *
 * @param   pAddr - The addres of the device requsting the data
 * @param   pFileId - Teh id of the image to read from
 * @param       len - Amount of data to read (must be smaller than the max MT message payload len)
 * @param    offset - The offset into the image to start reading from
 *
 * @return  status
 ***************************************************************************************************/
uint8_t MT_OtaFileReadReq(afAddrType_t *pAddr, zclOTA_FileID_t *pFileId, uint8_t len, uint32_t offset)
{
  uint8_t   msgLen;
  uint8_t   *pBuf;
  uint8_t   *p;
  //uint8_t   cmd = (uint8_t) MT_RPC_CMD_AREQ | (uint8_t) MT_RPC_SYS_OTA;
  uint8_t   cmd = (uint8_t) MT_RPC_SYS_OTA;

  uint8_t Buf[MT_OTA_FILE_READ_REQ_LEN];

  // Check if the requested length is longer than the RX receive buffer
  if (len + MT_OTA_FILE_READ_RSP_LEN + SPI_0DATA_MSG_LEN > NPI_TL_BUF_SIZE)
    return 0;

  // Get length
  msgLen = MT_OTA_FILE_READ_REQ_LEN;

  p = pBuf = Buf;

    // Add the file ID
    p = OTA_FileIdToStream(pFileId, p);

    // Add the device address
    p = OTA_AfAddrToStream(pAddr, p);

    // File ofset to read from
    *p++ = BREAK_UINT32(offset, 0);
    *p++ = BREAK_UINT32(offset, 1);
    *p++ = BREAK_UINT32(offset, 2);
    *p++ = BREAK_UINT32(offset, 3);

    *p = len;

    // Send command to server
    MT_BuildAndSendZToolResponse(cmd, MT_OTA_FILE_READ_REQ, msgLen, pBuf);

  return ZSuccess;
}

/***************************************************************************************************
 * @fn      MT_OtaGetImage
 *
 * @brief   Requests the next OTA image for a given device.
 *
 * @param   pAddr - Address of the device requesting the image
 * @param   pFileId - The file ID of the image currently on the device
 * @param   hwVer - The hardware version of the device (optional)
 * @param   ieee - The IEEE address of the device (optional)
 * @param   options - The get image options
 *
 * @return  Status
 *
 ***************************************************************************************************/
uint8_t MT_OtaGetImage(afAddrType_t *pAddr, zclOTA_FileID_t *pFileId, uint16_t hwVer,
                     uint8_t *ieee, uint8_t options)
{
  uint8_t   msgLen;
  uint8_t   *pBuf;
  uint8_t   *p;
  //uint8_t   cmd = (uint8_t) MT_RPC_CMD_AREQ | (uint8_t) MT_RPC_SYS_OTA;
  uint8_t   cmd = (uint8_t) MT_RPC_SYS_OTA;

  uint8_t Buf[MT_OTA_GET_IMG_MSG_LEN];

  p = pBuf = Buf;

  // Get length
  msgLen = MT_OTA_GET_IMG_MSG_LEN;

    // Add the file ID
    p = OTA_FileIdToStream(pFileId, p);

    // Add the device address
    p = OTA_AfAddrToStream(pAddr, p);

    // Add the options
    *p++ = options;

    // Add the hardware ID (optional)
    *p++ = LO_UINT16(hwVer);
    *p = HI_UINT16(hwVer);

    if (ieee)
      OsalPort_memcpy(p, ieee, Z_EXTADDR_LEN);

    // Send command to server
    MT_BuildAndSendZToolResponse(cmd, MT_OTA_NEXT_IMG_REQ, msgLen, pBuf);

  return ZSuccess;
}

/***************************************************************************************************
 * @fn      MT_OtaSendStatus
 *
 * @brief   Sends the status of the OTA transfer.
 *          eg. "MT_OTA_DL_COMPLETE" to the PC OTA Console tool
 *
 * @param   shortAddr - Short Address of the device the status relates to
 * @param   type - The type of status being reported
 * @param   status - The status value
 * @param   optional - Optional type specific additional information
 *
 * @return  status
 *
 ***************************************************************************************************/
uint8_t MT_OtaSendStatus(uint16_t shortAddr, uint8_t type, uint8_t status, uint8_t optional)
{
  uint8_t   msgLen;
  uint8_t   *pBuf;
  uint8_t   *p;
  //uint8_t   cmd = (uint8_t) MT_RPC_CMD_AREQ | (uint8_t) MT_RPC_SYS_OTA;
  uint8_t   cmd = (uint8_t) MT_RPC_SYS_OTA;

  uint8_t Buf[7];

  // Get length
  msgLen = 7;

  p = pBuf = Buf;

    // Add message parameters
    *p++ = LO_UINT16(_NIB.nwkPanId);
    *p++ = HI_UINT16(_NIB.nwkPanId);
    *p++ = LO_UINT16(shortAddr);
    *p++ = HI_UINT16(shortAddr);
    *p++ = type;
    *p++ = status;
    *p = optional;

    // Send command to server
    MT_BuildAndSendZToolResponse(cmd, MT_OTA_STATUS_IND, msgLen, pBuf);


  return ZSuccess;

}

#endif   // MT_OTA_FUNC
