/**************************************************************************************************
  Filename:       MT_ZNP.c
  Revised:        $Date: 2011-08-22 16:18:12 -0700 (Mon, 22 Aug 2011) $
  Revision:       $Revision: 27245 $

  Description:    Definitions for the ZNP sub-module of the MT API.

  Copyright 2011-2015 Texas Instruments Incorporated.

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

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "ti_zstack_config.h"
#include "comdef.h"
#include "mt.h"
#include "mt_rpc.h"
#include "mt_znp.h"
#include "osal_nv.h"
#include <driverlib/sys_ctrl.h>

#if defined (MT_ZNP_FUNC)
/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                          Global Variables
 * ------------------------------------------------------------------------------------------------
 */

uint32_t MT_PeriodicMsgRate;

/* ------------------------------------------------------------------------------------------------
 *                                          EXTERNAL REFERENCES
 * ------------------------------------------------------------------------------------------------
 */
extern uint8_t aExtendedAddress[];

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

static void znpBasicCfg(uint8_t *pBuf);
static void znpZCL_Cfg(uint8_t *pBuf);
static void znpSE_Cfg(uint8_t *pBuf);

/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */


/**************************************************************************************************
 * @fn          MT_ZnpCommandProcessing
 *
 * @brief       Process all MT ZNP commands.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing a ZNP command.
 *
 * output parameters
 *
 * None.
 *
 * @return      A 1-byte SRSP value or MT_RPC_SRSP_SENT or MT_RPC_SRSP_SENT;
 */
uint8_t MT_ZnpCommandProcessing(uint8_t *pBuf)
{
  const uint8_t cmd1 = pBuf[MT_RPC_POS_CMD1];
  pBuf += MT_RPC_FRAME_HDR_SZ;

  switch (cmd1)
  {
    case MT_ZNP_BASIC_CFG:
      znpBasicCfg(pBuf);
      break;

    case MT_ZNP_ZCL_CFG:
      znpZCL_Cfg(pBuf);
      break;

    case MT_ZNP_SE_CFG:
      znpSE_Cfg(pBuf);
      break;

    default:
      return MT_RPC_ERR_COMMAND_ID;
  }

#if defined MT_RPC_SRSP_SENT
  return MT_RPC_SRSP_SENT;
#else
  return MT_RPC_SUCCESS;
#endif
}

/**************************************************************************************************
 * @fn          MT_ZnpBasicRsp
 *
 * @brief       Build and send the ZNP Basic Response to the ZAP.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      true if message built and sent; false otherwise.
 */
bool MT_ZnpBasicRsp(void)
{
  uint8_t *pBuf = OsalPort_malloc(sizeof(mt_znp_basic_rsp_t));

  if (pBuf == NULL)
  {
    return false;
  }

  OsalPort_bufferUint32( &pBuf[0], MT_PeriodicMsgRate );

  OsalPort_bufferUint32( &pBuf[4], zgDefaultChannelList );

  pBuf[8] = LO_UINT16(zgConfigPANID);
  pBuf[9] = HI_UINT16(zgConfigPANID);

  osal_nv_read(ZCD_NV_STARTUP_OPTION, 0, 1, pBuf+10);
  pBuf[11] = zgDeviceLogicalType;

  pBuf[12] = LO_UINT16(_NIB.nwkDevAddress);
  pBuf[13] = HI_UINT16(_NIB.nwkDevAddress);

  pBuf[14] = LO_UINT16(_NIB.nwkCoordAddress);
  pBuf[15] = HI_UINT16(_NIB.nwkCoordAddress);

  pBuf[16] = LO_UINT16(_NIB.nwkPanId);
  pBuf[17] = HI_UINT16(_NIB.nwkPanId);

  pBuf[18] = _NIB.nwkLogicalChannel;
  pBuf[19] = _NIB.nwkState;

  (void)OsalPort_memcpy(pBuf+20, _NIB.nwkCoordExtAddress, Z_EXTADDR_LEN);
  (void)OsalPort_memcpy(pBuf+28, aExtendedAddress, Z_EXTADDR_LEN);

  pBuf[36] = devState;
#if defined ( INTER_PAN ) || defined ( BDB_TL_INITIATOR ) || defined ( BDB_TL_TARGET )
  extern uint8_t appEndPoint;
  pBuf[37] = appEndPoint;
  //rsp->spare1[2];
#else
  //rsp->spare1[3];
#endif

  // Initialize list with invalid EndPoints.
  (void)memset(pBuf+40, AF_BROADCAST_ENDPOINT, (MT_ZNP_EP_ID_LIST_MAX * 3));
  uint8_t idx = 40;
  epList_t *epItem = epList;

  for (uint8_t cnt = 0; cnt < MT_ZNP_EP_ID_LIST_MAX; cnt++)
  {
    if (epItem == NULL)
    {
      break;
    }
    if ((epItem->epDesc->simpleDesc != NULL) && (epItem->epDesc->simpleDesc->EndPoint != ZDO_EP))
    {
      pBuf[idx++] = epItem->epDesc->simpleDesc->EndPoint;
      pBuf[idx++] = LO_UINT16(epItem->epDesc->simpleDesc->AppProfId);
      pBuf[idx++] = HI_UINT16(epItem->epDesc->simpleDesc->AppProfId);
    }
    epItem = epItem->nextDesc;
  }
  idx = 40 + (MT_ZNP_EP_ID_LIST_MAX * 3);

  // Initialize list with invalid Cluster Id's.
  (void)memset(pBuf+idx, 0xFF, (MT_ZNP_ZDO_MSG_CB_LIST_MAX * 2));
  typedef struct
  {
    void *next;
    uint8_t taskID;
    uint16_t clusterID;
  } ZDO_MsgCB_t;
  extern ZDO_MsgCB_t *zdoMsgCBs;
  ZDO_MsgCB_t *pItem = zdoMsgCBs;

  for (uint8_t cnt = 0; cnt < MT_ZNP_ZDO_MSG_CB_LIST_MAX; cnt++)
  {
    if (pItem == NULL)
    {
      break;
    }
    else if (pItem->taskID == MT_TaskID)
    {
      pBuf[idx++] = LO_UINT16(pItem->clusterID);
      pBuf[idx++] = HI_UINT16(pItem->clusterID);
    }
    pItem = pItem->next;
  }
  idx = 40 + (MT_ZNP_EP_ID_LIST_MAX * 3) + (MT_ZNP_ZDO_MSG_CB_LIST_MAX * 2);

  extern pfnZdoCb zdoCBFunc[MAX_ZDO_CB_FUNC];
  for (uint8_t cnt = 0; cnt < MAX_ZDO_CB_FUNC; cnt++)
  {
    pBuf[idx++] = (zdoCBFunc[cnt] == NULL) ? 0 : 1;
  }

  MT_BuildAndSendZToolResponse(((uint8_t)MT_RPC_CMD_AREQ | (uint8_t)MT_RPC_SYS_ZNP), MT_ZNP_BASIC_RSP,
      40 + (MT_ZNP_EP_ID_LIST_MAX * 3) + (MT_ZNP_ZDO_MSG_CB_LIST_MAX * 2) + MAX_ZDO_CB_FUNC, pBuf);
  (void)OsalPort_free(pBuf);

  return true;
}

/**************************************************************************************************
 * @fn          znpBasicCfg
 *
 * @brief       Process the Conglomerate Basic Configuration command.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing the conglomerated configuration.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void znpBasicCfg(uint8_t *pBuf)
{
  uint32_t t32 = OsalPort_buildUint32( &pBuf[0], 4 );
  if (MT_PeriodicMsgRate != t32)
  {
    MT_PeriodicMsgRate = t32;
    (void)OsalPortTimers_startReloadTimer(MT_TaskID, MT_PERIODIC_MSG_EVENT, t32);
  }

  t32 = OsalPort_buildUint32( &pBuf[4], 4 );
  if (OsalPort_memcmp(&zgDefaultChannelList, &t32, 4) == FALSE)
  {
    (void)osal_nv_write(ZCD_NV_CHANLIST, 4, &t32);
  }

  uint16_t t16 = OsalPort_buildUint16( &pBuf[8] );
  if (OsalPort_memcmp(&zgConfigPANID, &t16, 2) == FALSE)
  {
    (void)osal_nv_write(ZCD_NV_PANID, 2, &t16);
  }

  if (zgDeviceLogicalType != pBuf[10])
  {
    (void)osal_nv_write(ZCD_NV_LOGICAL_TYPE, 1, pBuf+10);
  }

  if (pBuf[11] & MT_ZNP_CMD_DISC_RESET_NWK)
  {
    pBuf[0] = ZCD_STARTOPT_DEFAULT_NETWORK_STATE;
    (void)osal_nv_write(ZCD_NV_STARTUP_OPTION, 1, pBuf);
#if defined CC2531ZNP
    SystemResetSoft();
#else
    SysCtrlSystemReset();
#endif
  }
  else if (pBuf[11] & MT_ZNP_CMD_DISC_ZDO_START)
  {
    if (devState == DEV_HOLD)
    {
      ZDOInitDevice(0);
    }
  }
}

/**************************************************************************************************
 * @fn          znpZCL_Cfg
 *
 * @brief       Process the Conglomerate ZCL Configuration command.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing the conglomerated configuration.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void znpZCL_Cfg(uint8_t *pBuf)
{
}

/**************************************************************************************************
 * @fn          znpSE_Cfg
 *
 * @brief       Process the Conglomerate SE Configuration command.
 *
 * input parameters
 *
 * @param       pBuf - Pointer to the MT buffer containing the conglomerated configuration.
 *
 * output parameters
 *
 * None.
 *
 * @return      None.
 */
static void znpSE_Cfg(uint8_t *pBuf)
{
}

#endif
/**************************************************************************************************
*/
