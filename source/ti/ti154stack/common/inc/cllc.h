/******************************************************************************

 @file cllc.h

 @brief Coordinator Logical Link Controller

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
#ifndef CLLC_H
#define CLLC_H

/******************************************************************************
 Includes
 *****************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "llc.h"
#include "api_mac.h"
#include "ti_154stack_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/*! Event ID - LLC Event for PA Async command */
#define CLLC_PA_EVT             0x0001
/*! Event ID - LLC Event for PC Async command */
#define CLLC_PC_EVT             0x0002
/*! Event ID - LLC Event for Join Permit duration */
#define CLLC_JOIN_EVT           0x0004
/*! Event ID - State change event */
#define CLLC_STATE_CHANGE_EVT   0x0008

/*! Association status */
#define CLLC_ASSOC_STATUS_ALIVE 0x0001

/*!
 Coordinator State Values
 */
typedef enum
{
    /*! Powered up, not started and waiting for user to start */
    Cllc_states_initWaiting,
    /*! Starting coordinator, scanning and selecting the best parameters */
    Cllc_states_startingCoordinator,
    /*!
     Powered up, found network information, and restoring device in network
     */
    Cllc_states_initRestoringCoordinator,
    /*! Device is operating as coordinator */
    Cllc_states_started,
    /*! Device is restored as coordinator in the network */
    Cllc_states_restored,
    /*! Joining allowed state has changed to allowed */
    Cllc_states_joiningAllowed,
    /*! Joining allowed state has changed to not allowed */
    Cllc_states_joiningNotAllowed
} Cllc_states_t;

/*!
 Coordinator starting State Values
 */
typedef enum
{
    /*! Initialized state */
    Cllc_coordStates_initialized = 0,
    /*! MAC  coordinator is performing a active scan  */
    Cllc_coordStates_scanActive,
    /*! MAC  coordinator is performing a ED scan  */
    Cllc_coordStates_scanEnergyDetect,
    /*! active scan  confirm received*/
    Cllc_coordStates_scanActiveCnf,
    /*! Eneergy detect scan confirm received */
    Cllc_coordStates_scanEdCnf,
    /*! Start confirm received */
    Cllc_coordStates_startCnf
} Cllc_coord_states_t;

/*! Building block for association table */
typedef struct
{
    /*! Short address of associated device */
    uint16_t shortAddr;
    /*! capability information */
    ApiMac_capabilityInfo_t capInfo;
    /*! RSSI */
    int8_t rssi;
    /*! Device alive status */
    uint16_t status;
#ifdef FEATURE_SECURE_COMMISSIONING
    uint8_t reCM_status;
    uint8_t keyRef_statue;
    uint8_t NumOfKeyRefresh;
#endif
#ifdef USE_DMM
    uint8_t sensorData;
#endif
} Cllc_associated_devices_t;

/*! Cllc statistics */
typedef struct
{
    /*! number of PA Solicit messages received */
    uint32_t fhNumPASolicitReceived;
    /*! number of PA messages sent */
    uint32_t fhNumPASent;
    /*! number of PC Solicit messages received */
    uint32_t fhNumPANConfigSolicitsReceived;
    /*! number of PC messages sent */
    uint32_t fhNumPANConfigSent;
    uint32_t otherStats;
} Cllc_statistics_t;

/*! Association table */
extern Cllc_associated_devices_t Cllc_associatedDevList[CONFIG_MAX_DEVICES];
/*! Cllc statistics */
extern Cllc_statistics_t Cllc_statistics;

/*! Cllc events flags */
extern uint16_t Cllc_events;

/******************************************************************************
 Structures - Building blocks for the Coordinator LLC
 *****************************************************************************/

/*!
 Coordinator started callback.
 */
typedef void (*Cllc_startedFp_t)(Llc_netInfo_t *pStartedInfo);

/*!
 Device joining callback - Indication that a device is joining.  pDevInfo
 is a pointer to the device's information and capInfo is the device's
 capabilities.  The application will determine if the device can join by
 the return value - ApiMac_assocStatus_t.  If this callback is ignored,
 callback pointer is NULL, all devices will be allowed to join, assuming
 that associate permit is true.
 */
typedef ApiMac_assocStatus_t (*Cllc_deviceJoiningFp_t)(
                ApiMac_deviceDescriptor_t *pDevInfo,
                ApiMac_capabilityInfo_t *pCapInfo);

/*!
 State Changed callback - This indication says that the CLLC's state
 has changed.
 */
typedef void (*Cllc_stateChangedFp_t)(Cllc_states_t state);

/*!
 Structure containing all the CLLC callbacks (indications).
 To receive the callback fill in the structure item with a pointer
 to the function that will handle that callback.  To ignore a callback
 set that function pointer to NULL.
 */
typedef struct _Cllc_callbacks_t
{
    /*! Coordinator Started Indication callback */
    Cllc_startedFp_t pStartedCb;
    /*! Device joining callback */
    Cllc_deviceJoiningFp_t pDeviceJoiningCb;
    /*! The state has changed callback */
    Cllc_stateChangedFp_t pStateChangeCb;
} Cllc_callbacks_t;

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
 * @param       pCllcCbs - pointer to the CLLC callback structure
 */
extern void Cllc_init(ApiMac_callbacks_t *pMacCbs, Cllc_callbacks_t *pCllcCbs);

/*!
 * @brief       Application task processing.
 *              <BR>
 *              This function handles the events in CLLC for trickle timer,
 *              controlling associate permit flag and transitioning between
 *              various states during operation.
 */
extern void Cllc_process(void);

/*!
 * @brief       set the PAN ID to form.
 *
 * @param       panId - PAN ID to set
 */
extern void Cllc_setFormingPanId(uint16_t panId);

/*!
 * @brief       get the PAN ID to form.
 *
 * @param       panId - Pointer to current PAN ID in jdllc
 */
extern void Cllc_getFormingPanId(uint16_t *pPanId);


/*!
 * @brief       set the channel mask.
 *
 * @param       channelMask - channel mask to set
 */
extern void Cllc_setChanMask(uint8_t *channelMask);

/*!
 * @brief       get the channel mask.
 *
 * @param       channelMask - Pointer to current channel mask in cllc
 */
extern void Cllc_getChanMask(uint8_t *channelMask);


/*!
 * @brief       set the async channel mask.
 *
 * @param       asyncChanMask - async channel mask to set
 */
extern void Cllc_setAsyncChanMask(uint8_t *asyncChanMask);

/*!
 * @brief       get the async channel mask.
 *
 * @param       asyncChanMask - async channel mask to get
 */
extern void Cllc_getAsyncChanMask(uint8_t *asyncChanMask);

#ifdef FEATURE_MAC_SECURITY
/*!
 * @brief       set the default security key.
 *
 * @param       key - default key to set
 */
extern void Cllc_setDefaultKey(uint8_t *key);

/*!
 * @brief       get the default security key.
 *
 * @param       key - default key to get
 */
extern void Cllc_getDefaultKey(uint8_t *key);
#endif

/*!
 * @brief       Set the collector (Full Function Device - FFD) address.
 *
 * @param       addr - Pointer to FFD address to set
 */
extern void Cllc_setFfdShortAddr(uint16_t addr);

/*!
 * @brief       Get the collector (Full Function Device - FFD) address.
 *
 * @param       addr - FFD address from jdllc
 */
extern void Cllc_getFfdShortAddr(uint16_t *addr);

/*!
 * @brief       Get the current PHY ID of the sensor device.
 *
 * @return      frequency - Current MAC PHY ID
 */
extern uint8_t Cllc_getFreq(void);

/*!
 * @brief       Get the current channel of the collector device
 *
 * @return      channel - Current MAC channel
 */
extern uint8_t Cllc_getChan(void);

/*!
 * @brief       Get the current state of the sensor device.
 *
 * @return       state - Sensor state
 */
extern uint8_t Cllc_getProvState(void);

/*!
 * @brief       Start a network.
 *              <BR>
 *              The application will call this function to start the process
 *              of making this device a coordinator of a new network.
 *              <BR>
 *              This module will automatically determine the best channel(s)
 *              and PANID, before starting.
 */
extern void Cllc_startNetwork(void);

/*!
 * @brief       Restore the coodinator in the network.
 *              <BR>
 *              The application will call this function to restore the
 *              coordinator in the network by passing all the network
 *              information needed to restore the device.
 *              <BR>
 *              This module will configure the MAC with all the network
 *              information then start the coordinator without scanning.
 *
 * @param       pNetworkInfo - network information
 * @param       numDevices - number of devices in association table
 * @param       pDevList - list of devices
 */
extern void Cllc_restoreNetwork(Llc_netInfo_t *pNetworkInfo, uint16_t numDevices,
		Llc_deviceListItem_t *pDevList);
/*!
 * @brief       Remove device from the network.
 *              <BR>
 *              The application will call this function to remove a device
 *              from the network by passing the extended address of the
 *              device.
 *              <BR>
 *
 * @param       pExtAddr - pointer to extended address of the device
 */
extern void Cllc_removeDevice(ApiMac_sAddrExt_t *pExtAddr);

/*!
 * @brief       Set Join Permit PIB value.
 *              <BR>
 *              The application will call this function to set or clear
 *              <BR>
 *
 * @param       duration - duration for join permit to be turned on in
 *              milliseconds.
 *              0 sets it Off, 0xFFFFFFFF sets it ON indefinitely
 *              Any other non zero value sets it on for that duration
 *
 * @return      ApiMac_status_t - ApiMac_status_success for successful
 *                                operation
 */
extern ApiMac_status_t Cllc_setJoinPermit(uint32_t duration);

/*!
 * @brief       API for app to set disassociation request.
 *
 * @param       shortAddr - short address of device
 * @param       rxOnIdle -- true or false
 */
extern void Cllc_sendDisassociationRequest(uint16_t shortAddr,bool rxOnIdle);

/*!
 * @brief       Initialize the MAC Security
 *
 * @param       frameCounter - Initial frame counter
 */
extern void Cllc_securityInit(uint32_t frameCounter, uint8_t *key);

/*!
 * @brief       Fill in the security structure
 *
 * @param       pSec - pointer to security structure
 */
extern void Cllc_securityFill(ApiMac_sec_t *pSec);

/*!
 * @brief       Check the security level against expected level
 *
 * @param       pSec - pointer to security structure
 *
 * @return      true is matches expected security level, false if not
 */
extern bool Cllc_securityCheck(ApiMac_sec_t *pSec);

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
extern ApiMac_status_t Cllc_addSecDevice(uint16_t panID,
                                              uint16_t shortAddr,
                                              ApiMac_sAddrExt_t *pExtAddr,
                                              uint32_t frameCounter);

/*!
 * @brief      Find the associated device table entry matching an
 *             extended address.
 *
 * @param      shortAddr - device's short address
 *
 * @return     pointer to the associated device table entry,
 *             NULL if not found.
 */
extern Cllc_associated_devices_t *Cllc_findDevice(uint16_t shortAddr);
//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /* CLLC_H */

