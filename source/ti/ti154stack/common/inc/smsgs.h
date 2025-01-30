/******************************************************************************

 @file smsgs.h

 @brief Data Structures for the sensor messages sent over the air.

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
#ifndef SMGSS_H
#define SMGSS_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "ti_154stack_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 \defgroup Sensor Over-the-air Messages
 <BR>
 This header file defines the over-the-air messages between the collector
 and the sensor applications.
 <BR>
 Each field in these message are formatted low byte first, so a 16 bit field
 with a value of 0x1516 will be sent as 0x16, 0x15.
 <BR>
 The first byte of each message (data portion) contains the command ID (@ref
 Smsgs_cmdIds).
 <BR>
 The <b>Configuration Request Message</b> is defined as:
     - Command ID - [Smsgs_cmdIds_configReq](@ref Smsgs_cmdIds) (1 byte)
     - Frame Control field - Smsgs_dataFields (16 bits) - tells the sensor
     what to report in the Sensor Data Message.
     - Reporting Interval - in millseconds (32 bits) - how often to report, 0
     means to turn off automated reporting, but will force the sensor device
     to send the Sensor Data message once.
     - Polling Interval - in millseconds (32 bits) - If the sensor device is
     a sleep device, this tells the device how often to poll its parent for
     data.
 <BR>
 The <b>Configuration Response Message</b> is defined as:
     - Command ID - [Smsgs_cmdIds_configRsp](@ref Smsgs_cmdIds) (1 byte)
     - Status field - Smsgs_statusValues (16 bits) - status of the
     configuration request.
     - Frame Control field - Smsgs_dataFields (16 bits) - tells the collector
     what fields are supported by this device (this only includes the bits set
     in the request message).
     - Reporting Interval - in millseconds (32 bits) - how often to report, 0
     means to turn off reporting.
     - Polling Interval - in millseconds (32 bits) - If the sensor device is
     a sleep device, this tells how often this device will poll its parent.
     A value of 0 means that the device doesn't sleep.
 <BR>
The <b>Sensor Ramp Data Message</b> is defined as:
     - Command ID - [Smsgs_cmdIds_rampdata](@ref Smsgs_cmdIds) (1 byte)     
     - Data Fields - Variable length ramp data of size SENSOR_TEST_RAMP_DATA_SIZE
 <BR>
 The <b>Sensor Data Message</b> is defined as:
     - Command ID - [Smsgs_cmdIds_sensorData](@ref Smsgs_cmdIds) (1 byte)
     - Frame Control field - Smsgs_dataFields (16 bits) - tells the collector
     what fields are included in this message.
     - Data Fields - The length of this field is determined by what data fields
     are included.  The order of the data fields are determined by the bit
     position of the Frame Control field (low bit first).  For example, if the
     frame control field has Smsgs_dataFields_tempSensor and
     Smsgs_dataFields_lightSensor set, then the Temp Sensor field is first,
     followed by the light sensor field.
 <BR>
 The <b>Temp Sensor Field</b> is defined as:
    - Ambience Chip Temperature - (int16_t) - each value represents signed
      integer part of temperature in Deg C (-256 .. +255)
    - Object Temperature - (int16_t) -  each value represents signed
      integer part of temperature in Deg C (-256 .. +255)
 <BR>
 The <b>Light Sensor Field</b> is defined as:
    - Raw Sensor Data - (uint16_6) raw data read out of the OPT2001 light
    sensor.
 <BR>
 The <b>Humidity Sensor Field</b> is defined as:
    - Raw Temp Sensor Data - (uint16_t) - raw temperature data from the
    Texas Instruments HCD1000 humidity sensor.
    - Raw Humidity Sensor Data - (uint16_t) - raw humidity data from the
    Texas Instruments HCD1000 humidity sensor.
 <BR>
 The <b>Message Statistics Field</b> is defined as:
     - joinAttempts - uint16_t - total number of join attempts (associate
     request sent)
     - joinFails - uint16_t - total number of join attempts failed
     - msgsAttempted - uint16_t - total number of sensor data messages
     attempted.
     - msgsSent - uint16_t - total number of sensor data messages successfully
     sent (OTA).
     - trackingRequests - uint16_t - total number of tracking requests received.
     - trackingResponseAttempts - uint16_t - total number of tracking response
     attempted
     - trackingResponseSent - uint16_t - total number of tracking response
     success
     - configRequests - uint16_t - total number of config requests received.
     - configResponseAttempts - uint16_t - total number of config response
     attempted
     - configResponseSent - uint16_t - total number of config response
     success
     - channelAccessFailures - uint16_t - total number of Channel Access
     Failures.  These are indicated in MAC data confirms for MAC data requests.
     - macAckFailures - uint16_t - Total number of MAC ACK failures. These are
     indicated in MAC data confirms for MAC data requests.
     - otherDataRequestFailures - uint16_t - Total number of MAC data request
     failures, other than channel access failure or MAC ACK failures.
     - syncLossIndications - uint16_t - Total number of sync loss failures
     received for sleepy devices.
     - rxDecryptFailues - uint16_t - Total number of RX Decrypt failures.
     - txEncryptFailues - uint16_t - Total number of TX Encrypt failures.
     - resetCount - uint16_t - Total number of resets.
     - lastResetReason - uint16_t - 0 - no reason, 2 - HAL/ICALL,
     3 - MAC, 4 - TIRTOS
 <BR>
 The <b>Config Settings Field</b> is defined as:
     - Reporting Interval - in millseconds (32 bits) - how often to report, 0
     means reporting is off.
     - Polling Interval - in millseconds (32 bits) - If the sensor device is
     a sleep device, this states how often the device polls its parent for
     data. This field is 0 if the device doesn't sleep.
 */

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/*! Sensor Message Extended Address Length */
#define SMGS_SENSOR_EXTADDR_LEN 8

/*! Config Request message length (over-the-air length) */
#define SMSGS_CONFIG_REQUEST_MSG_LENGTH 11
/*! Config Response message length (over-the-air length) */
#define SMSGS_CONFIG_RESPONSE_MSG_LENGTH 13
/*! Tracking Request message length (over-the-air length) */
#define SMSGS_TRACKING_REQUEST_MSG_LENGTH 1
/*! Tracking Response message length (over-the-air length) */
#define SMSGS_TRACKING_RESPONSE_MSG_LENGTH 1
/*! Broadcast Command message length (over-the-air-length) */
#define SMSGS_BROADCAST_CMD_LENGTH  3

/*! Length of a sensor data message with no configured data fields */
#define SMSGS_BASIC_SENSOR_LEN (3 + SMGS_SENSOR_EXTADDR_LEN)
/*! Length of the tempSensor portion of the sensor data message */
#define SMSGS_SENSOR_TEMP_LEN 4
/*! Length of the lightSensor portion of the sensor data message */
#define SMSGS_SENSOR_LIGHT_LEN 2
/*! Length of the humiditySensor portion of the sensor data message */
#define SMSGS_SENSOR_HUMIDITY_LEN 4
/*! Length of the messageStatistics portion of the sensor data message */
#define SMSGS_SENSOR_MSG_STATS_LEN 44
/*! Length of the configSettings portion of the sensor data message */
#define SMSGS_SENSOR_CONFIG_SETTINGS_LEN 8
/*! Toggle Led Request message length (over-the-air length) */
#define SMSGS_TOGGLE_LED_REQUEST_MSG_LEN 1
/*! Toggle Led Request message length (over-the-air length) */
#define SMSGS_TOGGLE_LED_RESPONSE_MSG_LEN 2
/*! Identify Led Request message length (over-the-air length) */
#define SMSGS_INDENTIFY_LED_REQUEST_MSG_LEN 2
/*! Identify Led Response message length (over-the-air length) */
#define SMSGS_INDENTIFY_LED_RESPONSE_MSG_LEN 2
/*! Device type request message length (over-the-air length) */
#define SMSGS_DEVICE_TYPE_REQUEST_MSG_LEN 1
/*! Device type response message length (over-the-air length) */
#define SMSGS_DEVICE_TYPE_RESPONSE_MSG_LEN 3
/*! Length of a BLE Device Address */
#define B_ADDR_LEN 6
/*! Length of the ble sensor portion of the sensor data length not including variable data field */
#define SMSGS_SENSOR_BLE_LEN 5 + B_ADDR_LEN
/* Max BLE Data Length */
#define MAX_BLE_DATA_LEN 20

/*!
 Message IDs for Sensor data messages.  When sent over-the-air in a message,
 this field is one byte.
 */
 typedef enum
 {
    /*! Configuration message, sent from the collector to the sensor */
    Smsgs_cmdIds_configReq = 1,
    /*! Configuration Response message, sent from the sensor to the collector */
    Smsgs_cmdIds_configRsp = 2,
    /*! Tracking request message, sent from the the collector to the sensor */
    Smsgs_cmdIds_trackingReq = 3,
     /*! Tracking response message, sent from the sensor to the collector */
    Smsgs_cmdIds_trackingRsp = 4,
    /*! Sensor data message, sent from the sensor to the collector */
    Smsgs_cmdIds_sensorData = 5,
    /* Toggle LED message, sent from the collector to the sensor */
    Smsgs_cmdIds_toggleLedReq = 6,
    /* Toggle LED response msg, sent from the sensor to the collector */
    Smsgs_cmdIds_toggleLedRsp = 7,
    /* new data type for ramp data */
    Smsgs_cmdIds_rampdata = 8,
    /*! OAD mesages, sent/received from both collector and sensor */
    Smsgs_cmdIds_oad = 9,
    /* Broadcast control msg, sent from the collector to the sensor */
    Smgs_cmdIds_broadcastCtrlMsg = 10,
    /* KEY Exchange msg, between collector and the sensor */
    Smgs_cmdIds_KeyExchangeMsg = 11,
    /* Identify LED request msg */
    Smsgs_cmdIds_IdentifyLedReq = 12,
    /* Identify LED response msg */
    Smsgs_cmdIds_IdentifyLedRsp = 13,
    /*! SM Commissioning start command sent from collector to the sensor */
    Smgs_cmdIds_CommissionStart  = 14,
    /*! SM Commissioning message sent bi-directionally between the collector and sensor */
    Smgs_cmdIds_CommissionMsg  = 15,
    /* Device type request msg */
    Smsgs_cmdIds_DeviceTypeReq = 16,
    /* Device type response msg */
    Smsgs_cmdIds_DeviceTypeRsp = 17

 } Smsgs_cmdIds_t;

/*!
 Frame Control field states what data fields are included in reported
 sensor data, each value is a bit mask value so that they can be combined
 (OR'd together) in a control field.
 When sent over-the-air in a message this field is 2 bytes.
 */
typedef enum
{
    /*! Temperature Sensor */
    Smsgs_dataFields_tempSensor = 0x0001,
    /*! Light Sensor */
    Smsgs_dataFields_lightSensor = 0x0002,
    /*! Humidity Sensor */
    Smsgs_dataFields_humiditySensor = 0x0004,
    /*! Message Statistics */
    Smsgs_dataFields_msgStats = 0x0008,
    /*! Config Settings */
    Smsgs_dataFields_configSettings = 0x0010,
#ifdef LPSTK
    /*! Hall Effect Sensor */
    Smsgs_dataFields_hallEffectSensor = 0x0020,
    /*! Accelerometer Sensor */
    Smsgs_dataFields_accelSensor = 0x0040,
#endif /* LPSTK */
    Smsgs_dataFields_bleSensor = 0x0080,
} Smsgs_dataFields_t;

/*!
 Status values for the over-the-air messages
 */
typedef enum
{
    /*! Success */
    Smsgs_statusValues_success = 0,
    /*! Message was invalid and ignored */
    Smsgs_statusValues_invalid = 1,
    /*!
     Config message was received but only some frame control fields
     can be sent or the reportingInterval or pollingInterval fail
     range checks.
     */
    Smsgs_statusValues_partialSuccess = 2,
} Smsgs_statusValues_t;

/******************************************************************************
 Structures - Building blocks for the over-the-air sensor messages
 *****************************************************************************/

/*!
 Configuration Request message: sent from controller to the sensor.
 */
typedef struct _Smsgs_configreqmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
    /*! Frame Control field - bit mask of Smsgs_dataFields */
    uint16_t frameControl;
    /*! Reporting Interval */
    uint32_t reportingInterval;
    /*! Polling Interval */
    uint32_t pollingInterval;
} Smsgs_configReqMsg_t;

/*!
 Configuration Response message: sent from the sensor to the collector
 in response to the Configuration Request message.
 */
typedef struct _Smsgs_configrspmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
    /*! Response Status - 2 bytes */
    Smsgs_statusValues_t status;
    /*! Frame Control field - 2 bytes - bit mask of Smsgs_dataFields */
    uint16_t frameControl;
    /*! Reporting Interval - 4 bytes */
    uint32_t reportingInterval;
    /*! Polling Interval - 4 bytes */
    uint32_t pollingInterval;
} Smsgs_configRspMsg_t;

/*!
 Tracking Request message: sent from controller to the sensor.
 */
typedef struct _Smsgs_trackingreqmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
} Smsgs_trackingReqMsg_t;

/*!
 Tracking Response message: sent from the sensor to the collector
 in response to the Tracking Request message.
 */
typedef struct _Smsgs_trackingrspmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
} Smsgs_trackingRspMsg_t;

/*!
 Toggle LED Request message: sent from controller to the sensor.
 */
typedef struct _Smsgs_toggleledreqmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
} Smsgs_toggleLedReqMsg_t;

/*!
 Toggle LED Response message: sent from the sensor to the collector
 in response to the Toggle LED Request message.
 */
typedef struct _Smsgs_toggleledrspmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
    /*! LED State - 0 is off, 1 is on - 1 byte */
    uint8_t ledState;
} Smsgs_toggleLedRspMsg_t;

/*!
 Identify LED Request message: sent from controller to the sensor.
 */
typedef struct _Smsgs_identifyledreqmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
    /*! time to identify in s */
    uint8_t identifyTime;
} Smsgs_identifyLedReqMsg_t;

typedef struct _Smsgs_identifyledrspmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
    /* status */
    uint8_t status;
} Smsgs_identifyLedRspMsg_t;

/*!
 Temp Sensor Field
 */
typedef struct _Smsgs_tempsensorfield_t
{
    /*!
     Ambience Chip Temperature - each value represents a 0.01 C
     degree, so a value of 2475 represents 24.75 C.
     */
    int16_t ambienceTemp;
    /*!
     Object Temperature - each value represents a 0.01 C
     degree, so a value of 2475 represents 24.75 C.
     */
    int16_t objectTemp;
} Smsgs_tempSensorField_t;

/*!
 Light Sensor Field
 */
typedef struct _Smsgs_lightsensorfield_t
{
    /*! Raw Sensor Data read out of the OPT2001 light sensor */
    uint16_t rawData;
} Smsgs_lightSensorField_t;

/*!
 Humidity Sensor Field
 */
typedef struct _Smsgs_humiditysensorfield_t
{
    /*! Raw Temp Sensor Data from the TI HCD1000 humidity sensor. */
    uint16_t temp;
    /*! Raw Humidity Sensor Data from the TI HCD1000 humidity sensor. */
    uint16_t humidity;
} Smsgs_humiditySensorField_t;

/*!
 Hall Effect Sensor Field
 */
typedef struct _Smsgs_halleffectsensorfield_t
{
    /*! Magnetic Flux Switch. */
    uint8_t fluxLevel;
} Smsgs_hallEffectSensorField_t;

/*!
 Accelerometer Sensor Field
 */
typedef struct _Smsgs_accelsensorfield_t
{
    /*! X axis accelerometer value. */
    int16_t xAxis;
    /*! Y axis accelerometer value. */
    int16_t yAxis;
    /*! Z axis accelerometer value. */
    int16_t zAxis;
    /*! Device tilted in the X axis. */
    uint8_t xTiltDet;
    /*! Device tilted in the Y axis. */
    uint8_t yTiltDet;
} Smsgs_accelSensorField_t;

typedef struct _Smsgs_blesensorfield_t
{
    /*! BLE Sensor Address */
    uint8_t  bleAddr[B_ADDR_LEN];
    /*! Manufacturer ID */
    uint16_t manFacID;
    /*! UUID */
    uint16_t uuid;
    /*! Length of BLE Char */
    uint8_t dataLength;
    /*! Pointer to BLE Characteristic Value */
    uint8_t data[MAX_BLE_DATA_LEN];

} Smsgs_bleSensorField_t;

/*!
 Message Statistics Field
 */
typedef struct _Smsgs_msgstatsfield_t
{
    /*! total number of join attempts (associate request sent) */
    uint16_t joinAttempts;
    /*! total number of join attempts failed */
    uint16_t joinFails;
    /*! total number of sensor data messages attempted. */
    uint16_t msgsAttempted;
    /*! total number of sensor data messages sent. */
    uint16_t msgsSent;
    /*! total number of tracking requests received */
    uint16_t trackingRequests;
    /*! total number of tracking response attempted */
    uint16_t trackingResponseAttempts;
    /*! total number of tracking response success */
    uint16_t trackingResponseSent;
    /*! total number of config requests received */
    uint16_t configRequests;
    /*! total number of config response attempted */
    uint16_t configResponseAttempts;
    /*! total number of config response success */
    uint16_t configResponseSent;
    /*!
     Total number of Channel Access Failures.  These are indicated in MAC data
     confirms for MAC data requests.
     */
    uint16_t channelAccessFailures;
    /*!
     Total number of MAC ACK failures. These are indicated in MAC data
     confirms for MAC data requests.
     */
    uint16_t macAckFailures;
    /*!
     Total number of MAC data request failures, other than channel access
     failure or MAC ACK failures.
     */
    uint16_t otherDataRequestFailures;
    /*! Total number of sync loss failures received for sleepy devices. */
    uint16_t syncLossIndications;
    /*! Total number of RX Decrypt failures. */
    uint16_t rxDecryptFailures;
    /*! Total number of TX Encrypt failures. */
    uint16_t txEncryptFailures;
    /*! Total number of resets. */
    uint16_t resetCount;
    /*!
     Assert reason for the last reset -  0 - no reason, 2 - HAL/ICALL,
     3 - MAC, 4 - TIRTOS
     */
    uint16_t lastResetReason;
    /*! Amount of time taken for node to join */
    uint16_t joinTime;
    /*! Max re-join delay */
    uint16_t interimDelay;
    /*!
     Number of broadcast messages received from the collector
     */
    uint16_t numBroadcastMsgRcvd;
    /*!
    Number of broadcast messages missed from the collector
    */
    uint16_t numBroadcastMsglost;
    /*!
    Average end to end delay
    */
    uint16_t avgE2EDelay;
    /*!
    Worst Case end to end delay
    */
    uint16_t worstCaseE2EDelay;
} Smsgs_msgStatsField_t;

#ifdef POWER_MEAS
/*!
 Power Meas Statistics Field
 */
typedef struct _Smsgs_powerMeastatsField_t
{
    /*! total number of polls sent. Not applicable for beacon mode */
    uint16_t pollRequestsSent;
    /*! total number of collector ramp data received */
    uint16_t rampDataRcvd;
} Smsgs_powerMeastatsField_t;
#endif

/*!
 Message Statistics Field
 */
typedef struct _Smsgs_configsettingsfield_t
{
    /*!
     Reporting Interval - in millseconds, how often to report, 0
     means reporting is off
     */
    uint32_t reportingInterval;
    /*!
     Polling Interval - in millseconds (32 bits) - If the sensor device is
     a sleep device, this states how often the device polls its parent for
     data. This field is 0 if the device doesn't sleep.
     */
    uint32_t pollingInterval;
} Smsgs_configSettingsField_t;

/*!
 Sensor Data message: sent from the sensor to the collector
 */
typedef struct _Smsgs_sensormsg_t
{
    /*! Command ID */
    Smsgs_cmdIds_t cmdId;
    /*! Extended Address */
    uint8_t extAddress[SMGS_SENSOR_EXTADDR_LEN];
    /*! Frame Control field - bit mask of Smsgs_dataFields */
    uint16_t frameControl;
    /*!
     Temp Sensor field - valid only if Smsgs_dataFields_tempSensor
     is set in frameControl.
     */
    Smsgs_tempSensorField_t tempSensor;
    /*!
     Light Sensor field - valid only if Smsgs_dataFields_lightSensor
     is set in frameControl.
     */
    Smsgs_lightSensorField_t lightSensor;
    /*!
     Humidity Sensor field - valid only if Smsgs_dataFields_humiditySensor
     is set in frameControl.
     */
    Smsgs_humiditySensorField_t humiditySensor;
    /*!
     Message Statistics field - valid only if Smsgs_dataFields_msgStats
     is set in frameControl.
     */
    Smsgs_msgStatsField_t msgStats;
    /*!
     Configuration Settings field - valid only if
     Smsgs_dataFields_configSettings is set in frameControl.
     */
    Smsgs_configSettingsField_t configSettings;
#ifdef LPSTK
    /*!
     Hall Effect Sensor field - valid only if Smsgs_dataFields_hallEffectSensor
     is set in frameControl.
     */
    Smsgs_hallEffectSensorField_t hallEffectSensor;
    /*!
     Accelerometer Sensor field - valid only if Smsgs_dataFields_accelSensor
     is set in frameControl.
     */
    Smsgs_accelSensorField_t accelerometerSensor;
#endif /* LPSTK */
    /*!
     BLE Sensor field - valid only if Smsgs_dataFields_bleSensorField_t
     is set in frameControl.
     */
    Smsgs_bleSensorField_t bleSensor;
} Smsgs_sensorMsg_t;

/*!
 Broadcast Cmd Request message: sent from controller to the sensor.
 */
typedef struct _Smsgs_broadcastcmdmsg_t
{
    /*! Command ID - 1 byte */
    Smsgs_cmdIds_t cmdId;
    uint16_t broadcastMsgId;
}Smsgs_broadcastcmdmsg_t;


#ifdef __cplusplus
}
#endif

#endif /* SMGSS_H */

