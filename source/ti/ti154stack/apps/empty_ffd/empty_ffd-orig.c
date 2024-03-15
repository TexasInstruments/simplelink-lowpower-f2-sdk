/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty_ffd.c ========
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h> //for memset

#include <semaphore.h>
#include <signal.h>
#include <time.h>

//todo port mac_api to POSIX semaphores
//#include <semaphore.h>
#include <ti/sysbios/knl/Semaphore.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
// #include <ti/drivers/I2C.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>

/* Board Header file */
#include "ti_drivers_config.h"

/* MAC header file */
#include "mac_api.h"
#include "ti_154stack_config.h"
#include "advanced_config.h"

/*! non-beacn mode beacon order */
#define BEACON_ORDER_NON_BEACON 15
/*! link quality */
#define CONFIG_LINKQUALITY           1
/*! percent filter */
#define CONFIG_PERCENTFILTER         0xFF
/*! MPM Constants for start request */
#define FFD_OFFSET_TIMESLOT     0
#define FFD_EBEACONORDER        15
#define FFD_NBPANEBEACONORDER   16383

/* Number of superframe periods to hold a indirect packet at collector for
Sensor to poll and get the frame*/
#define BCN_MODE_INDIRECT_PERSISTENT_TIME 3

/* aBaseSuperFrame Duration in slots */
#define BASE_SUPER_FRAME_DURATION   960

/* symbol duration for 50 kbps mode in micro seconds*/
#define SYMBOL_DURATION_50_kbps 20

/* symbol duration for 200 kbps mode in micro seconds*/
#define SYMBOL_DURATION_200_kbps 5

/* symbol duration for 250 kbps mode in micro seconds*/
#define SYMBOL_DURATION_250_kbps 16

/* symbol duration for LRM mode in micro seconds*/
#define SYMBOL_DURATION_LRM 50

#if ((CONFIG_PHY_ID >= APIMAC_MRFSK_STD_PHY_ID_BEGIN) && (CONFIG_PHY_ID <= APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN))

/* MAC Indirect Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME ((5 * 1000 * CONFIG_POLLING_INTERVAL / 2)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_50_kbps))

#elif ((CONFIG_PHY_ID >= APIMAC_GENERIC_US_915_PHY_132) && (CONFIG_PHY_ID <= APIMAC_GENERIC_ETSI_863_PHY_133))

/* MAC Indirect Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME ((5 * 1000 * CONFIG_POLLING_INTERVAL / 2)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_200_kbps))

#elif (CONFIG_PHY_ID == APIMAC_PHY_ID_NONE)

/* MAC Indirect Persistent Timeout */
#ifdef FEATURE_SECURE_COMMISSIONING
/* Increase the persistence time by a factor of 10 */
#define INDIRECT_PERSISTENT_TIME ((5* 10 * 1000 * CONFIG_POLLING_INTERVAL / 2)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_250_kbps))
#else
#define INDIRECT_PERSISTENT_TIME ((5 * 1000 * CONFIG_POLLING_INTERVAL / 2)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_250_kbps))
#endif

#else
/* MAC Indirect Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME ((5 * 1000 * CONFIG_POLLING_INTERVAL / 2)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                    SYMBOL_DURATION_LRM))
#endif

/* callback prototypes */
static void scanCnfCb(ApiMac_mlmeScanCnf_t *pScanCnf);
static void startCnfCb(ApiMac_mlmeStartCnf_t *pStartCnf);
static void associateIndCb(ApiMac_mlmeAssociateInd_t *pAssocInd);
static void dataInd(ApiMac_mcpsDataInd_t *pDataInd);
static void pollInd(ApiMac_mlmePollInd_t *pPollInd);

static void sendPongEvent(void);
static void sendPong(uint16_t dstAddr);

/* API MAC Callbacks */
static ApiMac_callbacks_t macCallbacks =
{
    /*! Associate Indicated callback */
    associateIndCb,
    /*! Associate Confirmation callback */
    NULL,
    /*! Disassociate Indication callback */
    NULL,
    /*! Disassociate Confirmation callback */
    NULL,
    /*! Beacon Notify Indication callback */
    NULL,
    /*! Orphan Indication callback */
    NULL,
    /*! Scan Confirmation callback */
    scanCnfCb,
    /*! Start Confirmation callback */
    startCnfCb,
    /*! Sync Loss Indication callback */
    NULL,
    /*! Poll Confirm callback */
    NULL,
    /*! Comm Status Indication callback */
    NULL,
    /*! Poll Indication Callback */
    pollInd,
    /*! Data Confirmation callback */
    NULL,
    /*! Data Indication callback */
    dataInd,
    /*! Purge Confirm callback */
    NULL,
    /*! WiSUN Async Indication callback */
    NULL,
    /*! WiSUN Async Confirmation callback */
    NULL,
    /*! Unprocessed message callback */
    NULL
};

static ApiMac_mlmeBeaconNotifyInd_t discoveredNetwork;
/* todo: need to port api_mac to use POSIX semaphore */
Semaphore_Handle macApiSem;
uint16_t rfdAddr = 0;
bool sendPongFlag = false;

/*
 *  ======== sendPongEvent ========
 *  Function called when the timer (created in setupTimer) expires.
 */
static void sendPongEvent(void)
{
    sendPongFlag = true;
    Semaphore_post(macApiSem);
}

/*
 *  ======== sendPong ========
 *  Function called to send data when the timer expires.
 */
static void sendPong(uint16_t dstAddr)
{
    ApiMac_mcpsDataReq_t dataReq;
    uint8_t data[] = "pong";

    /* Fill the data request field */
    memset(&dataReq, 0, sizeof(ApiMac_mcpsDataReq_t));

    dataReq.srcAddrMode = ApiMac_addrType_short;
    dataReq.dstAddr.addr.shortAddr = dstAddr;
    dataReq.dstPanId = discoveredNetwork.panDesc.coordPanId;
    dataReq.txOptions.ack = true;
    dataReq.txOptions.indirect = true;
    dataReq.msduHandle = 0;
    dataReq.msdu.len = sizeof(data);
    dataReq.msdu.p = data;
    dataReq.sec.securityLevel = ApiMac_secLevel_none;

    /* Send the message */
    ApiMac_mcpsDataReq(&dataReq);

    GPIO_toggle(CONFIG_GPIO_GLED);
}

/*!
 Scan Confirmation Callback
 */
static void scanCnfCb(ApiMac_mlmeScanCnf_t *pScanCnf)
{
    ApiMac_mlmeStartReq_t startReq;
    ApiMac_status_t macStatus;

    memset(&startReq, 0, sizeof(ApiMac_mlmeStartReq_t));

    /* read network parameters fill them in start req */
    startReq.startTime = 0;
    startReq.panId = 0x0001; //CONFIG_PAN_ID;
    startReq.logicalChannel = 0; //channelMask[0]; //select first channel
    startReq.channelPage = CONFIG_CHANNEL_PAGE;
    startReq.phyID = CONFIG_PHY_ID;
    startReq.beaconOrder = CONFIG_MAC_BEACON_ORDER;
    startReq.superframeOrder = CONFIG_MAC_SUPERFRAME_ORDER;
    startReq.panCoordinator = true;
    startReq.batteryLifeExt = false;
    startReq.coordRealignment = false;
    startReq.realignSec.securityLevel = false;
    startReq.startFH = false;
    startReq.mpmParams.offsetTimeSlot = FFD_OFFSET_TIMESLOT;
    startReq.mpmParams.eBeaconOrder = FFD_EBEACONORDER;
    startReq.mpmParams.NBPANEBeaconOrder = FFD_NBPANEBEACONORDER;
    startReq.mpmParams.pIEIDs = NULL;

    /* send Start Req to MAC API */
    macStatus = ApiMac_mlmeStartReq(&startReq);
}

static void startCnfCb(ApiMac_mlmeStartCnf_t *pStartCnf)
{
    //turn on LED
    GPIO_write(CONFIG_GPIO_GLED, CONFIG_LED_OFF);

    /* Open network for joining */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_associatePermit, true);
}

ApiMac_mlmeAssociateRsp_t assocRsp;

static void associateIndCb(ApiMac_mlmeAssociateInd_t *pAssocInd)
{
    ApiMac_status_t macStatus;

    assocRsp.status = ApiMac_assocStatus_success;

    memset(&assocRsp.sec, 0, sizeof(ApiMac_sec_t));
    memcpy(&assocRsp.deviceAddress, &pAssocInd->deviceAddress, sizeof(ApiMac_sAddrExt_t));
    assocRsp.assocShortAddress = rfdAddr;

    /* Send response back to the device */
    macStatus = ApiMac_mlmeAssociateRsp(&assocRsp);

    if(macStatus == ApiMac_status_success)
    {
        rfdAddr++;
    }
}

static void dataInd(ApiMac_mcpsDataInd_t *pDataInd)
{
    uint8_t rxExpected[] = "ping";

    if(memcmp(rxExpected, pDataInd->msdu.p, pDataInd->msdu.len) == 0)
    {
        sendPongEvent();
    }
}

static void pollInd(ApiMac_mlmePollInd_t *pPollInd)
{
    GPIO_toggle(CONFIG_GPIO_RLED);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    uint8_t macTaskId = *((uint8_t*)arg0);
    ApiMac_status_t macStatus;
    ApiMac_mlmeScanReq_t scanReq;
    uint8_t channelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ] = CONFIG_CHANNEL_MASK;

    /* Call driver init functions */
    GPIO_init();
    // I2C_init();
    // SPI_init();
    // UART_init();
    // Watchdog_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_RLED, CONFIG_LED_ON);
    GPIO_write(CONFIG_GPIO_GLED, CONFIG_LED_OFF);

    /* init the MAC */
    macApiSem = (Semaphore_Handle) ApiMac_init(macTaskId, false);

    /* Register the MAC Callbacks */
    ApiMac_registerCallbacks(&macCallbacks);


    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyCurrentDescriptorId,
                           (uint8_t)CONFIG_PHY_ID);

    ApiMac_mlmeSetReqUint8(ApiMac_attribute_channelPage,
                           (uint8_t)CONFIG_CHANNEL_PAGE);

    ApiMac_mlmeSetReqBool(ApiMac_attribute_RxOnWhenIdle,true);

    /* set PIB items */
    /* setup short address */
    ApiMac_mlmeSetReqUint16(ApiMac_attribute_shortAddress,
                            0x1122);

    ApiMac_mlmeSetReqUint16(ApiMac_attribute_transactionPersistenceTime,
                                INDIRECT_PERSISTENT_TIME);

    /* Set Min BE */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_backoffExponent,
                              (uint8_t)CONFIG_MIN_BE);
    /* Set Max BE */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_maxBackoffExponent,
                              (uint8_t)CONFIG_MAX_BE);
    /* Set MAC MAX CSMA Backoffs */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_maxCsmaBackoffs,
                              (uint8_t)CONFIG_MAC_MAX_CSMA_BACKOFFS);
    /* Set MAC MAX Frame Retries */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_maxFrameRetries,
                              (uint8_t)CONFIG_MAX_RETRIES);

    /* configure the scan */
    memset(&scanReq, 0, sizeof(ApiMac_mlmeScanReq_t));
    memcpy(scanReq.scanChannels, channelMask,
           APIMAC_154G_CHANNEL_BITMAP_SIZ);
    if(CONFIG_MAC_BEACON_ORDER == BEACON_ORDER_NON_BEACON)
    {
        /* non-beacon mode scan */
        scanReq.scanDuration = CONFIG_SCAN_DURATION;
        scanReq.scanType = ApiMac_scantype_active;
    }
    else
    {
        /* beacon mode scan */
        scanReq.scanDuration = CONFIG_MAC_BEACON_ORDER;
        scanReq.scanType = ApiMac_scantype_energyDetect;
    }
    scanReq.maxResults = 0;/* Expecting beacon notifications */
    scanReq.permitJoining = false;
    scanReq.linkQuality = CONFIG_LINKQUALITY;
    scanReq.percentFilter = CONFIG_PERCENTFILTER;
    scanReq.channelPage = CONFIG_CHANNEL_PAGE;

    /* using no security for scan request command */
    memset(&scanReq.sec, 0, sizeof(ApiMac_sec_t));

    /* send scan Req */
    macStatus = ApiMac_mlmeScanReq(&scanReq);

    if(macStatus != ApiMac_status_success)
    {
        while(1);
    }

    while (1) {
        /* Wait for response message or events */
        ApiMac_processIncoming();

        /* Was pMacApiSem unblocked by app to send data */
        if(sendPongFlag)
        {
            /* Send data */
            sendPongFlag = false;
            sendPong(discoveredNetwork.panDesc.coordAddress.addr.shortAddr);
        }
    }
}
