/******************************************************************************

 @file collector.h

 @brief TIMAC 2.0 Collector Example Application Header

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
#ifndef COLLECTOR_H
#define COLLECTOR_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "ti_154stack_config.h"
#include "api_mac.h"

#ifdef OSAL_PORT2TIRTOS
#ifndef FREERTOS_SUPPORT
#include <ti/sysbios/knl/Task.h>
#endif
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
#include "sm_ti154.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif


/******************************************************************************
 Constants and definitions
 *****************************************************************************/
#ifdef IEEE_COEX_TEST
#define APS_RETRY_INTERVAL       (50)
#define APS_INTERVAL             (10)
#define APS_MAX_FRAME_RETRIES    (2)
#endif // IEEE_COEX_TEST
/*! Event ID - Start the device in the network */
#define COLLECTOR_START_EVT               0x0001
/*! Event ID - Tracking Timeout Event */
#define COLLECTOR_TRACKING_TIMEOUT_EVT    0x0002
/*! Event ID - Generate Configs Event */
#define COLLECTOR_CONFIG_EVT              0x0004
#define COLLECTOR_BROADCAST_TIMEOUT_EVT   0x0008

/*! Event ID - Start Provisioning Event */
#define COLLECTOR_PROV_EVT                0x0010
/*! Event ID - Open Network Event */
#define COLLECTOR_OPEN_NWK_EVT            0x0040
/*! Event ID - Close Network Event */
#define COLLECTOR_CLOSE_NWK_EVT           0x0080

#ifdef DMM_OAD
/*! Event ID - Pause 154 Collector */
#define COLLECTOR_PAUSE_EVT               0x0100
/*! Event ID - Resume 154 Collector */
#define COLLECTOR_RESUME_EVT              0x0200
#endif

#ifdef LPSTK
/* LPSTK configuration frame control */
#define CONFIG_FRAME_CONTROL (Smsgs_dataFields_tempSensor | \
                              Smsgs_dataFields_lightSensor | \
                              Smsgs_dataFields_humiditySensor | \
                              Smsgs_dataFields_msgStats | \
                              Smsgs_dataFields_configSettings | \
                              Smsgs_dataFields_hallEffectSensor | \
                              Smsgs_dataFields_accelSensor)
#else
/* Default configuration frame control */
#define CONFIG_FRAME_CONTROL (Smsgs_dataFields_tempSensor | \
                              Smsgs_dataFields_lightSensor | \
                              Smsgs_dataFields_humiditySensor | \
                              Smsgs_dataFields_msgStats | \
                              Smsgs_dataFields_configSettings)
#endif
/*! Collector Status Values */
typedef enum
{
    /*! Success */
    Collector_status_success = 0,
    /*! Device Not Found */
    Collector_status_deviceNotFound = 1,
    /*! Collector isn't in the correct state to send a message */
    Collector_status_invalid_state = 2
} Collector_status_t;

/* Beacon order for non beacon network */
#define NON_BEACON_ORDER      15

/* Number of superframe periods to hold a indirect packet at collector for
Sensor to poll and get the frame*/
#define BCN_MODE_INDIRECT_PERSISTENT_TIME 3

#define MIN_PERSISTENCE_TIME_USEC 2000000

#ifndef MAX
#define MAX(n,m)   (((n) < (m)) ? (m) : (n))
#endif

#ifndef MIN
#define MIN(n,m)   (((n) < (m)) ? (n) : (m))
#endif


#if (CONFIG_PHY_ID == APIMAC_CUSTOM_PHY_ID)
/* MAC Indirevt Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME (MIN((MAX((5 * 1000 * CONFIG_POLLING_INTERVAL / 2), MIN_PERSISTENCE_TIME_USEC)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_CUSTOM)), 65535))

#elif  ((CONFIG_PHY_ID >= APIMAC_MRFSK_STD_PHY_ID_BEGIN) && (CONFIG_PHY_ID <= APIMAC_MRFSK_GENERIC_PHY_ID_BEGIN))

/* MAC Indirect Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME (MIN((MAX((5 * 1000 * CONFIG_POLLING_INTERVAL / 2), MIN_PERSISTENCE_TIME_USEC)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_50_kbps)), 65535))

#elif ((CONFIG_PHY_ID >= APIMAC_200KBPS_915MHZ_PHY_132) && (CONFIG_PHY_ID <= APIMAC_200KBPS_868MHZ_PHY_133)) || \
       (CONFIG_PHY_ID == APIMAC_200KBPS_920MHZ_PHY_136)

/* MAC Indirect Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME (MIN((MAX((5 * 1000 * CONFIG_POLLING_INTERVAL / 2),MIN_PERSISTENCE_TIME_USEC) / \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_200_kbps)), 65535))

#elif (CONFIG_PHY_ID == APIMAC_250KBPS_IEEE_PHY_0)

/* MAC Indirect Persistent Timeout */
#ifdef FEATURE_SECURE_COMMISSIONING
/* Increase the persistence time by a factor of 10 */
#define INDIRECT_PERSISTENT_TIME (MIN((MAX((5* 10 * 1000 * CONFIG_POLLING_INTERVAL / 2), MIN_PERSISTENCE_TIME_USEC)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_250_kbps)), 65535))
#else
#define INDIRECT_PERSISTENT_TIME (MIN((MAX((5 * 1000 * CONFIG_POLLING_INTERVAL / 2), MIN_PERSISTENCE_TIME_USEC)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                   SYMBOL_DURATION_250_kbps)), 65535))
#endif

#else
/* MAC Indirect Persistent Timeout */
#define INDIRECT_PERSISTENT_TIME (MIN((MAX((5 * 1000 * CONFIG_POLLING_INTERVAL / 2), MIN_PERSISTENCE_TIME_USEC)/ \
                                  (BASE_SUPER_FRAME_DURATION * \
                                    SYMBOL_DURATION_LRM)), 65535))
#endif

/******************************************************************************
 Structures
 *****************************************************************************/

/*! Collector Statistics */
typedef struct
{
    /*!
     Total number of tracking request messages attempted
     */
    uint32_t trackingRequestAttempts;
    /*!
     Total number of tracking request messages sent
     */
    uint32_t trackingReqRequestSent;
    /*!
     Total number of tracking response messages received
     */
    uint32_t trackingResponseReceived;
    /*!
     Total number of config request messages attempted
     */
    uint32_t configRequestAttempts;
    /*!
     Total number of config request messages sent
     */
    uint32_t configReqRequestSent;
    /*!
     Total number of config response messages received
     */
    uint32_t configResponseReceived;
    /*!
     Total number of sensor messages received
     */
    uint32_t sensorMessagesReceived;
    /*!
     Total number of failed messages because of channel access failure
     */
    uint32_t channelAccessFailures;
    /*!
     Total number of failed messages because of ACKs not received
     */
    uint32_t ackFailures;
    /*!
     Total number of failed transmit messages that are not channel access
     failure and not ACK failures
     */
    uint32_t otherTxFailures;
    /*! Total number of RX Decrypt failures. */
    uint32_t rxDecryptFailures;
    /*! Total number of TX Encrypt failures. */
    uint32_t txEncryptFailures;
    /* Total Transaction Expired Count */
    uint32_t txTransactionExpired;
    /* Total transaction Overflow error */
    uint32_t txTransactionOverflow;
    /* Total broadcast messages sent */
    uint16_t broadcastMsgSentCnt;
} Collector_statistics_t;

#ifdef IEEE_COEX_METRICS
typedef struct
{
    uint32_t dbgCoexGrants;
    uint32_t dbgCoexRejects;
    uint16_t dbgCoexContRejects;
    uint16_t dbgCoexMaxContRejects;
} Collector_coexStatistics_t;

#endif

/******************************************************************************
 Global Variables
 *****************************************************************************/

/*! Collector events flags */
extern uint16_t Collector_events;

/*! Collector statistics */
extern Collector_statistics_t Collector_statistics;

extern ApiMac_callbacks_t Collector_macCallbacks;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief Initialize this application.
 */
#ifdef OSAL_PORT2TIRTOS
extern void Collector_init(uint8_t _macTaskId);
#else
extern void Collector_init(void);
#endif
/*!
 * @brief Application task processing.
 */
extern void Collector_process(void);

/*!
 * @brief Build and send the configuration message to a device.
 *
 * @param pDstAddr - destination address of the device to send the message
 * @param frameControl - configure what to the device is to report back.
 *                       Ref. Smsgs_dataFields_t.
 * @param reportingInterval - in milliseconds- how often to report, 0
 *                            means to turn off automated reporting, but will
 *                            force the sensor device to send the Sensor Data
 *                            message once.
 * @param pollingInterval - in milliseconds- how often to the device is to
 *                          poll its parent for data (for sleeping devices
 *                          only.
 *
 * @return Collector_status_success, Collector_status_invalid_state
 *         or Collector_status_deviceNotFound
 */
extern Collector_status_t Collector_sendConfigRequest(ApiMac_sAddr_t *pDstAddr,
                uint16_t frameControl,
                uint32_t reportingInterval,
                uint32_t pollingInterval);

/*!
 * @brief Update the collector statistics
 */
extern void Collector_updateStats( void );

/*!
 * @brief Build and send the toggle led message to a device.
 *
 * @param pDstAddr - destination address of the device to send the message
 *
 * @return Collector_status_success, Collector_status_invalid_state
 *         or Collector_status_deviceNotFound
 */
extern Collector_status_t Collector_sendToggleLedRequest(
                ApiMac_sAddr_t *pDstAddr);

#if defined(DEVICE_TYPE_MSG)
/*!
 * @brief Build and send the device type request message to a device.
 *
 * @param pDstAddr - destination address of the device to send the message
 *
 * @return Collector_status_success, Collector_status_invalid_state
 *         or Collector_status_deviceNotFound
 */
extern Collector_status_t Collector_sendDeviceTypeRequest(
        ApiMac_sAddr_t *pDstAddr);
#endif /* DEVICE_TYPE_MSG */

#ifdef POWER_MEAS
/*! Generate data of fixed size for power measurement testing */
extern void generateIndirectRampMsg(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* COLLECTOR_H */
