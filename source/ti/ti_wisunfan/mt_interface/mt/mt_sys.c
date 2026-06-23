/******************************************************************************

 @file  mt_sys.c

 @brief Monitor/Test functions for MT SYS commands

 Group: WCS LPC
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2016-2026, Texas Instruments Incorporated
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

#include "sys_ctrl.h"

#include "api_mac.h"
#include "mt_pkt.h"
#include "mt_sys.h"
#include "mt_util.h"

#if defined(MT_SYS_FUNC)
/******************************************************************************
 Macros
 *****************************************************************************/
/*!
 MT Version Information - can be defined by pre-processor declarations
 */
#if !defined(MT_VER_PROTO)
    /*! MT Protocol Version */
    #define MT_VER_PROTO  3
#endif
#if !defined(MT_VER_PROID)
    /*! MT Product ID */
    #define MT_VER_PROID  1
#endif
#if !defined(MT_VER_MAJOR)
    /*! MT Product Major Version */
    #define MT_VER_MAJOR  2
#endif
#if !defined(MT_VER_MINOR)
    /*! MT Product Minor Version */
    #define MT_VER_MINOR  0
#endif
#if !defined(MT_VER_MAINT)
    /*! MT Product Maintenance Version */
    #define MT_VER_MAINT  0
#endif

/*! MT capabilties - defined at compile time */
#define MT_CAPS (MT_CAP_SYS | MT_CAP_MAC | MT_CAP_UTIL | MT_CAP_APP | MT_CAP_RCP_LMAC)
//#define MT_CAPS (MT_CAP_SYS|MT_CAP_RCP_LMAC)

/*! AREQ RPC response for MT_MAC callbacks (indications/confirms) */
#define MT_ARSP_SYS ((uint8_t)MTRPC_CMD_AREQ | (uint8_t)MTRPC_SYS_SYS)
/*! SRSP RPC response for MT_MAC requests (commands/responses) */
#define MT_SRSP_SYS ((uint8_t)MTRPC_CMD_SRSP | (uint8_t)MTRPC_SYS_SYS)

/*! Reset by using DriverLib supplied function */
#define HAL_SYSTEM_RESET() SysCtrlSystemReset()

/******************************************************************************
 Constants
 *****************************************************************************/
/*! System version information for version query and reset indication */
static const uint8_t mtVersion[5] =
{
    MT_VER_PROTO,  /* Transport protocol */
    MT_VER_PROID,  /* Product ID */
    MT_VER_MAJOR,  /* Software major release number */
    MT_VER_MINOR,  /* Software minor release number */
    MT_VER_MAINT   /* Software maintenance release number */
};

/******************************************************************************
  Callback Subscription ID Bit Defintions
  *****************************************************************************/
/*! SYS Callback subscription bit */
#define CBSID_RESET_IND    0x00000001
/*! All SYS callback subscription bits */
#define CBSID_ALL          0x00000001

/*! SYS callback disable command bit */
#define CBSID_DISABLE_CMD  0x80000000

/*! Default callbacks to be enabled at system reset */
#define CBSID_DEFAULT      (CBSID_ALL)



/******************************************************************************
 Local Variables
 *****************************************************************************/
static uint32_t callbackEnables = CBSID_DEFAULT;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
/* MT API command functions */
static void getVersion(Mt_mpb_t *pMpb);
static void pingSystem(Mt_mpb_t *pMpb);


/* Utility functions */
static void sendDRSP(uint8_t rspId, uint16_t rspLen, uint8_t *rspPtr);
static void sendSRSP(uint8_t rId, uint8_t rsp);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Processes MT SYS commands received from the host

 Public function that is defined in mt_sys.h
 */
uint8_t MtSys_commandProcessing(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_success;

    switch(pMpb->cmd1)
    {
        case MT_SYS_RESET_REQ:
            MtSys_resetReq(MTSYS_RESET_HOST);
            break;

        case MT_SYS_PING:
            pingSystem(pMpb);
            break;

        case MT_SYS_VERSION:
            getVersion(pMpb);
            break;
        
        //MV to do remove all NV related constructs
        case MT_SYS_NV_CREATE:            
        case MT_SYS_NV_DELETE:
        case MT_SYS_NV_LENGTH:
        case MT_SYS_NV_READ:
        case MT_SYS_NV_WRITE:
        case MT_SYS_NV_UPDATE:
        case MT_SYS_NV_COMPACT:            
        status = ApiMac_status_success;
            break;

        default:
            status = ApiMac_status_commandIDError;
            break;
    }

    return(status);
}

/*!
 Send an MT "reset response" message

 Public function that is defined in mt_sys.h
 */
void MtSys_resetInd(void)
{
    uint8_t rsp[1 + sizeof(mtVersion)];

    /* Reason for the reset */
    rsp[0] = MTSYS_RESET_HARD;

    /* MT_MAC revision info */
    memcpy(&rsp[1], mtVersion, sizeof(mtVersion));

    if(callbackEnables & CBSID_RESET_IND)
    {
        /* Send out Reset Response message */
        (void)MT_sendResponse(MT_ARSP_SYS, MTSYS_RESET_IND, sizeof(rsp), rsp);
    }
}

/*!
 Process a Reset Request

 Public function that is defined in mt_sys.h
 */
void MtSys_resetReq(uint8_t reason)
{
    /* Pull the plug and start over */
    HAL_SYSTEM_RESET();
}

/*!
 Update enabled MT SYS callbacks

 Public function that is defined in mt_sys.h
 */
uint32_t MtSys_setCallbacks(uint32_t cbBits)
{
    if(cbBits & CBSID_DISABLE_CMD)
    {
        /* Disable callbacks for bits that are set */
        callbackEnables &= ~(cbBits ^ CBSID_DISABLE_CMD);
    }
    else
    {
        /* Enable callbacks for bits that are set */
        callbackEnables |= (cbBits & CBSID_ALL);
    }

    /* Tell caller which callbacks are enabled */
    return(callbackEnables);
}

/******************************************************************************
 Local API Command Functions
 *****************************************************************************/
/*!
 * @brief   Process the SYS_VERSION command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void getVersion(Mt_mpb_t *pMpb)
{
    /* Send back the version of this build */
    sendDRSP(MT_SYS_VERSION, sizeof(mtVersion),(uint8_t*)mtVersion);
}

/*!
 * @brief   Process the SYS_PING command issued by host
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void pingSystem(Mt_mpb_t *pMpb)
{
    uint8_t rspBuf[sizeof(uint16_t)];

    (void)Util_bufferUint16(rspBuf, MT_CAPS);

    /* Build and send back the response */
    sendDRSP(MT_SYS_PING, sizeof(rspBuf), rspBuf);
}

/******************************************************************************
 Local Utility Functions
 *****************************************************************************/
/*!
 * @brief   Wrapper for MT_sendResponse() for MT_SYS ARSP
 *
 * @param   rspId  - command ID
 * @param   rspLen - length of response buffer
 * @param   rspPtr - pointer to response buffer
 */
static void sendDRSP(uint8_t rspId, uint16_t rspLen, uint8_t *rspPtr)
{
    (void)MT_sendResponse(MT_SRSP_SYS, rspId, rspLen, rspPtr);
}

/*!
 * @brief   Wrapper for MT_sendResponse() for status SRSP
 *
 * @param   rspId - response ID
 * @param   rsp   - response status
 */
static void sendSRSP(uint8_t rspId, uint8_t rsp)
{
    (void)MT_sendResponse(MT_SRSP_SYS, rspId, 1, &rsp);
}

#endif /* MT_SYS_FUNC */
