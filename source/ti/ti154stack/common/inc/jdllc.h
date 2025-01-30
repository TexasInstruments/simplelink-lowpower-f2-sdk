/******************************************************************************

 @file jdllc.h

 @brief Joining Device Logical Link Controller

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
#ifndef JDLLC_H
#define JDLLC_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include "llc.h"
#include "ti_154stack_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(STATIC)
#if defined(UNIT_TEST)
/*! Allow access to the local variables to test code */
#define STATIC
#else
/*! Define STATIC as static for local variables */
#define STATIC static
#endif
#endif

/*! sensor minimum poll time = 10 ms */
#define SENSOR_MIN_POLL_TIME        10

/*! Event ID - LLC Event for PA Async command */
#define JDLLC_PAS_EVT            0x0001
/*! Event ID - LLC Event for PC Async command */
#define JDLLC_PCS_EVT            0x0002
/*! Event ID - LLC Event for sending association request */
#define JDLLC_JOIN_EVT           0x0004
/*! Event ID - State change event */
#define JDLLC_STATE_CHANGE_EVT   0x0008
/*! Event ID - Poll event */
#define JDLLC_POLL_EVT           0x0010
/*! Event ID - Associate Request */
#define JDLLC_ASSOCIATE_REQ_EVT  0x0020
/*! Event ID - Process coordinator realignment */
#define JDLLC_COORD_REALIGN      0x0040
/*! Event ID - Scan backoff , silent period before resuming scanning */
#define JDLLC_SCAN_BACKOFF       0x0100

 /*!
 Joining Device State Values
*/
typedef enum
{
    /*! Powered up, not started and waiting for user to start */
    Jdllc_states_initWaiting,
    /*! Starting device: scanning and selecting the best network to join */
    Jdllc_states_joining,
    /*! Powered up, found network information, and restoring device in network*/
    Jdllc_states_initRestoring,
    /*! Device is operating in network */
    Jdllc_states_joined,
    /*! Device is restored as device in the network */
    Jdllc_states_rejoined,
    /*! Device is orphaned */
    Jdllc_states_orphan,
    /*! Device join access denied or PAN at capacity */
    Jdllc_states_accessDenied
} Jdllc_states_t;

/*!
 Joining Device starting State Values
 */
typedef enum
{
    /*! Joining Device is performing a active scan  */
    Jdllc_deviceStates_scanActive,
    /*! Joining Device is performing a passive scan  */
    Jdllc_deviceStates_scanPassive,
    /*! Sync request for beacon enabled device*/
    Jdllc_deviceStates_syncReq,
    /*! Orphan scan */
    Jdllc_deviceStates_scanOrphan,
    /*! Scan backoff state */
    Jdllc_deviceStates_scanBackoff,
    /*! Device stops joining as PAN reaches Capacity */
    Jdllc_deviceStates_accessDenied
} Jdllc_device_states_t;

/*! Jdllc statistics */
typedef struct
{
    /*! Number of PAN Advertisement Solicits sent */
    int32_t fhNumPASolicitSent;
    /*! Number of PAN Advertisement received */
    int32_t fhNumPAReceived;
    /*! Number of PAN Config Solicits sent */
    int32_t fhNumPANConfigSolicitsSent;
    /*! Number of PAN Config received */
    int32_t fhNumPANConfigReceived;
    /*! Other statistics */
    int32_t otherStats;
} Jdllc_statistics_t;

extern Jdllc_statistics_t Jdllc_statistics;
extern uint16_t Jdllc_events;

/******************************************************************************
 Structures - Building blocks for the JDLLC
 *****************************************************************************/

/*!
 Device joined callback - passes the device information and its parent's
  information to app
 */
typedef void (*Jdllc_networkJoinedFp_t)(ApiMac_deviceDescriptor_t *pDevInfo,
                Llc_netInfo_t  *pParentInfo);
/*!
  Disassociation indication callback - passes reason for device disassociation
  or parent disassociating from device to app
 */
typedef void (*Jdllc_networkDisassocIndFp_t)(ApiMac_sAddrExt_t *extAddress,
                ApiMac_disassocateReason_t reason);

/*!
  Disassociation confirm callback - passes status of device's disassociation
  request to app
 */
typedef void (*Jdllc_networkDisassocCnfFp_t)(ApiMac_sAddrExt_t *extAddress,
                ApiMac_status_t status);

/*!
 State Changed callback - This indication says that the JDLLC's state
 has changed.
 */
typedef void (*Jdllc_stateChangedFp_t)(Jdllc_states_t state);

typedef struct _Jdllc_callbacks_t
{
    /*! Network Joined Indication callback */
    Jdllc_networkJoinedFp_t pJoinedCb;
    /* Disassociation Indication callback */
    Jdllc_networkDisassocIndFp_t pDisassocIndCb;
    /*! Disassociation Confirm callback */
    Jdllc_networkDisassocCnfFp_t pDisassocCnfCb;
    /*! State Changed indication callback */
    Jdllc_stateChangedFp_t pStateChangeCb;
} Jdllc_callbacks_t;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief       Initialize this module and update the callback structure.
 *              <BR>
 *              This function will take the application's MAC callback
 *              structure and override some callbacks with its own callbacks.
 *              The application callbacks that are overridden are saved so that
 *              they can be chain called when processing the callbacks.
 *
 * @param       pMacCbs - pointer to the API MAC callback structure
 * @param       pJdllcCbs - pointer to the JDLLC callback structure
 */
extern void Jdllc_init(ApiMac_callbacks_t *pMacCbs,
                   Jdllc_callbacks_t *pJdllcCbs);

/*!
 * @brief Application task processing.
 */
extern void Jdllc_process(void);

/*!
 * @brief       set the PAN ID to join.
 *
 * @param       panId - PAN ID to set
 */
extern void Jdllc_setJoiningPanId(uint16_t panId);

/*!
 * @brief       get the PAN ID to join.
 *
 * @param       panId - Pointer to current PAN ID in jdllc
 */
extern void Jdllc_getJoiningPanId(uint16_t *pPanId);

/*!
 * @brief       set the channel mask.
 *
 * @param       chanMask - channel mask to set
 */
extern void Jdllc_setChanMask(uint8_t *chanMask);

/*!
 * @brief       get the channel mask.
 *
 * @param       chanMask - Pointer to current channel mask in jdllc
 */
extern void Jdllc_getChanMask(uint8_t *chanMask);

/*!
 * @brief       set the async channel mask.
 *
 * @param       asyncChanMask - async channel mask to set
 */
extern void Jdllc_setAsyncChanMask(uint8_t *asyncChanMask);

/*!
 * @brief       get the async channel mask.
 *
 * @param       asyncChanMask - async channel mask to set
 */
extern void Jdllc_getAsyncChanMask(uint8_t *asyncChanMask);

#ifdef FEATURE_MAC_SECURITY
/*!
 * @brief       set the default security key.
 *
 * @param       key - default key to set
 */
extern void Jdllc_setDefaultKey(uint8_t *key);

/*!
 * @brief       get the default security key.
 *
 * @param       key - default key to get
 */
extern void Jdllc_getDefaultKey(uint8_t *key);
#endif

/*!
 * @brief       Get the collector (Full Function Device - FFD) address.
 *
 * @param       addr - FFD address from jdllc
 */
extern void Jdllc_getFfdAddr(uint8_t *addr);

/*!
 * @brief       Get the current PHY ID of the sensor device.
 *
 * @return       frequency - Current MAC PHY ID
 */
extern uint8_t Jdllc_getFreq(void);

/*!
 * @brief       Get the current channel of the sensor device
 *
 * @return      channel - Current MAC channel
 */
extern uint8_t Jdllc_getChan(void);

/*!
 * @brief       Get the current state of the sensor device.
 *
 * @return       state - Sensor state
 */
extern uint8_t Jdllc_getProvState(void);

/*!
 * @brief       Get the previous state of the sensor device.
 *
 * @return       state - Sensor state
 */
extern uint8_t Jdllc_getPrevProvState(void);

/* @brief       Start the joining process.
 *              <BR>
 *              The application will call this function to start the process
 *              of joining for this device a new network.
 *              <BR>
 *              This module will automatically determine the best channel(s)
 *              and PANID, before starting.
 */
extern void Jdllc_join(void);

/*!
 * @brief       Restore the device in the network.
 *              <BR>
 *              The application will call this function to restore the
 *              device in the network by passing all the network
 *              information needed to restore the device.
 *              <BR>
 *              This module will configure the MAC with all the network
 *              information then start the device without scanning.
 *
 * @param       pDevInfo - pointer to device descriptor for the end device
 * @param       pParentInfo - pointer to network information for the device's
 *              parent
 */
extern void Jdllc_rejoin(ApiMac_deviceDescriptor_t *pDevInfo,
                         Llc_netInfo_t *pParentInfo);
/*!
 * @brief       API for app to set the poll interval.
 *              <BR>
 *              The application will call this function to set the poll
 *              interval received in config message
 *
 * @param       pollInterval - poll interval in milliseconds
 */
extern void Jdllc_setPollRate(uint32_t pollInterval);

/*!
 * @brief       API for app to set disassociation request.
 */
extern void Jdllc_sendDisassociationRequest(void);

/*!
 * @brief       Initialize the MAC Security
 *
 * @param       frameCounter - Initial frame counter
 */
extern void Jdllc_securityInit(uint32_t frameCounter, uint8_t *key);

/*!
 * @brief       Fill in the security structure
 *
 * @param       pSec - pointer to security structure
 */
extern void Jdllc_securityFill(ApiMac_sec_t *pSec);

/*!
 * @brief       Check the security level against expected level
 *
 * @param       pSec - pointer to security structure
 *
 * @return      true is matches expected security level, false if not
 */
extern bool Jdllc_securityCheck(ApiMac_sec_t *pSec);

/*!
 * @brief      Add a device to the MAC security device table.
 *
 * @param      panID - PAN ID
 * @param      shortAddr - short address of the device
 * @param      pExtAddr - pointer to the extended address
 * @param      frameCounter - starting frame counter
 *
 * @return     status returned by ApiMac_secAddDevice()
 */
extern ApiMac_status_t Jdllc_addSecDevice(uint16_t panID,
                                          uint16_t shortAddr,
                                          ApiMac_sAddrExt_t *pExtAddr,
                                          uint32_t frameCounter);

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* JDLLC_H */

