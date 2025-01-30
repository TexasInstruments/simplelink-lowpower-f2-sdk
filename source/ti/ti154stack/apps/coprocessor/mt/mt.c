/******************************************************************************

 @file  mt.c

 @brief Monitor/Test event loop functions

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

/******************************************************************************
 Includes
 *****************************************************************************/

#include "npi_task.h"

#include "mt.h"
#include "mt_ext.h"
#include "mt_mac.h"
#include "mt_sys.h"
#include "mt_util.h"

/******************************************************************************
 Macros
 *****************************************************************************/
/*! RPC_CMD response for MT_RES0 commands */
#define MT_SRSP_RES0 ((uint8_t)MTRPC_CMD_SRSP | (uint8_t)MTRPC_SYS_RES0)

/******************************************************************************
 Typedefs
 *****************************************************************************/
/*! Definition of an MT command processing function */
typedef uint8_t (*mtProcessMsg_t)(Mt_mpb_t *pMpb);

/******************************************************************************
 MT Command processing function table
 *****************************************************************************/
/*!
 Table of function pointers to sub-system command processing functions
 This table should have at least MTRPC_SYS_MAX elements, NULL for unused
 */
mtProcessMsg_t mtProcessIncoming[MTRPC_SYS_MAX] =
{
    NULL,                      /* MTRPC_SYS_RES0 */

#if defined(MT_SYS_FUNC)
    MtSys_commandProcessing,   /* MTRPC_SYS_SYS */
#else
    NULL,
#endif

#if defined(MT_MAC_FUNC)
    MtMac_commandProcessing,   /* MTRPC_SYS_MAC */
#else
    NULL,
#endif

    NULL,                      /* MTRPC_SYS_RESERVED03 */

    NULL,                      /* MTRPC_SYS_RESERVED04 */

    NULL,                      /* MTRPC_SYS_RESERVED05 */

    NULL,                      /* MTRPC_SYS_RESERVED06 */

#if defined(MT_UTIL_FUNC)
    MtUtil_commandProcessing,  /* MTRPC_SYS_UTIL */
#else
    NULL,
#endif

    NULL,                      /* MTRPC_SYS_RESERVED08 */

#if defined(MT_UTIL_APP)
    NULL                       /* MTRPC_SYS_APP */
#else
    NULL
#endif
};

/******************************************************************************
 Local variables
 *****************************************************************************/
#if defined(USE_ICALL)
/*! ICall thread entity for NPI */
static ICall_EntityID npiEID;

/*! ICall thread service class for NPI */
static ICall_ServiceEnum npiSID;
#else
uint8_t entityID;

uint8_t npiEID;
uint8_t npiSID;

#endif

/******************************************************************************
 Local function prototypes
 *****************************************************************************/
/*! Format and send MT message to NPI via ICall */
static uint8_t sendNpiMessage(Mt_mpb_t *pMpb);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Initialize the MT system for use with NPI system

 Public function defined in mt.h
 */
#if defined(USE_ICALL)
void MT_init(ICall_EntityID entityID, ICall_ServiceEnum serviceID)
{
    /* ICall IDs for response messages */
    npiEID = entityID;
    npiSID = serviceID;
#else
void MT_init(uint8_t entityID, uint8_t serviceID)
{
    /* ICall IDs for response messages */
    npiEID = entityID;
    npiSID = serviceID;
#endif
    /* Set up extended command processor */
    MtExt_init();

    /* Set up UTIL command processor */
    MtUtil_init();

    /* Reset indication to host */
    MtSys_resetInd();
}

/*!
 Process incoming MT command messages

 Public function defined in mt.h
 */
void MT_processIncoming(uint8_t *pBuf)
{
    uint8_t len = pBuf[MTRPC_POS_LEN];
    uint8_t cmd0 = pBuf[MTRPC_POS_CMD0];
    uint8_t cmd1 = pBuf[MTRPC_POS_CMD1];
    uint8_t status = MTRPC_ERR_LENGTH;

    if(len <= MTRPC_DATA_MAX)
    {
        uint8_t subsys = (cmd0 & MTRPC_SUBSYSTEM_MASK);

        /* In case there's a sub-system problem */
        status = MTRPC_ERR_SUBSYSTEM;

        if(subsys < MTRPC_SYS_MAX)
        {
            /* Ptr to processing function for specified sub-system */
            mtProcessMsg_t func = mtProcessIncoming[subsys];

            if(func != NULL)
            {
                Mt_mpb_t mpb;

                /* Load the message parameters */
                mpb.cmd0 = cmd0;
                mpb.cmd1 = cmd1;
                mpb.length = len;
                mpb.pData = &pBuf[MTRPC_POS_DAT0];

                if(cmd0 & MTRPC_CMD_EXTN)
                {
                    /* Process an extended MT message */
                    status = MtExt_processIncoming(&mpb);

                    /* If last fragment... */
                    if(status == MTRPC_EXT_FRAGDONE)
                    {
                        /* Call processing function for extended message */
                        status = (*func)(&mpb);

                        if(mpb.pData != NULL)
                        {
                            /* Release memory for extended message */
                            MAP_ICall_free(mpb.pData);
                        }
                    }
                }
                else
                {
                    /* Call processing function for standard message */
                    status = (*func)(&mpb);
                }
            }
        }
    }

    /* If there was an error... */
    if(status != MTRPC_SUCCESS)
    {
        /* Send an RPC error response */
        uint8_t rsp[MTRPC_FRAME_HDR_SZ];

        rsp[0] = status;
        rsp[1] = cmd0;
        rsp[2] = cmd1;

        (void)MT_sendResponse(MT_SRSP_RES0, 0, sizeof(rsp), rsp);
    }
}

/*!
 Format and send outgoing MT messages to the NPI task

 Public function defined in mt.h
 */
uint8_t MT_sendResponse(uint8_t type, uint8_t cmd, uint16_t len, uint8_t *pRsp)
{
    uint8_t err;
    Mt_mpb_t mpb;

    mpb.cmd0 = type;
    mpb.cmd1 = cmd;
    mpb.length = len;
    mpb.pData = pRsp;

    if(len <= MTRPC_DATA_MAX)
    {
        /* Standard message - send it directly */
        err = sendNpiMessage(&mpb);
    }
    else
    {
        /* Long message - send via fragmentation */
        err = MtExt_sendMessage(&mpb);
    }

    /* Report dropped message */
    return(err);
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief   Format and send outgoing MT RPC message to the NPI task
 *
 * @param   pMCB - pointer to RPC command/response message control block
 *
 * @return  pMCB - pointer to RPC command/response message control block
 */
static uint8_t sendNpiMessage(Mt_mpb_t *pMpb)
{
    uint8_t err;

    /* Length of data block */
    uint8_t dLen = pMpb->length;

    /* Allocate ICall message buffer to send message to NPI task */
    uint8_t *pRspMsg = MAP_ICall_allocMsg(MTRPC_FRAME_HDR_SZ + dLen);

    if(pRspMsg != NULL)
    {
        /* Populate the MT header fields */
        pRspMsg[MTRPC_POS_LEN] = dLen;
        pRspMsg[MTRPC_POS_CMD0] = pMpb->cmd0;
        pRspMsg[MTRPC_POS_CMD1] = pMpb->cmd1;

        /* Append the data block */
        memcpy(&pRspMsg[MTRPC_POS_DAT0], pMpb->pData, dLen);

        // Send the message to NPI
#if defined(USE_ICALL)
        err = ICall_sendServiceMsg(npiEID, npiSID,
                                   ICALL_MSG_FORMAT_KEEP, pRspMsg);
#else

#ifdef  OSAL_PORT2TIRTOS
        err = OsalPort_msgSend(npiSID,pRspMsg);
#else
        NPITask_sendToHost(pRspMsg);
        err = MTRPC_SUCCESS;
#endif

#endif
    }
    else
    {
        /* Could not get NPI message buffer */
        err = MTRPC_ERR_NOMEMORY;
    }

    /* Report dropped message */
    return(err);
}
