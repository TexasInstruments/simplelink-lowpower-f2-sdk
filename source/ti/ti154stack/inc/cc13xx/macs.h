/******************************************************************************

 @file  macs.h

 @brief Mac common definitions between Stack and App

 Group: WCS, LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2015-2025, Texas Instruments Incorporated

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

#ifndef MACS_H
#define MACS_H

#include <stdint.h>


#define ICALL_MAC_CMD_EVENT_START   0xD0

enum
{
  MAC_STACK_INIT_PARAMS = ICALL_MAC_CMD_EVENT_START,
  MAC_INIT_DEVICE,
  MAC_INIT_COORD,
  MAC_INIT_BEACON_COORD,
  MAC_INIT_BEACON_DEVICE,
  MAC_INIT_FH,
  MAC_ENABLE_FH,
  MAC_START_FH,
  MAC_SET_REQ,
  MAC_GET_REQ,
  MAC_SET_SECURITY_REQ,
  MAC_GET_SECURITY_REQ,
  MAC_RESET_REQ,
  MAC_SRC_MATCH_ENABLE,
  MAC_SRC_MATCH_ADD_ENTRY,
  MAC_SRC_MATCH_DELETE_ENTRY,
  MAC_SRC_MATCH_ACK_ALL_PENDING,
  MAC_SRC_MATCH_CHECK_ALL_PENDING,
  MAC_SET_RADIO_REG,
  MAC_UPDATE_PANID,
  MAC_MCPS_DATA_REQ,
  MAC_MCPS_DATA_ALLOC,
  MAC_MCPS_PURGE_REQ,
  MAC_MLME_ASSOCIATE_REQ,
  MAC_MLME_ASSOCIATE_RSP,
  MAC_MLME_DISASSOCIATE_REQ,
  MAC_MLME_ORPHAN_RSP,
  MAC_MLME_POLL_REQ,
  MAC_MLME_SCAN_REQ,
  MAC_START_REQ,
  MAC_SYNC_REQ,
  MAC_RANDOM_BYTE,
  MAC_RESUME_REQ,
  MAC_YIELD_REQ,
  MAC_MSG_DEALLOCATE,
  MAC_GET_SECURITY_PTR_REQ,
  MAC_FH_SET_REQ,
  MAC_FH_GET_REQ,
  MAC_MLME_WS_ASYNC_REQ,
  MAC_SEC_ADD_DEVICE,
  MAC_SEC_DEL_DEVICE,
  MAC_SEC_DEL_KEY_AND_DEVICES,
  MAC_SEC_DEL_ALL_DEVICES,
  MAC_SEC_GET_DEFAULT_SOURCE_KEY,
  MAC_SEC_ADD_KEY_INIT_FC
};

#endif /* MACS_H */
