/******************************************************************************

 @file  oad_profile.c

 @brief This is the TI proprietary OAD profile.

 Group: WCS, BTS
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2022-2025, Texas Instruments Incorporated
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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "onboard.h"
#include "bcomdef.h"
#include "util.h"
#include "ti/bleapp/ble_app_util/inc/bleapputil_api.h"
#include "ti/bleapp/profiles/oad/oad_profile.h"
#include "ti/bleapp/util/sw_update/sw_update.h"

#ifdef OAD_APP_ONCHIP
#include "ti/bleapp/services/oad/oad_reset_service.h"
#else
#include "ti/bleapp/services/oad/oad_service.h"
#endif //OAD_APP_ONCHIP

/*********************************************************************
 * MACROS
 */
#ifndef OAD_APP_ONCHIP
  /*
  * Macro to parse op-code from external control command
  */
#define EXT_CTRL_OP_CODE(pData)(pData[0])
#endif //OAD_APP_ONCHIP
/*********************************************************************
 * TYPEDEFS
 */
/*!
 * @OADProfile_Status_e OAD status codes
 * @{
 */
typedef enum OADProfile_Status_e
{
    OAD_PROFILE_SUCCESS            = 0,  //!< OAD succeeded.
    OAD_PROFILE_VALIDATION_ERR     = 1,  //!< Downloaded image header doesn't match.
    OAD_PROFILE_FLASH_ERR          = 2,  //!< Flash function failure (int, ext).
    OAD_PROFILE_BUFFER_OFL         = 3,  //!< Block Number doesn't match requested.
    OAD_PROFILE_ALREADY_STARTED    = 4,  //!< OAD is already is progress.
    OAD_PROFILE_NOT_STARTED        = 5,  //!< OAD has not yet started.
    OAD_PROFILE_DL_NOT_COMPLETE    = 6,  //!< An OAD is ongoing.
    OAD_PROFILE_NO_RESOURCES       = 7,  //!< If memory allocation fails.
    OAD_PROFILE_IMAGE_TOO_BIG      = 8,  //!< Candidate image is too big.
    OAD_PROFILE_INCOMPATIBLE_IMAGE = 9,  //!< Image signing failure, boundary mismatch.
    OAD_PROFILE_INVALID_FILE       = 10, //!< If Invalid image ID received.
    OAD_PROFILE_INCOMPATIBLE_FILE  = 11, //!< BIM/MCUBOOT or FW mismatch.
    OAD_PROFILE_AUTH_FAIL          = 12, //!< Authorization failed.
    OAD_PROFILE_EXT_NOT_SUPPORTED  = 13, //!< Ctrl point command not supported.
    OAD_PROFILE_DL_COMPLETE        = 14, //!< OAD image payload download complete.
    OAD_PROFILE_CCCD_NOT_ENABLED   = 15, //!< CCCD is not enabled, notif can't be sent.
    OAD_PROFILE_IMG_ID_TIMEOUT     = 16, //!< Image identify timed out, too many failures.
    OAD_PROFILE_APP_STOP_PROCESS   = 17, //!< Target app cancel oad
    OAD_PROFILE_ERROR              = 18, //!< General internal error of the module
}OADProfile_Status_e;

#ifndef OAD_APP_ONCHIP
/*!
 * OAD state types
 */
typedef enum
{
    OAD_IDLE,                   //!< OAD module is not performing any action
    OAD_CONFIG,                 //!< OAD module is configuring for a download
    OAD_DOWNLOAD,               //!< OAD module is receiving an image
    OAD_COMPLETE                //!< OAD module has completed the process
}oadState_e;

/*!
 * OAD event types
 */

typedef enum
{
    OAD_EVT_IMG_IDENTIFY_REQ = 0x00,  //Image identify request - his event occurs when new oad process begin with new image request.

    OAD_EVT_BLOCK_REQ        = 0x01,  //Block request - This event occurs when new block is arrived

    OAD_EVT_TIMEOUT          = 0x02,  //Timeout - This event occurs by timeout clock interrupt

    OAD_EVT_START_OAD        = 0x03,  //Start OAD - This command is used after configuration state is done and we change to download.

    OAD_EVT_ENABLE_IMG       = 0x04,  //Enable image - This command is used to enable an image after download.

    OAD_EVT_CANCEL_OAD       = 0x05,  //Cancel OAD - This command is used to cancel the OAD process.

}oadEvent_e;


//Some of the opcode dosen't exist anymore, and we save the orginal opcode number for backward compatibility
typedef enum
{
    OAD_REQ_GET_BLK_SZ        = 0x01,  //Get Block Size - This command is used by a peer to determine
                                       //what is the largest block size the target can support.

    OAD_REQ_DISABLE_BLK_NOTIF = 0x06,  //Disable block notification - This command is used to disable the image
                                       //block request notifications.
    OAD_REQ_GET_SW_VER        = 0x07,  //Get software version - This command is used to query the OAD target
                                       //device for its software version.
    OAD_REQ_GET_OAD_STAT      = 0x08,  //Get oad  publiv state machine - This command is used to query the status of the OAD
                                       //process OAD_PROFILE_NOT_STARTED/OAD_PROFILE_ALREADY_STARTED.

  //these opcodes are not supported yet
  //OAD_REQ_GET_PROF_VER      = 0x09,  //Get profile version - This command is used to query the version of
                                            //the OAD profile.

  //OAD_REQ_GET_DEV_TYPE      = 0x10, //Get device type - This command is used to query type of the device the
                                      //profile is running on.

  //OAD_REQ_GET_IMG_INFO      = 0x11, //Get image info - This command is used to get the image info structure
                                      //corresponding to the the image asked for.

    OAD_RSP_BLK_RSP_NOTIF     = 0x12, //Send block request - This command is used to send a block request
                                            //notification to the peer device.

    OAD_REQ_ERASE_BONDS       = 0x13, //Erase bonds - This command is used to erase all BLE bonding info
                                           //on the device.
    OAD_RSP_CMD_NOT_SUPPORTED = 0xFF, //Error code returned when an external control command is received
                                                 //with an invalid opcode.

}oadProtocolOPCode_e;

/*!
 * A generic external control response command
 * Most commands simply return a status, this will cover those commands
 */

// Temporary workaround needed due to a collision on the __packed definition.
#ifdef __IAR_SYSTEMS_ICC__
typedef struct __attribute__((__packed__))
#else
PACKED_TYPEDEF_STRUCT
#endif
{
    uint8       cmdID;            //!< Ext Ctrl Op-code
    uint8       status;           //!< Status of command
} genericExtCtrlRsp_t;

/*!
 * Response to a @ref OAD_REQ_GET_BLK_SZ command
 */
// Temporary workaround needed due to a collision on the __packed definition.
#ifdef __IAR_SYSTEMS_ICC__
typedef struct __attribute__((__packed__))
#else
PACKED_TYPEDEF_STRUCT
#endif
{
    uint8     cmdID;          //!< Ext Ctrl Op-code
    uint16    oadBlkSz;       //!< OAD block size
} blockSizeRspPld_t;

/*!
 * Block request payload
 */
// Temporary workaround needed due to a collision on the __packed definition.
#ifdef __IAR_SYSTEMS_ICC__
typedef struct __attribute__((__packed__))
#else
PACKED_TYPEDEF_STRUCT
#endif
{
    uint8              cmdID;            //!< External control op-code
    uint8              prevBlkStat;      //!< Status of previous block write
    uint32             requestedBlk;     //!< Requested block number
}blockReqPld_t;

/*!
 * Response to a @ref OAD_REQ_GET_SW_VER command
 */
// Temporary workaround needed due to a collision on the __packed definition.
#ifdef __IAR_SYSTEMS_ICC__
typedef struct __attribute__((__packed__))
#else
PACKED_TYPEDEF_STRUCT
#endif
{
    uint8       cmdID;                     //!< Ctrl Op-code
    uint8       swVer[MCUBOOT_SW_VER_LEN]; //!< App version
} swVersionPld_t;

/*!
 * This struct contains all the global information that the module needs
 */
typedef struct
{
    oadState_e          state;            //!< OAD state machine state
    OADProfile_Status_e publicState;      //!< Store the publice machine state, which the peer can request
    uint16              blkSize;          //!< Block size include header
    uint16              imgBytesPerBlock; //!< Block size without header
    uint32              candidateImgLen;  //!< Total length of candidate image
    uint32              totalBlocks;      //!< Total number of blocks - derived from the size of the image
    uint8               lastBlockSize;    //!< For partial last block calculation, 0 means last block are in regular blkSize
    uint32              currentBlkNum;    //!< Current block count in active oad process
    uint16              activeConnHandle; //!< OAD is only allowed via a single connection, store it here
    uint8               blkReqActive;     //!< flag whether to send the peer an update after each block
    uint8               imgIDRetries;     //!< Number of retries allowed on image identify
    uint32              stateTimeout;     //!< After inactivity time oad process will reset
} oadModuleGlobalData_t;

/*********************************************************************
 * CONSTANTS
 */

/*!
 * Minimum OAD inactivity timeout, the OAD state machine will cancel OAD
 * if no activity is observed during this time.
 */
#define OAD_MIN_INACTIVITY_TIME         4000

/*!
 * The ATT header overhead required to send a GATT_notifiation()
 * This is used in calculating acceptable block sizes
 */
#define OAD_ATT_PADD                    3

/*!
 * The size of the OAD block number in a block write or block request
 */
#define OAD_BLK_NUM_HDR_SZ              4

/*!
 * Maximum OAD block size
 * OAD Block size can range from [OAD_DEFAULT_BLOCK_SIZE,OAD_MAX_BLOCK_SIZE]
 * \note OAD block size must be 4 Octet aligned
 */
#define OAD_MAX_BLOCK_SIZE              240

/*!
 * Size of the payload in an image identify response.
 */
#define OAD_IMAGE_ID_RSP_LEN            0x01

/*!
 * Default OAD inactivity timeout, the OAD state machine will cancel OAD
 * if no activity is observed during this time.
 *
 */
#define OAD_DEFAULT_INACTIVITY_TIME     10000
/*!
 * Number of failed image IDs are allowed before OAD terminates the connection
 */
#define OAD_IMG_ID_RETRIES              3

/*!
 * OAD Block size can range from [OAD_DEFAULT_BLOCK_SIZE,OAD_MAX_BLOCK_SIZE]
 * \note OAD block size must be 4 Octet aligned
 */
#define OAD_DEFAULT_BLOCK_SIZE          240

/*!
 * The following 2 definitions are for this function: HCI_LE_WriteSuggestedDefaultDataLenCmd
 * the explanation of them is indicated near the use of the function
 */

#define APP_SUGGESTED_PDU_SIZE 247 //default is 27 octets(TX)
#define APP_SUGGESTED_TX_TIME 2120 //default is 328us(TX)

#endif //OAD_APP_ONCHIP

/*********************************************************************
 * LOCAL VARIABLES
 */

static OADProfile_AppCallback_t oadProfileCB = NULL;
#ifndef OAD_APP_ONCHIP

/*!
 * Allocating space for this variable is mandatory for the module,
 * so there is no scenario that the internal functions will be called
 * without this variable being assigned, so there is no need to check
 * whether the variable has been assigned at the beginning
 * of each internal function.
 */
static oadModuleGlobalData_t* pOADModuleGlobalData;

// OAD Activity Clock
static Clock_Struct oadActivityClk;
#endif //OAD_APP_ONCHIP

/*********************************************************************
 * LOCAL FUNCTIONS - Prototypes
 */
#ifdef OAD_APP_ONCHIP
void oadInvokeFromFWContextOADReset(char *pData);
#else //OAD_APP_OFFCHIP || OAD_PERSISTENT
void oadInvokeFromFWContextOAD(char *pData);
void oadInvokeFromFWContextTimeoutOAD(char *pData);

static OADProfile_Status_e oadStateMachine(oadEvent_e event, uint16 dataLen, uint8* pData);
static OADProfile_Status_e oadEventHandleStateIdle(oadEvent_e event, uint16 dataLen, uint8* pData);
static OADProfile_Status_e oadEventHandleStateConfig(oadEvent_e event, uint16 dataLen, uint8* pData);
static OADProfile_Status_e oadEventHandleStateDownload(oadEvent_e event, uint16 dataLen, uint8* pData);
static OADProfile_Status_e oadEventHandleStateComplete(oadEvent_e event, uint16 dataLen, uint8* pData);

static OADProfile_Status_e oadSendGenericExtCtrlRsp(uint8 cmdID ,uint8 stat);
static OADProfile_Status_e oadSendBlockSizeRsp(oadProtocolOPCode_e cmdID ,uint16 oadBlkSz);
static OADProfile_Status_e oadSendNextBlockReq(uint32 blkNum, uint8 status);
static OADProfile_Status_e oadSendswVersionRsp();

static void oadResetState(void);
static OADProfile_Status_e oadImgIdentifyWrite(uint16 len, uint8 *pValue);
static OADProfile_Status_e oadImgBlockWrite(uint8 len, uint8 *pValue);

static void oadChangeMachineState(oadState_e next_state);
static OADProfile_Status_e oadSetGlobalActiveConnHandle(uint16 connhandle);

#ifdef FREERTOS
static void oadInactivityTimeout(uint32_t param);
#else
static void oadInactivityTimeout(UArg param);
#endif //FREERTOS
#endif //OAD_APP_ONCHIP
static OADProfile_Status_e oadResetDevice(uint16 connHandle);
void oadGAPConnEventHandler(uint32 event, BLEAppUtil_msgHdr_t *pMsgData);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OADProfile_start
 *
 * @brief   Initializes the OAD profile by configure oad parameters
 *          and call to add service that registering GATT attributes
 *          with the GATT server. Only call this function once.
 *
 * @param   pOADAppCB - Callback function from profile to application
 *
 * @return  SUCCESS or INVALIDPARAMETER or bleMemAllocError
 */
bStatus_t OADProfile_start(OADProfile_AppCallback_t pOADAppCB)
{
    bStatus_t status = SUCCESS;

    oadProfileCB = pOADAppCB;
#ifdef OAD_APP_ONCHIP
    status = OadReset_AddService(&oadInvokeFromFWContextOADReset);
#else //OAD_APP_OFFCHIP || OAD_PERSISTENT
    status = OAD_AddService(&oadInvokeFromFWContextOAD);
    if(status != SUCCESS)
    {
        return(status);
    }
    if(NULL == pOADModuleGlobalData)
    {
        pOADModuleGlobalData = (oadModuleGlobalData_t *)ICall_malloc(sizeof(oadModuleGlobalData_t));
        if(NULL == pOADModuleGlobalData)
        {
           return(bleMemAllocError);
        }
    }

    pOADModuleGlobalData->state            = OAD_IDLE;
    pOADModuleGlobalData->publicState      = OAD_PROFILE_NOT_STARTED;
    pOADModuleGlobalData->blkSize          = OAD_DEFAULT_BLOCK_SIZE;
    pOADModuleGlobalData->imgBytesPerBlock = OAD_DEFAULT_BLOCK_SIZE - OAD_BLK_NUM_HDR_SZ;
    pOADModuleGlobalData->candidateImgLen  = 0xFFFFFFFF;
    pOADModuleGlobalData->totalBlocks      = 0xFFFFFFFF;
    pOADModuleGlobalData->lastBlockSize    = 0;
    pOADModuleGlobalData->currentBlkNum    = 0;
    pOADModuleGlobalData->activeConnHandle = LINKDB_CONNHANDLE_INVALID;
    pOADModuleGlobalData->blkReqActive     = true;
    pOADModuleGlobalData->imgIDRetries     = OAD_IMG_ID_RETRIES;
    pOADModuleGlobalData->stateTimeout     = OAD_DEFAULT_INACTIVITY_TIME;

    /*
     * The following API call belongs to the previous implementation, it may not be needed anymore
     * Set default values for Data Length Extension
     * Set initial values to maximum, RX is set to max. by default(251 octets, 2120us)
     * Some brand smartphone is essentially needing 251/2120, so we set them here.
     * This API is documented in hci.h
    */
    HCI_LE_WriteSuggestedDefaultDataLenCmd(APP_SUGGESTED_PDU_SIZE, APP_SUGGESTED_TX_TIME);

    // Create OAD activity timer
    Util_constructClock(&oadActivityClk,
                        oadInactivityTimeout,
                        pOADModuleGlobalData->stateTimeout, 0, false,0);

#endif //OAD_APP_ONCHIP
    return (status);
}

/*********************************************************************
 * LOCAL FUNCTIONS
 */
/*
 * Event handlers and state machine functions
 */
#ifdef OAD_APP_ONCHIP
/*********************************************************************
 * @fn      oadInvokeFromFWContextOADReset
 *
 * @brief   Process a write request to the OAD from service module
 *
 * @param   pData - Contain reset request
 *
 * @return  None.
 */
void oadInvokeFromFWContextOADReset(char *pData)
{

    oadResetWrite_t oadResetWriteEvt = *(oadResetWrite_t *)pData;

    if(OAD_RESET_CMD_START_OAD == oadResetWriteEvt.cmd)
    {
        OADProfile_AppCommand_e permit = oadProfileCB(OAD_PROFILE_MSG_REVOKE_IMG_HDR);

        if(OAD_PROFILE_PROCEED == permit)
        {
            SwUpdate_RevokeImage(INT_PRIMARY_SLOT);

            oadResetDevice(oadResetWriteEvt.connHandle);
        }
    }
}
#else //OAD_APP_ONCHIP || OAD_PERSISTENT
/*********************************************************************
 * @fn      oadInvokeFromFWContextOAD
 *
 * @brief   Process a write request to the OAD from service module
 *
 * @param   pData - Contain write req date
 *
 * @return  None.
 */
void oadInvokeFromFWContextOAD(char *pData)
{
    oadSrvWriteReq_t pOADSrvWriteReq = *(oadSrvWriteReq_t *)pData;

    oadSetGlobalActiveConnHandle(pOADSrvWriteReq.connHandle);

    switch (pOADSrvWriteReq.serviceChar)
    {
        case OAD_SRV_IDENTIFY_REQ:
            oadStateMachine(OAD_EVT_IMG_IDENTIFY_REQ,
                            pOADSrvWriteReq.len, pOADSrvWriteReq.pData);
            break;
        case OAD_SRV_BLOCK_REQ:
            oadStateMachine(OAD_EVT_BLOCK_REQ,
                            pOADSrvWriteReq.len, pOADSrvWriteReq.pData);
            break;
        //for OAD_SRV_CTRL_CMD there are opcodes that depend on the state and those that don't,
        //therefore those that depend on the state we send to the
        //oadStateMachine() and those that don't we immediately return an answer.
        case OAD_SRV_CTRL_CMD:
        {
            uint8 opcode = EXT_CTRL_OP_CODE(pOADSrvWriteReq.pData);
            switch(opcode)
            {
                case OAD_EVT_START_OAD:
                case OAD_EVT_ENABLE_IMG:
                case OAD_EVT_CANCEL_OAD:
                {
                    oadStateMachine((oadEvent_e)opcode, pOADSrvWriteReq.len, pOADSrvWriteReq.pData);
                    break;
                }
                case OAD_REQ_GET_BLK_SZ:
                {
                    oadSendBlockSizeRsp(OAD_REQ_GET_BLK_SZ ,pOADModuleGlobalData->blkSize);
                    break;
                }
                case OAD_REQ_DISABLE_BLK_NOTIF:
                {
                    // Stop sending block request notifications
                    pOADModuleGlobalData->blkReqActive = false;
                    oadSendGenericExtCtrlRsp(OAD_REQ_DISABLE_BLK_NOTIF,OAD_PROFILE_SUCCESS);
                    break;
                }
                case OAD_REQ_GET_OAD_STAT:
                {
                    oadSendGenericExtCtrlRsp(OAD_REQ_GET_OAD_STAT,pOADModuleGlobalData->publicState);
                    break;
                }
                case OAD_REQ_GET_SW_VER:
                {
                    oadSendswVersionRsp();
                    break;
                }
                case OAD_REQ_ERASE_BONDS:
                {
                    // Control commands for erasing bonds
                    GAPBondMgr_SetParameter(GAPBOND_ERASE_ALLBONDS, 0, NULL);
                    oadSendGenericExtCtrlRsp(OAD_REQ_DISABLE_BLK_NOTIF,OAD_PROFILE_SUCCESS);
                    break;
                }
                default:
                {
                    oadSendGenericExtCtrlRsp(opcode,OAD_RSP_CMD_NOT_SUPPORTED);
                    break;
                }
            }
            break;
        }
    }
}

/*********************************************************************
 * @fn      oadInvokeFromFWContextTimeout
 *
 * @brief   Process Timeout event
 *
 * @param   pData - NULL
 *
 * @return  None.
 */
void oadInvokeFromFWContextTimeout(char *pData)
{
    oadStateMachine(OAD_EVT_TIMEOUT,0,NULL);
}

/*********************************************************************
 * @fn      oadStateMachine
 *
 * @brief   Process the OAD event depend on current state.
 *
 * @param   event - The new event
 * @param   dataLen - length of pData
 * @param   pData - The information accompanying the event
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadStateMachine(oadEvent_e event, uint16 dataLen, uint8* pData)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    // Timeout is state independent. Always results in OAD state reset
    if(OAD_EVT_TIMEOUT == event)
    {
        oadResetState();
    }
    else
    {
        //Switching on the current state of the OAD process.
        //There is no need to check that pOADModuleGlobalData does exist,
        //details are found next to the declaration of the variable
        switch (pOADModuleGlobalData->state)
        {
            case OAD_IDLE:
                status = oadEventHandleStateIdle(event, dataLen, pData);
                break;

            case OAD_CONFIG:
                status = oadEventHandleStateConfig(event, dataLen, pData);
                break;

            case OAD_DOWNLOAD:
                status = oadEventHandleStateDownload(event, dataLen, pData);
                break;

            case OAD_COMPLETE:
                status = oadEventHandleStateComplete(event, dataLen, pData);
                break;

            default:
                //It is not possible to reach this case.
                status = OAD_PROFILE_ERROR;
        }
    }

    // Free buffer.
    if(pData != NULL)
    {
        ICall_free(pData);
    }

    return (status);
}

/*********************************************************************
 * @fn      oadEventHandleStateIdle
 *
 * @brief   Event hander for idle state The only possible event that
 *          can be received is OAD_EVT_IMG_IDENTIFY_REQ , any other
 *          event will result in a reply message with the name of the
 *          event and OAD_PROFILE_NOT_STARTED status
 *
 * @param   event - The new event
 * @param   dataLen - length of pData
 * @param   pData - The information accompanying the event
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadEventHandleStateIdle(oadEvent_e event, uint16 dataLen, uint8* pData)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;
    OADProfile_Status_e statusid = OAD_PROFILE_SUCCESS;

    //Switching depending on input event
    switch(event)
    {
        case OAD_EVT_IMG_IDENTIFY_REQ:
        {
            status = (OADProfile_Status_e)SwUpdate_Open(SW_UPDATE_SOURCE_OAD);
            if(OAD_PROFILE_SUCCESS == status)
            {
                status = oadImgIdentifyWrite(dataLen,pData);
                // Advance the state based on status
                if(OAD_PROFILE_SUCCESS == status)
                {
                    if(NULL != oadProfileCB)
                    {
                        //inform app about new image
                        oadProfileCB(OAD_PROFILE_MSG_NEW_IMG_IDENDIFY);
                    }
                    oadChangeMachineState(OAD_CONFIG);
                }
                // Send a response to the ImgID command
                statusid = status;
                status = (OADProfile_Status_e)OADService_setParameter(OAD_SRV_IDENTIFY_REQ,OAD_IMAGE_ID_RSP_LEN,(void *)&statusid);
            }
            break;
        }
        case OAD_EVT_BLOCK_REQ:
        case OAD_EVT_TIMEOUT:
        case OAD_EVT_START_OAD:
        case OAD_EVT_ENABLE_IMG:
        case OAD_EVT_CANCEL_OAD:
        {
            // This opcode doesn't exist/doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_NOT_STARTED);
            // An error has occurred, reset and go back to idle
            oadResetState();
            break;
        }
        default:
            //It is not possible to reach this case.
            status = OAD_PROFILE_ERROR;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadEventHandleStateConfig
 *
 * @brief   Event hander for config state The only possible events that
 *          can be received is OAD_EVT_START_OAD or OAD_EVT_CANCEL_OAD,
 *          any other event will result in a reply message with the
 *          name of the event and OAD_PROFILE_ALREADY_STARTED status
 *
 * @param   event - The new event
 * @param   dataLen - length of pData
 * @param   pData - The information accompanying the event
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadEventHandleStateConfig(oadEvent_e event, uint16 dataLen, uint8* pData)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    //Switching depending on input event
    switch(event)
    {
        case OAD_EVT_START_OAD:
        {
            //Get app permission for download new image
            if((NULL != oadProfileCB) &&
               (OAD_PROFILE_PROCEED != oadProfileCB(OAD_PROFILE_MSG_START_DOWNLOAD)))
            {
                    status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_APP_STOP_PROCESS);
            }
            else
            {
                oadChangeMachineState(OAD_DOWNLOAD);
                // Send the first block request to kick off the OAD
                status = oadSendNextBlockReq(pOADModuleGlobalData->currentBlkNum, OAD_PROFILE_SUCCESS);
            }
            break;
        }
        case OAD_EVT_BLOCK_REQ:
        case OAD_EVT_TIMEOUT:
        case OAD_EVT_IMG_IDENTIFY_REQ:
        case OAD_EVT_ENABLE_IMG:
        {
            // This opcode doesn't doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_ALREADY_STARTED);
            break;
        }
        case OAD_EVT_CANCEL_OAD:
        {
            // reset and go back to idle
            oadResetState();
            // This opcode doesn't exist/doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_SUCCESS);
            break;
        }
        default:
            //It is not possible to reach this case.
            status = OAD_PROFILE_ERROR;
    }

    return (status);
}
/*********************************************************************
 * @fn      oadEventHandleStateDownload
 *
 * @brief   Event hander for download state The only possible events that
 *          can be received is OAD_EVT_BLOCK_REQ or OAD_EVT_CANCEL_OAD,
 *          any other event will result in a reply message with the
 *          name of the event and OAD_PROFILE_ALREADY_STARTED status
 *
 * @param   event - The new event
 * @param   dataLen - length of pData
 * @param   pData - The information accompanying the event
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadEventHandleStateDownload(oadEvent_e event, uint16 dataLen, uint8* pData)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    //Switching depending on input event
    switch(event)
    {
        case OAD_EVT_BLOCK_REQ:
        {
            status = oadImgBlockWrite(dataLen,pData);
            if(OAD_PROFILE_SUCCESS == status)
            {
                // If the block write was successful but the process
                // is not complete then stay in download state
                oadChangeMachineState(OAD_DOWNLOAD);
            }
            else if(OAD_PROFILE_DL_COMPLETE == status)
            {
                if(NULL != oadProfileCB)
                {
                    //inform app about download complete
                    oadProfileCB(OAD_PROFILE_MSG_FINISH_DOWNLOAD);
                }
                // Image download is complete, now wait for enable command
                oadChangeMachineState(OAD_COMPLETE);
            }
            else
            {
                // An error has occured, reset and go back to idle
                oadChangeMachineState(OAD_IDLE);
                oadResetState();
            }

            // Request the next block
            status = oadSendNextBlockReq(pOADModuleGlobalData->currentBlkNum, status);
            break;
        }
        case OAD_EVT_IMG_IDENTIFY_REQ:
        case OAD_EVT_TIMEOUT:
        case OAD_EVT_START_OAD:
        case OAD_EVT_ENABLE_IMG:
        {
            // This opcode doesn't doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_ALREADY_STARTED);
            break;
        }
        case OAD_EVT_CANCEL_OAD:
        {
            // reset and go back to idle
            oadResetState();
            // This opcode doesn't exist/doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_SUCCESS);
            break;
        }
        default:
            //It is not possible to reach this case.
            status = OAD_PROFILE_ERROR;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadEventHandleStateComplete
 *
 * @brief   Event hander for complete state The only possible events that
 *          can be received is OAD_EVT_ENABLE_IMG or OAD_EVT_CANCEL_OAD,
 *          any other event will result in a reply message with the
 *          name of the event and OAD_PROFILE_ALREADY_STARTED status
 *
 * @param   event - The new event
 * @param   dataLen - length of pData
 * @param   pData - The information accompanying the event
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadEventHandleStateComplete(oadEvent_e event, uint16 dataLen, uint8* pData)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    switch(event)
    {
        case OAD_EVT_ENABLE_IMG:
        {
            oadSendGenericExtCtrlRsp(OAD_EVT_ENABLE_IMG ,OAD_PROFILE_SUCCESS);
            //sw_update finish
            oadResetDevice(pOADModuleGlobalData->activeConnHandle);
            break;
        }
        case OAD_EVT_BLOCK_REQ:
        case OAD_EVT_TIMEOUT:
        case OAD_EVT_START_OAD:
        case OAD_EVT_IMG_IDENTIFY_REQ:
        {
            // This opcode doesn't doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_ALREADY_STARTED);
            break;
        }
        case OAD_EVT_CANCEL_OAD:
        {
            // reset and go back to idle
            oadResetState();
            // This opcode doesn't exist/doesn't match the current state
            status = oadSendGenericExtCtrlRsp(event ,OAD_PROFILE_SUCCESS);
            break;
        }
        default:
            //It is not possible to reach this case.
            status = OAD_PROFILE_ERROR;
    }

    return (status);
}

/*********************************************************************
 * Send responds functions
 */
/*********************************************************************
 * @fn      oadSendGenericExtCtrlRsp
 *
 * @brief  Function for build and send generic respond
 *
 * @param   cmdID command ID to which an answer is responded
 * @param   stat The respond value
 *
 * @return  OAD_PROFILE_SUCCESS or INVALIDPARAMETER
 */
static OADProfile_Status_e oadSendGenericExtCtrlRsp(uint8 cmdID ,uint8 stat)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;
    genericExtCtrlRsp_t rsp = {cmdID,stat};

    status = (OADProfile_Status_e)OADService_setParameter(OAD_SRV_CTRL_CMD,sizeof(genericExtCtrlRsp_t),(void *)&rsp);

    return (status);
}

/*********************************************************************
 * @fn      oadSendBlockSizeRsp
 *
 * @brief   Function for build and send respond for block size request
 *
 * @param   cmdID command ID to which an answer is responded
 * @param   oadBlkSz Block size
 *
 * @return  OAD_PROFILE_SUCCESS or INVALIDPARAMETER
 */
static OADProfile_Status_e oadSendBlockSizeRsp(oadProtocolOPCode_e cmdID ,uint16 oadBlkSz)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;
    blockSizeRspPld_t rsp = {cmdID,oadBlkSz};

    status = (OADProfile_Status_e)OADService_setParameter(OAD_SRV_CTRL_CMD,sizeof(blockSizeRspPld_t),(void *)&rsp);

    return (status);
}

/*********************************************************************
 * @fn      oadSendNextBlockReq
 *
 * @brief   Process the Request for next image block.
 *
 * @param   blkNum - block number to request from OAD peer.
 * @param   stat - status of prev block
 * @return  OAD_PROFILE_SUCCESS or INVALIDPARAMETER
 */
static OADProfile_Status_e oadSendNextBlockReq(uint32 blkNum, uint8 stat)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    //There is no need to check that pOADModuleGlobalData does exist,
    //details are found next to the declaration of the variable
    if(pOADModuleGlobalData->blkReqActive)
    {
        blockReqPld_t rsp = {OAD_RSP_BLK_RSP_NOTIF,stat,blkNum};

        status = (OADProfile_Status_e)OADService_setParameter(OAD_SRV_CTRL_CMD,sizeof(blockReqPld_t),(void *)&rsp);
    }

    return (status);
}

/*********************************************************************
 * @fn      oadSendswVersionRsp
 *
 * @brief   Respond for software version request.
 *
 * @return  OAD_PROFILE_SUCCESS or INVALIDPARAMETER
 */
static OADProfile_Status_e oadSendswVersionRsp()
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;
    uint8 *pCmdRsp = ICall_malloc(sizeof(swVersionPld_t));

    swVersionPld_t *rsp = (swVersionPld_t *)pCmdRsp;
    struct image_version * img_ver = (struct image_version *)SwUpdate_GetSWVersion(APP_HDR_ADDR);
    // Populate the software version field
    uint8 swVerCombined[MCUBOOT_SW_VER_LEN] = {img_ver->iv_major,
                                               img_ver->iv_minor,
                                               HI_UINT16(img_ver->iv_revision),
                                               LO_UINT16(img_ver->iv_revision),
                                               HI_UINT16(img_ver->iv_build_num),
                                               LO_UINT16(img_ver->iv_build_num),
                                              };
    // Pack up the payload
    rsp->cmdID = OAD_REQ_GET_SW_VER;
    // Copy combined version string into the response payload
    memcpy(rsp->swVer, swVerCombined, MCUBOOT_SW_VER_LEN);
    //swVersionPld_t rsp = {OAD_REQ_GET_SW_VER,swVerCombined};
    if(pCmdRsp != NULL)
    {
        // Send out the populated command structure
        status = (OADProfile_Status_e)OADService_setParameter(OAD_SRV_CTRL_CMD,sizeof(swVersionPld_t),(void *)rsp);
        ICall_free(pCmdRsp);
    }

    return (status);
}

/*********************************************************************
 * Action functions
 */

/*********************************************************************
 * @fn      oadResetState
 *
 * @brief   Reset OAD variables
 *
 * @param   none
 *
 * @return  none
 */
 static void oadResetState(void)
 {
    // Reset the state variable
    oadChangeMachineState(OAD_IDLE);

    //There is no need to check that pOADModuleGlobalData does exist,
    //details are found next to the declaration of the variable
    pOADModuleGlobalData->blkSize          = OAD_DEFAULT_BLOCK_SIZE;
    pOADModuleGlobalData->imgBytesPerBlock = OAD_DEFAULT_BLOCK_SIZE - OAD_BLK_NUM_HDR_SZ;
    pOADModuleGlobalData->candidateImgLen  = 0xFFFFFFFF;
    pOADModuleGlobalData->totalBlocks      = 0xFFFFFFFF;
    pOADModuleGlobalData->lastBlockSize    = 0;
    pOADModuleGlobalData->currentBlkNum    = 0;
    pOADModuleGlobalData->activeConnHandle = LINKDB_CONNHANDLE_INVALID;
    pOADModuleGlobalData->blkReqActive     = true;
    pOADModuleGlobalData->imgIDRetries     = OAD_IMG_ID_RETRIES;
    pOADModuleGlobalData->stateTimeout     = OAD_DEFAULT_INACTIVITY_TIME;

    SwUpdate_Close();
    // Stop the inactivity timer if running
    if(Util_isActive(&oadActivityClk))
    {
        Util_stopClock(&oadActivityClk);
    }
 }

/*********************************************************************
 * @fn      oadImgIdentifyWrite
 *
 * @brief   Process the Image Identify Write.  Determined if the image
 *          header identified here should or should not be downloaded by
 *          this application.
 *
 * @param   len     - length of pValue
 * @param   pValue  - pointer to image header data
 *
 * @return  none
 */
static OADProfile_Status_e oadImgIdentifyWrite(uint16 len, uint8 *pValue)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;
    int swUpdateStatus = 0;

    // Validate the Header
    swUpdateStatus = SwUpdate_CheckImageHeader(pValue);
    if(swUpdateStatus < 0)
    {
        status = OAD_PROFILE_INVALID_FILE;
        pOADModuleGlobalData->imgIDRetries--;

        //If the number of attempts for sending a new header is over.
        if(0 == pOADModuleGlobalData->imgIDRetries)
        {
            status = OAD_PROFILE_IMG_ID_TIMEOUT;
        }
    }
    else
    {
        pOADModuleGlobalData->candidateImgLen = swUpdateStatus;
    }
    // If image ID is accepted, set variables
    if(OAD_PROFILE_SUCCESS == status)
    {
        // Calculate total number of OAD blocks, round up if needed
        pOADModuleGlobalData->totalBlocks = (pOADModuleGlobalData->candidateImgLen) /
                                            (pOADModuleGlobalData->imgBytesPerBlock);

        // If there is a remainder after division, store it and round up
        pOADModuleGlobalData->lastBlockSize = (pOADModuleGlobalData->candidateImgLen) %
                                              (pOADModuleGlobalData->imgBytesPerBlock);
        if( 0 != (pOADModuleGlobalData->lastBlockSize))
        {
            pOADModuleGlobalData->totalBlocks += 1;
        }
        // Image has been accepted, start the inactivity timer
        Util_startClock(&oadActivityClk);
    }

    return (status);
}

/*********************************************************************
 * @fn      oadImgBlockWrite
 *
 * @brief   Process the Image Block Write.
 *
 * @param   len     - length of pValue
 * @param   pValue - pointer to data to be written
 *
 * @return  none
 */
static OADProfile_Status_e oadImgBlockWrite(uint8 len, uint8 *pValue)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;
    uint16 expectedBlkSz;

    if(Util_isActive(&oadActivityClk))
    {
        // Any over the air interaction with the OAD profile will reset the clk
        Util_rescheduleClock(&oadActivityClk, pOADModuleGlobalData->stateTimeout);
        Util_startClock(&oadActivityClk);
    }

    uint32 blkNum = BUILD_UINT32(pValue[0], pValue[1],
                                            pValue[2], pValue[3]);

    // Determine the expected block size, note the last block may be a partial
    if(blkNum == (pOADModuleGlobalData->totalBlocks- 1))
    {
        if(pOADModuleGlobalData->lastBlockSize != 0)
        {
            expectedBlkSz = (pOADModuleGlobalData->lastBlockSize) + OAD_BLK_NUM_HDR_SZ;
        }
        else
        {
            expectedBlkSz = pOADModuleGlobalData->blkSize;
        }
    }
    else
    {
        expectedBlkSz = pOADModuleGlobalData->blkSize;
    }

    // Check that this is the expected block number, and the block size is right
    if ((pOADModuleGlobalData->currentBlkNum == blkNum) && (len == expectedBlkSz))
    {
        // Calculate address to write as (start of OAD range) + (offset into range)
        uint32 blkStartAddr = (pOADModuleGlobalData->imgBytesPerBlock)*blkNum;
        status = (OADProfile_Status_e)SwUpdate_WriteBlock(blkStartAddr,(len - OAD_BLK_NUM_HDR_SZ),(pValue+OAD_BLK_NUM_HDR_SZ));

        // Increment received block count.
        pOADModuleGlobalData->currentBlkNum++;

        // Check if the OAD Image is complete.
        if (pOADModuleGlobalData->currentBlkNum == pOADModuleGlobalData->totalBlocks)
        {
            status = OAD_PROFILE_DL_COMPLETE;
        }
    }
    else
    {
        // Overflow, abort OAD
        pOADModuleGlobalData->currentBlkNum = 0;
        status = OAD_PROFILE_BUFFER_OFL;
    }

    return (status);
}

/*********************************************************************
 * @fn      oadChangeMachineState
 *
 * @brief   Process change of the state machine private and public
 *
 * @param   next_state - state to be change
 *
 * @return  none
 */
static void oadChangeMachineState(oadState_e next_state)
{
    //There is no need to check that pOADModuleGlobalData does exist,
    //details are found next to the declaration of the variable
    pOADModuleGlobalData->state = next_state;

    //Store the publice machine state, which the peer can request
    if(OAD_IDLE == next_state)
    {
        pOADModuleGlobalData->publicState = OAD_PROFILE_NOT_STARTED;
    }
    else
    {
        pOADModuleGlobalData->publicState = OAD_PROFILE_ALREADY_STARTED;
    }
}

/*********************************************************************
 * @fn      oadSetGlobalActiveConnHandle
 *
 * @brief   Process change of the active connhande in this module and also
 *          inform service module, We assume that a connhandle exists
 *          because this function is called from a callback that arrived
 *          as a result of a write performed by that connhandle.
 *
 * @param   connhandle - the request connhandle to be change
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadSetGlobalActiveConnHandle(uint16 connhandle)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    // reset connhandle
    if(LINKDB_CONNHANDLE_INVALID == connhandle)
    {
        pOADModuleGlobalData->activeConnHandle = connhandle;
        OADService_setActiveConnHandle(connhandle);
    }
    else
    {
        // Setup the active OAD connection handle
        if(LINKDB_CONNHANDLE_INVALID == pOADModuleGlobalData->activeConnHandle)
        {
            pOADModuleGlobalData->activeConnHandle = connhandle;
            OADService_setActiveConnHandle(connhandle);
        }
        //If there is interrupt from other peer send answer that we already started
        else if(connhandle != pOADModuleGlobalData->activeConnHandle)
        {
            uint16 prev_connhandle = pOADModuleGlobalData->activeConnHandle;
            OADService_setActiveConnHandle(connhandle);
            // Send notification about the action
            status = oadSendGenericExtCtrlRsp(OAD_EVT_CANCEL_OAD ,OAD_PROFILE_ALREADY_STARTED);
            OADService_setActiveConnHandle(prev_connhandle);
        }
    }

    return (status);
}

/*********************************************************************
 * @fn      oadInactivityTimeout
 *
 * @brief   This function is triggered when the OAD clock expires due
 *          to inactivity
 *
 */
#ifdef FREERTOS
static void oadInactivityTimeout(uint32_t param)
#else
static void oadInactivityTimeout(UArg param)
#endif //FREERTOS
{
    BLEAppUtil_invokeFunction(oadInvokeFromFWContextTimeout, NULL);
}
#endif //OAD_APP_ONCHIP

/* We cannot reboot the device immediately after receiving
* the enable command, we must allow the stack enough time
* to process and respond, The following functions are for
 implementing the reset after disconnecting from the peer
*/
BLEAppUtil_EventHandler_t oadConnHandler =
{
    .handlerType    = BLEAPPUTIL_GAP_CONN_TYPE,
    .pEventHandler  = oadGAPConnEventHandler,
    .eventMask      = BLEAPPUTIL_LINK_TERMINATED_EVENT,
};

/*********************************************************************
 * @fn      oadResetDevice
 *
 * @brief   Process register for connection event handler and
 *          terminate the link.
 *
 * @param   connhandle - connhandle to terminate
 *
 * @return  OADProfile_Status_e
 */
static OADProfile_Status_e oadResetDevice(uint16 connHandle)
{
    OADProfile_Status_e status = OAD_PROFILE_SUCCESS;

    status = (OADProfile_Status_e)BLEAppUtil_registerEventHandler(&oadConnHandler);
    if(SUCCESS == status)
    {
        status = (OADProfile_Status_e)GAP_TerminateLinkReq(connHandle, HCI_DISCONNECT_REMOTE_USER_TERM);
    }

    return(status);
}

/*********************************************************************
 * @fn      oadGAPConnEventHandler
 *
 * @brief   The purpose of this function is to wait to
 *          BLEAPPUTIL_LINK_TERMINATED_EVENT and execute reset
 *
 * @param   event - message event.
 * @param   pMsgData - pointer to message data.
 *
 * @return  none
 */
void oadGAPConnEventHandler(uint32 event, BLEAppUtil_msgHdr_t *pMsgData)
{
    if(BLEAPPUTIL_LINK_TERMINATED_EVENT == event)
    {
        if((NULL == oadProfileCB) ||
           (OAD_PROFILE_PROCEED == oadProfileCB(OAD_PROFILE_MSG_RESET_REQ)))
        {
                SystemReset();
        }
    }
}

/*********************************************************************
*********************************************************************/
