/**************************************************************************************************
  Filename:       zcl_sampleapps_ui.c
  Revised:        $Date: 2016-8-1 16:04:46 -0700 (Fri, 24 Oct 2014) $
  Revision:       $Revision: 40796 $


  Description:    Z-Stack Sample Application User Interface.


  Copyright 2006-2016 Texas Instruments Incorporated.

  All rights reserved not granted herein.
  Limited License.

  Texas Instruments Incorporated grants a world-wide, royalty-free,
  non-exclusive license under copyrights and patents it now or hereafter
  owns or controls to make, have made, use, import, offer to sell and sell
  ("Utilize") this software subject to the terms herein. With respect to the
  foregoing patent license, such license is granted solely to the extent that
  any such patent is necessary to Utilize the software alone. The patent
  license shall not apply to any combinations which include this software,
  other than combinations with devices manufactured by or for TI ("TI
  Devices"). No hardware patent is licensed hereunder.

  Redistributions must preserve existing copyright notices and reproduce
  this license (including the above copyright notice and the disclaimer and
  (if applicable) source code license limitations below) in the documentation
  and/or other materials provided with the distribution.

  Redistribution and use in binary form, without modification, are permitted
  provided that the following conditions are met:

    * No reverse engineering, decompilation, or disassembly of this software
      is permitted with respect to any software provided in binary form.
    * Any redistribution and use are licensed by TI for use only with TI Devices.
    * Nothing shall obligate TI to provide you with source code for the software
      licensed and provided to you in object code.

  If software source code is provided to you, modification and redistribution
  of the source code are permitted provided that the following conditions are
  met:

    * Any redistribution and use of the source code, including any resulting
      derivative works, are licensed by TI for use only with TI Devices.
    * Any redistribution and use of any object code compiled from the source
      code and any resulting derivative works, are licensed by TI for use
      only with TI Devices.

  Neither the name of Texas Instruments Incorporated nor the names of its
  suppliers may be used to endorse or promote products derived from this
  software without specific prior written permission.

  DISCLAIMER.

  THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
  IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/*********************************************************************
  This file implements the user interface that is common to most of the Z-Stack sample applications.

  The sample applications are intended for the following platforms:
  - LAUNCHXL-CC26X2R1
  - LAUNCHXL-CC1352R1
  - LAUNCHXL-CC1352P-2

  The UI peripherals being used:

  - UART based menu system:
    To display only using 3 lines of 18 characters each.

    As input the following keys are used to interact/navigate with the menu system, where applicable:

      - Left + Right arrow key: depending on the menu screen, these are used for either:
        - move to the previous/next menu screen
        - move to the previous/next digit/sub-item within a menu screen
      - Up + Down arrow key: change the value of the currently selected item
      - Enter key (Select): execute the operation associated with the current menu screen
      - Esc key: go back to previous higher level menu

  - Switchs:
    Two switches are used differently by the individual applications. This UI does not use them.

  - LEDs:
    LED1 is used differently by the individual applications.

    LED2:
      - Off: device is not on a network
      - On: device on a network
      - Blinking, 1 second period, 50% duty cycle: device is identifying

  The menu system:

    <HELP> Welcome screen
      Displays the sample application name.
      Press [OK] to show a simple help screen.

      Help screen
      Press [OK] to go back to the welcome screen.

    <CONFIGURE> Configuration sub-menu
      Press [OK] to display the configuration sub-menu

      <ADD INSTL CODE> install code sub-menu
        Press [OK] to display the install code sub-menu

        <SET INSTL CODE> set install code
          Press [OK] to to edit the install code

          Install code editing screen
            Press [LEFT] / [RIGHT] to select a digit
            Press [UP] / [DOWN] to change the selected digit
            press [OK] to to go back to the higher menu level

        <SET I.C. ADDR> set install code address (coordinator only)
          Press [OK] to to edit the install code address

          Install code address editing screen
            Press [LEFT] / [RIGHT] to select a digit
            Press [UP] / [DOWN] to change the selected digit
            press [OK] to to go back to the higher menu level

        <APLY INST CODE> apply install code
          Press [OK] to apply the address+Install-Code pair (coordinator) or the Install-Code (Routers and End-Devices)
          This menu screen also displays the status of the last apply-install-code operation

        <BACK> go back to higher menu level
          press [OK] to to go back to the higher menu level

      <T.L. TRGT TIME> Set the touchlink enable duration
        Press [UP] / [DOWN] to increase / decrease the displayed time.
        The maximum duration is 86400 seconds. Increasing it above this value will display '(forever)' -
        in this setting, the touchlink target will stay active once started and untill manually stopped.
        The minimum duration is 1 second. Decreasing it below this value sets tohchlink target to always disabled.
        This menu item is only available if BDB_TL_TARGET is defined.

      <T.L. STEALING> enable/disable touchlink stealing
        Press [OK] to toggle (enable/disable) touchlink stealing.
        When stealing is enabled, a touchlink target may be 'stolen' by another device acting as a touchlink
        initiator, which will take it out of its existing network and add it to another network.
        This menu item is only available if BDB_TL_TARGET is defined.

      <T.L. INITIATOR> enable/disable touchlink initiator
        Press [OK] to toggle (enable/disable) touchlink initiator.
        When enabled, touchlink initiator functionality will be executed as part of the commissioning procedure.
        This menu item is only available if BDB_TL_INITIATOR is defined.

      <NWK FORMATION> enable/disable network formation for when cannot connect to an existing network
        Press [OK] to toggle (enable/disable) network formation.
        When enabled, network formation will be executed as part of the commissioning procedure, unless
        the device has already joined a network.

      <NWK STEERING> enable/disable attempting to connect to an existing network
        Press [OK] to toggle (enable/disable) network steering.
        When enabled, network steering will be executed as part of the commissioning procedure, so the
        device will try to join an existing network, and will open the network for joining of other devices
        once it is joined.

      <FINDNG+BINDNG> enable/disable finding and binding
        Press [OK] to toggle (enable/disable) finding and binding (F&B).
        When enabled, finding and binding will be executed as part of the commissioning procedure, so the
        device will either start identifying, look for matchng devices that are currently identifying, or
        both (depending of whether it is a F&B target, initiator, or both).

      <PRI CHANL MASK> set the primary channel mask
        Press [OK] to edit the primary channel list

        Primary channel list editing screen
          Press [LEFT] / [RIGHT] to select a channel (from channel 11 to channel 26)
          Press [UP] / [DOWN] to enable / disable the selected channel, accordingly.
          press [OK] to to go back to the higher menu level

      <SEC CHANL MASK> set the secondary channel mask
        Press [OK] to edit the secondary channel list

        Secondary channel list editing screen
          Press [LEFT] / [RIGHT] to select a channel (from channel 11 to channel 26)
          Press [UP] / [DOWN] to enable / disable the selected channel, accordingly
          press [OK] to to go back to the higher menu level

      <PAN ID> set the PAN ID
        Press [OK] to edit the PAN ID to create / connect to

        PAN ID editing screen
          Press [LEFT] / [RIGHT] to select the digit to edit
          Press [UP] / [DOWN] to change the selected digit
          press [OK] to to go back to the higher menu level

      <BACK> go back to higher menu level
        press [OK] to to go back to the higher menu level

    <COMMISSION> start commissioning
      Press [OK] to start the commissioning procedure. (Note: this is disabled if a previous comissioning is
      already on-going).
      This procedure will execute the following methods, depending on whether they are enabled or disabled
      in the respective configuration item):
        - Touchlink (as Touchlink Initiator)
        - Network Steering
        - Network Formation
        - Finding and Binding

    <T.L. TARGET> start touchlink target
      Press [OK] to start / stop touchlink target functionality.
      When not active, pressing [OK] will start touchlink target for the duration defined by the respective
      configuration item.
      When active, pressing OK will immediately stop touchlink target functionality.
      This menu item is only available if BDB_TL_TARGET is defined.

    <APP MENU> application-specific sub-menu
      Press [OK] to enter the application-specific sub-menu.
      See the main comment in the specific application c file for more details.

    <RESET TO FN> reset the device to factory-new
      Press [OK] to reset the device to factory-new. The device will disconnect from the network, and
      all configurations and network information will be deleted.

    The Device Info line will display the following information:
      [IEEE Addr]
        Device IEEE address
      [Power Mode]
        RX Always On or Sleepy

    The NWK Info line will display the following information:
      [PAN ID]
        0xXXXX - the Network pan-id
      [Channel]
        XX - the IEEE Channel the network is active on, 11 - 26
      [Short Address]
        0xXXXX - the short address of the device
      [Parent Address] (only applicable to ZEDs)
        0xXXXX - the short address of the parent device

    The ZDO Info line will display the following information:
      [Logical Device]
        Shows the build type of this Zigbee device, which is determined at compile-time
        (Coordinator, Router, End Device)
      [State]
        The current ZDO state, as reported by the Stack task.

    The BDB Info line will display the following information:
      Current commissioning method being executed:
        TL: Touchlink
        NS: Network Steering
        NF: Network Formation
        FB: Finding And Binding
        PL: Parent Lost (for end devices only)
        -- - idle (commissioning not currently active)
      Network status
        NotOnNwk - not currently connected to a network
        FORM - network was formed by the current device during the latest execution of the NF method
        JOIN - the current device joined an existing network during the latest execution of the NS method
        TCHL - the current device joined a network using touchlink
      Joining permission state (not showing on end devices):
        CLOSED - the current device is closed for joining of other devices
        OpenXXX - the current device is open for joining of other devices, and will close in XXX seconds
      IdXXX - The device is identifying (if XXX > 0), and will stop identifying in XXX seconds.
      SrchXXX/YY - The device is currently performing F&B as an Initiator (if XXX > 0), and will stop in
        XXX seconds. YY is the number of matching endpoints that were found, for which bindings were
        successfully created or already existed.

    The Bind Info line will display the following information:
      [Last Bind]
        This shows the Address, Cluster ID, and Endpoint of the last bind created on the local device.

    The GP Info line will display the following information:
      [GPP Commissioning] (only applicable to routing devices)
        This shows whether or not Green Power Proxy Commissioning mode is currently active on this device.
      [GPS Commissioning] (only applicable to Green Power Sink devices)
        This shows whether or not Green Power Sink Commissioning mode is currently active on this device.

    The App Info line will vary based on each individual sample application.+

*********************************************************************/

#ifndef CUI_DISABLE


#if (BDB_INSTALL_CODE_USE!=BDB_INSTALL_CODE_USE_IC_CRC)
#warning Install-Code functionality is disabled. To enable, make sure BDB_INSTALL_CODE_USE==BDB_INSTALL_CODE_USE_IC_CRC
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ti_zstack_config.h"
#include "rom_jt_154.h"

#include "bdb_interface.h"
#include "zcl.h"
#include "zd_app.h"
#include "zglobals.h"
#include "zcl_sampleapps_ui.h"

#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/apps/Button.h>
#include <ti/drivers/apps/LED.h>
#include "util_timer.h"
#include "ti_drivers_config.h"
#include "cui.h"
#include <stdio.h>
#include <string.h>
#include "mac_user_config.h"

#ifdef BDB_TL_TARGET
  #include "bdb_touchlink_target.h"
  #include "touchlink_target_app.h"
#endif

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
#include "gp_common.h"
#endif

#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
#include "ota_client.h"
#endif


/*********************************************************************
 * CONSTANT-MACROS AND ENUMERATIONS
 */
enum {
    BDB_MODE_NETWORK_FORMATION,
    BDB_MODE_NETWORK_STEERING,
    BDB_MODE_FINDING_AND_BINDING,
    BDB_MODE_TOUCHLINK_INITIATOR,
    BDB_TOUCHLINK_STEALING,
};


#if ZG_BUILD_COORDINATOR_TYPE
//By default, Coordiantor has Formation selected in the UI menu
#define DEFAULT_COMISSIONING_MODE (BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_NWK_FORMATION | BDB_COMMISSIONING_MODE_FINDING_BINDING)
#else
//By default, joining devices such as Router and ZED do not have formation selected. It can be enabled in the UI if needed.
#define DEFAULT_COMISSIONING_MODE (BDB_COMMISSIONING_MODE_NWK_STEERING | BDB_COMMISSIONING_MODE_FINDING_BINDING)
#endif

//UI display defines
#define UI_AUTO_REFRESH_INTERVAL_INFO_LINE 1000
#define UI_DISPLAY_LINE_LENGTH 16


//Install code defines
#define UI_INSTALL_CODE_DEFAULT {0x83,0xFE,0xD3,0x40,0x7A,0x93,0x97,0x23,0xA5,0xC6,0x39,0xB2,0x69,0x16,0xD5,0x05,0xC3,0xB5} //This install code produces the key: 66B6900981E1EE3CA4206B6B861C02BB
#define UI_INSTALL_CODE_ADDR_DEFAULT {0xE7,0xFC,0x0E,0x04,0x00,0x4B,0x12,0x00}
#define UI_INSTALL_CODE_STATUS_UNKNOWN 0xFF

// Max number of status lines
#define SAMPLEAPP_UI_MAX_STATUS_LINES 9

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
* LOCAL FUNCTIONS DECLARATIONS
*/
static void uiUintToString(uint32_t value, char * str, uint8_t base, uint8_t num_of_digists, bool pad0, bool reverse);
static void uiArrayToString(uint8_t * buf, char * str, uint8_t num_of_digists, bool big_endian);
static void uiCreateChannelMaskString(uint32_t channel_mask, char * str);

static void generateBdbInfoString(uint8_t _mode, uint8_t _status, uint8_t _remainingModes, uint8_t* _pPermitJoinDuration, char _lineFormat[64]);
static void uiActionResetToFactoryNew(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void uiActionStartComissioning(const int32_t _itemEntry);
static void uiActionProcessConfigurePriChannels(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void uiActionProcessConfigureSecChannels(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void uiActionProcessConfigureChannels(const char _input, char* _pLines[3], CUI_cursorInfo_t * _pCurInfo, uint32_t* _mask);
static void uiActionProcessConfigurePanId(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void uiActionProcessSetInstallCode(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#if (ZG_BUILD_COORDINATOR_TYPE)
static void uiActionProcessSetInstallCodeAddress(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
static void uiActionProcessApplyInstallCode(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#if ZG_BUILD_RTR_TYPE
static void uiActionProcessConfigureNwkFormation(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
static void uiActionProcessConfigureNwkSteering(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void uiActionProcessConfigureFindBind(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#ifdef BDB_TL_INITIATOR
static void uiActionProcessConfigureTLInitiator(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
#ifdef BDB_TL_TARGET
static void uiActionProcessConfigureTLTargetTime(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void uiActionProcessConfigureTLStealTarget(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
#endif
static bool uiActionProcessConfigureBdbModes(const char _input, uint8_t _mode);

static void zclSampleApps_initializeClocks(void);
static void ui_ProcessUARTEvent(void);
static CUI_clientHandle_t UI_InitCUI(CONST char* pAppStr);

static void uiRaiseBdbNwkLineUpdateEvt(UArg a0);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static void zclSampleAppsUI_ProcessGPPUpdateTimeoutCallback(UArg a0);
#endif

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
static void zclSampleApps_blockModeTestClockHandler(UArg arg);
static void zclSampleApps_blockModeTestOn(int32_t menuEntryIndex);
static void zclSampleApps_blockModeTestOff(int32_t menuEntryIndex);
static void zclSampleApps_setBlockModeOnPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void zclSampleApps_setBlockModeOffPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static void zclSampleApps_setBlockModePeriodUiAction(uint16_t* blockModePeriod, const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo);
static uint8_t moveCursorLeft(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space);
static uint8_t moveCursorRight(uint8_t col, uint8_t left_boundary, uint8_t right_boundary, uint8_t skip_space);
#endif

/*********************************************************************
 * CONSTANTS
 */


#if (ZG_BUILD_COORDINATOR_TYPE)
#define INSTALL_CODE_SUBMENU     3
#else
#define INSTALL_CODE_SUBMENU     2
#endif
/*
 * Menu Lists Initializations
 */
// Menu: zclMenuConfigCodes
// Num Items: INSTALL_CODE_SUBMENU
// Upper: zclMenuConfig
CUI_SUB_MENU(zclMenuConfigCodes, "< ADD INSTL CODE >", INSTALL_CODE_SUBMENU, zclMenuConfig)
    CUI_MENU_ITEM_INT_ACTION("< SET INSTL CODE >", (CUI_pFnIntercept_t) uiActionProcessSetInstallCode)
#if (ZG_BUILD_COORDINATOR_TYPE)
    CUI_MENU_ITEM_INT_ACTION("< SET I.C. ADDR  >", (CUI_pFnIntercept_t) uiActionProcessSetInstallCodeAddress)
#endif
    CUI_MENU_ITEM_INT_ACTION("< APLY INSTL CODE >", (CUI_pFnIntercept_t) uiActionProcessApplyInstallCode)
CUI_SUB_MENU_END



#if defined (BDB_TL_TARGET)
    #define TL_TARGET_MENUS         2
#else
    #define TL_TARGET_MENUS         0
#endif

#if defined (BDB_TL_INITIATOR)
    #define TL_INITIATOR_MENUS      1
#else
    #define TL_INITIATOR_MENUS      0
#endif

#if ZG_BUILD_RTR_TYPE
    #define NWK_FORMATION_MENUS     1
#else
    #define NWK_FORMATION_MENUS     0
#endif

#define CONFIG_COMMISSIONING_MENU (6 + TL_TARGET_MENUS + TL_INITIATOR_MENUS + NWK_FORMATION_MENUS)
// Menu: zclMenuConfig
// Num Items: CONFIG_COMMISSIONING_MENU
// Upper: zclMenuMain
CUI_SUB_MENU(zclMenuConfig, "<     CONFIG     >", CONFIG_COMMISSIONING_MENU, zclMenuMain)
    CUI_MENU_ITEM_SUBMENU(zclMenuConfigCodes)
#ifdef BDB_TL_TARGET
    CUI_MENU_ITEM_INT_ACTION("<T.L. TRGT TIME>", (CUI_pFnIntercept_t) uiActionProcessConfigureTLTargetTime)
    CUI_MENU_ITEM_INT_ACTION("<T.L. STEALING >", (CUI_pFnIntercept_t) uiActionProcessConfigureTLStealTarget)
#else
#ifdef BDB_TL_INITIATOR
    CUI_MENU_ITEM_INT_ACTION("<T.L. INITIATOR>", (CUI_pFnIntercept_t) uiActionProcessConfigureTLInitiator)
#endif
#endif
#if ZG_BUILD_RTR_TYPE
    CUI_MENU_ITEM_INT_ACTION("< NWK FORMATION  >", (CUI_pFnIntercept_t) uiActionProcessConfigureNwkFormation)
#endif
    CUI_MENU_ITEM_INT_ACTION("<  NWK STEERING  >", (CUI_pFnIntercept_t) uiActionProcessConfigureNwkSteering)
    CUI_MENU_ITEM_INT_ACTION("< FINDING+BINDNG >", (CUI_pFnIntercept_t) uiActionProcessConfigureFindBind)
    CUI_MENU_ITEM_INT_ACTION("< PRI CHANL MASK >", (CUI_pFnIntercept_t) uiActionProcessConfigurePriChannels)
    CUI_MENU_ITEM_INT_ACTION("< SEC CHANL MASK >", (CUI_pFnIntercept_t) uiActionProcessConfigureSecChannels)
    CUI_MENU_ITEM_INT_ACTION("<     PAN ID     >", (CUI_pFnIntercept_t) uiActionProcessConfigurePanId)
CUI_SUB_MENU_END

// Menu: zclMenuApp
// Num Items: SAMPLE_APP_MENUS
// Upper: zclMenuMain
CUI_SUB_MENU(zclMenuApp, "<    APP MENU    >", SAMPLE_APP_MENUS, zclMenuMain)
    CUI_APP_MENU
CUI_SUB_MENU_END

// Menu: zclMenuBlockModeTest
// Num Items: 4
// Upper: zclMenuMain
#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
CUI_SUB_MENU(zclMenuBlockModeTest,"< BLOCK MODE TEST>", 4, zclMenuMain)
    CUI_MENU_ITEM_ACTION(         "<   BM TEST ON   >", zclSampleApps_blockModeTestOn)
    CUI_MENU_ITEM_ACTION(         "<   BM TEST OFF  >", zclSampleApps_blockModeTestOff)
    CUI_MENU_ITEM_INT_ACTION(     "<   ON  VAL (ms) >", zclSampleApps_setBlockModeOnPeriodAction)
    CUI_MENU_ITEM_INT_ACTION(     "<   OFF VAL (ms) >", zclSampleApps_setBlockModeOffPeriodAction)
CUI_SUB_MENU_END
#endif

// Menu: zclMenuMain
// Num Items: 4
// Process Fn: ui_ProcessUARTEvent
#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
CUI_MAIN_MENU(zclMenuMain, APP_TITLE_STR, 5, (CUI_pFnClientMenuUpdate_t) ui_ProcessUARTEvent)
#else
CUI_MAIN_MENU(zclMenuMain, APP_TITLE_STR, 4, (CUI_pFnClientMenuUpdate_t) ui_ProcessUARTEvent)
#endif
    CUI_MENU_ITEM_SUBMENU(zclMenuConfig)
    CUI_MENU_ITEM_ACTION("<   COMMISSION   >", (CUI_pFnAction_t) uiActionStartComissioning)
    CUI_MENU_ITEM_SUBMENU(zclMenuApp)
#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
    CUI_MENU_ITEM_SUBMENU(zclMenuBlockModeTest)
#endif
    CUI_MENU_ITEM_INT_ACTION("<  RESET TO FN   >", (CUI_pFnIntercept_t) uiActionResetToFactoryNew)
CUI_MAIN_MENU_END

static uint32_t gDeviceInfoLine;
static uint32_t gNwkInfoLine;
#if ZG_BUILD_ENDDEVICE_TYPE
static uint32_t gNwkInfoLine2;
#endif
static uint32_t gZdoInfoLine;
static uint32_t gBdbInfoLine;
static uint32_t gBindInfoLine;
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static uint32_t gGpInfoLine;
#endif

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
static uint32_t gBlockModeTestInfoLine;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * EXTERNAL REFERENCES
 */
extern uint8_t aExtendedAddress[];

/*********************************************************************
 * LOCAL VARIABLES
 */
#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static uint16_t gppCommissioningTimeout = 0;
#endif
static uiAppFNResetCB_t uiAppFNResetCB = NULL;

static CUI_clientHandle_t gCuiHandle;
static LED_Handle gGreenLedHandle;
static Button_Handle gRightButtonHandle;
static Button_Handle gLeftButtonHandle;

static uint8_t FBMatchesFound = 0;
static bdbBindNotificationData_t  lastBindNotification;

static bool uiCommissioningIsInitializing = FALSE;

static uint16_t *gpBdbCommisioningModes;
static uiAppProcessKeyCB_t gpAppKeyCB;

static char * uiCommissioningStateStr = "--";
static char * uiCommissioningNetworkConnectionStr = NULL;

CONST  uint8_t defaultuiInstallCode[] = UI_INSTALL_CODE_DEFAULT;
static uint8_t uiInstallCode[] = UI_INSTALL_CODE_DEFAULT;
#if (ZG_BUILD_COORDINATOR_TYPE)
CONST  uint8_t defaultuiInstallCodeAddr[] = UI_INSTALL_CODE_ADDR_DEFAULT;
static uint8_t uiInstallCodeAddr[Z_EXTADDR_LEN] = UI_INSTALL_CODE_ADDR_DEFAULT;
#endif

static ClockP_Struct uiBdbNwkLineUpdateClk;
static Semaphore_Handle uiAppSem;
static uint8_t  uiAppEntity;

static uint16_t * pUiIdentifyTimeAttribute;

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
static ClockP_Struct uiGppClkStruct;
static ClockP_Handle uiGppClkHandle;
#endif

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
static ClockP_Struct clkBlockModeTestStruct;
static ClockP_Handle clkBlockModeTestHandle;
#endif

static uint16_t events = 0;
static Button_Handle  keys;

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
DMMPolicy_StackRole DMMPolicy_StackRole_ZigbeeDevice =
#if ZG_BUILD_ENDDEVICE_TYPE
    DMMPolicy_StackRole_ZigbeeEndDevice;
#elif ZG_BUILD_RTRONLY_TYPE
    DMMPolicy_StackRole_ZigbeeRouter;
#elif ZG_BUILD_COORDINATOR_TYPE
    DMMPolicy_StackRole_ZigbeeCoordinator;
#endif

static uint16_t ZCL_BLOCK_MODE_ON_PERIOD  = 0x01F4;    // Default 500  ms
static uint16_t ZCL_BLOCK_MODE_OFF_PERIOD = 0x03E8;    // Default 1000 ms
#endif


/*********************************************************************
 * LOCAL UTILITY FUNCTIONS
 */

/*********************************************************************
 * @fn          uiUintToString
 *
 * @brief       format an integer into a string buffer.
 *
 * @param       value - 32bit unsigned int value to be formatted
 *              str - pointer to a buffer to store the formatted number
 *              base - base representation of the value. currently only tested base 10 and 16
 *              num_of_digists - number of digits to include in the formatted string
 *              pad0 - should be set to TRUE to pad the number with leading 0's as required
 *              reverse - should be set to TRUE to reverse the output string
 *
 * @return      none
 */
static void uiUintToString (uint32_t value, char * str, uint8_t base, uint8_t num_of_digists, bool pad0, bool reverse)
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

/*********************************************************************
 * @fn          uiArrayToString
 *
 * @brief       format a memory buffer into a string buffer in hex representation.
 *
 * @param       buf - pointer to a buffer to be formatted
 *              str - pointer to a buffer to store the formatted string
 *              num_of_digists - number of digits to include in the formatted string
 *              big_endian - whether the memory content should be represented as big or little endian
 *
 * @return      none
 */
static void uiArrayToString (uint8_t * buf, char * str, uint8_t num_of_digists, bool big_endian)
{
  int i;
  uint8_t stringIndex;
  uint8_t value;

  for (i = 0; i < num_of_digists; i++)
  {
    stringIndex = (big_endian ? i : num_of_digists - 1 - i);
    if(big_endian)
    {
      value = (buf[i / 2] >> (4 * (!(i % 2)))) & 0x0F;
    }
    else
    {
      value = (buf[i / 2] >> (4 * (i % 2))) & 0x0F;
    }
    str[stringIndex] = '0' + value;
    if (str[stringIndex] > '9')
    {
      str[stringIndex] += 'A' - '0' - 10;
    }
  }
}

/*********************************************************************
 * @fn          uiCreateChannelMaskString
 *
 * @brief       format a channel mask into a string bugger
 *
 * @param       channel_mask - bitmask of the enabled / disabled channels (bits 11-26 represent the
 *                respective channels)
 *              str - pointer to a buffer to store the formatted string
 *
 * @return      none
 */
static void uiCreateChannelMaskString(uint32_t channel_mask, char * str)
{
  int i;
  uint32_t mask = 0x00000800;

  for (i = 0; i < 16; i++)
  {
    str[i] = channel_mask & mask ? '*' : '-';
    mask <<= 1;
  }

  str[16] = 0;
}


/*********************************************************************
 * LOCAL FUNCTIONS
 */

/*********************************************************************
 * @fn          uiActionStartComissioning
 *
 * @brief       action function to trigger commissioning
 *
 * @return      none
 */
static void uiActionStartComissioning(const int32_t _itemEntry)
{
  // reset network connection string before commissioning again
  uiCommissioningNetworkConnectionStr = NULL;

  if (*gpBdbCommisioningModes != 0)
  {
    zstack_bdbStartCommissioningReq_t zstack_bdbStartCommissioningReq;

    FBMatchesFound = 0;

    uiCommissioningIsInitializing = TRUE;

    zstack_bdbStartCommissioningReq.commissioning_mode = *gpBdbCommisioningModes;

    Zstackapi_bdbStartCommissioningReq(uiAppEntity,&zstack_bdbStartCommissioningReq);
  }
}



/*********************************************************************
 * @fn          uiActionProcessConfigurePriChannels
 *
 * @brief       State-machine action for configuring channel masks
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigurePriChannels(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint32_t channelMask = BDB_DEFAULT_PRIMARY_CHANNEL_SET;

    if (_input == CUI_ITEM_INTERCEPT_START) {
        zstack_bdbGetAttributesRsp_t rsp;
        Zstackapi_bdbGetAttributesReq(uiAppEntity, &rsp);

        channelMask = rsp.bdbPrimaryChannelSet;
    }

    uiActionProcessConfigureChannels(_input, _pLines, _pCurInfo, &channelMask);

    if (_input == CUI_ITEM_INTERCEPT_STOP) {
        zstack_bdbSetAttributesReq_t req = {0};
        req.bdbPrimaryChannelSet = channelMask;
        req.has_bdbPrimaryChannelSet = true;
        Zstackapi_bdbSetAttributesReq(uiAppEntity, &req);
    }

    if (_input != CUI_ITEM_PREVIEW)
        strcpy(_pLines[2], " PRI CHANL MASK");
}

/*********************************************************************
 * @fn          uiActionProcessConfigureSecChannels
 *
 * @brief       State-machine action for configuring channel masks
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureSecChannels(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint32_t channelMask = SECONDARY_CHANLIST;

    if (_input == CUI_ITEM_INTERCEPT_START) {
        zstack_bdbGetAttributesRsp_t rsp;
        Zstackapi_bdbGetAttributesReq(uiAppEntity, &rsp);

        channelMask = rsp.bdbSecondaryChannelSet;
    }

    uiActionProcessConfigureChannels(_input, _pLines, _pCurInfo, &channelMask);

    if (_input == CUI_ITEM_INTERCEPT_STOP) {
        zstack_bdbSetAttributesReq_t req = {0};
        req.bdbSecondaryChannelSet = channelMask;
        req.has_bdbSecondaryChannelSet = true;
        Zstackapi_bdbSetAttributesReq(uiAppEntity, &req);
    }

    if (_input != CUI_ITEM_PREVIEW)
        strcpy(_pLines[2], " SEC CHANL MASK");
}

/*********************************************************************
 * @fn          uiActionProcessConfigureChannels
 *
 * @return      none
 */
static void uiActionProcessConfigureChannels(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo, uint32_t* _mask)
{
  static CUI_cursorInfo_t cursor = {0, 0};

  switch (_input) {
      case CUI_ITEM_INTERCEPT_START:
      case CUI_ITEM_INTERCEPT_STOP:
          // Reset the local cursor info
          cursor.col = 0;
          break;
      // Show the value of this screen w/o making changes
      case CUI_ITEM_PREVIEW:
          break;
      // Move the cursor to the left
      case CUI_INPUT_LEFT:
          cursor.col = (cursor.col - 1 + 16) % 16;
          break;
      // Move the cursor to the right
      case CUI_INPUT_RIGHT:
          cursor.col = (cursor.col + 1 + 16) % 16;
          break;
      // Set the bit
      case CUI_INPUT_UP:
          *_mask |= ((uint32_t)0x00000800 << cursor.col);
          break;
      // Clear the bit
      case CUI_INPUT_DOWN:
          *_mask &= ~(((uint32_t)0x00000800 << cursor.col));
          break;
      default:
          break;
  }


  strcpy(_pLines[0], "1234567890123456");
  uiCreateChannelMaskString(*_mask, _pLines[1]);

  if (_input != CUI_ITEM_PREVIEW) {
    _pCurInfo->col = cursor.col + 1; // Terminal isn't zero indexed
    _pCurInfo->row = 2;
  }
}

/*********************************************************************
 * @fn          uiActionProcessConfigurePanId
 *
 * @brief       State-machine action for configuring the PAN ID
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigurePanId(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  static CUI_cursorInfo_t cursor = {0, 6};
  static uint16_t panId = ZDAPP_CONFIG_PAN_ID;
  const char tmpInput[2] = {_input, '\0'};

  switch (_input) {
      case CUI_ITEM_INTERCEPT_START:
      {
          zstack_sysConfigReadReq_t req = {0};
          zstack_sysConfigReadRsp_t rsp;
          req.panID = true;
          Zstackapi_sysConfigReadReq(uiAppEntity, &req, &rsp);

          panId = rsp.panID;
          break;
      }
      // Submit the final modified value
      case CUI_ITEM_INTERCEPT_STOP:
      {
          zstack_sysConfigWriteReq_t writeReq = {0};
          // Update the config PAN ID, defined in znwk_config.h
          writeReq.has_panID = true;
          writeReq.panID = panId;
          Zstackapi_sysConfigWriteReq(uiAppEntity, &writeReq);

          // Reset the local cursor info
          cursor.col = 6;
          break;
      }
      // Show the value of this screen w/o making changes
      case CUI_ITEM_PREVIEW:
          break;
      // Move the cursor to the left
      case CUI_INPUT_LEFT:
          if (6 != cursor.col)
              cursor.col--;
          break;

      // Move the cursor to the right
      case CUI_INPUT_RIGHT:
          if (9 != cursor.col)
              cursor.col++;
          break;

      // Increment the digit
      case CUI_INPUT_UP:
      {
          uint8_t shift = 4 * (9 - cursor.col);
          uint8_t digit = (panId >> shift) & 0xF;

          digit = (digit + 1 + 16) % 16;

          panId &= ~((uint32_t)0xF << shift);
          panId |= (uint32_t)digit << shift;
          break;
      }

      // Decrement the digit
      case CUI_INPUT_DOWN:
      {
          uint8_t shift = 4 * (9 - cursor.col);
          uint8_t digit = (panId >> shift) & 0xF;

          digit = (digit - 1 + 16) % 16;

          panId &= ~((uint32_t)0xF << shift);
          panId |= (uint32_t)digit << shift;
          break;
      }
      case CUI_INPUT_EXECUTE:
          break;
      default:
      {
        // is input valid?
        if(CUI_IS_INPUT_HEX(_input))
        {
          uint8_t shift = 4 * (9 - cursor.col);
          uint8_t digit = strtol(tmpInput, NULL, 16);

          panId &= ~((uint32_t)0xF << shift);
          panId |= (uint32_t)digit << shift;

          if (9 != cursor.col)
            cursor.col++;
        }
        break;
      }
  }


  if (panId == 0xFFFF)
  {
    strcpy(_pLines[0], "    0xFFFF (any)");
  }
  else
  {
    char tmp[4];
    uiUintToString( panId, tmp, 16, 4, TRUE, FALSE);
    strcpy(_pLines[0], "    0x");
    strncat(_pLines[0], tmp, 4);
  }



  if (_input != CUI_ITEM_PREVIEW) {
      strcpy(_pLines[2], "     PAN ID");
      _pCurInfo->row = 1;
      _pCurInfo->col = cursor.col+1;
  }
}


#if (ZG_BUILD_COORDINATOR_TYPE)
/*********************************************************************
 * @fn          uiActionProcessSetInstallCodeAddress
 *
 * @brief       State-machine action for editing the install-code and the install-code address
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessSetInstallCodeAddress(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint8_t * pCurrentModifiedField;
    static uint8_t CurrentModifiedFieldLen;
    static bool CurrentModifiedFieldIsBigEndian;
    static CUI_cursorInfo_t cursor;
    static uint8_t index;
    const char tmpInput[2] = {_input, '\0'};

    uint8_t shift;
    uint8_t digit;
    uint8_t bufIndex;

    switch(_input)
    {
        case CUI_ITEM_INTERCEPT_START:
            OsalPort_memcpy(uiInstallCodeAddr, defaultuiInstallCodeAddr, Z_EXTADDR_LEN );
            cursor.col = 0;
            _pCurInfo->row = 1;
            _pCurInfo->col = cursor.col+1;

            pCurrentModifiedField = uiInstallCodeAddr;
            CurrentModifiedFieldLen = sizeof(uiInstallCodeAddr);
            CurrentModifiedFieldIsBigEndian = FALSE;

            index = 0;
        break;

        case CUI_ITEM_INTERCEPT_STOP:
        break;

        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
              _pCurInfo->col = cursor.col + 1;
              _pCurInfo->row = cursor.row + 1;
        break;

        case CUI_INPUT_UP:
        case CUI_INPUT_DOWN:
            shift = 4 * ((CurrentModifiedFieldIsBigEndian ? index : (index + 1)) % 2);
            bufIndex = (CurrentModifiedFieldIsBigEndian ? (index / 2) : (CurrentModifiedFieldLen - 1 - (index / 2)));
            digit = (pCurrentModifiedField[bufIndex] >> shift) & 0xF;

            if (_input == CUI_INPUT_UP)
            {
              if (digit < 15)
              {
                digit++;
              }
              else
              {
                digit = 0;
              }
            }
            else if (_input == CUI_INPUT_DOWN)
            {
              if (digit > 0)
              {
                digit--;
              }
              else
              {
                digit = 15;
              }
            }

            pCurrentModifiedField[bufIndex] &= ~((uint32_t)0xF << shift);
            pCurrentModifiedField[bufIndex] |= (uint32_t)digit << shift;

        break;
        case CUI_INPUT_RIGHT:
            if (index < CurrentModifiedFieldLen * 2 - 1)
            {
              index++;
            }
            else
            {
              index = 0;
            }
        break;
        case CUI_INPUT_LEFT:
            if (index > 0)
            {
              index --;
            }
            else
            {
              index = (CurrentModifiedFieldLen * 2 - 1);
            }
        break;
        default:
        {
          // is input valid?
          if(CUI_IS_INPUT_HEX(_input))
          {
            shift = 4 * ((CurrentModifiedFieldIsBigEndian ? index : (index + 1)) % 2);
            bufIndex = (CurrentModifiedFieldIsBigEndian ? (index / 2) : (CurrentModifiedFieldLen - 1 - (index / 2)));
            digit = strtol(tmpInput, NULL, 16);

            pCurrentModifiedField[bufIndex] &= ~((uint32_t)0xF << shift);
            pCurrentModifiedField[bufIndex] |= (uint32_t)digit << shift;

            if (index < CurrentModifiedFieldLen * 2 - 1)
            {
              index++;
            }
            else
            {
              index = 0;
            }
          }
          break;
        }
    }


    cursor.row = index / UI_DISPLAY_LINE_LENGTH;
    cursor.col = index % UI_DISPLAY_LINE_LENGTH;


    // Terminal isn't zero indexed
    _pCurInfo->row = cursor.row+1;
    _pCurInfo->col = cursor.col+1;


    // 17 bytes one for null character
    char tmp[17];
    memset(tmp, '\0', sizeof(tmp));
    strncpy(_pLines[1], tmp, sizeof(tmp));  //Empty line
    uiArrayToString(uiInstallCodeAddr, tmp, 16, FALSE);
    strncpy(_pLines[0], tmp, sizeof(tmp));
    strncpy(_pLines[2], "       I.C. ADDR", sizeof(tmp));

}
#endif

/*********************************************************************
 * @fn          uiActionProcessSetInstallCode
 *
 * @brief       State-machine action for editing the install-code and the install-code address
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessSetInstallCode(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  static uint8_t * pCurrentModifiedField;
  static uint8_t CurrentModifiedFieldLen;
  static bool CurrentModifiedFieldIsBigEndian;
  static CUI_cursorInfo_t cursor;
  static uint8_t index;
  const char tmpInput[2] = {_input, '\0'};

  uint8_t shift;
  uint8_t digit;
  uint8_t bufIndex;

  switch(_input)
  {
      case CUI_ITEM_INTERCEPT_START:
          OsalPort_memcpy(uiInstallCode, defaultuiInstallCode, (INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN) );
          cursor.col = 0;
          _pCurInfo->row = 1;
          _pCurInfo->col = cursor.col+1;

          pCurrentModifiedField = uiInstallCode;
          CurrentModifiedFieldLen = INSTALL_CODE_LEN;
          CurrentModifiedFieldIsBigEndian = TRUE;

          index = 0;
      break;

      case CUI_ITEM_INTERCEPT_STOP:
      break;

      // Show the value of this screen w/o making changes
      case CUI_ITEM_PREVIEW:
            _pCurInfo->col = cursor.col + 1;
            _pCurInfo->row = cursor.row + 1;
      break;

      case CUI_INPUT_UP:
      case CUI_INPUT_DOWN:
          shift = 4 * ((CurrentModifiedFieldIsBigEndian ? (index + 1) : index ) % 2);
          bufIndex = (CurrentModifiedFieldIsBigEndian ? (index / 2) : (CurrentModifiedFieldLen - 1 - (index / 2)));
          digit = (pCurrentModifiedField[bufIndex] >> shift) & 0xF;

          if (_input == CUI_INPUT_UP)
          {
            if (digit < 15)
            {
              digit++;
            }
            else
            {
              digit = 0;
            }
          }
          else if (_input == CUI_INPUT_DOWN)
          {
            if (digit > 0)
            {
              digit--;
            }
            else
            {
              digit = 15;
            }
          }

          pCurrentModifiedField[bufIndex] &= ~((uint32_t)0xF << shift);
          pCurrentModifiedField[bufIndex] |= (uint32_t)digit << shift;

      break;
      case CUI_INPUT_RIGHT:
          if (index < CurrentModifiedFieldLen * 2 - 1)
          {
            index++;
          }
          else
          {
            index = 0;
          }
      break;
      case CUI_INPUT_LEFT:
          if (index > 0)
          {
            index --;
          }
          else
          {
            index = (CurrentModifiedFieldLen * 2 - 1);
          }
      break;
      default:
      {
        // is input valid?
        if(CUI_IS_INPUT_HEX(_input))
        {
          shift = 4 * ((CurrentModifiedFieldIsBigEndian ? (index + 1) : index ) % 2);
          bufIndex = (CurrentModifiedFieldIsBigEndian ? (index / 2) : (CurrentModifiedFieldLen - 1 - (index / 2)));
          digit = strtol(tmpInput, NULL, 16);

          pCurrentModifiedField[bufIndex] &= ~((uint32_t)0xF << shift);
          pCurrentModifiedField[bufIndex] |= (uint32_t)digit << shift;

          if (index < CurrentModifiedFieldLen * 2 - 1)
          {
            index++;
          }
          else
          {
            index = 0;
          }
        }
        break;
      }
  }


  cursor.row = index / UI_DISPLAY_LINE_LENGTH;
  cursor.col = index % UI_DISPLAY_LINE_LENGTH;


  // Terminal isn't zero indexed
  _pCurInfo->row = cursor.row+1;
  _pCurInfo->col = cursor.col+1;


  zstack_bdbGenerateInstallCodeCRCReq_t zstack_bdbGenerateInstallCodeCRCReq;
  zstack_bdbGenerateInstallCodeCRCRsp_t zstack_bdbGenerateInstallCodeCRCRsp;

  OsalPort_memcpy(zstack_bdbGenerateInstallCodeCRCReq.installCode,uiInstallCode,INSTALL_CODE_LEN);

  Zstackapi_bdbGenerateInstallCodeCRCReq(uiAppEntity,&zstack_bdbGenerateInstallCodeCRCReq,
                                               &zstack_bdbGenerateInstallCodeCRCRsp);

  uiInstallCode[INSTALL_CODE_LEN] = zstack_bdbGenerateInstallCodeCRCRsp.CRC  & 0xFF;
  uiInstallCode[INSTALL_CODE_LEN + 1] = zstack_bdbGenerateInstallCodeCRCRsp.CRC  >> 8;

  // one additional byte for null terminator after install code is stored
  char tmp[17];
  memset(tmp, '\0', sizeof(tmp));
  uiArrayToString(uiInstallCode, tmp, 16, TRUE);
  strncpy(_pLines[0], tmp, sizeof(tmp));
  uiArrayToString(uiInstallCode + 8, tmp, 16, TRUE);
  strncpy(_pLines[1], tmp, sizeof(tmp));
  uiArrayToString( uiInstallCode + 16, tmp, 4, TRUE);
  strncpy(_pLines[2], tmp, 4);
  _pLines[2][4] = '\0';
  strncat(_pLines[2], "   INST CODE", MAX_MENU_LINE_LEN - strlen(_pLines[2]));

}

/*********************************************************************
 * @fn          uiActionProcessApplyInstallCode
 *
 * @brief       State-machine action for setting the install code
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessApplyInstallCode(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{

  static ZStatus_t uiApplyInstallCodeLastStatus = UI_INSTALL_CODE_STATUS_UNKNOWN;

  if (_input != CUI_ITEM_PREVIEW)
  {
      zstack_bdbGenerateInstallCodeCRCReq_t zstack_bdbGenerateInstallCodeCRCReq;
      zstack_bdbGenerateInstallCodeCRCRsp_t zstack_bdbGenerateInstallCodeCRCRsp;

      OsalPort_memcpy(zstack_bdbGenerateInstallCodeCRCReq.installCode,uiInstallCode,INSTALL_CODE_LEN);
      Zstackapi_bdbGenerateInstallCodeCRCReq(uiAppEntity,&zstack_bdbGenerateInstallCodeCRCReq,
                                                          &zstack_bdbGenerateInstallCodeCRCRsp);

      uiInstallCode[INSTALL_CODE_LEN] = zstack_bdbGenerateInstallCodeCRCRsp.CRC & 0xFF;
      uiInstallCode[INSTALL_CODE_LEN + 1] = zstack_bdbGenerateInstallCodeCRCRsp.CRC >> 8;

#if (ZG_BUILD_COORDINATOR_TYPE)
      if (ZG_DEVICE_COORDINATOR_TYPE)
      {
        zstack_bdbAddInstallCodeReq_t zstack_bdbAddInstallCodeReq;

        memset(&zstack_bdbAddInstallCodeReq,0,sizeof(zstack_bdbAddInstallCodeReq_t));
        OsalPort_memcpy(zstack_bdbAddInstallCodeReq.pExt, uiInstallCodeAddr, Z_EXTADDR_LEN);
        OsalPort_memcpy(zstack_bdbAddInstallCodeReq.pInstallCode, uiInstallCode, INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN);

        uiApplyInstallCodeLastStatus = Zstackapi_bdbAddInstallCodeReq(uiAppEntity, &zstack_bdbAddInstallCodeReq);
      }
      else
      {
          zstack_bdbSetActiveCentralizedLinkKeyReq_t zstack_bdbSetActiveCentralizedLinkKeyReq;

          memset(&zstack_bdbSetActiveCentralizedLinkKeyReq,0, sizeof(zstack_bdbSetActiveCentralizedLinkKeyReq_t));

          zstack_bdbSetActiveCentralizedLinkKeyReq.zstack_CentralizedLinkKeyModes = zstack_UseInstallCode;

          zstack_bdbSetActiveCentralizedLinkKeyReq.pKey = OsalPort_malloc(INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN);

          OsalPort_memcpy(zstack_bdbSetActiveCentralizedLinkKeyReq.pKey,uiInstallCode, INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN);

          uiApplyInstallCodeLastStatus = Zstackapi_bdbSetActiveCentralizedLinkKeyReq(uiAppEntity, &zstack_bdbSetActiveCentralizedLinkKeyReq);

          OsalPort_free(zstack_bdbSetActiveCentralizedLinkKeyReq.pKey);

      }
#else
          zstack_bdbSetActiveCentralizedLinkKeyReq_t zstack_bdbSetActiveCentralizedLinkKeyReq;

          zstack_bdbSetActiveCentralizedLinkKeyReq.zstack_CentralizedLinkKeyModes = zstack_UseInstallCode;

          zstack_bdbSetActiveCentralizedLinkKeyReq.pKey = OsalPort_malloc(INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN);

          if ( zstack_bdbSetActiveCentralizedLinkKeyReq.pKey != NULL )
          {
            OsalPort_memcpy(zstack_bdbSetActiveCentralizedLinkKeyReq.pKey,uiInstallCode,INSTALL_CODE_LEN + INSTALL_CODE_CRC_LEN);
            uiApplyInstallCodeLastStatus = Zstackapi_bdbSetActiveCentralizedLinkKeyReq(uiAppEntity, &zstack_bdbSetActiveCentralizedLinkKeyReq);
            OsalPort_free(zstack_bdbSetActiveCentralizedLinkKeyReq.pKey);
          }
#endif
  }


  strncpy(_pLines[0], "Last status: ", MAX_MENU_LINE_LEN);
  switch (uiApplyInstallCodeLastStatus)
  {
    case UI_INSTALL_CODE_STATUS_UNKNOWN:
      strncpy(_pLines[1], "---", MAX_MENU_LINE_LEN);
      break;
    case ZSuccess:
      strncpy(_pLines[1], "SUCCESS", MAX_MENU_LINE_LEN);
      break;
    default:
      System_snprintf(_pLines[1], MAX_MENU_LINE_LEN, "ERROR (0x%02X)    ", uiApplyInstallCodeLastStatus);
      break;
  }
  strncpy(_pLines[2], "  APLY INSTL CODE", MAX_MENU_LINE_LEN);

}

#if ZG_BUILD_RTR_TYPE
/*********************************************************************
 * @fn          uiActionProcessConfigureNwkFormation
 *
 * @brief       Process UART input for nwk formation configuration
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureNwkFormation(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    bool enabled = uiActionProcessConfigureBdbModes(_input, BDB_MODE_NETWORK_FORMATION);

    if (enabled)
        strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    else
        strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);

    if (_input != CUI_ITEM_PREVIEW) {
        strncpy(_pLines[2], " NWK FORMATION", MAX_MENU_LINE_LEN);
    }
}
#endif

/*********************************************************************
 * @fn          uiActionProcessConfigureNwkSteering
 *
 * @brief       Process UART input for nwk steering configuration
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureNwkSteering(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    bool enabled = uiActionProcessConfigureBdbModes(_input, BDB_MODE_NETWORK_STEERING);

    if (enabled)
        strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    else
        strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);

    if (_input != CUI_ITEM_PREVIEW) {
        strncpy(_pLines[2], "  NWK STEERING", MAX_MENU_LINE_LEN);
    }
}

/*********************************************************************
 * @fn          uiActionProcessConfigureFindBind
 *
 * @brief       Process UART input for Finding and Binding configuration
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureFindBind(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    bool enabled = uiActionProcessConfigureBdbModes(_input, BDB_MODE_FINDING_AND_BINDING);

    if (enabled)
        strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    else
        strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);

    if (_input != CUI_ITEM_PREVIEW) {
        strncpy(_pLines[2], " FINDING+BINDING", MAX_MENU_LINE_LEN);
    }
}


#ifdef BDB_TL_TARGET
/*********************************************************************
 * @fn          uiActionProcessConfigureTLTargetTime
 *
 * @brief       Process UART input for Touchlink target configuration
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureTLTargetTime(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    static uint32_t temp_u32 = 15000;

    switch(_input)
    {
        case CUI_ITEM_INTERCEPT_START:
        {
            temp_u32 = 15000;
        }
        break;
        case   CUI_INPUT_UP:
            if(temp_u32 < TOUCHLINK_TARGET_PERPETUAL)
            {
                temp_u32 += 1000;
            }
            else
            {
                temp_u32 = 0;
            }
        break;
        case CUI_INPUT_DOWN:
            if(temp_u32 != 0)
            {
                temp_u32 -= 1000;
            }
            else
            {
                temp_u32 = TOUCHLINK_TARGET_PERPETUAL;
            }
        break;
        case CUI_ITEM_INTERCEPT_STOP:

            Zstackapi_bdbTouchLinkTargetDisableCommissioningReq(uiAppEntity);

            if(temp_u32)
            {
                zstack_bdbTouchLinkTargetEnableCommissioningReq_t zstack_bdbTouchLinkTargetEnableCommissioningReq;
                zstack_bdbTouchLinkTargetEnableCommissioningReq.timeoutTime = temp_u32;

                Zstackapi_bdbTouchLinkTargetEnableCommissioningReq(uiAppEntity, &zstack_bdbTouchLinkTargetEnableCommissioningReq);
            }
        break;
        // Show the value of this screen w/o making changes
        case CUI_ITEM_PREVIEW:
            break;
    }

    if (temp_u32 == TOUCHLINK_TARGET_PERPETUAL)
    {
        strcpy(_pLines[1], "ENABLE FOREVER ");
    }
    else if (temp_u32 == 0)
    {
        strcpy(_pLines[1], "DISABLE(    0s)");
    }
    else
    {
      strcpy(_pLines[1], "ENABLE (     s)");
      uiUintToString( temp_u32 / 1000 + ((temp_u32 % 1000) > 0 ? 1 : 0), _pLines[1] + 9, 10, 5, FALSE, FALSE); //note: timeouts longer than 65535 seconds will not display correctly
    }

    strcpy(_pLines[2], " T.L. TRGT TIME ");
}


/*********************************************************************
 * @fn          uiActionProcessConfigureTLStealTarget
 *
 * @brief       Process UART input for Touchlink target stealing configuration
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureTLStealTarget(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    bool enabled = uiActionProcessConfigureBdbModes(_input, BDB_TOUCHLINK_STEALING);

    if (enabled)
        strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    else
        strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);

    if (_input != CUI_ITEM_PREVIEW) {
        strncpy(_pLines[2], " T.L. STEALING ", MAX_MENU_LINE_LEN);
    }
}
#endif

#ifdef BDB_TL_INITIATOR
/*********************************************************************
 * @fn          uiActionProcessConfigureTLInitiator
 *
 * @brief       Process UART input for Touchlink initiator configuration
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionProcessConfigureTLInitiator(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    bool enabled = uiActionProcessConfigureBdbModes(_input, BDB_MODE_TOUCHLINK_INITIATOR);

    if (enabled)
        strncpy(_pLines[1], "ENABLED", MAX_MENU_LINE_LEN);
    else
        strncpy(_pLines[1], "DISABLED", MAX_MENU_LINE_LEN);

    if (_input != CUI_ITEM_PREVIEW) {
        strncpy(_pLines[2], " T.L. INITIATOR ", MAX_MENU_LINE_LEN);
    }
}
#endif

/*********************************************************************
 * @fn          uiActionProcessConfigureBdbModes
 *
 * @brief       State-machine action for editing the install-code and the install-code address
 *
 * @param       _input - uart key or uart notification
 *              _mode  - commissioning mode to be process (enabled/disabled)
 *
 * @return      none
 */
static bool uiActionProcessConfigureBdbModes(const char _input, uint8_t _mode)
{
    bool enabled = false;
    bool modify = false;
    if (_input == CUI_INPUT_UP || _input == CUI_INPUT_DOWN)
        modify = true;
    switch(_mode) {
        case BDB_MODE_NETWORK_FORMATION:
          if (modify)
              *gpBdbCommisioningModes ^= BDB_COMMISSIONING_MODE_NWK_FORMATION;
          if (*gpBdbCommisioningModes & BDB_COMMISSIONING_MODE_NWK_FORMATION)
              enabled = true;
          break;
        case BDB_MODE_NETWORK_STEERING:
          if (modify)
              *gpBdbCommisioningModes ^= BDB_COMMISSIONING_MODE_NWK_STEERING;
          if (*gpBdbCommisioningModes & BDB_COMMISSIONING_MODE_NWK_STEERING)
              enabled = true;
          break;
        case BDB_MODE_FINDING_AND_BINDING:
          if (modify)
              *gpBdbCommisioningModes ^= BDB_COMMISSIONING_MODE_FINDING_BINDING;
          if (*gpBdbCommisioningModes & BDB_COMMISSIONING_MODE_FINDING_BINDING)
              enabled = true;
          break;
#ifdef BDB_TL_INITIATOR
        case BDB_MODE_TOUCHLINK_INITIATOR:
            if (modify)
                *gpBdbCommisioningModes ^= BDB_COMMISSIONING_MODE_INITIATOR_TL;
            if (*gpBdbCommisioningModes & BDB_COMMISSIONING_MODE_INITIATOR_TL)
                enabled = true;
            break;
#endif
#ifdef BDB_TL_TARGET
        case BDB_TOUCHLINK_STEALING:
        {
            zstack_bdbTouchlinkGetAllowStealingRsp_t AllowStealingRsp;
            zstack_bdbTouchlinkSetAllowStealingReq_t AllowStealingReq;
            Zstackapi_bdbTouchlinkGetAllowStealingReq(uiAppEntity,&AllowStealingRsp);

            AllowStealingReq.allowStealing = AllowStealingRsp.allowStealing;

            if(modify)
            {
                AllowStealingReq.allowStealing ^= 1;
                Zstackapi_bdbTouchlinkSetAllowStealingReq(uiAppEntity,&AllowStealingReq);
            }

            if(AllowStealingReq.allowStealing)
            {
                enabled = true;
            }
        }
        break;
#endif
    }
    return enabled;
}


/*********************************************************************
 * @fn          uiActionResetToFactoryNew
 *
 * @brief       State-machine action for resetting the device to factory new
 *
 * @param       _input - uart key or uart notification
 *              _pLines - pointers to the buffers to be displayed
 *              _pCurInfo - pointer to the cursor position. The position is given
 *                          by the length of _pLines and the number of lines.
 *                          If -1,-1 is provided, no pointer is displayed
 *
 * @return      none
 */
static void uiActionResetToFactoryNew(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
    if (CUI_ITEM_INTERCEPT_START == _input) {
        if (uiAppFNResetCB != NULL) {
            uiAppFNResetCB();
        }
        Zstackapi_bdbResetLocalActionReq(uiAppEntity);

        strncpy(_pLines[2], "Resetting, Please wait...", MAX_MENU_LINE_LEN);
    }
}

/*********************************************************************
 * @fn          generateBdbInfoString
 *
 * @brief       Generate part of the BDB info string
 *
 * @param       _mode - Commissioning mode being updated, if any.
 *              _status - Status being reported by the mode, if any.
 *              _remainingModes - Remaining commissioning modes, if any.
 *              _pPermitJoinDuration - Remaining time of permit join.
 *              _lineFormat - Buffer to store the BDB info string.
 *
 * @return      none
 */
static void generateBdbInfoString(uint8_t _mode, uint8_t _status, uint8_t _remainingModes, uint8_t* _pPermitJoinDuration, char _lineFormat[MAX_STATUS_LINE_VALUE_LEN])
{
    zstack_ZStatusValues statusValues;
    zstack_sysNwkInfoReadRsp_t rsp;
    statusValues = Zstackapi_sysNwkInfoReadReq(uiAppEntity, &rsp);
    if (zstack_ZStatusValues_ZSuccess != statusValues)
        return;

    switch(_mode)
    {
      case BDB_COMMISSIONING_FORMATION:
        uiCommissioningStateStr = "NF";
        if ((_status == BDB_COMMISSIONING_SUCCESS) && (uiCommissioningNetworkConnectionStr == NULL))
        {
          uiCommissioningNetworkConnectionStr = " FORM";
        }
        break;
      case BDB_COMMISSIONING_NWK_STEERING:
        uiCommissioningStateStr = "NS";
        if ((_status == BDB_COMMISSIONING_SUCCESS) && (uiCommissioningNetworkConnectionStr == NULL))
        {
          uiCommissioningNetworkConnectionStr = " JOIN";
        }
        break;
      case BDB_COMMISSIONING_FINDING_BINDING:
        uiCommissioningStateStr = "FB";
        break;
      case BDB_COMMISSIONING_INITIALIZATION:
        uiCommissioningStateStr = "IN";
        break;
  #if ZG_BUILD_ENDDEVICE_TYPE
      case BDB_COMMISSIONING_PARENT_LOST:
        if(_status == BDB_COMMISSIONING_NETWORK_RESTORED)
        {
          uiCommissioningStateStr = "--";
        }
        else
        {
          uiCommissioningStateStr = "PL";
        }
        break;
  #endif
  #if ( BDB_TOUCHLINK_CAPABILITY_ENABLED == TRUE )
      case BDB_COMMISSIONING_TOUCHLINK:
        uiCommissioningStateStr = "TL";
        if ((_status == BDB_COMMISSIONING_SUCCESS) && (uiCommissioningNetworkConnectionStr == NULL))
        {
          uiCommissioningNetworkConnectionStr = " TCHL";
        }
        break;
  #endif
    }

    if ( ((_status != BDB_COMMISSIONING_IN_PROGRESS) &&
          (_status != BDB_COMMISSIONING_FB_TARGET_IN_PROGRESS) &&
          (_status != BDB_COMMISSIONING_FB_INITITATOR_IN_PROGRESS))
          && (_remainingModes == 0) )
    {
      uiCommissioningStateStr = "--";
    }

    if (uiCommissioningIsInitializing)
    {
      strcpy(_lineFormat, "Please wait...  ");
    }
    else
    {
      strcpy(_lineFormat, uiCommissioningStateStr);

      if ((rsp.devState != zstack_DevState_DEV_END_DEVICE) && (rsp.devState != zstack_DevState_DEV_ZB_COORD) && (rsp.devState != zstack_DevState_DEV_ROUTER))
      {
        strcat(_lineFormat, " NotOnNwk");
      }
      else
      {
        if (uiCommissioningNetworkConnectionStr != NULL)
        {
          strcat(_lineFormat, uiCommissioningNetworkConnectionStr);
        }
        if ((rsp.devState == zstack_DevState_DEV_ZB_COORD) || (rsp.devState == zstack_DevState_DEV_ROUTER))
        {
          *_pPermitJoinDuration = NLME_GetRemainingPermitJoiningDuration();
          if (*_pPermitJoinDuration)
          {
            System_snprintf(&_lineFormat[strlen(_lineFormat)], 9, " Open%03d", *_pPermitJoinDuration);
          }
          else
          {
              strcat(_lineFormat, " CLOSED");
          }
        }
      }
    }
    strcat(&_lineFormat[strlen(_lineFormat)], " Id%03d Srch%03d/%02d");
}

/*********************************************************************
 * @fn          UI_UpdateDeviceInfoLine
 *
 * @brief       Generate part of the Device info string
 *
 * @param       none
 *
 * @return      none
 */
void UI_UpdateDeviceInfoLine(void)
{
    zstack_ZStatusValues statusValues;
    zstack_sysNwkInfoReadRsp_t rsp;
    statusValues = Zstackapi_sysNwkInfoReadReq(uiAppEntity, &rsp);
    if (zstack_ZStatusValues_ZSuccess != statusValues)
        return;

    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

    strcat(lineFormat, "["CUI_COLOR_CYAN"IEEE Addr"CUI_COLOR_RESET"] ");

    char* pBuffer = &lineFormat[strlen(lineFormat)];
    for (int8_t i = Z_EXTADDR_LEN - 1; i >= 0; i--)
    {
      uint8_t ch;
      ch = ((rsp.ieeeAddr[i] >> 4) & 0x0F);
      *pBuffer = ch + (( ch < 10 ) ? '0' : '7');
      pBuffer++;
      ch = (rsp.ieeeAddr[i] & 0x0F);
      *pBuffer = ch + (( ch < 10 ) ? '0' : '7');
      pBuffer++;
    }
    *pBuffer = 0;  //Add end of string

    strcat(lineFormat, " ["CUI_COLOR_CYAN"Power Mode"CUI_COLOR_RESET"] ");

    if(rsp.devState == zstack_DevState_HOLD)
    {
      strcat(lineFormat, "N/A");
    }
    else
    {
      if(rsp.capInfo.rxOnWhenIdle == TRUE)
      {
        strcat(lineFormat, "RX Always On");
      }
      else
      {
        strcat(lineFormat, "Sleepy");
      }
    }

    CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gDeviceInfoLine, lineFormat);
}

/*********************************************************************
 * @fn          UI_UpdateNwkStatusLine
 *
 * @brief       Generate part of the Nwk info string
 *
 * @param       none
 *
 * @return      none
 */
void UI_UpdateNwkStatusLine(void)
{
    zstack_ZStatusValues statusValues;
    zstack_sysNwkInfoReadRsp_t rsp;
    statusValues = Zstackapi_sysNwkInfoReadReq(uiAppEntity, &rsp);
    if (zstack_ZStatusValues_ZSuccess != statusValues)
        return;

    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

    strcat(lineFormat, "["CUI_COLOR_CYAN"PAN ID"CUI_COLOR_RESET"] 0x%04x ["CUI_COLOR_CYAN"Channel"CUI_COLOR_RESET"] %02d ["CUI_COLOR_CYAN"Short Address"CUI_COLOR_RESET"] 0x%04x");

    CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gNwkInfoLine, lineFormat, rsp.panId, rsp.logicalChannel, rsp.nwkAddr);

#if ZG_BUILD_ENDDEVICE_TYPE
    char lineFormat2[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

    strcat(lineFormat2, "["CUI_COLOR_CYAN"Parent Address"CUI_COLOR_RESET"] 0x%04x");

    retVal = CUI_statusLinePrintf(gCuiHandle, gNwkInfoLine2, lineFormat2, rsp.parentNwkAddr);
#endif
}



#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*******************************************************************************
 * @fn          UI_UpdateGpStatusLine
 *
 * @brief       Function to display in the GP status line the updates of GP proxy
 *              commissioning state and/or the GP Sink commissioning state.
 *
 * @param       none
 *
 * @return      none
 */
void UI_UpdateGpStatusLine(void)
{
  char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

  strcpy(lineFormat, "["CUI_COLOR_CYAN"GPP Commissioning"CUI_COLOR_RESET"] ");

  if(gp_GetProxyCommissioningMode() == TRUE)
  {
    if(gppCommissioningTimeout > 0)
    {
        gppCommissioningTimeout--;
        if(gppCommissioningTimeout > 0)
        {
            if(UtilTimer_isActive(&uiGppClkStruct) != true)
            {
               UtilTimer_start(&uiGppClkStruct);
            }
        }

        strcat(lineFormat, "TIME %03ds");
    }
    else
    {
      //No time, then it is just enabled
      strcat(lineFormat, "ENABLED");
    }
  }
  else
  {
    gppCommissioningTimeout = 0;
    strcat(lineFormat, "DISABLED");
  }

#if defined (ENABLE_GREENPOWER_COMBO_BASIC)
  strcat(lineFormat, " ["CUI_COLOR_CYAN"GPS Commissioning"CUI_COLOR_RESET"] ");
  if(gp_GetSinkCommissioningMode())
  {
      strcat(lineFormat, "ENABLED ");
  }
  else
  {
      strcat(lineFormat, "DISABLED");
  }
#endif

  CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gGpInfoLine,
                                                     lineFormat,gppCommissioningTimeout);

}
#endif

/*******************************************************************************
 * @fn          UI_UpdateBdbStatusLine
 *
 * @brief       Function to display information from the last successful bind
 *
 * @param
 *
 * @return      none
 */
void UI_UpdateBindInfoLine(void)
{
  char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

  strcat(lineFormat, "["CUI_COLOR_CYAN"Last bind"CUI_COLOR_RESET"] Addr ");

  char* pBuffer = &lineFormat[strlen(lineFormat)];
  for (int8_t i = Z_EXTADDR_LEN - 1; i >= 0; i--)
  {
    uint8_t ch;
    ch = ((lastBindNotification.dstAddr.addr.extAddr[i] >> 4) & 0x0F);
    *pBuffer = ch + (( ch < 10 ) ? '0' : '7');
    pBuffer++;
    ch = (lastBindNotification.dstAddr.addr.extAddr[i] & 0x0F);
    *pBuffer = ch + (( ch < 10 ) ? '0' : '7');
    pBuffer++;
  }
  *pBuffer = 0;  //Add end of string

  strcat(lineFormat, " ClusterId 0x%04x");
  strcat(lineFormat, " Endpoint 0x%02x");

  CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gBindInfoLine, lineFormat,
                        lastBindNotification.clusterId, lastBindNotification.ep);
}


/*******************************************************************************
 * @fn          UI_UpdateBdbStatusLine
 *
 * @brief       Function to display in the BDB status line the updates of BDB timers
 *              and/or state changes.
 *
 * @param       bdbCommissioningModeMsg - commissioning mode and state information,
 *              if NULL is provided, then these are read from the stack
 *
 * @return      none
 */
void UI_UpdateBdbStatusLine(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg)
{
    uiCommissioningIsInitializing = FALSE;
    uint8_t permitJoinDuration = 0;
    char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};
    uint8_t mode, status;

    if(bdbCommissioningModeMsg != NULL)
    {
        mode = bdbCommissioningModeMsg->bdbCommissioningMode;
        status = bdbCommissioningModeMsg->bdbCommissioningStatus;
    }
    else
    {
        zstack_bdbGetAttributesRsp_t rsp;
        Zstackapi_bdbGetAttributesReq(uiAppEntity, &rsp);

        mode = rsp.bdbCommissioningMode;
        status = rsp.bdbCommissioningStatus;
    }

    generateBdbInfoString(mode, status, 0,
                          &permitJoinDuration, lineFormat);

    zstack_bdbGetFBInitiatorStatusRsp_t zstack_bdbGetFBInitiatorStatusRsp;

    Zstackapi_bdbGetFBInitiatorStatusReq(uiAppEntity, &zstack_bdbGetFBInitiatorStatusRsp);

    uint16_t identifyTime = 0;

    if(pUiIdentifyTimeAttribute != NULL)
    {
      identifyTime = *pUiIdentifyTimeAttribute;
    }

#ifdef BDB_TL_TARGET
      uint32_t temp_u32;
      zstack_bdbTouchLinkTargetGetTimerRsp_t zstack_bdbTouchLinkTargetGetTimerRsp;

      Zstackapi_bdbTouchLinkTargetGetTimerReq(uiAppEntity, &zstack_bdbTouchLinkTargetGetTimerRsp);

      strcat(&lineFormat[strlen(lineFormat)], " ["CUI_COLOR_CYAN"TL Target"CUI_COLOR_RESET"] ");

      temp_u32 = (zstack_bdbTouchLinkTargetGetTimerRsp.Time);
      if (temp_u32 == 0)
      {
          strcat(&lineFormat[strlen(lineFormat)], "DISABLED");
      }
      else if (temp_u32 == TOUCHLINK_TARGET_PERPETUAL)
      {
        strcat(&lineFormat[strlen(lineFormat)], "ENABLED FOREVER ");
      }
      else
      {
        strcat(&lineFormat[strlen(lineFormat)], "Enabled for %05ds");

        temp_u32 = temp_u32 / 1000 + ((temp_u32 % 1000) > 0 ? 1 : 0);
      }

      CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gBdbInfoLine, lineFormat,
                                                 identifyTime,
                                                 zstack_bdbGetFBInitiatorStatusRsp.RemainingTime,
                                                 FBMatchesFound,
                                                 temp_u32);
#else

    CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gBdbInfoLine,
                                               lineFormat, identifyTime,
                                               zstack_bdbGetFBInitiatorStatusRsp.RemainingTime,
                                               FBMatchesFound);
#endif
}

static void UI_processKey(Button_Handle _buttonHandle,
                                   Button_EventMask _buttonEvents)
{
    if((_buttonHandle == gLeftButtonHandle) && gpAppKeyCB)
    {
        gpAppKeyCB(CONFIG_BTN_LEFT, _buttonEvents);
    }
    if((_buttonHandle == gRightButtonHandle) && gpAppKeyCB)
    {
        gpAppKeyCB(CONFIG_BTN_RIGHT, _buttonEvents);
    }
}



/*******************************************************************************
 * @fn      zclSampleApps_initializeClocks
 *
 * @brief   Initialize Clocks
 *
 * @param   none
 *
 * @return  none
 */
static void zclSampleApps_initializeClocks(void)
{
    // Construct the timer to update the Bdb and Nwk Info Line
    ClockP_Handle uiBdbNwkLineUpdate = UtilTimer_construct(
    &uiBdbNwkLineUpdateClk,
    uiRaiseBdbNwkLineUpdateEvt,
    UI_AUTO_REFRESH_INTERVAL_INFO_LINE,
    0,
    true,
    0);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
    // Construct the timer used to update gpp timeout screen
    uiGppClkHandle = UtilTimer_construct(
    &uiGppClkStruct,
    zclSampleAppsUI_ProcessGPPUpdateTimeoutCallback,
    UI_AUTO_REFRESH_INTERVAL_INFO_LINE,
    0,
    false,
    0);
#endif

#if defined(USE_DMM) &&  defined(BLOCK_MODE_TEST)
    clkBlockModeTestHandle = UtilTimer_construct(&clkBlockModeTestStruct,
                                             zclSampleApps_blockModeTestClockHandler,
                                             ZCL_BLOCK_MODE_ON_PERIOD, ZCL_BLOCK_MODE_ON_PERIOD, false,
                                             0);
#endif
}


/*******************************************************************************
 * @fn      uiRaiseNwkLineUpdateEvt
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void uiRaiseBdbNwkLineUpdateEvt(UArg a0)
{
    (void)a0; // Parameter is not used

    //Assign the UI event
    events |= SAMPLEAPP_UI_BDB_NWK_LINE_UPDATE_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(uiAppSem);

    if(UtilTimer_isActive(&uiBdbNwkLineUpdateClk) != true) {
        UtilTimer_start(&uiBdbNwkLineUpdateClk);
    }
}


#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*******************************************************************************
 * @fn      zclSampleAppsUI_ProcessGPPUpdateTimeoutCallback
 *
 * @brief   Timeout handler function
 *
 * @param   a0 - ignored
 *
 * @return  none
 */
static void zclSampleAppsUI_ProcessGPPUpdateTimeoutCallback(UArg a0)
{
  (void)a0; // Parameter is not used

  //Assign the gpp event
  events |= SAMPLEAPP_UI_GP_LINE_UPDATE_EVT;

  // Wake up the application thread when it waits for clock event
  Semaphore_post(uiAppSem);
}
#endif

/* new app callback created to set the new event */
static void ui_ProcessUARTEvent(void)
{
    events |= SAMPLEAPP_UI_INPUT_EVT;

    // Wake up the application thread when it waits for clock event
    Semaphore_post(uiAppSem);
}

/*********************************************************************
 * @fn      zclSampleAppsUI_changeKeyCallback
 *
 * @brief   Key event handler function
 *
 * @param   keysPressed - keys to be process in application context
 *
 * @return  none
 */
static void zclSampleAppsUI_changeKeyCallback(Button_Handle _buttonHandle, Button_EventMask _buttonEvents)
{
    if (_buttonEvents & Button_EV_CLICKED)
    {
        keys = _buttonHandle;

        events |= SAMPLEAPP_KEY_EVT_UI;

        // Wake up the application thread when it waits for clock event
        Semaphore_post(uiAppSem);
    }
}

/*********************************************************************
 * @fn          UI_Init
 *
 * @brief      Initialize the user-interface module.
 *             This function must be called by the application during its initialization.
 *
 * @param      zclSampleApp_Entity - Application Task ID.
               zclSampleAppEvents - The events processed by the sample application.
               zclSampleAppSem - Semaphore to post the events in the application thread.
               ui_IdentifyTimeAttribute_value - a pointer to the identify attribute's value.
               defaultBdbCommisioningModes - a pointer to the applications bdbCommissioningModes.
               pAppStr - Application string to initialize CUI module.
               zclSampleApp_changeKeyCallback - Application callback to process the keys pressed.
               uiAppFNResetCB - A pointer to the app-specific NV Item reset function.
 *
 * @return      CUI_clientHandle_t - CUI Handle
 */
CUI_clientHandle_t UI_Init(uint8_t  zclSampleApp_Entity, uint32_t *zclSampleAppEvents, Semaphore_Handle zclSampleAppSem,
              uint16_t *ui_IdentifyTimeAttribute_value, uint16_t *defaultBdbCommisioningModes,
              CONST char *pAppStr, uiAppProcessKeyCB_t zclSampleApp_processKey, uiAppFNResetCB_t _uiAppFNResetCB)
{
  uiAppEntity = zclSampleApp_Entity;
  uiAppSem = zclSampleAppSem;
  pUiIdentifyTimeAttribute = ui_IdentifyTimeAttribute_value;
  gpAppKeyCB = zclSampleApp_processKey;
  uiAppFNResetCB = _uiAppFNResetCB;
  gpBdbCommisioningModes = defaultBdbCommisioningModes;

  /* Initialize btns */
  Button_Params bparams;
  Button_Params_init(&bparams);
  gLeftButtonHandle = Button_open(CONFIG_BTN_LEFT, &bparams);
  // Open Right button without appCallBack
  gRightButtonHandle = Button_open(CONFIG_BTN_RIGHT, &bparams);

  // Read button state
  if (!GPIO_read(((Button_HWAttrs*)gRightButtonHandle->hwAttrs)->gpioIndex))
  {
      uiAppFNResetCB();
      Zstackapi_bdbResetLocalActionReq(uiAppEntity);
  }

  // Set button callback
  Button_setCallback(gRightButtonHandle, zclSampleAppsUI_changeKeyCallback);

#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
  // Read button state
  if (!GPIO_read(((Button_HWAttrs*)gLeftButtonHandle->hwAttrs)->gpioIndex))
  {
      //Return to FN image
      otaClient_loadExtImage(ST_FULL_FACTORY_IMAGE);
  }
#endif

  // Set button callback
  Button_setCallback(gLeftButtonHandle, zclSampleAppsUI_changeKeyCallback);

  gCuiHandle = UI_InitCUI(pAppStr);

  zclSampleApps_initializeClocks();

  /* Register the zigbee main menu with the CUI */
  CUI_registerMenu(gCuiHandle, &zclMenuMain);

  /* Request Async Line for Device Info */
  CUI_statusLineResourceRequest(gCuiHandle, "Device Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gDeviceInfoLine);

  UI_UpdateDeviceInfoLine();

  /* Request Async Line for NWK Info */
  CUI_statusLineResourceRequest(gCuiHandle, "   NWK Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gNwkInfoLine);

#if ZG_BUILD_ENDDEVICE_TYPE
  CUI_statusLineResourceRequest(gCuiHandle, "   NWK Info"CUI_DEBUG_MSG_START"2"CUI_DEBUG_MSG_END, false, &gNwkInfoLine2);
#endif

  UI_UpdateNwkStatusLine();

  /* Request Async Line for ZDO Info */
  CUI_statusLineResourceRequest(gCuiHandle, "   ZDO Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gZdoInfoLine);

  /* Request Async Line for BDB Info */
  CUI_statusLineResourceRequest(gCuiHandle, "   BDB Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gBdbInfoLine);

  /* Request Async Line for Bind Info */
  CUI_statusLineResourceRequest(gCuiHandle, "  Bind Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gBindInfoLine);

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  /* Request Async Line for GP Info */
  CUI_statusLineResourceRequest(gCuiHandle, "    GP Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gGpInfoLine);

  UI_UpdateGpStatusLine();
#endif

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
  /* Request Async Line for Block Mode Test Info */
  CUI_statusLineResourceRequest(gCuiHandle, "BMTest Info"CUI_DEBUG_MSG_START"1"CUI_DEBUG_MSG_END, false, &gBlockModeTestInfoLine);
#endif

  //Request the Green LED for UI
  /* Initialize the LEDS */

  LED_Params ledParams;
  LED_Params_init(&ledParams);
  gGreenLedHandle = LED_open(CONFIG_LED_GREEN, &ledParams);

  //Update ZDO status line
  UI_DeviceStateUpdated(NULL);

  //Update BDB status line
  UI_UpdateBdbStatusLine(NULL);

  return gCuiHandle;

}

/*********************************************************************
 * @fn      uiProcessIdentifyTimeChange
 *
 * @brief   Update the UI when the identify time attribute's value has changed
 *
 * @param   endpoint - the endpoints which had its identify time attribute changed
 *
 * @return  none
 */
void uiProcessIdentifyTimeChange( uint8_t *endpoint )
{
  if ((pUiIdentifyTimeAttribute != NULL) && *pUiIdentifyTimeAttribute > 0 )
  {
      LED_startBlinking(gGreenLedHandle, 500, LED_BLINK_FOREVER);
  }
  else
  {
    zstack_ZStatusValues statusValues;
    zstack_sysNwkInfoReadRsp_t rsp;
    statusValues = Zstackapi_sysNwkInfoReadReq(uiAppEntity, &rsp);
    if (zstack_ZStatusValues_ZSuccess != statusValues)
        return;

    switch(rsp.devState)
    {
      case zstack_DevState_DEV_END_DEVICE:
      case zstack_DevState_DEV_ROUTER:
      case zstack_DevState_DEV_ZB_COORD:
          LED_stopBlinking(gGreenLedHandle);
          LED_setOn(gGreenLedHandle, LED_BRIGHTNESS_MAX);
      break;
      default:
          LED_stopBlinking(gGreenLedHandle);
          LED_setOff(gGreenLedHandle);
      break;
    }
  }

  UI_UpdateBdbStatusLine(NULL);

  (void) endpoint;
}

/*********************************************************************
 * @fn      uiProcessBindNotification
 *
 * @brief   Update the UI when a bind is added
 *
 * @param   data - information about the new binding entry
 *
 * @return  none
 */
void uiProcessBindNotification( bdbBindNotificationData_t *data )
{
  lastBindNotification.ep = data->ep;
  lastBindNotification.clusterId = data->clusterId;

  memcpy(lastBindNotification.dstAddr.addr.extAddr, data->dstAddr.addr.extAddr, Z_EXTADDR_LEN);

  if (FBMatchesFound < 255)
  {
    FBMatchesFound++;
  }

  UI_UpdateBdbStatusLine(NULL);
  UI_UpdateBindInfoLine();
}

/*********************************************************************
 * @fn      UI_DeviceStateUpdated
 *
 * @brief   Update the UI when the device state has changed
 *
 * @param   NwkState - the new network-state of the device
 *
 * @return  none
 */
void UI_DeviceStateUpdated(zstack_devStateChangeInd_t *pReq)
{
  char lineFormat[MAX_STATUS_LINE_VALUE_LEN] = {'\0'};

  if ((pUiIdentifyTimeAttribute != NULL) && (*pUiIdentifyTimeAttribute == 0)  && (pReq != NULL))
  {
    switch (pReq->state)
    {
    case zstack_DevState_DEV_ZB_COORD:
    case zstack_DevState_DEV_ROUTER:
    case zstack_DevState_DEV_END_DEVICE:
        LED_stopBlinking(gGreenLedHandle);
        LED_setOn(gGreenLedHandle, LED_BRIGHTNESS_MAX);
      break;
    default:
        LED_stopBlinking(gGreenLedHandle);
        LED_setOff(gGreenLedHandle);
      break;
    }
  }
  else
  {
      LED_stopBlinking(gGreenLedHandle);
      LED_setOff(gGreenLedHandle);
  }

  strcat(lineFormat, "["CUI_COLOR_CYAN"Logical Device"CUI_COLOR_RESET"] ");

  if(ZG_BUILD_COORDINATOR_TYPE && ZG_DEVICE_COORDINATOR_TYPE)
  {
    strcat(lineFormat, "Coordinator");
  }
  else if (ZG_BUILD_RTRONLY_TYPE)
  {
    strcat(lineFormat, "Router");
  }
  else if (ZG_BUILD_ENDDEVICE_TYPE)
  {
    strcat(lineFormat, "End Device");
  }
  else
  {
      strcat(lineFormat, "Unknown");
  }

  if(pReq != NULL)
  {
    strcat(lineFormat," ["CUI_COLOR_CYAN"State"CUI_COLOR_RESET"] ");

    switch (pReq->state)
    {
      /** Initialized - not started automatically */
      case zstack_DevState_HOLD:
          strcat(lineFormat, "Initialized");
          break;
      /** Initialized - not connected to anything */
      case zstack_DevState_INIT:
          strcat(lineFormat, "Not On Network");
          break;
      /** Discovering PAN's to join */
      case zstack_DevState_NWK_DISC:
          strcat(lineFormat, "Discovering");
          break;
      /** Joining a PAN */
      case zstack_DevState_NWK_JOINING:
          strcat(lineFormat, "Joining");
          break;
      /**
       * ReJoining a PAN in secure mode scanning in current channel,
       * only for end devices
       */
      case zstack_DevState_NWK_REJOIN_SEC_CURR_CHANNEL:
          strcat(lineFormat, "Rejoining");
          break;
      /** Joined but not yet authenticated by trust center */
      case zstack_DevState_END_DEVICE_UNAUTH:
          strcat(lineFormat, "Joined Unsecured");
          break;
      /** Started as device after authentication */
      case zstack_DevState_DEV_END_DEVICE:
          strcat(lineFormat, "In the network");
          break;
      /** Device joined, authenticated and is a router */
      case zstack_DevState_DEV_ROUTER:
          strcat(lineFormat, "In the network");
          break;
      /** Started as Zigbee Coordinator */
      case zstack_DevState_COORD_STARTING:
          strcat(lineFormat, "In the network");
          break;
      /** Started as Zigbee Coordinator */
      case zstack_DevState_DEV_ZB_COORD:
          strcat(lineFormat, "In the network");
          break;
      /** Device has lost information about its parent */
      case zstack_DevState_NWK_ORPHAN:
          strcat(lineFormat, "Orphaned");
          break;
      /** Device is sending KeepAlive message to its parent */
      case zstack_DevState_NWK_KA:
          strcat(lineFormat, "Sending Keep Alive");
          break;
      /** Device is waiting before trying to rejoin */
      case zstack_DevState_NWK_BACKOFF:
          strcat(lineFormat, "Backing Off");
          break;
      /**
       * ReJoining a PAN in secure mode scanning in all channels,
       * only for end devices
       */
      case zstack_DevState_NWK_REJOIN_SEC_ALL_CHANNEL:
          strcat(lineFormat, "Rejoining Secured");
          break;
      /**
       * ReJoining a PAN in unsecure mode scanning in current channel,
       * only for end devices
       */
      case zstack_DevState_NWK_TC_REJOIN_CURR_CHANNEL:
          strcat(lineFormat, "Rejoining Unsecured");
          break;
      /**
       * ReJoining a PAN in unsecure mode scanning in all channels,
       * only for end devices
       */
      case zstack_DevState_NWK_TC_REJOIN_ALL_CHANNEL:
          strcat(lineFormat, "Rejoining Unsecured");
          break;
      default:
          strcat(lineFormat, "--");
        break;
    }
  }

  CUI_retVal_t retVal = CUI_statusLinePrintf(gCuiHandle, gZdoInfoLine, lineFormat);
}

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
/*******************************************************************************
 * @fn      UI_SetGPPCommissioningMode
 *
 * @brief   Interface to set the GP Proxy commissioning mode
 *
 * @param   isEntering  - TRUE if commissioning mode is being enabled, False otherwise
 * @param   hasTime     - TRUE if is entering the commissioning with a timeout, false otherwise
 * @param   time        - Time in seconds for which the device is in commissioning mode
 *
 * @return  void
 */
void UI_SetGPPCommissioningMode( zstack_gpCommissioningMode_t *Req )
{
  gp_SetProxyCommissioningMode(Req->isEntering);

  if(Req->hasTime)
  {
    if(Req->time > 0)
    {
      gppCommissioningTimeout = Req->time + 1;
      UtilTimer_stop(&uiGppClkStruct);
      UtilTimer_setTimeout(uiGppClkHandle,1000);
      UtilTimer_start(&uiGppClkStruct);
    }
    else
    {
      gp_SetProxyCommissioningMode(FALSE);
    }
  }
  else
  {
    gppCommissioningTimeout = 0;
    UtilTimer_stop(&uiGppClkStruct);
    UtilTimer_setTimeout(uiGppClkHandle, 0);
  }

  UI_UpdateGpStatusLine();
}

#endif



/*******************************************************************************
 * @fn      UI_InitCUI
 *
 * @brief   Initialize the CUI (common user interface)
 *
 * @param   pAppStr - Application string to initialize CUI module.
 *          appChangeKeyCB - Application callback to process the keys pressed.
 *          AppFNResetCB - A pointer to the app-specific NV Item reset function.
 *
 * @return  CUI_clientHandle_t - CUI Handle
 */
static CUI_clientHandle_t UI_InitCUI(CONST char* pAppStr)
{
  CUI_params_t params;
  CUI_paramsInit(&params);

#ifndef BOARD_DISPLAY_USE_UART
  //Do not initialize the UART, UI is not being used
  params.manageUart = FALSE;
#endif

  CUI_retVal_t retVal = CUI_init(&params);

  CUI_clientParams_t clientParams;
  CUI_clientParamsInit(&clientParams);

  strncpy(clientParams.clientName, pAppStr, MAX_CLIENT_NAME_LEN);
  clientParams.maxStatusLines = SAMPLEAPP_UI_MAX_STATUS_LINES;

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
  clientParams.maxStatusLines++;
#endif

  gCuiHandle = CUI_clientOpen(&clientParams);
  if (gCuiHandle == 0U)
      while(1){};

  return gCuiHandle;
}


void zclsampleApp_ui_event_loop(void)
{
  if(!events)
  {
    return;
  }

  if (events & SAMPLEAPP_UI_BDB_NWK_LINE_UPDATE_EVT)
  {
    UI_UpdateDeviceInfoLine();
    UI_UpdateNwkStatusLine();
    UI_UpdateBdbStatusLine(NULL);
#if (defined OTA_CLIENT_STANDALONE) || (defined OTA_CLIENT_INTEGRATED)
    otaClient_UpdateStatusLine();
#endif
    events &= ~SAMPLEAPP_UI_BDB_NWK_LINE_UPDATE_EVT;
  }

  if (events & SAMPLEAPP_UI_INPUT_EVT)
  {
    CUI_retVal_t retVal;
    events &= ~SAMPLEAPP_UI_INPUT_EVT;
    retVal = CUI_processMenuUpdate();
    if (CUI_SUCCESS != retVal)
    {
      while(1)
      {

      };
    }
  }

#if !defined (DISABLE_GREENPOWER_BASIC_PROXY) && (ZG_BUILD_RTR_TYPE)
  if (events & SAMPLEAPP_UI_GP_LINE_UPDATE_EVT)
  {
    UI_UpdateGpStatusLine();
    events &= ~SAMPLEAPP_UI_GP_LINE_UPDATE_EVT;
  }
#endif

  if (events & SAMPLEAPP_KEY_EVT_UI)
  {
    UI_processKey(keys, Button_EV_CLICKED);
    events &= ~SAMPLEAPP_KEY_EVT_UI;
  }
}

#if defined(USE_DMM) && defined(BLOCK_MODE_TEST)
/*********************************************************************
 * @fn      zclSampleApps_blockModeTestClockHandler
 *
 * @brief   Handler function for clock timeouts.
 *
 * @param   arg - event type
 *
 * @return  None.
 */
static void zclSampleApps_blockModeTestClockHandler(UArg arg)
{
  // stop the timer
  UtilTimer_stop(&clkBlockModeTestStruct);

  if (DMMPolicy_getBlockModeStatus(DMMPolicy_StackRole_ZigbeeDevice))
  {
    // update the DMM Block Mode status
    DMMPolicy_setBlockModeOff(DMMPolicy_StackRole_ZigbeeDevice);

    // restart the timer with new timeout value
    UtilTimer_setTimeout(clkBlockModeTestHandle, ZCL_BLOCK_MODE_OFF_PERIOD);
    UtilTimer_start(&clkBlockModeTestStruct);
  }
  else
  {
    // update the DMM Block Mode status
    DMMPolicy_setBlockModeOn(DMMPolicy_StackRole_ZigbeeDevice);

    // restart the timer with new timeout value
    UtilTimer_setTimeout(clkBlockModeTestHandle, ZCL_BLOCK_MODE_ON_PERIOD);
    UtilTimer_start(&clkBlockModeTestStruct);
  }
}

/*********************************************************************
 * @fn      zclSampleApps_blockModeTestOn
 *
 * @brief   Turn the periodic block mode on for BLE.
 *
 * @param   menuEntryIndex - index of CUI menu option
 */
static void zclSampleApps_blockModeTestOn(int32_t menuEntryIndex)
{
  if (!UtilTimer_isActive(&clkBlockModeTestStruct))
  {
    UtilTimer_setTimeout(clkBlockModeTestHandle, ZCL_BLOCK_MODE_ON_PERIOD);
    UtilTimer_start(&clkBlockModeTestStruct);
    DMMPolicy_setBlockModeOn(DMMPolicy_StackRole_ZigbeeDevice);
    CUI_statusLinePrintf(gCuiHandle, gBlockModeTestInfoLine, "Enabled");
  }
}

/*********************************************************************
 * @fn      zclSampleApps_blockModeTestOff
 *
 * @brief   Turn the periodic block mode off for BLE.
 *
 * @param   menuEntryIndex - index of CUI menu option
 */
static void zclSampleApps_blockModeTestOff(int32_t menuEntryIndex)
{
  if (UtilTimer_isActive(&clkBlockModeTestStruct))
  {
      UtilTimer_stop(&clkBlockModeTestStruct);
  }
  DMMPolicy_setBlockModeOff(DMMPolicy_StackRole_ZigbeeDevice);
  CUI_statusLinePrintf(gCuiHandle, gBlockModeTestInfoLine, "Disabled");
}


/*********************************************************************
 * @fn      zclSampleApps_setBlockModeOnPeriodAction
 *
 * @brief   Handle a user input to update the Block Mode On Period value
 *
 * @param   _input - input character
 * @param   _pLines - action menu title
 * @param   _pCurInfo - current cursor info
 */
static void zclSampleApps_setBlockModeOnPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  zclSampleApps_setBlockModePeriodUiAction(&ZCL_BLOCK_MODE_ON_PERIOD, _input, _pLines, _pCurInfo);
}

/*********************************************************************
 * @fn      zclSampleApps_setBlockModeOffPeriodAction
 *
 * @brief   Handle a user input to update the Block Mode Off Period value
 *
 * @param   _input - input character
 * @param   _pLines - action menu title
 * @param   _pCurInfo - current cursor info
 */
static void zclSampleApps_setBlockModeOffPeriodAction(const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
{
  zclSampleApps_setBlockModePeriodUiAction(&ZCL_BLOCK_MODE_OFF_PERIOD, _input, _pLines, _pCurInfo);
}

/*********************************************************************
 * @fn      zclSampleApps_setBlockModePeriodUiAction
 *
 * @brief   Handle a user input to update the Block Mode Off Period value
 *
 * @param   blockModePeriod - Block Mode Period
 * @param   _input - input character
 * @param   _pLines - action menu title
 * @param   _pCurInfo - current cursor info
 */
static void zclSampleApps_setBlockModePeriodUiAction(uint16_t* blockModePeriod, const char _input, char* _pLines[3], CUI_cursorInfo_t* _pCurInfo)
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
    if (blockModePeriod == &ZCL_BLOCK_MODE_ON_PERIOD)
    {
      strcpy(_pLines[2], "BM ON Period (ms)");
    }
    else if (blockModePeriod == &ZCL_BLOCK_MODE_OFF_PERIOD)
    {
      strcpy(_pLines[2], "BM OFF Period (ms)");
    }
    _pCurInfo->row = 1;
    _pCurInfo->col = cursor.col+1;
  }
}

/**
 *  @brief Callback to be called when the UI move a cursor Left.
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

  if (0 != skip_space)
  {
    //skip the white space, by continuing to move left over it
    if ((col % 3) == 0)
      col--;
  }
  return col;
}

/**
 *  @brief Callback to be called when the UI moves a cursor right.
 */
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

#endif

#endif //CUI_DISABLE


