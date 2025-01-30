/******************************************************************************

 @file ssf.c

 @brief Sensor Specific Functions

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

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE)
#include <stdio.h>
#endif /* defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE) */

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
#include <driverlib/aon_batmon.h>

#include "ti_drivers_config.h"

#include "util_timer.h"
#include "mac_util.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif /* CUI_DISABLE */

#include "macconfig.h"
#include "nvocmp.h"

#include "sensor.h"
#include "smsgs.h"
#include "ssf.h"
#include "ti_154stack_config.h"

#ifdef FEATURE_NATIVE_OAD
#include "oad_client.h"
#endif //FEATURE_NATIVE_OAD

#ifdef OSAL_PORT2TIRTOS
#include "osal_port.h"
#else
#include "icall.h"
#endif

#include "mac_user_config.h"
#include "mac.h"
#ifdef USE_DMM
#ifdef FEATURE_SECURE_COMMISSIONING
#include "remote_display.h"
#include <sm_commissioning_gatt_profile.h>
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /* USE_DMM */
/******************************************************************************
 Constants and definitions
 *****************************************************************************/

/* Initial timeout value for the reading clock */
#define READING_INIT_TIMEOUT_VALUE 100

/* SSF Events */
#define KEY_EVENT               0x0001
#define SENSOR_UI_INPUT_EVT     0x0002
#define SENSOR_SEND_COLLECTOR_IDENT_EVT  0x0004
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#define COEX_ACTION_SET_NUM_PACKET 0x0008
#define COEX_ACTION_START 0x0010
#define COEX_ACTION_END 0x0020
#endif
/* NV Item ID - the device's network information */
#define SSF_NV_NETWORK_INFO_ID 0x0001
/* NV Item ID - Config information */
#define SSF_NV_CONFIG_INFO_ID  0x0002
/* NV Item ID - this devices frame counter */
#define SSF_NV_FRAMECOUNTER_ID 0x0004
/* NV Item ID - Assert reset reason */
#define SSF_NV_RESET_REASON_ID 0x0005
/* NV Item ID - Number of resets */
#define SSF_NV_RESET_COUNT_ID 0x0006
/* NV Item ID - OAD information */
#define SSF_NV_OAD_ID 0x0007
/* NV Item ID - Device Key information */
#define SSF_NV_DEVICE_KEY_ID  0x0008

/* timeout value for trickle timer initialization */
#define TRICKLE_TIMEOUT_VALUE       30000

/* timeout value for poll timer initialization */
#define POLL_TIMEOUT_VALUE          30000

#define FH_ASSOC_TIMER              2000

/* timeout value for poll timer initialization */
#define SCAN_BACKOFF_TIMEOUT_VALUE  60000

/*! NV driver item ID for reset reason */
#define NVID_RESET {NVINTF_SYSID_APP, SSF_NV_RESET_REASON_ID, 0}

/*! Additional Random Delay for Association */
#define ADD_ASSOCIATION_RANDOM_WINDOW 10000
/*
 The increment value needed to save a frame counter. Example, setting this
 constant to 100, means that the frame counter will be saved when the new
 frame counter is 100 more than the last saved frame counter.  Also, when
 the get frame counter function reads the value from NV it will add this value
 to the read value.
 */
#define FRAME_COUNTER_SAVE_WINDOW     25

#if (USE_DMM) && !(DMM_CENTRAL)
#define PROVISIONING_ASSOC_TIMER    1000
#define PROVISIONING_DISASSOC_TIMER 10
#endif /* USE_DMM && !DMM_CENTRAL */

/* Structure to store the device information in NV */
typedef struct
{
    ApiMac_deviceDescriptor_t device;
    Llc_netInfo_t parent;
} nvDeviceInfo_t;

#ifndef CUI_DISABLE
// ARIB SysConfig/MAC channel 0 corresponds to channel bundle 24,25
#define CHANNEL_BUNDLE_OFFSET_920MHZ 24
/* Mask of all supported channels for input validation */
static const uint8_t validChannelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ] = CUI_VALID_CHANNEL_MASK;
#endif /* CUI_DISABLE */

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#define COEX_PACKETS_TO_SEND    10000
#endif
/******************************************************************************
 External variables
 *****************************************************************************/
#ifdef NV_RESTORE
/*! MAC Configuration Parameters */
extern mac_Config_t Main_user1Cfg;
#endif

/******************************************************************************
 Public variables
 *****************************************************************************/

/*!
 Assert reason for the last reset -  0 - no reason, 2 - HAL/ICALL,
 3 - MAC, 4 - TIRTOS
 */
uint8_t Ssf_resetReseason = 0;

/*! Number of times the device has reset */
uint16_t Ssf_resetCount = 0;

/******************************************************************************
 Local variables
 *****************************************************************************/

/* The application's semaphore */
#ifdef OSAL_PORT2TIRTOS
static sem_t *sensorSem;
#else
static ICall_Semaphore sensorSem;
#endif

/* Clock/timer resources */
static ClockP_Struct readingClkStruct;
#ifndef DMM_CENTRAL
static ClockP_Handle readingClkHandle;
#endif /* !DMM_CENTRAL */

/* Clock/timer resources for JDLLC */
/* trickle timer */
STATIC ClockP_Struct tricklePASClkStruct;
STATIC ClockP_Handle tricklePASClkHandle;
STATIC ClockP_Struct tricklePCSClkStruct;
STATIC ClockP_Handle tricklePCSClkHandle;
/* poll timer */
STATIC ClockP_Struct pollClkStruct;
STATIC ClockP_Handle pollClkHandle;
/* scan backoff timer */
STATIC ClockP_Struct scanBackoffClkStruct;
STATIC ClockP_Handle scanBackoffClkHandle;
/* FH assoc delay */
STATIC ClockP_Struct fhAssocClkStruct;
STATIC ClockP_Handle fhAssocClkHandle;

#if (USE_DMM) && !(DMM_CENTRAL)
STATIC ClockP_Struct provisioningClkStruct;
STATIC ClockP_Handle provisioningClkHandle;
#endif /* USE_DMM && !DMM_CENTRAL */

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE)
static ClockP_Struct clkBlockModeTestStruct;
static ClockP_Handle clkBlockModeTestHandle;
#endif /* defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE) */

/* Key press parameters */
static Button_Handle keys = NULL;

/* pending events */
static uint16_t events = 0;

/* NV Function Pointers */
static NVINTF_nvFuncts_t *pNV = NULL;

/* The last saved frame counter */
static uint32_t lastSavedFrameCounter = 0;

/*! NV driver item ID for reset reason */
static const NVINTF_itemID_t nvResetId = NVID_RESET;

static bool started = false;

static bool led1State = false;

#ifndef CUI_DISABLE
CUI_clientHandle_t ssfCuiHndl;
#endif
#if !defined(POWER_MEAS)
static LED_Handle gGreenLedHandle;
static LED_Handle gRedLedHandle;
#endif /* !POWER_MEAS */
static Button_Handle gRightButtonHandle;
static Button_Handle gLeftButtonHandle;
uint32_t sensorStatusLine;
uint32_t perStatusLine;
uint32_t RxStatsLine;
uint32_t macStatsLine1;

#ifdef DISPLAY_PER_STATS
/*!
Average end to end delay
*/
extern uint16_t avgE2EDelay;
/*!
Worst Case end to end delay
*/
extern uint16_t worstCaseE2EDelay;
#endif // DISPLAY_PER_STATS

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE)
uint32_t ssfStatusLineBlockModeTestState;

#if(CONFIG_MAC_BEACON_ORDER != NON_BEACON_ORDER)
static uint16_t SSF_BLOCK_MODE_ON_PERIOD  = (CONFIG_REPORTING_INTERVAL / 2);
static uint16_t SSF_BLOCK_MODE_OFF_PERIOD =  (CONFIG_REPORTING_INTERVAL / 1.2);
#else
static uint16_t SSF_BLOCK_MODE_ON_PERIOD  = (CONFIG_REPORTING_INTERVAL / 2);
static uint16_t SSF_BLOCK_MODE_OFF_PERIOD =  (CONFIG_REPORTING_INTERVAL / 2);
#endif
#endif /* defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE) */

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
uint32_t numPacket = 0;
uint32_t nSensorPkts = 0;
/* accumulated total E2E delay */
extern uint32_t totalE2EDelaySum;
#endif
/******************************************************************************
 Local function prototypes
 *****************************************************************************/

#ifndef DMM_CENTRAL
static void processReadingTimeoutCallback(uintptr_t a0);
#endif
static void processKeyChangeCallback(Button_Handle _buttonHandle, Button_EventMask _buttonEvents);
static void processPCSTrickleTimeoutCallback(uintptr_t a0);
static void processPASTrickleTimeoutCallback(uintptr_t a0);
static void processPollTimeoutCallback(uintptr_t a0);
static void processScanBackoffTimeoutCallback(uintptr_t a0);
static void processFHAssocTimeoutCallback(uintptr_t a0);
#if (USE_DMM) && !(DMM_CENTRAL)
static void processProvisioningCallback(uintptr_t a0);
#endif /* USE_DMM && !DMM_CENTRAL */

#ifndef CUI_DISABLE
static uint8_t moveCursorLeft(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space);
static uint8_t moveCursorRight(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space);
static void setPanIdAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void setChMaskAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#if CONFIG_FH_ENABLE
static void setAsyncChMaskAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
#ifdef FEATURE_MAC_SECURITY
static void setNwkKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
#ifdef FEATURE_SECURE_COMMISSIONING
static void setSmSetAuthModeAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void setSmPassKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif /* FEATURE_SECURE_COMMISSIONING */
static void assocAction(int32_t menuEntryInex);
static void disassocAction(int32_t menuEntryInex);
static void collectorLedIndentifyAction(int32_t menuEntryInex);
static void processMenuUpdate(void);

static void uintToString (uint32_t value, char * str, uint8_t base, uint8_t num_of_digists, bool pad0, bool reverse);
static void joinStatusLineUpdate(bool rejoined, uint16_t shortAddress, uint16_t panID, uint8_t channel);

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
static void Ssf_blockModeTestClockHandler(UArg arg);
static void Ssf_blockModeTestOn(int32_t menuEntryIndex);
static void Ssf_blockModeTestOff(int32_t menuEntryIndex);
static void Ssf_setBlockModeOnPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void Ssf_setBlockModeOffPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void Ssf_setBlockModePeriodUiAction(uint16_t* blockModePeriod, const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif /* defined(USE_DMM) && defined(BLOCK_MODE_TEST) */
#endif /* CUI_DISABLE */

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
static void setCoexNumPacket(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void startCoexTest(int32_t menuEntryInex);
static void endCoexTest(int32_t menuEntryInex);
#endif
/* Menu */
#ifdef OAD_ONCHIP
#ifdef OAD_IMG_A
    #define SFF_MENU_TITLE " TI Sensor (Persistent App) "
#else /* OAD_IMG_A */
#ifdef MCUBOOT
    #define SFF_MENU_TITLE " TI Sensor (onchip) [MCUBOOT] "
#else
    #define SFF_MENU_TITLE " TI Sensor (User App) "
#endif // MCUBOOT
#endif // OAD_IMG_A
#else
#ifdef S_NS_154_ENABLED
    #define SFF_MENU_TITLE " TI Sensor (TFM) "
#else
#if defined(MCUBOOT) && defined(FEATURE_NATIVE_OAD)
    #define SFF_MENU_TITLE " TI Sensor (offchip) [MCUBOOT] "
#else
    #define SFF_MENU_TITLE " TI Sensor "
#endif // OAD_ONCHIP
#endif
#endif

#if CONFIG_FH_ENABLE
#define FH_MENU_ENABLED 1
#else
#define FH_MENU_ENABLED 0
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
#define SM_MENU_ENABLED 1
#else
#define SM_MENU_ENABLED 0
#endif /* FEATURE_SECURE_COMMISSIONING */

#ifdef FEATURE_MAC_SECURITY
#define SECURITY_MENU_ENABLED 1
#else
#define SECURITY_MENU_ENABLED 0
#endif

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#define COEX_TRIGGER_MENU_ENABLED 1
#else
#define COEX_TRIGGER_MENU_ENABLED 0
#endif

#ifndef CUI_DISABLE
CUI_SUB_MENU(configureSubMenu, "<      CONFIGURE      >", 2 + FH_MENU_ENABLED + SM_MENU_ENABLED + SECURITY_MENU_ENABLED, ssfMainMenu)
    CUI_MENU_ITEM_INT_ACTION(  "<      SET PANID      >", setPanIdAction)
    CUI_MENU_ITEM_INT_ACTION(  "<    SET CHAN MASK    >", setChMaskAction)
#if CONFIG_FH_ENABLE
    CUI_MENU_ITEM_INT_ACTION(  "<  SET AS CHAN MASK   >", setAsyncChMaskAction)
#endif
#ifdef FEATURE_MAC_SECURITY
    CUI_MENU_ITEM_INT_ACTION(  "<     SET NWK KEY     >", setNwkKeyAction)
#endif
#ifdef FEATURE_SECURE_COMMISSIONING
    CUI_MENU_ITEM_INT_ACTION(  "<     AUTH METHOD     >", setSmSetAuthModeAction)
#endif /* FEATURE_SECURE_COMMISSIONING */
CUI_SUB_MENU_END

CUI_SUB_MENU(commissionSubMenu,"<   NETWORK ACTIONS   >", 2, ssfMainMenu)
    CUI_MENU_ITEM_ACTION(      "<      ASSOCIATE      >", assocAction)
    CUI_MENU_ITEM_ACTION(      "<    DISASSOCIATE     >", disassocAction)
CUI_SUB_MENU_END

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
CUI_SUB_MENU(coexSubMenu,      "<      COEX MENU      >", 3 , ssfMainMenu)
    CUI_MENU_ITEM_INT_ACTION(  "< SET COEX NUM PACKET >", setCoexNumPacket)
    CUI_MENU_ITEM_ACTION(      "<   START COEX TEST   >", startCoexTest)
    CUI_MENU_ITEM_ACTION(      "<    END COEX TEST    >", endCoexTest)
CUI_SUB_MENU_END
#endif

CUI_SUB_MENU(appSubMenu,       "<         APP         >", 1, ssfMainMenu)
    CUI_MENU_ITEM_ACTION(      "<   SEND LED IDENT    >", collectorLedIndentifyAction)
CUI_SUB_MENU_END

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
CUI_SUB_MENU(sensorBlockModeTestSubMenu,"<   BLOCK MODE TEST   >", 4, ssfMainMenu)
    CUI_MENU_ITEM_ACTION(               "<     BM TEST ON      >", Ssf_blockModeTestOn)
    CUI_MENU_ITEM_ACTION(               "<     BM TEST OFF     >", Ssf_blockModeTestOff)
    CUI_MENU_ITEM_INT_ACTION(           "<     ON  VAL (ms)    >", Ssf_setBlockModeOnPeriodAction)
    CUI_MENU_ITEM_INT_ACTION(           "<     OFF VAL (ms)    >", Ssf_setBlockModeOffPeriodAction)
CUI_SUB_MENU_END
#endif

/* This menu will be registered/de-registered at run time to create
a sort of "popup" menu for passkey entry. Since it is de-registered,
it is also completely hidden from the user at all other times.
Note: MAX_REGISTERED_MENUS must be >= 2 for both of the main
menus in this file. */
#ifdef FEATURE_SECURE_COMMISSIONING
CUI_MAIN_MENU(smPassKeyMenu, "<       SM MENU        >", 1, processMenuUpdate)
    CUI_MENU_ITEM_INT_ACTION(  "<    ENTER PASSKEY     >", setSmPassKeyAction)
CUI_MAIN_MENU_END
#endif /* FEATURE_SECURE_COMMISSIONING */

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
CUI_MAIN_MENU(ssfMainMenu, SFF_MENU_TITLE, 4+COEX_TRIGGER_MENU_ENABLED, processMenuUpdate)
#else
CUI_MAIN_MENU(ssfMainMenu, SFF_MENU_TITLE, 3+COEX_TRIGGER_MENU_ENABLED, processMenuUpdate)
#endif
    CUI_MENU_ITEM_SUBMENU(configureSubMenu)
    CUI_MENU_ITEM_SUBMENU(commissionSubMenu)
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    CUI_MENU_ITEM_SUBMENU(coexSubMenu)
#endif
    CUI_MENU_ITEM_SUBMENU(appSubMenu)
#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
    CUI_MENU_ITEM_SUBMENU(sensorBlockModeTestSubMenu)
#endif
CUI_MAIN_MENU_END
#endif /* CUI_DISABLE */


/******************************************************************************
 Public Functions
 *****************************************************************************/

/*!
 The application calls this function during initialization

 Public function defined in ssf.h
 */
void Ssf_init(void *sem)
{
#ifndef CUI_DISABLE
    CUI_clientParams_t clientParams;
#endif /* CUI_DISABLE */
#ifdef FEATURE_NATIVE_OAD
    OADClient_Params_t OADClientParams;
#endif //FEATURE_NATIVE_OAD

#ifdef NV_RESTORE
    /* Save off the NV Function Pointers */
    pNV = &Main_user1Cfg.nvFps;
#endif

    /* Save off the semaphore */
    sensorSem = (sem_t*)sem;

#ifndef CUI_DISABLE
    CUI_clientParamsInit(&clientParams);

    strncpy(clientParams.clientName, "154 Sensor", MAX_CLIENT_NAME_LEN);
    clientParams.maxStatusLines = 1;

#ifdef DISPLAY_PER_STATS
    clientParams.maxStatusLines += 3;
#endif /* DISPLAY_PER_STATS */
#ifdef FEATURE_SECURE_COMMISSIONING
    clientParams.maxStatusLines++;
#endif /* FEATURE_SECURE_COMMISSIONING */
#ifdef SECURE_MANAGER_DEBUG
    clientParams.maxStatusLines++;
#endif /* SECURE_MANAGER_DEBUG */
#ifdef SECURE_MANAGER_DEBUG2
    clientParams.maxStatusLines++;
#endif /* SECURE_MANAGER_DEBUG2 */
#ifdef FEATURE_NATIVE_OAD
    clientParams.maxStatusLines++;
#endif /* FEATURE_NATIVE_OAD */
#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
    clientParams.maxStatusLines++;
#endif

    /* Open UI for key and LED */
    ssfCuiHndl = CUI_clientOpen(&clientParams);
#endif /* CUI_DISABLE */

#ifdef FEATURE_SECURE_COMMISSIONING
#ifndef CUI_DISABLE
    /* Intialize the security manager and register callbacks */
    SM_init(sensorSem, ssfCuiHndl);
#else
    SM_init(sensorSem);
#endif /* CUI_DISABLE */
#endif /* FEATURE_SECURE_COMMISSIONING */

    /* Initialize Keys */
    Button_init();
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
        Ssf_clearAllNVItems();
        nvErased = true;
    }
    // Set button callback
    Button_setCallback(gRightButtonHandle, processKeyChangeCallback);

#ifndef POWER_MEAS
    /* Initialize the LEDs */
    LED_init();
    LED_Params ledParams;
    LED_Params_init(&ledParams);
    gGreenLedHandle = LED_open(CONFIG_LED_GREEN, &ledParams);
    // Removes unused var warning
    (void) gGreenLedHandle;
    gRedLedHandle = LED_open(CONFIG_LED_RED, &ledParams);

    // Blink to indicate the application started up correctly
    LED_startBlinking(gRedLedHandle, 500, 3);
#endif /* POWER_MEAS */

#ifndef CUI_DISABLE
    CUI_registerMenu(ssfCuiHndl, &ssfMainMenu);

    CUI_statusLineResourceRequest(ssfCuiHndl, "Status", false, &sensorStatusLine);
#ifdef DISPLAY_PER_STATS
    CUI_statusLineResourceRequest(ssfCuiHndl, "Sensor TX Stats", false, &perStatusLine);

    CUI_statusLineResourceRequest(ssfCuiHndl, "MAC Stats Latency", true, &macStatsLine1);
    CUI_statusLineResourceRequest(ssfCuiHndl, "Sensor RxStatsLine", false, &RxStatsLine);
#endif
#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
  CUI_statusLineResourceRequest(ssfCuiHndl, "BLOCK MODE TEST STATUS", false, &ssfStatusLineBlockModeTestState);
#endif
#endif /* CUI_DISABLE */

    if((pNV != NULL) && (pNV->readItem != NULL))
    {
        /* Attempt to retrieve reason for the reset */
        (void)pNV->readItem(nvResetId, 0, 1, &Ssf_resetReseason);
    }

    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        /* Only use this reason once */
        (void)pNV->deleteItem(nvResetId);
    }

    if((pNV != NULL) && (pNV->readItem != NULL))
    {
        NVINTF_itemID_t id;
        uint16_t resetCount = 0;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_RESET_COUNT_ID;
        id.subID = 0;

        /* Read the reset count */
        pNV->readItem(id, 0, sizeof(resetCount), &resetCount);

        Ssf_resetCount = resetCount;
        if(pNV->writeItem)
        {
          /* Update the reset count for the next reset */
          resetCount++;
          pNV->writeItem(id, sizeof(resetCount), &resetCount);
        }
    }

#ifdef FEATURE_NATIVE_OAD
    OADClientParams.pEvent = &Sensor_events;
    OADClientParams.eventSem = sensorSem;
#ifndef CUI_DISABLE
    OADClientParams.pOadCuiHndl = &ssfCuiHndl;
#endif /* CUI_DISABLE */

    OADClient_open(&OADClientParams);
#endif //FEATURE_NATIVE_OAD

#ifndef CUI_DISABLE
#if !defined(AUTO_START)
    CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Waiting...");
#else
    CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Starting");
#endif
    if (nvErased) {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "NV cleared");
    }
#endif /* CUI_DISABLE */

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE)
    clkBlockModeTestHandle = UtilTimer_construct(&clkBlockModeTestStruct,
                                             Ssf_blockModeTestClockHandler,
                                             SSF_BLOCK_MODE_ON_PERIOD, SSF_BLOCK_MODE_ON_PERIOD, false,
                                             0);
#endif /* defined(USE_DMM) && defined(BLOCK_MODE_TEST) && !defined(CUI_DISABLE) */
}

/*!
 The application must call this function periodically to
 process any events that this module needs to process.

 Public function defined in ssf.h
 */
void Ssf_processEvents(void)
{
    /* Did a key press occur? */
    if(events & KEY_EVENT)
    {
        /* Right key press is a PAN disassociation request, if the device has started. */
        if((keys == gRightButtonHandle) && (started == true))
        {
            uint8_t provState = Jdllc_getProvState();
            if ((provState == Jdllc_states_joined) || (provState== Jdllc_states_rejoined))
            {
                /* Send disassociation request only if you are in a network */
#ifndef CUI_DISABLE
                CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Leaving");
#endif /* CUI_DISABLE */
                Jdllc_sendDisassociationRequest();
            }
        }
        /* Left key press is for starting the sensor network */
        else if(keys == gLeftButtonHandle)
        {

            if(started == false)
            {
#ifndef CUI_DISABLE
                CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Starting");
#endif /* CUI_DISABLE */

                /* Tell the sensor to start */
                Util_setEvent(&Sensor_events, SENSOR_START_EVT);
                /* Wake up the application thread when it waits for clock event */
                sem_post(sensorSem);
            }
            else
            {   /* Send LED toggle request to identify collector */
                Sensor_sendIdentifyLedRequest();
            }
        }

        /* Clear the key press indication */
        keys = NULL;

        /* Clear the event */
        Util_clearEvent(&events, KEY_EVENT);
    }

#ifdef FEATURE_NATIVE_OAD
    /* Did a OAD event occur? */
#ifdef FEATURE_TOAD
    if(Sensor_events & SENSOR_OAD_TIMEOUT_EVT || Sensor_events & SENSOR_TOAD_DECODE_EVT)
#else
    if(Sensor_events & SENSOR_OAD_TIMEOUT_EVT)
#endif
    {
        OADClient_processEvent(&Sensor_events);
    }
#endif //FEATURE_NATIVE_OAD

    if(events & SENSOR_UI_INPUT_EVT)
    {
        Util_clearEvent(&events, SENSOR_UI_INPUT_EVT);
#ifndef CUI_DISABLE
        CUI_processMenuUpdate();
#endif /* CUI_DISABLE */
    }

    if(events & SENSOR_SEND_COLLECTOR_IDENT_EVT)
    {
        Sensor_sendIdentifyLedRequest();

        Util_clearEvent(&events, SENSOR_SEND_COLLECTOR_IDENT_EVT);
    }
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    if(events & COEX_ACTION_SET_NUM_PACKET)
    {
        Util_clearEvent(&events, COEX_ACTION_SET_NUM_PACKET);
    }
    if(events & COEX_ACTION_START)
    {
        Util_clearEvent(&events, COEX_ACTION_START);
    }
    if(events & COEX_ACTION_END)
    {
        Util_clearEvent(&events, COEX_ACTION_END);
    }
#endif
}

#ifndef CUI_DISABLE
/*!
 The application calls this function to indicate that the
 Sensor's state has changed.

 Public function defined in ssf.h
 */
void Ssf_stateChangeUpdate(Jdllc_states_t state)
{
#ifndef CUI_DISABLE
#if (CONFIG_MAC_BEACON_ORDER != NON_BEACON_ORDER)
    char macMode[4] = "BCN\0";
#elif (CONFIG_FH_ENABLE)
    char macMode[3] = "FH\0";
#else
    char macMode[5] = "NBCN\0";
#endif
#endif /* CUI_DISABLE */
    NVINTF_itemID_t id;
    nvDeviceInfo_t nvItem;

    if(state == Jdllc_states_initWaiting)
    {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Waiting");
    }
    else if(state == Jdllc_states_orphan)
    {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Orphaned");
    }
    else if(state == Jdllc_states_accessDenied)
    {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Access Denied");
    }

    if(Jdllc_getPrevProvState() == Jdllc_states_orphan)
    {
        if(state == Jdllc_states_joined)
        {
            CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Joined");
        }
        else if(state == Jdllc_states_rejoined)
        {
            if((pNV != NULL) && (pNV->readItem != NULL))
            {
                id.systemID = NVINTF_SYSID_APP;
                id.itemID = SSF_NV_NETWORK_INFO_ID;
                id.subID = 0;

                /* Write the NV item */
                if(pNV->readItem(id, 0, sizeof(nvDeviceInfo_t), &nvItem) == NVINTF_SUCCESS)
                {
                    joinStatusLineUpdate(true, nvItem.device.shortAddress, nvItem.device.panID, nvItem.parent.channel);
                }
            }
            else
            {
                CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Rejoined--Mode=%s", macMode);
            }
        }
    }
}

/*!
 * @brief       Display sensor join status line on CUI
 *
 * @param       rejoined - true if sensor rejoining, false otherwise
 * @param       shortAddress - sensor short address
 * @param       panID - network PAN ID
 * @param       channel - network channel
 */
static void joinStatusLineUpdate(bool rejoined, uint16_t shortAddress, uint16_t panID, uint8_t channel)
{
#if (CONFIG_MAC_BEACON_ORDER != NON_BEACON_ORDER)
    char macMode[4] = "BCN\0";
#elif (CONFIG_FH_ENABLE)
    char macMode[3] = "FH\0";
#else
    char macMode[5] = "NBCN\0";
#endif

    char* joinStatus = rejoined ? "Rejoined" : "Joined";

    if(!CONFIG_FH_ENABLE)
    {
        if(CONFIG_PHY_ID == APIMAC_200KBPS_920MHZ_PHY_136)
        {
            CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "%s--Mode=%s, Addr=0x%04x, PanId=0x%04x, Ch=%d,%d",
                                 joinStatus, macMode, shortAddress, panID,
                                 channel + CHANNEL_BUNDLE_OFFSET_920MHZ,
                                 channel + CHANNEL_BUNDLE_OFFSET_920MHZ + 1);
        }
        else
        {

          CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "%s--Mode=%s, Addr=0x%04x, PanId=0x%04x, Ch=%d",
                               joinStatus, macMode, shortAddress, panID, channel);
        }
    }
    else
    {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "%s--Mode=%s, Addr=0x%04x, PanId=0x%04x, Ch=FH",
                             joinStatus, macMode, shortAddress, panID);
    }
}
#endif /* CUI_DISABLE */

/*!
 The application calls this function to indicate that it has
 started or restored the device in a network.

 Public function defined in ssf.h
 */
void Ssf_networkUpdate(bool rejoined,
                       ApiMac_deviceDescriptor_t *pDevInfo,
                       Llc_netInfo_t  *pParentInfo)
{
    /* check for valid structure pointers, ignore if not */
    if((pDevInfo != NULL) && (pParentInfo != NULL))
    {
        if((pNV != NULL) && (pNV->writeItem != NULL))
        {
            NVINTF_itemID_t id;
            nvDeviceInfo_t nvItem;

            /* Setup NV ID */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = SSF_NV_NETWORK_INFO_ID;
            id.subID = 0;

            memcpy(&nvItem.device, pDevInfo, sizeof(ApiMac_deviceDescriptor_t));
            memcpy(&nvItem.parent, pParentInfo, sizeof(Llc_netInfo_t));

            /* Write the NV item */
            pNV->writeItem(id, sizeof(nvDeviceInfo_t), &nvItem);
        }


#ifndef CUI_DISABLE
        joinStatusLineUpdate(rejoined, pDevInfo->shortAddress, pDevInfo->panID, pParentInfo->channel);
#endif /* CUI_DISABLE */

        started = true;

#ifndef POWER_MEAS
        LED_stopBlinking(gRedLedHandle);
        LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
#endif /* !POWER_MEAS */
        led1State = true;
    }
}

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 The application calls this function to store the device key information to NV
 Public function defined in ssf.h
 */
void Ssf_DeviceKeyInfoUpdate(nvDeviceKeyInfo_t *pDevKeyInfo)
{
    /* check for valid structure pointers, ignore if not */
    if(pDevKeyInfo != NULL)
    {
        if((pNV != NULL) && (pNV->writeItem != NULL))
        {
            NVINTF_itemID_t id;
            nvDeviceKeyInfo_t nvItem;

            /* Setup NV ID */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = SSF_NV_DEVICE_KEY_ID;
            id.subID = 0;

            memcpy(&nvItem, pDevKeyInfo, sizeof(nvDeviceKeyInfo_t));

            /* Write the NV item */
            pNV->writeItem(id, sizeof(nvDeviceKeyInfo_t), &nvItem);
        }
    }
}

/*!
 The application calls this function to get the device
 *              Key information.

 Public function defined in ssf.h
 */
bool Ssf_getDeviceKeyInfo(nvDeviceKeyInfo_t *pDevKeyInfo)
{
    if((pNV != NULL) && (pNV->readItem != NULL) && (pDevKeyInfo != NULL))
    {
        NVINTF_itemID_t id;
        nvDeviceKeyInfo_t nvItem;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_DEVICE_KEY_ID;
        id.subID = 0;

        /* Read Network Information from NV */
        if(pNV->readItem(id, 0, sizeof(nvDeviceKeyInfo_t),
                         &nvItem) == NVINTF_SUCCESS)
        {
            memcpy(pDevKeyInfo, &nvItem,sizeof(nvDeviceKeyInfo_t));
            return (true);
        }
    }
    return (false);
}

/*!

 Clear device key information in NV

 Public function defined in ssf.h
 */
void Ssf_clearDeviceKeyInfo( void )
{
    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_DEVICE_KEY_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        /* sensor ready to associate again */
        started = false;
    }
}
#endif /* FEATURE_SECURE_COMMISSIONING */

/*!
 The application calls this function to get the device
 *              information in a network.

 Public function defined in ssf.h
 */
bool Ssf_getNetworkInfo(ApiMac_deviceDescriptor_t *pDevInfo,
                        Llc_netInfo_t  *pParentInfo)
{
    if((pNV != NULL) && (pNV->readItem != NULL)
                    && (pDevInfo != NULL) && (pParentInfo != NULL))
    {
        NVINTF_itemID_t id;
        nvDeviceInfo_t nvItem;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_NETWORK_INFO_ID;
        id.subID = 0;

        /* Read Network Information from NV */
        if(pNV->readItem(id, 0, sizeof(nvDeviceInfo_t),
                         &nvItem) == NVINTF_SUCCESS)
        {
            memcpy(pDevInfo, &nvItem.device,
                   sizeof(ApiMac_deviceDescriptor_t));
            memcpy(pParentInfo, &nvItem.parent, sizeof(Llc_netInfo_t));

            return (true);
        }
    }
    return (false);
}



#ifdef FEATURE_NATIVE_OAD
/*!
 The application calls this function to update the OAD info in NV.

 Public function defined in ssf.h
 */
void Ssf_oadInfoUpdate(uint16_t *pOadBlock, uint8_t *pOadImgHdr, uint8_t *pOadImgId, ApiMac_sAddr_t *pOadServerAddr)
{
    NVINTF_itemID_t id;

    /* Setup NV ID */
    id.systemID = NVINTF_SYSID_APP;
    id.itemID = SSF_NV_OAD_ID;

    if(pOadBlock != NULL){
        id.subID = 0;
        /* Write the NV item */
        pNV->writeItem(id, sizeof(uint16_t), pOadBlock);
    }

    if(pOadImgHdr != NULL)
    {
        id.subID = 1;
        /* Write the NV item */
        pNV->writeItem(id, sizeof(uint8_t) * OADProtocol_IMAGE_ID_LEN, pOadImgHdr);
    }

    if(pOadImgId != NULL)
    {
        id.subID = 2;
        /* Write the NV item */
        pNV->writeItem(id, sizeof(uint8_t), pOadImgId);
    }

    if(pOadServerAddr != NULL)
    {
        id.subID = 3;
        /* Write the NV item */
        pNV->writeItem(id, sizeof(ApiMac_sAddr_t), pOadServerAddr);
    }
}

/*!
 The application calls this function to get the device
 *              information in a network.

 Public function defined in ssf.h
 */
bool Ssf_getOadInfo(uint16_t *pOadBlock, uint8_t *pOadImgHdr, uint8_t *pOadImgId, ApiMac_sAddr_t *pOadServerAddr)
{
    NVINTF_itemID_t id;
    bool status = false;

    if((pNV != NULL) && (pNV->readItem != NULL)
                    && (pOadBlock != NULL) && (pOadImgHdr != NULL))
    {
        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_OAD_ID;
        id.subID = 0;

        /* Read OAD Block from NV */
        if(pNV->readItem(id, 0, sizeof(uint16_t),
                         pOadBlock) == NVINTF_SUCCESS)
        {
            status = true;
        }

        /* Always grab server address if it exists, even if there's no OAD block to be read */
        id.subID = 3;
        /* Read OAD Server Addr from NV */
        if(pNV->readItem(id, 0, sizeof(ApiMac_sAddr_t),
                         pOadServerAddr) != NVINTF_SUCCESS)
        {
            status = false;
        }

    }

    if(status == true)
    {
        id.subID = 1;
        /* Read OAD image hdr from NV */
        if(pNV->readItem(id, 0, sizeof(uint8_t) * OADProtocol_IMAGE_ID_LEN,
                         pOadImgHdr) != NVINTF_SUCCESS)
        {
            status = false;
        }
    }

    if(status == true)
    {
        id.subID = 2;
        /* Read OAD image ID from NV */
        if(pNV->readItem(id, 0, sizeof(uint8_t),
                         pOadImgId) != NVINTF_SUCCESS)
        {
            status = false;
        }
    }

    return status;
}

/*!

 Clear OAD information in NV

 Public function defined in ssf.h
 */
void Ssf_clearOadInfo()
{
    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_OAD_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        id.subID = 1;
        pNV->deleteItem(id);

        id.subID = 2;
        pNV->deleteItem(id);

        id.subID = 3;
        pNV->deleteItem(id);

    }
}
#endif /* FEATURE_NATIVE_OAD */

/*!
 The application calls this function to indicate a Configuration
 Request message.

 Public function defined in ssf.h
 */
void Ssf_configurationUpdate(Smsgs_configRspMsg_t *pRsp)
{
    if((pNV != NULL) && (pNV->writeItem != NULL) && (pRsp != NULL))
    {
        NVINTF_itemID_t id;
        Ssf_configSettings_t configInfo;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_CONFIG_INFO_ID;
        id.subID = 0;

        configInfo.frameControl = pRsp->frameControl;
        configInfo.reportingInterval = pRsp->reportingInterval;
        configInfo.pollingInterval = pRsp->pollingInterval;

        /* Write the NV item */
        pNV->writeItem(id, sizeof(Ssf_configSettings_t), &configInfo);
    }
}

/*!
 The application calls this function to get the saved device configuration.

 Public function defined in ssf.h
 */
bool Ssf_getConfigInfo(Ssf_configSettings_t *pInfo)
{
    if((pNV != NULL) && (pNV->readItem != NULL) && (pInfo != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_CONFIG_INFO_ID;
        id.subID = 0;

        /* Read Network Information from NV */
        if(pNV->readItem(id, 0, sizeof(Ssf_configSettings_t),
                         pInfo) == NVINTF_SUCCESS)
        {
            return (true);
        }
    }
    return (false);
}

/*!
 The application calls this function to indicate that a tracking message
 was received.

 Public function defined in ssf.h
 */
void Ssf_trackingUpdate(ApiMac_sAddr_t *pSrcAddr)
{
}

/*!
 The application calls this function to indicate sensor data.

 Public function defined in ssf.h
 */
void Ssf_sensorReadingUpdate(Smsgs_sensorMsg_t *pMsg)
{
}

#ifndef DMM_CENTRAL
/*!
 Initialize the reading clock.

 Public function defined in ssf.h
 */
void Ssf_initializeReadingClock(void)
{
    /* Initialize the timers needed for this application */
    readingClkHandle = UtilTimer_construct(&readingClkStruct,
                                        processReadingTimeoutCallback,
                                        READING_INIT_TIMEOUT_VALUE,
                                        0,
                                        false,
                                        0);
}
#endif /* !DMM_CENTRAL */

/*!
 Set the reading clock.

 Public function defined in ssf.h
 */
void Ssf_setReadingClock(uint32_t readingTime)
{
    /* Stop the Reading timer */
    if(UtilTimer_isActive(&readingClkStruct) == true)
    {
        UtilTimer_stop(&readingClkStruct);
    }
#ifdef POWER_MEAS
    if(POWER_TEST_PROFILE != DATA_ACK)
    {
        /* Do not sent data in other power test profiles */
        return;
    }
#endif /* !POWER_MEAS */
#ifdef DMM_CENTRAL
    // Only send data when a BLE notification is received
    return;
#else
    /* Setup timer */
    if ( readingTime )
    {
        UtilTimer_setTimeout(readingClkHandle, readingTime);
        UtilTimer_start(&readingClkStruct);
    }
#endif
}

/*!
 Ssf implementation for memory allocation

 Public function defined in ssf.h
 */
void *Ssf_malloc(uint16_t size)
{
#ifdef OSAL_PORT2TIRTOS
    return OsalPort_malloc(size);
#else
    return ICall_malloc(size);
#endif
}

/*!
 Ssf implementation for memory de-allocation

 Public function defined in ssf.h
 */
void Ssf_free(void *ptr)
{
    if(ptr != NULL)
    {
#ifdef OSAL_PORT2TIRTOS
        OsalPort_free(ptr);
#else
        ICall_free(ptr);
#endif
    }
}

/*!
 Initialize the trickle clock.

 Public function defined in ssf.h
 */
void Ssf_initializeTrickleClock(void)
{
    /* Initialize trickle timer */
    tricklePASClkHandle = UtilTimer_construct(&tricklePASClkStruct,
                                         processPASTrickleTimeoutCallback,
                                         TRICKLE_TIMEOUT_VALUE,
                                         0,
                                         false,
                                         0);

    tricklePCSClkHandle = UtilTimer_construct(&tricklePCSClkStruct,
                                         processPCSTrickleTimeoutCallback,
                                         TRICKLE_TIMEOUT_VALUE,
                                         0,
                                         false,
                                         0);
}

/*!
 Set the trickle clock.

 Public function defined in ssf.h
 */
void Ssf_setTrickleClock(uint16_t trickleTime, uint8_t frameType)
{
    uint16_t randomNum = 0;
    if(frameType == ApiMac_wisunAsyncFrame_advertisementSolicit)
    {
        /* Stop the PA trickle timer */
        if(UtilTimer_isActive(&tricklePASClkStruct) == true)
        {
            UtilTimer_stop(&tricklePASClkStruct);
        }

        if(trickleTime > 0)
        {
            /* Trickle Time has to be a value chosen random between [t/2, t] */
            randomNum = ((ApiMac_randomByte() << 8) + ApiMac_randomByte());
            trickleTime = (trickleTime >> 1) +
                          (randomNum % (trickleTime >> 1));
            /* Setup timer */
            UtilTimer_setTimeout(tricklePASClkHandle, trickleTime);
            UtilTimer_start(&tricklePASClkStruct);
        }
    }
    else if(frameType == ApiMac_wisunAsyncFrame_configSolicit)
    {
        /* Stop the PC trickle timer */
        if(UtilTimer_isActive(&tricklePCSClkStruct) == true)
        {
            UtilTimer_stop(&tricklePCSClkStruct);
        }

        if(trickleTime > 0)
        {
            /* Setup timer */
            /* Trickle Time has to be a value chosen random between [t/2, t] */
            /* Generate a 16 bit random number */
            randomNum = ((ApiMac_randomByte() << 8) + ApiMac_randomByte());
            trickleTime = (trickleTime >> 1) +
                          (randomNum % (trickleTime >> 1));
            UtilTimer_setTimeout(tricklePCSClkHandle, trickleTime);
            UtilTimer_start(&tricklePCSClkStruct);
        }
    }
}

/*!
 Initialize the poll clock.

 Public function defined in ssf.h
 */
void Ssf_initializePollClock(void)
{
    /* Initialize the timers needed for this application */
    pollClkHandle = UtilTimer_construct(&pollClkStruct,
                                     processPollTimeoutCallback,
                                     POLL_TIMEOUT_VALUE,
                                     0,
                                     false,
                                     0);
}

/*!
 Set the poll clock.

 Public function defined in ssf.h
 */
void Ssf_setPollClock(uint32_t pollTime)
{
    /* Stop the Reading timer */
    if(UtilTimer_isActive(&pollClkStruct) == true)
    {
        UtilTimer_stop(&pollClkStruct);
    }
#if !(defined(POWER_MEAS) && \
    ((POWER_TEST_PROFILE == DATA_ACK) || (POWER_TEST_PROFILE == SLEEP)))

#ifdef FEATURE_SECURE_COMMISSIONING
    /* Setup timer */
    if(pollTime > 0)
    {
        if(SM_Current_State == SM_CM_InProgress) {
            UtilTimer_setTimeout(pollClkHandle, SM_POLLING_INTERVAL);
        }
        else {
            UtilTimer_setTimeout(pollClkHandle, pollTime);
        }
        UtilTimer_start(&pollClkStruct);
    }
#else
    /* Setup timer */
    if(pollTime > 0)
    {
        UtilTimer_setTimeout(pollClkHandle, pollTime);
        UtilTimer_start(&pollClkStruct);
    }
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /* POWER_MEAS */
}

/*!
 Get the poll clock.

 Public function defined in ssf.h
 */
uint32_t Ssf_getPollClock(void)
{
    return UtilTimer_getTimeout(pollClkHandle);
}

/*!
 Initialize the scan backoff clock.

 Public function defined in ssf.h
 */
void Ssf_initializeScanBackoffClock(void)
{
    /* Initialize the timers needed for this application */
    scanBackoffClkHandle = UtilTimer_construct(&scanBackoffClkStruct,
                                           processScanBackoffTimeoutCallback,
                                           SCAN_BACKOFF_TIMEOUT_VALUE,
                                           0,
                                           false,
                                           0);
}

/*!
 Set the scan backoff clock.

 Public function defined in ssf.h
 */
void Ssf_setScanBackoffClock(uint32_t scanBackoffTime)
{
    /* Stop the Reading timer */
    if(UtilTimer_isActive(&scanBackoffClkStruct) == true)
    {
        UtilTimer_stop(&scanBackoffClkStruct);
    }

    /* Setup timer */
    if(scanBackoffTime > 0)
    {
        UtilTimer_setTimeout(scanBackoffClkHandle, scanBackoffTime);
        UtilTimer_start(&scanBackoffClkStruct);
    }
}

/*!
 Stop the scan backoff clock.

 Public function defined in ssf.h
 */
void Ssf_stopScanBackoffClock(void)
{
    /* Stop the Reading timer */
    if(UtilTimer_isActive(&scanBackoffClkStruct) == true)
    {
        UtilTimer_stop(&scanBackoffClkStruct);
    }
}

/*!
 Initialize the FH Association delay clock.

 Public function defined in ssf.h
 */
void Ssf_initializeFHAssocClock(void)
{
    /* Initialize the timers needed for this application */
    fhAssocClkHandle = UtilTimer_construct(&fhAssocClkStruct,
                                       processFHAssocTimeoutCallback,
                                       FH_ASSOC_TIMER,
                                        0,
                                        false,
                                        0);
}

/*!
 Set the FH Association delay clock.

 Public function defined in ssf.h
 */
void Ssf_setFHAssocClock(uint32_t fhAssocTime)
{
    /* Stop the Reading timer */
    if(UtilTimer_isActive(&fhAssocClkStruct) == true)
    {
        UtilTimer_stop(&fhAssocClkStruct);
    }

    /* Setup timer */
    if ( fhAssocTime )
    {
        if(!CERTIFICATION_TEST_MODE)
        {
            /* Adding an additional random delay */
            fhAssocTime = fhAssocTime + (((ApiMac_randomByte() << 8) +
                          ApiMac_randomByte()) % ADD_ASSOCIATION_RANDOM_WINDOW);
        }
        UtilTimer_setTimeout(fhAssocClkHandle, fhAssocTime);
        UtilTimer_start(&fhAssocClkStruct);
    }
}

#if (USE_DMM) && !(DMM_CENTRAL)

/*!
 Initialize the provisioning timeout clock.

 Public function defined in ssf.h
 */
void Ssf_initializeProvisioningClock(void)
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

 Public function defined in ssf.h
 */
void Ssf_setProvisioningClock(bool provision)
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
    /* Setup timer for disassociation delay */
    else
    {
        provisioningClkHandle = UtilTimer_construct(&provisioningClkStruct,
                                       processProvisioningCallback,
                                       PROVISIONING_DISASSOC_TIMER,
                                        0,
                                        false,
                                        provision);
    }

    UtilTimer_start(&provisioningClkStruct);
}


static void processProvisioningCallback(uintptr_t provision)
{
    static bool updateProvPolicy = true;

    //Arg used to select provision and associate to a network or disassociate
    if (provision)
    {
        if (updateProvPolicy)
        {
            Ssf_setProvisioningClock(true);

            /* Update policy */
            Util_setEvent(&Sensor_events, SENSOR_PROV_EVT);

            updateProvPolicy = false;
        }
        else
        {
#ifndef CUI_DISABLE
            CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Starting");
#endif /* CUI_DISABLE */
            Util_setEvent(&Sensor_events, SENSOR_START_EVT);
            updateProvPolicy = true;
        }
    }
    else
    {
#ifndef CUI_DISABLE
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Leaving");
#endif /* CUI_DISABLE */
        Util_setEvent(&Sensor_events, SENSOR_DISASSOC_EVT);
    }

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}
#endif /* USE_DMM && !DMM_CENTRAL */

/*!
 Update the Frame Counter

 Public function defined in ssf.h
 */
void Ssf_updateFrameCounter(ApiMac_sAddr_t *pDevAddr, uint32_t frameCntr)
{
    if(pDevAddr == NULL)
    {
        if((pNV != NULL) && (pNV->writeItem != NULL) && (frameCntr >=
              (lastSavedFrameCounter + FRAME_COUNTER_SAVE_WINDOW)))
        {
            NVINTF_itemID_t id;

            /* Setup NV ID */
            id.systemID = NVINTF_SYSID_APP;
            id.itemID = SSF_NV_FRAMECOUNTER_ID;
            id.subID = 0;

            /* Write the NV item */
            if(pNV->writeItem(id, sizeof(uint32_t), &frameCntr)
                            == NVINTF_SUCCESS)
            {
                lastSavedFrameCounter = frameCntr;
            }
        }
    }
}

/*!
 Get the Frame Counter

 Public function defined in ssf.h
 */
bool Ssf_getFrameCounter(ApiMac_sAddr_t *pDevAddr, uint32_t *pFrameCntr)
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
                id.itemID = SSF_NV_FRAMECOUNTER_ID;
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
                    id.itemID = SSF_NV_FRAMECOUNTER_ID;
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
 Display Error

 Public function defined in ssf.h
 */
void Ssf_displayError(const char *pTxt, uint8_t code)
{
#ifndef CUI_DISABLE
    CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "%s0x%02x", pTxt, code);
#endif /* CUI_DISABLE */
}

/*!
 Assert Indication

 Public function defined in ssf.h
 */
void Ssf_assertInd(uint8_t reason)
{
    if((pNV != NULL) && (pNV->writeItem != NULL))
    {
        /* Attempt to save reason to read after reset */
        (void)pNV->writeItem(nvResetId, 1, &reason);
    }
}

/*!

 Clear network information in NV

 Public function defined in ssf.h
 */
void Ssf_clearNetworkInfo()
{
    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        NVINTF_itemID_t id;

        /* Setup NV ID */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_NETWORK_INFO_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        /* sensor ready to associate again */
        started = false;
    }
}

/*!
 Clear all the NV Items

 Public function defined in ssf.h
 */
void Ssf_clearAllNVItems(void)

{
    /* Clear Network Information */
    Ssf_clearNetworkInfo();

#ifdef FEATURE_NATIVE_OAD
    /* Clear OAD Information */
    Ssf_clearOadInfo();
#endif

    if((pNV != NULL) && (pNV->deleteItem != NULL))
    {
        NVINTF_itemID_t id;

        /* Clear the device tx frame counter */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_FRAMECOUNTER_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        /* Clear the reset reason */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_RESET_REASON_ID;
        id.subID = 0;
        pNV->deleteItem(id);

        /* Clear the reset count */
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_RESET_COUNT_ID;
        id.subID = 0;
        pNV->deleteItem(id);

#ifdef FEATURE_SECURE_COMMISSIONING
        /* Clear the key info*/
        id.systemID = NVINTF_SYSID_APP;
        id.itemID = SSF_NV_DEVICE_KEY_ID;
        id.subID = 0;
        pNV->deleteItem(id);
#endif /* FEATURE_SECURE_COMMISSIONING */

    }
}

/*!
 Read the on-board temperature sensors

 Public function defined in ssf.h
 */
int16_t Ssf_readTempSensor(void)
{
#ifdef POWER_MEAS
    return (0);
#else
    return ((int16_t)AONBatMonTemperatureGetDegC());
#endif /* POWER_MEAS */
}

/*!
 The application calls this function to toggle an LED.

 Public function defined in ssf.h
 */
bool Ssf_toggleLED(void)
{
    if(led1State == true)
    {
        led1State = false;
#ifndef POWER_MEAS
        LED_stopBlinking(gRedLedHandle);
        LED_setOff(gRedLedHandle);
#endif /* !POWER_MEAS */
    }
    else
    {
        led1State = true;
#ifndef POWER_MEAS
        LED_stopBlinking(gRedLedHandle);
        LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
#endif /* !POWER_MEAS */
    }

    return(led1State);
}

/*!
 The application calls this function to switch on LED.

 Public function defined in ssf.h
 */
void Ssf_OnLED(void)
{
    if(led1State == false)
    {
        led1State = true;
#ifndef POWER_MEAS
        LED_stopBlinking(gRedLedHandle);
        LED_setOn(gRedLedHandle, LED_BRIGHTNESS_MAX);
#endif /* !POWER_MEAS */
    }
}

/*!
 The application calls this function to switch off LED.

 Public function defined in ssf.h
 */
void Ssf_OffLED(void)
{
    if(led1State == true)
    {
        led1State = false;
#ifndef POWER_MEAS
        LED_stopBlinking(gRedLedHandle);
        LED_setOff(gRedLedHandle);
#endif /* !POWER_MEAS */
    }
}

/*!
  A callback calls this function to post the application task semaphore.

 Public function defined in ssf.h
 */
void Ssf_PostAppSem(void)
{
    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

#ifdef LPSTK
/*!
  A callback calls this function to pend the application task semaphore.

 Public function defined in ssf.h
 */
void Ssf_PendAppSem(void)
{
    /* Halt the application thread when it waits for clock event */
    sem_wait(sensorSem);
}
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 The application calls this function to get a passkey.

 Public function defined in ssf.h
 */
void Ssf_SmPasskeyEntry(SM_passkeyEntry_t passkeyAction)
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
        CUI_deRegisterMenu(ssfCuiHndl, &ssfMainMenu);
        smMenuUsed = 1;

        // request a menu if available
        CUI_registerMenu(ssfCuiHndl, &smPassKeyMenu);

        // Open the menu itself
        // there is only 1 item in smPassKeyMenu list.
        CUI_menuNav(ssfCuiHndl, &smPassKeyMenu, 0);
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
        CUI_deRegisterMenu(ssfCuiHndl, &smPassKeyMenu);

        // Only re-enable the main menu if it was previously disabled
        if(smMenuUsed == 1)
        {
            // re-register the main menu again
            CUI_registerMenu(ssfCuiHndl, &ssfMainMenu);
        }

        // Go back to the help screen which is the last menu in the list.
        // third argument represents the index of the menu to travel to.
        CUI_menuNav(ssfCuiHndl, &ssfMainMenu, ssfMainMenu.numItems - 1);
#endif /* CUI_DISABLE */
    }
}
#endif /* FEATURE_SECURE_COMMISSIONING */

/******************************************************************************
 Local Functions
 *****************************************************************************/

#ifndef DMM_CENTRAL
/*!
 * @brief   Reading timeout handler function.
 *
 * @param   a0 - ignored
 */
static void processReadingTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Sensor_events, SENSOR_READING_TIMEOUT_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}
#endif /* !DMM_CENTRAL */

/*!
 * @brief       Key event handler function
 *
 * @param       keysPressed - keys that are pressed
 */
static void processKeyChangeCallback(Button_Handle _buttonHandle, Button_EventMask _buttonEvents)
{
    if (_buttonEvents & Button_EV_CLICKED)
    {
        keys = _buttonHandle;
        events |= KEY_EVENT;

        /* Wake up the application thread when it waits for keys event */
        sem_post(sensorSem);
    }
}

/*!
 * @brief       Trickle timeout handler function for PA .
 *
 * @param       a0 - ignored
 */
static void processPASTrickleTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Jdllc_events, JDLLC_PAS_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

/*!
 * @brief       Trickle timeout handler function for PC.
 *
 * @param       a0 - ignored
 */
static void processPCSTrickleTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Jdllc_events, JDLLC_PCS_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

/*!
 * @brief       Poll timeout handler function  .
 *
 * @param       a0 - ignored
 */
static void processPollTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Jdllc_events, JDLLC_POLL_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

/*!
 * @brief       Scan backoff timeout handler function  .
 *
 * @param       a0 - ignored
 */
static void processScanBackoffTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Jdllc_events, JDLLC_SCAN_BACKOFF);

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

/*!
 * @brief       FH Assoc Delay timeout handler function  .
 *
 * @param       a0 - ignored
 */
static void processFHAssocTimeoutCallback(uintptr_t a0)
{
    (void)a0; /* Parameter is not used */

    Util_setEvent(&Jdllc_events, JDLLC_ASSOCIATE_REQ_EVT);

    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

#ifndef CUI_DISABLE
#if defined(DISPLAY_PER_STATS)
/*!
 * @brief       The application calls this function to print updated sensor stats to the display.
 */
void Ssf_displayPerStats(Smsgs_msgStatsField_t* pstats)
{
    int per;
    macStatisticsStruct_t macStats;
    ApiMac_mlmeGetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    int failures = pstats->macAckFailures + pstats->otherDataRequestFailures;
    per = (100000 * failures) / (pstats->msgsSent + failures);

    CUI_statusLinePrintf(ssfCuiHndl, perStatusLine, "MsgsSent = %d, CcaRejects=%d, UCastRetries=%d, UCastFail=%d",
                         pstats->msgsSent,
                         pstats->channelAccessFailures,
                         macStats.mac_tx_ucast_retry,
                         pstats->macAckFailures);

    CUI_statusLinePrintf(ssfCuiHndl, macStatsLine1,
                         "avgE2EDelay=%d, worstCaseE2EDelay = %d",
                         Sensor_msgStats.avgE2EDelay,
                         Sensor_msgStats.worstCaseE2EDelay);
}

void Ssf_displayRxStats(int8_t rssi)
{
    CUI_statusLinePrintf(ssfCuiHndl, RxStatsLine,
                     "AppPktRcvd=%d, rssi = %d",
                     Sensor_msgStats.trackingRequests, rssi);
}
#endif /* DISPLAY_PER_STATS */

/**
 *  @brief Callback to be called when the UI sets PAN ID.
 */

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


static void setPanIdAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  static CUI_cursorInfo_t cursor = {0, 6};
  static bool initPanId = true;
  static uint16_t panId;

  const char tmpInput[2] = {_input, '\0'};

  if(initPanId)
  {
      Jdllc_getJoiningPanId(&panId);
      initPanId = false;
  }

  switch (_input) {
      case CUI_ITEM_INTERCEPT_START:
      {
          Jdllc_getJoiningPanId(&panId);
          break;
      }
      // Submit the final modified value
      case CUI_ITEM_INTERCEPT_STOP:
      {
          Jdllc_setJoiningPanId(panId);

          // Reset the local cursor info
          cursor.col = 6;
          break;
      }
      // Show the value of this screen w/o making changes
      case CUI_ITEM_PREVIEW:
          Jdllc_getJoiningPanId(&panId);
          break;
      // Move the cursor to the left
      case CUI_INPUT_LEFT:
      {
          // 6 is the left boundary of the cursor, and we don't need to skip spaces.
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
          // multiply by 4 to use binary
          // get the position of the nibble you want to change.
          uint8_t shift = (uint8_t)(4 * (9 - cursor.col));

          // make sure you don't exceed the left boundary
          if (6 <= cursor.col)
          {
              // clear the nibble to default to F
              panId |= (uint32_t)(0x0F<<shift) ;
              cursor.col = moveCursorLeft(cursor.col, 6, 9, 0);

          }
          break;
      }
      case CUI_INPUT_EXECUTE:
          break;
      default:
      {
          // is it a hex number
          if(CUI_IS_INPUT_HEX(_input))
          {
              /* multiply by 4 because you're working with binary numbers,
              and half a byte of hex = 4 bits. Calculate the bit shift
              based on the end of the line - the current column, to
              modify specific values.  */
              uint8_t shift = 4 * (9 - cursor.col);

              // convert from ascii to hex
              uint8_t digit = strtol(tmpInput, NULL, 16);

              // first clear the specific hex half byte, in the desired spot
              panId &= ~((uint32_t)0xF << shift);
              // then set the digit you have typed in.
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
    uintToString( panId, tmp, 16, 4, true, false);
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
 * @brief       Validate and handle errors in channel mask entered through a UI
 *
 * @param       _chanMask - channel mask updated with user input
 * @param       byteIdx   - index of modified byte
 */
void Ssf_validateChMask(uint8_t *_chanMask, uint8_t byteIdx)
{
    // Verify user input by comparing against valid channel mask
    uint8_t validChannelByte = _chanMask[byteIdx] & validChannelMask[byteIdx];
    if (validChannelByte != _chanMask[byteIdx])
    {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine,
                             "Invalid input. Only updated with supported channels");

        // Only accept inputs that represent supported channels
        _chanMask[byteIdx] = validChannelByte;
    }
    else
    {
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "");
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
        Jdllc_getChanMask(channelMask);
        initChanMask = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            // Reset the local cursor info
            cursor.col = 1;
            Jdllc_getChanMask(channelMask);
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Jdllc_setChanMask(channelMask);

            // Reset the local cursor info
            cursor.col = 1;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            Jdllc_getChanMask(channelMask);
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
            // get the exact nibble you want to modify.
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
                Ssf_validateChMask(channelMask, byteIdx);

                cursor.col = moveCursorRight(cursor.col, 1, 50, 1);
            }
        }
    }

    for(chanMaskByteIdx = 0; chanMaskByteIdx < APIMAC_154G_CHANNEL_BITMAP_SIZ; chanMaskByteIdx++)
    {
        char tmp[2];

        uintToString( channelMask[chanMaskByteIdx], tmp, 16, 2, true, false);

        // clear the label
        strcpy((_pLines[0] + chanMaskStrIdx), " ");
        chanMaskStrIdx += 1;
        strncat((_pLines[0] + chanMaskStrIdx), tmp, 2);
        chanMaskStrIdx += 2;
    }

    if (_input != CUI_ITEM_PREVIEW)
    {
        // set the label in the right place.
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
        Jdllc_getAsyncChanMask(channelMask);
        initChanMask = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            Jdllc_getAsyncChanMask(channelMask);
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Jdllc_setAsyncChanMask(channelMask);

            // Reset the local cursor info
            cursor.col = 1;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            Jdllc_getAsyncChanMask(channelMask);
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
            Ssf_validateChMask(channelMask, byteIdx);

            cursor.col = moveCursorRight(cursor.col, 1, 50, 1);
        }

    }

    for(chanMaskByteIdx = 0; chanMaskByteIdx < APIMAC_154G_CHANNEL_BITMAP_SIZ; chanMaskByteIdx++)
    {
        char tmp[2];

        uintToString( channelMask[chanMaskByteIdx], tmp, 16, 2, true, false);

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
#endif

#ifdef FEATURE_MAC_SECURITY
/**
 *  @brief Callback to be called when the UI sets Default Network Key.
 */
static void setNwkKeyAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint8_t defaultNwkKey[APIMAC_KEY_MAX_LEN];
    static bool initChanMask = true;
    static CUI_cursorInfo_t cursor = {0, 1};
    uint8_t defaultNwkKeyByteIdx = 0;
    uint8_t defaultNwkKeyStrIdx = 0;

    const char tmpInput[2] = {_input, '\0'};

    if(initChanMask)
    {
        Jdllc_getDefaultKey(defaultNwkKey);
        initChanMask = false;
    }

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            Jdllc_getDefaultKey(defaultNwkKey);
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            Jdllc_setDefaultKey(defaultNwkKey);

            // Reset the local cursor info
            cursor.col = 1;
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            Jdllc_getDefaultKey(defaultNwkKey);
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
            // get exact position of nibble you want to modify
            uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
            uint8_t byteIdx = nibbleIdx / 2;

            // don't exceed left boundary
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
                     // get exact position of nibble you want to modify
                     uint8_t nibbleIdx = (cursor.col-1) - ((cursor.col -1)/3);
                     uint8_t byteIdx = nibbleIdx / 2;

                     // you're at the left side
                     if(nibbleIdx % 2 == 0)
                         {
                             // First, clear the left side, keep the right side
                             defaultNwkKey[byteIdx] &= (uint32_t)(0x0F);
                             // Next, shift the input left, and or it with the existing side.
                             defaultNwkKey[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16) << 4);
                         }
                     // you're at the left side
                     else
                         {
                             // First, clear the right side, keep the left side
                             defaultNwkKey[byteIdx] &= (uint32_t)(0xF0);
                             // Next, use the input to or it with the existing side.
                             // strtol will convert from hex ascii to hex integers
                             defaultNwkKey[byteIdx] |= (uint32_t)(strtol(tmpInput, NULL, 16));
                         }

                     cursor.col = moveCursorRight(cursor.col, 1, 47, 1);
                 }

     }
    }

    for(defaultNwkKeyByteIdx = 0; defaultNwkKeyByteIdx < APIMAC_KEY_MAX_LEN; defaultNwkKeyByteIdx++)
    {
        char tmp[2];

        uintToString( defaultNwkKey[defaultNwkKeyByteIdx], tmp, 16, 2, true, false);

        // clear the label
        strcpy((_pLines[0] + defaultNwkKeyStrIdx), " ");
        defaultNwkKeyStrIdx += 1;
        strncat((_pLines[0] + defaultNwkKeyStrIdx), tmp, 2);
        defaultNwkKeyStrIdx += 2;
    }

    // set the label if you're not doing a preview.
    if (_input != CUI_ITEM_PREVIEW)
    {
        strcpy(_pLines[2], "     DEFAULT NWK KEY");
        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;
    }
}
#endif /* FEATURE_MAC_SECURITY */

#ifdef FEATURE_SECURE_COMMISSIONING
static void setSmSetAuthModeAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static CUI_cursorInfo_t cursor = {0, 0};
    static SMMsgs_authMethod_t smAuthMethod;

    switch (_input) {
        case CUI_ITEM_INTERCEPT_START:
        {
            smAuthMethod = Sensor_getSmAuthMethod();
            break;
        }
        // Submit the final modified value
        case CUI_ITEM_INTERCEPT_STOP:
        {
            //Set Auth Method
            Sensor_setSmAuthMethod(smAuthMethod);
            break;
        }
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            smAuthMethod = Sensor_getSmAuthMethod();
            break;
        // Decrease authMode
        case CUI_INPUT_LEFT:
            if(smAuthMethod == SMMsgs_authMethod_passkey)
            {
                smAuthMethod = SMMsgs_authMethod_justAllowed;
            }
            else if(smAuthMethod == SMMsgs_authMethod_justAllowed)
            {
                smAuthMethod = SMMsgs_authMethod_defaultCode;
            }
            else
            {
                smAuthMethod = SMMsgs_authMethod_passkey;
            }
            break;

        // Increase authMode
        case CUI_INPUT_RIGHT:
            if(smAuthMethod == SMMsgs_authMethod_passkey)
            {
                smAuthMethod = SMMsgs_authMethod_defaultCode;
            }
            else if(smAuthMethod == SMMsgs_authMethod_defaultCode)
            {
                smAuthMethod = SMMsgs_authMethod_justAllowed;
            }
            else
            {
                smAuthMethod = SMMsgs_authMethod_passkey;
            }
            break;
        case CUI_INPUT_DOWN:
            break;
        case CUI_INPUT_UP:
            break;
        case CUI_INPUT_EXECUTE:
            break;
        default:
            break;
    }

    if (_input != CUI_ITEM_PREVIEW) {
        strcpy(_pLines[2], "   AUTH METHOD    ");

        _pCurInfo->row = 1;
        _pCurInfo->col = cursor.col+1;

        if(smAuthMethod == SMMsgs_authMethod_passkey)
        {
            strcpy(_pLines[0], "<   PASSKEY    >");
        }
        else if(smAuthMethod == SMMsgs_authMethod_defaultCode)
        {
            strcpy(_pLines[0], "< DEFAULT CODE >");
        }
        else
        {
            strcpy(_pLines[0], "<  JUST ALLOW  >");
        }
    }
    else
    {
        if(smAuthMethod == SMMsgs_authMethod_passkey)
        {
            strcpy(_pLines[0], "    PASSKEY     ");
        }
        else if(smAuthMethod == SMMsgs_authMethod_defaultCode)
        {
            strcpy(_pLines[0], "  DEFAULT CODE  ");
        }
        else
        {
            strcpy(_pLines[0], "   JUST ALLOW  ");
        }
    }

}

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
#endif /* FEATURE_SECURE_COMMISSIONING */

/**
 *  @brief Callback to be called when the UI associates.
 */
static void assocAction(int32_t menuEntryInex)
{
    CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Starting");

    /* Tell the sensor to start */
    Util_setEvent(&Sensor_events, SENSOR_START_EVT);
    /* Wake up the application thread when it waits for clock event */
    sem_post(sensorSem);
}

/**
 *  @brief Callback to be called when the UI disassociates.
 */
static void disassocAction(int32_t menuEntryInex)
{

    if ((Jdllc_getProvState() == Jdllc_states_joined) ||
    (Jdllc_getProvState() == Jdllc_states_rejoined))
    {
        // Only send the disassociation if you're in the network.
        CUI_statusLinePrintf(ssfCuiHndl, sensorStatusLine, "Leaving");

#ifdef FEATURE_SECURE_COMMISSIONING
        if (SM_Current_State == SM_CM_InProgress)
        {
          SM_stopCMProcess();
        }
        else
        {
          Jdllc_sendDisassociationRequest();
        }
#else
        Jdllc_sendDisassociationRequest();
#endif /* FEATURE_SECURE_COMMISSIONING */
    }
}

/**
 *  @brief Callback to be called when the UI sets PAN ID.
 */
static void collectorLedIndentifyAction(int32_t menuEntryInex)
{
    events |= SENSOR_SEND_COLLECTOR_IDENT_EVT;

    // Wake up the application thread when it waits for clock event
    sem_post(sensorSem);
}

/**
 *  @brief Send process menu event.
 */
static void processMenuUpdate(void)
{
    events |= SENSOR_UI_INPUT_EVT;

    // Wake up the application thread when it waits for clock event
    sem_post(sensorSem);
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
            events |= COEX_ACTION_SET_NUM_PACKET;

            if(atoi(numPacketASCII) == 0x00)
            {
                memcpy(numPacketASCII, "00010000", 8);
            }

            numPacket = atoi(numPacketASCII);

            // Wake up the application thread when it waits for clock event
            sem_post(sensorSem);

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

    /* Reset Stats */

    macStatisticsStruct_t macStats;

    memset(&macStats, 0, sizeof(macStats));
    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    Sensor_msgStats.msgsSent              = 0;
    Sensor_msgStats.channelAccessFailures = 0;
    Sensor_msgStats.otherDataRequestFailures = 0;
    Sensor_msgStats.macAckFailures        = 0;
    Sensor_msgStats.avgE2EDelay           = 0;
    Sensor_msgStats.worstCaseE2EDelay     = 0;
    Sensor_msgStats.trackingRequests      = 0;
    totalE2EDelaySum = 0;
    Ssf_displayRxStats(0);
    Ssf_displayPerStats(&Sensor_msgStats);

    /* end reset Stats */
}
extern Smsgs_configReqMsg_t configSettings;
/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void startCoexTest(int32_t menuEntryInex)
{
    /* Reset Stats */

    macStatisticsStruct_t macStats;

    memset(&macStats, 0, sizeof(macStats));
    ApiMac_mlmeSetMacStatsReqStruct(ApiMac_macAttribute_macStatistics,
                                &macStats);

    Sensor_msgStats.msgsSent              = 0;
    Sensor_msgStats.channelAccessFailures = 0;
    Sensor_msgStats.otherDataRequestFailures = 0;
    Sensor_msgStats.macAckFailures        = 0;
    Sensor_msgStats.avgE2EDelay           = 0;
    Sensor_msgStats.worstCaseE2EDelay     = 0;
    Sensor_msgStats.trackingRequests      = 0;
    totalE2EDelaySum = 0;
    /* end reset Stats */

    /* Start the reporting timer */
    Ssf_setReadingClock(configSettings.reportingInterval);
    nSensorPkts = 0;
#if 0
    events |= COEX_ACTION_START;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(sensorSem);
#endif
}

/**
 *  @brief Callback to be called when the UI selects toggle.
 */
static void endCoexTest(int32_t menuEntryInex)
{

    /* Stop the reporting timer */
    Ssf_setReadingClock(0);
    numPacket = 0;
    nSensorPkts = 0;
}
#endif // defined(IEEE_COEX_TEST) && defined(COEX_MENU)

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
/*********************************************************************
 * @fn      Ssf_blockModeTestClockHandler
 *
 * @brief   Handler function for clock timeouts.
 *
 * @param   arg - event type
 *
 * @return  None.
 */
static void Ssf_blockModeTestClockHandler(UArg arg)
{
  // stop the timer
  UtilTimer_stop(&clkBlockModeTestStruct);

  if (DMMPolicy_getBlockModeStatus(DMMPolicy_StackRole_154Sensor))
  {
    // update the DMM Block Mode status
    DMMPolicy_setBlockModeOff(DMMPolicy_StackRole_154Sensor);

    // restart the timer with new timeout value
    UtilTimer_setTimeout(clkBlockModeTestHandle, SSF_BLOCK_MODE_OFF_PERIOD);
    UtilTimer_start(&clkBlockModeTestStruct);
  }
  else
  {
    // update the DMM Block Mode status
    DMMPolicy_setBlockModeOn(DMMPolicy_StackRole_154Sensor);

    // restart the timer with new timeout value
    UtilTimer_setTimeout(clkBlockModeTestHandle, SSF_BLOCK_MODE_ON_PERIOD);
    UtilTimer_start(&clkBlockModeTestStruct);
  }
}

/*********************************************************************
 * @fn      Ssf_blockModeTestOn
 *
 * @brief   Turn the periodic block mode on for BLE.
 *
 * @param   menuEntryIndex - index of CUI menu option
 */
static void Ssf_blockModeTestOn(int32_t menuEntryIndex)
{
  if (!UtilTimer_isActive(&clkBlockModeTestStruct))
  {
    UtilTimer_setTimeout(clkBlockModeTestHandle, SSF_BLOCK_MODE_ON_PERIOD);
    UtilTimer_start(&clkBlockModeTestStruct);
    DMMPolicy_setBlockModeOn(DMMPolicy_StackRole_154Sensor);
    CUI_statusLinePrintf(ssfCuiHndl, ssfStatusLineBlockModeTestState, "Enabled");
  }
}

/*********************************************************************
 * @fn      Ssf_blockModeTestOff
 *
 * @brief   Turn the periodic block mode off for BLE.
 *
 * @param   menuEntryIndex - index of CUI menu option
 */
static void Ssf_blockModeTestOff(int32_t menuEntryIndex)
{
  if (UtilTimer_isActive(&clkBlockModeTestStruct))
  {
      UtilTimer_stop(&clkBlockModeTestStruct);
  }
  DMMPolicy_setBlockModeOff(DMMPolicy_StackRole_154Sensor);
  CUI_statusLinePrintf(ssfCuiHndl, ssfStatusLineBlockModeTestState, "Disabled");
}


/*********************************************************************
 * @fn      Ssf_setBlockModeOnPeriodAction
 *
 * @brief   Handle a user input to update the Block Mode On Period value
 *
 * @param   _input - input character
 * @param   _pLines - action menu title
 * @param   _pCurInfo - current cursor info
 */
static void Ssf_setBlockModeOnPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  Ssf_setBlockModePeriodUiAction(&SSF_BLOCK_MODE_ON_PERIOD, _input, _pLines, _pCurInfo);
}

/*********************************************************************
 * @fn      Ssf_setBlockModeOffPeriodAction
 *
 * @brief   Handle a user input to update the Block Mode Off Period value
 *
 * @param   _input - input character
 * @param   _pLines - action menu title
 * @param   _pCurInfo - current cursor info
 */
static void Ssf_setBlockModeOffPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  Ssf_setBlockModePeriodUiAction(&SSF_BLOCK_MODE_OFF_PERIOD, _input, _pLines, _pCurInfo);
}

/*********************************************************************
 * @fn      Ssf_setBlockModePeriodUiAction
 *
 * @brief   Handle a user input to update the Block Mode Off Period value
 *
 * @param   blockModePeriod - Block Mode Period
 * @param   _input - input character
 * @param   _pLines - action menu title
 * @param   _pCurInfo - current cursor info
 */
static void Ssf_setBlockModePeriodUiAction(uint16_t* blockModePeriod, const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  static char periodValArr[5] = {};
  static CUI_cursorInfo_t cursor = {0, 4};

  switch (_input) {
    case CUI_ITEM_INTERCEPT_START:
    {
      sprintf(periodValArr, "%04d", *blockModePeriod);
      break;
    }
    // Submit the final modified value
    case CUI_ITEM_INTERCEPT_STOP:
    {
      *blockModePeriod = atoi(periodValArr);
      // Reset the local cursor info
      cursor.col = 4;
      break;
    }
    // Move the cursor to the left
    case CUI_INPUT_LEFT:
    {
      cursor.col = moveCursorLeft(cursor.col, 4, 7, 0);
      break;
    }
    // Move the cursor to the right
    case CUI_INPUT_RIGHT:
    {
      cursor.col = moveCursorRight(cursor.col, 4, 7, 0);
      break;
    }
    default:
    {
      // is the input a number
      if(CUI_IS_INPUT_NUM(_input))
      {
        periodValArr[cursor.col - 4] = _input;
        cursor.col = moveCursorRight(cursor.col, 4, 7, 0);
      }
      else
      {
        sprintf(periodValArr, "%04d", *blockModePeriod);
      }
    }
  }

  snprintf(_pLines[0], 16, "    %4s      ", periodValArr);

  if (_input != CUI_ITEM_PREVIEW)
  {
    if (blockModePeriod == &SSF_BLOCK_MODE_ON_PERIOD)
    {
      strcpy(_pLines[2], "BM ON Period (ms)");
    }
    else if (blockModePeriod == &SSF_BLOCK_MODE_OFF_PERIOD)
    {
      strcpy(_pLines[2], "BM OFF Period (ms)");
    }
    _pCurInfo->row = 1;
    _pCurInfo->col = cursor.col+1;
  }
}

#endif
#endif /* CUI_DISABLE */
