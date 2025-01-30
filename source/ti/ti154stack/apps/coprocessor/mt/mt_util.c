/******************************************************************************

 @file  mt_util.c

 @brief MonitorTest functions for MT UTIL commands

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
#include <string.h>

#include "ti_drivers_config.h"
#include "util_timer.h"
#include <inc/hw_ccfg_simple_struct.h>
#include <inc/hw_memmap.h>

#include "mt_mac.h"
#include "mt_pkt.h"
#include "mt_sys.h"
#include "mt_util.h"

#if defined(MT_UTIL_FUNC)
/******************************************************************************
 Macros
 *****************************************************************************/
/*! AREQ RPC response for MT_MAC callbacks (indications/confirms) */
#define MT_ARSP_UTIL ((uint8_t)MTRPC_CMD_AREQ | (uint8_t)MTRPC_SYS_UTIL)
/*! SRSP RPC response for MT_MAC requests (commands/responses) */
#define MT_SRSP_UTIL ((uint8_t)MTRPC_CMD_SRSP | (uint8_t)MTRPC_SYS_UTIL)

/*! Extended Address offset in FCFG (LSB..MSB) */
#define INFO_EXTADDR_OFFSET 0x2F0
/*! Memory location of cutomer-configured IEEE address */
#define CCFG_IEEE ((uint8_t *)&(__ccfg.CCFG_IEEE_MAC_0))
/*! Memory location of unique factory-programmed IEEE address */
#define INFO_IEEE ((uint8_t *)(FCFG1_BASE + INFO_EXTADDR_OFFSET))

/*! Default loopback timer (milliseconds) */
#define DEFAULT_LOOPBACK_TIME  1000

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
/* MT API command functions */
static void getExtAddr(Mt_mpb_t *pMpb);
static void getRandomNbr(Mt_mpb_t *pMpb);
static void sendLoopBack(Mt_mpb_t *pMpb);
static void setCallbacks(Mt_mpb_t *pMpb);

/* Utility functions */
static void loopTimerCB(uintptr_t a0);
static void sendARSP(uint8_t rspId, uint16_t rspLen, uint8_t *rspPtr);
static void sendSRSP(uint8_t rspId, uint16_t rspLen, uint8_t *rspPtr);

/******************************************************************************
 Local Variables
 *****************************************************************************/
/*! Length of loopback buffer */
static uint8_t loopCount;
/*! Length of loopback buffer */
static uint16_t loopLength;
/*! Ptr to loopback data buffer */
static uint8_t *pLoopData = NULL;

/*! Clock handle for loopbacks */
static ClockP_Handle clkHandle;
/*! Clock structure for loopbacks */
static ClockP_Struct clkStruct;

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Processes MT UTIL commands received from the host

 Public function that is defined in mt_util.h
 */
uint8_t MtUtil_commandProcessing(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_success;

    switch (pMpb->cmd1)
    {
        case MT_UTIL_CALLBACK_SUB:
            setCallbacks(pMpb);
            break;

        case MT_UTIL_LOOPBACK:
            sendLoopBack(pMpb);
            break;

        case MT_UTIL_RANDOM:
            getRandomNbr(pMpb);
            break;

        case MT_UTIL_EXT_ADDR:
            getExtAddr(pMpb);
            break;

        default:
            status = ApiMac_status_commandIDError;
            break;
    }

    return(status);
}

/*!
 Initialize the MT UTIL command processor

 Public function that is defined in mt_util.h
 */
void MtUtil_init(void)
{
    /* Allocate a loopback timer but don't start it */
    clkHandle = UtilTimer_construct(&clkStruct, loopTimerCB, 100, 0, false, 0);
}
/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief   Process UTIL_CALLBACK_SUB_CMD command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void setCallbacks(Mt_mpb_t *pMpb)
{
    uint32_t bitMask = 0x00000000;
    uint8_t rsp[sizeof(MtPkt_callBack_t)];

    if(pMpb->length == sizeof(MtPkt_callBack_t))
    {
        uint8_t subSys;
        uint8_t *pBuf = pMpb->pData;

        /* MT sub-system to be updated */
        subSys = *pBuf++;

        /* Bit mask of callbacks to enable/disable */
        bitMask = Util_parseUint32(pBuf);

        rsp[0] = ApiMac_status_success;

        switch(subSys)
        {
            case MTRPC_SYS_SYS:
                bitMask = MtSys_setCallbacks(bitMask);
                break;

            case MTRPC_SYS_MAC:
                bitMask = MtMac_setCallbacks(bitMask);
                break;

            default:
                rsp[0] = ApiMac_status_subSystemError;
                break;
        }
    }
    else
    {
        /* Invalid incoming message length */
        rsp[0] = ApiMac_status_lengthError;
    }

    /* Send back the updated bitmask */
    (void)Util_bufferUint32(&rsp[1], bitMask);

    sendSRSP(MT_UTIL_CALLBACK_SUB, sizeof(rsp), rsp);
}

/*!
 * @brief   Process UTIL_RANDOM command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void getRandomNbr(Mt_mpb_t *pMpb)
{
    uint8_t retArray[sizeof(uint16_t)];

    retArray[0] = ApiMac_randomByte();
    retArray[1] = ApiMac_randomByte();

    sendSRSP(MT_UTIL_RANDOM, sizeof(retArray), retArray);
}

/*!
 * @brief   Process UTIL_EXT_ADDR command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void getExtAddr(Mt_mpb_t *pMpb)
{
    uint8_t *pBuf = pMpb->pData;
    uint8_t rsp[1 + APIMAC_SADDR_EXT_LEN];

    /* Echo back the requested address type */
    rsp[0] = (pMpb->length == sizeof(MtPkt_extAddr_t)) ? *pBuf : 0xFF;

    switch(rsp[0])
    {
        case 0:
            /* Get address from the MAC */
            ApiMac_mlmeGetReqArray(ApiMac_attribute_extendedAddress, &rsp[1]);
            break;

        case 1:
            /* Get TI factory address from INFO memory */
            memcpy(&rsp[1], INFO_IEEE, APIMAC_SADDR_EXT_LEN);
            break;

        case 2:
            /* Get User-Configured address from CCFG memory */
            memcpy(&rsp[1], CCFG_IEEE, APIMAC_SADDR_EXT_LEN);
            break;

        default:
            /* Return 0xFF and 0xFFFFFFFF for unknown */
            memset(rsp, 0xFF, sizeof(rsp));
            break;
    }

    sendSRSP(MT_UTIL_EXT_ADDR, sizeof(rsp), rsp);
}

/*!
 * @brief   Process MT_UTIL_LOOPBACK command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void sendLoopBack(Mt_mpb_t *pMpb)
{
    /* Length of parameters and data */
    uint16_t rspLen = pMpb->length;

    /* Ptr to parameters and data */
    uint8_t *pBuf = (uint8_t*)pMpb->pData;

    if(rspLen >= sizeof(MtPkt_loopBack_t))
    {
        /* Requested number of AREQ responses */
        loopCount = pBuf[0];

        if(pLoopData == NULL)
        {
            if(loopCount > 0)
            {
                /* AREQ responses are requested */
                pLoopData = MAP_ICall_malloc(rspLen);
                if(pLoopData != NULL)
                {
                    uint32_t timeout = Util_parseUint32(&pBuf[1]);

                    if(timeout == 0)
                    {
                        /* Caller did not provide a timeout, we will */
                        timeout = DEFAULT_LOOPBACK_TIME;
                        /* Tell caller what we replaced it with */
                        (void)Util_bufferUint32(&pBuf[1], timeout);
                    }

                    /* Keep length of AREQ loopback buffer */
                    loopLength = rspLen;

                    /* And a copy of data to loopback later */
                    memcpy(pLoopData, pBuf, rspLen);

                    /* Set up the callback timeout */
                    UtilTimer_setTimeout(clkHandle, timeout);

                    /* Schedule the first callback */
                    UtilTimer_start(&clkStruct);
                }
            }
        }
    }

    /* After command processing, send back what was received */
    sendSRSP(MT_UTIL_LOOPBACK, rspLen, pBuf);
}

/*!
 * @brief   Loopback timeout handler
 *
 * @param   a0 - ignored
 */
static void loopTimerCB(uintptr_t a0)
{
    if(pLoopData != NULL)
    {
        if(loopCount > 0)
        {
            if(loopCount < 0xFF)
            {
                /* Not 'forever' - decrement repeat counter */
                loopCount--;

                /* Tell caller how many more times */
                pLoopData[0] = loopCount;
            }

            /* Send an AREQ loopback */
            sendARSP(MT_UTIL_LOOPBACK, loopLength, pLoopData);
        }

        if(loopCount > 0)
        {
            /* Schedule next callback */
            UtilTimer_start(&clkStruct);
        }
        else
        {
            /* No more callbacks */
            UtilTimer_stop(&clkStruct);

            /* Give back the buffer */
            MAP_ICall_free(pLoopData);
            pLoopData = NULL;
        }
    }
}

/******************************************************************************
 Local Utility Functions
 *****************************************************************************/
/*!
 * @brief   Wrapper for MT_sendResponse() for MT_UTIL ARSP
 *
 * @param   rspId  - command ID
 * @param   rspLen - length of response buffer
 * @param   rspPtr - pointer to response buffer
 */
static void sendARSP(uint8_t rspId, uint16_t rspLen, uint8_t *rspPtr)
{
    (void)MT_sendResponse(MT_ARSP_UTIL, rspId, rspLen, rspPtr);
}

/*!
 * @brief   Wrapper for MT_sendResponse() for MT_UTIL SRSP
 *
 * @param   rspId  - command ID
 * @param   rspLen - length of response buffer
 * @param   rspPtr - pointer to response buffer
 */
static void sendSRSP(uint8_t rspId, uint16_t rspLen, uint8_t *rspPtr)
{
    (void)MT_sendResponse(MT_SRSP_UTIL, rspId, rspLen, rspPtr);
}

#endif /* MT_UTIL_FUNC */
