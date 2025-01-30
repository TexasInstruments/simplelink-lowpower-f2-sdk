/******************************************************************************

 @file sensor.c

 @brief TIMAC 2.0 Sensor Example Application

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
#include <stdint.h>
#include "mac_util.h"
#include "api_mac.h"
#include "jdllc.h"
#include "ssf.h"
#include "smsgs.h"
#include "sensor.h"
#include <advanced_config.h>
#include "ti_154stack_config.h"

#ifdef FEATURE_NATIVE_OAD
#include "oad_client.h"
#endif /* FEATURE_NATIVE_OAD */

#ifdef OSAL_PORT2TIRTOS
#include <ti/drivers/dpl/ClockP.h>
#else
#include "icall.h"
#endif

#ifdef USE_DMM
#ifdef DMM_CENTRAL
#include "central_display.h"
#else
#include "remote_display.h"
#endif /* DMM_CENTRAL */
#include "ti_dmm_application_policy.h"
#ifdef FEATURE_SECURE_COMMISSIONING
#include "sm_commissioning_gatt_profile.h"
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /* USE_DMM */

#ifdef LPSTK
#include "lpstk/lpstk.h"
#endif /* LPSTK */

#ifndef CUI_DISABLE
#include "cui.h"
#endif /* CUI_DISABLE */

#ifdef DEVICE_TYPE_MSG
#include <ti/devices/DeviceFamily.h>
#include "device_type.h"
#endif /* DEVICE_TYPE_MSG */

#ifdef FEATURE_SECURE_COMMISSIONING
#include "sm_ti154.h"
#include "hal_types.h"
#endif /* FEATURE_SECURE_COMMISSIONING */

#ifdef MAC_DUTY_CYCLE_CHECKING
#include "mac_duty_cycle/mac_duty_cycle.h"
#endif

#ifdef MAC_OVERRIDE_TX_DELAY
#include "mac_api.h"
#endif

#include <ti/log/Log.h>

/******************************************************************************
 Constants and definitions
 *****************************************************************************/

#if !defined(CONFIG_AUTO_START)
#if defined(AUTO_START)
#define CONFIG_AUTO_START 1
#else
#define CONFIG_AUTO_START 0
#endif
#endif

/* default MSDU Handle rollover */
#define MSDU_HANDLE_MAX 0x1F

/* App marker in MSDU handle */
#define APP_MARKER_MSDU_HANDLE 0x80

/* App Message Tracking Mask */
#define APP_MASK_MSDU_HANDLE 0x60

/* App Sensor Data marker for the MSDU handle */
#define APP_SENSOR_MSDU_HANDLE 0x40

/* App tracking response marker for the MSDU handle */
#define APP_TRACKRSP_MSDU_HANDLE 0x20

/* App config response marker for the MSDU handle */
#define APP_CONFIGRSP_MSDU_HANDLE 0x60

/* Reporting Interval Min and Max (in milliseconds) */
#define MIN_REPORTING_INTERVAL 1000
#define MAX_REPORTING_INTERVAL 360000

/* Polling Interval Min and Max (in milliseconds) */
#define MIN_POLLING_INTERVAL 1000
#define MAX_POLLING_INTERVAL 10000

/* Blink Time for Identify LED Request (in seconds) */
#define IDENTIFY_LED_TIME 1

/* Inter packet interval in certification test mode */
#if CERTIFICATION_TEST_MODE
#if (((CONFIG_PHY_ID >= APIMAC_MRFSK_STD_PHY_ID_BEGIN) && (CONFIG_PHY_ID <= APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN)) || \
    ((CONFIG_PHY_ID >= APIMAC_200KBPS_915MHZ_PHY_132) && (CONFIG_PHY_ID <= APIMAC_200KBPS_868MHZ_PHY_133)) || \
    (CONFIG_PHY_ID == APIMAC_200KBPS_920MHZ_PHY_136))
/*! Regular Mode */
#define CERT_MODE_INTER_PKT_INTERVAL 50
#elif ((CONFIG_PHY_ID >= APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN + 1) && (CONFIG_PHY_ID <= APIMAC_5KBPS_868MHZ_PHY_131))
/*! LRM Mode */
#define CERT_MODE_INTER_PKT_INTERVAL 300
#else
#error "PHY ID is wrong."
#endif
#endif

/******************************************************************************
 Global variables
 *****************************************************************************/
/* MAC's IEEE address. This is only for Sensor */
extern ApiMac_sAddrExt_t ApiMac_extAddr;

/* Task pending events */
uint16_t Sensor_events = 0;

/* accumulated total E2E delay */
uint32_t totalE2EDelaySum = 0;

/* saved end to end delay */
uint32_t endToEndDelay = 0;

/*! Sensor statistics */
Smsgs_msgStatsField_t Sensor_msgStats =
    { 0 };
extern bool initBroadcastMsg;
extern bool parentFound;

#ifdef POWER_MEAS
/*! Power Meas Stats fields */
Smsgs_powerMeastatsField_t Sensor_pwrMeasStats =
    { 0 };
#endif
/******************************************************************************
 Local variables
 *****************************************************************************/

static void *sem;

/*! Rejoined flag */
static bool rejoining = false;

/*! Collector's address */
static ApiMac_sAddr_t collectorAddr = {0};

/* Join Time Ticks (used for average join time calculations) */
static uint_fast32_t joinTimeTicks = 0;

/* End to end delay statistics timestamp */
static uint32_t startSensorMsgTimeStamp = 0;

/*! Device's Outgoing MSDU Handle values */
STATIC uint8_t deviceTxMsduHandle = 0;

#ifdef IEEE_COEX_TEST
Smsgs_configReqMsg_t configSettings;
#else
STATIC Smsgs_configReqMsg_t configSettings;
#endif

#if !defined(OAD_IMG_A) && !defined(POWER_MEAS)
/*!
 Temp Sensor field - valid only if Smsgs_dataFields_tempSensor
 is set in frameControl.
 */
STATIC Smsgs_tempSensorField_t tempSensor =
    { 0 };

/*!
 Light Sensor field - valid only if Smsgs_dataFields_lightSensor
 is set in frameControl.
 */
STATIC Smsgs_lightSensorField_t lightSensor =
    { 0 };

/*!
 Humidity Sensor field - valid only if Smsgs_dataFields_humiditySensor
 is set in frameControl.
 */
STATIC Smsgs_humiditySensorField_t humiditySensor =
    { 0 };

#ifdef LPSTK
/*!
 Hall Effect Sensor field - valid only if Smsgs_dataFields_hallEffectSensor
 is set in frameControl.
 */
STATIC Smsgs_hallEffectSensorField_t hallEffectSensor =
    { 0 };

/*!
 Accelerometer Sensor field - valid only if Smsgs_dataFields_accelSensor
 is set in frameControl.
 */
STATIC Smsgs_accelSensorField_t accelerometerSensor =
    { 0 };
#endif /* LPSTK */

#ifdef DMM_CENTRAL
/*!
 BLE Sensor field - valid only if Smsgs_dataFields_bleSensor
 is set in frameControl.
 */
STATIC Smsgs_bleSensorField_t bleSensor =
    { 0 };
#endif

#endif /* !defined(OAD_IMG_A) && !defined(POWER_MEAS) */

STATIC Llc_netInfo_t parentInfo = {0};

STATIC uint16_t lastRcvdBroadcastMsgId = 0;

#ifdef FEATURE_SECURE_COMMISSIONING
/* variable to store the current setting of auto Request Pib attribute
 * before it gets modified by SM module, in beacon mode
 */
static bool currAutoReq = 0;
SMMsgs_authMethod_t smAuthMethod = SM_SENSOR_DEFAULT_AUTH_METHOD;
#endif /* FEATURE_SECURE_COMMISSIONING */

#ifdef DMM_OAD
ApiMac_deviceDescriptor_t cacheddevInfo = {0};
Llc_netInfo_t cachedparentInfo = {0};
#endif

#if defined(DISPLAY_PER_STATS)
extern void Ssf_displayRxStats(int8_t rssi);
#endif
/******************************************************************************
 Local function prototypes
 *****************************************************************************/
static void initializeClocks(void);
static void commStatusIndCB(ApiMac_mlmeCommStatusInd_t *pCommStatusInd);
static void dataCnfCB(ApiMac_mcpsDataCnf_t *pDataCnf);
static void dataIndCB(ApiMac_mcpsDataInd_t *pDataInd);
static uint8_t getMsduHandle(Smsgs_cmdIds_t msgType);

#if !defined(OAD_IMG_A) && !defined(POWER_MEAS)
static void processSensorMsgEvt(void);
static bool sendSensorMessage(ApiMac_sAddr_t *pDstAddr,
                              Smsgs_sensorMsg_t *pMsg);
static void readSensors(void);
#endif /* !defined(OAD_IMG_A) && !defined(POWER_MEAS) */

#if SENSOR_TEST_RAMP_DATA_SIZE && (CERTIFICATION_TEST_MODE || defined(POWER_MEAS))
static void processSensorRampMsgEvt(void);
#endif

#ifdef LPSTK
static void lpstkAccelerometerTiltCb(void);
#endif /* LPSTK */

static void processConfigRequest(ApiMac_mcpsDataInd_t *pDataInd);
static void processBroadcastCtrlMsg(ApiMac_mcpsDataInd_t *pDataInd);
static bool sendConfigRsp(ApiMac_sAddr_t *pDstAddr, Smsgs_configRspMsg_t *pMsg);
static uint16_t validateFrameControl(uint16_t frameControl);

#if defined(DEVICE_TYPE_MSG)
static void Sensor_sendDeviceTypeResponse(void);
#endif /* DEVICE_TYPE_MSG */

static void jdllcJoinedCb(ApiMac_deviceDescriptor_t *pDevInfo,
                          Llc_netInfo_t  *pStartedInfo);
static void jdllcDisassocIndCb(ApiMac_sAddrExt_t *extAddress,
                               ApiMac_disassocateReason_t reason);
static void jdllcDisassocCnfCb(ApiMac_sAddrExt_t *extAddress,
                               ApiMac_status_t status);
static void jdllcStateChangeCb(Jdllc_states_t state);

#ifdef FEATURE_SECURE_COMMISSIONING
/* Security Manager callback functions */
static void smFailCMProcessCb(ApiMac_deviceDescriptor_t *devInfo,
                              bool rxOnIdle, bool keyRefreshment,
                              SMMsgs_errorCode_t errorCode);
static void smSuccessCMProcessCb(ApiMac_deviceDescriptor_t *devInfo,
                                 bool keyRefreshment);
#endif /* FEATURE_SECURE_COMMISSIONING */

#if (USE_DMM)
static void macSyncLossCb(ApiMac_mlmeSyncLossInd_t *pSyncLossInd);
#if !(DMM_CENTRAL)
// Remote display callback functions
static void setRDAttrCb(RemoteDisplayAttr_t remoteDisplayAttr, void *const value, uint8_t len);
static void getRDAttrCb(RemoteDisplayAttr_t remoteDisplayAttr, void *value, uint8_t len);

// Provisioning callback functions
static void provisionConnectCb(void);
static void provisionDisconnectCb(void);
static void setProvisioningCb(ProvisionAttr_t provisioningAttr, void *const value, uint8_t len);
static void getProvisioningCb(ProvisionAttr_t provisioningAttr, void *value, uint8_t len);

// Helper functions
void swapBytes(uint8_t *arr, uint8_t len);
#endif /* !DMM_CENTRAL */
#endif /* USE_DMM */

#ifdef DMM_OAD
static void Sensor_dmmPausePolicyCb(uint16_t pause);
#endif /* DMM_OAD */

/******************************************************************************
 Callback tables
 *****************************************************************************/

/*! API MAC Callback table */
STATIC ApiMac_callbacks_t Sensor_macCallbacks =
    {
      /*! Associate Indicated callback */
      NULL,
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
      NULL,
      /*! Start Confirmation callback */
      NULL,
#if (USE_DMM)
      /*! Sync Loss Indication callback */
      macSyncLossCb,
#else
      /*! Sync Loss Indication callback */
      NULL,
#endif /* USE_DMM */
      /*! Poll Confirm callback */
      NULL,
      /*! Comm Status Indication callback */
      commStatusIndCB,
      /*! Poll Indication Callback */
      NULL,
      /*! Data Confirmation callback */
      dataCnfCB,
      /*! Data Indication callback */
      dataIndCB,
      /*! Purge Confirm callback */
      NULL,
      /*! WiSUN Async Indication callback */
      NULL,
      /*! WiSUN Async Confirmation callback */
      NULL,
      /*! Unprocessed message callback */
      NULL
    };

STATIC Jdllc_callbacks_t jdllcCallbacks =
    {
      /*! Network Joined Indication callback */
      jdllcJoinedCb,
      /* Disassociation Indication callback */
      jdllcDisassocIndCb,
      /* Disassociation Confirm callback */
      jdllcDisassocCnfCb,
      /*! State Changed indication callback */
      jdllcStateChangeCb
    };
#ifdef FEATURE_SECURE_COMMISSIONING
STATIC SM_callbacks_t SMCallbacks =
    {
      /*! Request passkey callback */
      Ssf_SmPasskeyEntry,
      /*! Security authentication failed callback */
      smFailCMProcessCb,
      /* Security authentication successful callback */
      smSuccessCMProcessCb
    };
#endif /* FEATURE_SECURE_COMMISSIONING */

#if (USE_DMM) && !(DMM_CENTRAL)
RemoteDisplay_clientProvisioningtCbs_t provisioning_sensorCbs =
{
    setProvisioningCb,
    getProvisioningCb,
    provisionConnectCb,
    provisionDisconnectCb
};

RemoteDisplayCbs_t remoteDisplay_sensorCbs =
{
    setRDAttrCb,
    getRDAttrCb
};
#endif /* USE_DMM && !DMM_CENTRAL */

#ifdef DMM_OAD
/*********************************************************************
 * DMM Policy Callbacks
 */
static DMMPolicy_AppCbs_t dmmPolicyAppCBs =
{
     Sensor_dmmPausePolicyCb
};
#endif
/******************************************************************************
 Public Functions
 *****************************************************************************/
#ifdef USE_DMM
/*!
 Initialize MAC level security for this application.
 Public function defined in sensor.h
 */
void Sensor_securityInit(uint32_t frameCounter)
{
    Ssf_getFrameCounter(NULL, &frameCounter);

#ifdef FEATURE_MAC_SECURITY
    /* Initialize the MAC Security */
    Jdllc_securityInit(frameCounter, NULL);
#endif /* FEATURE_MAC_SECURITY */

}
#endif /* USE_DMM */
/*!
 Initialize this application.

 Public function defined in sensor.h
 */
#ifdef OSAL_PORT2TIRTOS
void Sensor_init(uint8_t macTaskId)
#else
void Sensor_init(void)
#endif
{
    ApiMac_deviceDescriptor_t devInfo;
    Llc_netInfo_t parentInfo;
#ifndef USE_DMM
    uint32_t frameCounter = 0;
#endif /* USE_DMM */
    /* Initialize the sensor's structures */
    memset(&configSettings, 0, sizeof(Smsgs_configReqMsg_t));
#if defined(TEMP_SENSOR)
    configSettings.frameControl |= Smsgs_dataFields_tempSensor;
#endif
#if defined(LIGHT_SENSOR)
    configSettings.frameControl |= Smsgs_dataFields_lightSensor;
#endif
#if defined(HUMIDITY_SENSOR)
    configSettings.frameControl |= Smsgs_dataFields_humiditySensor;
#endif
#ifdef LPSTK
    configSettings.frameControl |= Smsgs_dataFields_humiditySensor   |
                                   Smsgs_dataFields_lightSensor      |
                                   Smsgs_dataFields_hallEffectSensor |
                                   Smsgs_dataFields_accelSensor;
#endif /* LPSTK */
    configSettings.frameControl |= Smsgs_dataFields_msgStats;
    configSettings.frameControl |= Smsgs_dataFields_configSettings;
#ifdef DMM_CENTRAL
    configSettings.frameControl |= Smsgs_dataFields_bleSensor;
#endif

    if(!CERTIFICATION_TEST_MODE)
    {
        configSettings.reportingInterval = CONFIG_REPORTING_INTERVAL;
    }
    else
    {
        /* start back to back data transmission at the earliest */
        configSettings.reportingInterval = 100;
    }
    configSettings.pollingInterval = CONFIG_POLLING_INTERVAL;

    /* Initialize the MAC */
#ifdef OSAL_PORT2TIRTOS
    sem = ApiMac_init(macTaskId, CONFIG_FH_ENABLE);
#else
    sem = ApiMac_init(CONFIG_FH_ENABLE);
#endif

    /* Initialize the Joining Device Logical Link Controller */
    Jdllc_init(&Sensor_macCallbacks, &jdllcCallbacks);

    /* Register the MAC Callbacks */
    ApiMac_registerCallbacks(&Sensor_macCallbacks);

    /* Initialize the platform specific functions */
    Ssf_init(sem);

#ifdef LPSTK
#ifdef BLE_START
    /*
     * NOTE!!
     * OAD_open() must be called before the LPSTK sensors are initialized because there
     * is no arbitration between the Flash and Accelerometer SPI writes.
     * OAD_open() is called in remote_display.c, see RemoteDisplay_init().
     * Here we refer to BLE OAD, not 15.4 OAD.
     */

    /* Wait for BLE application to finish initializing the BLE OAD Module */
    Ssf_PendAppSem();
#endif /* BLE_START */
    /* This initializes all LPSTK's sensors, LEDs, and Buttons */
    Lpstk_init(sem, lpstkAccelerometerTiltCb);

    /* Set up a periodic read for sensors specified by the sensor mask */
    Lpstk_initSensorReadTimer((Lpstk_SensorMask)(LPSTK_HUMIDITY|
                                                LPSTK_TEMPERATURE|
                                                LPSTK_LIGHT|
                                                //LPSTK_HALL_EFFECT
                                                LPSTK_ACCELEROMETER),
                                                2000);
#endif /* LPSTK */

#ifdef FEATURE_SECURE_COMMISSIONING
    /* Intialize the security manager and register callbacks */
    SM_registerCallback(&SMCallbacks);
#endif /* FEATURE_SECURE_COMMISSIONING */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyCurrentDescriptorId,
                           (uint8_t)CONFIG_PHY_ID);

    ApiMac_mlmeSetReqUint8(ApiMac_attribute_channelPage,
                           (uint8_t)CONFIG_CHANNEL_PAGE);
#ifndef USE_DMM
    Ssf_getFrameCounter(NULL, &frameCounter);

#ifdef FEATURE_MAC_SECURITY
    /* Initialize the MAC Security */
    Jdllc_securityInit(frameCounter, NULL);
#endif /* FEATURE_MAC_SECURITY */
#endif /* !USE_DMM */
    /* Set the transmit power */
    ApiMac_mlmeSetReqUint8(ApiMac_attribute_phyTransmitPowerSigned,
                           (uint8_t)CONFIG_TRANSMIT_POWER);
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
#ifdef FCS_TYPE16
    /* Set the fcs type */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_fcsType,
                           (bool)1);
#endif

#ifdef MAC_DUTY_CYCLE_CHECKING
    ApiMac_mlmeSetReqBool(ApiMac_attribute_dutyCycleEnabled, true);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleRegulated,
                            DUTY_CYCLE_MEAS_PERIOD*MAC_DUTY_CYCLE_THRESHOLD/100);

    /* Critical and limited duty cycle modes unused, set to max
     * value to avoid entering state */
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleCritical,
                            UINT32_MAX);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_dutyCycleLimited,
                            UINT32_MAX);
#endif

#ifdef MAC_OVERRIDE_TX_DELAY
    ApiMac_mlmeSetReqBool(ApiMac_attribute_customMinTxOffEnabled, true);
    ApiMac_mlmeSetReqUint32(ApiMac_attribute_minTxOffTime,
                            (uint32_t)MAC_CONFIG_MIN_TX_OFF);
#endif

    /* Initialize the app clocks */
    initializeClocks();

#if defined(BLE_START) && defined(USE_DMM) && !(DMM_CENTRAL)
    RemoteDisplay_registerRDCbs(remoteDisplay_sensorCbs);
    RemoteDisplay_registerClientProvCbs(provisioning_sensorCbs);
#endif /* BLE_START && USE_DMM */

    if(Ssf_getNetworkInfo(&devInfo, &parentInfo) == true)
    {
#if USE_DMM
        /* Update policy */
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);
#endif

        /* Update Channel Mask to show the previous network channel */
        if (!CONFIG_FH_ENABLE)
        {
            uint8_t channelMask[APIMAC_154G_CHANNEL_BITMAP_SIZ] = {0};
            uint8_t idx = parentInfo.channel / 8;
            uint8_t shift = (parentInfo.channel % 8);
            uint8_t chan = (0x01) << shift;
            channelMask[idx] = chan;
            Jdllc_setChanMask(channelMask);
        }
        /* Start the device */
        Util_setEvent(&Sensor_events, SENSOR_START_EVT);
    }
    else if (CONFIG_AUTO_START)
    {
#if USE_DMM
        /* Update policy */
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);
#endif
        /* Start the device */
        Util_setEvent(&Sensor_events, SENSOR_START_EVT);
    }

    /* Update BLE remote display with initial state, which will trigger a read
     * of the default JDLLC setting
     */
#if defined(BLE_START) && defined(USE_DMM) && !(DMM_CENTRAL)
    RemoteDisplay_updateSensorJoinState(Jdllc_states_initWaiting);
#endif

#ifdef DMM_OAD
    // register the app callbacks
    DMMPolicy_registerAppCbs(dmmPolicyAppCBs, DMMPolicy_StackRole_154Sensor);
#endif

    Log_printf(LogModule_154_App, Log_INFO, "Sensor Initialized");
}

/*!
 Application task processing.

 Public function defined in sensor.h
 */
void Sensor_process(void)
{
    /* Start the collector device in the network */
    if(Sensor_events & SENSOR_START_EVT)
    {
        ApiMac_deviceDescriptor_t devInfo;
        Llc_netInfo_t parentInfo;
#ifdef USE_DMM
        uint32_t frameCounter = 0;

        /* update policy */
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);

        Sensor_securityInit(frameCounter);
#endif /* USE_DMM */

        if(Ssf_getNetworkInfo(&devInfo, &parentInfo) == true)
        {
            rejoining = true;

            Ssf_configSettings_t configInfo;
#ifdef FEATURE_MAC_SECURITY
            ApiMac_status_t stat;
#endif /* FEATURE_MAC_SECURITY */

            /* Do we have config settings? */
            if(Ssf_getConfigInfo(&configInfo) == true)
            {
                /* Save the config information */
                configSettings.frameControl = configInfo.frameControl;
                configSettings.reportingInterval = configInfo.reportingInterval;
                configSettings.pollingInterval = configInfo.pollingInterval;

                /* Update the polling interval in the LLC */
                Jdllc_setPollRate(configSettings.pollingInterval);
            }

            /* Initially, setup the parent as the collector */
            if(parentInfo.fh == true && CONFIG_RX_ON_IDLE)
            {
                collectorAddr.addrMode = ApiMac_addrType_extended;
                memcpy(&collectorAddr.addr.extAddr,
                       parentInfo.devInfo.extAddress, APIMAC_SADDR_EXT_LEN);
            }
            else
            {
                collectorAddr.addrMode = ApiMac_addrType_short;
                collectorAddr.addr.shortAddr = parentInfo.devInfo.shortAddress;
            }

#ifdef FEATURE_MAC_SECURITY
            /* Put the parent in the security device list */
            stat = Jdllc_addSecDevice(parentInfo.devInfo.panID,
                                      parentInfo.devInfo.shortAddress,
                                      &parentInfo.devInfo.extAddress, 0);
            if(stat != ApiMac_status_success)
            {
                Ssf_displayError("Auth Error: 0x", (uint8_t)stat);
            }
#endif /* FEATURE_MAC_SECURITY */

#ifdef FEATURE_SECURE_COMMISSIONING
            if(!CONFIG_FH_ENABLE)
            {
                nvDeviceKeyInfo_t devKeyInfo;
                if(Ssf_getDeviceKeyInfo(&devKeyInfo) == TRUE)
                {
                    SM_recoverKeyInfo(devInfo, parentInfo, devKeyInfo);
#ifdef USE_DMM
                    RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_SUCCESS);
#endif /* USE_DMM */
                }

            }
#endif /* FEATURE_SECURE_COMMISSIONING */
            Jdllc_rejoin(&devInfo, &parentInfo);
        }
        else
        {
            /* Reset flag when joining a new network */
            rejoining = false;

            /* Get Start Timestamp */
#ifdef OSAL_PORT2TIRTOS
            joinTimeTicks = ClockP_getSystemTicks();
#else
            joinTimeTicks = ICall_getTicks();
#endif
            Jdllc_join();
        }

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_START_EVT);
    }


    /* Is it time to send the next sensor data message? */
    if(Sensor_events & SENSOR_READING_TIMEOUT_EVT)
    {

#if !defined(OAD_IMG_A)

        /* In certification test mode, back to back data shall be sent */
        if(!CERTIFICATION_TEST_MODE)
        {
            /* Setup for the next message */
            Ssf_setReadingClock(configSettings.reportingInterval);
        }

#ifdef FEATURE_SECURE_COMMISSIONING
        /* if secure Commissioning feature is enabled, read
         * sensor data and send it only after the secure
         * commissioning process is done successfully.
         * else, do not read and send sensor data.
         */
        if(SM_Current_State != SM_CM_InProgress)
        {
#endif /* FEATURE_SECURE_COMMISSIONING */


#if SENSOR_TEST_RAMP_DATA_SIZE && (CERTIFICATION_TEST_MODE || defined(POWER_MEAS))
        processSensorRampMsgEvt();
#endif /* SENSOR_TEST_RAMP_DATA_SIZE */

#if !defined(POWER_MEAS)
        /* Read sensors */
        readSensors();

        /* Process Sensor Reading Message Event */
        processSensorMsgEvt();
#endif /* POWER_MEAS */

#ifdef FEATURE_SECURE_COMMISSIONING
        }
#endif /* FEATURE_SECURE_COMMISSIONING */

#endif //OAD_IMG_A

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_READING_TIMEOUT_EVT);
    }

#if defined(OAD_IMG_A)
    if(Sensor_events & SENSOR_OAD_SEND_RESET_RSP_EVT)
    {
        OADClient_processEvent(&Sensor_events);
    }
#endif //OAD_IMG_A

#ifdef DISPLAY_PER_STATS
    /* Is it time to update the PER display? */
    if(Sensor_events & SENSOR_UPDATE_STATS_EVT)
    {
        Ssf_displayPerStats(&Sensor_msgStats);

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_UPDATE_STATS_EVT);
   }
#endif /* DISPLAY_PER_STATS */
#ifdef LPSTK
    /* Process Launchpad Sensortag specific events */
    Lpstk_processEvents();
#endif /* LPSTK */
#if (USE_DMM) && !(DMM_CENTRAL)
    /* Is it provision start event? */
    if(Sensor_events & SENSOR_PROV_EVT)
    {
        /* update policy */
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_PROV_EVT);
    }
#endif /* USE_DMM && !(DMM_CENTRAL) */

    /* Is it disassociate event? */
    if(Sensor_events & SENSOR_DISASSOC_EVT)
    {
        Jdllc_sendDisassociationRequest();

#ifdef USE_DMM
#ifdef FEATURE_SECURE_COMMISSIONING
        RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_IDLE);
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /* USE_DMM */

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_DISASSOC_EVT);
    }

#ifdef DMM_OAD
    if(Sensor_events & SENSOR_PAUSE_EVT)
    {
        /* Turn off timers to temporarily stop transmissions */
        Ssf_setPollClock(0);
        Ssf_setReadingClock(0);
        if(CONFIG_FH_ENABLE)
        {
            Ssf_setTrickleClock(0, ApiMac_wisunAsyncFrame_advertisementSolicit);
            Ssf_setTrickleClock(0, ApiMac_wisunAsyncFrame_configSolicit);
        }

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_PAUSE_EVT);
    }
    if(Sensor_events & SENSOR_RESUME_EVT)
    {
        /* Call SENSOR_START_EVT to re-associate with collector if BLE-OAD fails */
        Util_setEvent(&Sensor_events, SENSOR_START_EVT);

        /* Wake up the application thread when it waits for clock event */
        Ssf_PostAppSem();

        /* Clear the event */
        Util_clearEvent(&Sensor_events, SENSOR_RESUME_EVT);
    }
#endif /* DMM_OAD */

    /* Process LLC Events */
    Jdllc_process();

    /* Allow the Specific functions to process */
    Ssf_processEvents();

#ifdef FEATURE_SECURE_COMMISSIONING
    /* Allow the security manager specific functions to process */
    SM_process();
#endif /* FEATURE_SECURE_COMMISSIONING */
    /*
     Don't process ApiMac messages until all of the sensor events
     are processed.
     */
#ifdef FEATURE_SECURE_COMMISSIONING
    /*only if there are no sensor events and security manager events to handle*/
    if((Sensor_events == 0) && (SM_events == 0))
#else
    if(Sensor_events == 0)
#endif /* FEATURE_SECURE_COMMISSIONING */
    {
        /* Wait for response message or events */
        ApiMac_processIncoming();
    }
}
#ifdef TX_POWER_TEST
uint32_t txPowerLimitTest = 0;
uint8_t txTestPower1 = 0;
uint8_t txTestPower2 = 5;
#endif

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
extern uint32_t numPacket;
extern uint32_t nSensorPkts;
#endif
/*!
 * @brief   Send MAC data request
 *
 * @param   type - message type
 * @param   pDstAddr - destination address
 * @param   rxOnIdle - true if not a sleepy device
 * @param   len - length of payload
 * @param   pData - pointer to the buffer
 *
 * @return  true if sent, false if not
 */
bool Sensor_sendMsg(Smsgs_cmdIds_t type, ApiMac_sAddr_t *pDstAddr,
                    bool rxOnIdle, uint16_t len, uint8_t *pData)
{
    bool ret = false;
    /* information about the network */
    ApiMac_mcpsDataReq_t dataReq;

#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
    if(nSensorPkts >= numPacket)
    {
        return (true);
    }
    nSensorPkts++;
#endif

    /* Timestamp to compute end to end delay */
#ifdef OSAL_PORT2TIRTOS
    startSensorMsgTimeStamp = ClockP_getSystemTicks();
#else
    startSensorMsgTimeStamp = ICall_getTicks();
#endif

    /* Construct the data request field */
    memset(&dataReq, 0, sizeof(ApiMac_mcpsDataReq_t));
    memcpy(&dataReq.dstAddr, pDstAddr, sizeof(ApiMac_sAddr_t));

    /* set the correct address mode. */
    if(pDstAddr->addrMode == ApiMac_addrType_extended)
    {
        dataReq.srcAddrMode = ApiMac_addrType_extended;
    }
    else
    {
        dataReq.srcAddrMode = ApiMac_addrType_short;
    }

    if(rejoining == true)
    {
        /* get the new panID from the mac */
        ApiMac_mlmeGetReqUint16(ApiMac_attribute_panId,
                                &(parentInfo.devInfo.panID));
    }

    dataReq.dstPanId = parentInfo.devInfo.panID;

    dataReq.msduHandle = getMsduHandle(type);

    dataReq.txOptions.ack = true;

    if(CERTIFICATION_TEST_MODE)
    {
        dataReq.txOptions.ack = false;
    }

    if(rxOnIdle == false)
    {
        dataReq.txOptions.indirect = true;
    }

    dataReq.msdu.len = len;
    dataReq.msdu.p = pData;

#ifdef FEATURE_MAC_SECURITY
#ifdef FEATURE_SECURE_COMMISSIONING
    {
        extern ApiMac_sAddrExt_t ApiMac_extAddr;
        SM_getSrcDeviceSecurityInfo(ApiMac_extAddr, SM_Sensor_SAddress, &dataReq.sec);
    }
#else
    Jdllc_securityFill(&dataReq.sec);
#endif /* FEATURE_SECURE_COMMISSIONING */
#endif /* FEATURE_MAC_SECURITY */

    if(type == Smsgs_cmdIds_sensorData || type == Smsgs_cmdIds_rampdata)
    {
        Sensor_msgStats.msgsAttempted++;
    }
    else if(type == Smsgs_cmdIds_trackingRsp)
    {
        Sensor_msgStats.trackingResponseAttempts++;
    }
    else if(type == Smsgs_cmdIds_configRsp)
    {
        Sensor_msgStats.configResponseAttempts++;
    }
#ifdef TX_POWER_TEST
    if (txPowerLimitTest % 2 == 0)
    {
        dataReq.power = txTestPower1;
    }
    else
    {
        dataReq.power = txTestPower2;
    }
#endif
    /* Send the message */
    if(ApiMac_mcpsDataReq(&dataReq) == ApiMac_status_success)
    {
        ret = true;
    }
    else
    {
        /* handle transaction overflow by retrying */
        if(type == Smsgs_cmdIds_sensorData || type == Smsgs_cmdIds_rampdata)
        {
            Ssf_setReadingClock(configSettings.reportingInterval);
        }
    }
#ifdef TX_POWER_TEST
    txPowerLimitTest++;
#endif
    return (ret);
}


/*!
 Send LED Identify Request to collector

 Public function defined in sensor.h
 */
void Sensor_sendIdentifyLedRequest(void)
{
    uint8_t cmdBytes[SMSGS_INDENTIFY_LED_REQUEST_MSG_LEN];

    /* send the response message directly */
    cmdBytes[0] = (uint8_t) Smsgs_cmdIds_IdentifyLedReq;
    cmdBytes[1] = (uint8_t) IDENTIFY_LED_TIME;
    Sensor_sendMsg(Smsgs_cmdIds_IdentifyLedReq,
            &collectorAddr, true,
            SMSGS_INDENTIFY_LED_REQUEST_MSG_LEN,
            cmdBytes);
}

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 * @brief Sets the Security Authentication Mode
 *
 *  Public function defined in sensor.h
 */
extern void Sensor_setSmAuthMethod(SMMsgs_authMethod_t authMethod)
{
    smAuthMethod = authMethod;
}

/*!
 * @brief Gets the Security Authentication Mode
 *
 *  Public function defined in sensor.h
 */
extern SMMsgs_authMethod_t Sensor_getSmAuthMethod(void)
{
    return smAuthMethod;
}
#endif /* FEATURE_SECURE_COMMISSIONING */

/******************************************************************************
 Local Functions
 *****************************************************************************/

/*!
 * @brief       Initialize the clocks.
 *
 */
static void initializeClocks(void)
{
#ifndef DMM_CENTRAL
    /* Initialize the reading clock */
    Ssf_initializeReadingClock();
#if (USE_DMM)
    Ssf_initializeProvisioningClock();
#endif /* USE_DMM */
#endif /* !DMM_CENTRAL */
}

/*!
 * @brief      Process the MAC Comm Status Indication Callback
 *
 * @param      pCommStatusInd - Comm Status indication
 */
static void commStatusIndCB(ApiMac_mlmeCommStatusInd_t *pCommStatusInd)
{
    uint32_t stat;

    /* Frame received with security error */
    if (pCommStatusInd->reason == ApiMac_commStatusReason_rxSecure)
    {

        ApiMac_mlmeGetReqUint32(ApiMac_attribute_diagRxSecureFail, &stat);
        Sensor_msgStats.rxDecryptFailures = (uint16_t)stat;

#ifndef MAC_OVERRIDE_TX_DELAY
        /* Enabling the minimum custom TX off-time feature in SysConfig
         * will overwrite ApiMac_attribute_diagTxSecureFail MAC PIB. See
         * api_mac.c/h
         */
        ApiMac_mlmeGetReqUint32(ApiMac_attribute_diagTxSecureFail, &stat);
        Sensor_msgStats.txEncryptFailures = (uint16_t)stat;
#endif

        Ssf_displayError("Security Error: ", pCommStatusInd->status);
    }
}

/*!
 * @brief      MAC Data Confirm callback.
 *
 * @param      pDataCnf - pointer to the data confirm information
 */
static void dataCnfCB(ApiMac_mcpsDataCnf_t *pDataCnf)
{

    /* Record statistics */
    if(pDataCnf->status == ApiMac_status_channelAccessFailure)
    {
        Sensor_msgStats.channelAccessFailures++;
    }
    else if(pDataCnf->status == ApiMac_status_noAck)
    {
        Sensor_msgStats.macAckFailures++;
#ifdef DISPLAY_PER_STATS
        Util_setEvent(&Sensor_events, SENSOR_UPDATE_STATS_EVT);
#endif /* DISPLAY_PER_STATS */

    }
    else if(pDataCnf->status != ApiMac_status_success)
    {
        Sensor_msgStats.otherDataRequestFailures++;
#ifdef DISPLAY_PER_STATS
        Util_setEvent(&Sensor_events, SENSOR_UPDATE_STATS_EVT);
#endif /* DISPLAY_PER_STATS */

        Ssf_displayError("dataCnf: ", pDataCnf->status);
    }
    else if(pDataCnf->status == ApiMac_status_success)
    {
        Ssf_updateFrameCounter(NULL, pDataCnf->frameCntr);
    }

#ifdef FEATURE_SECURE_COMMISSIONING
    /* Strictly ensure that the message did not come from the app */
    if(!(pDataCnf->msduHandle & APP_MARKER_MSDU_HANDLE))
    {
        if ((SM_CM_InProgress == SM_Current_State) && (true == useSendPktStatus))
        {
            /* Make sure the message came from the SM module */
            if (pDataCnf->msduHandle & SM_FAIL_MSDU_HANDLE)
            {
                if(pDataCnf->status != ApiMac_status_success)
                {
                    /* Set an event that failure status from previous packet sent is seen */
                    Util_setEvent(&SM_events, SM_SEND_PKT_FB_EVT);
                }
                else
                {
                    /* Switch to fail state once fail packet is received */
                    Util_setEvent(&SM_events, SM_SENT_CM_FAIL_EVT);
                }
            }
            else if(pDataCnf->msduHandle & SM_DATA_MSDU_HANDLE)
            {
                if(pDataCnf->status != ApiMac_status_success)
                {
                    /* Set an event that failure status from previous packet sent is seen */
                    Util_setEvent(&SM_events, SM_SEND_PKT_FB_EVT);
                }
            }
        }
    }
#endif /* FEATURE_SECURE_COMMISSIONING */

    /* Make sure the message came from the app */
    if(pDataCnf->msduHandle & APP_MARKER_MSDU_HANDLE)
    {
        /* What message type was the original request? */
        if((pDataCnf->msduHandle & APP_MASK_MSDU_HANDLE)
           == APP_SENSOR_MSDU_HANDLE)
        {
#ifndef IEEE_COEX_TEST
            if(pDataCnf->status == ApiMac_status_success)
#endif
            {
                Sensor_msgStats.msgsSent++;
                if (Sensor_msgStats.msgsSent == 0)
                {
                    /* the count is wrapped around, set to 1 to avoid divided by zero */
                    Sensor_msgStats.msgsSent = 1;
                    totalE2EDelaySum = 0;
                }
#ifdef DISPLAY_PER_STATS
                Util_setEvent(&Sensor_events, SENSOR_UPDATE_STATS_EVT);
#endif /* DISPLAY_PER_STATS */
                /* Calculate end to end delay */
#ifdef OSAL_PORT2TIRTOS
                if(ClockP_getSystemTicks() < startSensorMsgTimeStamp)
                {
                    endToEndDelay = ClockP_getSystemTicks() +
                                    (0xFFFFFFFF-startSensorMsgTimeStamp);
                }
                else
                {
                    endToEndDelay = ClockP_getSystemTicks() - startSensorMsgTimeStamp;
                }
#else
                if(ICall_getTicks() < startSensorMsgTimeStamp)
                {
                    endToEndDelay = ICall_getTicks() +
                                    (0xFFFFFFFF-startSensorMsgTimeStamp);
                }
                else
                {
                    endToEndDelay = ICall_getTicks() - startSensorMsgTimeStamp;
                }
#endif
                endToEndDelay = (endToEndDelay + TICKPERIOD_MS_US - 1)/TICKPERIOD_MS_US;
                if ( (totalE2EDelaySum + endToEndDelay ) < totalE2EDelaySum)
                {
                    /* totalE2EDelaySum is wrapped around,reset the sent count 1 */
                    totalE2EDelaySum = endToEndDelay;
                    Sensor_msgStats.msgsSent = 1;
                }
                else
                {
                    totalE2EDelaySum += endToEndDelay;
                }

                Sensor_msgStats.worstCaseE2EDelay =
                   (Sensor_msgStats.worstCaseE2EDelay > endToEndDelay) ?
                    Sensor_msgStats.worstCaseE2EDelay:endToEndDelay;

                Sensor_msgStats.avgE2EDelay=totalE2EDelaySum/Sensor_msgStats.msgsSent;

            }


#if CERTIFICATION_TEST_MODE
            {
                /* Setup for the next message */
                Ssf_setReadingClock(CERT_MODE_INTER_PKT_INTERVAL);
            }
#endif
        }
        if((pDataCnf->msduHandle & APP_MASK_MSDU_HANDLE)
           == APP_TRACKRSP_MSDU_HANDLE)
        {
            if(pDataCnf->status == ApiMac_status_success)
            {
                Sensor_msgStats.trackingResponseSent++;
            }
        }
        if((pDataCnf->msduHandle & APP_MASK_MSDU_HANDLE)
           == APP_CONFIGRSP_MSDU_HANDLE)
        {
            if(pDataCnf->status == ApiMac_status_success)
            {
                Sensor_msgStats.configResponseSent++;
            }
        }
    }
}

/*!
 * @brief      MAC Data Indication callback.
 *
 * @param      pDataInd - pointer to the data indication information
 */
static void dataIndCB(ApiMac_mcpsDataInd_t *pDataInd)
{
    uint8_t cmdBytes[SMSGS_TOGGLE_LED_RESPONSE_MSG_LEN];

    if((pDataInd != NULL) && (pDataInd->msdu.p != NULL)
       && (pDataInd->msdu.len > 0))
    {
        Smsgs_cmdIds_t cmdId = (Smsgs_cmdIds_t)*(pDataInd->msdu.p);

#ifdef FEATURE_MAC_SECURITY
        {
            if(Jdllc_securityCheck(&(pDataInd->sec)) == false)
            {
                /* reject the message */
                return;
            }
        }
#endif /* FEATURE_MAC_SECURITY */

        switch(cmdId)
        {
            case Smsgs_cmdIds_configReq:
                processConfigRequest(pDataInd);
                Sensor_msgStats.configRequests++;
                break;

            case Smsgs_cmdIds_trackingReq:
                /* Make sure the message is the correct size */
#ifndef IEEE_COEX_TEST
                if(pDataInd->msdu.len == SMSGS_TRACKING_REQUEST_MSG_LENGTH)
#endif
                {
                    /* only send data if sensor is in the network */
                    if ((Jdllc_getProvState() == Jdllc_states_joined) ||
                            (Jdllc_getProvState() == Jdllc_states_rejoined))
                    {
                        /* Update stats */
                        Sensor_msgStats.trackingRequests++;
#ifdef DISPLAY_PER_STATS
                        Ssf_displayRxStats(pDataInd->rssi);
#endif // DISPLAY_PER_STATS
                        /* Indicate tracking message received */
                        Ssf_trackingUpdate(&pDataInd->srcAddr);
#if (defined(IEEE_COEX_TEST) && defined(EN_TRACKINGRSP)) || !defined(IEEE_COEX_TEST)
                        /* send the response message directly */
                        cmdBytes[0] = (uint8_t) Smsgs_cmdIds_trackingRsp;

                        Sensor_sendMsg(Smsgs_cmdIds_trackingRsp,
                                           &pDataInd->srcAddr, true,
                                           1, cmdBytes);
#endif
                    }
                }
                break;

            case Smsgs_cmdIds_toggleLedReq:
                /* Make sure the message is the correct size */
                if(pDataInd->msdu.len == SMSGS_TOGGLE_LED_REQUEST_MSG_LEN)
                {
                    /* only send data if sensor is in the network */
                    if ((Jdllc_getProvState() == Jdllc_states_joined) ||
                            (Jdllc_getProvState() == Jdllc_states_rejoined))
                    {
                        /* send the response message directly */
                        cmdBytes[0] = (uint8_t) Smsgs_cmdIds_toggleLedRsp;
                        cmdBytes[1] = Ssf_toggleLED();
                        Sensor_sendMsg(Smsgs_cmdIds_toggleLedRsp,
                            &pDataInd->srcAddr, true,
                            SMSGS_TOGGLE_LED_RESPONSE_MSG_LEN,
                            cmdBytes);
                    }
                }
                break;

            case Smgs_cmdIds_broadcastCtrlMsg:
                if(parentFound)
                {
                    /* Node has successfully associated with the network */
                    processBroadcastCtrlMsg(pDataInd);
                }
                break;
#ifdef POWER_MEAS
            case Smsgs_cmdIds_rampdata:
                Sensor_pwrMeasStats.rampDataRcvd++;
                break;
#endif

#ifdef FEATURE_NATIVE_OAD
            case Smsgs_cmdIds_oad:
                //Index past the Smsgs_cmdId
                OADProtocol_ParseIncoming((void*) &(pDataInd->srcAddr), pDataInd->msdu.p + 1);
                break;
#endif //FEATURE_NATIVE_OAD
#ifdef FEATURE_SECURE_COMMISSIONING
            case Smgs_cmdIds_CommissionStart:
                {
                    ApiMac_sec_t devSec;
                    extern ApiMac_sAddrExt_t ApiMac_extAddr;

                    /* Obtain MAC level security information. Use network key for SM */
                    Jdllc_securityFill(&devSec);

                    uint8_t *pBuf = pDataInd->msdu.p;
                    pBuf += sizeof(Smsgs_cmdIds_t);
                    SMMsgs_cmdIds_t CMMsgId = (SMMsgs_cmdIds_t)Util_buildUint16(pBuf[0], pBuf[1]);

                    /* read the current value */
                    ApiMac_mlmeGetReqBool(ApiMac_attribute_autoRequest, &currAutoReq);

                    /* beacon-mode of operation and autoRequest is set to true and sleepy device  */
                    if((CONFIG_MAC_BEACON_ORDER != 15) && (currAutoReq == true) && (!CONFIG_RX_ON_IDLE))
                    {
                        /* set autoRequest to false and enable explicit polling */
                        ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, false);
                        Util_setEvent(&Jdllc_events, JDLLC_POLL_EVT);
                    }

                    if (CMMsgId == SMMsgs_cmdIds_keyRefreshRequest)
                    {
                        if(SM_Current_State != SM_CM_InProgress)
                        {
                            /* Kick off key refreshment process if SM state machine is not being run */
                            SM_startKeyRefreshProcess(&parentInfo.devInfo, &devSec,
                                                               parentInfo.fh, true);
                        }
                    }
                    else if(CMMsgId == SMMsgs_cmdIds_processRequest)
                    {
#ifdef USE_DMM
                        /* Kick off commissioning process to obtain security information */
                        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);
                        RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_STARTING);
#endif /* USE_DMM */
                        SM_startCMProcess(&parentInfo.devInfo, &devSec, parentInfo.fh,
                                          true, SM_type_device, smAuthMethod);
                    }
                }
                break;
            case Smgs_cmdIds_CommissionMsg:
                {
                    /* Process Security manager commissioning data */
                    SM_processCommData(pDataInd);
                }
                break;


#endif /* FEATURE_SECURE_COMMISSIONING */

#if defined(DEVICE_TYPE_MSG)
            case Smsgs_cmdIds_DeviceTypeReq:
                /* Make sure the message is the correct size */
                if(pDataInd->msdu.len == SMSGS_DEVICE_TYPE_REQUEST_MSG_LEN)
                {
                    /* Only send data if sensor is in the network */
                    if ((Jdllc_getProvState() == Jdllc_states_joined) ||
                            (Jdllc_getProvState() == Jdllc_states_rejoined))
                    {
                        Sensor_sendDeviceTypeResponse();
                    }
                }
                break;
#endif /* DEVICE_TYPE_MSG */

            default:
                /* Should not receive other messages */
                break;
        }
    }
}

/*!
 * @brief      Get the next MSDU Handle
 *             <BR>
 *             The MSDU handle has 3 parts:<BR>
 *             - The MSBit(7), when set means the the application sent the
 *               message
 *             - Bit 6, when set means that the app message is a config request
 *             - Bits 0-5, used as a message counter that rolls over.
 *
 * @param      msgType - message command id needed
 *
 * @return     msdu Handle
 */
static uint8_t getMsduHandle(Smsgs_cmdIds_t msgType)
{
    uint8_t msduHandle = deviceTxMsduHandle;

    /* Increment for the next msdu handle, or roll over */
    if(deviceTxMsduHandle >= MSDU_HANDLE_MAX)
    {
        deviceTxMsduHandle = 0;
    }
    else
    {
        deviceTxMsduHandle++;
    }

    /* Add the App specific bit */
    msduHandle |= APP_MARKER_MSDU_HANDLE;

    /* Add the message type bit */
    if(msgType == Smsgs_cmdIds_sensorData || msgType == Smsgs_cmdIds_rampdata)
    {
        msduHandle |= APP_SENSOR_MSDU_HANDLE;
    }
    else if(msgType == Smsgs_cmdIds_trackingRsp)
    {
        msduHandle |= APP_TRACKRSP_MSDU_HANDLE;
    }
    else if(msgType == Smsgs_cmdIds_configRsp)
    {
        msduHandle |= APP_CONFIGRSP_MSDU_HANDLE;
    }

    return (msduHandle);
}

/*!
 @brief  Build and send fixed size ramp data
 */
#if SENSOR_TEST_RAMP_DATA_SIZE && (CERTIFICATION_TEST_MODE || defined(POWER_MEAS))
static void processSensorRampMsgEvt(void)
{
    uint8_t *pMsgBuf;
    uint16_t index;

    pMsgBuf = (uint8_t *)Ssf_malloc(SENSOR_TEST_RAMP_DATA_SIZE);
    if(pMsgBuf)
    {
        uint8_t *pBuf = pMsgBuf;
        *pBuf++ = (uint8_t)Smsgs_cmdIds_rampdata;
        for(index = 1; index < SENSOR_TEST_RAMP_DATA_SIZE; index++)
        {
            *pBuf++ = (uint8_t) (index & 0xFF);
        }

#if !defined(POWER_MEAS) && !defined(USE_DMM)
        //Toggle Sensor reading LED
        Ssf_sensorReadingUpdate(NULL);
#endif

        Sensor_sendMsg(Smsgs_cmdIds_rampdata, &collectorAddr, true,
                SENSOR_TEST_RAMP_DATA_SIZE, pMsgBuf);

        Ssf_free(pMsgBuf);
    }

}
#endif

#if !defined(OAD_IMG_A) && !defined(POWER_MEAS)
/*!
 @brief   Build and send sensor data message
 */
static void processSensorMsgEvt(void)
{
    Smsgs_sensorMsg_t sensor;

    memset(&sensor, 0, sizeof(Smsgs_sensorMsg_t));

    ApiMac_mlmeGetReqArray(ApiMac_attribute_extendedAddress,
                           sensor.extAddress);

    /* fill in the message */
    sensor.frameControl = configSettings.frameControl;
    if(sensor.frameControl & Smsgs_dataFields_tempSensor)
    {
        memcpy(&sensor.tempSensor, &tempSensor,
               sizeof(Smsgs_tempSensorField_t));
    }
    if(sensor.frameControl & Smsgs_dataFields_lightSensor)
    {
        memcpy(&sensor.lightSensor, &lightSensor,
               sizeof(Smsgs_lightSensorField_t));
    }
    if(sensor.frameControl & Smsgs_dataFields_humiditySensor)
    {
        memcpy(&sensor.humiditySensor, &humiditySensor,
               sizeof(Smsgs_humiditySensorField_t));
    }
    if(sensor.frameControl & Smsgs_dataFields_msgStats)
    {
        memcpy(&sensor.msgStats, &Sensor_msgStats,
               sizeof(Smsgs_msgStatsField_t));
    }
    if(sensor.frameControl & Smsgs_dataFields_configSettings)
    {
        sensor.configSettings.pollingInterval = configSettings.pollingInterval;
        sensor.configSettings.reportingInterval = configSettings
                        .reportingInterval;
    }

#ifdef LPSTK
    if(sensor.frameControl & Smsgs_dataFields_hallEffectSensor)
    {
        sensor.hallEffectSensor.fluxLevel = hallEffectSensor.fluxLevel;
    }
    if(sensor.frameControl & Smsgs_dataFields_accelSensor)
    {
        memcpy(&sensor.accelerometerSensor, &accelerometerSensor,
                       sizeof(Smsgs_accelSensorField_t));
    }
#endif /* LPSTK */

#ifdef DMM_CENTRAL
    if(sensor.frameControl & Smsgs_dataFields_bleSensor)
    {
        memcpy(&sensor.bleSensor, &bleSensor,
                               sizeof(Smsgs_bleSensorField_t));
    }
#endif

    /* inform the user interface */
    Ssf_sensorReadingUpdate(&sensor);

#if defined(BLE_START) && (USE_DMM) && !(DMM_CENTRAL)
    /* Sync BLE application with new data */
    RemoteDisplay_updateSensorData();
#endif /* BLE_START && USE_DMM && !(DMM_CENTRAL) */
    /* send the data to the collector */
    sendSensorMessage(&collectorAddr, &sensor);
}

/*!
 * @brief   Manually read the sensors
 */
static void readSensors(void)
{
#if defined(TEMP_SENSOR)
    /* Read the temp sensor values */
    tempSensor.ambienceTemp = Ssf_readTempSensor();
    tempSensor.objectTemp =  tempSensor.ambienceTemp;
#endif
#ifdef LPSTK
    Lpstk_Accelerometer accel;
    humiditySensor.temp = (uint16_t)Lpstk_getTemperature();
    humiditySensor.humidity = (uint16_t)Lpstk_getHumidity();
    hallEffectSensor.fluxLevel = Lpstk_getHallEffectSwitch();
    lightSensor.rawData = (uint16_t)Lpstk_getLux();
    Lpstk_getAccelerometer(&accel);
    accelerometerSensor.xAxis = accel.x;
    accelerometerSensor.yAxis = accel.y;
    accelerometerSensor.zAxis = accel.z;
    accelerometerSensor.xTiltDet = accel.xTiltDet;
    accelerometerSensor.yTiltDet = accel.yTiltDet;
#endif /* LPSTK */
}

/*!
 * @brief   Build and send sensor data message
 *
 * @param   pDstAddr - Where to send the message
 * @param   pMsg - pointer to the sensor data
 *
 * @return  true if message was sent, false if not
 */
static bool sendSensorMessage(ApiMac_sAddr_t *pDstAddr, Smsgs_sensorMsg_t *pMsg)
{
    bool ret = false;
    uint8_t *pMsgBuf;
    uint16_t len = SMSGS_BASIC_SENSOR_LEN;

    /* Figure out the length */
    if(pMsg->frameControl & Smsgs_dataFields_tempSensor)
    {
        len += SMSGS_SENSOR_TEMP_LEN;
    }
    if(pMsg->frameControl & Smsgs_dataFields_lightSensor)
    {
        len += SMSGS_SENSOR_LIGHT_LEN;
    }
    if(pMsg->frameControl & Smsgs_dataFields_humiditySensor)
    {
        len += SMSGS_SENSOR_HUMIDITY_LEN;
    }
    if(pMsg->frameControl & Smsgs_dataFields_msgStats)
    {
        //len += SMSGS_SENSOR_MSG_STATS_LEN;
        len += sizeof(Smsgs_msgStatsField_t);
    }
    if(pMsg->frameControl & Smsgs_dataFields_configSettings)
    {
        len += SMSGS_SENSOR_CONFIG_SETTINGS_LEN;
    }
#ifdef LPSTK
    if(pMsg->frameControl & Smsgs_dataFields_hallEffectSensor)
    {
        len += sizeof(Smsgs_hallEffectSensorField_t);
    }
    if(pMsg->frameControl & Smsgs_dataFields_accelSensor)
    {
        len += sizeof(Smsgs_accelSensorField_t);
    }
#endif /* LPSTK */
#ifdef DMM_CENTRAL
    if(pMsg-> frameControl & Smsgs_dataFields_bleSensor)
    {
        len += SMSGS_SENSOR_BLE_LEN;
        if(pMsg->bleSensor.dataLength > MAX_BLE_DATA_LEN)
        {
            pMsg->bleSensor.dataLength = MAX_BLE_DATA_LEN;
        }
        len += pMsg->bleSensor.dataLength;
    }
#endif
    pMsgBuf = (uint8_t *)Ssf_malloc(len);
    if(pMsgBuf)
    {
        uint8_t *pBuf = pMsgBuf;

        *pBuf++ = (uint8_t)Smsgs_cmdIds_sensorData;

        memcpy(pBuf, pMsg->extAddress, SMGS_SENSOR_EXTADDR_LEN);
        pBuf += SMGS_SENSOR_EXTADDR_LEN;

        pBuf  = Util_bufferUint16(pBuf,pMsg->frameControl);

        /* Buffer data in order of frameControl mask, starting with LSB */
        if(pMsg->frameControl & Smsgs_dataFields_tempSensor)
        {
            pBuf = Util_bufferUint16(pBuf, pMsg->tempSensor.ambienceTemp);
            pBuf = Util_bufferUint16(pBuf, pMsg->tempSensor.objectTemp);
        }
        if(pMsg->frameControl & Smsgs_dataFields_lightSensor)
        {
            pBuf = Util_bufferUint16(pBuf, pMsg->lightSensor.rawData);
        }
        if(pMsg->frameControl & Smsgs_dataFields_humiditySensor)
        {
            pBuf = Util_bufferUint16(pBuf, pMsg->humiditySensor.temp);
            pBuf = Util_bufferUint16(pBuf, pMsg->humiditySensor.humidity);
        }
        if(pMsg->frameControl & Smsgs_dataFields_msgStats)
        {
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.joinAttempts);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.joinFails);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.msgsAttempted);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.msgsSent);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.trackingRequests);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->msgStats.trackingResponseAttempts);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->msgStats.trackingResponseSent);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.configRequests);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->msgStats.configResponseAttempts);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->msgStats.configResponseSent);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->msgStats.channelAccessFailures);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.macAckFailures);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->msgStats.otherDataRequestFailures);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.syncLossIndications);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.rxDecryptFailures);
            pBuf = Util_bufferUint16(pBuf,  pMsg->msgStats.txEncryptFailures);
            pBuf = Util_bufferUint16(pBuf, Ssf_resetCount);
            pBuf = Util_bufferUint16(pBuf,  Ssf_resetReseason);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.joinTime);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.interimDelay);
            pBuf = Util_bufferUint16(pBuf, pMsg->msgStats.numBroadcastMsgRcvd);
            pBuf = Util_bufferUint16(pBuf,  pMsg->msgStats.numBroadcastMsglost);
            pBuf = Util_bufferUint16(pBuf,  pMsg->msgStats.avgE2EDelay);
            pBuf = Util_bufferUint16(pBuf,  pMsg->msgStats.worstCaseE2EDelay);
        }
        if(pMsg->frameControl & Smsgs_dataFields_configSettings)
        {
            pBuf = Util_bufferUint32(pBuf,
                                     pMsg->configSettings.reportingInterval);
            pBuf = Util_bufferUint32(pBuf,
                                     pMsg->configSettings.pollingInterval);

        }
#ifdef LPSTK
        if(pMsg->frameControl & Smsgs_dataFields_hallEffectSensor)
        {
            *pBuf++ = (uint8_t)pMsg->hallEffectSensor.fluxLevel;
        }
        if(pMsg->frameControl & Smsgs_dataFields_accelSensor)
        {
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->accelerometerSensor.xAxis);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->accelerometerSensor.yAxis);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->accelerometerSensor.zAxis);
            *pBuf++ = pMsg->accelerometerSensor.xTiltDet;
            *pBuf++ = pMsg->accelerometerSensor.yTiltDet;
        }
#endif /* LPSTK */
#ifdef DMM_CENTRAL
        if(pMsg->frameControl & Smsgs_dataFields_bleSensor)
        {
            uint8_t i;
            for(i= 0; i < B_ADDR_LEN; i++)
            {
                *pBuf++ = pMsg->bleSensor.bleAddr[i];
            }
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->bleSensor.manFacID);
            pBuf = Util_bufferUint16(pBuf,
                                     pMsg->bleSensor.uuid);
            *pBuf++ = pMsg->bleSensor.dataLength;

            i = pMsg->bleSensor.dataLength;
            while(i != 0)
            {
                i--;
                *pBuf++ = pMsg->bleSensor.data[i];
            }
        }
#endif

        ret = Sensor_sendMsg(Smsgs_cmdIds_sensorData, pDstAddr, true, len, pMsgBuf);

        Ssf_free(pMsgBuf);
    }

    return (ret);
}

#endif /* !defined(OAD_IMG_A) && !defined(POWER_MEAS) */


/*!
 * @brief      Process the Config Request message.
 *
 * @param      pDataInd - pointer to the data indication information
 */
static void processConfigRequest(ApiMac_mcpsDataInd_t *pDataInd)
{
    Smsgs_statusValues_t stat = Smsgs_statusValues_invalid;
    Smsgs_configRspMsg_t configRsp;

    memset(&configRsp, 0, sizeof(Smsgs_configRspMsg_t));

    /* Make sure the message is the correct size */
    if(pDataInd->msdu.len == SMSGS_CONFIG_REQUEST_MSG_LENGTH)
    {
        uint8_t *pBuf = pDataInd->msdu.p;
        uint16_t frameControl;
        uint32_t reportingInterval;
        uint32_t pollingInterval;

        /* Parse the message */
        configSettings.cmdId = (Smsgs_cmdIds_t)*pBuf++;
        frameControl = Util_parseUint16(pBuf);
        pBuf += 2;
        reportingInterval = Util_parseUint32(pBuf);
        pBuf += 4;
        pollingInterval = Util_parseUint32(pBuf);

        stat = Smsgs_statusValues_success;
        collectorAddr.addrMode = pDataInd->srcAddr.addrMode;
        if(collectorAddr.addrMode == ApiMac_addrType_short)
        {
            collectorAddr.addr.shortAddr = pDataInd->srcAddr.addr.shortAddr;
        }
        else
        {
            memcpy(collectorAddr.addr.extAddr, pDataInd->srcAddr.addr.extAddr,
                   (APIMAC_SADDR_EXT_LEN));
        }

        configSettings.frameControl = validateFrameControl(frameControl);
        if(configSettings.frameControl != frameControl)
        {
            stat = Smsgs_statusValues_partialSuccess;
        }
        configRsp.frameControl = configSettings.frameControl;

        if((reportingInterval < MIN_REPORTING_INTERVAL)
           || (reportingInterval > MAX_REPORTING_INTERVAL))
        {
            stat = Smsgs_statusValues_partialSuccess;
        }
        else
        {
#ifndef POWER_MEAS
            configSettings.reportingInterval = reportingInterval;
#endif
            {
                uint32_t randomNum;
                randomNum = ((ApiMac_randomByte() << 16) +
                             (ApiMac_randomByte() << 8) + ApiMac_randomByte());
                randomNum = (randomNum % reportingInterval) +
                        SENSOR_MIN_POLL_TIME;
#if !defined(IEEE_COEX_TEST) || !defined(COEX_MENU)
                Ssf_setReadingClock(randomNum);
#endif
#ifdef LPSTK
                Lpstk_setSensorReadTimer((Lpstk_SensorMask)(LPSTK_HUMIDITY|
                                                             LPSTK_TEMPERATURE|
                                                             LPSTK_LIGHT|
                                                             LPSTK_HALL_EFFECT|
                                                             LPSTK_ACCELEROMETER),
                                                             reportingInterval);
#endif /* LPSTK */
            }

        }
        configRsp.reportingInterval = configSettings.reportingInterval;

        if((pollingInterval < MIN_POLLING_INTERVAL)
           || (pollingInterval > MAX_POLLING_INTERVAL))
        {
            stat = Smsgs_statusValues_partialSuccess;
        }
        else
        {
            configSettings.pollingInterval = pollingInterval;
            Jdllc_setPollRate(configSettings.pollingInterval);
        }
        configRsp.pollingInterval = configSettings.pollingInterval;
    }

    /* Send the response message */
    configRsp.cmdId = Smsgs_cmdIds_configRsp;
    configRsp.status = stat;

    /* Update the user */
    Ssf_configurationUpdate(&configRsp);

    /* Response the the source device */
    sendConfigRsp(&pDataInd->srcAddr, &configRsp);
#if defined(BLE_START) && (USE_DMM) && !(DMM_CENTRAL)
    /* Sync BLE application with new data */
    RemoteDisplay_updateSensorData();
#endif /* USE_DMM && !DMM_CENTRAL */
}

/*!
 * @brief      Process the Broadcast Control Msg.
 *
 * @param      pDataInd - pointer to the data indication information
 */
static void processBroadcastCtrlMsg(ApiMac_mcpsDataInd_t *pDataInd)
{
    Smsgs_broadcastcmdmsg_t broadcastCmd;

    memset(&broadcastCmd, 0, sizeof(Smsgs_broadcastcmdmsg_t));

    /* Make sure the message is the correct size */
    if(pDataInd->msdu.len == SMSGS_BROADCAST_CMD_LENGTH)
    {
        uint8_t *pBuf = pDataInd->msdu.p;
        uint16_t broadcastMsgId;

        /* Parse the message */
        uint8_t cmdId = (Smsgs_cmdIds_t)*pBuf++;
        broadcastMsgId = Util_parseUint16(pBuf);

        /* Process Broadcast Command Message */
        Sensor_msgStats.numBroadcastMsgRcvd++;

        if(!initBroadcastMsg)
        {
            /* Not the first broadcast msg rcvdd after join or a rejoin*/
            if((broadcastMsgId - lastRcvdBroadcastMsgId) > 1)
            {
                Sensor_msgStats.numBroadcastMsglost +=
                                ((broadcastMsgId - lastRcvdBroadcastMsgId) -1);
            }
        }

        lastRcvdBroadcastMsgId = broadcastMsgId;
        /*To handle the very first broadcast msg rcvdd after join or a rejoin*/
        initBroadcastMsg = false;

        /* Switch On or Off LED based on broadcast Msg Id */
        if((broadcastMsgId % 2) == 0)
        {
            Ssf_OnLED();
        }
        else
        {
            Ssf_OffLED();
        }
    }
}

/*!
 * @brief   Build and send Config Response message
 *
 * @param   pDstAddr - Where to send the message
 * @param   pMsg - pointer to the Config Response
 *
 * @return  true if message was sent, false if not
 */
static bool sendConfigRsp(ApiMac_sAddr_t *pDstAddr, Smsgs_configRspMsg_t *pMsg)
{
    uint8_t msgBuf[SMSGS_CONFIG_RESPONSE_MSG_LENGTH];
    uint8_t *pBuf = msgBuf;

    *pBuf++ = (uint8_t) Smsgs_cmdIds_configRsp;
    pBuf = Util_bufferUint16(pBuf, pMsg->status);
    pBuf = Util_bufferUint16(pBuf, pMsg->frameControl);
    pBuf = Util_bufferUint32(pBuf, pMsg->reportingInterval);
    pBuf = Util_bufferUint32(pBuf, pMsg->pollingInterval);

    return (Sensor_sendMsg(Smsgs_cmdIds_configRsp, pDstAddr, true,
                    SMSGS_CONFIG_RESPONSE_MSG_LENGTH, msgBuf));
}

/*!
 * @brief   Filter the frameControl with readings supported by this device.
 *
 * @param   frameControl - suggested frameControl
 *
 * @return  new frame control settings supported
 */
static uint16_t validateFrameControl(uint16_t frameControl)
{
    uint16_t newFrameControl = 0;

#if defined(TEMP_SENSOR)
    if(frameControl & Smsgs_dataFields_tempSensor)
    {
        newFrameControl |= Smsgs_dataFields_tempSensor;
    }
#endif
#if defined(LIGHT_SENSOR) || defined(LPSTK)
    if(frameControl & Smsgs_dataFields_lightSensor)
    {
        newFrameControl |= Smsgs_dataFields_lightSensor;
    }
#endif
#if defined(HUMIDITY_SENSOR) || defined(LPSTK)
    if(frameControl & Smsgs_dataFields_humiditySensor)
    {
        newFrameControl |= Smsgs_dataFields_humiditySensor;
    }
#ifdef LPSTK
    if(frameControl & Smsgs_dataFields_hallEffectSensor)
    {
        newFrameControl |= Smsgs_dataFields_hallEffectSensor;
    }
    if(frameControl & Smsgs_dataFields_accelSensor)
    {
        newFrameControl |= Smsgs_dataFields_accelSensor;
    }
#endif /* LPSTK */
#endif
    if(frameControl & Smsgs_dataFields_msgStats)
    {
        newFrameControl |= Smsgs_dataFields_msgStats;
    }
    if(frameControl & Smsgs_dataFields_configSettings)
    {
        newFrameControl |= Smsgs_dataFields_configSettings;
    }
#ifdef DMM_CENTRAL
    if(frameControl & Smsgs_dataFields_bleSensor)
    {
        newFrameControl |= Smsgs_dataFields_bleSensor;
    }
#endif

    return (newFrameControl);
}

#if defined(DEVICE_TYPE_MSG)
static void Sensor_sendDeviceTypeResponse(void)
{
    uint8_t cmdBytes[SMSGS_DEVICE_TYPE_RESPONSE_MSG_LEN];

    /* send the response message directly */
    cmdBytes[0] = (uint8_t) Smsgs_cmdIds_DeviceTypeRsp;
    cmdBytes[1] = DeviceFamily_ID;
    cmdBytes[2] = DeviceType_ID;

    Sensor_sendMsg(Smsgs_cmdIds_DeviceTypeRsp,
            &collectorAddr, true,
            SMSGS_DEVICE_TYPE_RESPONSE_MSG_LEN, cmdBytes);
}
#endif /* DEVICE_TYPE_MSG */

/*!
 * @brief   The device joined callback.
 *
 * @param   pDevInfo - This device's information
 * @param   pParentInfo - This is the parent's information
 */
static void jdllcJoinedCb(ApiMac_deviceDescriptor_t *pDevInfo,
                          Llc_netInfo_t *pParentInfo)
{
    uint32_t randomNum = 0;

    /* Copy the parent information */
    memcpy(&parentInfo, pParentInfo, sizeof(Llc_netInfo_t));

    /* Set the collector's address as the parent's address */
    if (pParentInfo->fh && CONFIG_RX_ON_IDLE)
    {
        collectorAddr.addrMode = ApiMac_addrType_extended;
        memcpy(collectorAddr.addr.extAddr, pParentInfo->devInfo.extAddress,
               (APIMAC_SADDR_EXT_LEN));
    }
    else
    {
        collectorAddr.addrMode = ApiMac_addrType_short;
        collectorAddr.addr.shortAddr = pParentInfo->devInfo.shortAddress;
    }

    /* Start the reporting timer */
    if(CONFIG_FH_ENABLE)
    {
        randomNum = ((ApiMac_randomByte() << 16) +
                     (ApiMac_randomByte() << 8) + ApiMac_randomByte());
        randomNum = (randomNum % configSettings.reportingInterval) +
                    SENSOR_MIN_POLL_TIME;
        Ssf_setReadingClock(randomNum);
    }
    else
    {
       uint32_t randomNum;
       randomNum = ((ApiMac_randomByte() << 16) +
                    (ApiMac_randomByte() << 8) + ApiMac_randomByte());
       randomNum = (randomNum % configSettings.reportingInterval ) +
                   SENSOR_MIN_POLL_TIME;
#if !defined(IEEE_COEX_TEST) || !defined(COEX_MENU)
       Ssf_setReadingClock(randomNum);
#endif
    }

    /* Inform the user of the joined information */
    Ssf_networkUpdate(rejoining, pDevInfo, pParentInfo);

#ifdef FEATURE_SECURE_COMMISSIONING
        SM_Sensor_SAddress = pDevInfo->shortAddress;
#endif /* FEATURE_SECURE_COMMISSIONING */
    if((rejoining == false) && (pParentInfo->fh == false))
    {
#ifdef FEATURE_MAC_SECURITY
        ApiMac_status_t stat;
        /* Add the parent to the security device list */
        stat = Jdllc_addSecDevice(pParentInfo->devInfo.panID,
                                  pParentInfo->devInfo.shortAddress,
                                  &pParentInfo->devInfo.extAddress, 0);
        if(stat != ApiMac_status_success)
        {
            Ssf_displayError("Auth Error: 0x", (uint8_t)stat);
        }
#endif /* FEATURE_MAC_SECURITY */
    }

#if (CONFIG_MAC_SUPERFRAME_ORDER != 15) && defined(MAC_NO_AUTO_REQ)
    /*
     * Set MAC Auto Request to false to enable multiple poll requests
     * per beacon interval
     */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, false);
#endif

#ifdef OSAL_PORT2TIRTOS
    /* Calculate Join Time */
    if(ClockP_getSystemTicks() < joinTimeTicks)
    {
        joinTimeTicks = ClockP_getSystemTicks() + (0xFFFFFFFF-joinTimeTicks);
    }
    else
    {
        joinTimeTicks = ClockP_getSystemTicks() - joinTimeTicks;
    }
#else
    /* Calculate Join Time */
    if(ICall_getTicks() < joinTimeTicks)
    {
        joinTimeTicks = ICall_getTicks() + (0xFFFFFFFF-joinTimeTicks);
    }
    else
    {
        joinTimeTicks = ICall_getTicks() - joinTimeTicks;
    }
#endif
    Sensor_msgStats.joinTime = joinTimeTicks / TICKPERIOD_MS_US;
#ifdef DISPLAY_PER_STATS
    /* clear the stats used for PER so that we start out at a
     * zeroed state
     */
    Sensor_msgStats.macAckFailures = 0;
    Sensor_msgStats.otherDataRequestFailures = 0;
    Sensor_msgStats.msgsSent = 0;
#endif
}

/*!
 * @brief   Disassociation indication callback.
 *
 * @param   pExtAddress - extended address
 * @param   reason - reason for disassociation
 */
static void jdllcDisassocIndCb(ApiMac_sAddrExt_t *pExtAddress,
                               ApiMac_disassocateReason_t reason)
{
    /* Stop the reporting timer */
    Ssf_setReadingClock(0);
    Ssf_clearNetworkInfo();

#ifdef FEATURE_MAC_SECURITY
    ApiMac_secDeleteDevice(pExtAddress);
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
    SM_removeEntryFromSeedKeyTable(&ApiMac_extAddr);
    Ssf_clearDeviceKeyInfo();
#ifdef USE_DMM
    RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_IDLE);
#endif /* USE_DMM */
#endif /* FEATURE_SECURE_COMMISSIONING */

#ifdef FEATURE_NATIVE_OAD
    /* OAD abort with no auto resume */
    OADClient_abort(false);
#endif //FEATURE_NATIVE_OAD
}

/*!
 * @brief   Disassociation confirm callback to an application intiated
 *          disassociation request.
 *
 * @param   pExtAddress - extended address
 * @param   status - status of disassociation
 */
static void jdllcDisassocCnfCb(ApiMac_sAddrExt_t *pExtAddress,
                               ApiMac_status_t status)
{
    /* Stop the reporting timer */
    Ssf_setReadingClock(0);
    Ssf_clearNetworkInfo();

#ifdef FEATURE_MAC_SECURITY
    ApiMac_secDeleteDevice(pExtAddress);
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
    SM_removeEntryFromSeedKeyTable(&ApiMac_extAddr);
    Ssf_clearDeviceKeyInfo();
#ifdef USE_DMM
    RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_IDLE);
#endif /* USE_DMM */
#endif /* FEATURE_SECURE_COMMISSIONING */
#ifdef FEATURE_NATIVE_OAD
    /* OAD abort with no auto resume */
    OADClient_abort(false);
#endif //FEATURE_NATIVE_OAD
}

/*!
 * @brief   JDLLC state change callback.
 *
 * @param   state - new state
 */
static void jdllcStateChangeCb(Jdllc_states_t state)
{
#ifdef FEATURE_NATIVE_OAD
    if( (state == Jdllc_states_joined) || (state == Jdllc_states_rejoined))
    {
#if (CONFIG_MAC_SUPERFRAME_ORDER == 15)
        /* resume an OAD that may have aborted */
        OADClient_resume(30000);
#else
        /* resume an OAD that may have aborted */
        OADClient_resume(60000);
#endif
    }
    else if(state == Jdllc_states_orphan)
    {
        /* OAD abort with no auto resume */
        OADClient_abort(false);
    }
#endif /* FEATURE_NATIVE_OAD */

#if USE_DMM
    if(state == Jdllc_states_initRestoring)
    {
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);
    }
    else if( (state == Jdllc_states_joined) || (state == Jdllc_states_rejoined))
    {
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_CONNECTED);
    }
    else if(state == Jdllc_states_orphan)
    {
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);
    }
    else if( (state == Jdllc_states_initWaiting) || (state == Jdllc_states_accessDenied) )
    {
        DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_UNINIT);
    }
#endif /* USE_DMM */

#if defined(BLE_START) && (USE_DMM) && !(DMM_CENTRAL)
    RemoteDisplay_updateSensorJoinState(state);
#endif /* BLE_START && USE_DMM && !DMM_CENTRAL */

#ifndef CUI_DISABLE
    Ssf_stateChangeUpdate(state);
#endif /* CUI_DISABLE */

#ifdef OAD_IMG_A
    if( (state == Jdllc_states_joined) || (state == Jdllc_states_rejoined))
    {
        Util_setEvent(&Sensor_events, SENSOR_OAD_SEND_RESET_RSP_EVT);
    }
#endif /* OAD_IMG_A */
}

#ifdef USE_DMM
/*!
 * @brief   Sync Loss callback indication beacon sync has been lost.
 *
 * @param   pSyncLossInd - Sync Loss Indication
 */
static void macSyncLossCb(ApiMac_mlmeSyncLossInd_t *pSyncLossInd)
{
    /* Update policy */
    DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_PROVISIONING);
#if defined(BLE_START) && (USE_DMM) && !(DMM_CENTRAL)
    RemoteDisplay_updateSensorJoinState((Jdllc_states_t)RemoteDisplay_JOIN_STATE_SYNC_LOSS);
#endif
}

#if !(DMM_CENTRAL)
/*!
 * @brief      DMM Provisioning connect (Association) callback function
 */
static void provisionConnectCb(void)
{
    Ssf_setProvisioningClock(true);

}

/*!
 * @brief      DMM Provisioning disconnect (Disassocation) callback function
 */
static void provisionDisconnectCb(void)
{
    Ssf_setProvisioningClock(false);
}

#endif /* !DMM_CENTRAL */
#endif /* USE_DMM */

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 * @brief      Security manager failure processing function
 */
void smFailCMProcessCb(ApiMac_deviceDescriptor_t *devInfo, bool rxOnIdle,
                       bool keyRefreshment, SMMsgs_errorCode_t errorCode)
{
    /* restore, write back current Pib value for auto request attribute */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, currAutoReq);

    if (SM_forceStopped == true)
    {
        Util_setEvent(&Sensor_events, SENSOR_DISASSOC_EVT);
    }

#ifdef USE_DMM
    DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_UNINIT);
    RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_FAIL);
#endif /* USE_DMM */
}

/*!
 * @brief      Security manager success processing function
 */
void smSuccessCMProcessCb(ApiMac_deviceDescriptor_t *devInfo, bool keyRefreshment)
{
    /* restore, write back current Pib value for auto request attribute */
    ApiMac_mlmeSetReqBool(ApiMac_attribute_autoRequest, currAutoReq);

#ifdef USE_DMM
    DMMPolicy_updateApplicationState(DMMPolicy_StackRole_154Sensor, DMMPOLICY_154_CONNECTED);
    RemoteDisplay_updateSmState(SMCOMMISSIONSTATE_SUCCESS);
#endif
}

#endif /* FEATURE_SECURE_COMMISSIONING */

#if (USE_DMM) && !(DMM_CENTRAL)
/** @brief  Set remote display callback functions
 *
 *  @param  remoteDisplayAttr  Remote display attribute value to set
 *  @param  value  pointer to data from remote dispaly application
 *  @param  len  length of data from remote display application
 */
static void setRDAttrCb(RemoteDisplayAttr_t remoteDisplayAttr,
    void *const value, uint8_t len)
{
    switch(remoteDisplayAttr)
    {
        case RemoteDisplayAttr_ReportInterval:
        {
            uint8_t *byteArr = (uint8_t*)value;
            uint32_t reportingInterval;
            uint32_t randomNum;

            reportingInterval = Util_buildUint32(byteArr[3], byteArr[2], byteArr[1], byteArr[0]);
            configSettings.reportingInterval = reportingInterval;

            randomNum = ((ApiMac_randomByte() << 16) +
                         (ApiMac_randomByte() << 8) + ApiMac_randomByte());
            randomNum = (randomNum % reportingInterval) +
                    SENSOR_MIN_POLL_TIME;

            Ssf_setReadingClock(randomNum);

            break;
        }
        case RemoteDisplayAttr_CollLed:
        {
            uint8_t cmdBytes[SMSGS_INDENTIFY_LED_REQUEST_MSG_LEN];

            /* send the response message directly */
            cmdBytes[0] = (uint8_t) Smsgs_cmdIds_IdentifyLedReq;
            cmdBytes[1] = *((uint8_t*)value);
            Sensor_sendMsg(Smsgs_cmdIds_IdentifyLedReq,
                    &collectorAddr, true,
                    SMSGS_INDENTIFY_LED_REQUEST_MSG_LEN,
                    cmdBytes);

            break;
        }
        default:
            return;
    }
}

/** @brief  Get remote display callback functions
 *
 *  @param  remoteDisplayAttr  Remote display attribute value to set
 *
 *  @return  void *  Current value of data present in 15.4 application
 */
static void getRDAttrCb(RemoteDisplayAttr_t remoteDisplayAttr, void *value, uint8_t len)
{
    switch(remoteDisplayAttr)
    {
        case RemoteDisplayAttr_ReportInterval:
        {
            Util_bufferUint32(value, configSettings.reportingInterval);
            // Fix byte order
            swapBytes((uint8_t *)value, len);
            break;
        }
        case RemoteDisplayAttr_CollLed:
        {
            break;
        }
        case RemoteDisplayAttr_SensorData:
        {
#if defined(TEMP_SENSOR) && !defined(POWER_MEAS)
            memcpy(value, &tempSensor.ambienceTemp, len);
#endif
            break;
        }
        default:
            // Attribute not found
            break;
        }
}



/** @brief  Set provisioning callback functions
 *
 *  @param  ProvisionAttr_t  Remote display attribute value to set
 *  @param  value  pointer to data from remote dispaly application
 *  @param  len  length of data from remote display application
 */
static void setProvisioningCb(ProvisionAttr_t provisioningAttr,
    void *const value, uint8_t len)
{
    uint8_t *byteArr = (uint8_t *)value;

    switch(provisioningAttr)
    {
        case ProvisionAttr_PanId:
        {
            uint16_t newPanId;

            newPanId = Util_buildUint16(byteArr[1], byteArr[0]);
            Jdllc_setJoiningPanId(newPanId);
            break;
        }
        case ProvisionAttr_SensorChannelMask:
        {
#ifndef CUI_DISABLE
            // Validate and correct the channel mask received over BLE
            for (uint8_t i = 0; i < APIMAC_154G_CHANNEL_BITMAP_SIZ; i++)
            {
                Ssf_validateChMask(byteArr, i);
            }
#endif /* CUI_DISABLE */

            Jdllc_setChanMask(byteArr);
#if CONFIG_FH_ENABLE
            Jdllc_setAsyncChanMask(byteArr);
#endif
            break;
        }
        case ProvisionAttr_NtwkKey:
        {
#ifdef FEATURE_MAC_SECURITY
            Jdllc_setDefaultKey(byteArr);
#endif
            break;
        }
        default:
            return;
        }
}

/** @brief  Get provisioning callback functions
 *
 *  @param  ProvisionAttr_t  Remote display attribute value to set
 *
 *  @return  uint8_t  Current value of data present in 15.4 application
 */
static void getProvisioningCb(ProvisionAttr_t provisioningAttr, void *value, uint8_t len)
{
    switch(provisioningAttr)
    {
        case ProvisionAttr_PanId:
        {
            if(len == 2)
            {
                uint16_t panId;
                uint8_t tmpByteSwap;

                Jdllc_getJoiningPanId(&panId);

                Util_bufferUint16(value, panId);

                // Switch byte order
                tmpByteSwap = ((uint8_t*)value)[0];
                ((uint8_t *)value)[0] = ((uint8_t *)value)[1];
                ((uint8_t *)value)[1] = tmpByteSwap;
            }
            break;
        }
        case ProvisionAttr_Freq:
        {
            *(uint8_t*)value = Jdllc_getFreq();
            break;
        }
        case ProvisionAttr_SensorChannelMask:
        {
            Jdllc_getChanMask((uint8_t *)value);
            break;
        }
        case ProvisionAttr_FFDAddr:
        {
            Jdllc_getFfdAddr((uint8_t *)value);
            break;
        }
        case ProvisionAttr_NtwkKey:
        {
#ifdef FEATURE_MAC_SECURITY
            Jdllc_getDefaultKey((uint8_t *)value);
#endif
            break;
        }
        case ProvisionAttr_ProvState:
        {
            *(uint8_t *)value = Jdllc_getProvState();
            break;
        }
        default:
            // Attribute not found
            break;
        }
}

/*!
 * @brief   Helper function to swap the byte order for an input array
 */
void swapBytes(uint8_t *arr, uint8_t len)
{
    uint8_t i,j, tmp;

    for (i = 0, j = len - 1; i < len/2 ; i++,j--)
    {
        tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}
#endif /* USE_DMM && !DMM_CENTRAL */

#ifdef DMM_OAD

/*********************************************************************
 * @fn      Sensor_dmmPausePolicyCb
 *
 * @brief   DMM Policy callback to pause 154 Stack
 */
static void Sensor_dmmPausePolicyCb(uint16_t pause)
{
    if (pause)
    {   //pause 154sensor
        Util_setEvent(&Sensor_events, SENSOR_PAUSE_EVT);
    }
    else
    {   //unpause 154sensor
        Util_setEvent(&Sensor_events, SENSOR_RESUME_EVT);
    }

    /* Wake up the application thread when it waits for clock event */
    Ssf_PostAppSem();
}

#endif

#ifdef LPSTK
/*********************************************************************
 * @fn      lpstkAccelerometerTiltCb
 *
 * @brief   SENSOR CONTROLLER Accelerometer CB when Tilt is detected
 */
static void lpstkAccelerometerTiltCb(void)
{
    Lpstk_Accelerometer accel;
    uint16_t tempFrameCtrl = configSettings.frameControl;
    Lpstk_getAccelerometer(&accel);
    accelerometerSensor.xAxis = accel.x;
    accelerometerSensor.yAxis = accel.y;
    accelerometerSensor.zAxis = accel.z;
    accelerometerSensor.xTiltDet = accel.xTiltDet;
    accelerometerSensor.yTiltDet = accel.yTiltDet;
    configSettings.frameControl = Smsgs_dataFields_accelSensor;
    processSensorMsgEvt();
    configSettings.frameControl = tempFrameCtrl;
}
#endif /* LPSTK */

#ifdef DMM_CENTRAL
/*********************************************************************
 * @fn      Sensor_forwardBleData
 *
 * @brief   Forward BLE data to collector
 */
void Sensor_forwardBleData(const Smsgs_bleSensorField_t *bleInfo)
{
    // Update sensor data to be sent
    memcpy(&bleSensor, bleInfo, sizeof(Smsgs_bleSensorField_t));

    // If connected forward BLE sensor data
    if ((Jdllc_getProvState() == Jdllc_states_joined) ||
        (Jdllc_getProvState() == Jdllc_states_rejoined))
    {
        // Ensure Frame Control is set as Config Messages set frame control to default
        configSettings.frameControl |= Smsgs_dataFields_bleSensor;
        processSensorMsgEvt();
    }
}
#endif
