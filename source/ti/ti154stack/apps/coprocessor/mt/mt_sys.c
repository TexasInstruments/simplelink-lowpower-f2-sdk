/******************************************************************************

 @file  mt_sys.c

 @brief Monitor/Test functions for MT SYS commands

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

#include "sys_ctrl.h"

#include "api_mac.h"
#include "macconfig.h"
#include "nvintf.h"

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
#define MT_CAPS (MT_CAP_SYS | MT_CAP_MAC | MT_CAP_UTIL | MT_CAP_APP)

/*! AREQ RPC response for MT_MAC callbacks (indications/confirms) */
#define MT_ARSP_SYS ((uint8_t)MTRPC_CMD_AREQ | (uint8_t)MTRPC_SYS_SYS)
/*! SRSP RPC response for MT_MAC requests (commands/responses) */
#define MT_SRSP_SYS ((uint8_t)MTRPC_CMD_SRSP | (uint8_t)MTRPC_SYS_SYS)

/*! Reset by using DriverLib supplied function */
#define HAL_SYSTEM_RESET() SysCtrlSystemReset()

/*! NV driver item ID for reset reason */
#define MTSYS_NVID_RESET {NVINTF_SYSID_APP, 1, 0}

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

/*! NV driver item ID for reset reason */
static const NVINTF_itemID_t nvResetId = MTSYS_NVID_RESET;

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
 External Variables
 *****************************************************************************/
extern mac_Config_t Main_user1Cfg;

/******************************************************************************
 Local Variables
 *****************************************************************************/
static uint32_t callbackEnables = CBSID_DEFAULT;
static NVINTF_nvFuncts_t *pNV = &Main_user1Cfg.nvFps;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
/* MT API command functions */
static void compactNvPage(Mt_mpb_t *pMpb);
static void createNvItem(Mt_mpb_t *pMpb);
static void deleteNvItem(Mt_mpb_t *pMpb);
static void getNvLength(Mt_mpb_t *pMpb);
static void getVersion(Mt_mpb_t *pMpb);
static void pingSystem(Mt_mpb_t *pMpb);
static void readNvItem(Mt_mpb_t *pMpb);
static void writeNvItem(Mt_mpb_t *pMpb);

/* Utility functions */
static uint8_t mapNvError(uint8_t error);
static uint8_t *parseNvExtId(uint8_t *pBuf, NVINTF_itemID_t *nvId);
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

        case MT_SYS_NV_CREATE:
            createNvItem(pMpb);
            break;

        case MT_SYS_NV_DELETE:
            deleteNvItem(pMpb);
            break;

        case MT_SYS_NV_LENGTH:
            getNvLength(pMpb);
            break;

        case MT_SYS_NV_READ:
            readNvItem(pMpb);
            break;

        case MT_SYS_NV_WRITE:
        case MT_SYS_NV_UPDATE:
            writeNvItem(pMpb);
            break;

        case MT_SYS_NV_COMPACT:
            compactNvPage(pMpb);
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

    if(pNV->readItem != NULL)
    {
        /* Attempt to retrieve reason for the reset */
        (void)pNV->readItem(nvResetId, 0, 1, &rsp[0]);
    }

    if(pNV->deleteItem != NULL)
    {
        /* Only use this reason once */
        (void)pNV->deleteItem(nvResetId);
    }

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
    if(pNV->writeItem != NULL)
    {
        /* Attempt to save reason to read after reset */
        (void)pNV->writeItem(nvResetId, 1, &reason);
    }

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
 * @brief   Attempt to compact the active NV page
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void compactNvPage(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_nvCompact_t))
    {
        if(pNV->compactNV == NULL)
        {
            /* NV item compact function not available */
            status = ApiMac_status_unsupported;
        }
        else
        {
            uint16_t minSize;

            /* Get the remaining size threshold */
            minSize = Util_parseUint16(pMpb->pData);

            /* Attempt to compact the active NV page */
            status = mapNvError(pNV->compactNV(minSize));
        }
    }

    /* Build and send back the response */
    sendSRSP(MT_SYS_NV_COMPACT, status);
}

/*!
 * @brief   Attempt to create an NV item
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void createNvItem(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_nvCreate_t))
    {
        if(pNV->createItem == NULL)
        {
            /* NV item create function not available */
            status = ApiMac_status_unsupported;
        }
        else
        {
            uint32_t nvLen;
            NVINTF_itemID_t nvId;
            uint8_t *pBuf = pMpb->pData;

            /* Get the NV ID parameters */
            pBuf = parseNvExtId(pBuf, &nvId);

            /* Get the length */
            nvLen = Util_parseUint32(pBuf);

            /* Attempt to create the specified item with no initial data */
            status = mapNvError(pNV->createItem(nvId, nvLen, NULL));
        }
    }

    /* Build and send back the response */
    sendSRSP(MT_SYS_NV_CREATE, status);
}

/*!
 * @brief   Attempt to delete an NV item
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void deleteNvItem(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_nvDelete_t))
    {
        if(pNV->deleteItem == NULL)
        {
            /* NV item delete function not available */
            status = ApiMac_status_unsupported;
        }
        else
        {
            NVINTF_itemID_t nvId;

            /* Get the NV ID parameters */
            parseNvExtId(pMpb->pData, &nvId);

            /* Attempt to delete the specified item */
            status = mapNvError(pNV->deleteItem(nvId));
        }
    }

    /* Build and send back the response */
    sendSRSP(MT_SYS_NV_DELETE, status);
}

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
 * @brief   Attempt to get the length of an NV item
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void getNvLength(Mt_mpb_t *pMpb)
{
    uint32_t nvLen = 0;
    uint8_t rspBuf[sizeof(uint32_t)];

    if(pMpb->length == sizeof(MtPkt_nvLength_t))
    {
        if(pNV->getItemLen != NULL)
        {
            NVINTF_itemID_t nvId;

            /* Get the NV ID parameters */
            parseNvExtId(pMpb->pData, &nvId);

            /* Attempt to get length of the specified item */
            nvLen = pNV->getItemLen(nvId);
        }
    }

    /* Serialize the length bytes */
    (void)Util_bufferUint32(rspBuf, nvLen);

    /* Build and send back the response */
    sendDRSP(MT_SYS_NV_LENGTH, sizeof(rspBuf), rspBuf);
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

/*!
 * @brief   Attempt to read an NV item
 *
 * @param   pMpb - pointer to incoming message parameter block
 */
static void readNvItem(Mt_mpb_t *pMpb)
{
    uint8_t status = ApiMac_status_lengthError;

    if(pMpb->length == sizeof(MtPkt_nvRead_t))
    {
        if(pNV->readItem == NULL)
        {
            /* NV item length/read function not available */
            status = ApiMac_status_unsupported;
        }
        else
        {
            uint8_t dataLen;
            uint16_t dataOfs;
            uint8_t *pRspBuf;
            uint8_t respLen = 2;  /* Rsp header: [0]=status,[1]=length */
            NVINTF_itemID_t nvId;
            uint8_t *pBuf = pMpb->pData;

            /* Get the NV ID parameters */
            pBuf = parseNvExtId(pBuf, &nvId);

            /* Get the read data offset */
            dataOfs = Util_parseUint16(pBuf);

            /* And the read data length */
            dataLen = pBuf[2];

            if(dataLen > (MTRPC_DATA_MAX - respLen))
            {
                /* Data length is limited by TX buffer size and MT protocol */
                dataLen = (MTRPC_DATA_MAX - respLen);
            }
            respLen += dataLen;

            pRspBuf = MAP_ICall_malloc(respLen);
            if(pRspBuf != NULL)
            {
                /* Attempt to read data from the specified item */
                status = pNV->readItem(nvId, dataOfs, dataLen, pRspBuf+2);
                if(status == ApiMac_status_success)
                {
                    pRspBuf[0] = status;
                    pRspBuf[1] = dataLen;
                    sendDRSP(MT_SYS_NV_READ, respLen, pRspBuf);
                }
                else
                {
                    /* Convert to ApiMac error code */
                    status = mapNvError(status);
                }
                MAP_ICall_free(pRspBuf);
            }
            else
            {
                /* Could not get buffer for NV data */
                status = ApiMac_status_noResources;
            }
        }
    }

    if(status != ApiMac_status_success)
    {
        uint8_t tmp[2] = {status, 0};
        sendDRSP(MT_SYS_NV_READ, sizeof(tmp), tmp);
    }
}

/*!
 * @brief   Attempt to write an NV item
 *
 * @param   pBuf - pointer to incoming data
 */
static void writeNvItem(Mt_mpb_t *pMpb)
{
    uint8_t cmdId;
    uint8_t status;

    /* MT command ID */
    cmdId = pMpb->cmd1;

    if(pNV->writeItem == NULL)
    {
        /* NV item length/read function not available */
        status = ApiMac_status_unsupported;
    }
    else
    {
        uint8_t dataLen;
        NVINTF_itemID_t nvId;
        uint8_t *pBuf = pMpb->pData;

        /* Get the NV ID parameters */
        pBuf = parseNvExtId(pBuf, &nvId);

        if(cmdId == MT_SYS_NV_WRITE)
        {
            /* Get data offset for Write command */
            pBuf += 2;
        }

        /* Get the write data length */
        dataLen = *pBuf++;

        if((cmdId == MT_SYS_NV_UPDATE) &&
           (pMpb->length == (sizeof(MtPkt_nvUpdate_t) + dataLen)))
        {
            /* Attempt to update (create) data to the specified item */
            status = mapNvError(pNV->writeItem(nvId, dataLen, pBuf));
        }
        else if((cmdId == MT_SYS_NV_WRITE) &&
                (pMpb->length == (sizeof(MtPkt_nvWrite_t) + dataLen)))
        {
            /* Attempt to write data (existing) to the specified item */
            status = mapNvError(pNV->writeItem(nvId, dataLen, pBuf));
        }
        else
        {
            /* Invalid incoming message length */
            status = ApiMac_status_lengthError;
        }
    }

    /* Build and send back the response */
    sendSRSP(cmdId, status);
}

/******************************************************************************
 Local Utility Functions
 *****************************************************************************/
/*!
 * @brief   Map NV error code into ApiMac error code
 *
 * @param   nvErr - error code returned from NV function
 *
 * @return  error - ApiMac error code
 */
static uint8_t mapNvError(uint8_t nvErr)
{
    uint8_t error;

    switch(nvErr)
    {
        case NVINTF_SUCCESS:
            error = ApiMac_status_success;
            break;

        case NVINTF_BADPARAM:
        case NVINTF_BADLENGTH:
        case NVINTF_BADOFFSET:
        case NVINTF_BADITEMID:
        case NVINTF_BADSUBID:
        case NVINTF_BADSYSID:
        case NVINTF_NOTFOUND:
            error = ApiMac_status_invalidParameter;
            break;

        case NVINTF_FAILURE:
        case NVINTF_CORRUPT:
        case NVINTF_NOTREADY:
        case NVINTF_LOWPOWER:
        case NVINTF_BADVERSION:
        default:
            error = ApiMac_status_unsupported;
            break;
    }

    return(error);
}

/*!
 * @brief   Parse the incoming NV ID parameters
 *
 * @param   pBuf - pointer to incoming data
 * @param   nvId - pointer to outgoing NV ID
 *
 * @return  pointer to next incoming data byte
 */
static uint8_t *parseNvExtId(uint8_t *pBuf, NVINTF_itemID_t *nvId)
{
    nvId->systemID = pBuf[0];
    nvId->itemID = Util_parseUint16(pBuf+1);
    nvId->subID = Util_parseUint16(pBuf+3);

    return(pBuf + 5);
}

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
