/******************************************************************************

 @file oad_client.c

 @brief OAD Client

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
#ifndef OAD_CLIENT_H
#define OAD_CLIENT_H

#ifdef FEATURE_NATIVE_OAD
/******************************************************************************
 Includes
 *****************************************************************************/
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <ti/drivers/dpl/ClockP.h>
#include <semaphore.h>

#include "oad_client.h"
#include "ti_154stack_oad_config.h"
#include "native_oad/oad_protocol.h"
#include "native_oad/oad_storage.h"
#include <oad_image_header_app.h>

#include "mac_util.h"
#include "jdllc.h"
#include "ssf.h"
#include "sensor.h"
#include "util_timer.h"

#ifndef CUI_DISABLE
#include "cui.h"
#endif

#if defined(FEATURE_TOAD)
#include <crc32.h>
#include "turbo_oad.h"
#endif

#include <flash_interface.h>
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/sys_ctrl.h)
#include DeviceFamily_constructPath(driverlib/flash.h)

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

#if (CONFIG_PHY_ID == APIMAC_50KBPS_915MHZ_PHY_1) || \
    (CONFIG_PHY_ID == APIMAC_50KBPS_868MHZ_PHY_3) || \
    (CONFIG_PHY_ID == APIMAC_50KBPS_433MHZ_PHY_128)
    #define SYMBOL_DURATION         (SYMBOL_DURATION_50_kbps)  //us

#elif (CONFIG_PHY_ID == APIMAC_200KBPS_915MHZ_PHY_132) || \
      (CONFIG_PHY_ID == APIMAC_200KBPS_868MHZ_PHY_133)
    #define SYMBOL_DURATION         (SYMBOL_DURATION_200_kbps) //us

#elif (CONFIG_PHY_ID == APIMAC_5KBPS_915MHZ_PHY_129) || \
      (CONFIG_PHY_ID == APIMAC_5KBPS_433MHZ_PHY_130) || \
      (CONFIG_PHY_ID == APIMAC_5KBPS_868MHZ_PHY_131)
    #define SYMBOL_DURATION         (SYMBOL_DURATION_LRM)      //us

#elif (CONFIG_PHY_ID == APIMAC_250KBPS_IEEE_PHY_0)  // 2.4g
    #define SYMBOL_DURATION         (SYMBOL_DURATION_250_kbps)  //us
#else
    #define SYMBOL_DURATION         (SYMBOL_DURATION_50_kbps)  //us
#endif

#define BEACON_INTERVAL             ((((0x01) << (CONFIG_MAC_BEACON_ORDER)) * \
                                      (SYMBOL_DURATION) * (BASE_SUPER_FRAME_DURATION)) / (1000)) // ms

#if (CONFIG_MAC_SUPERFRAME_ORDER == 15)
#define OAD_MAX_TIMEOUTS            3
#define OAD_MAX_RETRIES             3
#define OAD_BLOCK_AUTO_RESUME_DELAY 5000
#else
#define OAD_MAX_TIMEOUTS            ((uint8_t) ((BEACON_INTERVAL/OAD_BLOCK_REQ_RATE) + 1))
#define OAD_MAX_RETRIES             3
#define OAD_BLOCK_AUTO_RESUME_DELAY BEACON_INTERVAL
#endif

/* Re-define the OAD block request rate to match the beacon interval
 * if implicit polling is used
 */
#if !defined(OAD_EXPLICIT_POLLING)
#undef OAD_BLOCK_REQ_RATE
#undef OAD_BLOCK_REQ_POLL_DELAY

#define OAD_BLOCK_REQ_RATE          ((BEACON_INTERVAL) - 100)
#define OAD_BLOCK_REQ_POLL_DELAY    ((BEACON_INTERVAL) - 400)
#endif

/*!
 OAD block variables.
 */
static uint16_t oadBNumBlocks = 0;
static uint16_t oadBlock = 0;
static ApiMac_sAddr_t oadServerAddr = {0};
static bool oadInProgress = false;
static uint8_t oadImgId = 0;
static uint8_t oadRetries = 0;
static uint8_t oadTimeouts = 0;

uint8_t oadImgIdPld[OADProtocol_IMAGE_ID_LEN];

OADClient_Params_t oadClientParams;

#ifndef CUI_DISABLE
static uint32_t oadStatusLine;
#endif

static ClockP_Struct oadClkStruct;
static ClockP_Handle oadClkHandle;

static uint32_t defaultPollInterval;

#if OAD_EXPLICIT_POLLING
/* Stores the current setting of auto Request Pib attribute
 * before it gets modified by the OAD application
 */
static bool currAutoReq = 0;
#endif

/* OAD pause and resume variables */
#if defined(OAD_ONCHIP)
static uint32_t flashPageSize = INTFLASH_PAGE_SIZE;
#else
static uint32_t flashPageSize = EFL_PAGE_SIZE;
#endif

/* NV OAD-block writing control variables */
static uint32_t flashPageBytesDownloaded = 0;

/* Turbo OAD control variables */
static bool isToadImage = false;
static bool isOADPaused = false;

/******************************************************************************
 Local function prototypes
 *****************************************************************************/

static void oadClockInitialize(void);
static void oadClockCallback(uintptr_t a0);
static void oadClockSet(uint32_t oadTimeout);
#ifndef CUI_DISABLE
static void displayOadBlockUpdate(uint16_t oadBlock, uint16_t oadBNumBlocks, uint8_t retries);
static void displayOadStatusUpdate(OADStorage_Status_t status);
#endif
static void oadFwVersionReqCb(void* pSrcAddr);
static void oadImgIdentifyReqCb(void* pSrcAddr, uint8_t imgId, uint8_t *imgMetaData);
static void oadBlockRspCb(void* pSrcAddr, uint8_t imgId, uint16_t blockNum, uint8_t *blkData);
#ifdef OAD_ONCHIP
#if defined(OAD_IMG_B)
static bool OAD_evenBitCount(uint32_t value);
#endif//OAD_IMG_B
static void oadResetReqCb(void* pSrcAddr);
#endif
void* oadRadioAccessAllocMsg(uint32_t msgLen);
static OADProtocol_Status_t oadRadioAccessPacketSend(void* pDstAddr, uint8_t *pMsg, uint32_t msgLen);

#if defined(FEATURE_TOAD)
static void toadWriteDeltaBlock(uint8_t* pData, uint32_t len);
#endif

/******************************************************************************
 Callback tables
 *****************************************************************************/

static OADProtocol_RadioAccessFxns_t  oadRadioAccessFxns =
    {
      oadRadioAccessAllocMsg,
      oadRadioAccessPacketSend
    };

static OADProtocol_MsgCBs_t oadMsgCallbacks =
    {
      /*! Incoming FW Req */
      oadFwVersionReqCb,
      /*! Incoming FW Version Rsp */
      NULL,
      /*! Incoming Image Identify Req */
      oadImgIdentifyReqCb,
      /*! Incoming Image Identify Rsp */
      NULL,
      /*! Incoming OAD Block Req */
      NULL,
      /*! Incoming OAD Block Rsp */
      oadBlockRspCb,
#ifdef OAD_ONCHIP
      /*! Incoming OAD Reset Req */
      oadResetReqCb,
      /*! Incoming OAD Reset Rsp */
      NULL
#endif
    };

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 Initialize this application.

 Public function defined in sensor.h
 */
void OADClient_open(OADClient_Params_t *params)
{
    OADProtocol_Params_t OADProtocol_params;

    memcpy(&oadClientParams, params, sizeof(OADClient_Params_t));

    OADProtocol_Params_init(&OADProtocol_params);
    OADProtocol_params.pRadioAccessFxns = &oadRadioAccessFxns;
    OADProtocol_params.pProtocolMsgCallbacks = &oadMsgCallbacks;

    OADProtocol_open(&OADProtocol_params);

#ifndef CUI_DISABLE
    CUI_statusLineResourceRequest(*(oadClientParams.pOadCuiHndl), "OAD Status", false, &oadStatusLine);
#endif

    oadClockInitialize();

#ifndef MCUBOOT
    // check if external flash memory available
    if(hasExternalFlash() == true)
    {
        // Create factory image if there isn't one
        if(!OADStorage_checkFactoryImage())
        {
            OADStorage_createFactoryImageBackup();
        }
    }
#endif
}

/*!
 Application task processing.

 Public function defined in sensor.h
 */
void OADClient_processEvent(uint16_t *pEvent)
{
    /* Is it time to send the next sensor data message? */
    if(*pEvent & SENSOR_OAD_TIMEOUT_EVT)
    {
        static int32_t lastBlock = -1;
        static bool oadComplete = false;

        if(oadComplete)
        {
            /* reset to BIM */
            SysCtrlSystemReset();

            /* Clear complete flag in case reset did not work */
            oadComplete = false;
        }

        if(oadInProgress)
        {
#if OAD_EXPLICIT_POLLING
            /* On successful beacon reception, auto request is set back to true.
             * Set auto request back to false and restore original value after OAD
             * has completed.
             */
            bool autoReq = false;
            ApiMac_mlmeGetReqBool(ApiMac_attribute_autoRequest, &autoReq);

            if (autoReq)
            {
                ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, false);
            }
#endif

            if(oadBlock == lastBlock && (!isToadImage || (isToadImage && !isOADPaused)))
            {
                if(oadTimeouts++ > OAD_MAX_TIMEOUTS)
                {
                    /*
                     * allow retries before aborting
                     */
                    if(oadRetries++ > OAD_MAX_RETRIES)
                    {
                        /*abort OAD with auto resume */
                        OADClient_abort(true);
                    }
                    else
                    {
                        /* reduce the lastBlock by 1 to force a re-request */
                        lastBlock -= 1;
                        oadTimeouts = 0;
                    }
                }
                else
                {
                    /*
                     * req timed out, set timer again
                     * */
                    oadClockSet(OAD_BLOCK_REQ_RATE);
                    Ssf_setPollClock(OAD_BLOCK_REQ_POLL_DELAY);
                }
            }

            if( (oadBlock < oadBNumBlocks) && (oadBlock != lastBlock) )
            {
                if(oadBlock == 0)
                {
                    /* OAD cannot resume if block number in NV is 0. Store
                     * the value in NV as 1, and on resume, this value will
                     * be rounded down to resume OAD at block 0.
                     */
                    uint16_t resumeBlock = 1;

                    /* Only store image header and image ID on first block */
                    Ssf_oadInfoUpdate(&resumeBlock, oadImgIdPld, &oadImgId, &oadServerAddr);
                }
                /* Only write the last received block number when the block
                 * crosses a flash page. If OAD aborts, it will resume on the
                 * page-boundry where the last block was downloaded.
                 */
                else if(flashPageBytesDownloaded > flashPageSize)
                {
                    Ssf_oadInfoUpdate((uint16_t*) &lastBlock, 0, 0, 0);
                    flashPageBytesDownloaded -= flashPageSize;
                }
                lastBlock = oadBlock;
                flashPageBytesDownloaded += OAD_BLOCK_SIZE;

                /* Send Block request specifying number of blocks per multi
                 * blocks. The multi block hard coded to 1 here, but it is
                 * intended that an advanced user would use information from
                 * oadRetries and Sensor_msgStats to implement a sliding window
                 * algorithm
                 */
                OADProtocol_Status_t status = OADProtocol_sendOadImgBlockReq(&oadServerAddr, oadImgId, oadBlock, 1);

#ifndef CUI_DISABLE
                /* update display */
                displayOadBlockUpdate(oadBlock, oadBNumBlocks, oadRetries);
#endif

                if (status != OADProtocol_Status_Success){
                    // Let the user know there was an error and retry on next round
                    displayOadStatusUpdate(OADStorage_txError);
                }

                /*
                 * Send poll request to get rsp in OAD_BLOCK_REQ_POLL_DELAY ms
                 * to flush the block response from OAD servers MAC Queue
                 */
                Ssf_setPollClock(OAD_BLOCK_REQ_POLL_DELAY);

                /*
                 * set timer to time out in OAD_BLOCK_REQ_RATE ms to request
                 * or re-request block
                 * */
                oadClockSet(OAD_BLOCK_REQ_RATE);
            }
#if defined(FEATURE_TOAD)
            else if(oadBlock >= oadBNumBlocks && (!isToadImage ||
                    (isToadImage && TOAD_getState() == TOAD_State_decodingDone)))
            {
                if (isToadImage)
                {
                    TOAD_close();
                    isToadImage = false;
                    isOADPaused = false;

                    Util_clearEvent(pEvent, SENSOR_TOAD_DECODE_EVT);
                }
#else
            else if(oadBlock >= oadBNumBlocks)
            {
#endif

                OADStorage_Status_t status;

                /*
                 * Check that CRC is correct and mark the image as new
                 * image to be booted in to by BIM on next reset
                 */
                status = OADStorage_imgFinalise();

#ifndef CUI_DISABLE
                /* Display result */
                displayOadStatusUpdate(status);
#endif

                /* Stop any further OAD message processing */
                oadInProgress = false;
                lastBlock = -1;
                oadBlock = 0;

                /* set block count to 0 in NV */
                Ssf_oadInfoUpdate(&oadBlock, 0, 0, 0);

                /* Set poll rate back to default */
                Jdllc_setPollRate(defaultPollInterval);

#if OAD_EXPLICIT_POLLING
                /* Set autoRequest back to default */
                ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, currAutoReq);
#endif

                /* If OAD was successful reset the device */
                if(status == OADStorage_Status_Success)
                {
                    oadComplete = true;

                    /*
                     * Set timer 1 more time to reset the device
                     * after the display has been written to
                     */
                    oadClockSet((OAD_BLOCK_REQ_RATE*2));
                }
                else
                {
                    //something went wrong abort
                    oadClockSet(0);
                }
            }
        }

        /* Clear the event  */
        Util_clearEvent(pEvent, SENSOR_OAD_TIMEOUT_EVT);
    }

#if defined(FEATURE_TOAD)
    if(*pEvent & SENSOR_TOAD_DECODE_EVT)
    {
        // Start or continue decoding the next stored delta block
        if (TOAD_decode() == TOAD_Status_decodingError)
        {
            Util_clearEvent(pEvent, SENSOR_TOAD_DECODE_EVT);
            OADClient_abort(false);
        }

        // Resume OAD if there is available room in the block cache
        if (isOADPaused && !TOAD_isBlockCacheFull())
        {
            Ssf_setPollClock(OAD_BLOCK_REQ_POLL_DELAY);
            oadClockSet(OAD_BLOCK_REQ_RATE);
            isOADPaused = false;
        }

        /* If TOAD is blocked on waiting for data in the block cache,
         * stop attempting to continue decoding until more data arrives
         */
        if (TOAD_getState() == TOAD_State_decodingWaiting ||
            (TOAD_getState() == TOAD_State_idle && TOAD_isBlockCacheEmpty()))
        {
            Util_clearEvent(pEvent, SENSOR_TOAD_DECODE_EVT);
        }

        if (TOAD_getState() == TOAD_State_decodingDone)
        {
            // Set the timer to finalize the image
            oadClockSet(OAD_BLOCK_REQ_RATE);
            Util_clearEvent(pEvent, SENSOR_TOAD_DECODE_EVT);
        }
    }
#endif

#if defined(OAD_IMG_A)
    if(Sensor_events & SENSOR_OAD_SEND_RESET_RSP_EVT )
    {
        /* send OAD reset response */
        OADProtocol_Status_t  status;
        status = OADProtocol_sendOadResetRsp(&oadServerAddr);

        if(OADProtocol_Status_Success == status)
        {
#ifndef CUI_DISABLE
            //notify to user
            CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Sent Reset Response");
#endif
        }
        else
        {
#ifndef CUI_DISABLE
            CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Failed to send Reset Response");
#endif
        }

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_OAD_SEND_RESET_RSP_EVT);
    }
#endif //OAD_IMG_A
}

/*!
 Abort the OAD.

 Public function defined in sensor.h
 */
void OADClient_abort(bool resume)
{
    if(oadInProgress)
    {
        static uint16_t prevResumeBlock = 0;

        /* Stop OAD timer */
        oadClockSet(0);

        /* Set poll rate back to default */
        Jdllc_setPollRate(defaultPollInterval);

#if OAD_EXPLICIT_POLLING
        /* Set autoRequest back to default */
        ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, currAutoReq);
#endif

#ifndef CUI_DISABLE
        displayOadStatusUpdate(OADStorage_Aborted);
#endif

        /* start timer to auto resume */
        if( resume &&
            (OAD_BLOCK_AUTO_RESUME_DELAY > 0) &&
            (oadBlock != prevResumeBlock) )
        {
            oadClockSet(OAD_BLOCK_AUTO_RESUME_DELAY);
            prevResumeBlock = oadBlock;
            oadRetries = 0;
            oadTimeouts = 0;
        }
        else
        {
            /*abort OAD */
            oadInProgress = false;
            OADStorage_close();

#if defined(FEATURE_TOAD)
            if (isToadImage)
            {
                TOAD_close();
                isToadImage = false;
                isOADPaused = false;
            }
#endif
        }
    }
}

/*!
 Resume the OAD.

 Public function defined in sensor.h
 */
void OADClient_resume(uint32_t delay)
{
    Ssf_getOadInfo(&oadBlock, oadImgIdPld, &oadImgId, &oadServerAddr);

    if(oadBlock != 0)
    {
        uint32_t page = (oadBlock * (OADStorage_BLOCK_SIZE - OADStorage_BLK_NUM_HDR_SZ)) / flashPageSize;
        /* Round block number to nearest page boundary in case flash page is
         * corrupted
         */
        oadBlock = (page * flashPageSize) / (OADStorage_BLOCK_SIZE - OADStorage_BLK_NUM_HDR_SZ);

        OADStorage_init();

        oadBNumBlocks = OADStorage_imgIdentifyWrite(oadImgIdPld);
        if(oadBNumBlocks > 0)
        {
            oadInProgress = true;

#if OAD_EXPLICIT_POLLING
            /* Re-enable explicit polling when resuming OAD */
            ApiMac_mlmeGetReqBool(ApiMac_attribute_autoRequest, &currAutoReq);
            ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, false);
#endif

#if defined(FEATURE_TOAD)
            // Restart from the beginning for delta images
            if (((OADProtocol_imgIdentifyPld_t*)oadImgIdPld)->isDeltaImg)
            {
                oadRetries = 0;
                oadTimeouts = 0;
                oadBlock = 0;
                isToadImage = true;

                TOAD_params_t toadParams;
                TOAD_paramsInit(&toadParams);

                toadParams.oadBlockSize = OAD_BLOCK_SIZE;
                toadParams.pfnWriteDataCb = toadWriteDeltaBlock;
                toadParams.newImgLen = ((OADProtocol_imgIdentifyPld_t*)oadImgIdPld)->newImgLen;

                TOAD_status_t toadStatus = TOAD_Status_initializationError;
                toadStatus = TOAD_init(&toadParams);

                /* Flash pages will not need to be erased for delta images.
                 * After receiving the header (stored in RAM), the oad storage
                 * module will erase flash pages before downloading.
                 */
                if (toadStatus != TOAD_Status_success)
                {
                    OADClient_abort(false);
                    return;
                }
            }
            else
            {
#endif
                //erase the page in case it was corrupted during a reset / power cycle
                OADStorage_eraseImgPage(page);
#if defined(FEATURE_TOAD)
            }
#endif

            oadClockSet(delay);
        }
    }
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief   OAD timeout handler function.
 *
 * @param   a0 - ignored
 */
static void oadClockCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(oadClientParams.pEvent, SENSOR_OAD_TIMEOUT_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(oadClientParams.eventSem);
}

static void oadClockInitialize(void)
{
    /* Initialize the timers needed for this application */
    oadClkHandle = UtilTimer_construct(&oadClkStruct,
                                        oadClockCallback,
                                        OAD_BLOCK_REQ_RATE,
                                        0,
                                        false,
                                        0);
}

/*!
 Set the oad clock.

 Public function defined in ssf.h
 */
static void oadClockSet(uint32_t oadTimeout)
{
    /* Stop the Reading timer */
    if(UtilTimer_isActive(&oadClkStruct) == true)
    {
        UtilTimer_stop(&oadClkStruct);
    }

    /* Setup timer */
    if ( oadTimeout )
    {
        UtilTimer_setTimeout(oadClkHandle, oadTimeout);
        UtilTimer_start(&oadClkStruct);
    }
}

#ifndef CUI_DISABLE
/*!
 The application calls this function to indicate oad block.

 Public function defined in ssf.h
 */
static void displayOadBlockUpdate(uint16_t oadBlock, uint16_t oadBNumBlocks, uint8_t retries)
{
    static uint8_t lastRetries;
    CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Transferring Block %u of %u", oadBlock, oadBNumBlocks);

    if(lastRetries != retries)
    {
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Retrying Block Request - Attempt %u", retries);
    }
}

/*!
 The application calls this function to indicate oad block.

 Public function defined in ssf.h
 */
static void displayOadStatusUpdate(OADStorage_Status_t status)
{
    switch(status)
    {
    case OADStorage_Status_Success:
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Completed Successfully");
        break;
    case OADStorage_CrcError:
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "CRC Failed");
        break;
    case OADStorage_Aborted:
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Aborted");
        break;
    case OADStorage_FlashError:
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Flash Error");
        break;
    case OADStorage_txError:
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Error Transmitting Block Request");
        break;
    default:
        CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Failed");
        break;
    }
}
#endif /* CUI_DISABLE */

#if defined(OAD_ONCHIP) && defined(OAD_IMG_B)
/*********************************************************************
 * @fn      OAD_evenBitCount
 *
 * @brief   Checks if the value field has even number of 1's bit set.
 *
 * @param   value  Value field to check for even number of 1's.
 *
 * @return  TRUE if even number of 1's otherwise false.
 */
static bool OAD_evenBitCount(uint32_t value)
{
  uint8_t count;

  for (count = 0; value; count++)
  {
    value &= value - 1;
  }

  if (count % 2)
  {
    return false;
  }
  else
  {
    return true;
  }
}
#endif

/*!
 * @brief      FW Version request callback
 */
static void oadFwVersionReqCb(void* pSrcAddr)
{
    char fwVersionStr[OADProtocol_FW_VERSION_STR_LEN] = {0};
    char bimVerStr[4];
#ifdef FEATURE_SECURE_OAD
    char securityTypeStr[2];
#endif

    memcpy(fwVersionStr, "sv:", 3);
#ifdef MCUBOOT
    fwVersionStr[3] = ' ';
    fwVersionStr[4] = _mcuBootHdr->ih_ver.iv_major + '0';
    fwVersionStr[5]= '.';
    fwVersionStr[6] = _mcuBootHdr->ih_ver.iv_minor + '0';

    memcpy(&fwVersionStr[7], " bv:", 4);

    //Set BIM Ver to 0x96 for MCUBoot images
    bimVerStr[0] = '0';
    bimVerStr[1] = 'x';
    bimVerStr[2] = '9';
    bimVerStr[3] = '6';

    memcpy(&fwVersionStr[11], bimVerStr, 4);
#else
    memcpy(&fwVersionStr[3], _imgHdr.fixedHdr.softVer, 4);

    memcpy(&fwVersionStr[7], " bv:", 4);

    //convert bimVer to string
    bimVerStr[1] = (_imgHdr.fixedHdr.bimVer & 0x0F) < 0xA ?
            ((_imgHdr.fixedHdr.bimVer & 0x0F) + 0x30):
            ((_imgHdr.fixedHdr.bimVer & 0x0F) + 0x41);
    bimVerStr[0] = ((_imgHdr.fixedHdr.bimVer & 0xF0) >> 8) < 0xA ?
            (((_imgHdr.fixedHdr.bimVer & 0xF0) >> 8) + 0x30):
            (((_imgHdr.fixedHdr.bimVer & 0xF0) >> 8) + 0x41);

    memcpy(&fwVersionStr[11], bimVerStr, 2);
#endif // MCUBOOT

#ifdef FEATURE_SECURE_OAD
    memcpy(&fwVersionStr[13], " st:", 4);

    //convert bimVer to string
    securityTypeStr[1] = (_imgHdr.fixedHdr.secVer & 0x0F) < 0xA ?
            ((_imgHdr.fixedHdr.secVer & 0x0F) + 0x30):
            ((_imgHdr.fixedHdr.secVer & 0x0F) + 0x41);
    securityTypeStr[0] = ((_imgHdr.fixedHdr.secVer & 0xF0) >> 8) < 0xA ?
            (((_imgHdr.fixedHdr.secVer & 0xF0) >> 8) + 0x30):
            (((_imgHdr.fixedHdr.secVer & 0xF0) >> 8) + 0x41);

    memcpy(&fwVersionStr[17], securityTypeStr, 2);
#endif

    //Send response back
    OADProtocol_sendFwVersionRsp(pSrcAddr, fwVersionStr);
}

#if defined(FEATURE_TOAD)
/******************************************************************************
 * @fn     toadWriteDeltaBlock
 *
 * @brief  Writes a decoded delta block to flash
 *
 * @param  pData - address of the block data to write
 * @param  len   - amount of data to write to flash
 */
static void toadWriteDeltaBlock(uint8_t* pData, uint32_t len)
{
    static uint32_t writeOffset = 0;
    OADStorage_Status_t status = OADStorage_Status_Success;

    status = OADStorage_imgDataWrite(writeOffset, pData, 0, len);

    if(status != OADStorage_Status_Success)
    {
#ifndef CUI_DISABLE
        displayOadStatusUpdate(status);
#endif
        /* OAD abort with no auto resume */
        OADClient_abort(false);
        return;
    }

    writeOffset += len;
}
#endif

/*!
 * @brief      OAD image identify request callback
 */
static void oadImgIdentifyReqCb(void* pSrcAddr, uint8_t imgId, uint8_t *imgMetaData)
{
    if (oadInProgress)
    {
        //Send fail response back, but continue existing OAD thats in progress
        OADProtocol_sendOadIdentifyImgRsp(pSrcAddr, 0);
        return;
    }

    /* Store the img header incase of resume */
    memcpy(oadImgIdPld, imgMetaData, OADProtocol_IMAGE_ID_LEN);

    /*store the image ID to get blocks for */
    oadImgId = imgId;

    OADStorage_init();

    OADStorage_Status_t status = OADStorage_Failed;
    oadBNumBlocks = OADStorage_imgIdentifyWrite(oadImgIdPld);

    oadServerAddr.addrMode = ((ApiMac_sAddr_t*)pSrcAddr)->addrMode;
    if(oadServerAddr.addrMode == ApiMac_addrType_short)
    {
        oadServerAddr.addr.shortAddr = ((ApiMac_sAddr_t*)pSrcAddr)->addr.shortAddr;
    }
    else
    {
        memcpy(oadServerAddr.addr.extAddr, ((ApiMac_sAddr_t*)pSrcAddr)->addr.extAddr,
               (APIMAC_SADDR_EXT_LEN));
    }

    if(oadBNumBlocks)
    {
        OADProtocol_imgIdentifyPld_t* imgIdentifyPld = (OADProtocol_imgIdentifyPld_t*)&oadImgIdPld;
        if (imgIdentifyPld->isDeltaImg)
        {
#if defined(FEATURE_TOAD)
            // Also ensure delta image is compatible by comparing CRC from internal flash
            uint32_t* crcFromHdr = (uint32_t*)DELTA_OLD_IMG_CRC_OFFSET;
            uint32_t imgDataCrc;

            // oldImgCrc field is not updated if the image has flashed
            if(*crcFromHdr == DEFAULT_CRC)
            {
                uint32_t* oldImgLen = (uint32_t*)IMG_LEN_OFFSET;
                imgDataCrc = CRC32_calc(0, INTFLASH_PAGE_SIZE, sizeof(imgHdr_t) - IMG_DATA_OFFSET, *oldImgLen, false);
                crcFromHdr = (uint32_t*)&imgDataCrc;
            }

            if (imgIdentifyPld->toadMetaVer == TOAD_META_VER &&
                    imgIdentifyPld->toadVer == TOAD_VER &&
                    imgIdentifyPld->memoryCfg == TOAD_MEMORY_CFG &&
                    imgIdentifyPld->oldImgCrc == *crcFromHdr)
            {
                isToadImage = true;

                TOAD_params_t toadParams;
                TOAD_paramsInit(&toadParams);

                toadParams.oadBlockSize = OAD_BLOCK_SIZE;
                toadParams.pfnWriteDataCb = toadWriteDeltaBlock;
                toadParams.newImgLen = ((OADProtocol_imgIdentifyPld_t*)oadImgIdPld)->newImgLen;

                TOAD_status_t toadStatus = TOAD_Status_initializationError;
                toadStatus = TOAD_init(&toadParams);

                // Sending delta Turbo OAD image if no initialization errors
                status = (toadStatus == TOAD_Status_success ? OADStorage_Status_Success : OADStorage_Rejected);
            }
            else
            {
                /* Reject delta image if version is unsupported, built with the
                 * wrong memory configuration, or created from a different old image
                 */
                status = OADStorage_Rejected;
            }
#else
            // Reject delta image if Turbo OAD is not enabled
            status = OADStorage_Rejected;
#endif
        }
        else
        {
            // Sending regular OAD image
            status = OADStorage_Status_Success;
        }
    }
    else
    {
        // Error reading image since oadBNumBlocks is 0
        status = OADStorage_Rejected;
    }

    if (status == OADStorage_Status_Success)
    {
        oadInProgress = true;
        oadRetries = 0;
        oadTimeouts = 0;
        oadBlock = 0;

        //Save current Poll interval
        defaultPollInterval = Ssf_getPollClock();

#if OAD_EXPLICIT_POLLING
        /* Save the current autoRequest value to restore it after OAD is complete */
        ApiMac_mlmeGetReqBool(ApiMac_attribute_autoRequest, &currAutoReq);
        ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, false);
#endif

        //Send success response back
        OADProtocol_sendOadIdentifyImgRsp(pSrcAddr, 1);

        //Set OAD time out to OAD_BLOCK_REQ_RATE ms
        oadClockSet(OAD_BLOCK_REQ_RATE);
    }
    else
    {
#ifndef CUI_DISABLE
        displayOadStatusUpdate(status);
#endif

        //Send fail response back
        OADProtocol_sendOadIdentifyImgRsp(pSrcAddr, 0);
    }
}

/*!
 * @brief      OAD image block response callback
 */
static void oadBlockRspCb(void* pSrcAddr, uint8_t imgId, uint16_t blockNum, uint8_t *blkData)
{
    OADStorage_Status_t status = OADStorage_Status_Success;

    if( (blockNum == oadBlock) && (oadInProgress) && (oadImgId == imgId))
    {
        oadBlock++;
        oadRetries = 0;
        oadTimeouts = 0;

        if(isToadImage)
        {
#if defined(FEATURE_TOAD)
            // OAD storage header bytes are ignored
            if (TOAD_addBlockToCache(blkData + OADStorage_BLK_NUM_HDR_SZ) == TOAD_Status_noMemory)
            {
                // Should not happen, but if so, re-request the block when more room is available
                oadBlock--;
            }

            if (TOAD_isBlockCacheFull())
            {
                // Stop OAD timer until there is more room in the cache to continue the transfer
                oadClockSet(0);
                isOADPaused = true;
            }

            Util_setEvent(oadClientParams.pEvent, SENSOR_TOAD_DECODE_EVT);
#endif
        }
        else
        {
            //Cast to uint32_t as the OAD BIM Header and storage now supports 32b block size
            status = OADStorage_imgBlockWrite( (uint32_t) blockNum, blkData, OADStorage_BLOCK_SIZE);

            if(status != OADStorage_Status_Success)
            {
#ifndef CUI_DISABLE
                displayOadStatusUpdate(status);
#endif
                /* OAD abort with no auto resume */
                OADClient_abort(false);
                return;
            }
        }

        //set poll rate back to default
        Ssf_setPollClock(defaultPollInterval);
        Jdllc_setPollRate(defaultPollInterval);
    }
}

#ifdef OAD_ONCHIP
/*!
 * @brief     Oad Reset request callback
 */
static void oadResetReqCb(void* pSrcAddr)
{
#ifdef OAD_IMG_A
    Util_setEvent(&Sensor_events, SENSOR_OAD_SEND_RESET_RSP_EVT);
#endif

#ifdef OAD_IMG_B //u-app only
    CUI_statusLinePrintf(*(oadClientParams.pOadCuiHndl), oadStatusLine, "Resetting the device to boot to Persistent App");

    //write to NV memory the server address
    oadServerAddr.addrMode = ((ApiMac_sAddr_t*)pSrcAddr)->addrMode;
    if(oadServerAddr.addrMode == ApiMac_addrType_short)
    {
        oadServerAddr.addr.shortAddr = ((ApiMac_sAddr_t*)pSrcAddr)->addr.shortAddr;
    }
    else
    {
        memcpy(oadServerAddr.addr.extAddr, ((ApiMac_sAddr_t*)pSrcAddr)->addr.extAddr,
               (APIMAC_SADDR_EXT_LEN));
    }

    Ssf_oadInfoUpdate(0, 0, 0, &oadServerAddr);

    uint8_t status = FLASH_FAILURE;

    //read the image validation bytes and set it appropriately.
    imgHdr_t imgHdr = {0};

    if(flash_open())
    {
        status = readFlash(0x0, (uint8_t *)&imgHdr, OAD_IMG_HDR_LEN);
    }

    if ((FLASH_SUCCESS == status) && ( imgHdr.fixedHdr.imgVld != 0))
    {
        if ( true == OAD_evenBitCount(imgHdr.fixedHdr.imgVld) )
        {
          imgHdr.fixedHdr.imgVld = imgHdr.fixedHdr.imgVld << 1;
          writeFlash((uint32_t)FLASH_ADDRESS(0, IMG_VALIDATION_OFFSET),
                     (uint8_t *)&(imgHdr.fixedHdr.imgVld), sizeof(imgHdr.fixedHdr.imgVld));
        }
    }

    /* reset to BIM */
    SysCtrlSystemReset();
#endif
}
#endif //OAD_ONCHIP

/*!
 * @brief      Radio access function for OAD module to send messages
 */
void* oadRadioAccessAllocMsg(uint32_t msgLen)
{
    uint8_t *msgBuffer;

    //allocate buffer for CmdId + message
    msgBuffer = Ssf_malloc(msgLen + 1);

    memset(msgBuffer, 0, msgLen + 1);

    return msgBuffer + 1;
}

/*!
 * @brief      Radio access function for OAD module to send messages
 */
static OADProtocol_Status_t oadRadioAccessPacketSend(void* pDstAddr, uint8_t *pMsg, uint32_t msgLen)
{
    OADProtocol_Status_t status = OADProtocol_Failed;
    uint8_t* pMsduPayload;

    //buffer should have been allocated with oadRadioAccessAllocMsg,
    //so 1 byte before the oad msg buffer was allocated for the Smsgs_cmdId
    pMsduPayload = pMsg - 1;
    pMsduPayload[0] = Smsgs_cmdIds_oad;

    if(Sensor_sendMsg(Smsgs_cmdIds_oad, (ApiMac_sAddr_t*) pDstAddr, true,
               msgLen+1 , pMsduPayload) == true)
    {
        status = OADProtocol_Status_Success;
    }

    //free the memory allocated in oadRadioAccessAllocMsg
    Ssf_free(pMsduPayload);

    return status;
}

#endif //FEATURE_NATIVE_OAD

#endif //OAD_CLIENT_H
