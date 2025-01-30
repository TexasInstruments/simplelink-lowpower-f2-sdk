/******************************************************************************

 @file ssf.h

 @brief Sensor Specific Functions API

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
#ifndef SSF_H
#define SSF_H

/******************************************************************************
 Includes
 *****************************************************************************/
#include "llc.h"
#include "jdllc.h"
#include "smsgs.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif /* CUI_DISABLE */
#ifdef FEATURE_SECURE_COMMISSIONING
#include "sm_ti154.h"
#endif
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Typedefs
 *****************************************************************************/

/* Structure of validated config information */
typedef struct
{
    /* Frame Control field of the Config Response message */
    uint16_t frameControl;
    /* Reporting Interval field of the Config Response message */
    uint32_t reportingInterval;
    /* Polling Interval field of the Config Response message */
    uint32_t pollingInterval;
} Ssf_configSettings_t;

/******************************************************************************
 Public Variables
 *****************************************************************************/

/*!
 Assert reason for the last reset -  0 - no reason, 2 - HAL/ICALL,
 3 - MAC, 4 - TIRTOS
 */
extern uint8_t Ssf_resetReseason;

/*! Number of times the device has reset */
extern uint16_t Ssf_resetCount;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/

/*!
 * @brief       The application calls this function during initialization.
 *
 * @param       sem - pointer to semaphore used by MAC API
 */
extern void Ssf_init(void *sem);

/*!
 * @brief       The application must call this function periodically to
 *              process any events that this module needs to process.
 */
extern void Ssf_processEvents(void);

#if USE_DMM && !(DMM_CENTRAL)
 /*!
 * @brief       Key event handler function
 *
 * @param       keysPressed - keys that are pressed
 */
extern void Ssf_processKeyChange(uint8_t keysPressed);
#endif

/*!
 * @brief       The application calls this function to indicate that it has
 *              started or restored the device in a network.
 *
 *              The information will be saved and used to determine if a
 *              network was already started and should be restored instead
 *              of started.
 *
 * @param       rejoined - true if rejoined to network, false if joined
 * @param       pDevInfo - This device's information
 * @param       pParentInfo - This is the parent's information
 */
extern void Ssf_networkUpdate(bool rejoined,
                              ApiMac_deviceDescriptor_t *pDevInfo,
                              Llc_netInfo_t  *pParentInfo);

/*!
 * @brief       The application calls this function to get the device
 *              information in a network.
 *
 * @param       pDevInfo - This device's information
 * @param       pParentInfo - This is the parent's information
 *
 * @return      true if info found, false if not
 */
extern bool Ssf_getNetworkInfo(ApiMac_deviceDescriptor_t *pDevInfo,
                              Llc_netInfo_t  *pParentInfo);

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 The application calls this function to store the device key information to NV
 Public function defined in ssf.h
 */
extern void Ssf_DeviceKeyInfoUpdate(nvDeviceKeyInfo_t *pDevKeyInfo);

/*!
 The application calls this function to get the device
 *              Key information.

 Public function defined in ssf.h
 */
extern bool Ssf_getDeviceKeyInfo(nvDeviceKeyInfo_t *pDevKeyInfo);

/*!

 Clear device key information in NV

 Public function defined in ssf.h
 */
extern void Ssf_clearDeviceKeyInfo(void);

#endif

#ifdef FEATURE_NATIVE_OAD
/*!
 * @brief       The application calls this function to update OAD info in NV.
 *
 *
 * @param       pOadBlock - current OAD block
 * @param       pOadImgHdr - OAD image header
 * @param       pOadImgId - OAD image ID
 */
extern void Ssf_oadInfoUpdate(uint16_t *pOadBlock, uint8_t *pOadImgHdr, uint8_t *pOadImgId, ApiMac_sAddr_t *pOadServerAddr);

/*!
 * @brief       The application calls this function to get the OAD
 *              information.
 *
 * @param       pOadBlock - current OAD block
 * @param       pOadImgHdr - OAD image header
 * @param       pOadImgId - OAD image ID
 *
 * @return      true if info found, false if not
 */
extern bool Ssf_getOadInfo(uint16_t *pOadBlock, uint8_t *pOadImgHdr, uint8_t *pOadImgId, ApiMac_sAddr_t *pOadServerAddr);

/*!
 * @brief       The application calls this function to clear OAD
 *              information from NV.
 *
 * @return      void
 */
extern void Ssf_clearOadInfo();
#endif //FEATURE_NATIVE_OAD

/*!
 * @brief       The application calls this function to indicate that the
 *              device's state has changed.
 *
 * @param       state - new state
 */
extern void Ssf_stateChangeUpdate(Jdllc_states_t state);

/*!
 * @brief       Initialize the reading clock.
 */
extern void Ssf_initializeReadingClock(void);

/*!
 * @brief       set the reading clock.
 *
 * @param       readingTime - timer duration to read from sensor (in msec)
 */
extern void Ssf_setReadingClock(uint32_t readingTime);

/*!
 * @brief       The application calls this function to indicate that this
 *              device has been removed from the network.
 *
 *              The information will be saved.
 *
 * @param       shortAddr - short address of device that was removed
 */
extern void Ssf_removedUpdate(uint16_t shortAddr);

/*!
 * @brief       The application calls this function to indicate
 *              a Configuration Request message.
 *
 * @param       pRsp - pointer to the Config Response message.
 */
extern void Ssf_configurationUpdate(Smsgs_configRspMsg_t *pRsp);

/*!
 * @brief       The application calls this function to get the
 *              saved device configuration.
 *
 * @param       pInfo - Place to put the config information
 *
 * @return      true if info found, false if not
 */
extern bool Ssf_getConfigInfo(Ssf_configSettings_t *pInfo);

/*!
 * @brief       The application calls this function to indicate sensor data.
 *
 * @param       pMsg - pointer to the Sensor Data message
 */
extern void Ssf_sensorReadingUpdate(Smsgs_sensorMsg_t *pMsg);

/*!
 * @brief       The application calls this function to indicate
 *              that a tracking message was received.
 *
 * @param       pSrcAddr - address of the device that sent the
 *                         tracking message.
 */
extern void Ssf_trackingUpdate(ApiMac_sAddr_t *pSrcAddr);

/*!
 * @brief       Ssf implementation for memory allocation
 *
 * @param       size - number of bytes to allocate from heap
 *
 * @return      true if found, false if not
 */
extern void *Ssf_malloc(uint16_t size);

/*!
 * @brief       Ssf implementation for memory de-allocation
 *
 * @param       ptr - a valid pointer to the memory to free
 */
extern void Ssf_free(void *ptr);

/*!
 * @brief       Initialize the trickle timer clock
 */
extern void Ssf_initializeTrickleClock(void);

/*!
 * @brief       Set trickle timer clock
 *
 * @param       trickleTime - duration of trickle timer( in msec)
 * @param       frameType - type of Async frame
 */
extern void Ssf_setTrickleClock(uint16_t trickleTime, uint8_t frameType);

/*!
 * @brief       Initialize the poll timer clock
 */
extern void Ssf_initializePollClock(void);

/*!
 * @brief       Set the poll timer clock
 *
 * @param       pollTime - duration of poll timer in milliseconds
 */
extern void Ssf_setPollClock(uint32_t pollTime);

/*!
 * @brief       Get the poll timer clock
 *
 * @return       pollTime - duration of poll timer in milliseconds
 */
extern uint32_t Ssf_getPollClock(void);

/*!
 * @brief       Initialize the scan backoff timer clock
 */
extern void Ssf_initializeScanBackoffClock(void);

/*!
 * @brief       Set the scan backoff timer clock
 *
 * @param       scanBackoffTime - duration of scan backoff timer in milliseconds
 */
extern void Ssf_setScanBackoffClock(uint32_t scanBackoffTime);

/*!
 * @brief       Stop the scan backoff timer clock
 */
extern void Ssf_stopScanBackoffClock(void);

/*!
 * @brief       Initialize the scan backoff timer clock
 */
extern void Ssf_initializeFHAssocClock(void);

/*!
 * @brief       Set the FH association delay timer clock
 *
 *
 * @param       fhAssocTime - duration of FH association delay timer in milliseconds
 */
extern void Ssf_setFHAssocClock(uint32_t fhAssocTime);

/*!
 * @brief       Update the Frame Counter
 *
 * @param       pDevAddr - pointer to device's address. If this pointer
 *                         is NULL, it means that this is the frame counter
 *                         for this device.
 * @param       frameCntr -  valur of frame counter
 */
extern void Ssf_updateFrameCounter(ApiMac_sAddr_t *pDevAddr,
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
extern bool Ssf_getFrameCounter(ApiMac_sAddr_t *pDevAddr,
                                   uint32_t *pFrameCntr);


/*!
 * @brief       Display Error
 *
 * @param       pTxt - Text string to display.
 * @param       code -  numeric code to display
 */
extern void Ssf_displayError(const char *pTxt, uint8_t code);

/*!
 * @brief       Assert Indication
 *
 * @param       reason - Reason for Assert
 *                     2 - HAL/ICALL
 *                     3 - MAC
 *                     4 - TIRTOS
 */
extern void Ssf_assertInd(uint8_t reason);

/*!
 * @brief       The application calls this function to clear the network
 *              information from NV
 */
extern void Ssf_clearNetworkInfo(void);

/*!
 * @brief       Clear all the NV Items
 */
extern void Ssf_clearAllNVItems(void);

/*!
 * @brief       The application calls this function to get the
 *              temperature from the onboard sensor
 *
 * @return      value of on-board temperature sensor in Deg C
 */
extern int16_t Ssf_readTempSensor(void);

/*!
 * @brief       The application calls this function to toggle an LED.
 *
 * @return      state of LED after this call: 0 is off, 1 is on
 */
extern bool Ssf_toggleLED(void);

/*!
 * @brief       The application calls this function to switch on LED.
 */
extern void Ssf_OnLED(void);

/*!
 * @brief       The application calls this function to switch off LED.
 */
extern void Ssf_OffLED(void);


#if defined(DISPLAY_PER_STATS)
/*!
 * @brief       The application calls this function to print updated PER stats to the display.
 */
extern void Ssf_displayPerStats(Smsgs_msgStatsField_t* stats);
#endif /* DISPLAY_PER_STATS */

#if (USE_DMM) && !(DMM_CENTRAL)
/*!
 * @brief       Initialize the provisioning clock.
 */
extern void Ssf_initializeProvisioningClock(void);

/*!
 * @brief       set the provisioning clock.
 *
 * @param       provision - true to set/start provisioning timeout clock,
 *                          false to set disassociation delay.
 */
extern void Ssf_setProvisioningClock(bool provision);

#ifndef CUI_DISABLE
/*!
 * @brief       Validate and handle errors in channel mask entered through a UI
 *
 * @param       _chanMask - channel mask updated with user input
 * @param       byteIdx   - index of modified byte
 */
extern void Ssf_validateChMask(uint8_t *_chanMask, uint8_t byteIdx);
#endif /* CUI_DISABLE */

#endif
/*!
 * @brief       A callback calls this function to post the application task semaphore..
 */
void Ssf_PostAppSem(void);

#ifdef LPSTK
/*!
 * @brief       A callback calls this function to pend the application task semaphore..
 */
void Ssf_PendAppSem(void);
#endif

#ifdef FEATURE_SECURE_COMMISSIONING
/*!
 * @brief       The application calls this function to get a passkey.
 *
 * @param       passkeyAction - passkey entry request or timeout.
 */
extern void Ssf_SmPasskeyEntry(SM_passkeyEntry_t passkeyAction);
#endif

#ifdef __cplusplus
}
#endif

#endif /* SSF_H */
