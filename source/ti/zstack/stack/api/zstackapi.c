/**
   @file  zstackapi.c
   @brief ZStack C interface implementation on top of
   dispatcher messaging interface.

   <!--
   Copyright 2014 - 2015 Texas Instruments Incorporated.

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
   -->
 */
#include <string.h>
#include <ti/sysbios/knl/Task.h>

#include "ti_zstack_config.h"
#include "rom_jt_154.h"
#include "zstackapi.h"

//*****************************************************************************
// Structures
//*****************************************************************************
typedef struct _genericreqrsp_t
{
    /** message header<br>
     */
    zstackmsg_HDR_t hdr;

    /** Message command fields */
    void *pReq;

    /** Response fields (immediate response) */
    void *pRsp;

} GenericReqRsp_t;


//*****************************************************************************
// Local variables
//*****************************************************************************

uint8_t stackServiceTaskId;

//*****************************************************************************
// Local Functions
//*****************************************************************************

/**
 * Generic function to send a request message to the ZStack Thread
 * and wait for a "default" response message.
 *
 * @param appServiceTaskId - Application Task ID
 * @param cmdID - Command ID of the message
 * @param pReq - Pointer to the request's structure
 * @param msgSize - length of the message being sent
 * @param pFnMatch - Function pointer to the response matching function
 *
 * @return zstack_ZStatusValues
 */
static zstack_ZStatusValues sendReqDefaultRsp(uint8_t appServiceTaskId,
                                              zstack_CmdIDs cmdID, void *pReq,
                                              int msgSize)
{
    zstack_ZStatusValues status = zstack_ZStatusValues_ZMemError;
    uint8_t msgStatus;

    //Make sure allocate space enought for the msg, even
    //if the message does not have payload
    if(msgSize < sizeof(zstackmsg_genericReq_t))
    {
      msgSize = sizeof(zstackmsg_genericReq_t);
    }

    zstackmsg_genericReq_t *pMsg =
        (zstackmsg_genericReq_t *)OsalPort_msgAllocate(msgSize);

    // Make sure the allocation was successful
    if(pMsg != NULL)
    {
        // Fill in the message header
        pMsg->hdr.event = cmdID;
        pMsg->hdr.status = 0;
        pMsg->hdr.srcServiceTask = appServiceTaskId;

        // Update the messges's request field
        pMsg->pReq = pReq;

        // Send the message
        msgStatus = OsalPort_msgSend( stackServiceTaskId, (uint8_t*) pMsg );

        // Was the message sent successfully
        if(msgStatus == OsalPort_SUCCESS)
        {
            bool gotRsp = false;

            // Return status
            zstackmsg_genericReq_t *pCmdStatus = NULL;

            while(!gotRsp)
            {
                // Wait for the response message
                OsalPort_blockOnEvent(Task_self());

                pCmdStatus = (zstackmsg_genericReq_t*)OsalPort_msgFindDequeue(appServiceTaskId, pMsg->hdr.event);

                if(pCmdStatus)
                {
                  gotRsp = true;
                }
            }

             // setup return of
            status = (zstack_ZStatusValues)pCmdStatus->hdr.status;
        }

        // pCmdStatus is the same as pMsg
        OsalPort_msgDeallocate( (uint8_t*)pMsg);
    }

    // function status
    return(status);
}

/**
 * Generic function to send a request message to the ZStack Thread
 * and specific response message.
 *
 * @param appServiceTaskId - Application Task ID
 * @param cmdID - Command ID of the message
 * @param pReq - Pointer to the request's structure
 * @param pRsp - Pointer to the request's structure
 * @param msgSize - length of the message being sent
 * @param pFnMatch - Function pointer to the response matching function
 *
 * @return zstack_ZStatusValues
 */
static zstack_ZStatusValues sendReqRsp(uint8_t appServiceTaskId,
                                       zstack_CmdIDs cmdID, void *pReq,
                                       void *pRsp, int msgSize)
{
    zstack_ZStatusValues status = zstack_ZStatusValues_ZMemError;
    uint8_t msgStatus;

    //Make sure allocate space enough for the msg, even
    //if the message does not have payload
    if(msgSize < sizeof(GenericReqRsp_t))
    {
      msgSize = sizeof(GenericReqRsp_t);
    }

    GenericReqRsp_t *pMsg = (GenericReqRsp_t *)OsalPort_msgAllocate(msgSize);

    // Make sure the allocation was successful
    if(pMsg != NULL)
    {
        // Fill in the message content
        pMsg->hdr.event = cmdID;
        pMsg->hdr.status = 0;
        pMsg->hdr.srcServiceTask = appServiceTaskId;

        /*
         * Set the pointer for the request and response structures.
         * The caller allocated space for the response
         */
        pMsg->pReq = pReq;
        pMsg->pRsp = pRsp;

        // Send the message
        msgStatus = OsalPort_msgSend( stackServiceTaskId, (uint8_t*) pMsg );

        // Was the message sent successfully
        if(msgStatus == OsalPort_SUCCESS)
        {
            bool gotRsp = false;

            // Return status
            GenericReqRsp_t *pCmdStatus = NULL;

            while(!gotRsp)
            {
                // Wait for the response message
                OsalPort_blockOnEvent(Task_self());

                pCmdStatus = (GenericReqRsp_t*)OsalPort_msgFindDequeue(appServiceTaskId, pMsg->hdr.event);

                if(pCmdStatus)
                {
                  gotRsp = true;
                }
            }

            // setup return of
            status = (zstack_ZStatusValues)pCmdStatus->hdr.status;
        }

        // pCmdStatus is the same as pMsg
        OsalPort_msgDeallocate( (uint8_t*)pMsg);
    }

    // Function status
    return(status);
}

//*****************************************************************************
// Public Functions
//*****************************************************************************

/**
 * Call to set the Stacks Service Task ID used to send messages to the stack.
 *
 * Public function defined in zstackapi.h
 */
void Zstackapi_init(uint8_t stackTaskId)
{
    stackServiceTaskId = stackTaskId;
}

/**
 * Call to send a System Reset Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysResetReq(uint8_t appServiceTaskId,
                                           zstack_sysResetReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SYS_RESET_REQ, pReq,
                              sizeof(zstackmsg_sysResetReq_t)) );
}

/**
 * Call to send a System Version Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysVersionReq(uint8_t appServiceTaskId,
                                             zstack_sysVersionRsp_t *pRsp)
{
    zstack_ZStatusValues status = zstack_ZStatusValues_ZMemError;
    uint8_t msgStatus;

    // Allocate message buffer space
    zstackmsg_sysVersionReq_t *pMsg =
        (zstackmsg_sysVersionReq_t *)OsalPort_msgAllocate( sizeof(*pMsg) );

    // Was the message allocated?
    if(pMsg != NULL)
    {
        /* Fill in the message content */
        pMsg->hdr.event = zstackmsg_CmdIDs_SYS_VERSION_REQ;
        pMsg->hdr.status = 0;
        pMsg->hdr.srcServiceTask = appServiceTaskId;

        /*
         * Set the pointer for the response structure.
         * The caller allocated space for the response
         */
        pMsg->pRsp = pRsp;

        // Send the message
        msgStatus = OsalPort_msgSend(stackServiceTaskId, (uint8_t*) pMsg );

        // Was the message sent successfully
        if(msgStatus == OsalPort_SUCCESS)
        {
            bool gotRsp = false;

            // Return status
            zstackmsg_sysVersionReq_t *pCmdStatus = NULL;

            while(!gotRsp)
            {
                // Wait for the response message
                OsalPort_blockOnEvent(Task_self());

                pCmdStatus = (zstackmsg_sysVersionReq_t*)OsalPort_msgFindDequeue(appServiceTaskId, pMsg->hdr.event);

                if(pCmdStatus)
                {
                  gotRsp = true;
                }
            }

            // setup return of
            status = (zstack_ZStatusValues)pCmdStatus->hdr.status;
        }

        // pCmdStatus is the same as pMsg
        OsalPort_msgDeallocate((uint8_t*) pMsg);
    }

    return(status);
}

/**
 * Call to send a System Version Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysConfigReadReq(
    uint8_t appServiceTaskId, zstack_sysConfigReadReq_t *
    pReq, zstack_sysConfigReadRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_SYS_CONFIG_READ_REQ, pReq,
                       pRsp, sizeof(zstackmsg_sysConfigReadReq_t)) );
}

/**
 * Call to send a System Reset Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysConfigWriteReq(
    uint8_t appServiceTaskId, zstack_sysConfigWriteReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SYS_CONFIG_WRITE_REQ,
                              pReq, sizeof(zstackmsg_sysConfigWriteReq_t)) );
}

/**
 * Call to send a System Set TX Power Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysSetTxPowerReq(
    uint8_t appServiceTaskId, zstack_sysSetTxPowerReq_t *
    pReq, zstack_sysSetTxPowerRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_SYS_SET_TX_POWER_REQ, pReq,
                       pRsp, sizeof(zstackmsg_sysSetTxPowerReq_t)) );
}

/**
 * Call to send a System Force Link Status Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysForceLinkStatusReq(uint8_t appServiceTaskId)
{
    zstack_ZStatusValues status = zstack_ZStatusValues_ZMemError;
    uint8_t msgStatus;

    // Allocate message buffer space
    zstackmsg_sysForceLinkStatusReq_t *pMsg =
        (zstackmsg_sysForceLinkStatusReq_t *)OsalPort_msgAllocate( sizeof(*pMsg) );

    // Was the allocation successful?
    if(pMsg != NULL)
    {
        // Fill in the message content
        pMsg->hdr.event = zstackmsg_CmdIDs_SYS_FORCE_LINK_STATUS_REQ;
        pMsg->hdr.status = 0;
        pMsg->hdr.srcServiceTask = appServiceTaskId;

        // Send the message
        msgStatus = OsalPort_msgSend(stackServiceTaskId, (uint8_t*) pMsg );

        // Was the message sent successfully
        if(msgStatus == OsalPort_SUCCESS)
        {
            bool gotRsp = false;

            // Return status
            zstackmsg_sysForceLinkStatusReq_t *pCmdStatus = NULL;

            while(!gotRsp)
            {
                // Wait for the response message
                OsalPort_blockOnEvent(Task_self());

                pCmdStatus = (zstackmsg_sysForceLinkStatusReq_t*)OsalPort_msgFindDequeue(appServiceTaskId, pMsg->hdr.event);

                if(pCmdStatus)
                {
                  gotRsp = true;
                }
            }

            // setup return of
            status = (zstack_ZStatusValues)pCmdStatus->hdr.status;
        }

        // pCmdStatus is the same as pMsg
        OsalPort_msgDeallocate( (uint8_t*)pMsg );
    }

    return(status);
}

/**
 * Call to send a System Network Info Read Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_sysNwkInfoReadReq(
    uint8_t appServiceTaskId, zstack_sysNwkInfoReadRsp_t *
    pRsp)
{
    zstack_ZStatusValues status = zstack_ZStatusValues_ZMemError;
    uint8_t msgStatus;

    // Allocate message buffer space
    zstackmsg_sysNwkInfoReadReq_t *pMsg =
        (zstackmsg_sysNwkInfoReadReq_t *)OsalPort_msgAllocate( sizeof(*pMsg) );

    if(pMsg != NULL)
    {
        // Fill in the message content
        pMsg->hdr.event = zstackmsg_CmdIDs_SYS_NWK_INFO_READ_REQ;
        pMsg->hdr.status = 0;
        pMsg->hdr.srcServiceTask = appServiceTaskId;

        /*
         * Set the pointer for the response structure.
         * The caller allocated space for the response
         */
        pMsg->pRsp = pRsp;

        // Send the message
        msgStatus = OsalPort_msgSend( stackServiceTaskId, (uint8_t*)pMsg );

        // Was the message sent successfully
        if(msgStatus == OsalPort_SUCCESS)
        {
            bool gotRsp = false;

            // Return status
            zstackmsg_sysNwkInfoReadReq_t *pCmdStatus = NULL;

            while(!gotRsp)
            {
                // Wait for the response message
                OsalPort_blockOnEvent(Task_self());

                pCmdStatus = (zstackmsg_sysNwkInfoReadReq_t*)OsalPort_msgFindDequeue(appServiceTaskId, pMsg->hdr.event);

                if(pCmdStatus)
                {
                  gotRsp = true;
                }
            }

            // setup return of
            status = (zstack_ZStatusValues)pCmdStatus->hdr.status;
        }

        // pCmdStatus is the same as pMsg
        OsalPort_msgDeallocate( (uint8_t*)pMsg );
    }

    return(status);
}

/**
 * Call to send a Device Start Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevStartReq(uint8_t        appServiceTaskId,
                                           zstack_devStartReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_DEV_START_REQ, pReq,
                              sizeof(zstackmsg_devStartReq_t)) );
}

/**
 * Call to send a Device Network Discovery Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevNwkDiscReq(uint8_t appServiceTaskId,
                                             zstack_devNwkDiscReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp
                (appServiceTaskId, zstackmsg_CmdIDs_DEV_NWK_DISCOVERY_REQ, pReq,
                sizeof(zstackmsg_devNwkDiscReq_t)) );
}

/**
 * Call to Enable/disable Frame forward notification
 * This will enable stack notifications whenever a frame is being forwared to another
 * node in the network to allow traffic analysis.
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_SetNwkFrameFwdNotificationReq(uint8_t appServiceTaskId,
                                                             zstack_setNwkFrameFwdNotificationReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp
                (appServiceTaskId, zstackmsg_CmdIDs_SYS_NWK_FRAME_FWD_NOTIFICATION_REQ, pReq,
                sizeof(zstackmsg_setNwkFrameFwdNotificationReq_t)) );
}


/**
 * Call to send a Device Join Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevJoinReq(uint8_t       appServiceTaskId,
                                          zstack_devJoinReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_DEV_JOIN_REQ, pReq,
                              sizeof(zstackmsg_devJoinReq_t)) );
}

/**
 * Call to send a Device Rejoin Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevRejoinReq(uint8_t         appServiceTaskId,
                                            zstack_devRejoinReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_DEV_REJOIN_REQ, pReq,
                              sizeof(zstackmsg_devRejoinReq_t)) );
}

/**
 * Call to send a Device ZDO Callback Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevZDOCBReq(uint8_t        appServiceTaskId,
                                           zstack_devZDOCBReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp
                (appServiceTaskId, zstackmsg_CmdIDs_DEV_ZDO_CBS_REQ, pReq,
                sizeof(zstackmsg_devZDOCBReq_t)) );
}

/**
 * Call to send a Device Network Route Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevNwkRouteReq(
    uint8_t appServiceTaskId, zstack_devNwkRouteReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_DEV_NWK_ROUTE_REQ,
                              pReq, sizeof(zstackmsg_devNwkRouteReq_t)) );
}

/**
 * Call to send a Device Network Check Route Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevNwkCheckRouteReq(
    uint8_t appServiceTaskId, zstack_devNwkCheckRouteReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_DEV_NWK_CHECK_ROUTE_REQ, pReq,
                              sizeof(zstackmsg_devNwkCheckRouteReq_t)) );
}

/**
 * Call to send a Device Update Neighbor's TxCost Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevUpdateNeighborTxCostReq(
    uint8_t
    appServiceTaskId, zstack_devUpdateNeighborTxCostReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_DEV_UPDATE_NEIGHBOR_TXCOST_REQ,
                              pReq,
                              sizeof(zstackmsg_devUpdateNeighborTxCostReq_t)) );
}

/**
 * Call to send a Device Force Network Settings Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevForceNetworkSettingsReq(
    uint8_t
    appServiceTaskId, zstack_devForceNetworkSettingsReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_DEV_FORCE_NETWORK_SETTINGS_REQ,
                              pReq,
                              sizeof(zstackmsg_devForceNetworkSettingsReq_t)) );
}

/**
 * Call to send a Device Force Network Update Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevForceNetworkUpdateReq(
    uint8_t
    appServiceTaskId, zstack_devForceNetworkUpdateReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_DEV_FORCE_NETWORK_UPDATE_REQ,
                              pReq,
                              sizeof(zstackmsg_devForceNetworkUpdateReq_t)) );
}

/**
 * Call to send a Device Force MAC Params Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_DevForceMacParamsReq(
    uint8_t appServiceTaskId, zstack_devForceMacParamsReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_DEV_FORCE_MAC_PARAMS_REQ, pReq,
                              sizeof(zstackmsg_devForceMacParamsReq_t)) );
}

/**
 * Call to send an APS Remove Group Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ApsRemoveGroupReq(
    uint8_t appServiceTaskId, zstack_apsRemoveGroup_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_APS_REMOVE_GROUP,
                              pReq, sizeof(zstackmsg_apsRemoveGroup_t)) );
}

/**
 * Call to send an APS Remove All Groups Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ApsRemoveAllGroupsReq(
    uint8_t appServiceTaskId, zstack_apsRemoveAllGroups_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp
                (appServiceTaskId, zstackmsg_CmdIDs_APS_REMOVE_ALL_GROUPS, pReq,
                sizeof(zstackmsg_apsRemoveAllGroups_t)) );
}

/**
 * Call to send an APS Find All Groups Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ApsFindAllGroupsReq(
    uint8_t appServiceTaskId, zstack_apsFindAllGroupsReq_t
    *pReq, zstack_apsFindAllGroupsRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_APS_FIND_ALL_GROUPS_REQ,
                       pReq, pRsp, sizeof(zstackmsg_apsFindAllGroupsReq_t)) );
}

/**
 * Call to send an APS Find Group Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ApsFindGroupReq(
    uint8_t appServiceTaskId, zstack_apsFindGroupReq_t *pReq,
    zstack_apsFindGroupRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_APS_FIND_GROUP_REQ, pReq,
                       pRsp, sizeof(zstackmsg_apsFindGroupReq_t)) );
}

/**
 * Call to send an APS Add Group Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ApsAddGroupReq(uint8_t        appServiceTaskId,
                                              zstack_apsAddGroup_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_APS_ADD_GROUP, pReq,
                              sizeof(zstackmsg_apsAddGroup_t)) );
}

/**
 * Call to send an APS Count All Groups Request
 *
 * Public function defined in zstackapi.h
 */
int Zstackapi_ApsCountAllGroupsReq(uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( (int)( sendReqDefaultRsp(appServiceTaskId,
                                     zstackmsg_CmdIDs_APS_COUNT_ALL_GROUPS,
                                     NULL,
                                     sizeof(zstackmsg_apsCountAllGroups_t))) );
}

/**
 * Call to send an AF Register Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_AfRegisterReq(uint8_t appServiceTaskId,
                                             zstack_afRegisterReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp
                (appServiceTaskId, zstackmsg_CmdIDs_AF_REGISTER_REQ, pReq,
                sizeof(zstackmsg_afRegisterReq_t)) );
}

/**
 * Call to send an AF Unregister Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_AfUnRegisterReq(
    uint8_t appServiceTaskId, zstack_afUnRegisterReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_AF_UNREGISTER_REQ,
                              pReq, sizeof(zstackmsg_afUnRegisterReq_t)) );
}

/**
 * Call to send an AF Data Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_AfDataReq(uint8_t      appServiceTaskId,
                                         zstack_afDataReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_AF_DATA_REQ, pReq,
                              sizeof(zstackmsg_afDataReq_t)) );
}

/**
 * Call to send an AF InterPAN Control Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_AfInterpanCtlReq(
    uint8_t appServiceTaskId, zstack_afInterPanCtlReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_AF_INTERPAN_CTL_REQ,
                              pReq, sizeof(zstackmsg_afInterPanCtlReq_t)) );
}

/**
 * Call to send an AF Config Get Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_AfConfigGetReq(
    uint8_t appServiceTaskId, zstack_afConfigGetReq_t *pReq,
    zstack_afConfigGetRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_AF_CONFIG_GET_REQ, pReq,
                       pRsp, sizeof(zstackmsg_afConfigGetReq_t)) );
}

/**
 * Call to send an AF Config Set Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_AfConfigSetReq(
    uint8_t appServiceTaskId, zstack_afConfigSetReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_AF_CONFIG_SET_REQ,
                              pReq, sizeof(zstackmsg_afConfigSetReq_t)) );
}

/**
 * Call to send a ZDO Network Address Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoNwkAddrReq(uint8_t appServiceTaskId,
                                             zstack_zdoNwkAddrReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_NWK_ADDR_REQ,
                              pReq, sizeof(zstackmsg_zdoNwkAddrReq_t)) );
}

/**
 * Call to send a ZDO IEEE Address Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoIeeeAddrReq(
    uint8_t appServiceTaskId, zstack_zdoIeeeAddrReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_IEEE_ADDR_REQ,
                              pReq, sizeof(zstackmsg_zdoIeeeAddrReq_t)) );
}

/**
 * Call to send a ZDO Node Descriptor Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoNodeDescReq(
    uint8_t appServiceTaskId, zstack_zdoNodeDescReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_NODE_DESC_REQ,
                              pReq, sizeof(zstackmsg_zdoNodeDescReq_t)) );
}

/**
 * Call to send a ZDO Power Descriptor Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoPowerDescReq(
    uint8_t appServiceTaskId, zstack_zdoPowerDescReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_POWER_DESC_REQ,
                              pReq, sizeof(zstackmsg_zdoPowerDescReq_t)) );
}

/**
 * Call to send a ZDO Simple Descriptor Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoSimpleDescReq(
    uint8_t appServiceTaskId, zstack_zdoSimpleDescReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_REQ,
                              pReq, sizeof(zstackmsg_zdoSimpleDescReq_t)) );
}

/**
 * Call to send a ZDO Active Endpoint Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoActiveEndpointReq(
    uint8_t appServiceTaskId, zstack_zdoActiveEndpointReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_ACTIVE_ENDPOINT_REQ, pReq,
                              sizeof(zstackmsg_zdoActiveEndpointReq_t)) );
}

/**
 * Call to send a ZDO Match Descriptor Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMatchDescReq(
    uint8_t appServiceTaskId, zstack_zdoMatchDescReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_MATCH_DESC_REQ,
                              pReq, sizeof(zstackmsg_zdoMatchDescReq_t)) );
}

/**
 * Call to send a ZDO Complex Descriptor Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoComplexDescReq(
    uint8_t appServiceTaskId, zstack_zdoComplexDescReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_COMPLEX_DESC_REQ,
                              pReq, sizeof(zstackmsg_zdoComplexDescReq_t)) );
}

/**
 * Call to send a ZDO User Descriptor Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoUserDescReq(
    uint8_t appServiceTaskId, zstack_zdoUserDescReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_USER_DESC_REQ,
                              pReq, sizeof(zstackmsg_zdoUserDescReq_t)) );
}

/**
 * Call to set Bind/Unbind authenticated address
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoSetBindUnbindAuthAddrReq(
    uint8_t appServiceTaskId, zstack_zdoSetBindUnbindAuthAddr_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_SET_BIND_UNBIND_AUTH_ADDR_REQ,
                              pReq, sizeof(zstackmsg_zdoSetBindUnbindAuthAddrReq_t)) );
}


/**
 * Call to send a ZDO Device Announce Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoDeviceAnnounceReq(
    uint8_t appServiceTaskId, zstack_zdoDeviceAnnounceReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE_REQ, pReq,
                              sizeof(zstackmsg_zdoDeviceAnnounceReq_t)) );
}

/**
 * Call to send a ZDO User Descriptor Set Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoUserDescSetReq(
    uint8_t appServiceTaskId, zstack_zdoUserDescSetReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_USER_DESCR_SET_REQ, pReq,
                              sizeof(zstackmsg_zdoUserDescSetReq_t)) );
}

/**
 * Call to send a ZDO Server Discovery Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoServerDiscReq(
    uint8_t appServiceTaskId, zstack_zdoServerDiscReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_SERVER_DISC_REQ,
                              pReq, sizeof(zstackmsg_zdoServerDiscReq_t)) );
}

/**
 * Call to send a ZDO End Device Bind Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoEndDeviceBindReq(
    uint8_t appServiceTaskId, zstack_zdoEndDeviceBindReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_REQ, pReq,
                              sizeof(zstackmsg_zdoEndDeviceBindReq_t)) );
}

/**
 * Call to send a ZDO Bind Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoBindReq(uint8_t       appServiceTaskId,
                                          zstack_zdoBindReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_BIND_REQ, pReq,
                              sizeof(zstackmsg_zdoBindReq_t)) );
}

/**
 * Call to send a ZDO Unbind Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoUnbindReq(uint8_t         appServiceTaskId,
                                            zstack_zdoUnbindReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_UNBIND_REQ, pReq,
                              sizeof(zstackmsg_zdoUnbindReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Network Discovery Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtNwkDiscReq(
    uint8_t appServiceTaskId, zstack_zdoMgmtNwkDiscReq_t *
    pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp
                (appServiceTaskId, zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_REQ, pReq,
                sizeof(zstackmsg_zdoMgmtNwkDiscReq_t)) );
}

/**
 * Call to send a ZDO Mgmt LQI Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtLqiReq(uint8_t appServiceTaskId,
                                             zstack_zdoMgmtLqiReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_MGMT_LQI_REQ,
                              pReq, sizeof(zstackmsg_zdoMgmtLqiReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Routing Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtRtgReq(uint8_t appServiceTaskId,
                                             zstack_zdoMgmtRtgReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_MGMT_RTG_REQ,
                              pReq, sizeof(zstackmsg_zdoMgmtRtgReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Bind Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtBindReq(
    uint8_t appServiceTaskId, zstack_zdoMgmtBindReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_MGMT_BIND_REQ,
                              pReq, sizeof(zstackmsg_zdoMgmtBindReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Leave Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtLeaveReq(
    uint8_t appServiceTaskId, zstack_zdoMgmtLeaveReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_REQ,
                              pReq, sizeof(zstackmsg_zdoMgmtLeaveReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Direct Join Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtDirectJoinReq(
    uint8_t appServiceTaskId, zstack_zdoMgmtDirectJoinReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_REQ, pReq,
                              sizeof(zstackmsg_zdoMgmtDirectJoinReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Permit Join Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtPermitJoinReq(
    uint8_t appServiceTaskId, zstack_zdoMgmtPermitJoinReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_REQ, pReq,
                              sizeof(zstackmsg_zdoMgmtPermitJoinReq_t)) );
}

/**
 * Call to send a ZDO Mgmt Network Update Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_ZdoMgmtNwkUpdateReq(
    uint8_t appServiceTaskId, zstack_zdoMgmtNwkUpdateReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_REQ, pReq,
                              sizeof(zstackmsg_zdoMgmtNwkUpdateReq_t)) );
}

/**
 * Call to send a Security Network Key Get Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secNwkKeyGetReq(
    uint8_t appServiceTaskId, zstack_secNwkKeyGetReq_t *pReq,
    zstack_secNwkKeyGetRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_NWK_KEY_GET_REQ, pReq,
                       pRsp, sizeof(zstackmsg_secNwkKeyGetReq_t)) );
}

/**
 * Call to send a Security Network Key Set Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secNwkKeySetReq(
    uint8_t appServiceTaskId, zstack_secNwkKeySetReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_NWK_KEY_SET_REQ,
                              pReq, sizeof(zstackmsg_secNwkKeySetReq_t)) );
}

/**
 * Call to send a Security Network Key Update Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secNwkKeyUpdateReq(
    uint8_t appServiceTaskId, zstack_secNwkKeyUpdateReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_SEC_NWK_KEY_UPDATE_REQ, pReq,
                              sizeof(zstackmsg_secNwkKeyUpdateReq_t)) );
}

/**
 * Call to send a Security Network Key Switch Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secNwkKeySwitchReq(
    uint8_t appServiceTaskId, zstack_secNwkKeySwitchReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_SEC_NWK_KEY_SWITCH_REQ, pReq,
                              sizeof(zstackmsg_secNwkKeySwitchReq_t)) );
}

/**
 * Call to send a Security APS Link Key Get Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secApsLinkKeyGetReq(
    uint8_t appServiceTaskId, zstack_secApsLinkKeyGetReq_t
    *pReq, zstack_secApsLinkKeyGetRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_APS_LINKKEY_GET_REQ,
                       pReq, pRsp, sizeof(zstackmsg_secApsLinkKeyGetReq_t)) );
}

/**
 * Call to send a Security APS Link Key Set Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secApsLinkKeySetReq(
    uint8_t appServiceTaskId, zstack_secApsLinkKeySetReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_SEC_APS_LINKKEY_SET_REQ, pReq,
                              sizeof(zstackmsg_secApsLinkKeySetReq_t)) );
}

/**
 * Call to send a Security APS Link Key Remove Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secApsLinkKeyRemoveReq(
    uint8_t appServiceTaskId, zstack_secApsLinkKeyRemoveReq_t
    *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId,
                              zstackmsg_CmdIDs_SEC_APS_LINKKEY_REMOVE_REQ,
                              pReq, sizeof(zstackmsg_secApsLinkKeyRemoveReq_t)) );
}

/**
 * Call to send a Security APS Remove Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secApsRemoveReq(
    uint8_t appServiceTaskId, zstack_secApsRemoveReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_APS_REMOVE_REQ,
                              pReq, sizeof(zstackmsg_secApsRemoveReq_t)) );
}

/**
 * Call to send a Security Setup Partner Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secMgrSetupPartnerReq(
    uint8_t appServiceTaskId, zstack_secMgrSetupPartnerReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_MGR_SETUP_PARTNER_REQ,
                              pReq, sizeof(zstackmsg_secMgrSetupPartnerReq_t) ));
}

/**
 * Call to send a Security App Key Type Set Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secMgrAppKeyTypeSetReq(
    uint8_t appServiceTaskId, zstack_secMgrAppKeyTypeSetReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_MGR_APP_KEY_TYPE_SET_REQ,
                              pReq, sizeof(zstackmsg_secMgrAppKeyTypeSetReq_t) ));
}

/**
 * Call to send a Security App Key Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_secMgrAppKeyReq(
    uint8_t appServiceTaskId, zstack_secMgrAppKeyReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_SEC_MGR_APP_KEY_REQ,
                              pReq, sizeof(zstackmsg_secMgrAppKeyReq_t) ));
}

/**
 * Call to send a Device Network Manager Set Request
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_devNwkManagerSetReq(
    uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_DEV_NWK_MANAGER_SET_REQ,
                              NULL, 0 ));
}

/**
 * Call to send a Zstackapi_bdbStartCommissioningReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbStartCommissioningReq(
    uint8_t appServiceTaskId, zstack_bdbStartCommissioningReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_START_COMMISSIONING_REQ,
                              pReq, sizeof(zstackmsg_bdbStartCommissioningReq_t)) );
}

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
/**
 * Call to send a Zstackapi_bdbSetIdentifyActiveEndpointReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbSetIdentifyActiveEndpointReq(
    uint8_t appServiceTaskId, zstack_bdbSetIdentifyActiveEndpointReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_SET_IDENTIFY_ACTIVE_ENDPOINT_REQ,
                              pReq, sizeof(zstackmsg_bdbSetIdentifyActiveEndpointReq_t)) );
}
#endif

/**
 * Call to send a Zstackapi_bdbGetIdentifyActiveEndpointReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbGetIdentifyActiveEndpointReq(
    uint8_t appServiceTaskId, zstack_bdbGetIdentifyActiveEndpointRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_GET_IDENTIFY_ACTIVE_ENDPOINT_REQ,NULL,
                              pRsp, sizeof(zstackmsg_bdbGetIdentifyActiveEndpointReq_t)) );
}

#if (BDB_FINDING_BINDING_CAPABILITY_ENABLED == 1)
/**
 * Call to send a Zstackapi_bdbStopInitiatorFindingBindingReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbStopInitiatorFindingBindingReq(
    uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_STOP_INITIATOR_FINDING_BINDING_REQ,
                              NULL, sizeof(zstackmsg_bdbStopInitiatorFindingBindingReq_t)) );
}
#endif

/**
 * Call to send a Zstackapi_getZCLFrameCounterReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_getZCLFrameCounterReq(
    uint8_t appServiceTaskId, zstack_getZCLFrameCounterRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_GET_ZCL_FRAME_COUNTER_REQ,NULL,
                              pRsp, sizeof(zstackmsg_getZCLFrameCounterReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbZclIdentifyCmdIndReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbZclIdentifyCmdIndReq(
    uint8_t appServiceTaskId, zstack_bdbZCLIdentifyCmdIndReq_t *pReq)
{
    // Build and send the message, then wait of the response message
return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_ZCL_IDENTIFY_CMD_IND_REQ,pReq,
                              sizeof(zstackmsg_bdbZCLIdentifyCmdIndReq_t)) );
}


/**
 * Call to send a Zstackapi_bdbSetEpDescListToActiveEndpoint
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbSetEpDescListToActiveEndpoint(
    uint8_t appServiceTaskId,zstack_bdbSetEpDescListToActiveEndpointRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_SET_EP_DESC_LIST_ACTIVE_EP,
                              NULL, pRsp, sizeof(zstackmsg_bdbSetEpDescListToActiveEndpointReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbResetLocalActionReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbResetLocalActionReq(
    uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_RESET_LOCAL_ACTION_REQ,
                              NULL, sizeof(zstackmsg_bdbResetLocalActionReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbSetAttributesReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbSetAttributesReq(
    uint8_t appServiceTaskId, zstack_bdbSetAttributesReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_SET_ATTRIBUTES_REQ,
                              pReq, sizeof(zstackmsg_bdbSetAttributesReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbGetAttributesReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbGetAttributesReq(
    uint8_t appServiceTaskId, zstack_bdbGetAttributesRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_GET_ATTRIBUTES_REQ,NULL,
                              pRsp, sizeof(zstackmsg_bdbGetAttributesReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbGetFBInitiatorStatusReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbGetFBInitiatorStatusReq(
    uint8_t appServiceTaskId,zstack_bdbGetFBInitiatorStatusRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_GET_FB_INITIATOR_STATUS_REQ,NULL,
                              pRsp, sizeof(zstackmsg_bdbGetFBInitiatorStatusReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbGenerateInstallCodeCRCReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbGenerateInstallCodeCRCReq(
    uint8_t appServiceTaskId, zstack_bdbGenerateInstallCodeCRCReq_t *pReq,
    zstack_bdbGenerateInstallCodeCRCRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_GENERATE_INSTALL_CODE_CRC_REQ,
                              pReq, pRsp, sizeof(zstackmsg_bdbGenerateInstallCodeCRCReq_t)) );
}


#ifdef BDB_REPORTING
/**
 * Call to send a Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbRepAddAttrCfgRecordDefaultToListReq(
    uint8_t appServiceTaskId, zstack_bdbRepAddAttrCfgRecordDefaultToListReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_REP_ADD_ATTR_CFG_RECORD_DEFAULT_TO_LIST_REQ,
                              pReq, sizeof(zstackmsg_bdbRepAddAttrCfgRecordDefaultToListReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbRepChangedAttrValueReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbRepChangedAttrValueReq(
    uint8_t appServiceTaskId, zstack_bdbRepChangedAttrValueReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_REP_CHANGED_ATTR_VALUE_REQ,
                              pReq, sizeof(zstackmsg_bdbRepChangedAttrValueReq_t)) );
}


/**
 * Call to send a Zstackapi_bdbRepChangedAttrValueReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbProcessInConfigReportCmd(
    uint8_t appServiceTaskId, zstack_bdbProcessInConfigReportReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_PROCESS_IN_CONFIG_REPORT_REQ,
                              pReq, sizeof(zstackmsg_bdbProcessInConfigReportReq_t)) );
}


/**
 * Call to send a Zstackapi_bdbRepChangedAttrValueReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbProcessInReadReportCfgCmd(
    uint8_t appServiceTaskId, zstack_bdbProcessInReadReportCfgReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_PROCESS_IN_READ_REPORT_CFG_REQ,
                              pReq, sizeof(zstackmsg_bdbProcessInReadReportCfgReq_t)) );
}

#endif

/**
 * Call to send a Zstackapi_bdbAddInstallCodeReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbAddInstallCodeReq(
    uint8_t appServiceTaskId, zstack_bdbAddInstallCodeReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_ADD_INSTALL_CODE_REQ,
                              pReq, sizeof(zstackmsg_bdbAddInstallCodeReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbTouchLinkTargetEnableCommissioningReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbTouchLinkTargetEnableCommissioningReq(
    uint8_t appServiceTaskId, zstack_bdbTouchLinkTargetEnableCommissioningReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_COMMISSIONING_REQ,
                              pReq, sizeof(zstackmsg_bdbTouchLinkTargetEnableCommissioningReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbTouchLinkTargetDisableCommissioningReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbTouchLinkTargetDisableCommissioningReq(
    uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_DISABLE_COMMISSIONING_REQ,
                              NULL, sizeof(zstackmsg_bdbTouchLinkTargetDisableCommissioningReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbTouchLinkTargetGetTimerReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbTouchLinkTargetGetTimerReq(
    uint8_t appServiceTaskId, zstack_bdbTouchLinkTargetGetTimerRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_GETT_IMER_REQ,
                              NULL,pRsp, sizeof(zstackmsg_bdbTouchLinkTargetGetTimerReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbSetActiveCentralizedLinkKeyReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbSetActiveCentralizedLinkKeyReq(
    uint8_t appServiceTaskId, zstack_bdbSetActiveCentralizedLinkKeyReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_SET_ACTIVE_CENTRALIZED_LINK_KEY_REQ,
                              pReq, sizeof(zstackmsg_bdbSetActiveCentralizedLinkKeyReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbCBKETCLinkKeyExchangeAttemptReq(
    uint8_t appServiceTaskId, zstack_bdbCBKETCLinkKeyExchangeAttemptReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_ATTEMPT_REQ,
                              pReq, sizeof(zstackmsg_bdbCBKETCLinkKeyExchangeAttemptReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbNwkDescFreeReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbNwkDescFreeReq(
    uint8_t appServiceTaskId, zstack_bdbNwkDescFreeReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_NWK_DESC_FREE_REQ,
                              pReq, sizeof(zstackmsg_bdbNwkDescFreeReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbNwkDescFreeReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbFilterNwkDescComplete(
    uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_FILTER_NWK_DESC_COMPLETE_REQ,
                              NULL, sizeof(zstackmsg_bdbFilterNwkDescCompleteReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbTouchlinkSetAllowStealingReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbTouchlinkSetAllowStealingReq(
    uint8_t appServiceTaskId, zstack_bdbTouchlinkSetAllowStealingReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_TOUCHLINK_SET_ALLOW_STEALING_REQ,
                              pReq, sizeof(zstackmsg_bdbTouchlinkSetAllowStealingReq_t)) );
}

/**
 * Call to send a Zstackapi_bdbTouchlinkGetAllowStealingReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbTouchlinkGetAllowStealingReq(
    uint8_t appServiceTaskId, zstack_bdbTouchlinkGetAllowStealingRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_TOUCHLINK_GET_ALLOW_STEALING_REQ,
                              NULL, pRsp, sizeof(zstackmsg_bdbTouchlinkGetAllowStealingReq_t)) );
}


#if (ZG_BUILD_JOINING_TYPE)
/**
 * Call to send a Zstackapi_bdbRecoverNwkReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_bdbRecoverNwkReq(
    uint8_t appServiceTaskId, zstack_bdbRecoverNwkRsp_t *pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_BDB_ZED_ATTEMPT_RECOVER_NWK_REQ,
                              NULL,pRsp, sizeof(zstackmsg_bdbRecoverNwkReq_t)) );
}
#endif


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/**
 * Call to send a Zstackapi_gpAllowChangeChannelReq
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpAllowChangeChannelReq(
    uint8_t appServiceTaskId, zstack_gpAllowChangeChannelReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_ALLOW_CHANNEL_CHANGE_REQ,
                              pReq, sizeof(zstack_gpAllowChangeChannelReq_t)) );
}

/**
 * Call to send a Zstackapi_gpSecRsp
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpSecRsp(
    uint8_t appServiceTaskId, zstack_gpSecRsp_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_SECURITY_RSP,
                              pReq, sizeof(zstackmsg_gpSecRsp_t)) );
}

/**
 * Call to send a Zstackapi_gpAliasConflict
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpAliasConflict(
    uint8_t appServiceTaskId, zstack_gpAddrConflict_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_ADDRESS_CONFLICT,
                              pReq, sizeof(zstackmsg_gpAliasConflict_t)) );
}

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
/**
 * Call to send a Zstackapi_gpEncryptDecryptCommissioningKey
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpEncryptDecryptCommissioningKey(
    uint8_t appServiceTaskId, zstack_gpEncryptDecryptCommissioningKey_t *pReq, zstack_gpEncryptDecryptCommissioningKeyRsp_t* pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_CCM_STAR_COMMISSIONING_KEY, pReq,
                              pRsp, sizeof(zstackmsg_gpEncryptDecryptCommissioningKey_t)) );
}

/**
 * Call to send a Zstackapi_gptDecryptDataInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gptDecryptDataInd(
    uint8_t appServiceTaskId, zstack_gpDecryptDataInd_t *pReq, zstack_gpDecryptDataIndRsp_t* pRsp)
{
    // Build and send the message, then wait of the response message
    return( sendReqRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_CCM_STAR_DATA_IND, pReq,
                              pRsp, sizeof(zstackmsg_gpDecryptDataInd_t)) );
}

/**
 * Call to send a Zstackapi_gpCommissioningSucess
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpCommissioningSucess(
    uint8_t appServiceTaskId, zstack_gpCommissioningSuccess_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_COMMISSIONING_SUCCESS,
                              pReq, sizeof(zstackmsg_gpCommissioningSuccess_t)) );
}

/**
 * Call to send a Zstackapi_gpSendDeviceAnnounce
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_gpSendDeviceAnnounce(
    uint8_t appServiceTaskId, zstack_gpAliasNwkAddress_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_GP_SEND_DEV_ANNOUNCE,
                              pReq, sizeof(zstackmsg_gpSendDeviceAnnounce_t)) );
}
#endif
#endif

#if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
/**
 * Call to send a Zstackapi_tlScanReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlScanReqInd(
    uint8_t appServiceTaskId, zstack_tlScanReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_SCAN_REC_IND,
                              pReq, sizeof(zstackmsg_tlScanReq_t)) );
}

/**
 * Call to send a Zstackapi_touchlinkNwkJoinReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_touchlinkNwkJoinReqInd(
    uint8_t appServiceTaskId, zstack_touchlinkNwkJointReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TOUCHLINK_NWK_JOIN_REC_IND,
                              pReq, sizeof(zstackmsg_touchlinkNwkJointReq_t)) );
}

/**
 * Call to send a Zstackapi_touchlinkNwkUpdateReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_touchlinkNwkUpdateReqInd(
    uint8_t appServiceTaskId, zstack_touchlinkNwkUpdateReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TOUCHLINK_NWK_UPDATE_REC_IND,
                              pReq, sizeof(zstackmsg_touchlinkNwkUpdateReq_t)) );
}
#endif

#if defined BDB_TL_TARGET
/**
 * Call to send a Zstackapi_tlTargetNwkStartReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlTargetNwkStartReqInd(
    uint8_t appServiceTaskId, zstack_tlTargetNwkStartReq_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_TARGET_NWK_START_REC_IND,
                              pReq, sizeof(zstackmsg_tlTargetNwkStartReq_t)) );
}

/**
 * Call to send a Zstackapi_tlTargetResetToFNReqInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlTargetResetToFNReqInd(uint8_t appServiceTaskId)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_TARGET_RESET_TO_FN_REC_IND,
                              NULL, sizeof(zstackmsg_HDR_t)) );
}
#endif

#if defined BDB_TL_INITIATOR
/**
 * Call to send a Zstackapi_touchlinkGetScanBaseTime
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_touchlinkGetScanBaseTime(
    uint8_t appServiceTaskId, zstack_touchlinkGetScanBaseTime_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_GET_SCAN_BASE_TIME,
                              pReq, sizeof(zstackmsg_touchlinkGetScanBaseTime_t)) );
}

/**
 * Call to send a Zstackapi_tlInitiatorScanRspInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorScanRspInd(
    uint8_t appServiceTaskId, zstack_touchlinkScanRsp_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_INITIATOR_SCAN_RSP_IND,
                              pReq, sizeof(zstackmsg_touchlinkScanRsp_t)) );
}

/**
 * Call to send a Zstackapi_tlInitiatorDevInfoRspInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorDevInfoRspInd(
    uint8_t appServiceTaskId, zstack_touchlinkDevInfoRsp_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TOUCHLINK_DEV_INFO_RSP_IND,
                              pReq, sizeof(zstackmsg_touchlinkDevInfoRsp_t)) );
}

/**
 * Call to send a Zstackapi_tlInitiatorNwkStartRspInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorNwkStartRspInd(
    uint8_t appServiceTaskId, zstack_touchlinkNwkStartRsp_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_INITIATOR_NWK_START_RSP_IND,
                              pReq, sizeof(zstackmsg_touchlinkNwkStartRsp_t)) );
}

/**
 * Call to send a Zstackapi_tlInitiatorNwkJoinRspInd
 *
 * Public function defined in zstackapi.h
 */
zstack_ZStatusValues Zstackapi_tlInitiatorNwkJoinRspInd(
    uint8_t appServiceTaskId, zstack_touchlinkNwkJoinRsp_t *pReq)
{
    // Build and send the message, then wait of the response message
    return( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_TL_INITIATOR_NWK_JOIN_RSP_IND,
                              pReq, sizeof(zstackmsg_touchlinkNwkJoinRsp_t)) );
}
#endif


/**
 * Call to pause/resume the device on the nwk.
 *
 * Public function defined in zstackapi.h
 */
 zstack_ZStatusValues  Zstackapi_pauseResumeDeviceReq(
         uint8_t appServiceTaskId, zstack_pauseResumeDeviceReq_t *pReq )
 {
     // Build and send the message, then wait of the response message
     return ( sendReqDefaultRsp(appServiceTaskId, zstackmsg_CmdIDs_PAUSE_DEVICE_REQ,
                               pReq, sizeof(zstack_pauseResumeDeviceReq_t)) );
 }


/**
 * Call to free the memory used by an Indication message, messages
 * sent asynchronously from the ZStack thread.
 *
 * Public function defined in zstackapi.h
 */
bool Zstackapi_freeIndMsg(void *pMsg)
{
    bool processed = true;
    GenericReqRsp_t *pTemp = (GenericReqRsp_t *)pMsg;

    // Determine the type of message to free
    switch(pTemp->hdr.event)
    {
        // ZDO Source Routing Indicaiton
        case zstackmsg_CmdIDs_ZDO_SRC_RTG_IND:
            {
                zstackmsg_zdoSrcRtgInd_t *pInd =
                    (zstackmsg_zdoSrcRtgInd_t *)pMsg;

                // Does it have any relay addresses?
                if(pInd->req.pRelay)
                {
                    // Free relay data
                    OsalPort_free(pInd->req.pRelay);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Network Address Response
        case zstackmsg_CmdIDs_ZDO_NWK_ADDR_RSP:
            {
                zstackmsg_zdoNwkAddrRspInd_t *pInd =
                    (zstackmsg_zdoNwkAddrRspInd_t *)pMsg;

                // Does it have an associated device list?
                if(pInd->rsp.pAssocDevList)
                {
                    // Free the associated device list
                    OsalPort_free(pInd->rsp.pAssocDevList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO IEEE Address Response
        case zstackmsg_CmdIDs_ZDO_IEEE_ADDR_RSP:
            {
                zstackmsg_zdoIeeeAddrRspInd_t *pInd =
                    (zstackmsg_zdoIeeeAddrRspInd_t *)pMsg;

                // Does it have an associated device list?
                if(pInd->rsp.pAssocDevList)
                {
                    // Free the associated device list
                    OsalPort_free(pInd->rsp.pAssocDevList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Simple Descriptor Response
        case zstackmsg_CmdIDs_ZDO_SIMPLE_DESC_RSP:
            {
                zstackmsg_zdoSimpleDescRspInd_t *pInd =
                    (zstackmsg_zdoSimpleDescRspInd_t *)pMsg;

                // Free an input cluster list
                if(pInd->rsp.simpleDesc.pInputClusters)
                {
                    OsalPort_free(pInd->rsp.simpleDesc.pInputClusters);
                }
                // Free an output cluster list
                if(pInd->rsp.simpleDesc.pOutputClusters)
                {
                    OsalPort_free(pInd->rsp.simpleDesc.pOutputClusters);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Active Enpoint Response
        case zstackmsg_CmdIDs_ZDO_ACTIVE_EP_RSP:
            {
                zstackmsg_zdoActiveEndpointsRspInd_t *pInd =
                    (zstackmsg_zdoActiveEndpointsRspInd_t *)pMsg;

                // Free the active endpoint list
                if(pInd->rsp.pActiveEPList)
                {
                    OsalPort_free(pInd->rsp.pActiveEPList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Match Descriptor Response
        case zstackmsg_CmdIDs_ZDO_MATCH_DESC_RSP:
            {
                zstackmsg_zdoMatchDescRspInd_t *pInd =
                    (zstackmsg_zdoMatchDescRspInd_t *)pMsg;

                // Free the match descriptor's endpoint list
                if(pInd->rsp.pMatchList)
                {
                    OsalPort_free(pInd->rsp.pMatchList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO User Descriptor Response
        case zstackmsg_CmdIDs_ZDO_USER_DESC_RSP:
            {
                zstackmsg_zdoUserDescRspInd_t *pInd =
                    (zstackmsg_zdoUserDescRspInd_t *)pMsg;

                // Free the descriptor
                if(pInd->rsp.pDesc)
                {
                    OsalPort_free(pInd->rsp.pDesc);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Mgmt Network Discovery Response
        case zstackmsg_CmdIDs_ZDO_MGMT_NWK_DISC_RSP:
            {
                zstackmsg_zdoMgmtNwkDiscRspInd_t *pInd =
                    (zstackmsg_zdoMgmtNwkDiscRspInd_t *)pMsg;

                // Free the network list
                if(pInd->rsp.pNetList)
                {
                    OsalPort_free(pInd->rsp.pNetList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Mgmt LQI Response
        case zstackmsg_CmdIDs_ZDO_MGMT_LQI_RSP:
            {
                zstackmsg_zdoMgmtLqiRspInd_t *pInd =
                    (zstackmsg_zdoMgmtLqiRspInd_t *)pMsg;

                // Free the LQI list
                if(pInd->rsp.pLqiList)
                {
                    OsalPort_free(pInd->rsp.pLqiList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Mgmt Routing Response
        case zstackmsg_CmdIDs_ZDO_MGMT_RTG_RSP:
            {
                zstackmsg_zdoMgmtRtgRspInd_t *pInd =
                    (zstackmsg_zdoMgmtRtgRspInd_t *)pMsg;

                // Free the routing list
                if(pInd->rsp.pRtgList)
                {
                    OsalPort_free(pInd->rsp.pRtgList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Mgmt Binding Response
        case zstackmsg_CmdIDs_ZDO_MGMT_BIND_RSP:
            {
                zstackmsg_zdoMgmtBindRspInd_t *pInd =
                    (zstackmsg_zdoMgmtBindRspInd_t *)pMsg;

                // Free the binding list
                if(pInd->rsp.pBindList)
                {
                    OsalPort_free(pInd->rsp.pBindList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // ZDO Mgmt Network Update Notifiy Indication
        case zstackmsg_CmdIDs_ZDO_MGMT_NWK_UPDATE_NOTIFY:
            {
                zstackmsg_zdoMgmtNwkUpdateNotifyInd_t *pInd =
                    (zstackmsg_zdoMgmtNwkUpdateNotifyInd_t *)pMsg;

                // Free the Energy List
                if(pInd->rsp.pEnergyValuesList)
                {
                    OsalPort_free(pInd->rsp.pEnergyValuesList);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;

        // AF Incoming Message Indication
        case zstackmsg_CmdIDs_AF_INCOMING_MSG_IND:
            {
                zstackmsg_afIncomingMsgInd_t *pInd =
                    (zstackmsg_afIncomingMsgInd_t *)pMsg;

                // Free the message payload
                if(pInd->req.pPayload)
                {
                    OsalPort_free(pInd->req.pPayload);
                }

                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
            break;
        case zstackmsg_CmdIDs_SYS_OTA_MSG_REQ:
            {
                zstackmsg_sysOtaMsg_t *pInd;
                pInd = (zstackmsg_sysOtaMsg_t*)pMsg;

                // Free the message payload
                if(pInd->Req.pData)
                {
                    OsalPort_free(pInd->Req.pData);
                }
                // Free the message
                OsalPort_msgDeallocate(pMsg);
            }
        break;


        // All other messages
        case zstackmsg_CmdIDs_ZDO_CONCENTRATOR_IND:
        case zstackmsg_CmdIDs_ZDO_JOIN_CNF:
        case zstackmsg_CmdIDs_ZDO_LEAVE_IND:
        case zstackmsg_CmdIDs_DEV_PERMIT_JOIN_IND:
        case zstackmsg_CmdIDs_ZDO_TC_DEVICE_IND:
        case zstackmsg_CmdIDs_ZDO_DEVICE_ANNOUNCE:
        case zstackmsg_CmdIDs_ZDO_NODE_DESC_RSP:
        case zstackmsg_CmdIDs_ZDO_POWER_DESC_RSP:
        case zstackmsg_CmdIDs_ZDO_SERVER_DISC_RSP:
        case zstackmsg_CmdIDs_ZDO_BIND_RSP:
        case zstackmsg_CmdIDs_ZDO_END_DEVICE_BIND_RSP:
        case zstackmsg_CmdIDs_ZDO_UNBIND_RSP:
        case zstackmsg_CmdIDs_ZDO_MGMT_LEAVE_RSP:
        case zstackmsg_CmdIDs_ZDO_MGMT_DIRECT_JOIN_RSP:
        case zstackmsg_CmdIDs_ZDO_MGMT_PERMIT_JOIN_RSP:
        case zstackmsg_CmdIDs_AF_DATA_CONFIRM_IND:
        case zstackmsg_CmdIDs_BDB_NOTIFICATION:
        case zstackmsg_CmdIDs_BDB_IDENTIFY_TIME_CB:
        case zstackmsg_CmdIDs_BDB_BIND_NOTIFICATION_CB:
        case zstackmsg_CmdIDs_BDB_TC_LINK_KEY_EXCHANGE_NOTIFICATION_IND:
        case zstackmsg_CmdIDs_BDB_TOUCHLINK_TARGET_ENABLE_IND:
        case zstackmsg_CmdIDs_BDB_CBKE_TC_LINK_KEY_EXCHANGE_IND:
        case zstackmsg_CmdIDs_BDB_FILTER_NWK_DESCRIPTOR_IND:
        case zstackmsg_CmdIDs_GP_COMMISSIONING_MODE_IND:
        case zstackmsg_CmdIDs_GP_DATA_IND:
        case zstackmsg_CmdIDs_GP_SECURITY_REQ:
        case zstackmsg_CmdIDs_PAUSE_DEVICE_REQ:
            // Free the message
            OsalPort_msgDeallocate(pMsg);
            break;

        default:               // Ignore the other messages
            processed = false;
            break;
    }

    return(processed);
}
