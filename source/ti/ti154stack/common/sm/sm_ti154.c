/******************************************************************************

 @file sm.c

 @brief TI 15.4 Security Manager
 This module implements the Device Commissioning process that includes
 key generation, exchange and network authentication for the application.

 Group: LPRF
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2025, Texas Instruments Incorporated
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
#include "ti_154stack_features.h"

#ifdef FEATURE_SECURE_COMMISSIONING
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#include "mac_util.h"
#include "ti_154stack_config.h"
#include "advanced_config.h"
#include "sm_ti154.h"
#include "sm_ecc_ti154.h"
#include "sm_toolbox.h"
#include "util_timer.h"
#include "mac_api.h"
#include "icall_osal_rom_jt.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif /* CUI_DISABLE */
#include <ti/drivers/dpl/ClockP.h>
#include <semaphore.h>

#include "ti_drivers_config.h"

/* Include if collector for disassociation functions */
#ifdef FEATURE_FULL_FUNCTION_DEVICE
    #include "cllc.h"
    #include "csf.h"
#else
    #include "jdllc.h"
    #include "ssf.h"
#endif

#include "osal_port.h"

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* Uncomment below defines to enable debug messages */
//#define SECURE_MANAGER_DEBUG

#ifdef SECURE_MANAGER_DEBUG
//#define SECURE_MANAGER_DEBUG_ADVANCED
#endif

/* Uncomment below define to access local variables by making them public */
//#define SM_INFO_DEBUG

#ifdef SM_INFO_DEBUG
#define STATIC_SM
#else
/*! Define STATIC_SM as static for local variables */
#define STATIC_SM static
#endif

/******************************************************************************
 Structures
 *****************************************************************************/
 /* Structure used for holding the values from authentication state */
 typedef struct _SM_authVals_Descriptor_t{
    uint8_t Code[SM_AUTH_CODE_LEN];
    uint8_t Nonce[SM_AUTH_NONCE_LEN];
 } SM_authVals_Descriptor_t;

 /* Structure used for holding the security constants from key generation state */
typedef struct _SM_keyConsts_Descriptor_t{
    uint8_t localE[SM_KEY_GEN_DATA_SIZE];
    uint8_t expectedE[SM_KEY_GEN_DATA_SIZE];
    uint8_t receivedE[SM_KEY_GEN_DATA_SIZE];
    uint8_t conKey[APIMAC_KEY_MAX_LEN];
    uint8_t deviceKey[APIMAC_KEY_MAX_LEN];
} SM_keyVals_Descriptor_t;

/******************************************************************************
 Global variables
 *****************************************************************************/
/* Security manager latest state */
SM_lastState_t SM_Current_State = SM_CM_Not_Started;
/* Security manager pending events */
uint16_t SM_events = 0;
/* Seed Key Table List*/
List_List SeedKeyTableList;
/* Copy of sensor device information during commissioning procedure */
ApiMac_deviceDescriptor_t commissionDevInfo;
/* Number of commissioning attempts for current sensor device */
uint8_t SM_cmAttempts = 0;
/* Flag to determine when the SM process was force stopped */
bool SM_forceStopped = false;
/* Flag to tell whether to process success/failure status of previous packet */
bool volatile useSendPktStatus = false;

/* MAC's IEEE address. This is only for Sensor */
extern ApiMac_sAddrExt_t ApiMac_extAddr;
/*assert function*/
extern void Main_assertHandler(uint8_t assertReason);
/*Sensor Address */
uint16_t SM_Sensor_SAddress;

#ifdef FEATURE_FULL_FUNCTION_DEVICE
/*need to re-do commissioning*/
bool fCommissionRequired = false;
/* number of devices to rebuild key info */
uint16_t keyRecoverDeviceNumber =0;
/* time to indicate MAC is ready */
bool readySMToRun= false;
#endif

/* Used to set UART read and write timeout values */
extern const uint32_t Clock_tickPeriod;

/******************************************************************************
 Local variables
 *****************************************************************************/
/* Saved Security manager state */
STATIC_SM SM_states_t currentSMState = SM_states_entry;
/* Keeps track of whether in regular or key refreshment SM process */
STATIC_SM bool keyRefreshmentProcess = false;
/* ID of message waited on */
STATIC_SM SMMsgs_cmdIds_t waitingForMsgID = SMMsgs_noMsg;
/* Holds local and foreign security values */
STATIC_SM SM_keyVals_Descriptor_t keyGenInfo;
STATIC_SM SMMsgs_errorCode_t errorCode;

static ApiMac_sec_t commissionSec;
static bool devRxOnIdle;
static bool devFHEnabled;
static uint8_t localSupportedAuthMethods;   /*bitmask containing values of type SMMsgs_authMethod_t*/
static uint8_t foreignSupportedAuthMethods; /*bitmask containing values of type SMMsgs_authMethod_t*/
static uint8_t agreedUponAuthMethod = 0;
static SM_ECC_securityDeviceDescriptor_t localSecDevice;

/* Actual Passkey used in authentication; Will be updated based of auth method chosen */
static uint32_t SM_authPassKey = 0x0;
static bool awaitingPasskey = false;

/* Variable used to detect timeout during passkey process */
static volatile bool timeoutOccurred = false;

/* Variable to keep track of the iteration number in the iterative authentication process */
static int8_t SM_authItrVal = 0;

/* Code Nonce pairs generated per iteration during authentication */
static SM_authVals_Descriptor_t localAuthCodeNoncePairs[SM_NUM_AUTH_ITERATIONS];
static SM_authVals_Descriptor_t foreignAuthCodeNoncePairs[SM_NUM_AUTH_ITERATIONS];

#ifdef FEATURE_FULL_FUNCTION_DEVICE
static SM_types_t deviceType = SM_type_coordinator;
/* Key refreshment wait time in ms */
static uint32_t keyRefreshTimeout = SM_KEYREFRESH_TIMEOUT * 1000;
/* Timer for key refreshment wait time */
static ClockP_Struct smKeyRefreshTimeoutClkStruct;
static ClockP_Handle smKeyRefreshTimeoutClkHandle;
#else
static SM_types_t deviceType = SM_type_device;
#endif

/*! SM callback table, initialized to NULL */
static SM_callbacks_t *pSMCallbacks = (SM_callbacks_t *) NULL;

/* Timer for commissioning request/response wait time */
static ClockP_Struct smProcessTimeoutClkStruct;
static ClockP_Handle smProcessTimeoutClkHandle;

#ifndef CUI_DISABLE
CUI_clientHandle_t securityCuiHndl;
uint32_t securityStatusLine;
#ifdef SECURE_MANAGER_DEBUG
uint32_t securityDebugStatusLine;
#endif
#ifdef SECURE_MANAGER_DEBUG_ADVANCED
uint32_t securityDebugStatusLine2;
#endif
#endif /* CUI_DISABLE */

/* semaphore in application */
static sem_t *hSMappSem;

/******************************************************************************
 Local Function Prototypes
 *****************************************************************************/
static void switchState(SM_states_t newState);
static void processState(SM_states_t state);
static void initializeCommStatusClock(void);
static void setSMProcessTimeoutClock(uint32_t timeout);
static bool stopSMProcessTimeoutClock(void);
static void processSMProcessTimeoutCallback(uintptr_t a0);

#ifdef FEATURE_FULL_FUNCTION_DEVICE
static void initializeKeyRefreshClock(void);
static void keyRefreshTimeoutCallback(uintptr_t a0);
static void setKeyRefreshTimeoutClock(uint32_t timeout);

static void keyRecoverProcess (void);
static void allDevice_keyRefresh (void);
static Cllc_associated_devices_t *findDevice(uint16_t shortAddr);
#endif
static bool isSupportedAuthType(uint8_t authmethods);
static bool sendSMMsg(uint8_t *pData, uint16_t len, uint8_t msduHandle);
static bool SMSendInfo(Smsgs_cmdIds_t msgID, SMMsgs_cmdIds_t smCmdID, void *pData, uint8_t dataLen, uint8_t msduHandle, bool retryFlag);
static bool isValidState(SM_states_t currentState, SM_states_t nextState);
static void clearSecurityVariables(void);

static void processKeyGenState();
static void startKeyGenProcess(void);
static void SMAuthStateGenCodeAndNonce(uint8_t itrNum, uint8_t curPassKey);
static void SMProcessAuthState();

static void genNewKeyLookUpData4KeyMode3(ApiMac_sAddrExt_t extAddress, uint8_t *lookUp);
static bool genRefreshKeyLookUpData4KeyMode3(uint8_t *lookUp);

#ifdef SECURE_MANAGER_DEBUG
static void printStateString(SM_states_t newState);
static void printAuthMethodString(SMMsgs_authMethod_t authMethod);
static void printCmdString(SMMsgs_cmdIds_t CMMsgId, bool isMsgSent, bool retryFlag);
static void printErrorString(SMMsgs_errorCode_t errorCode);
#endif
/******************************************************************************
 Public Functions
 *****************************************************************************/
/*!
 Initialize this module.

 Public function defined in sm.h
 */
#ifndef CUI_DISABLE
void SM_init(void* sem, CUI_clientHandle_t cuiHndl)
#else
void SM_init(void* sem)
#endif /* CUI_DISABLE */
{
    SM_seedKey_Entry_t *pSeedKeyEnty;

    /* Initialize ECC module */
    SM_ECC_init();

#ifndef USE_DMM
    /* Initialize AESECB module */
    SM_AESECB_init();
#endif /* USE_DMM */

    /* Save off the semaphore */
    hSMappSem = (sem_t*)sem;

#ifndef CUI_DISABLE
    /* Saveoff the CUI handle */
    securityCuiHndl = cuiHndl;

    CUI_statusLineResourceRequest(securityCuiHndl, "Security Status", false, &securityStatusLine);
#ifdef SECURE_MANAGER_DEBUG
    CUI_statusLineResourceRequest(securityCuiHndl, "Security Debug", false, &securityDebugStatusLine);
#endif

#ifdef SECURE_MANAGER_DEBUG_ADVANCED
    CUI_statusLineResourceRequest(securityCuiHndl, "Security Debug 2", false, &securityDebugStatusLine2);
#endif
#endif /* CUI_DISABLE */

    /* Initialize commissioning timeout clock */
    initializeCommStatusClock();
#ifdef FEATURE_FULL_FUNCTION_DEVICE
    /* Initialize key refresh timeout clock */
    initializeKeyRefreshClock();
#endif
    List_clearList(&SeedKeyTableList);

    pSeedKeyEnty = (SM_seedKey_Entry_t *)OsalPort_malloc(sizeof (SM_seedKey_Entry_t));

    if (pSeedKeyEnty) {
        // the first entry is all 0xFF
        memset(pSeedKeyEnty->lookUpData, 0xFF, APIMAC_KEY_SOURCE_MAX_LEN);
        memset(pSeedKeyEnty->extAddress, 0xFF, APIMAC_SADDR_EXT_LEN);
        memset(pSeedKeyEnty->seedKey, 0xFF, SM_ECC_PUBLIC_KEY_SIZE);
        pSeedKeyEnty->index = 0xFF;
        pSeedKeyEnty->shortAddress = 0xFF;

        // put the default key into the head of the table
        List_putHead(&SeedKeyTableList, (List_Elem *)pSeedKeyEnty);

    }
    else {
        // allocation error
        Main_assertHandler(SM_MEMALLOC_ERROR);
    }

#ifdef FEATURE_FULL_FUNCTION_DEVICE
        /* Start the Key Refresh Timer forever from now on */
        setKeyRefreshTimeoutClock(keyRefreshTimeout);
#endif

}

/*!
 SM task processing.

 Public function defined in sm.h
 */
void SM_process(void)
{
    if (SM_events & SM_TIMEOUT_EVT)
    {
        /* Clear the event */
        Util_clearEvent(&SM_events, SM_TIMEOUT_EVT);

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                    "Timeout Occurred !!!");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */

        /* Security establishment failure due to procedure timeout */
        /* Send failure message to other party via SMMsgs_cmdIds_processFail */
        errorCode = SMMsgs_errorCode_timeout;

        SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processFail,
                   &errorCode, sizeof(SMMsgs_errorCode_t), SM_FAIL_MSDU_HANDLE, false);

        if(awaitingPasskey)
        {
            awaitingPasskey = false;
            if(pSMCallbacks->pfnRequestPasskeyCb)
            {
                /*passkey timeout*/
                pSMCallbacks->pfnRequestPasskeyCb(SM_passkeyEntryTimeout);
            }
#ifndef CUI_DISABLE
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine, "Passkey Entry Timeout!");
#endif /* CUI_DISABLE */
        }
    }
    else if (SM_events & SM_SENT_CM_FAIL_EVT)
    {
        /* Clear the event */
        Util_clearEvent(&SM_events, SM_SENT_CM_FAIL_EVT);

        /* Switch to fail state after transmitting the fail message */
        switchState(SM_states_finish_fail);
    }
    /* Handle success/failure status from previous packet sent */
    else if(SM_events & SM_SEND_PKT_FB_EVT)
    {
        /* Clear the event */
        Util_clearEvent(&SM_events, SM_SEND_PKT_FB_EVT);

        /* Stop the timer */
        bool restartTimer = stopSMProcessTimeoutClock();

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                    "Will Retry Sending previous Packet");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */

        /* Resend packet as fail status was seen for previous attempt */
        SMSendInfo((Smsgs_cmdIds_t)0, (SMMsgs_cmdIds_t)0, NULL, (uint8_t)0,
                   (uint8_t)0, true);

        /* Restart the timer if timer had been active. Excludes fail messages */
        if (restartTimer)
        {
            setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);
        }
    }
    else if(SM_events & SM_COLLECTOR_CM_START_EVT)
    {
        /* Goes to the CM start state */
        switchState(SM_states_start);

        /* Clear the event */
        Util_clearEvent(&SM_events, SM_COLLECTOR_CM_START_EVT);
    }

    /* Successful passkey input process */
    else if (SM_events & SM_PASSKEY_EVT)
    {
        /* If passkey entered successfully, trigger main menu */
        if ((pSMCallbacks->pfnRequestPasskeyCb) && (SM_cmAttempts < SM_CM_MAX_RETRY_ATTEMPTS))
        {
            pSMCallbacks->pfnRequestPasskeyCb(SM_passkeyEntered);
        }

        /* clear the current event */
        Util_clearEvent(&SM_events, SM_PASSKEY_EVT);

        /* Notify other party of passkey selection completion */
        SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processPasskeyConfirm, \
                   0, 0, SM_DATA_MSDU_HANDLE, false);

        if (deviceType == SM_type_coordinator)
        {
            /* Set ID of expected reply from other party */
            waitingForMsgID = SMMsgs_cmdIds_processPublicKey;
            /* Start timeout for commissioning response */
            setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);
        }
        else
        {
            /* Set ID of expected reply from other party */
            waitingForMsgID = SMMsgs_cmdIds_processPasskeyConfirm;
            /* Start long timer to account for possible user passkey input process */
            setSMProcessTimeoutClock(SM_PASSKEY_WAIT_TIMEOUT_VALUE);
        }
    }

    else if (SM_events & SM_AUTH_RSP_EVT)
    {
        /* clear the current event */
        Util_clearEvent(&SM_events, SM_AUTH_RSP_EVT);

        /* consider calling the auth sub state machine process*/
        SMProcessAuthState();
    }
    else if(SM_events & SM_KEY_GEN_EVT)
    {
        /* clear the current event */
        Util_clearEvent(&SM_events, SM_KEY_GEN_EVT);

        processKeyGenState();
    }
    else if(SM_events & SM_STATE_CHANGE_EVT)
    {
        /* Clear the event */
        Util_clearEvent(&SM_events, SM_STATE_CHANGE_EVT);

        /* Process new SM state */
        processState(currentSMState);
    }
#ifdef FEATURE_FULL_FUNCTION_DEVICE
    else if(SM_events & SM_KEY_REFRESH_EVT)
    {
        Util_clearEvent(&SM_events, SM_KEY_REFRESH_EVT);
        allDevice_keyRefresh();

        /* start the timer for the next one */
        setKeyRefreshTimeoutClock(keyRefreshTimeout);
    }
    else
    {
        if((readySMToRun == TRUE)&&(fCommissionRequired == TRUE))
        {
            keyRecoverProcess();
        }
    }
#endif
}

/*!
 Start key refreshment process

 Public function defined in sm.h
 */
void SM_startKeyRefreshProcess(ApiMac_deviceDescriptor_t *pDevInfo, ApiMac_sec_t *pSec,
                           bool fhEnabled, bool rxOnIdle)
{

    SM_seedKey_Entry_t *pSeedEntry;

    /* Verify security manager is not processing another device */
    if (currentSMState == SM_states_entry)
    {
#ifndef CUI_DISABLE
        if (SM_type_coordinator == deviceType)
        {
             CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Key Refreshment Started: 0x%04x",
                                 pDevInfo->shortAddress);
         }
        else
        {
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Key Refreshment Started: 0x%04x",
                                 SM_Sensor_SAddress);
        }
#endif /* CUI_DISABLE */

        /* SM in key refreshment process */
        SM_Current_State = SM_CM_InProgress;
        keyRefreshmentProcess = true;

        /* Set state to key gen */
        switchState(SM_states_key_gen);

        /* Store local copy of device information */
        devRxOnIdle = rxOnIdle;
        devFHEnabled = fhEnabled;
        SM_events = 0;

        /* Copy the device information settings */
        OsalPort_memcpy(&commissionDevInfo, pDevInfo, sizeof(ApiMac_deviceDescriptor_t));

        /* Copy the security settings */
        OsalPort_memcpy(&commissionSec, pSec, sizeof(ApiMac_sec_t));

        /* Find seedKey entry */
        if (deviceType == SM_type_coordinator) {
            pSeedEntry = getEntryFromSeedKeyTable (commissionDevInfo.extAddress,commissionDevInfo.shortAddress);
        }
        else {
            pSeedEntry = getEntryFromSeedKeyTable (ApiMac_extAddr, SM_Sensor_SAddress);
        }

        if(pSeedEntry)
        {
            /* Set up for key generation state */
            /* Store seed key in localSecDevice */
            OsalPort_memcpy(localSecDevice.sharedSecretKeyMaterial, pSeedEntry->seedKey, SM_ECC_PUBLIC_KEY_SIZE);

            /* Prepare variables used for key generation */
            memset(localAuthCodeNoncePairs, 0, sizeof(localAuthCodeNoncePairs));
            memset(foreignAuthCodeNoncePairs, 0, sizeof(localAuthCodeNoncePairs));
            memset(&agreedUponAuthMethod, 0, sizeof(agreedUponAuthMethod));
            memset(&SM_authPassKey, 0, sizeof(SM_authPassKey));

            /* Use key lookup data for nonces used in key generation */
            OsalPort_memcpy(localAuthCodeNoncePairs[0].Nonce, pSeedEntry->lookUpData, APIMAC_KEY_SOURCE_MAX_LEN);
            OsalPort_memcpy(foreignAuthCodeNoncePairs[0].Nonce, pSeedEntry->lookUpData, APIMAC_KEY_SOURCE_MAX_LEN);
        }

        /* Both devices wait for the foreign security constants */
        waitingForMsgID = SMMsgs_cmdIds_processInfo128SecurityVal;

        if (SM_type_coordinator == deviceType)
        {
            /* Send key refresh request msg to device */
            SMSendInfo(Smgs_cmdIds_CommissionStart, SMMsgs_cmdIds_keyRefreshRequest, 0, 0, \
                       SM_DATA_MSDU_HANDLE, false);

            /* Start timeout on collector side since sensor is first to send constant */
            setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);
        }
        else
        {
            /* Set ID of expected reply from other party */
            waitingForMsgID = SMMsgs_cmdIds_processInfo128SecurityVal;

            SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processInfo128SecurityVal,
                       keyGenInfo.localE, SM_KEY_GEN_DATA_SIZE, SM_DATA_MSDU_HANDLE, false);

            /* Start timeout for commissioning response */
            setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);
        }
    }
}

/*!
 Function to kick off security manager for a specific device.

 Public function defined in sm.h
 */
void SM_startCMProcess(ApiMac_deviceDescriptor_t *devInfo, ApiMac_sec_t *sec,
    bool fhEnabled, bool rxOnIdle, SM_types_t devType, uint8_t authMethods)
{
    if (deviceType == SM_type_coordinator)
    {
        SM_Sensor_SAddress = devInfo->shortAddress;
    }
    SM_Current_State = SM_CM_InProgress;
    /* Store local copy of device information */
    devRxOnIdle = rxOnIdle;
    devFHEnabled = fhEnabled;
    deviceType = devType;
    SM_events = 0;
    localSupportedAuthMethods = authMethods;
    foreignSupportedAuthMethods = 0;

    /* Copy the device information settings */
    OsalPort_memcpy(&commissionDevInfo, devInfo, sizeof(ApiMac_deviceDescriptor_t));

    /* Copy the security settings */
    OsalPort_memcpy(&commissionSec, sec, sizeof(ApiMac_sec_t));

    /* Initial commissioning process */
    keyRefreshmentProcess = false;

    /* No message being currently waited on */
    waitingForMsgID = SMMsgs_noMsg;

    /* stop timeout clock */
    stopSMProcessTimeoutClock();

    /* Reset timeout monitoring variable */
    timeoutOccurred = false;

    /* clear out error codes from previous run */
    errorCode = SMMsgs_errorCode_none;

    /*clear out security variables used from prev run */
    clearSecurityVariables();

    /* Reset the current SM state */
    currentSMState = SM_states_entry;

    /* clear the flag which tells whether to use the async status of sending a packet */
    useSendPktStatus = false;

    if (deviceType == SM_type_coordinator) {
        /* Generate the starting event for collector */
        Util_setEvent(&SM_events, SM_COLLECTOR_CM_START_EVT);

#ifndef CUI_DISABLE
        CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Commissioning Started: 0x%04x",
                                 devInfo->shortAddress);
#endif /* CUI_DISABLE */
    }
    /* Non-Coordinator device; sensor */
    else
    {
#ifndef CUI_DISABLE
        CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                             "Commissioning Started: 0x%04x",
                                             SM_Sensor_SAddress);
#endif /* CUI_DISABLE */

        /* Device association complete */
        switchState(SM_states_auth_method);
    }
}

/*!
 Function to stop a currently active commissioning process

 Public function defined in sm_ti154.h
 */
void SM_stopCMProcess(void)
{
    if (SM_Current_State == SM_CM_InProgress)
    {
      SM_cmAttempts = SM_CM_MAX_RETRY_ATTEMPTS;
      SM_forceStopped = true;
      switchState(SM_states_finish_fail);
    }
}

/*!
 SM callback registration function.

 Public function defined in sm.h
 */
void SM_registerCallback(SM_callbacks_t *pCallbacks)
{
    pSMCallbacks = pCallbacks;
}

bool SM_removeEntryFromSeedKeyTable (ApiMac_sAddrExt_t *extAddress) {
    /* Load list head as the start point of the iterator. It is always the default key. */
    SM_seedKey_Entry_t* pSeedKey = (SM_seedKey_Entry_t*) List_head(&SeedKeyTableList);

    while(1) {
        /* get the next entry */
        pSeedKey = (SM_seedKey_Entry_t*)List_next((List_Elem *)pSeedKey);
        if (pSeedKey) {
            if (OsalPort_memcmp(pSeedKey->extAddress, extAddress, sizeof(ApiMac_sAddrExt_t)) == TRUE)
                /* found a matching entry */
                break;
            else
                /* move to the next entry */
                continue;
        }
        else {
            /* cannot find the matching ext address */
            return(false);
        }
    }
    /* remove the entry from the table */
    List_remove(&SeedKeyTableList, (List_Elem *)pSeedKey);

    /* free the space */
    OsalPort_free(pSeedKey);

#ifndef CUI_DISABLE
    CUI_statusLinePrintf(securityCuiHndl, securityStatusLine, "Decommissioned");
#endif /* CUI_DISABLE */
    
    return(true);
}

/*!
 SM callback registration function.

 Public function defined in sm.h
 */
void SM_processCommData(ApiMac_mcpsDataInd_t *pDataInd)
{
    uint8_t *pBuf = pDataInd->msdu.p;
    pBuf += sizeof(Smsgs_cmdIds_t);
    bool isSrcAddrCorrect = false;

    SMMsgs_cmdIds_t CMMsgId = (SMMsgs_cmdIds_t)Util_buildUint16(pBuf[0], pBuf[1]);
    pBuf += sizeof(SMMsgs_cmdIds_t);

    if(((pDataInd->srcAddr.addrMode == ApiMac_addrType_short) &&
        (pDataInd->srcAddr.addr.shortAddr == commissionDevInfo.shortAddress))
       || ((pDataInd->srcAddr.addrMode == ApiMac_addrType_extended) &&
           (OsalPort_memcmp(&pDataInd->srcAddr.addr.extAddr[0], &commissionDevInfo.extAddress[0],
                                                APIMAC_SADDR_EXT_LEN) == TRUE)))
    {
        isSrcAddrCorrect = true;
    }

    /* Error Checking */
    if ((currentSMState == SM_states_finish_fail) || (currentSMState == SM_states_entry) ||
        (isSrcAddrCorrect == false))
    {
        return;
    }
    /* Unexpected message received: ignore it */
    else if ((waitingForMsgID != SMMsgs_noMsg) && (waitingForMsgID != CMMsgId) &&
            (SMMsgs_cmdIds_processFail != CMMsgId))
    {
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                             "Unexpected message received - Ignoring it!");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
        /* Do nothing and return */
        return;
    }
    /* Received expected message: process it further */
    else
    {
        /* Halt timeout clock */
        stopSMProcessTimeoutClock();
    }

#ifdef SECURE_MANAGER_DEBUG
    printCmdString(CMMsgId, false, false);
#endif

    /* No message being currently waited on */
    waitingForMsgID = SMMsgs_noMsg;

    /* Expected message is already received, don't bother about the send status of the send Packet  */
    useSendPktStatus =  false;

    /* process the packet based on type of packet */
    switch (CMMsgId & 0xFFF0)
    {
        case SMMsgs_cmdIds_processFail:
        {
            /* Security establishment failure */
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                            "Rx'ed SMMsgs_cmdIds_processFail");
            printErrorString((SMMsgs_errorCode_t)*pBuf);
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
            errorCode = (SMMsgs_errorCode_t)*pBuf;
            switchState(SM_states_finish_fail);
            break;
        }
         case SMMsgs_cmdIds_processResponse:
        {
            if ((SM_type_coordinator == deviceType) && (SM_states_start == currentSMState))
            {
                /* Parse the message and obtain authentication method supported */
                foreignSupportedAuthMethods = *pBuf;
                switchState(SM_states_auth_method);
            }
            break;
        }
        case SMMsgs_cmdIds_processMethodConfirm:
        {
            if ((SM_type_device == deviceType) && (SM_states_auth_method == currentSMState))
            {
                /* Parse the message and obtain authentication method agreed upon,
                 * collector takes precedence
                 */
                 agreedUponAuthMethod = *pBuf;

#ifdef SECURE_MANAGER_DEBUG
                 printAuthMethodString((SMMsgs_authMethod_t)agreedUponAuthMethod);
#endif
                 /* Switch state for passkey selection */
                 switchState(SM_states_passkey);
            }
             break;
        }
        case SMMsgs_cmdIds_processPasskeyConfirm:
        {
            /* Other party has completed passkey input process */
            if ((SM_type_coordinator == deviceType) && (SM_states_auth_method == currentSMState))
            {
                switchState(SM_states_passkey);
            }

            if ((SM_type_device == deviceType) && (SM_states_passkey == currentSMState))
            {
                switchState(SM_states_key_exchange);
            }
           break;
        }
        case SMMsgs_cmdIds_processPublicKey:
        {
            OsalPort_memcpy(localSecDevice.foreignPublicKeyMaterial, pBuf,
                SM_ECC_PUBLIC_KEY_SIZE);

            if ((SM_type_coordinator == deviceType) && (SM_states_passkey == currentSMState))
            {
                /* Received foreign public key, but has not sent local key */
                switchState(SM_states_key_exchange);
            }

            if ((SM_type_device == deviceType) && (SM_states_key_exchange == currentSMState))
            {
                /* Received foreign public key, and already sent local key. Generate shared secret */
                SM_ECC_genSharedSecretKey(&localSecDevice);
                SM_authItrVal = -1;
                switchState(SM_states_authentication);
            }
            break;
        }
        case SMMsgs_cmdIds_processInfo128CodeNonce:
        {
            if (SM_type_coordinator == deviceType)
            {
                if(SM_states_key_exchange == currentSMState)
                {
                    SM_authItrVal = 0;
                    switchState(SM_states_authentication);
                }
                else //already in authentication state
                {
                    SM_authItrVal++;
                    /* set event */
                    Util_setEvent(&SM_events, SM_AUTH_RSP_EVT);
                }
            }
            else //type device
            {
                /* set event */
                Util_setEvent(&SM_events, SM_AUTH_RSP_EVT);
            }

            /* copy received code */
            OsalPort_memcpy(&foreignAuthCodeNoncePairs[SM_authItrVal].Code[0], \
                            pBuf, SM_AUTH_CODE_LEN);

            /* copy received Nonce */
            OsalPort_memcpy(&foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[0], \
                                                            (pBuf + SM_AUTH_CODE_LEN), SM_AUTH_NONCE_LEN);

            break;
        }
        case SMMsgs_cmdIds_processInfo128SecurityVal:
        {
            /* Received security constant needed for key verification */
            /* Store foreign security constant */
            OsalPort_memcpy(&keyGenInfo.receivedE[0], pBuf, SM_KEY_GEN_DATA_SIZE);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                     "Key Gen - Security constant received:");

            uint8_t i = 0;

            for (i = 0; i < 2; i++)
            {
                CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                        "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                                        keyGenInfo.receivedE[i*8], keyGenInfo.receivedE[i*8 + 1],
                                        keyGenInfo.receivedE[i*8 + 2], keyGenInfo.receivedE[i*8 + 3],
                                        keyGenInfo.receivedE[i*8 + 4], keyGenInfo.receivedE[i*8 + 5],
                                        keyGenInfo.receivedE[i*8 + 6], keyGenInfo.receivedE[i*8 + 7]);
            }
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
            if ((SM_type_coordinator == deviceType) && (SM_states_authentication == currentSMState))
            {
                switchState(SM_states_key_gen);
            }

            else if((true == keyRefreshmentProcess) ||
                    ((SM_type_device == deviceType) && (currentSMState == SM_states_key_gen)))
            {
                /* Generate the event so that sensor can verify key info sent by collector */
                Util_setEvent(&SM_events, SM_KEY_GEN_EVT);
            }
            break;
        }
        case SMMsgs_cmdIds_processSuccess:
        {
            /* Key generation successful */
            if (currentSMState == SM_states_key_gen)
            {
                 switchState(SM_states_finish_success);
            }
            break;
        }
    }//end of switch
}//end of function

/******************************************************************************
 Local Functions
 *****************************************************************************/
/*!
 * @brief       Function to transition various states.
 *
 * @param       state - Current startup state of the coordinator or device
 */
static void processState(SM_states_t state)
{
    switch(state)
    {
        case SM_states_entry:
        {
            /* Default state */
            /* if this is a sensor device, reset the SM_forceStopped flag */
            if (deviceType == SM_type_device)
            {
              SM_forceStopped = false;
            }
            break;
        }
        /* Coordinator specific state */
        case SM_states_start:
        {
            /* Set ID of expected reply from other party */
            waitingForMsgID = SMMsgs_cmdIds_processResponse;

            /* reset the SM_forceStopped flag */
            SM_forceStopped = false;

            /* Send data message containing commissioning request to device via Cm_ProcessRequest */
            SMSendInfo(Smgs_cmdIds_CommissionStart, SMMsgs_cmdIds_processRequest,
                       &commissionDevInfo, sizeof(ApiMac_deviceDescriptor_t), SM_DATA_MSDU_HANDLE, false);

            /* Start timeout for commissioning response */
            setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);

            break;
        }

        case SM_states_auth_method:
        {
            /* Coordinator has no further action in this state, pending on device's response */
            if (deviceType == SM_type_coordinator)
            {
                /* Verify authorization type from Cm_ProcessResponse */
                if (isSupportedAuthType(foreignSupportedAuthMethods))
                {
                    /* Set ID of expected reply from other party */
                    waitingForMsgID = SMMsgs_cmdIds_processPasskeyConfirm;

                    /* Send confirmation message to sensor via SMmsgs_CmProcessMethodConfirm_t */
                    /* Communicate the common Auth method type chosen by the collector */
                    SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processMethodConfirm,
                              &agreedUponAuthMethod, sizeof(agreedUponAuthMethod), SM_DATA_MSDU_HANDLE, false);

                    /* Start long timer to account for possible user passkey input process */
                    setSMProcessTimeoutClock(SM_PASSKEY_WAIT_TIMEOUT_VALUE);
                }
                else
                {
                    /* Send failure message to sensor via SMMsgs_cmdIds_processFail */
                    /* Communicate the Auth method types supported by the collector */
                    uint8_t authMethodFailBuf[SM_AUTH_METHOD_FAIL_BUF_SIZE];
                    errorCode = SMMsgs_errorCode_unsupportedAuthMethod;
                    authMethodFailBuf[0] = SMMsgs_errorCode_unsupportedAuthMethod;
                    authMethodFailBuf[SM_AUTH_METHOD_FAIL_BUF_SIZE - 1] = localSupportedAuthMethods;

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
                    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                                    "Auth Method agreement failed");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
                    SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processFail,
                               authMethodFailBuf, SM_AUTH_METHOD_FAIL_BUF_SIZE, \
                               SM_FAIL_MSDU_HANDLE, false);
                }
            }
            else
            {
                /* Set ID of expected reply from other party */
                waitingForMsgID = SMMsgs_cmdIds_processMethodConfirm;

                /* Send device supported auth method to coordinator via Cm_ProcessResponse */
                SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processResponse,
                           &localSupportedAuthMethods, sizeof(localSupportedAuthMethods), \
                           SM_DATA_MSDU_HANDLE, false);

                /* Start timeout for commissioning response */
                setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);
            }
            break;
        }

        case SM_states_passkey:
        {
            /* update on what passkey should be used for authentication */
           if(SMMsgs_authMethod_defaultCode == agreedUponAuthMethod)
           {
#ifdef SM_DEFAULT_AUTH_CODE
               SM_authPassKey = SM_DEFAULT_AUTH_CODE;
               /* set event */
               Util_setEvent(&SM_events, SM_PASSKEY_EVT);
#endif
           }
           else if(SMMsgs_authMethod_passkey == agreedUponAuthMethod)
           {
               /* first ever attempt..not retry attempt
                * get the passkey from user. For retry cases
                * no need to prompt user to input again */
               if ((pSMCallbacks->pfnRequestPasskeyCb) && (SM_cmAttempts < SM_CM_MAX_RETRY_ATTEMPTS))
               {
                   /* Start timer for passkey input process */
                   setSMProcessTimeoutClock(SM_USER_INPUT_TIMEOUT_VALUE);
                   awaitingPasskey = true;
                   /*request passkey */
                   pSMCallbacks->pfnRequestPasskeyCb(SM_passkeyEntryReq);
               }
           }
           else //SMMsgs_authMethod_justAllowed
           {
               SM_authPassKey = 0x0;
               /* set event */
               Util_setEvent(&SM_events, SM_PASSKEY_EVT);
           }
           break;
        }

        case SM_states_key_exchange:
        {
            /* Generate ECDH public/private key-pair */
            SM_ECC_genLocalKeyPair(&localSecDevice);

            /* Coordinator has already received sensor public key so must generate shared secret.
             * Sensor will generate shared secret upon receipt of coordinator public key.
             */
            if (deviceType == SM_type_coordinator)
            {
                SM_ECC_genSharedSecretKey(&localSecDevice);
            }

            /* Set ID of expected reply from other party */
            if (deviceType == SM_type_coordinator)
            {   /* Coordinator ready for authentication stage */
                waitingForMsgID = SMMsgs_cmdIds_processInfo128CodeNonce;

            }
            else
            {   /* Sensor awaits coordinator public key */
                waitingForMsgID = SMMsgs_cmdIds_processPublicKey;
            }

            /* Send newly generated public key to foreign party */
            SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processPublicKey,
                       localSecDevice.localPublicKeyMaterial, SM_ECC_PUBLIC_KEY_SIZE,
                       SM_DATA_MSDU_HANDLE, false);

            /* Start timeout for commissioning response */
            setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);

            break;
        }

        case SM_states_authentication:
        {
#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
           uint8_t i = 0;

            /* Print sharedSecretKey here as both device and coordinator create key in separate locations */
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                "Derived shared secret: ");
            for (i = 0; i < 8; i++)
            {
                 CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                     "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                    localSecDevice.sharedSecretKeyMaterial[i],
                    localSecDevice.sharedSecretKeyMaterial[i + 1], localSecDevice.sharedSecretKeyMaterial[i + 2],
                    localSecDevice.sharedSecretKeyMaterial[i + 3], localSecDevice.sharedSecretKeyMaterial[i + 4],
                    localSecDevice.sharedSecretKeyMaterial[i + 5], localSecDevice.sharedSecretKeyMaterial[i + 6],
                    localSecDevice.sharedSecretKeyMaterial[i + 7]);
            }
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

            /* prepare for doing authentication */
            SMProcessAuthState();
            break;
        }

        case SM_states_key_gen:
        {
            /* Process started by SM_startKeyRefreshProcess for key refreshment */
            if (keyRefreshmentProcess == false)
            {
                startKeyGenProcess();
            }
            break;
        }

        case SM_states_finish_success:
        {
            uint8_t IsKeyNew;
            SM_seedKey_Entry_t * pSeedKeyEnty;
#ifdef FEATURE_FULL_FUNCTION_DEVICE
            Cllc_associated_devices_t *pExistingDevice;

             /* Update the seedKeyTable */
             /* Use device ext address */
             IsKeyNew = addEntry2SeedKeyTable(commissionDevInfo.extAddress, commissionDevInfo.shortAddress, localSecDevice.sharedSecretKeyMaterial);
             pSeedKeyEnty = getEntryFromSeedKeyTable(commissionDevInfo.extAddress, commissionDevInfo.shortAddress);

             /* Update the info for key refreshment */
             pExistingDevice = findDevice(commissionDevInfo.shortAddress);
             /* Pass the device key to MAC */
             if(IsKeyNew == SM_Key_New)
             {
                 addDeviceKey(pSeedKeyEnty,keyGenInfo.deviceKey,true);
                 /* New Key */
                 pExistingDevice->NumOfKeyRefresh = 0;
                 pExistingDevice->keyRef_statue = SM_KEYRF_REQUIRED;

             }
             else
             {
                 addDeviceKey(pSeedKeyEnty,keyGenInfo.deviceKey,false);
                 /* Do not set SM_KEYRF_REQUIRED here for key refreshment */
                 if(keyRefreshmentProcess == false)
                 {
                     pExistingDevice->NumOfKeyRefresh = 0;
                     pExistingDevice->keyRef_statue = SM_KEYRF_REQUIRED;
                 }
             }

#else
             nvDeviceKeyInfo_t devKeyInfo;

             /* Update the seedKeyTable */
             /* Use its own ext address */
             IsKeyNew = addEntry2SeedKeyTable(ApiMac_extAddr, SM_Sensor_SAddress, localSecDevice.sharedSecretKeyMaterial);
             pSeedKeyEnty = getEntryFromSeedKeyTable(ApiMac_extAddr, SM_Sensor_SAddress);

             /* Pass the device key to MAC */
             if(IsKeyNew == SM_Key_New)
             {
                 addDeviceKey(pSeedKeyEnty,keyGenInfo.deviceKey,true);
             }
             else
             {
                 addDeviceKey(pSeedKeyEnty,keyGenInfo.deviceKey,false);
             }

             /*save the key info to NV */
             memcpy(devKeyInfo.lookUpData, pSeedKeyEnty->lookUpData, (APIMAC_KEY_SOURCE_MAX_LEN));
             memcpy(devKeyInfo.deviceKey, keyGenInfo.deviceKey, (APIMAC_KEY_MAX_LEN));
             memcpy(devKeyInfo.seedKey, pSeedKeyEnty->seedKey, (SM_ECC_PUBLIC_KEY_SIZE));
             devKeyInfo.index = pSeedKeyEnty->index;

             Ssf_DeviceKeyInfoUpdate(&devKeyInfo);
#endif
            /* No message expected */
            waitingForMsgID = SMMsgs_noMsg;
            /* Clear security-related variables */
            clearSecurityVariables();
            /* Reset SM state machine */
            switchState(SM_states_entry);
            /* update SM_Current_State so that key refreshment is enabled */
            SM_Current_State = SM_CM_Finished_Successfully;
            /* clear the SM_cmAttempts count */
            SM_cmAttempts = 0;

#ifndef CUI_DISABLE
            if (keyRefreshmentProcess == true)
            {
                CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                     "Key Refreshed: 0x%04x",
                                     SM_Sensor_SAddress);
            }
            else
            {
                CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                     "Commissioned: 0x%04x",
                                     SM_Sensor_SAddress);
            }
#endif /* CUI_DISABLE */

            /* if co-ordinator */
            if(deviceType == SM_type_coordinator)
            {
                SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processSuccess,
                           0, 0, SM_DATA_MSDU_HANDLE, false);
            }

            /* Notify application of success */
            if(pSMCallbacks->pfnSuccessCMProcessCb) {
                pSMCallbacks->pfnSuccessCMProcessCb(&commissionDevInfo, keyRefreshmentProcess);
            }

            break;
        }

        case SM_states_finish_fail:
        {
            /* increment CM attempts */
            SM_cmAttempts++;
#ifdef FEATURE_FULL_FUNCTION_DEVICE
            Cllc_associated_devices_t *pExistingDevice;
            /* Update the info for key refreshment */
            pExistingDevice = findDevice(commissionDevInfo.shortAddress);
            pExistingDevice->keyRef_statue = SM_KEYRF_FAIL;

#endif
            /* stop timeout clock */
            stopSMProcessTimeoutClock();

            /* Reset timeout monitoring variable */
            timeoutOccurred = false;

            /* No message expected */
            waitingForMsgID = SMMsgs_noMsg;

            /* Clear security-related variables */
            clearSecurityVariables();

            /* Reset SM state machine */
            switchState(SM_states_entry);

            /* Update SM_Current_State so that further commissioning can occur */
            SM_Current_State = SM_CM_Not_Started;

            if (keyRefreshmentProcess)
            {
#ifndef CUI_DISABLE
                CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                     "Key Refresh Failed: 0x%04x", SM_Sensor_SAddress);
#endif /* CUI_DISABLE */
            }
            else if ((errorCode == SMMsgs_errorCode_noMatchAuthVal) ||
                     (errorCode == SMMsgs_errorCode_noMatchKeyConfirm) ||
                     (errorCode == SMMsgs_errorCode_unsupportedAuthMethod) ||
                     (SM_cmAttempts >= SM_CM_MAX_RETRY_ATTEMPTS))
            {
                if ((deviceType == SM_type_device) && (SM_cmAttempts >= SM_CM_MAX_RETRY_ATTEMPTS))
                {
                  SM_cmAttempts = 0;
                }

#ifndef CUI_DISABLE
                /* Device failed authentication, exceeded max commissioning attempts,
                 * or user has not allowed commissioning retry */
                CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                         "Commissioning Failed: 0x%04x", SM_Sensor_SAddress);
#endif /* CUI_DISABLE */
            }
#ifndef CUI_DISABLE
            else
            {
#ifdef FEATURE_FULL_FUNCTION_DEVICE
                /* sensor not responding to process start message in key recovery mode */
                if ((fCommissionRequired) && (errorCode == SMMsgs_errorCode_reComm_sensor_notResp))
                {
                    CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                            "Commissioning Postponed: 0x%04x", SM_Sensor_SAddress);
                }
                else
#endif
                {
                    /* Commissioning retry */
                    CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                            "Commissioning being Re-attempted: Attempt %d, Sensor 0x%04x", SM_cmAttempts, SM_Sensor_SAddress);
                }
            }
#endif /* CUI_DISABLE */

            /* Notify application of failure */
           if(pSMCallbacks->pfnFailCMProcessCb)
           {
               pSMCallbacks->pfnFailCMProcessCb(&commissionDevInfo, devRxOnIdle,
                                              keyRefreshmentProcess, errorCode);
           }

           break;
        }

        default:
        {
            break;
        }
    }
}


/*!
 * @brief       Switch to the new security manager state and set the event bit.
 *
 * @param       newState - next state of security manager
 */
static void switchState(SM_states_t newState)
{
    /* Validate current state */
    if (isValidState(currentSMState, newState)) {
        currentSMState = newState;
        Util_setEvent(&SM_events, SM_STATE_CHANGE_EVT);
    }

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
    else
    {
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                             "Invalid state attempted %d", newState);
    }
     printStateString(newState);
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */

}

/*!
 * @brief       Function to validate the next desired SM state.
 *
 * @param       currentState - Current state of the SM module
 * @param       nextState - Next state of the SM module
 *
 * @return      true - If next state is valid based on device type and current state
 *              false - If next state is invalid
 */
bool isValidState(SM_states_t currentState, SM_states_t nextState)
{
    bool ret = false;

    if(nextState == SM_states_finish_fail) {
        ret = true;
    }
    else {
        switch(currentState)
        {
            case SM_states_entry:
                if(deviceType == SM_type_coordinator) {
                    if((nextState == SM_states_start) ||
                        ((nextState == SM_states_key_gen) && (keyRefreshmentProcess == true)))
                        ret = true;
                }
                else {
                    if((nextState == SM_states_auth_method)||
                       ((nextState == SM_states_key_gen) && (keyRefreshmentProcess == true)))
                        ret = true;
                }
            break;
            case SM_states_start:
                if ((deviceType == SM_type_coordinator) && (nextState == SM_states_auth_method))
                {
                    ret = true;
                }
            break;
            case SM_states_auth_method:
                if (nextState == SM_states_passkey)
                {
                    ret = true;
                }
            break;
            case SM_states_passkey:
                if (nextState == SM_states_key_exchange)
                {
                   ret = true;
                }
            break;
            case SM_states_key_exchange:
                if (nextState == SM_states_authentication)
                {
                    ret = true;
                }
            break;
            case SM_states_authentication:
                if (nextState == SM_states_key_gen)
                {
                    ret = true;
                }
            break;
            case SM_states_key_gen:
                if (nextState == SM_states_finish_success)
                {
                    ret = true;
                }
            break;
            case SM_states_finish_success:
                if (nextState == SM_states_entry)
                {
                    ret = true;
                }
            break;
            case SM_states_finish_fail:
                if (nextState == SM_states_entry)
                {
                    ret = true;
                }
            break;
        }
    }


    return (ret);
}

/*!
 * @brief       Clear all security-related local static variables
 */
static void clearSecurityVariables(void) {

    /* Clear any previous security information */
    memset(&localSecDevice, 0, sizeof(SM_ECC_securityDeviceDescriptor_t));

    /* Clear any previous authentication values */
    memset(localAuthCodeNoncePairs, 0, sizeof(localAuthCodeNoncePairs));
    memset(foreignAuthCodeNoncePairs, 0, sizeof(foreignAuthCodeNoncePairs));

    /* Clear key generation values */
    memset(&keyGenInfo, 0, sizeof(SM_keyVals_Descriptor_t));

}

/*!
 * @brief       Initialize the commissioning status clock.
 *              This clock is to be set when a commissioning request/response
 *              is pending from a third party.
 */
static void initializeCommStatusClock(void)
{
    /* Initialize SM timeout timer */
    smProcessTimeoutClkHandle = UtilTimer_construct(&smProcessTimeoutClkStruct,
                                    processSMProcessTimeoutCallback,
                                    SM_COMMISIONING_STATUS_TIMEOUT_VALUE,
                                    0,
                                    false,
                                    0);
}

/*!
 * @brief       Commissioning request/response timeout handler.
 *
 * @param       a0 - ignored
 */
static void processSMProcessTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    timeoutOccurred = true;

    /* Set timeout event during commissioning process */
    /* Event not set for overall SM retry user input timeout */
    if (SM_CM_InProgress == SM_Current_State)
    {
        Util_setEvent(&SM_events, SM_TIMEOUT_EVT);
        sem_post(hSMappSem);
    }

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
    CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                         "Message timeout");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
}

/*!
 * @brief       Commissioning request/response timeout handler.
 *
 * @param       a0 - ignored
 */
static void setSMProcessTimeoutClock(uint32_t timeout)
{
    /* Stop the SM timeout timer */
    if(UtilTimer_isActive(&smProcessTimeoutClkStruct) == true)
    {
        UtilTimer_stop(&smProcessTimeoutClkStruct);
    }

    if(timeout != 0)
    {
        /* Setup timer */
        UtilTimer_setTimeout(smProcessTimeoutClkHandle, timeout);
        UtilTimer_start(&smProcessTimeoutClkStruct);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                             "Timer started");
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
    }
}

/*!
 * @brief       Commissioning request/response timeout handler.
 *
 * @param       a0 - ignored
 *
 * @returns     true if timer was active before stopped, false otherwise
 */
static bool stopSMProcessTimeoutClock(void)
{
    bool wasActive = false;

    /* Stop the SM timeout timer */
    if(UtilTimer_isActive(&smProcessTimeoutClkStruct) == true)
    {
        UtilTimer_stop(&smProcessTimeoutClkStruct);
        wasActive = true;
    }
#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                        "Timer stopped");
#endif
 /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
    return wasActive;
}


/*!
 * @brief       Stub function for determining whether input authorization method is supported.
 *
 * @param       authMethods - The authMethod parameter for this function is a bitmask containing
 *                           values of type SMMsgs_authMethod_t.
 */
static bool isSupportedAuthType(uint8_t authMethods)
{
    uint8_t commonAuthMethods = authMethods & localSupportedAuthMethods;

    if (0 != commonAuthMethods)
    {
        /* pick a common auth method as per priority (collector's responsibility) */
        agreedUponAuthMethod = SMMsgs_authMethod_passkey;
        while( 0 == (agreedUponAuthMethod & commonAuthMethods))
        {
            agreedUponAuthMethod = (agreedUponAuthMethod << 1);
        }

#ifdef SECURE_MANAGER_DEBUG
             printAuthMethodString((SMMsgs_authMethod_t)agreedUponAuthMethod);
#endif
        return (true);
    }
    else /* nothing in common */
    {
#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                             "Error - No common Auth Method!");
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
        return (false);
    }
}

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
/*!
 * @brief       Debug helper function to print selected authentication method string
 *
 * @param       authMethod - agreed upon authentication Method value
 */
static void printAuthMethodString(SMMsgs_authMethod_t authMethod)
{
    switch(authMethod)
        {
            case SMMsgs_authMethod_passkey:
                CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                     "Agreed upon Auth Method is Pass Key based Authentication");
                break;

            case SMMsgs_authMethod_defaultCode:
                CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                     "Agreed upon Auth Method is Default code based Authentication");
                break;

            case SMMsgs_authMethod_justAllowed:
                CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                     "Agreed upon Auth Method is Just Allowed based Authentication");
                break;
        }
}

/*!
 * @brief       Debug helper function to print current state string
 *
 * @param       newState - next state of security manager
 */
static void printStateString(SM_states_t newState)
{
    switch(newState)
    {
        case SM_states_entry:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_entry");
            break;

        case SM_states_start:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_start");
            break;

        case SM_states_auth_method:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_auth_method");
            break;

        case SM_states_passkey:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_passkey");
            break;

        case SM_states_key_exchange:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_key_exchange");
            break;

        case SM_states_authentication:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_authentication");
            break;

        case SM_states_key_gen:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_key_gen");
            break;

        case SM_states_finish_success:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_finish_success");
            break;

        case SM_states_finish_fail:
            CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Changing State to SM_states_finish_fail");
            break;
    }
}

/*!
 * @brief       Debug helper function to print current command message id
 *
 * @param       CMMsgId - command id received
 * @param       cmdStr - command id received from external security manager in string form
 * @param       isMsgSent - true if the message was sent, false if it was received
 * @param       retryFlag - true if packet is being reset, else it is false
 */
static void printCmdString(SMMsgs_cmdIds_t CMMsgId, bool isMsgSent, bool retryFlag)
{
    char sendOrRecv[8] = {0};
    char SmItr[2] = {0};

    /* Differentiate whether message was sent or received */
    if (isMsgSent == true)
    {
        retryFlag == false? (strcpy(sendOrRecv, "sent")):(strcpy(sendOrRecv, "sent: Retry Attempt")) ;
    }
    else
    {
        strcpy(sendOrRecv, "received");
    }

    switch((CMMsgId & 0xFFF0))
    {
        case SMMsgs_cmdIds_processRequest:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processRequest command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processResponse:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processResponse command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processMethodConfirm:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processMethodConfirm command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processPasskeyConfirm:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processPasskeyConfirm command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processPublicKey:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processPublicKey command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processInfo128SecurityVal:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processInfo128Code command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processInfo128CodeNonce:
            SmItr[0] = '0' + (char)(CMMsgId & 0x000F);
            SmItr[1] = '\0';

            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processInfo128CodeNonce command : Pkt Num = %s %s", sendOrRecv, SmItr );
            break;

        case SMMsgs_cmdIds_processSuccess:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processSuccess command %s", sendOrRecv);
            break;

        case SMMsgs_cmdIds_processFail:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_cmdIds_processFail command %s", sendOrRecv);
            break;

    }
}

/*!
 * @brief       Debug helper function to print the error code received
 *
 * @param       errorCode - error code received
 */
void printErrorString(SMMsgs_errorCode_t errorCode)
{
    switch(errorCode)
    {
        case SMMsgs_errorCode_unsupportedAuthMethod:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_errorCode_unsupportedAuthMethod error");
            break;

        case SMMsgs_errorCode_noMatchAuthVal:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_errorCode_noMatchAuthVal error");
            break;

        case SMMsgs_errorCode_noMatchKeyConfirm:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_errorCode_noMatchKeyConfirm error");
            break;

        case SMMsgs_errorCode_unexpectedMsg:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_errorCode_unexpectedMsg error");
            break;

        case SMMsgs_errorCode_timeout:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SMMsgs_errorCode_timeout error");
            break;

        default:
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "SM: unknown error code");
            break;
    }
}
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */

/******************************************************************************
 Message specific functions
 *****************************************************************************/
/*!
 * @brief      Send MAC data request
 *
 * @param      len - length of payload
 * @param      pData - pointer to the buffer
 * @param      msduHandle - MSDU handle
 *
 * @return  true if sent, false if not
 */
static bool sendSMMsg(uint8_t *pData, uint16_t len, uint8_t msduHandle)
{
    ApiMac_mcpsDataReq_t dataReq;

    /* Fill the data request field */
    memset(&dataReq, 0, sizeof(ApiMac_mcpsDataReq_t));

    dataReq.dstAddr.addrMode = ApiMac_addrType_short;
    dataReq.dstAddr.addr.shortAddr = commissionDevInfo.shortAddress;
    dataReq.srcAddrMode = ApiMac_addrType_short;

    if(devFHEnabled && devRxOnIdle)
    {
        /* Switch to the extended address */
        dataReq.dstAddr.addrMode = ApiMac_addrType_extended;
        memcpy(&dataReq.dstAddr.addr.extAddr, &commissionDevInfo.extAddress,
               (APIMAC_SADDR_EXT_LEN));
        dataReq.srcAddrMode = ApiMac_addrType_extended;
    }

    dataReq.dstPanId = commissionDevInfo.panID;

    dataReq.msduHandle = msduHandle;

    dataReq.txOptions.ack = true;
    if(devRxOnIdle == false)
    {
        dataReq.txOptions.indirect = true;
    }
    else
    {
        dataReq.txOptions.indirect = false;
    }

    dataReq.msdu.len = len;
    dataReq.msdu.p = pData;

    /* Fill in the appropriate security fields */
    memcpy(&dataReq.sec, &commissionSec, sizeof(ApiMac_sec_t));

    /* Send the message */
    if(ApiMac_mcpsDataReq(&dataReq) != ApiMac_status_success)
    {
        /*  Transaction overflow occurred */
        return (false);
    }
    else
    {
        return (true);
    }
}

/*!
 * @brief       Wrapper function to prepare and send data during key generation state
 *
 * @parm        msgID - sensor data message ID
 * @param       smCmdID - SM message ID
 * @param       pData - data to send
 * @param       dataLen - length of data to send
 * @param       msduHandle - MSDU handle of packet to send
 * @param       retryFlag - 0 if the packet is sent for first time, 1 if being retried
 *
 * @return      true if sent, false if not
 */
static bool SMSendInfo(Smsgs_cmdIds_t msgID, SMMsgs_cmdIds_t smCmdID,
                       void *pData, uint8_t dataLen, uint8_t msduHandle,
                       bool retryFlag)
{
    static uint8_t *pMsgBuf;
    static uint16_t len;
    static SMMsgs_cmdIds_t smCmdIdToSend;
    static uint8_t msduHandleToSend;
    /* Flag used to track number of retry attempt, either because
     * the packet couldnt be handed over to MAC or dint get ack from other party */
    static uint8_t sendMsgAttempts = 0;

    uint8_t retVal = false;

    /* trying to send packet for the first time */
    if(false == retryFlag)
    {
        /* Transmission of fresh packet : clear the flag */
        sendMsgAttempts = 0;

        len = SMMSGS_BASE_CMD_LENGTH + dataLen;

        /* Free the memory allocated to the previous packet */
        if(pMsgBuf)
        {
            OsalPort_free(pMsgBuf);
        }

        /* Allocate memory to the current message/pkt to send out */
        pMsgBuf = (uint8_t *)OsalPort_malloc(len);

        if(pMsgBuf)
        {
            /* Prepare the message/ pkt to be sent out */
            uint8_t *pBuf = pMsgBuf;
            *pBuf++ = (uint8_t)msgID;
            smCmdIdToSend = smCmdID;
            pBuf = Util_bufferUint16(pBuf, smCmdIdToSend);
            msduHandleToSend = msduHandle;

            /* Check if there is data to append */
            if (pData)
            {
                OsalPort_memcpy(pBuf, pData , dataLen);
            }
        }
        else
        {
            /* could not allocate memory */
            return(retVal);
        }

    }// end of false == retryFlag

    /* Exceeded max failure attempts */
    if (sendMsgAttempts > SM_PKT_MAX_RETRY_ATTEMPTS)
    {
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                "SM: Failed to send a packet: exceeded Max Attempts!");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
        if(SMMsgs_cmdIds_processRequest == smCmdIdToSend)
        {
            errorCode = SMMsgs_errorCode_reComm_sensor_notResp;
        }

        /* switch to fail state */
        switchState(SM_states_finish_fail);

        /* exit this function */
        return(retVal);
    }

    /* if we are here: we still have retry attempts for the packet or its the first time */

    /* Actually send the message */
    retVal = sendSMMsg(((uint8_t*)pMsgBuf), len, msduHandleToSend);

    /* Could not admit packet into the MAC queue */
    if(retVal != true)
    {
        /* Set an event that failure status from previous packet sent is seen */
        Util_setEvent(&SM_events, SM_SEND_PKT_FB_EVT);

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                "Failed to put pkt in MAC queue. Will retry!");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */

    }
    else /* Was able to admit the packet into MAC */
    {
        /* Set flag to wait and use the status of the send Info */
        useSendPktStatus =  true;

        /* increment the counter tracking number of retry attempts */
        sendMsgAttempts++;

#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        /* print the message that packet was sent */
        printCmdString(smCmdID, true, retryFlag); /* tony: logic wrt retry flag needs to be fixed */
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
    }

    return(retVal);
}

/*!
 * @brief       Function to kick off security manager for a specific device.
 */
static void startKeyGenProcess()
{
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                         "Beginning Key Generation");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
    /* Temporary storage for ConKey || Device Key (concatenated) */
    uint8_t concatKeys[APIMAC_KEY_MAX_LEN * 2];
    uint8_t passkeyBuf[SM_PASSKEY_BUF_SIZE];
    uint8_t agreedUponAuthMethodBuf[SM_AUTH_METHOD_BUF_SIZE];
    uint8_t xorNonceLocal[SM_AUTH_NONCE_LEN];
    uint8_t xorNonceForeign[SM_AUTH_NONCE_LEN];
    uint8_t nonceIdx = 0;
    uint8_t nonceByteIdx = 0;


    /* Clear local buffers */
    memset(xorNonceLocal, 0, SM_AUTH_NONCE_LEN);
    memset(xorNonceForeign, 0, SM_AUTH_NONCE_LEN);
    memset(passkeyBuf, 0, SM_PASSKEY_BUF_SIZE);
    memset(agreedUponAuthMethodBuf, 0, SM_AUTH_METHOD_BUF_SIZE);

    /* XOR array of nonces to combine into single nonce for f2() and f3() input */
    for(nonceIdx = 0; nonceIdx < SM_NUM_AUTH_ITERATIONS; nonceIdx++)
    {
        for (nonceByteIdx = 0; nonceByteIdx < SM_AUTH_NONCE_LEN; nonceByteIdx++)
        {
            xorNonceLocal[nonceByteIdx] ^= localAuthCodeNoncePairs[nonceIdx].Nonce[nonceByteIdx];
            xorNonceForeign[nonceByteIdx] ^= foreignAuthCodeNoncePairs[nonceIdx].Nonce[nonceByteIdx];
        }
    }

    /* Format agreedUponAuthMethod for f3() crypto call */
    OsalPort_memcpy(agreedUponAuthMethodBuf, &agreedUponAuthMethod, sizeof(agreedUponAuthMethod));

    /* Format passkey for f3() call. Place passkey in lowest bytes of byte array */
    uint8_t passkeyBufOffset = SM_PASSKEY_BUF_SIZE - sizeof(SM_authPassKey);
    OsalPort_memcpy(passkeyBuf + passkeyBufOffset, &SM_authPassKey, sizeof(SM_authPassKey));

    /* Generate keys */
    if (deviceType == SM_type_coordinator)
    {
        SM_f2(localSecDevice.sharedSecretKeyMaterial, xorNonceLocal,
              xorNonceForeign, ApiMac_extAddr, commissionDevInfo.extAddress,
              concatKeys);
    }
    else
    {
        SM_f2(localSecDevice.sharedSecretKeyMaterial, xorNonceForeign,
              xorNonceLocal, commissionDevInfo.extAddress, ApiMac_extAddr,
              concatKeys);
    }

    /* Store conKey and Device key */
    OsalPort_memcpy(keyGenInfo.conKey, &concatKeys[0], APIMAC_KEY_MAX_LEN);
    OsalPort_memcpy(keyGenInfo.deviceKey, &concatKeys[APIMAC_KEY_MAX_LEN], APIMAC_KEY_MAX_LEN);

    /* Local security value (Collector Ec/Sensor Es) to be sent to other party */
    SM_f3(keyGenInfo.conKey, xorNonceLocal, xorNonceForeign,
          passkeyBuf, agreedUponAuthMethodBuf, ApiMac_extAddr,
          commissionDevInfo.extAddress, keyGenInfo.localE);

    /* Comparator security value to verify value received from other party */
    SM_f3(keyGenInfo.conKey, xorNonceForeign, xorNonceLocal,
          passkeyBuf, agreedUponAuthMethodBuf, commissionDevInfo.extAddress,
          ApiMac_extAddr, keyGenInfo.expectedE);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
    uint8_t i = 0;

    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                         "Key Gen: Generated keys (Concat'd): ");
    for (i = 0; i < 4; i++)
    {
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                            concatKeys[i*8], concatKeys[i*8 + 1], concatKeys[i*8 + 2],
                            concatKeys[i*8 + 3], concatKeys[i*8 + 4], concatKeys[i*8 + 5],
                            concatKeys[i*8 + 6], concatKeys[i*8 + 7]);
    }

    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                             "Key Gen: Local Security Val:");
    for (i = 0; i < 2; i++)
    {
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                            keyGenInfo.localE[i*8], keyGenInfo.localE[i*8 + 1],
                            keyGenInfo.localE[i*8 + 2], keyGenInfo.localE[i*8 + 3],
                            keyGenInfo.localE[i*8 + 4], keyGenInfo.localE[i*8 + 5],
                            keyGenInfo.localE[i*8 + 6], keyGenInfo.localE[i*8 + 7]);
    }

    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                             "Key Gen: Expected Security Val:", 6);
    for (i = 0; i < 2; i++)
    {
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                             "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                            keyGenInfo.expectedE[i*8], keyGenInfo.expectedE[i*8 + 1],
                            keyGenInfo.expectedE[i*8 + 2], keyGenInfo.expectedE[i*8 + 3],
                            keyGenInfo.expectedE[i*8 + 4], keyGenInfo.expectedE[i*8 + 5],
                            keyGenInfo.expectedE[i*8 + 6], keyGenInfo.expectedE[i*8 + 7]);
    }
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

    /* Sensor sends local Es value */
    if (deviceType == SM_type_device)
    {
        /* Set ID of expected reply from other party */
        waitingForMsgID = SMMsgs_cmdIds_processInfo128SecurityVal;

        SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processInfo128SecurityVal,
                   keyGenInfo.localE, SM_KEY_GEN_DATA_SIZE, SM_DATA_MSDU_HANDLE, false);

        /* Start timeout for commissioning response */
        setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                             "Key Gen: Sensor Es sent");
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
    }
    else // type coordinator
    {
        processKeyGenState();
    }// type coordinator
}

/*!
 * @brief       Function to transition between key generation sub states.
 *
 * @param       state - Current key generation sub state of the coordinator or device
 */
static void processKeyGenState()
{
    /* Compare foreign and locally calculated security constant Es */
    if (OsalPort_memcmp(keyGenInfo.expectedE, keyGenInfo.receivedE,
                        SM_KEY_GEN_DATA_SIZE) == TRUE)
    {
        if (deviceType == SM_type_coordinator)
        {
            /* Set ID of expected reply from other party */
            waitingForMsgID = SMMsgs_cmdIds_processSuccess;

            /* Send local E back */
            SMSendInfo(Smgs_cmdIds_CommissionMsg, \
                       SMMsgs_cmdIds_processInfo128SecurityVal, \
                       keyGenInfo.localE, SM_KEY_GEN_DATA_SIZE, \
                       SM_DATA_MSDU_HANDLE, false);
        }
        else
        {
            /* Set ID of expected reply from other party */
            waitingForMsgID = SMMsgs_cmdIds_processSuccess;

            /* Send confirmation back */
            SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processSuccess, \
                       0, 0, SM_DATA_MSDU_HANDLE, false);
        }

        /* Start timeout for commissioning response */
        setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);

    }
    else //verification failed
    {
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                            "Key Gen - Failed");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
        /* Send failure message to other party via SMMsgs_cmdIds_processFail */
        errorCode = SMMsgs_errorCode_noMatchKeyConfirm;

        SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processFail,
                   &errorCode, sizeof(SMMsgs_errorCode_t), SM_FAIL_MSDU_HANDLE, false);
    }
}

/******************************************************************************
 MAC Key specific functions
 *****************************************************************************/
/* Add a new devcie key to key table*/
/* input param:     device ext. Address, localSecDevice
 *
 * ouput param:     true = added to the table, false = either already exists or mem allocation fail
 */
void addDeviceKey (SM_seedKey_Entry_t* pSeedKeyEntry, uint8_t *deviceKey, bool newkey) {

    ApiMac_secAddKeyInitFrameCounter_t secInfo;

    memcpy(secInfo.key, deviceKey, APIMAC_KEY_MAX_LEN);
    secInfo.frameCounter = 0;
    secInfo.replaceKeyIndex = pSeedKeyEntry->index;
    secInfo.newKeyFlag = newkey;
    secInfo.lookupDataSize = APIMAC_KEY_LOOKUP_LONG_LEN;
    secInfo.networkKey = false;
    memcpy(&secInfo.lookupData[0], &pSeedKeyEntry->lookUpData[0], (APIMAC_KEY_SOURCE_MAX_LEN));

    if(SM_type_coordinator == deviceType) {
        /* pass sensor's ext address */
        memcpy(&secInfo.ApiMac_extAddr, &pSeedKeyEntry->extAddress, (APIMAC_SADDR_EXT_LEN));
    }
    else
    {
        /* pass collector's ext address */
        memcpy(&secInfo.ApiMac_extAddr, &commissionDevInfo.extAddress, (APIMAC_SADDR_EXT_LEN));
    }

    secInfo.lookupData[APIMAC_KEY_LOOKUP_LONG_LEN-1] = pSeedKeyEntry->shortAddress;

    ApiMac_secAddKeyInitFrameCounter(&secInfo);
}

/* Get a SeedKey Entry pointer from the SeedKeyTable*/
/* input param:     device ext. Address + short address
 *
 * ouput param:    pointer of a SeedKey Entry that matches the device ext. address, NULL => cannot find the entry
 */
SM_seedKey_Entry_t * getEntryFromSeedKeyTable (ApiMac_sAddrExt_t extAddress, uint16_t shortAddress) {
    /* Load list head as the start point of the iterator. It is always the default key. */
    SM_seedKey_Entry_t* pSeedKey = (SM_seedKey_Entry_t*) List_head(&SeedKeyTableList);

    while(1) {
        /* get the next entry */
        pSeedKey = (SM_seedKey_Entry_t*)List_next((List_Elem *)pSeedKey);
        if (pSeedKey) {
            if ((OsalPort_memcmp(pSeedKey->extAddress, extAddress, sizeof(ApiMac_sAddrExt_t)) == TRUE)&& (pSeedKey->shortAddress == shortAddress))
                /* found a matching entry */
                return(pSeedKey);
            else
                /* move to the next entry */
                continue;
        }
        else {
            /* cannot find the matching ext address */
            return(NULL);
        }
    }
}


void SM_getSrcDeviceSecurityInfo(ApiMac_sAddrExt_t extAddress, uint16_t shorAddress, ApiMac_sec_t *pSec) {

    SM_seedKey_Entry_t* pSeedKey = getEntryFromSeedKeyTable(extAddress, shorAddress);

    if(pSeedKey) {

        memcpy(pSec->keySource, &pSeedKey->lookUpData[0],(APIMAC_KEY_SOURCE_MAX_LEN));
        pSec->securityLevel = ApiMac_secLevel_encMic32;
        pSec->keyIdMode = ApiMac_keyIdMode_8;
        pSec->keyIndex = pSeedKey->shortAddress;

    }
    else {
#ifdef FEATURE_FULL_FUNCTION_DEVICE
        Cllc_securityFill(pSec);
#else
        Jdllc_securityFill(pSec);
#endif
    }
}

/******************************************************************************
 Authentication specific functions
 *****************************************************************************/

static void SMAuthStateGenCodeAndNonce(uint8_t itrNum, uint8_t curPassKey)
{
    /* Generate Nonce for this iteration */
    SM_ECC_genRandomPrivateKeyMaterial( &localAuthCodeNoncePairs[itrNum].Nonce[0], SM_AUTH_NONCE_LEN);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
            "Authentication: Locally generated Nonce :0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", \
            localAuthCodeNoncePairs[itrNum].Nonce[0], localAuthCodeNoncePairs[itrNum].Nonce[1],\
            localAuthCodeNoncePairs[itrNum].Nonce[2], localAuthCodeNoncePairs[itrNum].Nonce[3],\
            localAuthCodeNoncePairs[itrNum].Nonce[4], localAuthCodeNoncePairs[itrNum].Nonce[5],\
            localAuthCodeNoncePairs[itrNum].Nonce[6], localAuthCodeNoncePairs[itrNum].Nonce[7] );
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

    /* Calculate output of F1() i.e. Code for this iteration */
    /* SM_f1( local Public Key, foreign public key, Local Nonce, Passkey sub set, Output */
    SM_f1( localSecDevice.localPublicKeyMaterial, localSecDevice.foreignPublicKeyMaterial,
           &localAuthCodeNoncePairs[itrNum].Nonce[0],\
           curPassKey, \
           &localAuthCodeNoncePairs[itrNum].Code[0]);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
    CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                "Authentication: Locally generated Code :0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", \
                localAuthCodeNoncePairs[itrNum].Code[0], localAuthCodeNoncePairs[itrNum].Code[1],\
                localAuthCodeNoncePairs[itrNum].Code[2], localAuthCodeNoncePairs[itrNum].Code[3],\
                localAuthCodeNoncePairs[itrNum].Code[4], localAuthCodeNoncePairs[itrNum].Code[5],\
                localAuthCodeNoncePairs[itrNum].Code[6], localAuthCodeNoncePairs[itrNum].Code[7] );
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

}

/*!
 * @brief       Core of State Machine implementation
 *              for Authentication step
 *
 * @param       None
 */
static void SMProcessAuthState()
{
    /*static*/ uint32_t mask;
    static uint8_t itrPassKey;
    char tempBuf[SM_AUTH_CODE_NONCE_LEN];

    uint8_t locallyCompForeignCode[SM_AUTH_CODE_LEN];

    /* code and Nonce verification stage */
    /* skip it for device on 0th iteration */
    if(((SM_type_device == deviceType) && (-1 != SM_authItrVal)) || (SM_type_coordinator == deviceType))
    {
#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                "Authentication - Received Code : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", \
                foreignAuthCodeNoncePairs[SM_authItrVal].Code[0], foreignAuthCodeNoncePairs[SM_authItrVal].Code[1],\
                foreignAuthCodeNoncePairs[SM_authItrVal].Code[2], foreignAuthCodeNoncePairs[SM_authItrVal].Code[3],\
                foreignAuthCodeNoncePairs[SM_authItrVal].Code[4], foreignAuthCodeNoncePairs[SM_authItrVal].Code[5],\
                foreignAuthCodeNoncePairs[SM_authItrVal].Code[6], foreignAuthCodeNoncePairs[SM_authItrVal].Code[7] );

        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                "Authentication - Received Nonce : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", \
                foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[0], foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[1],\
                foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[2], foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[3],\
                foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[4], foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[5],\
                foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[6], foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[7] );
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

        if((SM_type_coordinator == deviceType))
        {
            /* Get passkey-i for this iteration */
            mask = (((uint32_t)1<<PASSKEY_TEST_SIZE) -1) << (SM_authItrVal*PASSKEY_TEST_SIZE);
            itrPassKey = ((mask & SM_authPassKey) >> (SM_authItrVal*PASSKEY_TEST_SIZE));

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                    "Authentication - Iteration Number = %d", SM_authItrVal);
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                    "Authentication - Iteration Pass Key = %d", itrPassKey);
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
        }

        /* do calculations on received (code, nonce) pair */
        /* SM_f1( foreign Public Key, local public key, foreign Nonce, Passkey sub set, Output */
        SM_f1( localSecDevice.foreignPublicKeyMaterial, localSecDevice.localPublicKeyMaterial,
                               &foreignAuthCodeNoncePairs[SM_authItrVal].Nonce[0],\
                               itrPassKey, \
                               &locallyCompForeignCode[0]);
    }

    /* code and Nonce verification stage */
    /* skip it for device on 0th iteration */
    if(((SM_type_device == deviceType) && (-1 != SM_authItrVal)) || (SM_type_coordinator == deviceType))
    {
#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                            "Authentication - Locally computed Foreign Code : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
                            locallyCompForeignCode[0], locallyCompForeignCode[1],\
                            locallyCompForeignCode[2], locallyCompForeignCode[3],\
                            locallyCompForeignCode[4], locallyCompForeignCode[5],\
                            locallyCompForeignCode[6], locallyCompForeignCode[7] );
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

        /* verify results */
        if( FALSE == OsalPort_memcmp(&locallyCompForeignCode[0], \
                                    &foreignAuthCodeNoncePairs[SM_authItrVal].Code[0], \
                                    SM_AUTH_CODE_LEN)) //verification failed
        {
            //switch state and get out
            /* If Authentication failed, switch states to finish fail */
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "Authentication Failed");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
            /* Send failure message to other party via SMMsgs_cmdIds_processFail */
            errorCode = SMMsgs_errorCode_noMatchAuthVal;

            SMSendInfo(Smgs_cmdIds_CommissionMsg, SMMsgs_cmdIds_processFail,
                       &errorCode, sizeof(SMMsgs_errorCode_t), SM_FAIL_MSDU_HANDLE, false);

            /* move out of this function */
            return;
        }/* memcmp is false */
    }

    /* If verification is done earlier, implies Verification passed; */
    /* Or device is generating code and Nonce for first itr of authentcation */
    /* Either cases: go ahead with code and Nonce generation */
    /* for coordinator, we generate for current iteration */
    /* for sensor, we do it for next iteration */

    /* Generate Code and Nonce */

     if(SM_type_device == deviceType)
    {
         /* For 0th iteration, it becomes 0 here */
         SM_authItrVal++;

         if(SM_authItrVal == SM_NUM_AUTH_ITERATIONS)
         {
             /* If Authentication successful, switch states to generate device key */
#if defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE)
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine,
                                 "Authentication Successful");
#endif /* defined(SECURE_MANAGER_DEBUG) && !defined(CUI_DISABLE) */
            /* switch state for the SM state machine */
            switchState(SM_states_key_gen);

            /* move out of this function */
            return;
         }

        /* Get passkey-i for this iteration */
        mask = (((uint32_t)1<<PASSKEY_TEST_SIZE) -1) << (SM_authItrVal*PASSKEY_TEST_SIZE);
        itrPassKey = ((mask & SM_authPassKey) >> (SM_authItrVal*PASSKEY_TEST_SIZE));

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                "Authentication Iteration Number = %d", SM_authItrVal);
        CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                "Authentication Iteration Pass Key = %d", itrPassKey);
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */
    }

    /* generate Code and Nonce */
    SMAuthStateGenCodeAndNonce(SM_authItrVal, itrPassKey);

#if defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE)
            CUI_statusLinePrintf(securityCuiHndl, securityDebugStatusLine2,
                                    "Authentication Sending Code and Nonce");
#endif /* defined(SECURE_MANAGER_DEBUG_ADVANCED) && !defined(CUI_DISABLE) */

    /* concatenate code and nonce to be sent */
    memcpy(&tempBuf[0], &localAuthCodeNoncePairs[SM_authItrVal].Code[0], SM_AUTH_CODE_LEN);
    memcpy(&tempBuf[16], &localAuthCodeNoncePairs[SM_authItrVal].Nonce[0], SM_AUTH_NONCE_LEN);

    /* set wait for Msg Id */
    if(SM_type_device == deviceType)
    {
        waitingForMsgID = (SMMsgs_cmdIds_t)(SMMsgs_cmdIds_processInfo128CodeNonce + SM_authItrVal);
    }
    else if(SM_type_coordinator == deviceType)
    {
        if((SM_authItrVal+1) == SM_NUM_AUTH_ITERATIONS)
        {
            waitingForMsgID = SMMsgs_cmdIds_processInfo128SecurityVal;
        }
        else
        {
            waitingForMsgID = (SMMsgs_cmdIds_t)(SMMsgs_cmdIds_processInfo128CodeNonce + (SM_authItrVal + 1));
        }
    }

    /* send code to the other party */
    SMSendInfo(Smgs_cmdIds_CommissionMsg,
                (SMMsgs_cmdIds_t)(SMMsgs_cmdIds_processInfo128CodeNonce + SM_authItrVal) ,
                tempBuf, SM_AUTH_CODE_NONCE_LEN, SM_DATA_MSDU_HANDLE, false);

    /* Start timeout for commissioning response */
    setSMProcessTimeoutClock(SM_COMMISIONING_STATUS_TIMEOUT_VALUE);

}//end of SMProcessAuthState

/* New Key Lookup Data generation for Key Identifier mode 3  */
/* input param:     sensor ext Address (pointer),
 *
 * ouput param:     Key lookup data array (Note: the arrary size must be APIMAC_KEY_SOURCE_MAX_LEN bytes)
 */
static void genNewKeyLookUpData4KeyMode3(ApiMac_sAddrExt_t extAddress, uint8_t *lookUp) {

    OsalPort_memcpy(lookUp, extAddress, sizeof(ApiMac_sAddrExt_t));
    lookUp[APIMAC_KEY_SOURCE_MAX_LEN-1] = 1;
}

/* Key refreshment: Key Lookup Data generation for Key Identifier mode 3  */
/* input param:     existing Key lookup data array (Note: the arrary size must be APIMAC_KEY_SOURCE_MAX_LEN bytes)
 *
 * ouput param:     Updated Key lookup data array (Note: the arrary size must be APIMAC_KEY_SOURCE_MAX_LEN bytes)
 * Note: KeyLookUpdata = MSB 7 bytes of ext. address + keyID (starting from 1)
 */
static bool genRefreshKeyLookUpData4KeyMode3(uint8_t *lookUp) {

    uint8_t KeyID = lookUp[APIMAC_KEY_SOURCE_MAX_LEN-1];

    if (KeyID == 0)
        return (false);
    else if (KeyID == 0xFF)
        KeyID = 1;          // key id starts from 1
    else
        KeyID ++;

    lookUp[APIMAC_KEY_SOURCE_MAX_LEN-1] = KeyID;

    return (true);
}

/* Add SeedKey Entry to the SeedKeyTable */
/* input param:     device ext. Address, localSecDevice
 *
 * ouput param:     true = added to the table, false = either already exists or mem allocation fail
 */
SM_AddResult_t addEntry2SeedKeyTable (ApiMac_sAddrExt_t extAddress, uint16_t shortAddress, uint8_t *seedKey)
{

    /* Load list head as the start point of the iterator. It is always the default key. */
    SM_seedKey_Entry_t* pSeedKeyEntry = (SM_seedKey_Entry_t*) List_head(&SeedKeyTableList);
    bool found = false;
    uint8_t listIndex =1;
    SM_AddResult_t ret = SM_Key_Mem_Error;

    while(1) {
        /* Get the next entry */
        pSeedKeyEntry = (SM_seedKey_Entry_t*)List_next((List_Elem *)pSeedKeyEntry);
        if (pSeedKeyEntry) {
            if (OsalPort_memcmp(pSeedKeyEntry->extAddress, extAddress, sizeof(ApiMac_sAddrExt_t)) == TRUE)
            {
                /* Matching entry already exists  */
                found = true;
                break;
            }
            else {
                listIndex ++;
                continue;
            }
        }
        else {
            /* No matching entry */
            break;
        }
    }
    if(found == false)
    {
        /* Add a new entry */
        pSeedKeyEntry =(SM_seedKey_Entry_t *)OsalPort_malloc(sizeof (SM_seedKey_Entry_t));
        if (pSeedKeyEntry) {

            ret = SM_Key_New;
            /* Generate key lookup data */
            genNewKeyLookUpData4KeyMode3(extAddress,&pSeedKeyEntry->lookUpData[0]);
            /* Copy ext Address */
            OsalPort_memcpy(pSeedKeyEntry->extAddress, extAddress, sizeof(ApiMac_sAddrExt_t));
            /* Copy shared secret*/
            OsalPort_memcpy(pSeedKeyEntry->seedKey, seedKey,SM_ECC_PUBLIC_KEY_SIZE);
            /* Sensor short address */
            pSeedKeyEntry->shortAddress = shortAddress;
            pSeedKeyEntry->index = listIndex;

            /* Put this seed key entry to the table */
            List_put(&SeedKeyTableList, (List_Elem *)pSeedKeyEntry);

        }
        else {
            /* Allocation error */
            Main_assertHandler(SM_MEMALLOC_ERROR);
            return(ret);
        }
    }
    else
    {
        ret = SM_Key_Exist;
        /* Generate key lookup data */
        if (keyRefreshmentProcess == true)
        {
            genRefreshKeyLookUpData4KeyMode3(&pSeedKeyEntry->lookUpData[0]);
        }
        else
        {
            genNewKeyLookUpData4KeyMode3(extAddress,&pSeedKeyEntry->lookUpData[0]);
        }

        /* Copy ext Address */
        OsalPort_memcpy(pSeedKeyEntry->extAddress, extAddress, sizeof(ApiMac_sAddrExt_t));

        /* Copy shared secret*/
        OsalPort_memcpy(pSeedKeyEntry->seedKey, seedKey,SM_ECC_PUBLIC_KEY_SIZE);
        /* Sensor short address */
        pSeedKeyEntry->shortAddress = shortAddress;
    }

    return(ret);
}



#ifdef FEATURE_FULL_FUNCTION_DEVICE

/*!
 * @brief       Initialize the key refresh clock.
 *              At the event, a key refreshment will be sent to one device
 */
static void initializeKeyRefreshClock(void)
{
    /* Initialize SM timeout timer */
    smKeyRefreshTimeoutClkHandle = UtilTimer_construct(&smKeyRefreshTimeoutClkStruct,
                                    keyRefreshTimeoutCallback,
                                    keyRefreshTimeout,
                                    0,
                                    false,
                                    0);
}

/*!
 * @brief       key refreshment timeout callback.
 *
 * @param       a0 - ignored
 */
static void keyRefreshTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&SM_events, SM_KEY_REFRESH_EVT);

    sem_post(hSMappSem);
}

/*!
 * @brief       Set key refresh timeout
 *
 * @param       a0 - ignored
 */
static void setKeyRefreshTimeoutClock(uint32_t timeout)
{
    /* Stop the SM timeout timer */
    if(UtilTimer_isActive(&smKeyRefreshTimeoutClkStruct) == true)
    {
        UtilTimer_stop(&smKeyRefreshTimeoutClkStruct);
    }

    if(timeout != 0)
    {
        /* Setup timer */
        UtilTimer_setTimeout(smKeyRefreshTimeoutClkHandle, timeout);
        UtilTimer_start(&smKeyRefreshTimeoutClkStruct);
    }
}

static void keyRecoverProcess (void)
{
    uint8_t i;
    bool found = false;
    Llc_deviceListItem_t item;
    /* Duration value for re-opening network */
    uint32_t duration = 0xFFFFFFFF;

    if(SM_Current_State == SM_CM_InProgress)
    {
        return;
    }

    /*get the device short address to do CM */
    for(i = 0; i < keyRecoverDeviceNumber; i++)
    {
        if( Cllc_associatedDevList[i].reCM_status == SM_RE_CM_REQUIRED)
        {
            /* the device index in NV is identical to index in Cllc_associatedDevList */
            Csf_getDeviceItem(i, &item, NULL);
            found = true;
            break;
        }
    }

    if(found == true)
    {
        /* First commissioning attempt for new device */
        SM_cmAttempts = 0;

        ApiMac_sec_t DeviceSecurityInfo;
        Cllc_securityFill(&DeviceSecurityInfo);

        SM_startCMProcess(&item.devInfo, &DeviceSecurityInfo,CONFIG_FH_ENABLE, item.capInfo.rxOnWhenIdle,
                          SM_type_coordinator, SM_COLLECTOR_AUTH_METHODS);
        Cllc_associatedDevList[i].reCM_status = SM_RE_CM_ATTEMPTED;
    }
    else
    {
        /* recovering key is done */
        fCommissionRequired = false;
#ifndef CUI_DISABLE
        CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                                 "Key Recovery is done");
#endif /* CUI_DISABLE */

        /* turn back on commissioning if no more devices need to be re-commissioned */
        Cllc_setJoinPermit(duration);
    }
}

static void allDevice_keyRefresh (void)
{
    uint8_t i;
    bool found = false;
    Llc_deviceListItem_t item;
    ApiMac_sAddr_t devAddr;

    if(SM_Current_State == SM_CM_InProgress)
    {
        return;
    }

    /*get the device short address to do CM */
    for(i = 0; i < CONFIG_MAX_DEVICES; i++)
    {
        if((Cllc_associatedDevList[i].keyRef_statue == SM_KEYRF_REQUIRED) &&
           ((Cllc_associatedDevList[i].reCM_status != SM_RE_CM_REQUIRED) ||
            (Cllc_associatedDevList[i].reCM_status != SM_RE_CM_PENDING)))

        {
            /* Find the device using short address */
            devAddr.addrMode = ApiMac_addrType_short;
            devAddr.addr.shortAddr = Cllc_associatedDevList[i].shortAddr;
            Csf_getDevice(&devAddr,&item);
            found = true;
            break;
        }
    }

    if(found == true)
    {
        /* Block association during key refreshment process */
        uint32_t duration=0;
        Cllc_setJoinPermit(duration);

        ApiMac_sec_t DeviceSecurityInfo;
        Cllc_securityFill(&DeviceSecurityInfo);

        SM_startKeyRefreshProcess(&item.devInfo, &DeviceSecurityInfo,CONFIG_FH_ENABLE, item.capInfo.rxOnWhenIdle);

        Cllc_associatedDevList[i].keyRef_statue = SM_KEYRF_ATTEMPTED;
        Cllc_associatedDevList[i].NumOfKeyRefresh ++;

    }
    else
    {
        /*every device has been key refreshed, now do the whole cycle again*/
        for(i = 0; i < CONFIG_MAX_DEVICES; i++)
        {
            if( Cllc_associatedDevList[i].keyRef_statue == SM_KEYRF_ATTEMPTED)
            {
                Cllc_associatedDevList[i].keyRef_statue = SM_KEYRF_REQUIRED;
            }
        }
    }
}

/*!
 * @brief      Find the associated device table entry matching an
 *             extended address.
 *
 * @param      shortAddr - device's short address
 *
 * @return     pointer to the associated device table entry,
 *             NULL if not found.
 */
static Cllc_associated_devices_t *findDevice(uint16_t shortAddr)
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
#else
/* Add SeedKey Entry to the SeedKeyTable */
/* input param:     device ext. Address, localSecDevice
 *
 * ouput param:     true = added to the table, false = either already exists or mem allocation fail
 */
SM_AddResult_t updateSeedKeyFromNV (ApiMac_deviceDescriptor_t *pDevInfo,  nvDeviceKeyInfo_t *pDevKeyInfo)
{

    SM_seedKey_Entry_t* pSeedKeyEntry;
    SM_AddResult_t ret = SM_Key_Mem_Error;

    pSeedKeyEntry =(SM_seedKey_Entry_t *)OsalPort_malloc(sizeof (SM_seedKey_Entry_t));
    if (pSeedKeyEntry) {

        ret = SM_Key_New;
        /* Copy lookup data */
        OsalPort_memcpy(pSeedKeyEntry->lookUpData, pDevKeyInfo->lookUpData, APIMAC_KEY_SOURCE_MAX_LEN);
        /* Copy shared secret*/
        OsalPort_memcpy(pSeedKeyEntry->seedKey, pDevKeyInfo->seedKey, SM_ECC_PUBLIC_KEY_SIZE);
        /* Copy index */
        pSeedKeyEntry->index = pDevKeyInfo->index;
        /* Copy ext Address */
        OsalPort_memcpy(pSeedKeyEntry->extAddress, pDevInfo->extAddress, sizeof(ApiMac_sAddrExt_t));
        /* Sensor short address */
        pSeedKeyEntry->shortAddress = pDevInfo->shortAddress;

        /* Put this seed key entry to the table */
        List_put(&SeedKeyTableList, (List_Elem *)pSeedKeyEntry);

    }
    else {
        /* Allocation error */
        Main_assertHandler(SM_MEMALLOC_ERROR);
        return(ret);
    }

    return(ret);
}


/* Recover SeedKey Entry
 *
 */
void SM_recoverKeyInfo(ApiMac_deviceDescriptor_t devInfo, Llc_netInfo_t parentInfo, nvDeviceKeyInfo_t devKeyInfo)
{
    SM_seedKey_Entry_t * pSeedKeyEnty;

    /* Update the seedKeyTable and MAC Key Table */
    /* Use its own ext address */
    updateSeedKeyFromNV(&devInfo,&devKeyInfo);
    pSeedKeyEnty = getEntryFromSeedKeyTable(devInfo.extAddress,devInfo.shortAddress);
    /* Do not change the order below to lines */
    /* Copy collector ext Address first */
    memcpy(commissionDevInfo.extAddress, parentInfo.devInfo.extAddress, sizeof(ApiMac_sAddrExt_t));
    addDeviceKey(pSeedKeyEnty,devKeyInfo.deviceKey, true);

#ifndef CUI_DISABLE
    CUI_statusLinePrintf(securityCuiHndl, securityStatusLine,
                         "KeyInfo recovered", 6);
#endif /* CUI_DISABLE */
}

#endif /*FEATURE_FULL_FUNCTION_DEVICE*/

/* Set the passkey */
/* input param:     passkey
 *
 * ouput param:     none
 */
void SM_setPasskey (uint32_t passkey)
{
    //Check if passkey is being requested
    if ( (UtilTimer_isActive(&smProcessTimeoutClkStruct)) &&
         (SMMsgs_authMethod_passkey == agreedUponAuthMethod) )
    {
        SM_authPassKey = passkey;
        awaitingPasskey = false;
        stopSMProcessTimeoutClock();
        /* set event */
        Util_setEvent(&SM_events, SM_PASSKEY_EVT);
    }
}
#endif /*FEATURE_SECURE_COMMISSIONING*/
