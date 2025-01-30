/******************************************************************************

 @file csf.h

 @brief Collector Specific Functions API

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
#ifndef CSF_H
#define CSF_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "llc.h"
#include "cllc.h"
#include "smsgs.h"

#ifdef __unix__
#include "csf_linux.h"
#elif !defined(CUI_DISABLE)
#include "cui.h"
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
#include "sm_ti154.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants
 *****************************************************************************/

/*! CSF Events - Key Event */
#define CSF_KEY_EVENT 0x0001
#define COLLECTOR_UI_INPUT_EVT            0x0002
#define COLLECTOR_SENSOR_ACTION_EVT       0x0004
#ifdef IEEE_COEX_METRICS
#define COEX_IEEE_METRICS_EVT             0x0008
#endif
#ifdef MAC_STATS
#define MAC_STATS_EVT                     0x0010
#endif

#define CSF_INVALID_SHORT_ADDR            0xFFFF
#define CSF_INVALID_SUBID                 0xFFFF

#define SENSOR_ACTION_TOGGLE              0
#define SENSOR_ACTION_SET_RPT_INT         1
#define SENSOR_ACTION_DISASSOC            2
#define SENSOR_ACTION_DEVICE_TYPE_REQ     3
#if defined(IEEE_COEX_TEST) && defined(COEX_MENU)
#define COEX_ACTION_SET_NUM_PACKET        4
#define COEX_ACTION_SET_PKT_SIZE          5
#define COEX_ACTION_SET_TRACK_INT         6
#define COEX_ACTION_START                 7
#define COEX_ACTION_END                   8
#endif // defined(IEEE_COEX_TEST) && defined(COEX_MENU)

/******************************************************************************
 Typedefs
 *****************************************************************************/

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief       The application calls this function during initialization.
 *
 * @param       sem - pointer to semaphore used by MAC API
 */
extern void Csf_init(void *sem);
/*!
 * @brief       The application must call this function periodically to
 *              process any events that this module needs to process.
 */
extern void Csf_processEvents(void);

#ifdef USE_DMM
 /*!
 * @brief       Key event handler function
 *
 * @param       keysPressed - keys that are pressed
 */
extern void Csf_processKeyChange(uint8_t keysPressed);
#endif

/*!
 * @brief       The application calls this function to retrieve the stored
 *              network information.  The stored network information was saved
 *              after starting a network.
 *              <BR>
 *              NOTE: If the "fh" is true you will need to free the
 *              pInfo->info.fhNetInfo.fhInfo.bcNumChans and
 *              pInfo->info.fhNetInfo.fhInfo.pUnicastChans buffers through
 *              Csf_free().
 *
 * @param       pInfo - pointer to network information structure
 *
 * @return      True if the network information is available
 */
extern bool Csf_getNetworkInformation(Llc_netInfo_t *pInfo);

/*!
 * @brief       The application calls this function to indicate that it has
 *              started or restored the device in a network.
 *
 *              The information will be saved and used to determine if a
 *              network was already started and should be restored instead
 *              of started.
 *
 * @param       restored - true if restored in network
 * @param       pNetworkInfo - network information structure
 */
extern void Csf_networkUpdate(bool restored, Llc_netInfo_t *pNetworkInfo);

/*!
 * @brief       The application calls this function to indicate that a device
 *              has joined the network.
 *
 *              The information will be saved.
 *
 * @param       pDevInfo - pointer to the device information
 * @param       capInfo - capability information of the joining device.
 *
 * @return      ApiMac_assocStatus_success, ApiMac_assocStatus_panAtCapacity,
 *              or ApiMac_assocStatus_panAccessDenied
 */
extern ApiMac_assocStatus_t Csf_deviceUpdate(
                ApiMac_deviceDescriptor_t *pDevInfo,
                ApiMac_capabilityInfo_t *pCapInfo);

/*!
 * @brief       The application calls this function to indicate that a device
 *              is no longer active in the network.  This function will be
 *              called when the device doesn't respond to the tracking request.
 *
 *              The information will be saved.
 *
 * @param       pDevInfo - pointer to the device information
 * @param       timeout - true if not active because of tracking timeout.
 *              meaning that the device didn't respond to the tracking request
 *              within the timeout period.
 */
extern void Csf_deviceNotActiveUpdate(ApiMac_deviceDescriptor_t *pDevInfo,
                                      bool timeout);

/*!
 * @brief       The application calls this function to indicate that a device
 *              has responded to a Config Request.
 *
 *              The information will be saved.
 *
 * @param       pSrcAddr - short address of the device that sent the message
 * @param       rssi - the received packet's signal strength
 * @param       pMsg - pointer to the Config Response message
 */
extern void Csf_deviceConfigUpdate(ApiMac_sAddr_t *pSrcAddr, int8_t rssi,
                                   Smsgs_configRspMsg_t *pMsg);

/*!
 * @brief       The application calls this function to indicate that a device
 *              has reported sensor data.
 *
 *              The information will be saved.
 *
 * @param       pSrcAddr - short address of the device that sent the message
 * @param       rssi - the received packet's signal strength
 * @param       pMsg - pointer to the Sensor Data message
 */
extern void Csf_deviceSensorDataUpdate(ApiMac_sAddr_t *pSrcAddr, int8_t rssi,
                                       Smsgs_sensorMsg_t *pMsg);
/*!
 * @brief       The application calls this function to indicate that a device
 *              has been disassociated.
 *
 * @param       shortAddr - short address of the device that has been disassociated
 */
#ifndef __unix__
extern void Csf_deviceDisassocUpdate(uint16_t shortAddr);
#endif

#if defined(DEVICE_TYPE_MSG) && !defined(__unix__) && !defined(CUI_DISABLE)
/*!
 * @brief       The application calls this function to print out the reported
 *              device type
 *
 * @param       deviceFamilyID - the integer ID of the device family
 * @param       deviceTypeID - the integer ID of the board/device
 */
extern void Csf_deviceSensorDeviceTypeResponseUpdate(uint8_t deviceFamilyID, uint8_t deviceTypeID);
#endif /* DEVICE_TYPE_MSG */

/*!
 * @brief       The application calls this function to indicate that a device
 *              set a Toggle LED Response message.
 *
 * @param       pSrcAddr - short address of the device that sent the message
 * @param       ledState - 0 is off, 1 is on
 */
extern void Csf_toggleResponseReceived(ApiMac_sAddr_t *pSrcAddr, bool ledState);

/*!
 * @brief       The application calls this function to indicate that a device
 *              set a Toggle LED Response message.
 *
 * @param       pSrcAddr - short address of the device that sent the message
 * @param       ledState - 0 is off, 1 is on
 */
extern void Csf_toggleResponseReceived(ApiMac_sAddr_t *pSrcAddr, bool ledState);

/*!
 * @brief       The application calls this function to blink the identify LED.
 *
 * @param       identifyTime - time in seconds for which to identify
 */
#ifndef __unix__
extern void Csf_identifyLED(uint8_t identifyTime);
#endif

/*!
 * @brief       The application calls this function to indicate that the
 *              Coordinator's state has changed.
 *
 * @param       state - new state
 */
extern void Csf_stateChangeUpdate(Cllc_states_t state);

/*!
 * @brief       Initialize the tracking clock.
 */
extern void Csf_initializeTrackingClock(void);

#ifdef IEEE_COEX_METRICS
/*!
 * @brief       Initialize the coex clock.
 */
extern void Csf_initializeCoexClock(void);
#endif

#ifdef MAC_STATS
/*!
 * @brief       Initialize the mac stats clock.
 */
extern void Csf_initializeStatsClock(void);
#endif

/*!
 * @brief       Initialize the identify clock.
 */
extern void Csf_initializeIdentifyClock(void);

/*!
 * @brief       set the tracking clock.
 *
 * @param       trackingTime - set timer this value (in msec)
 */
extern void Csf_setTrackingClock(uint32_t trackingTime);

#ifdef IEEE_COEX_METRICS
/*!
 * @brief       Set the coex metrics clock.
 *
 * @param       coexTime - set timer this value (in msec)
 */
extern void Csf_setCoexClock(uint32_t coexTime);
#endif

#ifdef MAC_STATS
/*!
 * @brief       Set the mac stats clock.
 *
 * @param       statsTime - set timer this value (in msec)
 */
extern void Csf_setStatsClock(uint32_t statsTime);
#endif

/*!
 * @brief       Initialize the broadcast clock.
 */
extern void Csf_initializeBroadcastClock(void);

/*!
 * @brief       set the broadcast cmd clock.
 *
 * @param       broadcast time - set timer this value (in msec)
 */
extern void Csf_setBroadcastClock(uint32_t trackingTime);

/*!
 * @brief       Initialize the trickle timer clock
 */
extern void Csf_initializeTrickleClock(void);

/*!
 * @brief       Initialize the clock setting join permit duration
 */
extern void Csf_initializeJoinPermitClock(void);

/*!
 * @brief       Initialize the clock setting config request delay
 */
extern void Csf_initializeConfigClock(void);

/*!
 * @brief       Set trickle clock
 *
 * @param       trickleTime - duration of trickle timer( in msec)
 * @param       frameType - type of Async frame
 */
extern void Csf_setTrickleClock(uint32_t trickleTime, uint8_t frameType);

/*!
 * @brief       Set Join Permit clock
 *
 * @param       joinDuration - duration for which join permit is TRUE( in msec)
 */
extern void Csf_setJoinPermitClock(uint32_t joinDuration);

/*!
 * @brief       Set Config request delay clock
 *
 * @param       delay - duration config request event is set( in msec)
 */
extern void Csf_setConfigClock(uint32_t delay);

/*!
 * @brief       Read the number of device list items stored
 *
 * @return      number of entries in the device list
 */
extern uint16_t Csf_getNumDeviceListEntries(void);

/*!
 * @brief       Find the short address from a given extended address
 *
 * @param       pExtAddr - extended address
 *
 * @return      CSF_INVALID_SHORT_ADDR if not found, otherwise the short address
 */
extern uint16_t Csf_getDeviceShort(ApiMac_sAddrExt_t *pExtAddr);

/*!
 * @brief       Find entry in device list from an address
 *
 * @param       pDevAddr - device address
 * @param       pItem - place to put the device information
 *
 * @return      true if found, false if not
 */
extern bool Csf_getDevice(ApiMac_sAddr_t *pDevAddr, Llc_deviceListItem_t *pItem);

/*!
 * @brief       Find entry in device list
 *
 * @param       devIndex - Device number (not address)
 * @param       pItem - place to put the device information
 * @param       pSubID - pointer to location to store subID of the device found.
 *              If no device was found, pSubID content is not modified
 *
 * @return      true if found, false if not
 */
extern bool Csf_getDeviceItem(uint16_t devIndex, Llc_deviceListItem_t *pItem,
                              uint16_t *pSubID);

/*!
 * @brief       Find entry in device list
 *
 * @param       size - number of bytes to allocate from heap
 *
 * @return      true if found, false if not
 */
extern void *Csf_malloc(uint16_t size);

/*!
 * @brief       Csf implementation for memory de-allocation
 *
 * @param       ptr - a valid pointer to the memory to free
 */
extern void Csf_free(void *ptr);

/*!
 * @brief       Update the Frame Counter
 *
 * @param       pDevAddr - pointer to device's address. If this pointer
 *                         is NULL, it means that this is the frame counter
 *                         for this device.
 * @param       frameCntr -  valur of frame counter
 */
extern void Csf_updateFrameCounter(ApiMac_sAddr_t *pDevAddr,
                                   uint32_t frameCntr);

/*!
 * @brief       Get the Frame Counter
 *
 * @param       pDevAddr - pointer to device's address. If this pointer
 *                         is NULL, it means that this is the frame counter
 *                         for this device.
 * @param       pFrameCntr -  pointer to place to put the frame counter
 *
 * @return      true if the frame counter existed, false if not.
 */
extern bool Csf_getFrameCounter(ApiMac_sAddr_t *pDevAddr,
                                   uint32_t *pFrameCntr);

/*!
 * @brief       Delete an entry from the device list
 *
 * @param       pAddr - address to remove from device list.
 */
extern void Csf_removeDeviceListItem(ApiMac_sAddrExt_t *pAddr);

/*!
 * @brief       Display Error
 *
 * @param       pTxt - Text string to display.
 * @param       code -  numeric code to display
 * @param       shortAddress -  sensor short address
 *
 */
extern void Csf_displayError(const char *pTxt, uint8_t code, uint16_t shortAddress);

/*!
 * @brief       Assert Indication
 *
 * @param       reason - Reason for Assert
 *                     2 - HAL/ICALL
 *                     3 - MAC
 *                     4 - TIRTOS
 */
extern void Csf_assertInd(uint8_t reason);

/*!
 * @brief       Clear all the NV Items
 */
extern void Csf_clearAllNVItems(void);

/*!
 * @brief       Check if config timer is active
 *
 * @return      true if active, false if not active
 */
extern bool Csf_isConfigTimerActive(void);

/*!
 * @brief       Check if tracking timer is active
 *
 * @return      true if active, false if not active
 */
extern bool Csf_isTrackingTimerActive(void);

#ifndef CUI_DISABLE
/*!
 * @brief       Handles printing that the orphaned device joined back
 *
 * @return      none
 */
extern void Csf_IndicateOrphanReJoin(uint16_t shortAddr);
#endif /* CUI_DISABLE */

/*!
 * @brief       The application calls this function to open the network.
 */
extern void Csf_openNwk(void);

/*!
 * @brief       The application calls this function to close the network.
 */
extern void Csf_closeNwk(void);

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 * @brief       The application calls this function to get a passkey.
 *
 * @param       passkeyAction - passkey entry request or timeout.
 */
extern void Csf_SmPasskeyEntry(SM_passkeyEntry_t passkeyAction);
#endif //FEATURE_SECURE_COMMISSIONING

/*!
 * @brief       The application calls this function to disassociate from a sensor.
 */
extern int Csf_sendDisassociateMsg(uint16_t deviceShortAddr);

/*!
 * @brief       The application calls this function to get device ext. address.
 */
extern int Csf_getDeviceExtAdd(uint16_t deviceShortAddr, ApiMac_sAddrExt_t * extAddr);


#ifdef USE_DMM
/*!
 * @brief       Initialize the provisioning clock.
 */
extern void Csf_initializeProvisioningClock(void);

/*!
 * @brief       set the provisioning clock.
 *
 * @param       provision - true to set/start provisioning timeout clock,
 *                          false to set disassociation delay.
 */
extern void Csf_setProvisioningClock(bool provision);
#endif



#ifdef __cplusplus
}
#endif

#endif /* CSF_H */

