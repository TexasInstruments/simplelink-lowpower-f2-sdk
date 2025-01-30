/******************************************************************************

 @file cllc.c

 @brief Coordinator Logical Link Controller

 This module is the Coordinator Logical Link Controller for the application.

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
#include "cllc.h"
#include "csf.h"
#include "mac_util.h"
#ifdef __unix__
#include "csf_linux.h"
#include "cllc_linux.h"
#endif
#ifdef FEATURE_SECURE_COMMISSIONING
#ifdef USE_DMM
#include "remote_display.h"
#include <sm_commissioning_gatt_profile.h>
#endif /* USE_DMM */
#include "sm_ti154.h"
#include "icall_osal_rom_jt.h"
#endif /* FEATURE_SECURE_COMMISSIONING */
#ifdef POWER_MEAS
#include "collector.h"
#endif
#ifndef __unix__
#include "advanced_config.h"
#endif
#if defined(S_NS_154_ENABLED) && defined(MAC_LEVEL_SKS)
#include "mac_assert.h"
#endif

#if defined(S_NS_154_ENABLED) && !defined(MAC_LEVEL_SKS)
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyKeyStore_PSA_init.h>
#endif
/******************************************************************************
 Constants and definitions
 *****************************************************************************/
#define CLLC_CHAN_LOWEST             0
/* Returns if the specific bit in the scan channel map array is set */
#define CLLC_IS_CHANNEL_MASK_SET(a, c) \
                     (*((uint8_t*)(a) + ((c) - CLLC_CHAN_LOWEST) / 8) & \
                     ((uint8_t) 1 << (((c) - CLLC_CHAN_LOWEST) % 8)))
#define CLLC_DEFAULT_KEY_SOURCE {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33}
#define CLLC_INVALID_PAN         0xFFFF
#define CLLC_JOIN_PERMIT_ON      0xFFFFFFFF
#define CLLC_MAX_ENERGY          255
#define CLLC_PAN_NOT_FOUND       0x0000
#define CLLC_SET_CHANNEL(a,b) (a)[(b)>>3] |= (1 << ((b) & 7))

/*! MPM Constants for start request */
#define CLLC_OFFSET_TIMESLOT     0
#define CLLC_EBEACONORDER        15
#define CLLC_NBPANEBEACONORDER   16383

/*! FH default PIB values */
/*! value for ApiMac_FHAttribute_routingCost PIB */
#define CLLC_FH_ROUTING_COST            0x00
/*! value for ApiMac_FHAttribute_routingMethod PIB */
#define CLLC_FH_ROUTING_METHOD          0x01
/*! value for ApiMac_FHAttribute_eapolReady PIB */
#define CLLC_FH_EAPOL_READY             0x01
/*! value for ApiMac_FHAttribute_fanTPSVersion PIB */
#define CLLC_FH_FANTPSVERSION           0x01
/*! value for ApiMac_FHAttribute_gtk0Hash PIB */
#define CLLC_FH_GTK0HASH                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
/*! value for ApiMac_FHAttribute_gtk1Hash PIB */
#define CLLC_FH_GTK1HASH                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}
/*! value for ApiMac_FHAttribute_gtk2Hash PIB */
#define CLLC_FH_GTK2HASH                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}
/*! value for ApiMac_FHAttribute_gtk3Hash PIB */
#define CLLC_FH_GTK3HASH                {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03}
#define CLLC_FH_PANVERSION              0x0000

#define CLLC_FH_MAX_TRICKLE             CONFIG_TRICKLE_MAX_CLK_DURATION
#define CLLC_FH_MIN_TRICKLE             CONFIG_TRICKLE_MIN_CLK_DURATION

#define PA_HOP_NEIGHBOR_FOUND_MASK      0x1
#define PA_FIXED_NEIGHBOR_FOUND_MASK    0x2
#define PC_HOP_NEIGHBOR_FOUND_MASK      0x4
#define PC_FIXED_NEIGHBOR_FOUND_MASK    0x8

#define USIE_FIXED_CHANNEL_OFFSET_CP0   6
#define USIE_FIXED_CHANNEL_OFFSET_CP1   10

/*!
 Variable to start the assignment of short addresses by the coordinator
 to each the device that associates to it
 */
#define CLLC_ASSOC_DEVICE_STARTING_NUMBER 1
/*! link quality */
#define CONFIG_LINKQUALITY                1
/*! percent filter */
#define CONFIG_PERCENTFILTER              0xFF

/* Definition of maintainAssocTable modes */
#define CLLC_MAINTAIN_ASSOC_TABLE_ADD_FREE   0
#define CLLC_MAINTAIN_ASSOC_TABLE_ADD_INDEX  1
#define CLLC_MAINTAIN_ASSOC_TABLE_UPDATE     2

/******************************************************************************
 Security constants and definitions
 *****************************************************************************/

#define KEY_TABLE_ENTRIES 1
#define KEY_ID_LOOKUP_ENTRIES 1
#define KEY_DEVICE_TABLE_ENTRIES 8
#define KEY_USAGE_TABLE_ENTRIES 1
#define SECURITY_LEVEL_ENTRIES 1

#define MAC_FRAME_TYPE_DATA 1
#define MAC_DATA_REQ_FRAME 4

/******************************************************************************
 Structures
 *****************************************************************************/
/* building block for PAN descriptor linked list used to store PAN
 descriptors received during beacon and scan */
typedef struct
{
    void *pNext;
    ApiMac_panDesc_t panDescList;
} panDescList_t;

/* Coordinator information, used to store default parameters */
typedef struct
{
    uint8_t channel;
    uint16_t panID;
    uint16_t shortAddr;
    Cllc_states_t currentCllcState;
    Cllc_coord_states_t currentCoordState;
} coordInformation_t;

/******************************************************************************
 Global variables
 *****************************************************************************/
/* Task pending events */
uint16_t Cllc_events = 0;
/* Association table */
Cllc_associated_devices_t Cllc_associatedDevList[CONFIG_MAX_DEVICES];
Cllc_statistics_t Cllc_statistics;

/**
 * Variable to start the assignment of short addresses by the coordinator
 * to each the device that associates to it
 */
uint16_t Cllc_devShortAddr = CLLC_ASSOC_DEVICE_STARTING_NUMBER;
uint8_t CONST Cllc_keySource[] = CLLC_DEFAULT_KEY_SOURCE;

/******************************************************************************
 Local variables
 *****************************************************************************/
/* Default variables structure for coordinator */
STATIC coordInformation_t coordInfoBlock =
                {
                 CLLC_CHAN_LOWEST,
                 0x0001,
#ifndef __unix__
                 CONFIG_COORD_SHORT_ADDR,
#else
                 CONFIG_COORD_SHORT_ADDR_DEFAULT,
#endif
                 Cllc_states_initWaiting,
                 Cllc_coordStates_initialized
                };
/* default channel mask */
#ifndef __unix__
#if CONFIG_FH_ENABLE
STATIC uint8_t chanMask[APIMAC_154G_CHANNEL_BITMAP_SIZ] =
                                  FH_ASYNC_CHANNEL_MASK;
#else
STATIC uint8_t chanMask[APIMAC_154G_CHANNEL_BITMAP_SIZ] =
                                  CONFIG_CHANNEL_MASK;
#endif
#else
STATIC uint8_t chanMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
#endif
/* Linked list to store incoming PAN descriptors */
STATIC panDescList_t *pPANDesclist = NULL;
/* number of devices associated with the coordinator */
STATIC uint16_t Cllc_numOfDevices = 0;
/* copy of MAC API callbacks */
STATIC ApiMac_callbacks_t macCallbacksCopy = { 0 };
/* copy of CLLC callbacks */
STATIC Cllc_callbacks_t *pCllcCallbacksCopy = (Cllc_callbacks_t *) NULL;
#ifndef __unix__
STATIC CONST uint8_t fhNetname[APIMAC_FH_NET_NAME_SIZE_MAX] = CONFIG_FH_NETNAME;
#else
#define fhNetname ((uint8_t *)(&CONFIG_FH_NETNAME[0]))
#endif
STATIC CONST uint8_t fhGtkHash0[] = CLLC_FH_GTK0HASH;
STATIC CONST uint8_t fhGtkHash1[] = CLLC_FH_GTK1HASH;
STATIC CONST uint8_t fhGtkHash2[] = CLLC_FH_GTK2HASH;
STATIC CONST uint8_t fhGtkHash3[] = CLLC_FH_GTK3HASH;
#ifndef __unix__
STATIC uint32_t fhPAtrickleTime = CLLC_FH_MIN_TRICKLE;
STATIC uint32_t fhPCtrickleTime = CLLC_FH_MIN_TRICKLE;
#else
STATIC uint32_t fhPAtrickleTime;
STATIC uint32_t fhPCtrickleTime;
#endif
/* set of channels on which target nodes are expected to listen */
STATIC uint8_t optPAChMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
/* set of channels on which target nodes are expected to listen */
STATIC uint8_t optPCChMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
/* Flag to specify whether set of expected target channel is known */
STATIC uint8_t optAsyncFlag = false;

#ifndef __unix__
#if CONFIG_FH_ENABLE
/* FH Channel Mask */
STATIC uint8_t fhChannelMask[] = CONFIG_FH_CHANNEL_MASK;
STATIC uint8_t asyncChannelMask[] = FH_ASYNC_CHANNEL_MASK;
#endif /* CONFIG_FH_ENABLE */
#else
STATIC uint8_t asyncChannelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
#endif

#ifdef FEATURE_MAC_SECURITY
/******************************************************************************
 Local security variables
 *****************************************************************************/

STATIC CONST ApiMac_keyIdLookupDescriptor_t keyIdLookupList[] =
    {
      {
        /* Key identity data */
        { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x03 },
        0x01 /* 9 octets */
      }
    };

/* Key device list can be modified at run time */
STATIC CONST ApiMac_keyDeviceDescriptor_t keyDeviceList[] =
    {
      { 0x00, false, false },
      { 0x00, false, false },
      { 0x00, false, false },
      { 0x00, false, false },
      { 0x00, false, false },
      { 0x00, false, false },
      { 0x00, false, false },
      { 0x00, false, false }
    };

STATIC CONST ApiMac_keyUsageDescriptor_t keyUsageList[] =
    {
      { MAC_FRAME_TYPE_DATA, MAC_DATA_REQ_FRAME }
    };

STATIC ApiMac_keyDescriptor_t keyTable[] =
    {
      {
        (ApiMac_keyIdLookupDescriptor_t *)keyIdLookupList,
        KEY_ID_LOOKUP_ENTRIES,
        (ApiMac_keyDeviceDescriptor_t *)keyDeviceList,
        KEY_DEVICE_TABLE_ENTRIES,
        (ApiMac_keyUsageDescriptor_t *)keyUsageList,
        KEY_USAGE_TABLE_ENTRIES,
        KEY_TABLE_DEFAULT_KEY,
        0 /* frame counter */
      }
    };

STATIC CONST ApiMac_securityPibSecurityLevelEntry_t securityLevelEntry =
    {
      0,
      { MAC_FRAME_TYPE_DATA, MAC_DATA_REQ_FRAME, 0, false }
    };

STATIC CONST ApiMac_secLevel_t secLevel = ApiMac_secLevel_encMic32;

STATIC CONST ApiMac_keyIdMode_t secKeyIdMode = ApiMac_keyIdMode_8;
STATIC CONST uint8_t secKeyIndex = 3; /* cant be zero for implicit key identifier */

#ifndef __unix__
STATIC bool macSecurity = CONFIG_SECURE;
#else
#define macSecurity CONFIG_SECURE /* linux, we use the config variable */
bool networkStarted = 0;
#endif
#endif /* FEATURE_MAC_SECURITY */

#ifdef FEATURE_SECURE_COMMISSIONING
/* Copy of sensor device info received in Assoc Request: to be used for commissioning */
STATIC ApiMac_deviceDescriptor_t assocDevInfo;
/* Copy of RX on Idle info received in Assoc Request: to be used for commissioning */
STATIC bool assocDevRxOnIdle;
#endif /* FEATURE_SECURE_COMMISSIONING */
/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
/* CLLC callbacks */
static void assocIndCb(ApiMac_mlmeAssociateInd_t *pData);
static void beaconNotifyIndCb(ApiMac_mlmeBeaconNotifyInd_t *pData);
static void scanCnfCb(ApiMac_mlmeScanCnf_t *pData);
static void startCnfCb(ApiMac_mlmeStartCnf_t *pData);
static void disassocIndCb(ApiMac_mlmeDisassociateInd_t *pData);
static void disassocCnfCb(ApiMac_mlmeDisassociateCnf_t *pData);
static void wsAsyncIndCb(ApiMac_mlmeWsAsyncInd_t *pData);
static void dataIndCb(ApiMac_mcpsDataInd_t *pData);
static void orphanIndCb(ApiMac_mlmeOrphanInd_t *pData);
static void commStatusIndCb(ApiMac_mlmeCommStatusInd_t *pCommStatusInd);

#ifdef __unix__
static void resetIndCb(ApiMac_mcpsResetInd_t *pResetInd);
#endif

static void switchState(Cllc_coord_states_t newState);
static void processState(Cllc_coord_states_t state);
static void maintainAssocTable(ApiMac_deviceDescriptor_t *pDevInfo,
                               ApiMac_capabilityInfo_t *pCapInfo,
                               int8_t rssi,
                               uint16_t status,
                               uint16_t index,
                               uint8_t mode);
static void configureStartParam(uint8_t channel);

/* PAN decriptor list management functions */
static void addToPANList(ApiMac_panDesc_t *pData);
static void clearPANList(void);
static bool findInList(ApiMac_panDesc_t *pData);

/* Scan results parsing */
static uint8_t findBestChannel(uint8_t *pResults);
static uint16_t findChannel(uint16_t panID,uint8_t channel);

static void updateState(Cllc_states_t state);
static void sendAsyncReq(uint8_t frameType);
static void joinPermitExpired(void);
static void sendStartReq(bool startFH);
static uint8_t calculateChannelCount(uint8_t *channelMask);
static void sendScanReq(ApiMac_scantype_t type);
static void setTrickleTime(uint32_t *pTrickleTime, uint8_t frameType);
static void processIncomingFHframe(uint8_t frameType);
static void processIncomingAsyncUSIE(uint8_t frameType, uint8_t* pIEContent);

/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Initialize this module.

 Public function defined in cllc.h
 This init function saves all of the mac callbacks in macCallbacksCopy,
 and then overwrites them with the application callbacks.
 pCllcCbs is passed in from Cllc_Callbacks defined in collector.c.
 */
void Cllc_init(ApiMac_callbacks_t *pMacCbs, Cllc_callbacks_t *pCllcCbs)
{
    uint16_t panId = CONFIG_PAN_ID;

#ifdef __unix__
    CLLC_LINUX_init(chanMask, &coordInfoBlock.shortAddr, &fhPAtrickleTime, &fhPCtrickleTime);
#endif

    /* Initialize coordinator PAN ID if a valid value is defined in config */
    if((panId != 0x0000) && (panId != CLLC_INVALID_PAN))
    {
        coordInfoBlock.panID = panId;
    }

    /* Save MAC API callback */
    memcpy(&macCallbacksCopy, pMacCbs, sizeof(ApiMac_callbacks_t));
    pCllcCallbacksCopy = pCllcCbs;

    /* overwrite callbacks with llc callbacks
     * set the callbacks defined in the application */
    pMacCbs->pAssocIndCb = assocIndCb;
    pMacCbs->pStartCnfCb = startCnfCb;
    pMacCbs->pDisassociateIndCb = disassocIndCb;
    pMacCbs->pDisassociateCnfCb = disassocCnfCb;
    pMacCbs->pDataIndCb = dataIndCb;
    pMacCbs->pCommStatusCb = commStatusIndCb;
#ifdef __unix__
    pMacCbs->pResetIndCb = resetIndCb;
#endif

    if(!CONFIG_FH_ENABLE)
    {
        pMacCbs->pBeaconNotifyIndCb = beaconNotifyIndCb;
        pMacCbs->pScanCnfCb = scanCnfCb;
        pMacCbs->pOrphanIndCb = orphanIndCb;
    }
    else
    {
        pMacCbs->pWsAsyncIndCb = wsAsyncIndCb;
    }

    /* initialize association table */
    memset(Cllc_associatedDevList, 0xFF,
           (sizeof(Cllc_associated_devices_t) * CONFIG_MAX_DEVICES));

    ApiMac_mlmeSetReqBool(ApiMac_attribute_RxOnWhenIdle,true);

    /* set PIB items */
    /* setup short address */
    ApiMac_mlmeSetReqUint16(ApiMac_attribute_shortAddress,
                            coordInfoBlock.shortAddr);
#if defined( POWER_MEAS )
    /* Always set association permit to 1 */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_associatePermit, true);
#endif


    if(CONFIG_FH_ENABLE)
    {
        uint8_t excludeChannels[APIMAC_154G_CHANNEL_BITMAP_SIZ];
        uint8_t sizeOfChannelMask, idx;

        /* Always set association permit to 1 for FH */
        ApiMac_mlmeSetReqBool(ApiMac_attribute_associatePermit, true);

#ifndef __unix__
        uint8_t configChannelMask[] = CONFIG_FH_CHANNEL_MASK;
#else
        uint8_t configChannelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
        memcpy( configChannelMask, linux_CONFIG_FH_CHANNEL_MASK, sizeof(configChannelMask) );
#endif

        /* initialize app clocks */
        Csf_initializeTrickleClock();
        /* set PIB to FH coordinator */
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastChannelFunction, 2);
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastChannelFunction,
                                 2);
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_unicastDwellInterval,
                                 CONFIG_DWELL_TIME);
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_broadcastDwellInterval,
                                 FH_BROADCAST_DWELL_TIME);

        /* Stack broadcast interval is set to half the application broadcast 
         * message generation rate. This prevents the transmit queue from
         * overflowing by transmitting faster than the queue can fill. */
        ApiMac_mlmeSetFhReqUint32(ApiMac_FHAttribute_BCInterval,
                                  (FH_BROADCAST_INTERVAL >> 1));

         /* set up the number of NON-sleep and sleep device
         * the order is important. Need to set up the number of non-sleep first
         */

        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_numNonSleepDevice,
                                 FH_NUM_NON_SLEEPY_HOPPING_NEIGHBORS);
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_numSleepDevice,
                                 FH_NUM_NON_SLEEPY_FIXED_CHANNEL_NEIGHBORS);

        /* set Exclude Channels */
        sizeOfChannelMask = sizeof(configChannelMask)/sizeof(uint8_t);
        if(sizeOfChannelMask > APIMAC_154G_CHANNEL_BITMAP_SIZ)
        {
            sizeOfChannelMask = APIMAC_154G_CHANNEL_BITMAP_SIZ;
        }
        memset(excludeChannels, 0, APIMAC_154G_CHANNEL_BITMAP_SIZ);
        for(idx = 0; idx < sizeOfChannelMask; idx++)
        {
            excludeChannels[idx] = ~configChannelMask[idx];
        }
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_unicastExcludedChannels,
                                 excludeChannels);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_broadcastExcludedChannels,
                                 excludeChannels);
    }
}

/*!
 Cllc task processing.

 Public function defined in cllc.h
 */
void Cllc_process(void)
{
    /* The LLC has an event */
    if(Cllc_events & CLLC_PA_EVT)
    {
        if(CONFIG_FH_ENABLE)
        {
            setTrickleTime(&fhPAtrickleTime,
                           ApiMac_wisunAsyncFrame_advertisement);
        }

        /* Clear the event */
        Util_clearEvent(&Cllc_events, CLLC_PA_EVT);
    }

    /* The LLC has an PC event */
    if(Cllc_events & CLLC_PC_EVT)
    {
        if(CONFIG_FH_ENABLE)
        {
            setTrickleTime(&fhPCtrickleTime, ApiMac_wisunAsyncFrame_config);
        }

        /* Clear the event */
        Util_clearEvent(&Cllc_events, CLLC_PC_EVT);
    }

    /* Process state change event */
    if(Cllc_events & CLLC_STATE_CHANGE_EVT)
    {
        /* Process LLC Event */
        processState(coordInfoBlock.currentCoordState);

        /* Clear the event */
        Util_clearEvent(&Cllc_events, CLLC_STATE_CHANGE_EVT);
    }

    /* Process join permit event */
    if(Cllc_events & CLLC_JOIN_EVT)
    {
        joinPermitExpired();

        /* Clear the event */
        Util_clearEvent(&Cllc_events, CLLC_JOIN_EVT);
    }
}

/*!
 Set PANID

 Public function defined in cllc.h
 */
void Cllc_setFormingPanId(uint16_t panId)
{
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting)
    {
        coordInfoBlock.panID = panId;
    }
}


/*!
 Get PANID

 Public function defined in cllc.h
 */
void Cllc_getFormingPanId(uint16_t *pPanId)
{
    static uint8_t panIdInitialized = 0;
    Llc_netInfo_t netInfo;

    /* If this is the second time that a network has existed,
     * and a reset took place, restore the old value from NV */
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting && panIdInitialized != 0)
    {
        if(Csf_getNetworkInformation(&netInfo))
        {
            *pPanId = netInfo.devInfo.panID;
        }
    }
    else
    {
        /* If this is the first time the collector is setting the panId, use the value that
         * was either compiled or selected through CUI. */
        *pPanId = coordInfoBlock.panID;
        panIdInitialized = 1;
    }

}

/*!
 Set Channel Mask

 Public function defined in cllc.h
 */
void Cllc_setChanMask(uint8_t *_chanMask)
{
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting)
    {
#ifndef __unix__
#if CONFIG_FH_ENABLE
        memcpy(fhChannelMask, _chanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#else
        memcpy((void *)chanMask, _chanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#endif
#else
        memcpy((void *)chanMask, _chanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#endif
    }
}

/*!
 Get Channel Mask

 Public function defined in cllc.h
 */
void Cllc_getChanMask(uint8_t *_chanMask)
{
#ifndef __unix__
#if CONFIG_FH_ENABLE
        memcpy(_chanMask, fhChannelMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#else
        memcpy(_chanMask, chanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#endif
#else
        memcpy(_chanMask, chanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#endif
}

/*!
 Set Async Channel Mask

 Public function defined in cllc.h
 */
void Cllc_setAsyncChanMask(uint8_t *asyncChanMask)
{
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting)
    {
#ifndef __unix__
#if CONFIG_FH_ENABLE
        memcpy(asyncChannelMask, asyncChanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#endif
#else
        if(CONFIG_FH_ENABLE)
        {
            memcpy(asyncChannelMask, asyncChanMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
        }
#endif
    }
}

/*!
 Get Async Channel Mask

 Public function defined in cllc.h
 */
void Cllc_getAsyncChanMask(uint8_t *asyncChanMask)
{
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting)
    {
#ifndef __unix__
#if CONFIG_FH_ENABLE
        memcpy(asyncChanMask, asyncChannelMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
#endif
#else
        if(CONFIG_FH_ENABLE)
        {
            memcpy(asyncChanMask, asyncChannelMask, APIMAC_154G_CHANNEL_BITMAP_SIZ);
        }
#endif
    }
}

#ifdef FEATURE_MAC_SECURITY
/*!
 Set default security key

 Public function defined in cllc.h
 */
void Cllc_setDefaultKey(uint8_t *key)
{
    /* Frame Counter is set to 0 because no packets have been
     * exchanged yet */
    uint32_t frameCounter = 0;
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting)
    {
        /* Copy the key passed in into the key table */
        memcpy((void *)keyTable[0].key, key, APIMAC_KEY_MAX_LEN);
        Cllc_securityInit(frameCounter, key);
    }
}

/*!
 Get default security key

 Public function defined in cllc.h
 */
void Cllc_getDefaultKey(uint8_t *key)
{
    memcpy(key, keyTable[0].key, APIMAC_KEY_MAX_LEN);
}
#endif /* FEATURE_MAC_SECURITY */

/*!
 Set the collector (Full Function Device - FFD) address

 Public function defined in cllc.h
 */
void Cllc_setFfdShortAddr(uint16_t addr)
{
    if(coordInfoBlock.currentCllcState == Cllc_states_initWaiting)
    {
        coordInfoBlock.shortAddr = addr;
    }
}

/*!
 Get the collector (Full Function Device - FFD) address

 Public function defined in cllc.h
 */
void Cllc_getFfdShortAddr(uint16_t *addr)
{
    *addr = coordInfoBlock.shortAddr;
}

/*!
 Get the current PHY ID of the collector device

 Public function defined in cllc.h
 */
uint8_t Cllc_getFreq(void)
{
    return (CONFIG_PHY_ID);
}

/*!
 Get the current channel of the collector device

 Public function defined in cllc.h
 */
uint8_t Cllc_getChan(void)
{
    return coordInfoBlock.channel;
}


/*!
 Get the current state of the sensor device

 Public function defined in cllc.h
 */
uint8_t Cllc_getProvState(void)
{
    return (coordInfoBlock.currentCllcState);
}

/*!
 Start network

 Public function defined in cllc.h
 */
void Cllc_startNetwork(void)
{
    /* update state */
    updateState(Cllc_states_startingCoordinator);

    if(!CONFIG_FH_ENABLE)
    {
        /*
         Start active scan request to determine channel and PAN ID
         for coordinator
         */
        switchState(Cllc_coordStates_scanEnergyDetect);
    }
    else
    {
        uint8_t startChan = 0;
        for(startChan = 0; startChan < APIMAC_154G_MAX_NUM_CHANNEL; startChan++)
        {
            if(CLLC_IS_CHANNEL_MASK_SET(chanMask, startChan))
            {
                coordInfoBlock.channel = startChan;
                break;
            }
        }
        /* start req */
        switchState(Cllc_coordStates_scanEdCnf);
    }
}
/*!
 Restore network

 Public function defined in cllc.h
 */
void Cllc_restoreNetwork(Llc_netInfo_t *pNetworkInfo, uint16_t numDevices,
        Llc_deviceListItem_t *pDevList)
{
    uint16_t i = 0;
    uint16_t subID;

    /* set state */
    updateState(Cllc_states_initRestoringCoordinator);

    coordInfoBlock.panID = pNetworkInfo->devInfo.panID;

    /* Populate network info according to type of network */
    if(pNetworkInfo->fh == true)
    {
        ApiMac_mlmeSetReqArray(ApiMac_attribute_extendedAddress,
                (uint8_t*)(pNetworkInfo->devInfo.extAddress));
    }
    else
    {
        coordInfoBlock.channel = pNetworkInfo->channel;
    }

    ApiMac_mlmeSetReqUint16(ApiMac_attribute_shortAddress,
                             pNetworkInfo->devInfo.shortAddress);

    sendStartReq(pNetworkInfo->fh);

    if (pDevList)
    {
        /* repopulate association table */
        for(i = 0; i < numDevices; i++, pDevList++)
        {
            /* Add to association table */
            maintainAssocTable(&pDevList->devInfo, &pDevList->capInfo, 1, 0, 0,
                               CLLC_MAINTAIN_ASSOC_TABLE_ADD_FREE);

            /* Get the address for assigning to new devices */
            if( pDevList->devInfo.shortAddress >= Cllc_devShortAddr)
            {
                Cllc_devShortAddr = pDevList->devInfo.shortAddress + 1;
            }
        }
    }
    else
    {
        Llc_deviceListItem_t item;
        /* repopulate association table */
        for(i = 0; i < numDevices; i++)
        {
            Csf_getDeviceItem(i, &item, &subID);
#ifdef FEATURE_MAC_SECURITY
            /* Add device to security device table */
            Cllc_addSecDevice(item.devInfo.panID,
                              item.devInfo.shortAddress,
                              &item.devInfo.extAddress,
                              item.rxFrameCounter);
#endif /* FEATURE_MAC_SECURITY */
            /* Add to association table */
            maintainAssocTable(&item.devInfo, &item.capInfo, 1, 0, subID,
                               CLLC_MAINTAIN_ASSOC_TABLE_ADD_INDEX);

            /* Get the address for assigning to new devices */
            if( item.devInfo.shortAddress >= Cllc_devShortAddr)
            {
                Cllc_devShortAddr = item.devInfo.shortAddress + 1;
            }

#ifdef FEATURE_SECURE_COMMISSIONING
            {
                /* Mark the devices that need to be re-commissioned */
                Cllc_associated_devices_t *pExistingDevice;
                pExistingDevice = Cllc_findDevice(item.devInfo.shortAddress);
                if(pExistingDevice != NULL)
                {
                    pExistingDevice->reCM_status = SM_RE_CM_REQUIRED;
                    /* Do not update key refresh info here. It should be done when CM is done */
                }
            }
#endif /* FEATURE_SECURE_COMMISSIONING */
        }
    }
}

/*!
 Set Join Permit On or Off

 Public function defined in cllc.h
 */
ApiMac_status_t Cllc_setJoinPermit(uint32_t duration)
{
    uint32_t joinDuration = 0;
    if(duration > 0)
    {
        /*  set join permit */
        ApiMac_mlmeSetReqBool(ApiMac_attribute_associatePermit, true);
        updateState(Cllc_states_joiningAllowed);
        if(duration != CLLC_JOIN_PERMIT_ON)
        {
            /* set timer for duration */
            joinDuration = duration;
        }
    }
    else
    {
        /*  set join permit */
        ApiMac_mlmeSetReqBool(ApiMac_attribute_associatePermit, false);
        updateState(Cllc_states_joiningNotAllowed);
    }

    Csf_setJoinPermitClock(joinDuration);
    return (ApiMac_status_success);
}

/*!
 Remove Device

 Public function defined in cllc.h
 */
void Cllc_removeDevice(ApiMac_sAddrExt_t *pExtAddr)
{
    uint8_t i = 0;
    uint16_t shortAddr = Csf_getDeviceShort(pExtAddr);

    if(shortAddr != CSF_INVALID_SHORT_ADDR)
    {
        for(i = 0; i < CONFIG_MAX_DEVICES; i++)
        {
            if(Cllc_associatedDevList[i].shortAddr == shortAddr)
            {
#ifdef FEATURE_MAC_SECURITY
                /* Delete the device from the key table */
                ApiMac_secDeleteDevice(pExtAddr);
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
                if (SM_Current_State == SM_CM_InProgress)
                {
                   SM_stopCMProcess();
                   return;
                }
                else
                {
                  SM_removeEntryFromSeedKeyTable(pExtAddr);
                }
#endif /* FEATURE_SECURE_COMMISSIONING */
                /* Clear the entry - delete */
                memset(&Cllc_associatedDevList[i], 0xFF,
                       sizeof(Cllc_associated_devices_t));
                /* remove from NV */
                Csf_removeDeviceListItem(pExtAddr);

                /* update CUI */
                #ifndef __unix__
                Csf_deviceDisassocUpdate(shortAddr);
                #else
                ApiMac_sAddr_t sAddr;
                sAddr.addr.shortAddr = shortAddr;
                sAddr.addrMode = ApiMac_addrType_short;

                Csf_deviceDisassocUpdate(&sAddr);
                #endif
                /* The corresponding device is removed, return from the function call */
                return;
            }
        }
    }
}

/*!
 Send disassociation request.

 Public function defined in cllc.h
 */
void Cllc_sendDisassociationRequest(uint16_t shortAddr,bool rxOnIdle)
{
    ApiMac_mlmeDisassociateReq_t disassocReq;
    memset(&disassocReq, 0, sizeof(ApiMac_mlmeDisassociateReq_t));
    disassocReq.deviceAddress.addrMode = ApiMac_addrType_short;
    disassocReq.deviceAddress.addr.shortAddr = shortAddr;
    disassocReq.devicePanId = coordInfoBlock.panID;
    disassocReq.disassociateReason = ApiMac_disassocateReason_coord;
    if(rxOnIdle == false)
    {   /* Sleep device */
        disassocReq.txIndirect = true;
    }
    else
    {   /* Non-sleep device */
        disassocReq.txIndirect = false;
    }
    ApiMac_mlmeDisassociateReq(&disassocReq);
}

#ifdef FEATURE_MAC_SECURITY
/*!
 Initialize the MAC Security

 Public function defined in cllc.h
 If a key is provided, use it to initialize the security module.
 Otherwise, use the one in the lookup table and pass in NULL for the key.
 */
void Cllc_securityInit(uint32_t frameCounter, uint8_t *key)
{
    if(macSecurity == true)
    {
        ApiMac_secAddKeyInitFrameCounter_t secInfo;
        if(key != NULL)
        {
            /* Copy the updated key into the table if one is provided */
            memcpy((void *)keyTable[0].key, key, APIMAC_KEY_MAX_LEN);
        }


#if defined(S_NS_154_ENABLED) && !defined(MAC_LEVEL_SKS)
        //convert key in keytable to a keyid object
        KeyStore_PSA_KeyFileId keyID;
        KeyStore_PSA_KeyAttributes attributes = KEYSTORE_PSA_KEY_ATTRIBUTES_INIT;
        uint32_t id;

        // setup attributes for importing key
        KeyStore_PSA_setKeyUsageFlags(&attributes, (KEYSTORE_PSA_KEY_USAGE_DECRYPT | KEYSTORE_PSA_KEY_USAGE_ENCRYPT | KEYSTORE_PSA_KEY_USAGE_EXPORT ));
        KeyStore_PSA_setKeyAlgorithm(&attributes, KEYSTORE_PSA_ALG_CCM);
        KeyStore_PSA_setKeyType(&attributes, KEYSTORE_PSA_KEY_TYPE_AES);
#if defined(PERSISTENT_KEY)
        KeyStore_PSA_setKeyLifetime(&attributes, KEYSTORE_PSA_KEY_LIFETIME_PERSISTENT);

        // set the keyID to something in the persistent key range (0x41 - 0x1fffffff)
        GET_KEY_ID(keyID, KEYSTORE_PSA_KEY_ID_PERSISTENT_USER_MIN);
        KeyStore_PSA_setKeyId(&attributes, keyID);
#else
        // keyID should be set to 0 to grab the next available volatile key
        GET_KEY_ID(keyID, 0);
#endif
        int_fast16_t cryptoStatus;
        // import the key
        cryptoStatus = KeyStore_PSA_importKey(&attributes, (uint8_t *) &(keyTable[0].key), (size_t)APIMAC_KEY_MAX_LEN, &keyID);

        if (cryptoStatus != KEYSTORE_PSA_STATUS_SUCCESS)
        {
#if defined(PERSISTENT_KEY)
            // If we have reset the device at any point, the key will already be there
            if (cryptoStatus != KEYSTORE_PSA_STATUS_ALREADY_EXISTS)
            {
                /*
                 * We can further check the key contents to see if it's the right key,
                 * but for now assume that if a key is present at that id then it's
                 * the same one we used last time.
                 */
                return;
            }
#else
            return;
#endif
        }

        //convert the keyID into array
        SET_KEY_ID(id, keyID);

        //clear the key in the key table
        memset((void *)keyTable[0].key, 0, APIMAC_KEY_MAX_LEN);

        //copy the key ID into the table
        memcpy((void *)keyTable[0].key, &id, sizeof(id));

        //~~~~
#endif

        /* Set the key based on whatever is stored in the key table */
        memcpy(secInfo.key, keyTable[0].key, APIMAC_KEY_MAX_LEN);

        secInfo.frameCounter = frameCounter;
        secInfo.replaceKeyIndex = 0;
        secInfo.newKeyFlag = true;
        secInfo.lookupDataSize = APIMAC_KEY_LOOKUP_LONG_LEN;
        memcpy(secInfo.lookupData, keyIdLookupList[0].lookupData,
               (APIMAC_MAX_KEY_LOOKUP_LEN));
#ifdef FEATURE_SECURE_COMMISSIONING
        secInfo.networkKey = true;
#endif /* FEATURE_SECURE_COMMISSIONING */
#if defined(S_NS_154_ENABLED) && defined(MAC_LEVEL_SKS)
        /* KeyStore Operations may have return statuses that  */
        uint8_t status = ApiMac_secAddKeyInitFrameCounter(&secInfo);
        if (status != ApiMac_status_success)
        {
            // assert here. probably an issue with key storage being out of memory
            MAC_ASSERT(status == ApiMac_status_success);
        }
#else
        ApiMac_secAddKeyInitFrameCounter(&secInfo);
#endif
        ApiMac_mlmeSetSecurityReqArray(
                        ApiMac_securityAttribute_defaultKeySource,
                        (void *) Cllc_keySource);
        ApiMac_mlmeSetSecurityReqStruct(ApiMac_securityAttribute_keyTable,
                                        (void *) NULL);

        /* Set the number of security keys */
        ApiMac_mlmeSetSecurityReqUint16(ApiMac_securityAttribute_keyTableEntries, KEY_TABLE_ENTRIES);

        /* Write a security level entry to PIB */
        ApiMac_mlmeSetSecurityReqStruct(
                        ApiMac_securityAttribute_securityLevelEntry,
                        (void *)&securityLevelEntry);

        /* Set the MAC security */
        ApiMac_mlmeSetReqBool(ApiMac_attribute_securityEnabled, macSecurity);

#if defined(S_NS_154_ENABLED) && defined(MAC_LEVEL_SKS)
        /* Erase the key on the application layer */
        memset((void *)keyTable[0].key, 0, APIMAC_KEY_MAX_LEN);
#endif
    }
}

/*!
 Fill in the security structure

 Public function defined in cllc.h
 */
void Cllc_securityFill(ApiMac_sec_t *pSec)
{
    if(pSec)
    {
        if(macSecurity == true)
        {
            memcpy(pSec->keySource, keyIdLookupList[0].lookupData,
                   (APIMAC_KEY_SOURCE_MAX_LEN));
            pSec->securityLevel = secLevel;
            pSec->keyIdMode = secKeyIdMode;
            pSec->keyIndex = secKeyIndex;
        }
    }
}

/*!
 Check the security level against expected level

 Public function defined in cllc.h
 */
bool Cllc_securityCheck(ApiMac_sec_t *pSec)
{
    bool ret = false;

    if(macSecurity == true)
    {
        if(pSec)
        {
            if(pSec->securityLevel == secLevel)
            {
                ret = true;
            }
        }
    }
    else
    {
       ret = true;
    }

    return(ret);
}


/*!
 Add a device to the MAC security device table.

 Public function defined in cllc.h
 */
ApiMac_status_t Cllc_addSecDevice(uint16_t panID, uint16_t shortAddr,
                         ApiMac_sAddrExt_t *pExtAddr, uint32_t frameCounter)
{
    if(macSecurity == true)
    {
        ApiMac_secAddDevice_t device;
#ifndef FEATURE_SECURE_COMMISSIONING
        uint8_t keyIndex = 0;
#endif /* FEATURE_SECURE_COMMISSIONING */
        device.panID = panID;
        device.shortAddr = shortAddr;
        memcpy(device.extAddr, pExtAddr, sizeof(ApiMac_sAddrExt_t));
        device.frameCounter = frameCounter;
        device.exempt = false;
#ifndef FEATURE_SECURE_COMMISSIONING
        /* get the key lookup information from the initial loaded key */
        device.keyIdLookupDataSize = keyIdLookupList[keyIndex].lookupDataSize;
        memcpy(device.keyIdLookupData, keyIdLookupList[keyIndex].lookupData,
               (APIMAC_MAX_KEY_LOOKUP_LEN));

        device.uniqueDevice = false;
        device.duplicateDevFlag = false;
#endif /* !FEATURE_SECURE_COMMISSIONING */
        return(ApiMac_secAddDevice(&device));
    }
    else
    {
        return(ApiMac_status_success);
    }
}
#endif /* FEATURE_MAC_SECURITY */


/*!
 * @brief      Find the associated device table entry matching an
 *             short address.
 *
 * @param      shortAddr - device's short address
 *
 * @return     pointer to the associated device table entry,
 *             NULL if not found.
 */
Cllc_associated_devices_t *Cllc_findDevice(uint16_t shortAddr)
{
    int x;

    for(x = 0; (x < CONFIG_MAX_DEVICES); x++)
    {
        /* Make sure the entry is valid. */
        if(shortAddr == Cllc_associatedDevList[x].shortAddr)
        {
            return (&Cllc_associatedDevList[x]);
        }
    }
    return (NULL);
}

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief       Switch to the new  coordinator state and set the event bit.
 *
 * @param       newState - next state of coordinator
 */
static void switchState(Cllc_coord_states_t newState)
{
    coordInfoBlock.currentCoordState = newState;
    Util_setEvent(&Cllc_events, CLLC_STATE_CHANGE_EVT);
}

/*!
 * @brief       Function to transition various states involved with scan request
 *               and start request before the coordinator is started.
 *
 * @param       state - current startup state of coordinator
 */
static void processState(Cllc_coord_states_t state)
{

    switch(state)
    {
        case Cllc_coordStates_scanActive:
            if(!CONFIG_FH_ENABLE)
            {
                /* Active scan */
                sendScanReq(ApiMac_scantype_active);
            }
            break;

        case Cllc_coordStates_scanEnergyDetect:
            if(!CONFIG_FH_ENABLE)
            {
                /* Energy Detect scan */
                sendScanReq(ApiMac_scantype_energyDetect);
            }
            break;

        case Cllc_coordStates_scanActiveCnf:
        case Cllc_coordStates_scanEdCnf:
            /* Do not re-send start request for Scan if network has started */
            if(coordInfoBlock.currentCllcState < Cllc_states_started)
            {
                if(!CONFIG_FH_ENABLE)
                {
                    /* check for duplicate PAN ID */
                    configureStartParam(coordInfoBlock.channel);

                    /* setup short address */
                    ApiMac_mlmeSetReqUint16(ApiMac_attribute_shortAddress,
                                            coordInfoBlock.shortAddr);
                }

                sendStartReq(CONFIG_FH_ENABLE);
            }
            break;

        case Cllc_coordStates_startCnf:
        {
            Llc_netInfo_t networkInfo = {0};

            /*
             device has started hence we do not need the
             saved PAN descriptors
             */
            clearPANList();

            networkInfo.fh = CONFIG_FH_ENABLE;
            /* Setup basics */
            ApiMac_mlmeGetReqUint8(ApiMac_attribute_logicalChannel,
                                   &networkInfo.channel);
            ApiMac_mlmeGetReqUint16(ApiMac_attribute_panId,
                                    &networkInfo.devInfo.panID);
            ApiMac_mlmeGetReqArray(ApiMac_attribute_extendedAddress,
                                   (uint8_t*)&networkInfo.devInfo.extAddress);
            ApiMac_mlmeGetReqUint16(ApiMac_attribute_shortAddress,
                                    &networkInfo.devInfo.shortAddress);

            if(CONFIG_FH_ENABLE)
            {
                ApiMac_startFH();
                /* start trickle timer for PA */
                Csf_setTrickleClock(CLLC_FH_MIN_TRICKLE,
                                    ApiMac_wisunAsyncFrame_advertisement);
                /* start trickle timer for PC */
                Csf_setTrickleClock(CLLC_FH_MIN_TRICKLE + (CLLC_FH_MIN_TRICKLE/2),
                                    ApiMac_wisunAsyncFrame_config);
            }

            /* Inform the application of a start */
            if(pCllcCallbacksCopy && pCllcCallbacksCopy->pStartedCb)
            {
                pCllcCallbacksCopy->pStartedCb(&networkInfo);
            }

            /*  coordinator started , callback for start indication*/
            if(coordInfoBlock.currentCllcState ==
                            Cllc_states_initRestoringCoordinator)
            {
                updateState(Cllc_states_restored);
            }
            else
            {
                updateState(Cllc_states_started);
            }
        }
            break;

        default:
            break;
    }
}

/*!
 * @brief       Process  Beacon Notification callback.
 *
 * @param       pData - pointer MAC Beacon indication info
 */
static void beaconNotifyIndCb(ApiMac_mlmeBeaconNotifyInd_t *pData)
{
    /* check beacon type */
    if(pData->beaconType == ApiMac_beaconType_normal)
    {
        /* add received PAN descriptor to list */
        addToPANList(&pData->panDesc);
    }

    /* Callback to MAC API */
    if(macCallbacksCopy.pBeaconNotifyIndCb != NULL)
    {
        macCallbacksCopy.pBeaconNotifyIndCb(pData);
    }
}

/*!
 * @brief       Determine parameters to be used to send Start request from
 *              results of energy scan and active scan
 *
 * @param       channel - best channel to start the coordinator
 */
static void configureStartParam(uint8_t channel)
{
    /* parse incoming beacons to search if another
     *  coordinator exists with this PAN ID */
    while(coordInfoBlock.panID == (findChannel(coordInfoBlock.panID, channel)))
    {
        /* another coordinator with this PAN ID exists,
         *  so we change our PAN ID*/
        coordInfoBlock.panID = coordInfoBlock.panID + 1;
    }
}

/*!
 * @brief       Process  Scan Confirm  callback.
 *
 * @param       pData - pointer to Scan Confirm
 */
static void scanCnfCb(ApiMac_mlmeScanCnf_t *pData)
{
    if((pData->status == ApiMac_status_success) || (pData->status
                      == ApiMac_status_noBeacon))
    {
        if(pData->scanType == ApiMac_scantype_energyDetect)
        {
            switchState(Cllc_coordStates_scanActive);
        }
        else if(pData->scanType == ApiMac_scantype_active)
        {
            /* Do not update coordinator channel if network has already started */
            if(coordInfoBlock.currentCllcState < Cllc_states_started)
            {
                // If we have a valid results list, find the best channel
                if (pData->resultListSize && pData->result.pEnergyDetect)
                {
                    coordInfoBlock.channel
                    = findBestChannel(pData->result.pEnergyDetect);
                }
                // otherwise pick the first valid channel
                else
                {
                    for(int chan = 0; chan < APIMAC_154G_MAX_NUM_CHANNEL; chan++)
                    {
                        if(CLLC_IS_CHANNEL_MASK_SET(chanMask, chan))
                        {
                            coordInfoBlock.channel = chan;
                            break;
                        }
                    }
                }
            }

            switchState(Cllc_coordStates_scanActiveCnf);
        }
    }
    else
    {
        switchState(Cllc_coordStates_scanEnergyDetect);
    }

    if(macCallbacksCopy.pScanCnfCb != NULL)
    {
        macCallbacksCopy.pScanCnfCb(pData);
    }
}

/*!
 * @brief       Determine channels set in the channel mask
 *
 * @param       pData - Pointer to MAC start confirm
 */
static void startCnfCb(ApiMac_mlmeStartCnf_t *pData)
{
    if(pData->status == ApiMac_status_success)
    {
#ifndef __unix__
#ifdef FREQ_2_4G
        ApiMac_srcMatchEnable();
#endif
#else
        if(CONFIG_PHY_ID == APIMAC_250KBPS_IEEE_PHY_0)
        {
            ApiMac_srcMatchEnable();
        }
#endif
        switchState(Cllc_coordStates_startCnf);
    }
    else
    {
        switchState(Cllc_coordStates_scanEnergyDetect);
    }

    if(macCallbacksCopy.pStartCnfCb)
    {
        macCallbacksCopy.pStartCnfCb(pData);
    }
}

/*!
 * @brief       Parse the energy scan results to determine the best channel
 *              for the coordinator.
 *
 *              The results of the energy detect scan represent the energy
 *              measurements on each provided channel after normalizing and
 *              scaling the RF power level. A measured RSSI value is converted
 *              to a score between 0 and 255, using the minimum measured RSSI
 *              (-90 dBm) and saturation energy (-5 dBm) values and the
 *              formula: ED = (255 * (RSSI + 90))/85. A lower energy detect
 *              measurement represents a more suitable channel.
 *
 * @param       pResults -  pointer to energy scan results
 */
static uint8_t findBestChannel(uint8_t *pResults)
{
    uint8_t chan;
    uint8_t currEnergy;
    uint8_t lastEnergy = CLLC_MAX_ENERGY;
    uint8_t coordStartChan = 0;

    for(chan = 0; chan < APIMAC_154G_MAX_NUM_CHANNEL; chan++)
    {
        if(CLLC_IS_CHANNEL_MASK_SET(chanMask, chan))
        {
            currEnergy = pResults[chan];
            if(currEnergy < lastEnergy)
            {
                coordStartChan = chan;
                lastEnergy = currEnergy;
            }
        }
    }
    return (coordStartChan);
}

/*!
 * @brief       Add PAN descriptor to linked list
 *
 * @param       pData - pointer to PAN descriptor
 */
static void addToPANList(ApiMac_panDesc_t *pData)
{
    /* check for duplicate PAN ID before adding */
    if(findInList(pData) == false)
    {
        panDescList_t *pItem = Csf_malloc(sizeof(panDescList_t));
        if(pItem)
        {
            panDescList_t *pList = pPANDesclist;

            memcpy(&(pItem->panDescList), pData, sizeof(ApiMac_panDesc_t));
            pItem->pNext = NULL;

            if(pList == NULL)
            {
                /* list is null initially, add first entry */
                pPANDesclist = pItem;
            }
            else
            {
                /* find the end */
                while(pList->pNext != NULL)
                {
                    pList = pList->pNext;
                }

                /* add new item to the end of the list */
                pList->pNext = pItem;
            }
        }
    }
}

/*!
 * @brief       Remove all entries in PAN decriptor linked list
 */
static void clearPANList(void)
{
    panDescList_t* pCurrent = pPANDesclist;
    panDescList_t* pNext;

    while(pCurrent != NULL)
    {
        pNext = pCurrent->pNext;
        Csf_free(pCurrent);
        pCurrent = pNext;
    }
    pPANDesclist = NULL;
}

/*!
 * @brief       Check for duplicate PAN descriptor
 *
 * @param       pData - pointer to PAN descriptor structure
 *
 * @return      true if found, else false
 */
static bool findInList(ApiMac_panDesc_t *pData)
{
    /* search in list for duplicates */
    panDescList_t *pItem = pPANDesclist;
    while(pItem != NULL)
    {
        if((pItem->panDescList.logicalChannel == pData->logicalChannel)
           && (pItem->panDescList.coordPanId == pData->coordPanId))
        {
            return (true);
        }
        else
        {
            pItem = pItem->pNext;
        }
    }

    return (false);
}

/*!
 * @brief       Find the PAN descriptor corresponding to a given channel
 *              in the PAN descriptor list
 *
 * @param       panID - pan ID to search
 * @param       channel - channel to search
 *
 * @return      PAN ID for that channel
 */
static uint16_t findChannel(uint16_t panID, uint8_t channel)
{
    /* search in PAN descriptors list for that channel */
    panDescList_t* pItem = pPANDesclist;

    while(pItem != NULL)
    {
        if(pItem->panDescList.logicalChannel == channel &&
                    pItem->panDescList.coordPanId == panID)
        {
           return (pItem->panDescList.coordPanId);
        }

        pItem = pItem->pNext;
    }
    return (CLLC_PAN_NOT_FOUND);
}

/*!
 * @brief       Handle Cllc associate indication callback
 *
 * @param       pData - pointer to Associate Indication structure
 * Set the correct status of the device to determine whether or not it can join.
 */
static void assocIndCb(ApiMac_mlmeAssociateInd_t *pData)
{
    /* Device joining callback */
    ApiMac_deviceDescriptor_t devInfo;
    ApiMac_mlmeAssociateRsp_t assocRsp;

#ifdef FEATURE_SECURE_COMMISSIONING
    uint32_t duration=0;
    /* Disable Join Permit during CM process */
    Cllc_setJoinPermit(duration);
#endif /* FEATURE_SECURE_COMMISSIONING */

    /* Setup the device information structure */
    Util_copyExtAddr(&devInfo.extAddress, &pData->deviceAddress);
    devInfo.panID = coordInfoBlock.panID;

#ifndef NV_RESTORE
    /* If NV_RESTORE is disabled device gets a new short address every time it
       joins i.e there may be duplicate entries in the Association table
     */
    devInfo.shortAddress = CSF_INVALID_SHORT_ADDR;
#else
    /* Check to see if the device exists */
    devInfo.shortAddress = Csf_getDeviceShort(&pData->deviceAddress);
#endif
    if(devInfo.shortAddress == CSF_INVALID_SHORT_ADDR)
    {
        /* New device, make a new short address */
        assocRsp.status = ApiMac_assocStatus_panAccessDenied;

        devInfo.shortAddress = Cllc_devShortAddr;
        Cllc_devShortAddr++;

        if(pCllcCallbacksCopy && pCllcCallbacksCopy->pDeviceJoiningCb)
        {
            /* callback for device joining */
            assocRsp.status = pCllcCallbacksCopy->pDeviceJoiningCb(&devInfo,
                                                 &pData->capabilityInformation);
        }
#ifndef NV_RESTORE
        if(Cllc_numOfDevices == CONFIG_MAX_DEVICES)
        {
            assocRsp.status = ApiMac_assocStatus_panAtCapacity;
        }
#endif
        if(assocRsp.status == ApiMac_assocStatus_success)
        {
            /* add to association table */
            maintainAssocTable(&devInfo, &pData->capabilityInformation, 1, 0, 0,
                               CLLC_MAINTAIN_ASSOC_TABLE_ADD_FREE);
#ifdef POWER_MEAS
            if(POWER_TEST_PROFILE == POLL_DATA)
            {
                generateIndirectRampMsg();
            }
#endif
        }
    }
    else
    {
        /* Device already exists use the old short address */
        assocRsp.status = ApiMac_assocStatus_success;
        /* callback for device joining */
        if(pCllcCallbacksCopy && pCllcCallbacksCopy->pDeviceJoiningCb)
        {
            assocRsp.status = pCllcCallbacksCopy->pDeviceJoiningCb(&devInfo, \
                                              &pData->capabilityInformation);
        }
    }

#ifdef FEATURE_SECURE_COMMISSIONING
    /* Keep local copy of info from sensor in Assoc Req: to be used in commissioning */
    OsalPort_memcpy(&assocDevInfo, &devInfo, sizeof(ApiMac_deviceDescriptor_t));
    assocDevRxOnIdle = pData->capabilityInformation.rxOnWhenIdle;
#endif /* FEATURE_SECURE_COMMISSIONING */

    /* Fill assoc rsp fields */
    memset(&assocRsp.sec, 0, sizeof(ApiMac_sec_t));
    Util_copyExtAddr(&assocRsp.deviceAddress, &devInfo.extAddress);
    assocRsp.assocShortAddress = devInfo.shortAddress;

    /* Send response back to the device */
    ApiMac_mlmeAssociateRsp(&assocRsp);

    if(macCallbacksCopy.pAssocIndCb != NULL)
    {
        /* pass back to MAC API */
        macCallbacksCopy.pAssocIndCb(pData);
    }
}

/*!
 * @brief       Handle Disassociate indication callback
 *
 * @param       pData - pointer to disassociate indication structure
 */
static void disassocCnfCb(ApiMac_mlmeDisassociateCnf_t *pData)
{

    ApiMac_sAddrExt_t extAddr;

    // get ext address from short address
    Csf_getDeviceExtAdd(pData->deviceAddress.addr.shortAddr, &extAddr);

    /* remove device from association table */
    Cllc_removeDevice(&extAddr);

    if(macCallbacksCopy.pDisassociateCnfCb)
    {
        macCallbacksCopy.pDisassociateCnfCb(pData);
    }
}

/*!
 * @brief       Handle Disassociate indication callback
 *
 * @param       pData - pointer to disassociate indication structure
 */
static void disassocIndCb(ApiMac_mlmeDisassociateInd_t *pData)
{
    if(pData->disassociateReason == ApiMac_disassocateReason_device)
    {
        /* remove device from association table */
        Cllc_removeDevice(&pData->deviceAddress);
    }

    if(macCallbacksCopy.pDisassociateIndCb)
    {
        macCallbacksCopy.pDisassociateIndCb(pData);
    }
}

/*!
 * @brief       Function to add or update a device in association table
 *
 * @param       pDevInfo - pointer to device descriptor information structure
 * @param       pCapInfo - pointer to capability information of the device
 * @param       rssi     - RSSI value
 * @param       status   - status to indicate if the device is still
 *                         communicating with the coordinator
 *              index    - Used by CLLC_MAINTAIN_ASSOC_TABLE_ADD_INDEX mode to
 *                         indicate index to add device to in association table
 * @param       mode     - One of three modes of operation for the function:
 *                         CLLC_MAINTAIN_ASSOC_TABLE_ADD_FREE:
 *                             Add to next free table entry
 *                         CLLC_MAINTAIN_ASSOC_TABLE_ADD_INDEX:
 *                             Add to table entry at value indicated by index.
 *                             Only add when table entry is empty
 *                         CLLC_MAINTAIN_ASSOC_TABLE_UPDATE
 *                             Update entry in table
 */
static void maintainAssocTable(ApiMac_deviceDescriptor_t *pDevInfo,
                               ApiMac_capabilityInfo_t *pCapInfo,
                               int8_t rssi,
                               uint16_t status,
                               uint16_t index,
                               uint8_t mode)
{
    if(CLLC_MAINTAIN_ASSOC_TABLE_ADD_FREE == mode ||
       CLLC_MAINTAIN_ASSOC_TABLE_ADD_INDEX == mode)
    {
        Cllc_associated_devices_t *pItem = NULL;

        /* look for an empty slot */
        if (CLLC_MAINTAIN_ASSOC_TABLE_ADD_FREE == mode)
        {
            pItem = Cllc_findDevice(CSF_INVALID_SHORT_ADDR);
        }
        else // CLLC_MAINTAIN_ASSOC_TABLE_ADD_INDEX
        {
            if (index < CONFIG_MAX_DEVICES &&
                CSF_INVALID_SHORT_ADDR == Cllc_associatedDevList[index].shortAddr)
            {
                pItem = &Cllc_associatedDevList[index];
            }
        }

        if(pItem != NULL)
        {
            /* table is not full yet */
            /* increment the number of devices */
            Cllc_numOfDevices++;

            /* insert one of the blank spaces in the table */
            pItem->shortAddr = pDevInfo->shortAddress;
            memcpy(&pItem->capInfo, pCapInfo, sizeof(ApiMac_capabilityInfo_t));
            pItem->rssi = rssi;
            pItem->status = status;
        }
    }
    else if(mode == CLLC_MAINTAIN_ASSOC_TABLE_UPDATE)
    {
        uint16_t shortAddr = Csf_getDeviceShort(&pDevInfo->extAddress);
        if(shortAddr != CSF_INVALID_SHORT_ADDR)
        {
            static Cllc_associated_devices_t *pItem;

            pItem = Cllc_findDevice(shortAddr);
            if(pItem != NULL)
            {
                pItem->rssi = rssi;
                pItem->status = status;
            }
        }
    }
}

/*!
 * @brief       callback for Async indication
 *
 * @param       pData - pointer to Async indication structure
 */
static void wsAsyncIndCb(ApiMac_mlmeWsAsyncInd_t *pData)
{
    ApiMac_status_t status;
    ApiMac_payloadIeRec_t *pPayloadGroupRec = NULL;
    uint8_t netname[32];
    bool netNameIEFound = false;

    /* Parse group IEs */
    status = ApiMac_parsePayloadGroupIEs(pData->pPayloadIE, pData->payloadIeLen,
                                         &pPayloadGroupRec);

    if((status == ApiMac_status_success) && (pPayloadGroupRec != NULL))
    {
        ApiMac_payloadIeRec_t *pGroup = pPayloadGroupRec;

        while(pGroup != NULL)
        {
            if(pGroup->item.ieId == ApiMac_payloadIEGroup_WiSUN)
            {
                ApiMac_payloadIeRec_t *pPayloadSubRec = NULL;

                status = ApiMac_parsePayloadSubIEs(pGroup->item.pIEContent,
                                                   pGroup->item.ieContentLen,
                                                   &pPayloadSubRec);
                if((status == ApiMac_status_success) &&
                                (pPayloadSubRec!= NULL))
                {
                    ApiMac_payloadIeRec_t *pSubGroup = pPayloadSubRec;

                    while(pSubGroup != NULL)
                    {
                        uint8_t *pIEContent = pSubGroup->item.pIEContent;

                        switch(pSubGroup->item.ieId)
                        {
                            case ApiMac_wisunSubIE_netNameIE:
                                if(pSubGroup->item.ieContentLen <=
                                    APIMAC_FH_NET_NAME_SIZE_MAX)
                                {
                                    memset(&netname, 0,
                                           APIMAC_FH_NET_NAME_SIZE_MAX);
                                    memcpy(&netname, pIEContent,
                                           pSubGroup->item.ieContentLen);
                                    netNameIEFound = true;
                                }
                                break;
                            case ApiMac_wisunSubIE_USIE:
                                processIncomingAsyncUSIE(pData->fhFrameType,
                                                         pIEContent);
                                break;
                            default:
                                break;
                        }

                        if(netNameIEFound)
                        {
                            break;
                        }
                        /* move to the next item*/
                        pSubGroup = pSubGroup->pNext;
                    }

                  /* Free the IE List allocated by
                     ApiMac_parsePayloadSubIEs()
                   */
                    ApiMac_freeIEList(pPayloadSubRec);
                }
            }

            /* Move to next item*/
            pGroup = pGroup->pNext;
        }

         /* Free the IE List allocated by ApiMac_parsePayloadGroupIEs() */
        ApiMac_freeIEList(pPayloadGroupRec);
    }

    if((!netNameIEFound) ||
       (memcmp(netname, fhNetname, APIMAC_FH_NET_NAME_SIZE_MAX) != 0))
    {
        /* Drop PAS and PCS from other networks */
        return;
    }

    processIncomingFHframe(pData->fhFrameType);

    if(macCallbacksCopy.pWsAsyncIndCb != NULL)
    {
        macCallbacksCopy.pWsAsyncIndCb(pData);
    }
}

/*!
 * @brief       Process Data indication callback
 *
 * @param       pData - pointer to data indication callback structure
 */

static void dataIndCb(ApiMac_mcpsDataInd_t *pData)
{
    Csf_updateFrameCounter(&pData->srcAddr, pData->frameCntr);

    if(macCallbacksCopy.pDataIndCb != NULL)
    {
        macCallbacksCopy.pDataIndCb(pData);
    }
}

static void commStatusIndCb(ApiMac_mlmeCommStatusInd_t *pCommStatusInd)
{
#ifdef FEATURE_SECURE_COMMISSIONING
    /* Association response also received by sensor - start CM is not already in progress */
    if(pCommStatusInd->reason == ApiMac_commStatusReason_assocRsp)
    {
        if((pCommStatusInd->status == ApiMac_status_success) &&(SM_Current_State != SM_CM_InProgress))
        {
                ApiMac_sec_t devSec;

                /* Obtain MAC level security information */
                Cllc_securityFill(&devSec);

                /* First commissioning attempt for new device */
                SM_cmAttempts = 0;

                /* Kick off commissioning process to obtain security information */
                SM_startCMProcess(&assocDevInfo, &devSec, CONFIG_FH_ENABLE,
                                  assocDevRxOnIdle, SM_type_coordinator, SM_COLLECTOR_AUTH_METHODS);

#ifdef USE_DMM
                RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_STARTING);
#endif /* USE_DMM */

        }
        else /* not a success for assocRsp */
        {
            uint32_t duration = 0xFFFFFFFF;
            /* Enable Join Permit since CM is not going to start */
            Cllc_setJoinPermit(duration);
        }
    }
#endif/* FEATURE_SECURE_COMMISSIONING */

    if(macCallbacksCopy.pCommStatusCb != NULL)
    {
        macCallbacksCopy.pCommStatusCb(pCommStatusInd);
    }
}

#ifdef __unix__
/*!
 * @brief      Process the MAC Reset Indication Callback
 *
 * @param      pResetInd - reset indication
 */
static void resetIndCb(ApiMac_mcpsResetInd_t *pResetInd)
{
  /* Update state for CoP initialization */
  updateState(Cllc_states_initWaiting);

  /* Initialize the MAC */
  ApiMac_init(CONFIG_FH_ENABLE);

  /* Set MAC attributes */
  Csf_restoreMacAttributes();

  /* Start the device */
  Csf_processCoPReset();

  if(macCallbacksCopy.pResetIndCb)
  {
      macCallbacksCopy.pResetIndCb(pResetInd);
  }
}
#endif

/*!
 * @brief       Process Orphan indication callback
 *
 * @param       pData - pointer to orphan indication callback structure
 */
static void orphanIndCb(ApiMac_mlmeOrphanInd_t *pData)
{
    Llc_deviceListItem_t item;
    ApiMac_sAddr_t devAddr;
    devAddr.addrMode = ApiMac_addrType_extended;
    memcpy(&devAddr.addr.extAddr, &pData->orphanAddress,
           sizeof(ApiMac_sAddrExt_t));

    if(Csf_getDevice(&devAddr, &item))
    {
        ApiMac_mlmeOrphanRsp_t orphanRsp;

        /* Send orphan response */
        Util_copyExtAddr(&orphanRsp.orphanAddress, &item.devInfo.extAddress);
        memset(&orphanRsp.sec, 0, sizeof(ApiMac_sec_t));
        orphanRsp.associatedMember = true;
        orphanRsp.shortAddress = item.devInfo.shortAddress;

        ApiMac_mlmeOrphanRsp(&orphanRsp);

        /* Update Assoc Table */
        maintainAssocTable(&item.devInfo, &item.capInfo, 1, 0, 0,
                           CLLC_MAINTAIN_ASSOC_TABLE_UPDATE);
    }

    /* Invoke call back from cllc into application layer/collector if not NULL */
    if(macCallbacksCopy.pOrphanIndCb != NULL)
    {
        macCallbacksCopy.pOrphanIndCb(pData);
    }
}

/*!
 * @brief       Update Cllc state
 *
 * @param       state - new state
 */
static void updateState(Cllc_states_t state)
{
    coordInfoBlock.currentCllcState = state;
    if(pCllcCallbacksCopy && pCllcCallbacksCopy->pStateChangeCb)
    {
        /* state change callback */
        pCllcCallbacksCopy->pStateChangeCb(coordInfoBlock.currentCllcState);
    }
}

/*!
 * @brief       Send Async Frame for trickle timer
 *
 * @param       frameType - type of frame to send
 */
static void sendAsyncReq(uint8_t frameType)
{
    ApiMac_mlmeWSAsyncReq_t asyncReq;
    uint8_t sizeOfChannelMask;
#ifndef __unix__
    uint8_t asyncChannelMask[] = FH_ASYNC_CHANNEL_MASK;
#else
    uint8_t asyncChannelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];

    if(networkStarted == 0)
    {
      return;
    }

    memcpy( asyncChannelMask, linux_FH_ASYNC_CHANNEL_MASK, sizeof(asyncChannelMask));
#endif

    /* set of Exclude Channels */
    sizeOfChannelMask = sizeof(asyncChannelMask)/sizeof(uint8_t);
    if(sizeOfChannelMask > APIMAC_154G_CHANNEL_BITMAP_SIZ)
    {
        sizeOfChannelMask = APIMAC_154G_CHANNEL_BITMAP_SIZ;
    }
    memset(asyncReq.channels, 0, (APIMAC_154G_CHANNEL_BITMAP_SIZ));
    memcpy(asyncReq.channels, asyncChannelMask, sizeOfChannelMask);

    asyncReq.operation = ApiMac_wisunAsycnOperation_start;

    if(frameType ==  ApiMac_wisunAsyncFrame_advertisement)
    {
        if((optAsyncFlag & PA_FIXED_NEIGHBOR_FOUND_MASK)
            &&  !(optAsyncFlag & PA_HOP_NEIGHBOR_FOUND_MASK))
        {
            memset(asyncReq.channels, 0, (APIMAC_154G_CHANNEL_BITMAP_SIZ));
            memcpy(asyncReq.channels, optPAChMask,
                   (APIMAC_154G_CHANNEL_BITMAP_SIZ));
        }
        /* clear the optimal PA Async Config */
        memset(optPAChMask, 0, (APIMAC_154G_CHANNEL_BITMAP_SIZ));
        optAsyncFlag &= ~PA_FIXED_NEIGHBOR_FOUND_MASK;
        optAsyncFlag &= ~PA_HOP_NEIGHBOR_FOUND_MASK;
    }
    else if(frameType ==  ApiMac_wisunAsyncFrame_config)
    {
        if((optAsyncFlag & PC_FIXED_NEIGHBOR_FOUND_MASK)
           &&  !(optAsyncFlag & PC_HOP_NEIGHBOR_FOUND_MASK))
        {
            memset(asyncReq.channels, 0, (APIMAC_154G_CHANNEL_BITMAP_SIZ));
            memcpy(asyncReq.channels, optPCChMask,
                   (APIMAC_154G_CHANNEL_BITMAP_SIZ));
        }
        /* clear the optimal PC Async Config */
        memset(optPCChMask, 0, (APIMAC_154G_CHANNEL_BITMAP_SIZ));
        optAsyncFlag &= ~PC_FIXED_NEIGHBOR_FOUND_MASK;
        optAsyncFlag &= ~PC_HOP_NEIGHBOR_FOUND_MASK;
    }

    /* no security for PA */
    memset(&asyncReq.sec, 0, sizeof(ApiMac_sec_t));

    /* send PA or PC according to frame type */
    if(frameType == ApiMac_wisunAsyncFrame_advertisement)
    {
        /* set necessary PIB values for PA */
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_routingCost,
                                 (CLLC_FH_ROUTING_COST));
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_routingMethod,
                                 (CLLC_FH_ROUTING_METHOD));
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_eapolReady,
                                 (CLLC_FH_EAPOL_READY));
        ApiMac_mlmeSetFhReqUint8(ApiMac_FHAttribute_fanTPSVersion,
                                 (CLLC_FH_FANTPSVERSION));
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_netName,
                                 (uint8_t*)&fhNetname[0]);
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_panSize,
                                  (CONFIG_MAX_DEVICES));

        /* Fill in the information for async request */
        asyncReq.frameType = ApiMac_wisunAsyncFrame_advertisement;

        Cllc_statistics.fhNumPASent++;

    }
    else if(frameType == ApiMac_wisunAsyncFrame_config)
    {
        /* set necessary PIB values for PA*/
        ApiMac_mlmeSetFhReqUint16(ApiMac_FHAttribute_panVersion,
                                  (CLLC_FH_PANVERSION));
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk0Hash,
                                 (uint8_t*)fhGtkHash0);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk1Hash,
                                 (uint8_t*)fhGtkHash1);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk2Hash,
                                 (uint8_t*)fhGtkHash2);
        ApiMac_mlmeSetFhReqArray(ApiMac_FHAttribute_gtk3Hash,
                                 (uint8_t*)fhGtkHash3);

        /* Fill in the information for async request */
        asyncReq.operation = ApiMac_wisunAsycnOperation_start;
        asyncReq.frameType = ApiMac_wisunAsyncFrame_config;

#ifdef FEATURE_MAC_SECURITY
        /* security for PC */
        Cllc_securityFill(&asyncReq.sec);
#endif /* FEATURE_MAC_SECURITY */

        Cllc_statistics.fhNumPANConfigSent++;

    }
    ApiMac_mlmeWSAsyncReq(&asyncReq);
}

/*!
 * @brief       Handle join permit duration expiry
 */
static void joinPermitExpired(void)
{
    /* set join permit to false */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_associatePermit, false);
    updateState(Cllc_states_joiningNotAllowed);
}

/*!
 * @brief       Send Start Request
 *
 * @param       startFH - true if FH enable else false
 */
static void sendStartReq(bool startFH)
{
    ApiMac_mlmeStartReq_t startReq;
    memset(&startReq, 0, sizeof(ApiMac_mlmeStartReq_t));

    /* read network parameters fill them in start req */
    startReq.startTime = 0;
    startReq.panId = coordInfoBlock.panID;
    startReq.logicalChannel = coordInfoBlock.channel;
    startReq.channelPage = CONFIG_CHANNEL_PAGE;
    startReq.phyID = CONFIG_PHY_ID;
    startReq.beaconOrder = CONFIG_MAC_BEACON_ORDER;
    startReq.superframeOrder = CONFIG_MAC_SUPERFRAME_ORDER;
    startReq.panCoordinator = true;
    startReq.batteryLifeExt = false;
    startReq.coordRealignment = false;
    startReq.realignSec.securityLevel = false;
    startReq.startFH = startFH;
    startReq.mpmParams.offsetTimeSlot = CLLC_OFFSET_TIMESLOT;
    startReq.mpmParams.eBeaconOrder = CLLC_EBEACONORDER;
    startReq.mpmParams.NBPANEBeaconOrder = CLLC_NBPANEBEACONORDER;
    startReq.mpmParams.pIEIDs = NULL;

    /* send Start Req to MAC API */
    ApiMac_mlmeStartReq(&startReq);

#ifdef __unix__
    networkStarted = 1;
#endif
}

/*!
 * @brief       Calculate the number of channels selected in the mask
 *
 * @param       channelMask - channel mask to calculate for
 *
 * @return      Number of channels selected
 */
static uint8_t calculateChannelCount(uint8_t *channelMask)
{
    uint8_t count = 0;

    for(int chan = 0; chan < APIMAC_154G_MAX_NUM_CHANNEL; chan++)
    {
        if(CLLC_IS_CHANNEL_MASK_SET(chanMask, chan))
        {
            count++;
        }
    }

    return count;
}

/*!
 * @brief       Send scan request
 *
 * @param       type - type of scan: active ,passive or orphan
 */
static void sendScanReq(ApiMac_scantype_t type)
{
    ApiMac_mlmeScanReq_t scanReq;
    /* set common parameters for all scans */
    memset(&scanReq, 0, sizeof(ApiMac_mlmeScanReq_t));
    /* set scan channels from channel mask*/
    memcpy(scanReq.scanChannels, chanMask,
    APIMAC_154G_CHANNEL_BITMAP_SIZ);
    scanReq.scanType = type;
    scanReq.scanDuration = CONFIG_SCAN_DURATION;
    scanReq.maxResults = calculateChannelCount(chanMask);/* Expecting beacon notifications */
    scanReq.permitJoining = false;
    scanReq.linkQuality = CONFIG_LINKQUALITY;
    scanReq.percentFilter = CONFIG_PERCENTFILTER;
    scanReq.channelPage = CONFIG_CHANNEL_PAGE;
    /* Should be set to the curent PHY ID*/
    scanReq.phyID = CONFIG_PHY_ID;
    /* using no security for scan request command */
    memset(&scanReq.sec, 0, sizeof(ApiMac_sec_t));
    /* send scan Req */
    ApiMac_mlmeScanReq(&scanReq);
}

/*!
 * @brief       Set Trickle time
 *
 * @param       pTrickleTime - pointer to trickle time duration variable type
 *                            fhPAtrickleTime or fhPCtrickleTime
 * @param       frameType   - type of FH frame to be sent
 */
static void setTrickleTime(uint32_t *pTrickleTime, uint8_t frameType)
{
    if(CONFIG_DOUBLE_TRICKLE_TIMER)
    {
        if((2 * (*pTrickleTime)) < CLLC_FH_MAX_TRICKLE)
        {
            *pTrickleTime = 2 * (*pTrickleTime);
        }
        else if(((2 * (*pTrickleTime)) == CLLC_FH_MAX_TRICKLE) ||
               ((2 * (*pTrickleTime)) > CLLC_FH_MAX_TRICKLE))
        {
            (*pTrickleTime) = CLLC_FH_MAX_TRICKLE;
        }
        /* Reset trickle timer */
        Csf_setTrickleClock((*pTrickleTime),frameType);
    }
    else
    {
        *pTrickleTime = 0;
    }
    /* Process LLC Event */
    sendAsyncReq(frameType);
}

/*!
 * @brief       Process incoming FH frame
 *
 * @param       frameType   - type of FH frame to be sent
 */
static void processIncomingFHframe(uint8_t frameType)
{

    if(frameType == ApiMac_fhFrameType_panAdvertSolicit)
    {
        if(fhPAtrickleTime != CLLC_FH_MIN_TRICKLE)
        {
            fhPAtrickleTime = CLLC_FH_MIN_TRICKLE;
            /* reset trickle timer only if trickleTime not at Min */
            Csf_setTrickleClock(fhPAtrickleTime,
                                ApiMac_wisunAsyncFrame_advertisement);
        }
        /* PAS is received , increment statistics */
        Cllc_statistics.fhNumPASolicitReceived++;
    }
    else if(frameType == ApiMac_fhFrameType_configSolicit)
    {
        if(fhPCtrickleTime != CLLC_FH_MIN_TRICKLE)
        {
            fhPCtrickleTime = CLLC_FH_MIN_TRICKLE;
            /* reset trickle timer only if trickleTime not at Min */
            Csf_setTrickleClock(fhPCtrickleTime,
                                ApiMac_wisunAsyncFrame_config);
        }
        /* PCS is received , increment statistics */
        Cllc_statistics.fhNumPANConfigSolicitsReceived++;
    }
}

/*!
 * @brief       Process incoming Async US IE content
 *
 * @param       pIEContent   - Pointer to USIE Content
 */
static void processIncomingAsyncUSIE(uint8_t frameType, uint8_t* pIEContent)
{
    uint8_t channelInfo = *(pIEContent + 3);
    uint8_t chPlan = channelInfo & 7;
    uint8_t chFn = (channelInfo >> 3) & 7;
    uint16_t fixedChannel = 0xFFFF;
    /* Check if channel function is fixed (sleepy device) */
    if (chFn == 0)
    {
        /* Check if channel plan is based on REG_DOMAIN */
        if (chPlan == 0)
        {
            fixedChannel = Util_parseUint16(pIEContent + USIE_FIXED_CHANNEL_OFFSET_CP0);
        }
        else
        {
            fixedChannel = Util_parseUint16(pIEContent + USIE_FIXED_CHANNEL_OFFSET_CP1);
        }
        if(fixedChannel < APIMAC_154G_MAX_NUM_CHANNEL)
        {
            if(frameType == ApiMac_fhFrameType_panAdvertSolicit)
            {
                optAsyncFlag |= PA_FIXED_NEIGHBOR_FOUND_MASK;
                CLLC_SET_CHANNEL(optPAChMask,fixedChannel);
            }
            else if(frameType == ApiMac_fhFrameType_configSolicit)
            {
                optAsyncFlag |= PC_FIXED_NEIGHBOR_FOUND_MASK;
                CLLC_SET_CHANNEL(optPCChMask,fixedChannel);
            }
        }
    }
    else
    {
        if(frameType == ApiMac_fhFrameType_panAdvertSolicit)
        {
            optAsyncFlag |= PA_HOP_NEIGHBOR_FOUND_MASK;
        }
        else if(frameType == ApiMac_fhFrameType_configSolicit)
        {
            optAsyncFlag |= PC_HOP_NEIGHBOR_FOUND_MASK;
        }
    }
}
