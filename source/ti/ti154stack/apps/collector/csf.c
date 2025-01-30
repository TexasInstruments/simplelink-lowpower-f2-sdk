
/******************************************************************************

 @file csf.c

 @brief Collector Specific Functions

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
#ifndef FREERTOS_SUPPORT
#include <xdc/std.h>
#endif
#include <string.h>
#include <stdlib.h>

#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <pthread.h>
#include <semaphore.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>
#include <inc/hw_ints.h>
#include <aon_event.h>
#include <ioc.h>

#include "ti_drivers_config.h"

#include "util_timer.h"
#include "mac_util.h"
#include "hal_types.h"

#ifndef CUI_DISABLE
#include "cui.h"
#endif /* CUI_DISABLE */

#include "macconfig.h"

#include "nvocmp.h"
#include "api_mac.h"
#include "collector.h"
#include "cllc.h"
#include "csf.h"
#include "ti_154stack_config.h"

#if defined(MT_CSF)
#include "mt_csf.h"
#endif

#ifdef OSAL_PORT2TIRTOS
#include "osal_port.h"
#else
#include "icall.h"
#endif

#ifdef USE_DMM
#ifdef FEATURE_SECURE_COMMISSIONING
#include "remote_display.h"
#include <sm_commissioning_gatt_profile.h>
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /* USE_DMM */

#if defined(DEVICE_TYPE_MSG)
#include <ti/devices/DeviceFamily.h>
#include "device_type.h"
#endif /* DEVICE_TYPE_MSG */

#ifdef MAC_STATS
#include "mac.h"
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* Milliseconds to seconds conversion */
#define MSEC_PER_SEC 1000

/* Initial timeout value for the tracking clock */
#define TRACKING_INIT_TIMEOUT_VALUE 100

/* NV Item ID - the device's network information */
#define CSF_NV_NETWORK_INFO_ID 0x0001
/* NV Item ID - the number of device list entries */
#define CSF_NV_DEVICELIST_ENTRIES_ID 0x0004
/* NV Item ID - the device list, use sub ID for each record in the list */
#define CSF_NV_DEVICELIST_ID 0x0005
/* NV Item ID - this devices frame counter */
#define CSF_NV_FRAMECOUNTER_ID 0x0006
/* NV Item ID - reset reason */
#define CSF_NV_RESET_REASON_ID 0x0007

/* Maximum number of device list entries */
#define CSF_MAX_DEVICELIST_ENTRIES CONFIG_MAX_DEVICES

/*
 Maximum sub ID for a device list item, this is failsafe.  This is
 not the maximum number of items in the list
 */
#define CSF_MAX_DEVICELIST_IDS (2*CONFIG_MAX_DEVICES)

/* timeout value for trickle timer initialization */
#define TRICKLE_TIMEOUT_VALUE       20

/* timeout value for join timer */
#define JOIN_TIMEOUT_VALUE       20
/* timeout value for config request delay */
#define CONFIG_TIMEOUT_VALUE 1000

#ifdef IEEE_COEX_METRICS
/* timeout value for the coex metrics timer */
#define COEX_METRICS_TIMEOUT_VALUE  120000
#endif

#ifdef MAC_STATS
/* timeout value for the MAC statistics timer */
#define MAC_STATS_TIMEOUT_VALUE  10000
#endif

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#define COEX_PACKETS_TO_SEND    10000
#endif

#if defined(USE_DMM)
#define FH_ASSOC_TIMER              2000
#define PROVISIONING_ASSOC_TIMER    1000
#endif /* USE_DMM */

/*
 The increment value needed to save a frame counter. Example, setting this
 constant to 100, means that the frame counter will be saved when the new
 frame counter is 100 more than the last saved frame counter.  Also, when
 the get frame counter function reads the value from NV it will add this value
 to the read value.
 */
#define FRAME_COUNTER_SAVE_WINDOW     25

/* Value returned from findDeviceListIndex() when not found */
#define DEVICE_INDEX_NOT_FOUND  -1

/*! NV driver item ID for reset reason */
#define NVID_RESET {NVINTF_SYSID_APP, CSF_NV_RESET_REASON_ID, 0}

#ifndef CUI_DISABLE
// ARIB SysConfig/MAC channel 0 corresponds to channel bundle 24,25
#define CHANNEL_BUNDLE_OFFSET_920MHZ 24
/* Mask of all supported channels for input validation */
static const uint8_t validChannelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ] = CUI_VALID_CHANNEL_MASK;
#endif /* CUI_DISABLE */

/******************************************************************************
 External variables
 *****************************************************************************/

#ifdef NV_RESTORE
/*! MAC Configuration Parameters */
extern mac_Config_t Main_user1Cfg;
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
/* Security manager latest state */
extern SM_lastState_t SM_Current_State;

/* Need to re-do commissioning*/
extern bool fCommissionRequired;
#endif /* FEATURE_SECURE_COMMISSIONING */
/******************************************************************************
 Local variables
 *****************************************************************************/

/* The application's semaphore */
#ifdef OSAL_PORT2TIRTOS
static sem_t *collectorSem;
#else
static ICall_Semaphore &collectorSem;
#endif

/* Clock/timer resources */
static ClockP_Struct trackingClkStruct;
static ClockP_Handle trackingClkHandle;
static ClockP_Struct broadcastClkStruct;
static ClockP_Handle broadcastClkHandle;

#ifdef IEEE_COEX_METRICS
static ClockP_Struct coexClkStruct;
static ClockP_Handle coexClkHandle;
#endif

#ifdef MAC_STATS
static ClockP_Struct statsClkStruct;
static ClockP_Handle statsClkHandle;
#endif


/* Clock/timer resources for CLLC */
/* trickle timer */
STATIC ClockP_Struct tricklePAClkStruct;
STATIC ClockP_Handle tricklePAClkHandle;
STATIC ClockP_Struct tricklePCClkStruct;
STATIC ClockP_Handle tricklePCClkHandle;

/* timer for join permit */
STATIC ClockP_Struct joinClkStruct;
STATIC ClockP_Handle joinClkHandle;

/* timer for config request delay */
STATIC ClockP_Struct configClkStruct;
STATIC ClockP_Handle configClkHandle;

/* timer for LED blink timeout*/
STATIC ClockP_Struct identifyClkStruct;
STATIC ClockP_Handle identifyClkHandle;

#ifdef USE_DMM
STATIC ClockP_Struct provisioningClkStruct;
STATIC ClockP_Handle provisioningClkHandle;
#endif /* USE_DMM */

/* NV Function Pointers */
static NVINTF_nvFuncts_t *pNV = NULL;

static bool started = false;

/* The last saved coordinator frame counter */
static uint32_t lastSavedCoordinatorFrameCounter = 0;

#if defined(MT_CSF)
/*! NV driver item ID for reset reason */
static const NVINTF_itemID_t nvResetId = NVID_RESET;
#endif

#ifdef MAC_STATS
bool statsStarted;
#ifdef IEEE_COEX_ENABLED
uint32_t totalTime;
uint32_t prevTick;
uint32_t currTick;
#endif
#endif
/******************************************************************************
 Global variables
 *****************************************************************************/
/* Key press parameters */
Button_Handle Csf_keys = NULL;

/* pending Csf_events */
uint16_t Csf_events = 0;

/* Saved CLLC state */
Cllc_states_t savedCllcState = Cllc_states_initWaiting;

/* Permit join setting */
bool permitJoining = false;

#if !defined(POWER_MEAS)
static LED_Handle gGreenLedHandle;
static LED_Handle gRedLedHandle;
#endif /* !POWER_MEAS */
static Button_Handle gRightButtonHandle;
static Button_Handle gLeftButtonHandle;

#ifndef CUI_DISABLE
CUI_clientHandle_t csfCuiHndl;
uint32_t collectorStatusLine;
uint32_t deviceStatusLine;
uint32_t numJoinDevStatusLine;
#ifdef LPSTK
uint32_t lpstkDataStatusLine;
#endif
#ifdef IEEE_COEX_METRICS
uint32_t coexStatusLine;
#endif
#ifdef MAC_STATS
uint32_t macStatsLine1;
#ifdef IEEE_COEX_ENABLED
uint32_t macStatsLine2;
uint32_t macStatsLine3;
#ifdef IEEE_COEX_TEST
uint32_t appStatsLine1;
uint32_t appStatsLine2;

extern uint32_t TrackMsgsSent;
extern uint32_t TrackMsgRetries;
extern uint32_t TrackMsgFail;
/*! total number of Track data messages sent. */
extern uint32_t TrackMsgsSuccess;
/*!
Average end to end delay
*/
extern uint16_t avgE2EDelay;
/*!
Worst Case end to end delay
*/
extern uint16_t worstCaseE2EDelay;

/* accumulated total E2E delay */
extern uint32_t totalE2EDelaySum;
#endif // IEEE_COEX_TEST
#endif
#endif

#endif /* CUI_DISABLE */

static uint16_t SelectedSensor;
static uint32_t reportInterval;
static uint8_t Csf_sensorAction;

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
static uint32_t numPacket;
static uint32_t trackingInterval;
static uint32_t trackingPktSize;
#endif


/******************************************************************************
 Local function prototypes
 *****************************************************************************/

static void processTrackingTimeoutCallback(uintptr_t a0);
static void processBroadcastTimeoutCallback(uintptr_t a0);
static void processKeyChangeCallback(Button_Handle _buttonHandle, Button_EventMask _buttonEvents);
static void processPATrickleTimeoutCallback(uintptr_t a0);
static void processPCTrickleTimeoutCallback(uintptr_t a0);
static void processJoinTimeoutCallback(uintptr_t a0);
static void processConfigTimeoutCallback(uintptr_t a0);
static void processIdentifyTimeoutCallback(uintptr_t a0);
#ifdef IEEE_COEX_METRICS
static void processCoexTimeoutCallback(uintptr_t a0);
static void readCoexMetrics(void);
#endif
#ifdef MAC_STATS
static void processStatsTimeoutCallback(uintptr_t a0);
static void readMacStats(void);
#endif
#ifndef CUI_DISABLE
static uint16_t getNumActiveDevices(void);
#endif /* CUI_DISABLE */
#if defined(USE_DMM)
static void processProvisioningCallback(uintptr_t a0);
#endif /* USE_DMM */
static bool addDeviceListItem(Llc_deviceListItem_t *pItem, bool *pNewDevice);
static void updateDeviceListItem(Llc_deviceListItem_t *pItem);
static int findDeviceListIndex(ApiMac_sAddrExt_t *pAddr);
static int findUnusedDeviceListIndex(void);
static void saveNumDeviceListEntries(uint16_t numEntries);
#if defined(TEST_REMOVE_DEVICE)
static void removeTheFirstDevice(void);
#else
#ifndef POWER_MEAS
static uint16_t getTheFirstDevice(void);
#endif
#endif

#ifndef CUI_DISABLE
static void updateCollectorStatusLine(bool restored, Llc_netInfo_t *pNetworkInfo);
static uint8_t moveCursorLeft(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space);
static uint8_t moveCursorRight(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space);
static void setPanIdAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void validateChMask(uint8_t *_chanMask, uint8_t byteIdx);
static void setChMaskAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#if CONFIG_FH_ENABLE
static void setAsyncChMaskAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
#ifdef FEATURE_MAC_SECURITY
static void setNwkKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
#ifdef FEATURE_SECURE_COMMISSIONING
static void setSmPassKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif /* FEATURE_SECURE_COMMISSIONING */
static void formNwkAction(int32_t menuEntryInex);
static void openNwkAction(int32_t menuEntryInex);
static void closeNwkAction(int32_t menuEntryInex);
static void sensorSelectAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void sensorSetReportInterval(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void sensorLedToggleAction(int32_t menuEntryInex);
static void sensorDisassocAction(int32_t menuEntryInex);
#ifdef IEEE_COEX_METRICS
static void coexMetricsRequestAction(int32_t menuEntryInex);
static void coexMetricsResetAction(int32_t menuEntryInex);
#endif
#ifdef MAC_STATS
static void macStatsRequestAction(int32_t menuEntryInex);
static void macStatsResetAction(int32_t menuEntryInex);
#endif
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
static void setCoexNumPacket(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void setCoexPacketSize(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void setCoexTrackingInterval(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);

static void startCoexTest(int32_t menuEntryInex);
static void endCoexTest(int32_t menuEntryInex);
#endif

#if defined(DEVICE_TYPE_MSG)
static void sensorDeviceTypeRequestAction(int32_t menuEntryInex);
#endif /* DEVICE_TYPE_MSG */
static void processMenuUpdate(void);

static void uintToString (uint32_t value, char * str, uint8_t base, uint8_t num_of_digists, bool pad0, bool reverse);
#endif /* CUI_DISABLE */

#ifndef POWER_MEAS
static void sendToggleAndUpdateUser(uint16_t shortAddr);
#endif

/* POWER_MEAS protects the UART in these functions */
static void formNwkAndUpdateUser(void);
static void openCloseNwkAndUpdateUser(bool openNwkRequest);

#ifndef CUI_DISABLE
/* Menu */
#ifdef S_NS_154_ENABLED
#define SFF_MENU_TITLE " TI Collector (TFM) "
#else
#define SFF_MENU_TITLE " TI Collector "
#endif

#if CONFIG_FH_ENABLE
#define FH_MENU_ENABLED 1
#else
#define FH_MENU_ENABLED 0
#endif

#ifdef FEATURE_MAC_SECURITY
#define SECURITY_MENU_ENABLED 1
#else
#define SECURITY_MENU_ENABLED 0
#endif

#ifdef IEEE_COEX_METRICS
#define COEX_MENU_ENABLED 2
#else
#define COEX_MENU_ENABLED 0
#endif

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#define COEX_TRIGGER_MENU_ENABLED 1
#else
#define COEX_TRIGGER_MENU_ENABLED 0
#endif

CUI_SUB_MENU(configureSubMenu, "<      CONFIGURE      >", 2 + FH_MENU_ENABLED + SECURITY_MENU_ENABLED, csfMainMenu)
    CUI_MENU_ITEM_INT_ACTION(  "<      SET PANID      >", setPanIdAction)
    CUI_MENU_ITEM_INT_ACTION(  "<    SET CHAN MASK    >", setChMaskAction)
#if CONFIG_FH_ENABLE
    CUI_MENU_ITEM_INT_ACTION(  "<  SET AS CHAN MASK   >", setAsyncChMaskAction)
#endif
#ifdef FEATURE_MAC_SECURITY
    CUI_MENU_ITEM_INT_ACTION(  "<     SET NWK KEY     >", setNwkKeyAction)
#endif

CUI_SUB_MENU_END
CUI_SUB_MENU(commissionSubMenu,"<   NETWORK ACTIONS   >", 3 + COEX_MENU_ENABLED, csfMainMenu)
    CUI_MENU_ITEM_ACTION(      "<       FORM NWK      >", formNwkAction)
    CUI_MENU_ITEM_ACTION(      "<       OPEN NWK      >", openNwkAction)
    CUI_MENU_ITEM_ACTION(      "<       CLOSE NWK     >", closeNwkAction)
//    CUI_MENU_ITEM_ACTION(    "<    DISASSOCIATE     >", disassocAction)
#if defined(IEEE_COEX_METRICS)
    CUI_MENU_ITEM_ACTION(      "<  READ COEX METRICS  >", coexMetricsRequestAction)
    CUI_MENU_ITEM_ACTION(      "<  RESET COEX METRICS >", coexMetricsResetAction)
#endif /* IEEE_COEX_METRICS */
#if defined(MAC_STATS)
    CUI_MENU_ITEM_ACTION(      "<   READ MAC STATS    >", macStatsRequestAction)
    CUI_MENU_ITEM_ACTION(      "<   RESET MAC STATS   >", macStatsResetAction)
#endif /* MAC_STATS */
CUI_SUB_MENU_END

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
   CUI_SUB_MENU(coexSubMenu,"<      COEX MENU      >", 3 , csfMainMenu)
   //CUI_SUB_MENU(coexSubMenu,"<      COEX MENU      >", 5 , csfMainMenu)
    CUI_MENU_ITEM_INT_ACTION(  "< SET COEX NUM PACKET >", setCoexNumPacket)
    //CUI_MENU_ITEM_INT_ACTION(  "<  SET COEX PKT SIZE  >", setCoexPacketSize)
    //CUI_MENU_ITEM_INT_ACTION(  "<  SET COEX INTERVAL  >", setCoexTrackingInterval)
    CUI_MENU_ITEM_ACTION(      "<   START COEX TEST   >", startCoexTest)
    CUI_MENU_ITEM_ACTION(      "<    END COEX TEST    >", endCoexTest)
CUI_SUB_MENU_END
#endif
/* This menu will be registered/de-registered at run time to create
a sort of "popup" menu for passkey entry. Since it is de-registered,
it is also completely hidden from the user at all other times.
Note: MAX_REGISTERED_MENUS must be >= 2 for both of the main
menus in this file. */

#ifdef FEATURE_SECURE_COMMISSIONING
CUI_MAIN_MENU(smPassKeyMenu, "<       SM MENU       >", 1, processMenuUpdate)
    CUI_MENU_ITEM_INT_ACTION(  "<    ENTER PASSKEY    >", setSmPassKeyAction)
CUI_MAIN_MENU_END
#endif /* FEATURE_SECURE_COMMISSIONING */


#if defined(DEVICE_TYPE_MSG)
CUI_SUB_MENU(appSubMenu,       "<         APP         >", 5, csfMainMenu)
#else
CUI_SUB_MENU(appSubMenu,       "<         APP         >", 4, csfMainMenu)
#endif /* DEVICE_TYPE_MSG */

    CUI_MENU_ITEM_INT_ACTION(  "<     SELECT SENSOR   >", sensorSelectAction)
    CUI_MENU_ITEM_INT_ACTION(  "< SET REPORT INTERVAL >", sensorSetReportInterval)
    CUI_MENU_ITEM_ACTION(      "<     SEND TOGGLE     >", sensorLedToggleAction)
    CUI_MENU_ITEM_ACTION(      "< SEND DISASSOCIATION >", sensorDisassocAction)
#if defined(DEVICE_TYPE_MSG)
    CUI_MENU_ITEM_ACTION(      "<  SEND TYPE REQUEST  >", sensorDeviceTypeRequestAction)
#endif /* DEVICE_TYPE_MSG */
CUI_SUB_MENU_END

CUI_MAIN_MENU(csfMainMenu, SFF_MENU_TITLE, 3+COEX_TRIGGER_MENU_ENABLED, processMenuUpdate)
    CUI_MENU_ITEM_SUBMENU(configureSubMenu)
    CUI_MENU_ITEM_SUBMENU(commissionSubMenu)
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    CUI_MENU_ITEM_SUBMENU(coexSubMenu)
#endif
    CUI_MENU_ITEM_SUBMENU(appSubMenu)
CUI_MAIN_MENU_END
#endif /* CUI_DISABLE */

/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 The application calls this function during initialization

 Public function defined in csf.h
 */
void Csf_init(void *sem)
{
#ifndef CUI_DISABLE
    CUI_clientParams_t clientParams;
#endif /* CUI_DISABLE */
#ifdef NV_RESTORE
    /* Save off the NV Function Pointers */
    pNV = &Main_user1Cfg.nvFps;
#endif

    /* Save off the semaphore */
    collectorSem = (sem_t*)sem;

#ifndef CUI_DISABLE
    /* Open UI for key and LED */
    CUI_clientParamsInit(&clientParams);

    strncpy(clientParams.clientName, "154 Collector", MAX_CLIENT_NAME_LEN);
#ifdef LPSTK
    clientParams.maxStatusLines = 4;
#else
    clientParams.maxStatusLines = 3;
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
    clientParams.maxStatusLines++;
#endif /* FEATURE_SECURE_COMMISSIONING */
#ifdef SECURE_MANAGER_DEBUG
    clientParams.maxStatusLines++;
#endif /* SECURE_MANAGER_DEBUG */
#ifdef SECURE_MANAGER_DEBUG2
    clientParams.maxStatusLines++;
#endif /* SECURE_MANAGER_DEBUG2 */
#ifdef IEEE_COEX_METRICS
    clientParams.maxStatusLines++;
#endif /* IEEE_COEX_METRICS */
#ifdef MAC_STATS
#ifdef IEEE_COEX_ENABLED
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    clientParams.maxStatusLines+=5;
#else
    clientParams.maxStatusLines+=5;
#endif // IEEE_COEX_TEST && COEX_MENU
    totalTime = 0;
    prevTick = 0;
    currTick = 0;
#else
    clientParams.maxStatusLines++;
#endif
    statsStarted = false;
#endif /* MAC_STATS */

    csfCuiHndl = CUI_clientOpen(&clientParams);
#endif /* CUI_DISABLE */

#ifdef FEATURE_SECURE_COMMISSIONING
#ifndef CUI_DISABLE
    /* Initialize the security manager and register callbacks */
    SM_init(collectorSem, csfCuiHndl);
#else
    SM_init(collectorSem);
#endif /* CUI_DISABLE */
#endif //FEATURE_SECURE_COMMISSIONING

    /* Initialize Keys */
    Button_Params bparams;
    Button_Params_init(&bparams);
    gLeftButtonHandle = Button_open(CONFIG_BTN_LEFT, &bparams);
    Button_setCallback(gLeftButtonHandle, processKeyChangeCallback);
    // Open Right button without appCallBack
    gRightButtonHandle = Button_open(CONFIG_BTN_RIGHT, &bparams);

    // Read button state
    bool nvErased = false;
    if (!GPIO_read(((Button_HWAttrs*)gRightButtonHandle->hwAttrs)->gpioIndex))
    {
        /* Right key is pressed on power up, clear all NV */
        Csf_clearAllNVItems();
        nvErased = true;
    }
    // Set button callback
    Button_setCallback(gRightButtonHandle, processKeyChangeCallback);

#if !defined(POWER_MEAS)
    /* Initialize the LEDs */
    LED_Params ledParams;
    LED_Params_init(&ledParams);
    gGreenLedHandle = LED_open(CONFIG_LED_GREEN, &ledParams);
    gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);

    // Blink to indicate the application started up correctly
    LED_startBlinking(gRedLedHandle, 500, 3);
#endif /* POWER_MEAS */

#ifndef CUI_DISABLE
    CUI_registerMenu(csfCuiHndl, &csfMainMenu);

    CUI_statusLineResourceRequest(csfCuiHndl, "Status", false, &collectorStatusLine);
#ifdef IEEE_COEX_METRICS
    CUI_statusLineResourceRequest(csfCuiHndl, "Coex Status", true, &coexStatusLine);
#endif
#ifdef MAC_STATS
    CUI_statusLineResourceRequest(csfCuiHndl, "MAC Stats 1", true, &macStatsLine1);
#ifdef IEEE_COEX_ENABLED
    CUI_statusLineResourceRequest(csfCuiHndl, "MAC Stats 2", true, &macStatsLine2);
    CUI_statusLineResourceRequest(csfCuiHndl, "MAC Stats 3", true, &macStatsLine3);
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    CUI_statusLineResourceRequest(csfCuiHndl, "App Stats Tx", true, &appStatsLine1);
    CUI_statusLineResourceRequest(csfCuiHndl, "App Stats Rx", true, &appStatsLine2);
#endif // IEEE_COEX_TEST && COEX_MENU
#endif
#endif
    CUI_statusLineResourceRequest(csfCuiHndl, "Device Status", true, &deviceStatusLine);
#ifdef LPSTK
    CUI_statusLineResourceRequest(csfCuiHndl, "LPSTK Data", true, &lpstkDataStatusLine);
#endif /* LPSTK */
    CUI_statusLineResourceRequest(csfCuiHndl, "Number of Joined Devices", false, &numJoinDevStatusLine);

#if !defined(AUTO_START)
    CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine, "Waiting...");
#endif /* AUTO_START */
    if (nvErased) {
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "NV cleared");
    }
#endif /* CUI_DISABLE */


#if defined(MT_CSF)
    {
        uint8_t resetReseason = 0;

        if(pNV != NULL)
        {
            if(pNV->readItem != NULL)
            {
                /* Attempt to retrieve reason for the reset */
                (void)pNV->readItem(nvResetId, 0, 1, &resetReseason);
            }

            if(pNV->deleteItem != NULL)
            {
                /* Only use this reason once */
                (void)pNV->deleteItem(nvResetId);
            }
        }

        /* Start up the MT message handler */
        MTCSF_init(resetReseason, gLeftButtonHandle);

        /* Did we reset because of assert? */
        if(resetReseason > 0)
        {
#ifndef CUI_DISABLE
            CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine, "Restarting...");
#endif /* CUI_DISABLE */

            /* Tell the collector to restart */
            Csf_events |= CSF_KEY_EVENT;
            Csf_keys = gLeftButtonHandle;
        }
    }
#endif
}

/*!
 The application must call this function periodically to
 process any Csf_events that this module needs to process.

 Public function defined in csf.h
 */
void Csf_processEvents(void)
{
    /* Did a key press occur? LaunchPad only supports CONFIG_BTN_LEFT and CONFIG_BTN_RIGHT */
    if(Csf_events & CSF_KEY_EVENT)
    {
        /* Process the Left Key */
        if(Csf_keys == gLeftButtonHandle)
        {
            if(started == false)
            {
                /* Tell the collector to start */
                formNwkAndUpdateUser();
            }
            else
            {
#if defined(TEST_REMOVE_DEVICE)
                /*
                 Remove the first device found in the device list.
                 Nobody would do something like this, it's just
                 and example on the use of the device list and remove
                 function.
                 */
                removeTheFirstDevice();
#else
                /*
                 Send a Toggle LED request to the first device
                 in the device list if left button is pressed
                 */
#ifndef POWER_MEAS
                ApiMac_sAddr_t firstDev;
                firstDev.addr.shortAddr = getTheFirstDevice();
                sendToggleAndUpdateUser(firstDev.addr.shortAddr);
#endif /* POWER_MEAS */
#endif /* TEST_REMOVE_DEVICE */
            }
        }

        /* Process the Right Key */
        if(Csf_keys == gRightButtonHandle)
        {
            openCloseNwkAndUpdateUser(!permitJoining);
        }
        /* Clear the key press indication */
        Csf_keys = NULL;

        /* Clear the event */
        Util_clearEvent(&Csf_events, CSF_KEY_EVENT);
    }

    if(Csf_events & COLLECTOR_UI_INPUT_EVT)
    {
        /* Clear the event */
        Util_clearEvent(&Csf_events, COLLECTOR_UI_INPUT_EVT);
#ifndef CUI_DISABLE
        CUI_processMenuUpdate();
#endif /* CUI_DISABLE */
    }

    if(Csf_events & COLLECTOR_SENSOR_ACTION_EVT)
    {
        ApiMac_sAddr_t SelectedSensorAddr;

        SelectedSensorAddr.addrMode = ApiMac_addrType_short;
        SelectedSensorAddr.addr.shortAddr = SelectedSensor;

        switch(Csf_sensorAction)
        {
            case SENSOR_ACTION_SET_RPT_INT:
            {
                Collector_sendConfigRequest(
                        &SelectedSensorAddr, (CONFIG_FRAME_CONTROL),
                        reportInterval,
                        (CONFIG_POLLING_INTERVAL));
                break;
            }
            case SENSOR_ACTION_TOGGLE:
            {
                /* send Toggle if CUI */
#ifndef POWER_MEAS
                sendToggleAndUpdateUser(SelectedSensor);
#endif /* endif for POWER_MEAS */
                break;
            }
            case SENSOR_ACTION_DISASSOC:
            {
                if(Csf_sendDisassociateMsg(SelectedSensorAddr.addr.shortAddr) != 0)
                {
#ifndef CUI_DISABLE
                    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Disassociate Sensor Error - Addr=0x%04x not found",
                                         SelectedSensorAddr.addr.shortAddr);
#endif /* CUI_DISABLE */
                }
                break;
            }
#if defined(DEVICE_TYPE_MSG)
            case SENSOR_ACTION_DEVICE_TYPE_REQ:
            {
                Collector_sendDeviceTypeRequest(&SelectedSensorAddr);
                break;
            }
#endif /* endif for DEVICE_TYPE_MSG */
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
            case COEX_ACTION_SET_NUM_PACKET:
            {
                //Set the number of packets to be sent
                //variable is numPackets. Pass this into your setting function

            }
            case COEX_ACTION_SET_PKT_SIZE:
            {


            }
            case COEX_ACTION_SET_TRACK_INT:
            {


            }
            case COEX_ACTION_START:
            {
                //Start Coex send
            }
            case COEX_ACTION_END:
            {
                //end coex send
            }
#endif
            default:
                break;
        }

        /* Clear the event */
        Util_clearEvent(&Csf_events, COLLECTOR_SENSOR_ACTION_EVT);
    }

#ifdef IEEE_COEX_METRICS
    if(Csf_events & COEX_IEEE_METRICS_EVT)
    {
        /* Read and print coex metrics */
        readCoexMetrics();

        /* Clear the event */
        Util_clearEvent(&Csf_events, COEX_IEEE_METRICS_EVT);
    }
#endif /* endif for IEEE_COEX_METRICS */

#ifdef MAC_STATS
    if(Csf_events & MAC_STATS_EVT)
    {
        /* Read and print coex metrics */
        readMacStats();

        /* Clear the event */
        Util_clearEvent(&Csf_events, MAC_STATS_EVT);
    }
#endif /* endif for MAC_STATS */

#if defined(MT_CSF)
    MTCSF_displayStatistics();
#endif
}

/*!
 The application calls this function to retrieve the stored
 network information.

 Public function defined in csf.h
 */
bool Csf_getNetworkInformation(Llc_netInfo_t *pInfo)
{
    if((pNV != NULL) && (pNV->readItem != NULL) && (pInfo != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_NETWORK_INFO_ID;
        id.subID = 0;

        /* Read Network Information from NV */
        if(pNV->readItem(id, 0, sizeof(Llc_netInfo_t), pInfo) == NVINTF_SUCCESS)
        {
            return(true);
        }
    }
    return(false);
}

/*!
 The application calls this function to indicate that it has
 started or restored the device in a network

 Public function defined in csf.h
 */
void Csf_networkUpdate(bool restored, Llc_netInfo_t *pNetworkInfo)
{
    /* check for valid structure pointer, ignore if not */
    if(pNetworkInfo != NULL)
    {
        if((pNV != NULL) && (pNV->writeItem != NULL))
        {
            NVINTF_itemID_t id;

            /* Setup NV ID */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_NETWORK_INFO_ID;
            id.subID = 0;

            /* Write the NV item */
            pNV->writeItem(id, sizeof(Llc_netInfo_t), pNetworkInfo);
        }

        started = true;
#ifndef CUI_DISABLE
        updateCollectorStatusLine(restored, pNetworkInfo);
#endif /* CUI_DISABLE */
#ifndef POWER_MEAS
        LED_stopBlinking(gGreenLedHandle);
        LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
#endif

#if defined(MT_CSF)
        MTCSF_networkUpdateIndCB();
#endif /* endif for MT_CSF */
    }
}


/*!
 The application calls this function to indicate that a device
 has joined the network.

 Public function defined in csf.h
 */
ApiMac_assocStatus_t Csf_deviceUpdate(ApiMac_deviceDescriptor_t *pDevInfo,
                                      ApiMac_capabilityInfo_t *pCapInfo)
{
    ApiMac_assocStatus_t status = ApiMac_assocStatus_success;

    /* flag which will be updated based on if the device joining is
     a new device or already existing one */
    bool newDevice;

    /* Save the device information */
    Llc_deviceListItem_t dev;

    memcpy(&dev.devInfo, pDevInfo, sizeof(ApiMac_deviceDescriptor_t));
    memcpy(&dev.capInfo, pCapInfo, sizeof(ApiMac_capabilityInfo_t));
    dev.rxFrameCounter = 0;

    if(addDeviceListItem(&dev, &newDevice) == false)
    {
#ifdef NV_RESTORE
        status = ApiMac_assocStatus_panAtCapacity;

#ifndef CUI_DISABLE
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Failed - 0x%04x ", pDevInfo->shortAddress);
#endif /* CUI_DISABLE */
#else
        status = ApiMac_assocStatus_success;
#ifndef CUI_DISABLE
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Joined - 0x%04x ", pDevInfo->shortAddress);
#endif /* CUI_DISABLE */
#endif
    }
#ifndef CUI_DISABLE
    else if (true == newDevice) {
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Joined - 0x%04x ", pDevInfo->shortAddress);
    }
    else {
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Rejoined - 0x%04x ", pDevInfo->shortAddress);
    }

    CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());
#endif /* CUI_DISABLE */
#if defined(MT_CSF)
    MTCSF_deviceUpdateIndCB(pDevInfo, pCapInfo);
#endif

    /* Return the status of the joining device */
    return (status);
}

/*!
 The application calls this function to indicate that a device
 is no longer active in the network.

 Public function defined in csf.h
 */
void Csf_deviceNotActiveUpdate(ApiMac_deviceDescriptor_t *pDevInfo,
bool timeout)
{
#ifndef CUI_DISABLE
    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "!Responding - 0x%04x ", pDevInfo->shortAddress);
#endif /* CUI_DISABLE */
#if defined(MT_CSF)
    MTCSF_deviceNotActiveIndCB(pDevInfo, timeout);
#endif /* endif for MT_CSF */
}

/*!
 The application calls this function to indicate that a device
 has responded to a Config Request.

 Public function defined in csf.h
 */
void Csf_deviceConfigUpdate(ApiMac_sAddr_t *pSrcAddr, int8_t rssi,
                            Smsgs_configRspMsg_t *pMsg)
{
#ifndef CUI_DISABLE
    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "ConfigRsp - 0x%04x ", pSrcAddr->addr.shortAddr);
    CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());
#endif /* CUI_DISABLE */

#if defined(MT_CSF)
    MTCSF_configResponseIndCB(pSrcAddr, rssi, pMsg);
#endif /* endif for MT_CSF */
}

/*!
 The application calls this function to indicate that a device
 has reported sensor data.

 Public function defined in csf.h
 */
void Csf_deviceSensorDataUpdate(ApiMac_sAddr_t *pSrcAddr, int8_t rssi,
                                Smsgs_sensorMsg_t *pMsg)
{
#ifndef POWER_MEAS
    LED_toggle(gGreenLedHandle);
#endif /* endif for POWER_MEAS */
#ifndef CUI_DISABLE
    if(pMsg->frameControl & Smsgs_dataFields_bleSensor)
    {
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "ADDR:%2x%2x%2x%2x%2x%2x, UUID:0x%04x, "
                             "ManFac:0x%04x, Length:%d, Data:0x%02x", pMsg->bleSensor.bleAddr[5],
                             pMsg->bleSensor.bleAddr[4], pMsg->bleSensor.bleAddr[3], pMsg->bleSensor.bleAddr[2],
                             pMsg->bleSensor.bleAddr[1], pMsg->bleSensor.bleAddr[0], pMsg->bleSensor.uuid,
                             pMsg->bleSensor.manFacID, pMsg->bleSensor.dataLength, pMsg->bleSensor.data[0]);
    }
    else
    {
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Sensor - Addr=0x%04x, Temp=%d, RSSI=%d",
                             pSrcAddr->addr.shortAddr, pMsg->tempSensor.ambienceTemp, rssi);
#ifdef LPSTK
        CUI_statusLinePrintf(csfCuiHndl, lpstkDataStatusLine, "Humid=%d, Light=%d, Accl=(%d, %d, %d, %d, %d), Hall=%d",
                             pMsg->humiditySensor.humidity, pMsg->lightSensor.rawData,
                             pMsg->accelerometerSensor.xAxis, pMsg->accelerometerSensor.yAxis,
                             pMsg->accelerometerSensor.zAxis, pMsg->accelerometerSensor.xTiltDet,
                             pMsg->accelerometerSensor.yTiltDet,
                             pMsg->hallEffectSensor.fluxLevel);
#endif
    }
    CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());
#endif /* CUI_DISABLE */

#if defined(MT_CSF)
    MTCSF_sensorUpdateIndCB(pSrcAddr, rssi, pMsg);
#endif /* endif for MT_CSF */
}

/*!
 The application calls this function to indicate that a device
 has been disassociated.

 Public function defined in csf.h
 */
void Csf_deviceDisassocUpdate(uint16_t shortAddr)
{
#ifndef CUI_DISABLE
    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Disassociate Sensor - Addr=0x%04x", shortAddr);
    CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());
#endif /* CUI_DISABLE */
#ifdef FEATURE_SECURE_COMMISSIONING
#ifndef CUI_DISABLE
    CUI_deRegisterMenu(csfCuiHndl, &smPassKeyMenu);
    CUI_registerMenu(csfCuiHndl, &csfMainMenu);
    CUI_menuNav(csfCuiHndl, &csfMainMenu, csfMainMenu.numItems - 1);
#endif /* CUI_DISABLE */
#ifdef USE_DMM
    RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_IDLE);
#endif /* USE_DMM */
#endif /* FEATURE_SECURE_COMMISSIONING */
}

#if defined(DEVICE_TYPE_MSG) && !defined(CUI_DISABLE)
/*!
 * @brief       The application calls this function to print out the reported
 *              device type
 *
 * @param       deviceFamilyID - the integer ID of the device family
 * @param       deviceTypeID - the integer ID of the board/device
 *
 * Public function defined in csf.h
 */
void Csf_deviceSensorDeviceTypeResponseUpdate(uint8_t deviceFamilyID, uint8_t deviceTypeID)
{
    char* deviceStr;

    switch (deviceTypeID)
    {
        case DeviceType_ID_CC1310:
            deviceStr = "cc1310";
            break;
        case DeviceType_ID_CC1350:
            deviceStr = "cc1350";
            break;
        case DeviceType_ID_CC2640R2:
            deviceStr = "cc2640r2";
            break;
        case DeviceType_ID_CC1312R1:
            deviceStr = "cc1312r1";
            break;
        case DeviceType_ID_CC1352R1:
            deviceStr = "cc1352r1";
            break;
        case DeviceType_ID_CC1352P1:
            deviceStr = "cc1352p1";
            break;
        case DeviceType_ID_CC1352P_2:
            deviceStr = "cc1352p2";
            break;
        case DeviceType_ID_CC1352P_4:
            deviceStr = "cc1352p4";
            break;
        case DeviceType_ID_CC2642R1:
            deviceStr = "cc2642r1";
            break;
        case DeviceType_ID_CC2652R1:
            deviceStr = "cc2652r1";
            break;
        case DeviceType_ID_CC2652RB:
            deviceStr = "cc2652rb";
            break;
        default:
            deviceStr = "generic";
            break;
    }

    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Sensor - Device=%s, DeviceFamilyID=%i, DeviceTypeID=%i",
                         deviceStr, deviceFamilyID, deviceTypeID);

}

#endif /* CUI_DISABLE && DEVICE_TYPE_MSG */


/*!
 * @brief       The application calls this function to blink an LED in
 *              response to a identify request from a sensor
 *
 * @param       identifyTime - duration of LED blink (in seconds)
 *
 * Public function defined in csf.h
 */
void Csf_identifyLED(uint8_t identifyTime)
{
#ifndef POWER_MEAS
    LED_startBlinking(gGreenLedHandle, 200, LED_BLINK_FOREVER);
#endif

    /* Setup timer */
    UtilTimer_setTimeout(identifyClkHandle, identifyTime * MSEC_PER_SEC);
    UtilTimer_start(&identifyClkStruct);
}

/*!
 The application calls this function to indicate that a device
 set a Toggle LED Response message.

 Public function defined in csf.h
 */
void Csf_toggleResponseReceived(ApiMac_sAddr_t *pSrcAddr, bool ledState)
{
#if defined(MT_CSF)
    uint16_t shortAddr = 0xFFFF;

    if(pSrcAddr)
    {
        if(pSrcAddr->addrMode == ApiMac_addrType_short)
        {
            shortAddr = pSrcAddr->addr.shortAddr;
        }
        else
        {
            /* Convert extended to short addr */
            shortAddr = Csf_getDeviceShort(&pSrcAddr->addr.extAddr);
        }
    }
    MTCSF_deviceToggleIndCB(shortAddr, ledState);
#endif /* endif for MT_CSF */
}

/*!
 The application calls this function to indicate that the
 Coordinator's state has changed.

 Public function defined in csf.h
 */
void Csf_stateChangeUpdate(Cllc_states_t state)
{
    if(started == true)
    {
#ifndef POWER_MEAS
        /* always blink in FH mode because permit join is always on */
        if(state == Cllc_states_joiningAllowed || CONFIG_FH_ENABLE)
        {
            /* Flash LED1 while allowing joining */
            LED_startBlinking(gRedLedHandle, 500, LED_BLINK_FOREVER);
        }
        else if(state == Cllc_states_joiningNotAllowed)
        {
            /* Don't flash when not allowing joining */
            LED_stopBlinking(gRedLedHandle);
            LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
        }
#endif
    }

    /* Save the state to be used later */
    savedCllcState = state;

#if defined(MT_CSF)
    MTCSF_stateChangeIndCB(state);
#endif /* endif for MT_CSF */
}

/*!
 Initialize the tracking clock.

 Public function defined in csf.h
 */
void Csf_initializeTrackingClock(void)
{
    /* Initialize the timers needed for this application */
    trackingClkHandle = UtilTimer_construct(&trackingClkStruct,
                                        processTrackingTimeoutCallback,
                                        TRACKING_INIT_TIMEOUT_VALUE,
                                        0,
                                        false,
                                        0);
}

#ifdef IEEE_COEX_METRICS
/*!
 Initialize the coex clock.

 Public function defined in csf.h
 */
void Csf_initializeCoexClock(void)
{
    /* Initialize the timers needed for this application */
    coexClkHandle = UtilTimer_construct(&coexClkStruct,
                                        processCoexTimeoutCallback,
                                        COEX_METRICS_TIMEOUT_VALUE,
                                        COEX_METRICS_TIMEOUT_VALUE,
                                        false,
                                        0);
}
#endif

#ifdef MAC_STATS
/*!
 Initialize the coex clock.

 Public function defined in csf.h
 */
void Csf_initializeStatsClock(void)
{
    /* Initialize the timers needed for this application */
    statsClkHandle = UtilTimer_construct(&statsClkStruct,
                                        processStatsTimeoutCallback,
                                        MAC_STATS_TIMEOUT_VALUE,
                                        MAC_STATS_TIMEOUT_VALUE,
                                        false,
                                        0);
}
#endif

/*!
 Initialize the broadcast cmd clock.

 Public function defined in csf.h
 */
void Csf_initializeBroadcastClock(void)
{
    /* Initialize the timers needed for this application */
    broadcastClkHandle = UtilTimer_construct(&broadcastClkStruct,
                                        processBroadcastTimeoutCallback,
                                        TRACKING_INIT_TIMEOUT_VALUE,
                                        0,
                                        false,
                                        0);
}

/*!
 Initialize the trickle clock.

 Public function defined in csf.h
 */
void Csf_initializeTrickleClock(void)
{
    /* Initialize trickle timer */
    tricklePAClkHandle = UtilTimer_construct(&tricklePAClkStruct,
                                         processPATrickleTimeoutCallback,
                                         TRICKLE_TIMEOUT_VALUE,
                                         0,
                                         false,
                                         0);

    tricklePCClkHandle = UtilTimer_construct(&tricklePCClkStruct,
                                         processPCTrickleTimeoutCallback,
                                         TRICKLE_TIMEOUT_VALUE,
                                         0,
                                         false,
                                         0);
}

/*!
 Initialize the clock for join permit attribute.

 Public function defined in csf.h
 */
void Csf_initializeJoinPermitClock(void)
{
    /* Initialize join permit timer */
    joinClkHandle = UtilTimer_construct(&joinClkStruct,
                                    processJoinTimeoutCallback,
                                    JOIN_TIMEOUT_VALUE,
                                    0,
                                    false,
                                    0);
}

/*!
 Initialize the clock for config request delay

 Public function defined in csf.h
 */
void Csf_initializeConfigClock(void)
{
    /* Initialize join permit timer */
    configClkHandle = UtilTimer_construct(&configClkStruct,
                                    processConfigTimeoutCallback,
                                    CONFIG_TIMEOUT_VALUE,
                                    0,
                                    false,
                                    0);
}

/*!
 Initialize the clock for identify timeout

 Public function defined in csf.h
 */
void Csf_initializeIdentifyClock(void)
{
    /* Initialize identify clock timer */
    identifyClkHandle = UtilTimer_construct(&identifyClkStruct,
                                    processIdentifyTimeoutCallback,
                                    10,
                                    0,
                                    false,
                                    0);
}


/*!
 Set the tracking clock.

 Public function defined in csf.h
 */
void Csf_setTrackingClock(uint32_t trackingTime)
{
    /* Stop the Tracking timer */
    if(UtilTimer_isActive(&trackingClkStruct) == true)
    {
        UtilTimer_stop(&trackingClkStruct);
    }

    if(trackingTime)
    {
        /* Setup timer */
        UtilTimer_setTimeout(trackingClkHandle, trackingTime);
        UtilTimer_start(&trackingClkStruct);
    }
}

#ifdef IEEE_COEX_METRICS
/*!
 Set the coex clock.

 Public function defined in csf.h
 */
void Csf_setCoexClock(uint32_t coexTime)
{
    /* Stop the Coex timer */
    if(UtilTimer_isActive(&coexClkStruct) == true)
    {
        UtilTimer_stop(&coexClkStruct);
    }

    if(coexTime)
    {
        /* Setup timer */
        UtilTimer_setTimeout(coexClkHandle, coexTime);
        UtilTimer_start(&coexClkStruct);
    }
}
#endif

#ifdef MAC_STATS
/*!
 Set the mac stats clock.

 Public function defined in csf.h
 */
void Csf_setStatsClock(uint32_t statsTime)
{
    /* Stop the Coex timer */
    if(UtilTimer_isActive(&statsClkStruct) == true)
    {
        UtilTimer_stop(&statsClkStruct);
    }

    if(statsTime)
    {
        /* Setup timer */
        UtilTimer_setTimeout(statsClkHandle, statsTime);
        UtilTimer_start(&statsClkStruct);
    }
}
#endif

/*!
 Set the broadcast clock.

 Public function defined in csf.h
 */
void Csf_setBroadcastClock(uint32_t broadcastTime)
{
    /* Stop the Tracking timer */
    if(UtilTimer_isActive(&broadcastClkStruct) == true)
    {
        UtilTimer_stop(&broadcastClkStruct);
    }

    if(broadcastTime)
    {
        /* Setup timer */
        UtilTimer_setTimeout(broadcastClkHandle, broadcastTime);
        UtilTimer_start(&broadcastClkStruct);
    }
}

/*!
 Set the trickle clock.

 Public function defined in csf.h
 */
void Csf_setTrickleClock(uint32_t trickleTime, uint8_t frameType)
{
    uint16_t randomNum = 0;
    uint16_t randomTime = 0;

    if(trickleTime > 0)
    {
        randomNum = ((ApiMac_randomByte() << 8) + ApiMac_randomByte());
        randomTime = (trickleTime >> 1) +
                      (randomNum % (trickleTime >> 1));
    }

    if(frameType == ApiMac_wisunAsyncFrame_advertisement)
    {
        /* Stop the PA trickle timer */
        if(UtilTimer_isActive(&tricklePAClkStruct) == true)
        {
            UtilTimer_stop(&tricklePAClkStruct);
        }

        if(trickleTime > 0)
        {
            /* Setup timer */
            UtilTimer_setTimeout(tricklePAClkHandle, randomTime);
            UtilTimer_start(&tricklePAClkStruct);
        }
    }
    else if(frameType == ApiMac_wisunAsyncFrame_config)
    {
        /* Stop the PC trickle timer */
        if(UtilTimer_isActive(&tricklePCClkStruct) == true)
        {
            UtilTimer_stop(&tricklePCClkStruct);
        }

        if(trickleTime > 0)
        {
            /* Setup timer */
            UtilTimer_setTimeout(tricklePCClkHandle, randomTime);
            UtilTimer_start(&tricklePCClkStruct);
        }
    }
}

/*!
 Set the clock join permit attribute.

 Public function defined in csf.h
 */
void Csf_setJoinPermitClock(uint32_t joinDuration)
{
    /* Stop the join timer */
    if(UtilTimer_isActive(&joinClkStruct) == true)
    {
        UtilTimer_stop(&joinClkStruct);
    }

    if(joinDuration != 0)
    {
        /* Setup timer */
        UtilTimer_setTimeout(joinClkHandle, joinDuration);
        UtilTimer_start(&joinClkStruct);
    }
}

/*!
 Set the clock config request delay.

 Public function defined in csf.h
 */
void Csf_setConfigClock(uint32_t delay)
{
    /* Stop the join timer */
    if(UtilTimer_isActive(&configClkStruct) == true)
    {
        UtilTimer_stop(&configClkStruct);
    }

    if(delay != 0)
    {
        /* Setup timer */
        UtilTimer_setTimeout(configClkHandle, delay);
        UtilTimer_start(&configClkStruct);
    }
}



/*!
 Read the number of device list items stored

 Public function defined in csf.h
 */
uint16_t Csf_getNumDeviceListEntries(void)
{
    uint16_t numEntries = 0;

    if(pNV != NULL)
    {
        NVINTF_itemID_t id;
        uint8_t stat;

        /* Setup NV ID for the number of entries in the device list */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_DEVICELIST_ENTRIES_ID;
        id.subID = 0;

        /* Read the number of device list items from NV */
        stat = pNV->readItem(id, 0, sizeof(uint16_t), &numEntries);
        if(stat != NVINTF_SUCCESS)
        {
            numEntries = 0;
        }
    }
    return (numEntries);
}

/*!
 Find the short address from a given extended address

 Public function defined in csf.h
 */
uint16_t Csf_getDeviceShort(ApiMac_sAddrExt_t *pExtAddr)
{
    Llc_deviceListItem_t item;
    ApiMac_sAddr_t devAddr;
    uint16_t shortAddr = CSF_INVALID_SHORT_ADDR;

    devAddr.addrMode = ApiMac_addrType_extended;
    memcpy(&devAddr.addr.extAddr, pExtAddr, sizeof(ApiMac_sAddrExt_t));

    if(Csf_getDevice(&devAddr,&item))
    {
        shortAddr = item.devInfo.shortAddress;
    }

    return(shortAddr);
}

/*!
 Find entry in device list

 Public function defined in csf.h
 */
bool Csf_getDevice(ApiMac_sAddr_t *pDevAddr, Llc_deviceListItem_t *pItem)
{
    if((pNV != NULL) && (pItem != NULL))
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;
            id.subID = 0;
            /* Read Network Information from NV */
            if(pDevAddr->addrMode == ApiMac_addrType_short)
            {
                pNV->readContItem(id, 0, sizeof(Llc_deviceListItem_t), pItem,
                                      sizeof(uint16_t),
                                      (uint16_t)((uint32_t)&pItem->devInfo.shortAddress-(uint32_t)&pItem->devInfo.panID),
                                      &pDevAddr->addr.shortAddr, &id.subID);
            }
            else
            {
                pNV->readContItem(id, 0, sizeof(Llc_deviceListItem_t), pItem,
                                      APIMAC_SADDR_EXT_LEN,
                                      (uint16_t)((uint32_t)&pItem->devInfo.extAddress-(uint32_t)&pItem->devInfo.panID),
                                      &pDevAddr->addr.extAddr, &id.subID);
            }


            if(id.subID != CSF_INVALID_SUBID)
            {
                return(true);
            }
        }
    }
    return (false);
}

/*!
 Find entry in device list

 Public function defined in csf.h
 */
bool Csf_getDeviceItem(uint16_t devIndex, Llc_deviceListItem_t *pItem,
                       uint16_t *pSubID)
{
    if((pNV != NULL) && (pItem != NULL))
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;
            uint8_t stat;
            uint16_t currSubID = 0;
            int readItems = 0;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;

            while((readItems < numEntries) && (currSubID
                                               < CSF_MAX_DEVICELIST_IDS))
            {
                Llc_deviceListItem_t item;

                id.subID = currSubID;

                /* Read Network Information from NV */
                stat = pNV->readItem(id, 0, sizeof(Llc_deviceListItem_t),
                                     &item);
                if(stat == NVINTF_SUCCESS)
                {
                    if(readItems == devIndex)
                    {
                        memcpy(pItem, &item, sizeof(Llc_deviceListItem_t));
                        if (NULL != pSubID)
                        {
                            *pSubID = currSubID;
                        }
                        return (true);
                    }
                    readItems++;
                }
                currSubID++;
            }
        }
    }

    return (false);
}

/*!
 Csf implementation for memory allocation

 Public function defined in csf.h
 */
void *Csf_malloc(uint16_t size)
{
#ifdef OSAL_PORT2TIRTOS
    return OsalPort_malloc(size);
#else
    return(ICall_malloc(size));
#endif /* endif for OSAL_PORT2TIRTOS */
}

/*!
 Csf implementation for memory de-allocation

 Public function defined in csf.h
 */
void Csf_free(void *ptr)
{
    if(ptr != NULL)
    {
#ifdef OSAL_PORT2TIRTOS
        OsalPort_free(ptr);
#else
        ICall_free(ptr);
#endif /* endif for OSAL_PORT2TIRTOS */
    }
}

/*!
 Update the Frame Counter

 Public function defined in csf.h
 */
void Csf_updateFrameCounter(ApiMac_sAddr_t *pDevAddr, uint32_t frameCntr)
{
    if((pNV != NULL) && (pNV->writeItem != NULL))
    {
        if(pDevAddr == NULL)
        {
            /* Update this device's frame counter */
            if((frameCntr >=
                (lastSavedCoordinatorFrameCounter + FRAME_COUNTER_SAVE_WINDOW)))
            {
                NVINTF_itemID_t id;

                /* Setup NV ID */
                id.systemID = NVINTF_SYSID_APP;
                id.itemID = CSF_NV_FRAMECOUNTER_ID;
                id.subID = 0;

                /* Write the NV item */
                if(pNV->writeItem(id, sizeof(uint32_t), &frameCntr)
                                == NVINTF_SUCCESS)
                {
                    lastSavedCoordinatorFrameCounter = frameCntr;
                }
            }
        }
        else
        {
            /* Child frame counter update */
            Llc_deviceListItem_t devItem;

            /* Is the device in our database? */
            if(Csf_getDevice(pDevAddr, &devItem))
            {
                /*
                 Don't save every update, only save if the new frame
                 counter falls outside the save window.
                 */
                if((devItem.rxFrameCounter + FRAME_COUNTER_SAVE_WINDOW)
                                <= frameCntr)
                {
                    /* Update the frame counter */
                    devItem.rxFrameCounter = frameCntr;
                    updateDeviceListItem(&devItem);
                }
            }
        }
    }
}

/*!
 Get the Frame Counter

 Public function defined in csf.h
 */
bool Csf_getFrameCounter(ApiMac_sAddr_t *pDevAddr, uint32_t *pFrameCntr)
{
    /* Check for valid pointer */
    if(pFrameCntr != NULL)
    {
        /*
         A pDevAddr that is null means to get the frame counter for this device
         */
        if(pDevAddr == NULL)
        {
            if((pNV != NULL) && (pNV->readItem != NULL))
            {
                NVINTF_itemID_t id;

                /* Setup NV ID */
                id.systemID = NVINTF_SYSID_APP;
                id.itemID = CSF_NV_FRAMECOUNTER_ID;
                id.subID = 0;

                /* Read Network Information from NV */
                if(pNV->readItem(id, 0, sizeof(uint32_t), pFrameCntr)
                                == NVINTF_SUCCESS)
                {
                    /* Set to the next window */
                    *pFrameCntr += FRAME_COUNTER_SAVE_WINDOW;
                    return(true);
                }
                else
                {
                    /*
                     Wasn't found, so write 0, so the next time it will be
                     greater than 0
                     */
                    uint32_t fc = 0;

                    /* Setup NV ID */
                    id.systemID = NVINTF_SYSID_APP;
                    id.itemID = CSF_NV_FRAMECOUNTER_ID;
                    id.subID = 0;

                    /* Write the NV item */
                    pNV->writeItem(id, sizeof(uint32_t), &fc);
                }
            }
        }

        *pFrameCntr = 0;
    }
    return (false);
}


/*!
 Delete an entry from the device list

 Public function defined in csf.h
 */
void Csf_removeDeviceListItem(ApiMac_sAddrExt_t *pAddr)
{
    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        int index;

        /* Does the item exist? */
        index = findDeviceListIndex(pAddr);
        if(index != DEVICE_INDEX_NOT_FOUND)
        {
            uint8_t stat;
            NVINTF_itemID_t id;

            /* Setup NV ID for the device list record */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;
            id.subID = (uint16_t)index;

            stat = pNV->deleteItem(id);
            if(stat == NVINTF_SUCCESS)
            {
                /* Update the number of entries */
                uint16_t numEntries = Csf_getNumDeviceListEntries();
                if(numEntries > 0)
                {
                    numEntries--;
                    saveNumDeviceListEntries(numEntries);
                }
            }
        }
    }
}

/*!
 Display Error

 Public function defined in csf.h
 */
void Csf_displayError(const char *pTxt, uint8_t code, uint16_t shortAddress)
{
#ifndef CUI_DISABLE
    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "%s0x%02x - Addr=0x%04x", pTxt, code, shortAddress);
#endif /* CUI_DISABLE */
}

/*!
 Assert Indication

 Public function defined in csf.h
 */
void Csf_assertInd(uint8_t reason)
{
#if defined(MT_CSF)
    if((pNV != NULL) && (pNV->writeItem != NULL))
    {
        /* Attempt to save reason to read after reset */
        (void)pNV->writeItem(nvResetId, 1, &reason);
    }
#endif /* endif for MT_CSF */
}

/*!
 Clear all the NV Items

 Public function defined in csf.h
 */
void Csf_clearAllNVItems(void)
{
    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        NVINTF_itemID_t id;
        uint16_t entries;

        /* Clear Network Information */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_NETWORK_INFO_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        /* Clear the device list entries number */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_DEVICELIST_ENTRIES_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        /*
         Clear the device list entries.  Brute force through
         every possible subID, if it doesn't exist that's fine,
         it will fail in deleteItem.
         */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_DEVICELIST_ID;
        for(entries = 0; entries < CSF_MAX_DEVICELIST_IDS; entries++)
        {
            id.subID = entries;
            pNV->deleteItem(id);
        }

        /* Clear the device tx frame counter */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_FRAMECOUNTER_ID;
        id.subID = 0;
        pNV->deleteItem(id);
    }
}

/*!
 Check if config timer is active

 Public function defined in csf.h
 */
bool Csf_isConfigTimerActive(void)
{
    return(UtilTimer_isActive(&configClkStruct));
}

/*!
 Check if tracking timer is active

 Public function defined in csf.h
 */
bool Csf_isTrackingTimerActive(void)
{
    return(UtilTimer_isActive(&trackingClkStruct));
}

/*!
 The application calls this function to open the network.

 Public function defined in ssf.h
 */
void Csf_openNwk(void)
{
    permitJoining = true;
    Cllc_setJoinPermit(0xFFFFFFFF);
#ifndef CUI_DISABLE
    updateCollectorStatusLine(false, NULL);
#endif /* CUI_DISABLE */
}

/*!
 The application calls this function to close the network.

 Public function defined in ssf.h
 */
void Csf_closeNwk(void)
{
    if(!CONFIG_FH_ENABLE)
    {
        permitJoining = false;
        Cllc_setJoinPermit(0);
#ifndef CUI_DISABLE
        updateCollectorStatusLine(false, NULL);
#endif /* CUI_DISABLE */
    }
}

/*!
 * @brief       Removes a device from the network.
 *
 * @param        deviceShortAddr - device short address to remove.
 */
int Csf_sendDisassociateMsg(uint16_t deviceShortAddr)
{
    int status = -1;

    if(pNV != NULL)
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;
            uint16_t subId = 0;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;

            while(subId < CSF_MAX_DEVICELIST_IDS)
            {
                Llc_deviceListItem_t item;
                uint8_t stat;

                id.subID = (uint16_t)subId;

                /* Read Network Information from NV */
                stat = pNV->readItem(id, 0, sizeof(Llc_deviceListItem_t),
                                     &item);

                if( (stat == NVINTF_SUCCESS) && (deviceShortAddr == item.devInfo.shortAddress))
                {
                    /* Send a disassociate to the device */
                    Cllc_sendDisassociationRequest(item.devInfo.shortAddress,
                                                   item.capInfo.rxOnWhenIdle);
                    //* Remove device from the NV list when receiving disassocCnfCb */

                    status = 0;
                    break;
                }
                subId++;
            }
        }
    }

    return status;
}

/*!
 * @brief       Get the device Ext. Address from short address
 *
 * @param        deviceShortAddr - device short address
 */
int Csf_getDeviceExtAdd(uint16_t deviceShortAddr, ApiMac_sAddrExt_t * extAddr)
{
    int status = -1;

    if(pNV != NULL)
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;
            uint16_t subId = 0;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;

            while(subId < CSF_MAX_DEVICELIST_IDS)
            {
                Llc_deviceListItem_t item;
                uint8_t stat;

                id.subID = (uint16_t)subId;

                /* Read Network Information from NV */
                stat = pNV->readItem(id, 0, sizeof(Llc_deviceListItem_t),
                                     &item);

                if( (stat == NVINTF_SUCCESS) && (deviceShortAddr == item.devInfo.shortAddress))
                {
                    memcpy(extAddr, &item.devInfo.extAddress, sizeof(ApiMac_sAddrExt_t));
                    status = 0;
                    break;
                }
                subId++;
            }
        }
    }

    return status;
}

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 The application calls this function to get a passkey.

 Public function defined in ssf.h
 */
void Csf_SmPasskeyEntry(SM_passkeyEntry_t passkeyAction)
{
#ifndef CUI_DISABLE
    static uint8_t smMenuUsed = 0;
#endif /* CUI_DISABLE */
    // if passkey is selected
    if(passkeyAction == SM_passkeyEntryReq)
    {
#ifdef USE_DMM
        RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_PASSKEY_REQUEST);

        if (RemoteDisplay_getBleAuthConnectionStatus() == false)
        {
#endif /* USE_DMM */
#ifndef CUI_DISABLE
        // deregister main menu when you switch to SM menu
        CUI_deRegisterMenu(csfCuiHndl, &csfMainMenu);

        // request a menu if available
        CUI_registerMenu(csfCuiHndl, &smPassKeyMenu);
        smMenuUsed = 1;

        // Open the menu itself
        // there is only 1 item in smPassKeyMenu list.
        CUI_menuNav(csfCuiHndl, &smPassKeyMenu, 0);
#endif /* CUI_DISABLE */
#ifdef USE_DMM
        }
#endif /* USE_DMM */
    }
    else
    {
#ifdef USE_DMM
        if(passkeyAction == SM_passkeyEntryTimeout)
        {
          RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_PASSKEY_TIMEOUT);
        }
#endif /* USE_DMM */
#ifndef CUI_DISABLE
        // deregister the passkey menu
        CUI_deRegisterMenu(csfCuiHndl, &smPassKeyMenu);

        // Only re-enable the main menu if it was previously disabled
        if(smMenuUsed == 1)
        {
            // re-register the main menu again
            CUI_registerMenu(csfCuiHndl, &csfMainMenu);

        }
        // Go back to the help screen which is the last menu in the list.
        // third argument represents the index of the menu to travel to.
        CUI_menuNav(csfCuiHndl, &csfMainMenu, csfMainMenu.numItems - 1);
#endif /* CUI_DISABLE */
    }
}
#endif /* endif for FEATURE_SECURE_COMMISSIONING */

/******************************************************************************
 Local Functions
 *****************************************************************************/

/*!
 * @brief       Tracking timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processTrackingTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Collector_events, COLLECTOR_TRACKING_TIMEOUT_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

#ifdef IEEE_COEX_METRICS
/*!
 * @brief       Coex timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processCoexTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Csf_events |= COEX_IEEE_METRICS_EVT;

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}
#endif

#ifdef MAC_STATS
/*!
 * @brief       MAC Stats timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processStatsTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Csf_events |= MAC_STATS_EVT;

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}
#endif

/*!
 * @brief       Tracking timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processBroadcastTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Collector_events, COLLECTOR_BROADCAST_TIMEOUT_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/*!
 * @brief       Join permit timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processJoinTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Cllc_events, CLLC_JOIN_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/*!
 * @brief       Config delay timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processConfigTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Collector_events, COLLECTOR_CONFIG_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/*!
 * @brief       Identify timeout handler function.
 *
 * @param       a0 - ignored
 */
static void processIdentifyTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

#ifndef POWER_MEAS
    /* Stop LED blinking - we can write to GPIO in SWI */
    LED_stopBlinking(gGreenLedHandle);
#endif
}

/*!
 * @brief       Trickle timeout handler function for PA .
 *
 * @param       a0 - ignored
 */
static void processPATrickleTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Cllc_events, CLLC_PA_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/*!
 * @brief       Trickle timeout handler function for PC.
 *
 * @param       a0 - ignored
 */
static void processPCTrickleTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Cllc_events, CLLC_PC_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/*!
 * @brief       Key event handler function
 *
 * @param       keysPressed - Csf_keys that are pressed
 */
static void processKeyChangeCallback(Button_Handle _buttonHandle, Button_EventMask _buttonEvents)
{
    if ((_buttonEvents & Button_EV_CLICKED) || (_buttonEvents & Button_EV_PRESSED))
    {
        Csf_keys = _buttonHandle;
        Csf_events |= CSF_KEY_EVENT;
        /* Wake up the application thread when it waits for keys event */
        sem_post(collectorSem);
    }
}

#ifdef IEEE_COEX_METRICS
/*!
 * @brief       Read coex metrics from MAC
 */
static void readCoexMetrics(void)
{
    Collector_coexStatistics_t coexStatistics;
    ApiMac_mlmeGetCoexReqStruct(ApiMac_coexAttribute_coexMetrics,
                                &coexStatistics);

#ifndef CUI_DISABLE
    CUI_statusLinePrintf(csfCuiHndl, coexStatusLine,
                         "Grants=%d, Rejects=%d, Cont Rejects=%d, Max Cont Rejects=%d",
                         coexStatistics.dbgCoexGrants,
                         coexStatistics.dbgCoexRejects,
                         coexStatistics.dbgCoexContRejects,
                         coexStatistics.dbgCoexMaxContRejects);
#endif
}

/*!
 * @brief       Reset coex metrics in MAC
 */
static void coexMetricsResetAction(int32_t menuEntryInex)
{
    Collector_coexStatistics_t coexStatistics;
    memset(&coexStatistics, 0, sizeof(Collector_coexStatistics_t));

    ApiMac_mlmeSetCoexReqStruct(ApiMac_coexAttribute_coexMetrics,
                                &coexStatistics);

    Csf_events |= COEX_IEEE_METRICS_EVT;

    // Wake up the application thread when it waits for event
    sem_post(collectorSem);
}
#endif /* IEEE_COEX_METRICS */

#ifdef MAC_STATS
/*!
 * @brief       Read mac stats from MAC
 */
static void readMacStats(void)
{
    macStatisticsStruct_t macStats;
    ApiMac_mlmeGetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

#ifndef CUI_DISABLE
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    CUI_statusLinePrintf(csfCuiHndl, macStatsLine1,
                         "CcaRetries=%d, CcaFailures=%d, UCastRetries=%d",
                         macStats.cca_retries,
                         macStats.cca_failures,
                         macStats.mac_tx_ucast_retry); //NO_GRANT failure
#else
    CUI_statusLinePrintf(csfCuiHndl, macStatsLine1,
                         "CcaRetries=%d, CcaRejects=%d, UCastRetries=%d, UCastRejects=%d",
                         macStats.cca_retries,
                         macStats.cca_failures,
                         macStats.mac_tx_ucast_retry,
                         macStats.mac_tx_ucast_fail);
#endif // defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#ifdef IEEE_COEX_ENABLED
    CUI_statusLinePrintf(csfCuiHndl, macStatsLine2,
                         "LowPrioReq=%d, HighPrioReq=%d, LowPrioDenied=%d, HighPrioDenied=%d",
                         macStats.pta_lo_pri_req,
                         macStats.pta_hi_pri_req,
                         macStats.pta_lo_pri_denied,
                         macStats.pta_hi_pri_denied);
    uint16_t totalDenied = macStats.pta_hi_pri_denied + macStats.pta_lo_pri_denied;
    if (prevTick == 0)
    {
        prevTick = Clock_getTicks();
    }
    else
    {
        currTick = Clock_getTicks();
        totalTime += ((currTick - prevTick) * Clock_tickPeriod) / 1000000;
        prevTick = currTick;
    }

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    CUI_statusLinePrintf(csfCuiHndl, macStatsLine3,
                         "DeniedRate=%d per %d secs",
                         totalDenied,
                         (macStats.pta_lo_pri_req + macStats.pta_hi_pri_req)); //totalTime);
    macStats.pta_denied_rate = ((totalDenied * 100)/(macStats.pta_lo_pri_req + macStats.pta_hi_pri_req));

    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    CUI_statusLinePrintf(csfCuiHndl, appStatsLine1,
                         "AppPktSent=%d, AppPktSucess = %d, AppPktFail=%d, AppPktRetries=%d, avgE2EDelay=%d, worstCaseE2EDelay = %d",
                         Collector_statistics.trackingRequestAttempts,
                         TrackMsgsSent,
                         TrackMsgFail,
                         TrackMsgRetries,
                         avgE2EDelay,
                         worstCaseE2EDelay);
    CUI_statusLinePrintf(csfCuiHndl, appStatsLine2,
                         "AppPktRcvd=%d",
                         Collector_statistics.sensorMessagesReceived);

    readCoexMetrics();
#else
    CUI_statusLinePrintf(csfCuiHndl, macStatsLine3,
                         "DeniedRate=%d per %d secs",
                         totalDenied,
                         totalTime);
    macStats.pta_denied_rate = totalTime;

    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);
#endif // defined(IEEE_COEX_TEST) && defined(COEX_MENU)

#endif

#endif
}

/*!
 * @brief       Reset mac stats in MAC
 */
static void macStatsResetAction(int32_t menuEntryInex)
{
    macStatisticsStruct_t macStats;
    memset(&macStats, 0, sizeof(macStatisticsStruct_t));

    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    Csf_events |= MAC_STATS_EVT;

    // Wake up the application thread when it waits for event
    sem_post(collectorSem);
}
#endif /* MAC_STATS */

/*!
 * @brief       Add an entry into the device list
 *
 * @param       pItem - pointer to the device list entry
 * @param       pNewDevice - pointer to a flag which will be updated
 *              based on if the sensor joining is already assoc with
 *              the collector or freshly joining the network
 * @return      true if added or already existed, false if problem
 */
static bool addDeviceListItem(Llc_deviceListItem_t *pItem, bool *pNewDevice)
{
    bool retVal = false;
    int subId = DEVICE_INDEX_NOT_FOUND;
    /* By default, set this flag to true;
    will be updated - if device already found in the list*/
    *pNewDevice = true;

    if((pNV != NULL) && (pItem != NULL))
    {
        subId = findDeviceListIndex(&pItem->devInfo.extAddress);
        if(subId != DEVICE_INDEX_NOT_FOUND)
        {
            retVal = true;

            /* Not a new device; already exists */
            *pNewDevice = false;
        }
        else
        {
            uint8_t stat;
            NVINTF_itemID_t id;
            uint16_t numEntries = Csf_getNumDeviceListEntries();

            /* Check the maximum size */
            if(numEntries < CSF_MAX_DEVICELIST_ENTRIES)
            {
                /* Setup NV ID for the device list record */
                id.systemID = NVINTF_SYSID_APP;
                id.itemID = CSF_NV_DEVICELIST_ID;
                id.subID = (uint16_t)findUnusedDeviceListIndex();

                /* write the device list record */
                if(id.subID != CSF_INVALID_SUBID)
                {
                    stat = pNV->writeItem(id, sizeof(Llc_deviceListItem_t), pItem);
                    if(stat == NVINTF_SUCCESS)
                    {
                        /* Update the number of entries */
                        numEntries++;
                        saveNumDeviceListEntries(numEntries);
                        retVal = true;
                    }
                }
            }
        }
    }

    return (retVal);
}

#ifndef CUI_DISABLE
/*!
 Read the number of currently active sensors

 * */
static uint16_t getNumActiveDevices(void)
{
    uint16_t activeSensors = 0;
    int x;
    for(x = 0; x < CONFIG_MAX_DEVICES; x++)
        {
            if((Cllc_associatedDevList[x].shortAddr != CSF_INVALID_SHORT_ADDR)
               && (Cllc_associatedDevList[x].status & CLLC_ASSOC_STATUS_ALIVE))
            {
                activeSensors++;
            }


        }
    return (activeSensors);
}
#endif /* CUI_DISABLE */

/*!
 * @brief       Update an entry in the device list
 *
 * @param       pItem - pointer to the device list entry
 */
static void updateDeviceListItem(Llc_deviceListItem_t *pItem)
{
    if((pNV != NULL) && (pItem != NULL))
    {
        int idx;

        idx = findDeviceListIndex(&pItem->devInfo.extAddress);
        if(idx != DEVICE_INDEX_NOT_FOUND)
        {
            NVINTF_itemID_t id;

            /* Setup NV ID for the device list record */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;
            id.subID = (uint16_t)idx;

            /* write the device list record */
            pNV->writeItem(id, sizeof(Llc_deviceListItem_t), pItem);
        }
    }
}

/*!
 * @brief       Find entry in device list
 *
 * @param       pAddr - address to of device to find
 *
 * @return      sub index into the device list, -1 (DEVICE_INDEX_NOT_FOUND)
 *              if not found
 */
static int findDeviceListIndex(ApiMac_sAddrExt_t *pAddr)
{
    if((pNV != NULL) && (pAddr != NULL))
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;
            Llc_deviceListItem_t item;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;
            id.subID = 0;

            /* Read Network Information from NV */
            pNV->readContItem(id, 0, sizeof(Llc_deviceListItem_t), &item,
                                  APIMAC_SADDR_EXT_LEN,
                                  (uint16_t)((uint32_t)&item.devInfo.extAddress-(uint32_t)&item), pAddr, &id.subID);

            if(id.subID != CSF_INVALID_SUBID)
            {
                return(id.subID);
            }
        }
    }
    return (DEVICE_INDEX_NOT_FOUND);
}

/*!
 * @brief       Find an unused device list index
 *
 * @return      index that is not in use
 */
static int findUnusedDeviceListIndex(void)
{
    int x;

    for(x = 0; (x < CONFIG_MAX_DEVICES); x++)
    {
        /* Make sure the entry is valid. */
        if(CSF_INVALID_SHORT_ADDR == Cllc_associatedDevList[x].shortAddr)
        {
            return (x);
        }
    }
    return (CSF_INVALID_SUBID);
}

/*!
 * @brief       Read the number of device list items stored
 *
 * @param       numEntries - number of entries in the device list
 */
static void saveNumDeviceListEntries(uint16_t numEntries)
{
    if(pNV != NULL)
    {
        NVINTF_itemID_t id;

        /* Setup NV ID for the number of entries in the device list */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = CSF_NV_DEVICELIST_ENTRIES_ID;
        id.subID = 0;

        /* Read the number of device list items from NV */
        pNV->writeItem(id, sizeof(uint16_t), &numEntries);
    }
}

#if defined(TEST_REMOVE_DEVICE)
/*!
 * @brief       This is an example function on how to remove a device
 *              from this network.
 */
static void removeTheFirstDevice(void)
{
    if(pNV != NULL)
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;
            uint16_t subId = 0;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;

            while(subId < CSF_MAX_DEVICELIST_IDS)
            {
                Llc_deviceListItem_t item;
                uint8_t stat;

                id.subID = (uint16_t)subId;

                /* Read Network Information from NV */
                stat = pNV->readItem(id, 0, sizeof(Llc_deviceListItem_t),
                                     &item);

                if(stat == NVINTF_SUCCESS)
                {
                    /* Send a disassociate to the device */
                    Cllc_sendDisassociationRequest(item.devInfo.shortAddress,
                                                   item.capInfo.rxOnWhenIdle);
                    /* remove device from the NV list */
                    Cllc_removeDevice(&item.devInfo.extAddress);

                    /* Remove it from the Device list */
                    Csf_removeDeviceListItem(&item.devInfo.extAddress);

                    break;
                }
                subId++;
            }
        }
    }
}
#else

#ifndef POWER_MEAS
/*!
 * @brief       Retrieve the first device's short address
 *
 * @return      short address or 0xFFFF if not found
 */
static uint16_t getTheFirstDevice(void)
{
    uint16_t found = CSF_INVALID_SHORT_ADDR;
    if(pNV != NULL)
    {
        uint16_t numEntries;

        numEntries = Csf_getNumDeviceListEntries();

        if(numEntries > 0)
        {
            NVINTF_itemID_t id;

            /* Setup NV ID for the device list records */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = CSF_NV_DEVICELIST_ID;
            id.subID = 0;

            while(id.subID < CSF_MAX_DEVICELIST_IDS)
            {
                Llc_deviceListItem_t item;
                uint8_t stat;

                /* Read Network Information from NV */
                stat = pNV->readItem(id, 0, sizeof(Llc_deviceListItem_t),
                                     &item);
                if(stat == NVINTF_SUCCESS)
                {
                    found = item.devInfo.shortAddress;
                    break;
                }
                id.subID++;
            }
        }
    }
    return(found);
}
#endif /* endif for POWER_MEAS */
#endif /* endif for TEST_REMOVE_DEVICE */

#ifndef CUI_DISABLE
/*!
 * @brief       Handles printing that the orphaned device joined back
 *
 * @return      none
 */
void Csf_IndicateOrphanReJoin(uint16_t shortAddr)
{
    CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "Orphaned Sensor Re-Joined - Addr=0x%04x",
                         shortAddr);
    CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());
}
#endif /* CUI_DISABLE */

#ifdef USE_DMM

/*!
 Initialize the provisioning timeout clock.

 Public function defined in csf.h
 */
void Csf_initializeProvisioningClock(void)
{
    /* Initialize the timers needed for this application */
    provisioningClkHandle = UtilTimer_construct(&provisioningClkStruct,
                                       processProvisioningCallback,
                                       FH_ASSOC_TIMER,
                                        0,
                                        false,
                                        0);
}

/*!
 Set the provisioning timeout clock.

 Public function defined in csf.h
 */
void Csf_setProvisioningClock(bool provision)
{
    /* Stop the Provisioning timer */
    if(UtilTimer_isActive(&provisioningClkStruct) == true)
    {
        UtilTimer_stop(&provisioningClkStruct);
    }

    UtilTimer_destruct(&provisioningClkStruct);

    /* Setup timer for provisioning association timeout */
    if (provision)
    {
        provisioningClkHandle = UtilTimer_construct(&provisioningClkStruct,
                                       processProvisioningCallback,
                                       PROVISIONING_ASSOC_TIMER,
                                        0,
                                        false,
                                        provision);
    }
    else
    {
        provisioningClkHandle = UtilTimer_construct(&provisioningClkStruct,
                                       processProvisioningCallback,
                                       FH_ASSOC_TIMER,
                                        0,
                                        false,
                                        provision);
    }

    UtilTimer_start(&provisioningClkStruct);
}


static void processProvisioningCallback(uintptr_t provision)
{
    static bool updateProvPolicy = true;

    //Arg used to select provision and associate to a network
    if (provision)
    {
        if (updateProvPolicy)
        {
            Csf_setProvisioningClock(true);

            /* Update policy */
            Util_setEvent(&Collector_events, COLLECTOR_PROV_EVT);

            updateProvPolicy = false;
        }
        else
        {
            Util_setEvent(&Collector_events, COLLECTOR_START_EVT);
            updateProvPolicy = true;
        }
    }

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}
#endif /* endif for USE_DMM */

#ifndef CUI_DISABLE
/**
 *  @brief Updates the collector's status line
 *
 *  If pNetworkInfo is NULL, cached network information is printed to UART
 */
static void updateCollectorStatusLine(bool restored, Llc_netInfo_t *pNetworkInfo)
{
#if (CONFIG_MAC_BEACON_ORDER != NON_BEACON_ORDER)
    char macMode[4] = "BCN\0";
#elif (CONFIG_FH_ENABLE)
    char macMode[3] = "FH\0";
#else
    char macMode[5] = "NBCN\0";
#endif

    static bool restoredStatus;
    static Llc_netInfo_t networkInfo;
    static char* deviceStatus;
    char* permitJoinStatus;

    /* Network information is not always available when this function is invoked,
     * so cache information upon network startup */
    if(pNetworkInfo != NULL)
    {
        networkInfo.channel = pNetworkInfo->channel;
        networkInfo.devInfo.panID = pNetworkInfo->devInfo.panID;
        networkInfo.devInfo.shortAddress = pNetworkInfo->devInfo.shortAddress;
        networkInfo.fh = pNetworkInfo->fh;

        restoredStatus = restored;

        if(restoredStatus == false)
        {
            deviceStatus = "Started";
        }
        else
        {
            deviceStatus = "Restarted";
        }

    }

    /* Permit join is always on in FH Mode */
    if(permitJoining == false && !CONFIG_FH_ENABLE)
    {
        permitJoinStatus = "Off";
    }
    else
    {
        permitJoinStatus = "On";
    }

    CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());

    /* Print out collector status information */
    if(networkInfo.fh == false)
    {
        if(CONFIG_PHY_ID == APIMAC_200KBPS_920MHZ_PHY_136)
        {
            CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine,
                                 "%s--Mode=%s, Addr=0x%04x, PanId=0x%04x, Ch=%d,%d, PermitJoin=%s ",
                                 deviceStatus, macMode, networkInfo.devInfo.shortAddress, networkInfo.devInfo.panID,
                                 networkInfo.channel + CHANNEL_BUNDLE_OFFSET_920MHZ,
                                 networkInfo.channel + CHANNEL_BUNDLE_OFFSET_920MHZ + 1, permitJoinStatus);
        }
        else
        {
            CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine,
                                 "%s--Mode=%s, Addr=0x%04x, PanId=0x%04x, Ch=%d, PermitJoin=%s ",
                                 deviceStatus, macMode, networkInfo.devInfo.shortAddress, networkInfo.devInfo.panID,
                                 networkInfo.channel, permitJoinStatus);
        }
    }
    else
    {
        CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine,
                             "%s--Mode=%s, Addr=0x%04x, PanId=0x%04x, Ch=FH, PermitJoin=%s ",
                             deviceStatus, macMode, networkInfo.devInfo.shortAddress, networkInfo.devInfo.panID,
                             permitJoinStatus);
    }
}
#endif /* CUI_DISABLE */

/*
 *  @brief Handles common network starting actions
 */
static void formNwkAndUpdateUser(void)
{
/* Network already formed if AUTO_START defined */
#ifndef AUTO_START
    if(started == false)
    {
#ifndef CUI_DISABLE
        CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine, "Starting...");
        CUI_statusLinePrintf(csfCuiHndl, numJoinDevStatusLine, "%x", getNumActiveDevices());
#endif /* CUI_DISABLE */
        /* Tell the collector to start */
        Util_setEvent(&Collector_events, COLLECTOR_START_EVT);
    }
#endif /* AUTO_START */
}

/*
 *  @brief Handles common network opening actions
 *
 *  @param  openNwkRequest - true to open network, false to close
 */
static void openCloseNwkAndUpdateUser(bool openNwkRequest)
{
#ifdef FEATURE_SECURE_COMMISSIONING
    /* Network permission changes not accepted during CM Process */
    if((fCommissionRequired == TRUE) || (SM_Current_State == SM_CM_InProgress))
    {
#ifndef CUI_DISABLE
        CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine, "PermitJoin blocked during commissioning");
#endif /* CUI_DISABLE */
    }
    else
#endif /* FEATURE_SECURE_COMMISSIONING */
    {
        if(started)
        {
            if (openNwkRequest)
            {
                /* Set event to open network */
                Util_setEvent(&Collector_events, COLLECTOR_OPEN_NWK_EVT);
            }
            else
            {
                /* Set event to close network */
                Util_setEvent(&Collector_events, COLLECTOR_CLOSE_NWK_EVT);
            }
        }
    }
}

#ifndef POWER_MEAS
/*!
 * @brief       The application calls this function to toggle an LED request
 *              and update the user through the CUI.
 *
 * @param       shortAddr - address of the sensor
 *
 */
static void sendToggleAndUpdateUser(uint16_t shortAddr)
{
#ifdef FEATURE_SECURE_COMMISSIONING
   /* LED toggle not accepted during CM Process*/
   if(SM_Current_State != SM_CM_InProgress)
#endif /* endif for FEATURE_SECURE_COMMISSIONING */
   {
        ApiMac_sAddr_t toggleDev;
        toggleDev.addr.shortAddr = shortAddr;
        if(toggleDev.addr.shortAddr != CSF_INVALID_SHORT_ADDR)
        {
            toggleDev.addrMode = ApiMac_addrType_short;
            Collector_sendToggleLedRequest(&toggleDev);
#ifndef CUI_DISABLE
            CUI_statusLinePrintf(csfCuiHndl, deviceStatusLine,
                                 "ToggleLEDRequest Sent - Addr=0x%04x",
                                 toggleDev.addr.shortAddr);
#endif /* CUI_DISABLE */
        }
   }
}
#endif /* endif for POWER_MEAS */

#ifndef CUI_DISABLE
static uint8_t moveCursorLeft(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space)
{
    // If you haven't hit the end of left boundary, keep moving cursor left.
    if (left_boundary != col)
    {
        col--;

    }
    else
    {
        col = right_boundary;
    }

    if(0 != skip_space)
    {
        //skip the white space, by continuing to move left over it
        if((col % 3) == 0)
            col--;
    }

    return col;
}

static uint8_t moveCursorRight(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space)
{
    // If you haven't hit the end of modifiable lines, keep moving cursor right.
    if (right_boundary != col)
    {
        col++;
    }
    else
    {
        col = left_boundary;
    }

    // if skip_space is true
    if(0 != skip_space)
    {
        //skip the white space, by continuing to move right over it
        if((col % 3) == 0)
            col++;
    }
    return col;
}

/**
 *  @brief Callback to be called when the UI sets PAN ID.
 */
static void setPanIdAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  static CUI_cursorInfo_t cursor = {0, 6};
  static bool initPanId = true;
  static uint16_t panId;

  const char tmpInput[2] = {_input, '\0'};

  if(initPanId)
  {
      Cllc_getFormingPanId(&panId);
      initPanId = false;
  }

  switch (_input) {
      case CUI_ITEM_INTERCEPT_START:
      {
          Cllc_getFormingPanId(&panId);
          break;
      }
      // Submit the final modified value
      case CUI_ITEM_INTERCEPT_STOP:
      {
          Cllc_setFormingPanId(panId);
          // Reset the local cursor info
          cursor.col = 6;
          break;
      }
      // Get the correct value if there was a network previously.
      // previously initialized in the Waiting state.
      case CUI_ITEM_PREVIEW:
          Cllc_getFormingPanId(&panId);
          break;
      // Move the cursor to the left
      case CUI_INPUT_LEFT:
      {
          cursor.col = moveCursorLeft(cursor.col, 6, 9, 0);
          break;
      }
      // Move the cursor to the right
      case CUI_INPUT_RIGHT:
      {
          cursor.col = moveCursorRight(cursor.col, 6, 9, 0);
          break;
      }
      case CUI_INPUT_UP:
          break;

      case CUI_INPUT_DOWN:
          break;

      case CUI_INPUT_BACK:
      {
          // get the position of the nibble to change
          uint8_t shift = 4 * (9 - cursor.col);

          // make sure you don't exceed the left boundary
          if (6 <= cursor.col)
          {
              // clear the desired nibble with F
              panId |= (uint32_t)(0x0F<<shift) ;
              cursor.col = moveCursorLeft(cursor.col, 6, 9, 0);
          }

          break;
      }
      case CUI_INPUT_EXECUTE:
          break;
      default:
      {
          // is it a number
          if(CUI_IS_INPUT_HEX(_input))
          {
              // multiply by 4 to use binary
              // get nibble position to change
              uint8_t shift = 4 * (9 - cursor.col);
              // convert from ascii to uint8
              uint8_t digit = strtol(tmpInput, NULL, 16);

              // clear the nibble
              panId &= ~((uint32_t)0xF << shift);
              // set the nibble to desired digit
              panId |= (uint32_t)digit << shift;

              cursor.col = moveCursorRight(cursor.col, 6, 9, 0);

          }
      }
  }

  if (panId == 0xFFFF)
  {
    strcpy(_pLines[0], "    0xFFFF (any)");
  }
  else
  {
    char tmp[4];
    uintToString( panId, tmp, 16, 4, TRUE, FALSE);
    strcpy(_pLines[0], "    0x");
    strncat(_pLines[0], tmp, 4);
  }

  if (_input != CUI_ITEM_PREVIEW) {
      strcpy(_pLines[2], "     PAN ID");
      _pCurInfo->row = 1;
      _pCurInfo->col = cursor.col+1;
  }
}

/*!
 * @brief       Validate and handle errors in channel mask entered through CUI
 *
 * @param       _chanMask - channel mask updated with user input
 * @param       byteIdx   - index of modified byte
 */
static void validateChMask(uint8_t *_chanMask, uint8_t byteIdx)
{
    // Verify user input by comparing against valid channel mask
    uint8_t validChannelByte = _chanMask[byteIdx] & validChannelMask[byteIdx];
    if (validChannelByte != _chanMask[byteIdx])
    {
        CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine,
                             "Invalid input. Only updated with supported channels");

        // Only accept inputs that represent supported channels
        _chanMask[byteIdx] = validChannelByte;
    }
    else
    {
        CUI_statusLinePrintf(csfCuiHndl, collectorStatusLine, "");
    }
}

/**
 *  @brief Callback to be called when the UI sets Channel Mask.
 */
static void setChMaskAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint8_t channelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
    static bool initChanMask = true;
    static CUI_cursorInfo_t cursor = {0, 1};
    uint8_t chanMaskByteIdx = 0;
    uint8_t chanMaskStrIdx = 0;

    const char tmpInput[2] = {_input, '\0'};

    if(initChanMask)
    {
        Cllc_getChanMask(channelMask);
        initChanMask = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            // Reset the local cursor info
            cursor.col = 1;
            Cllc_getChanMask(channelMask);
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Cllc_setChanMask(channelMask);

            // Reset the local cursor info
            cursor.col = 1;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            Cllc_getChanMask(channelMask);
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
        {
            cursor.col = moveCursorLeft(cursor.col, 1, 50, 1);
            break;
        }
        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
        {
            cursor.col = moveCursorRight(cursor.col, 1, 50, 1);
            break;
        }
        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_BACK:
        {
            // get the position of the entire hex byte based on the column you're at
            uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
            // get the position of the nibble that you'd like to change.
            uint8_t byteIdx = nibbleIdx / 2;

            // make sure you haven't exceeded the boundary
            if (1 <= cursor.col)
            {
                if(cursor.col % 3 == 2)
                {
                    channelMask[byteIdx] |= (uint32_t)(0x0F) ;
                    /* You are at the right side of the byte */
                }
                else if(cursor.col % 3 == 1)
                {
                    /* You are at the left side of the byte
                     * Set the value at the cursor to be F by default
                     * The shift by 4 is to modify only the left
                     * side of the byte. */
                    channelMask[byteIdx] |= (uint32_t)(0x0F<<4) ;
                }

                cursor.col = moveCursorLeft(cursor.col, 1, 50, 1);

            }
            break;
        }
        case CUI_INPUT_EXECUTE:
            break;

        default:
        {
            // Make sure channel mask input is valid hex
            if(CUI_IS_INPUT_HEX(_input))
            {
                // get the position of the entire hex byte based on the column you're at
                uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
                // get the position of the nibble that you'd like to set the value of
                uint8_t byteIdx = nibbleIdx / 2;

                // you're at the left side
                if(cursor.col % 3 == 1)
                {
                    // First, clear the left side, keep the right side
                    channelMask[byteIdx] &= (uint32_t)(0x0F);
                    // Next, shift the input left, and or it with the existing side.
                    channelMask[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16) << 4);
                }
                // you're at the right side
                else if(cursor.col % 3 == 2)
                {
                    // First, clear the right side, keep the left side
                    channelMask[byteIdx] &= (uint32_t)(0xF0);
                    // Next, use the input to or it with the existing side.
                    channelMask[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16));
                }

                // Verify and correct user input
                validateChMask(channelMask, byteIdx);

                cursor.col = moveCursorRight(cursor.col, 1, 50, 1);
            }
        }

    }

    for(chanMaskByteIdx = 0; chanMaskByteIdx < APIMAC_154G_CHANNEL_BITMAP_SIZ; chanMaskByteIdx++)
    {
        char tmp[2];

        uintToString( channelMask[chanMaskByteIdx], tmp, 16, 2, TRUE, FALSE);

        // clear out the UART lines
        strcpy((_pLines[0] + chanMaskStrIdx), " ");
        chanMaskStrIdx += 1;
        strncat((_pLines[0] + chanMaskStrIdx), tmp, 2);
        chanMaskStrIdx += 2;
    }

    if (_input != CUI_ITEM_PREVIEW)
    {
        // copy the label in.
        strcpy(_pLines[2], "     CHAN MASK");

        // set the label at the right place.
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}

#if CONFIG_FH_ENABLE
/**
 *  @brief Callback to be called when the UI sets Channel Mask.
 */
static void setAsyncChMaskAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint8_t channelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ];
    static bool initChanMask = true;
    static CUI_cursorInfo_t cursor = {0, 1};
    uint8_t chanMaskByteIdx = 0;
    uint8_t chanMaskStrIdx = 0;

    const char tmpInput[2] = {_input, '\0'};

    if(initChanMask)
    {
        Cllc_getAsyncChanMask(channelMask);
        initChanMask = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            Cllc_getAsyncChanMask(channelMask);
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Cllc_setAsyncChanMask(channelMask);

            // Reset the local cursor info
            cursor.col = 1;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            Cllc_getAsyncChanMask(channelMask);
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
            cursor.col = moveCursorLeft(cursor.col, 1, 50, 1);
            break;

        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
            cursor.col = moveCursorRight(cursor.col, 1, 50, 1);
            break;

        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_EXECUTE:
            break;

        case CUI_INPUT_BACK:
        {
            // get exact position of nibble you want to modify
            uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
            uint8_t byteIdx = nibbleIdx / 2;

            // don't exceed left boundary
            if (1 <= cursor.col)
            {
                if(cursor.col % 3 == 2)
                {
                    channelMask[byteIdx] |= (uint32_t)(0x0F) ;
                    /* You are at the right side of the byte */
                }
                else if(cursor.col % 3 == 1)
                {
                    /* You are at the left side of the byte */
                    channelMask[byteIdx] |= (uint32_t)(0x0F<<4) ;
                }
                cursor.col = moveCursorLeft(cursor.col, 1, 50, 1);
            }
            break;
        }
        default:
            // Make sure channel mask input is valid hex
            if(CUI_IS_INPUT_HEX(_input))
            {
                // get exact position of nibble you want to modify
                uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
                uint8_t byteIdx = nibbleIdx / 2;

                // you're at the left side
                if(nibbleIdx % 2 == 0)
                {
                     // First, clear the left side, keep the right side
                    channelMask[byteIdx] &= (uint32_t)(0x0F);
                    // Next, shift the input left, and or it with the existing side.
                    channelMask[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16) << 4);
                }
                // you're at the left side
                else
                {
                    // First, clear the right side, keep the left side
                    channelMask[byteIdx] &= (uint32_t)(0xF0);
                    // Next, use the input to or it with the existing side.
                    // strtol will convert from hex ascii to hex integers
                    channelMask[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16));
                }

                // Verify and correct user input
                validateChMask(channelMask, byteIdx);

                cursor.col = moveCursorRight(cursor.col, 1, 50, 1);
            }

    }

    for(chanMaskByteIdx = 0; chanMaskByteIdx < APIMAC_154G_CHANNEL_BITMAP_SIZ; chanMaskByteIdx++)
    {
        char tmp[2];

        uintToString( channelMask[chanMaskByteIdx], tmp, 16, 2, TRUE, FALSE);

        strcpy((_pLines[0] + chanMaskStrIdx), " ");
        chanMaskStrIdx += 1;
        strncat((_pLines[0] + chanMaskStrIdx), tmp, 2);
        chanMaskStrIdx += 2;
    }

    if (_input != CUI_ITEM_PREVIEW)
    {
        strcpy(_pLines[2], "     ASYNC CHAN MASK");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}
#endif /* endif for CONFIG_FH_ENABLE */

#ifdef FEATURE_MAC_SECURITY
/**
 *  @brief Callback to be called when the UI sets Default Network Key.
 *  The cursor represents where your keyboard is currently 'pointing' at.
 */
static void setNwkKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint8_t defaultNwkKey[APIMAC_KEY_MAX_LEN];
    static bool initChanMask = true;
    /* The particular row the cursor is on */
    static CUI_cursorInfo_t cursor = {0, 1};

    uint8_t defaultNwkKeyByteIdx = 0;
    uint8_t defaultNwkKeyStrIdx = 0;

    const char tmpInput[2] = {_input, '\0'};

    if(initChanMask)
    {
        Cllc_getDefaultKey(defaultNwkKey);
        initChanMask = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            Cllc_getDefaultKey(defaultNwkKey);
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Cllc_setDefaultKey(defaultNwkKey);

            // Reset the local cursor info
            cursor.col = 1;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            Cllc_getDefaultKey(defaultNwkKey);
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
        {
            cursor.col = moveCursorLeft(cursor.col, 1, 47, 1);
            break;
        }
        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
        {
            cursor.col = moveCursorRight(cursor.col, 1, 47, 1);
            break;
        }
        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_BACK:
        {
            // get the byte index that needs to be changed
            uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
            // get the specific nibble location that needs to be set
            uint8_t byteIdx = nibbleIdx / 2;

            // make sure you don't exceed the boundary
            if (1 <= cursor.col)
            {
                if(cursor.col % 3 == 2)
                {
                    defaultNwkKey[byteIdx] |= (uint32_t)(0x0F) ;
                    /* You are at the right side of the byte */
                }
                else if(cursor.col % 3 == 1)
                {
                    /* You are at the left side of the byte */
                    defaultNwkKey[byteIdx] |= (uint32_t)(0x0F<<4) ;
                }

                cursor.col = moveCursorLeft(cursor.col, 1, 47, 1);
            }
            break;
        }
        case CUI_INPUT_EXECUTE:
            break;
        case CUI_INPUT_ESC:
            break;
        default:
        {
        // Make sure channel mask input is valid hex
        if(CUI_IS_INPUT_HEX(_input))
        {
            // grab the byte you need to modify
            uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
            // pick the nibble you need to modify
            uint8_t byteIdx = nibbleIdx / 2;

            // you are at the left side
            if(nibbleIdx % 2 == 0)
                {
                    // First, clear the left side, keep the right side
                    defaultNwkKey[byteIdx] &= (uint32_t)(0x0F);
                    // Next, shift the input left, and or it with the existing side.
                    defaultNwkKey[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16) << 4);
                }
            // you are the right side
            else
                {
                    // First, clear the right side, keep the left side
                    defaultNwkKey[byteIdx] &= (uint32_t)(0xF0);
                    // Next, use the input to or it with the existing side.
                    // strtol will convert from hex ascii to hex integers
                    defaultNwkKey[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16));
                }
            /* Move the cursor forward if the space is the next one */
            cursor.col = moveCursorRight(cursor.col, 1, 47, 1);
        }

        }

    }

    for(defaultNwkKeyByteIdx = 0; defaultNwkKeyByteIdx < APIMAC_KEY_MAX_LEN; defaultNwkKeyByteIdx++)
    {
        char tmp[2];

        uintToString( defaultNwkKey[defaultNwkKeyByteIdx], tmp, 16, 2, TRUE, FALSE);

        // clear the labels
        strcpy((_pLines[0] + defaultNwkKeyStrIdx), " ");
        defaultNwkKeyStrIdx += 1;
        strncat((_pLines[0] + defaultNwkKeyStrIdx), tmp, 2);
        defaultNwkKeyStrIdx += 2;
    }

    if (_input != CUI_ITEM_PREVIEW)
    {
        // set the label in the right place
        strcpy(_pLines[2], "     DEFAULT NWK KEY");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}
#endif /* FEATURE_MAC_SECURITY */

#ifdef FEATURE_SECURE_COMMISSIONING

static void setSmPassKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    // Update the title in case of entering the multi menu
    static const char passkeyTitle[] = "TI Passkey Menu";
    CUI_updateMultiMenuTitle(passkeyTitle);

    static CUI_cursorInfo_t cursor = {0, 4};
    static char passkeyASCII[SM_PASSKEY_LEN + 1] = {0};

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            strcpy(passkeyASCII, "000000");
            cursor.col = 4;
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            uint32_t passkeyValue = atoi(passkeyASCII);
            //Set passkey in SM
            SM_setPasskey(passkeyValue);
#ifdef USE_DMM
            RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_COMMISSIONING);
#endif /* USE_DMM */
            strcpy(passkeyASCII, "");
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
            cursor.col = moveCursorLeft(cursor.col, 4, 9, 0);
            break;

        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
            cursor.col = moveCursorRight(cursor.col, 4, 9, 0);
            break;

        case CUI_INPUT_UP:
        {
            break;
        }

        case CUI_INPUT_DOWN:
        {
            break;
        }

        case CUI_INPUT_BACK:
            passkeyASCII[cursor.col-4] = 'F';
            cursor.col = moveCursorLeft(cursor.col, 4, 9, 0);
            break;
        case CUI_INPUT_EXECUTE:
            break;
        default:
        {
            //is it a number
            if(CUI_IS_INPUT_HEX(_input) )
            {
                passkeyASCII[cursor.col-4] = _input;

                cursor.col = moveCursorRight(cursor.col, 4, 9, 0);
            }
        }
    }

    strcpy(_pLines[0], "    ");
    strncat(_pLines[0], passkeyASCII, 8);

    if (_input != CUI_ITEM_PREVIEW) {
        strcpy(_pLines[2], "   ENTER PASSKEY  ");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}
#endif /* endif for FEATURE_SECURE_COMMISSIONING */

static void sensorSelectAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static CUI_cursorInfo_t cursor = {0, 6};
    static bool initSelectAddr = true;

    const char tmpInput[2] = {_input, '\0'};

    if(initSelectAddr)
    {
        SelectedSensor = 1;
        initSelectAddr = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            // Reset the local cursor info
            cursor.col = 6;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
        {
            cursor.col = moveCursorLeft(cursor.col, 6, 9, 0);
            break;
        }
        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
        {
            cursor.col = moveCursorRight(cursor.col, 6, 9, 0);
            break;
        }
        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_BACK:
        {
            // multiply by 4 because we are working with binary
            // subtract the current column from the right boundary
            // in order to modify the correct nibble
            uint8_t shift = 4 * (9 - cursor.col);
            // don't exceed the boundary
            if (6 <= cursor.col)
            {
                // set the default of the nibble you want to change to F

                SelectedSensor &= ~((uint32_t)((0x0F<<shift)));
                cursor.col = moveCursorLeft(cursor.col, 6, 9, 0);

            }
            break;
        }
        case CUI_INPUT_EXECUTE:
            break;
        default:
        {
            //is it a hex number
            if(CUI_IS_INPUT_HEX(_input))
            {
                // multiply by 4 because we're working with binary
                // get the specific nibble position that you want to modify
                uint8_t shift = 4 * (9 - cursor.col);

                // Use strtol to convert from ASCII Hex to actual hex
                uint8_t digit = strtol(tmpInput, NULL, 16);

                // clear the nibble first
                SelectedSensor &= ~((uint32_t)0xF << shift);
                // set the nibble to the desired digit
                SelectedSensor |= (uint32_t)digit << shift;

                cursor.col = moveCursorRight(cursor.col, 6, 9, 0);

            }
        }
    }

    if(SelectedSensor == 0x00)
    {
        SelectedSensor = 0x01;
    }

    char tmp[4];
    uintToString( SelectedSensor, tmp, 16, 4, TRUE, FALSE);
    strcpy(_pLines[0], "    0x");
    strncat(_pLines[0], tmp, 4);

    if (_input != CUI_ITEM_PREVIEW) {
        strcpy(_pLines[2], "  SELECTED SENSOR");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}

static void sensorSetReportInterval(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static CUI_cursorInfo_t cursor = {0, 4};
    static char reportIntervalASCII[9] = "00090000";

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            reportInterval = 0;
            cursor.col = 4;

            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
            Csf_sensorAction = SENSOR_ACTION_SET_RPT_INT;

            if(atoi(reportIntervalASCII) == 0x00)
            {
                memcpy(reportIntervalASCII, "00090000", 8);
            }

            reportInterval = atoi(reportIntervalASCII);

            // Wake up the application thread when it waits for clock event
            sem_post(collectorSem);

            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
            cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);
            break;

        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
            cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            break;

        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_EXECUTE:
            break;
        case CUI_INPUT_BACK:
            if (4 <= cursor.col)
            {
                // clear the ASCII directly with 0
                reportIntervalASCII[cursor.col-4] = '0' ;
                cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);

            }
            break;
        default:
        {
            //is it a number
            //is it a number
            if(CUI_IS_INPUT_NUM(_input))
            {
                // directly set the ASCII value because the array
                // is ASCII And so is the input
                reportIntervalASCII[cursor.col-4] = _input;

                cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            }

        }
    }

    // clear the label.
    strcpy(_pLines[0], "    ");
    strncat(_pLines[0], reportIntervalASCII, 8);

    if (_input != CUI_ITEM_PREVIEW) {
        // set hte label in the right place.
        strcpy(_pLines[2], "  REPORT INTERVAL");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}

/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void sensorLedToggleAction(int32_t menuEntryInex)
{
    Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
    Csf_sensorAction = SENSOR_ACTION_TOGGLE;

    // Wake up the application thread when it waits for clock event
    sem_post(collectorSem);
}

/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void sensorDisassocAction(int32_t menuEntryInex)
{
    Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
    Csf_sensorAction = SENSOR_ACTION_DISASSOC;

#ifdef FEATURE_SECURE_COMMISSIONING
    CUI_deRegisterMenu(csfCuiHndl, &smPassKeyMenu);
    CUI_registerMenu(csfCuiHndl, &csfMainMenu);
    CUI_menuNav(csfCuiHndl, &csfMainMenu, csfMainMenu.numItems - 1);
    SM_stopCMProcess();
#endif /* FEATURE_SECURE_COMMISSIONING */

    // Wake up the application thread when it waits for clock event
    sem_post(collectorSem);
}

#if defined(DEVICE_TYPE_MSG)
/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void sensorDeviceTypeRequestAction(int32_t menuEntryInex)
{
    Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
    Csf_sensorAction = SENSOR_ACTION_DEVICE_TYPE_REQ;

    // Wake up the application thread when it waits for clock event
    sem_post(collectorSem);
}
#endif /* DEVICE_TYPE_MSG */

/**
 *  @brief Callback to be called when the UI forms network.
 */
static void formNwkAction(int32_t menuEntryInex)
{
    /* Tell the collector to start network */
    formNwkAndUpdateUser();

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/**
 *  @brief Callback to be called when the UI opens network.
 */
static void openNwkAction(int32_t menuEntryInex)
{
    /* Open network */
    openCloseNwkAndUpdateUser(true);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

/**
 *  @brief Callback to be called when the UI closes network.
 */
static void closeNwkAction(int32_t menuEntryInex)
{
    /* Close network */
    openCloseNwkAndUpdateUser(false);

    /* Wake up the application thread when it waits for clock event */
    sem_post(collectorSem);
}

#ifdef IEEE_COEX_METRICS
/**
 *  @brief Callback to be called when the UI requests coex metrics
 */
static void coexMetricsRequestAction(int32_t menuEntryInex)
{
    Csf_events |= COEX_IEEE_METRICS_EVT;

    // Wake up the application thread when it waits for event
    sem_post(collectorSem);
}
#endif

#ifdef MAC_STATS
/**
 *  @brief Callback to be called when the UI requests mac stats
 */
static void macStatsRequestAction(int32_t menuEntryInex)
{
    Csf_events |= MAC_STATS_EVT;

    // Wake up the application thread when it waits for event
    sem_post(collectorSem);
}
#endif

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
static void setCoexNumPacket(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static CUI_cursorInfo_t cursor = {0, 4};
    static char numPacketASCII[9] = "00010000";

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            numPacket = 0;
            cursor.col = 4;

            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
            Csf_sensorAction = COEX_ACTION_SET_NUM_PACKET;

            if(atoi(numPacketASCII) == 0x00)
            {
                memcpy(numPacketASCII, "00010000", 8);
            }

            numPacket = atoi(numPacketASCII);

            // Wake up the application thread when it waits for clock event
            sem_post(collectorSem);

            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
            cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);
            break;

        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
            cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            break;

        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_EXECUTE:
            break;
        case CUI_INPUT_BACK:
            if (4 <= cursor.col)
            {
                // clear the ASCII directly with 0
                numPacketASCII[cursor.col-4] = '0' ;
                cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);

            }
            break;
        default:
        {
            //is it a number
            //is it a number
            if(CUI_IS_INPUT_NUM(_input))
            {
                // directly set the ASCII value because the array
                // is ASCII And so is the input
                numPacketASCII[cursor.col-4] = _input;

                cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            }

        }
    }

    // clear the label.
    strcpy(_pLines[0], "    ");
    strncat(_pLines[0], numPacketASCII, 8);

    if (_input != CUI_ITEM_PREVIEW) {
        // set hte label in the right place.
        strcpy(_pLines[2], "  NUM PACKETS");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }

    Collector_coexStatistics_t coexStatistics;
    macStatisticsStruct_t macStats;
    /* Reset Stats */
    memset(&coexStatistics, 0, sizeof(Collector_coexStatistics_t));

    ApiMac_mlmeSetCoexReqStruct(ApiMac_coexAttribute_coexMetrics,
                                &coexStatistics);

    memset(&macStats, 0, sizeof(macStats));
    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    TrackMsgsSent = 0;
    TrackMsgsSuccess = 0;
    TrackMsgFail = 0;
    TrackMsgRetries = 0;
    avgE2EDelay = 0;
    worstCaseE2EDelay = 0;
    totalE2EDelaySum = 0;
    Collector_statistics.sensorMessagesReceived = 0;
    Collector_statistics.trackingRequestAttempts = 0;
    readMacStats();

    /* end Stats reset */
}


static void setCoexPacketSize(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static CUI_cursorInfo_t cursor = {0, 4};
    static char pktSizeASCII[9] = "00001000";

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            trackingPktSize = 0;
            cursor.col = 4;

            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
            Csf_sensorAction = COEX_ACTION_SET_PKT_SIZE;

            if(atoi(pktSizeASCII) == 0x00)
            {
                memcpy(pktSizeASCII, "00001000", 8);
            }

            trackingPktSize = atoi(pktSizeASCII);

            // Wake up the application thread when it waits for clock event
            sem_post(collectorSem);

            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
            cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);
            break;

        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
            cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            break;

        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_EXECUTE:
            break;
        case CUI_INPUT_BACK:
            if (4 <= cursor.col)
            {
                // clear the ASCII directly with 0
                pktSizeASCII[cursor.col-4] = '0' ;
                cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);

            }
            break;
        default:
        {
            //is it a number
            //is it a number
            if(CUI_IS_INPUT_NUM(_input))
            {
                // directly set the ASCII value because the array
                // is ASCII And so is the input
                pktSizeASCII[cursor.col-4] = _input;

                cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            }

        }
    }

    // clear the label.
    strcpy(_pLines[0], "    ");
    strncat(_pLines[0], pktSizeASCII, 8);

    if (_input != CUI_ITEM_PREVIEW) {
        // set hte label in the right place.
        strcpy(_pLines[2], "  NUM PACKETS");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }

}

static void setCoexTrackingInterval(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static CUI_cursorInfo_t cursor = {0, 4};
    static char trackingIntervalASCII[9] = "00000500";

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            numPacket = 0;
            cursor.col = 4;

            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
            Csf_sensorAction = COEX_ACTION_SET_TRACK_INT;

            if(atoi(trackingIntervalASCII) == 0x00)
            {
                memcpy(trackingIntervalASCII, "00000500", 8);
            }

            trackingInterval = atoi(trackingIntervalASCII);

            // Wake up the application thread when it waits for clock event
            sem_post(collectorSem);

            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
        // Move the cursor to the left
        case CUI_INPUT_LEFT:
            cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);
            break;

        // Move the cursor to the right
        case CUI_INPUT_RIGHT:
            cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            break;

        case CUI_INPUT_UP:
            break;

        case CUI_INPUT_DOWN:
            break;

        case CUI_INPUT_EXECUTE:
            break;
        case CUI_INPUT_BACK:
            if (4 <= cursor.col)
            {
                // clear the ASCII directly with 0
                trackingIntervalASCII[cursor.col-4] = '0' ;
                cursor.col = moveCursorLeft(cursor.col, 4, 11, 0);

            }
            break;
        default:
        {
            //is it a number
            //is it a number
            if(CUI_IS_INPUT_NUM(_input))
            {
                // directly set the ASCII value because the array
                // is ASCII And so is the input
                trackingIntervalASCII[cursor.col-4] = _input;

                cursor.col = moveCursorRight(cursor.col, 4, 11, 0);
            }

        }
    }

    // clear the label.
    strcpy(_pLines[0], "    ");
    strncat(_pLines[0], trackingIntervalASCII, 8);

    if (_input != CUI_ITEM_PREVIEW) {
        // set hte label in the right place.
        strcpy(_pLines[2], "  NUM PACKETS");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }

}

/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void startCoexTest(int32_t menuEntryInex)
{
#if 1
    Collector_coexStatistics_t coexStatistics;
    macStatisticsStruct_t macStats;
    /* Reset Stats */
    memset(&coexStatistics, 0, sizeof(Collector_coexStatistics_t));

    ApiMac_mlmeSetCoexReqStruct(ApiMac_coexAttribute_coexMetrics,
                                &coexStatistics);

    memset(&macStats, 0, sizeof(macStats));
    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    TrackMsgsSent = 0;
    TrackMsgsSuccess = 0;
    TrackMsgFail = 0;
    TrackMsgRetries = 0;
    avgE2EDelay = 0;
    worstCaseE2EDelay = 0;
    totalE2EDelaySum = 0;
    Collector_statistics.sensorMessagesReceived = 0;
    Collector_statistics.trackingRequestAttempts = 0;
    /* end Stats reset */

    Cllc_associatedDevList[0].status = 1;
    /* Setup for next tracking */
    Csf_setTrackingClock( APS_INTERVAL);
    //nSensorPkts = 0;
#else
    Csf_events |= COLLECTOR_SENSOR_ACTION_EVT;
    Csf_sensorAction = COEX_ACTION_START;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(collectorSem);
#endif
}

/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void endCoexTest(int32_t menuEntryInex)
{
    Csf_setTrackingClock( 0);
    readMacStats();
}
#endif // defined(IEEE_COEX_TEST) && defined(COEX_MENU)
/**
 *  @brief Send process menu event.
 */
static void processMenuUpdate(void)
{
    Csf_events |= COLLECTOR_UI_INPUT_EVT;

    // Wake up the application thread when it waits for clock event
    sem_post(collectorSem);
}

/**
 *  @brief called when the UI converts Uint to string.
 */
static void uintToString (uint32_t value, char * str, uint8_t base, uint8_t num_of_digists, bool pad0, bool reverse)
{
  int i;
  uint8_t index;

  for (i = 0; i < num_of_digists; i++)
  {
    index = (reverse ? i : num_of_digists - 1 - i);
    str[index] = '0' + (value % base);
    if (str[index] > '9')
    {
      str[index] += 'A' - '0' - 10;
    }
    value /= base;
    if ((!pad0) && (value == 0))
    {
      break;
    }
  }
}
#endif /* CUI_DISABLE */
