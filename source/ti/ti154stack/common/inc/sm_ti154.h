/******************************************************************************

 @file sm_ti154.h

 @brief TI 15.4 Security Manager interface

 Group: LPRF
 Target Device: cc13xx_cc26xx

 ******************************************************************************
 
 Copyright (c) 2018-2025, Texas Instruments Incorporated
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
#ifndef SM_TI154_H
#define SM_TI154_H

#ifdef FEATURE_SECURE_COMMISSIONING
/******************************************************************************
 Includes
 ************************************* ****************************************/

#include "ti_154stack_config.h"
#include "advanced_config.h"
#include "api_mac.h"
#include "smsgs.h"
#include "sm_ecc_ti154.h"
#ifndef CUI_DISABLE
#include "cui.h"
#endif /* CUI_DISABLE */
#include "llc.h"

#include <ti/drivers/utils/List.h>
#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************
 Constants and definitions
 *****************************************************************************/
/* SM EVNT Begin*/
/*! Event ID - Collector start SM */
#define SM_COLLECTOR_CM_START_EVT (0x0001)
/*! Event ID - process passkey */
#define SM_PASSKEY_EVT (0x0002)
/*! Event ID - SM key refresh, Collector side */
#define SM_KEY_REFRESH_EVT (0x0004)
/*! Event ID - SM Auth Response Event */
#define SM_AUTH_RSP_EVT (0x0008)
/*! Event ID - SM Auth Response Event */
#define SM_KEY_GEN_EVT (0x0010)
/*! Event ID - SM state command timeout */
#define SM_TIMEOUT_EVT (0x0020)
/*! Event ID - SM state change */
#define SM_STATE_CHANGE_EVT (0x0040)
/*! Event ID - Feedback from sending an SM packet: Success or failure would mean different actions */
#define SM_SEND_PKT_FB_EVT (0x0080)
/*! Event ID - SM state change to fail state after successful failure msg transmission */
#define SM_SENT_CM_FAIL_EVT (0x0100)
/* SM EVNT End*/

#define SM_PKT_MAX_RETRY_ATTEMPTS   5   /* max times each individual SM packet retry attempts allowed */
#define SM_CM_MAX_RETRY_ATTEMPTS    3   /* max commissioning attempts allowed */

/*! Passkey input process timeout value in ms */
#define SM_USER_INPUT_TIMEOUT_VALUE (30000)
/*! Commissioning request/response timeout value in ms */
#define SM_COMMISIONING_STATUS_TIMEOUT_VALUE (CONFIG_POLLING_INTERVAL + ((SM_CM_MAX_RETRY_ATTEMPTS - 1) * SM_USER_INPUT_TIMEOUT_VALUE))
/*! Passkey input process timeout per char in ms */
#define SM_PASSKEY_UART_TIMEOUT_VALUE (10)
/*! Waiting for passkey timeout value in ms */
#define SM_PASSKEY_WAIT_TIMEOUT_VALUE (SM_USER_INPUT_TIMEOUT_VALUE + SM_COMMISIONING_STATUS_TIMEOUT_VALUE)

/*! Polling interval during Commissioning in ms, Sensor only */
#define SM_POLLING_INTERVAL (300)

/*! Security Manager command message length in bytes */
#define SMMSGS_CMD_MSG_LEN (2)

/*! Security Manager data-specific MSDU Handle, independent from application level handles */
#define SM_DATA_MSDU_HANDLE (0x20)
/*! Security Manager fail state specific MSDU Handle, independent from application level handles */
#define SM_FAIL_MSDU_HANDLE (0x40)

#define SMMSGS_BASE_CMD_LENGTH (sizeof(Smsgs_cmdIds_t) + sizeof(uint16_t))

// SM Status
#define SM_SUCCESS                   (0x00)
#define SM_ENCRYPT_ERROR             (0xC1)    /* Cryptographic processing failed */
#define SM_INVALID_PARAM             (0xE0)
#define SM_MEMALLOC_ERROR            (0xE1)

// options (this should be exposed to users later)
#define PASSKEY_SIZE_BITS            (20)      // Number of bits in the pass key
#define PASSKEY_BIT                  (1)       // test 1 bit at a time
#define PASSKEY_ONEDIGIT             (4)       // test 1 digits (4bits) at a time (default)
#define PASSKEY_TEST_SIZE            (PASSKEY_ONEDIGIT)        // 1 = 1 bit at a time, 4 = 4 bits at a time. No other option must be allowed

// SM KEY related
#define SM_ECC_KEYLEN                (32)      // 256 bit / 8 = 32 bytes
#define SM_KEY_LENGTH                (16)      // 128 bit /8 = 16 bytes
#define SM_AES_BLOCK_LEN             (16)      // 128 bit /8 = 16 bytes

// User generated passkey related
#define SM_PASSKEY_LEN               (6)       // number of digits in user-inputted passkey
#define DECIMAL                      (10)      // for passkey ascii to int conversion
#define RESTART_CMD                  ('r')     // key for user to restart passkey input process
#define NO_CMD                       ('n')     // key for user to deny (re-enter) passkey
#define YES_CMD                      ('y')     // key for user to confirm passkey
#define END_CMD                      ('\r')    // enter key signaling user input passkey

//SM Auth Related
#define SM_NUM_AUTH_ITERATIONS     (PASSKEY_SIZE_BITS/PASSKEY_TEST_SIZE)
#define SM_AUTH_CODE_LEN           (16)        //16 bytes = 128 bits
#define SM_AUTH_NONCE_LEN          (16)        //16 bytes = 128 bits
#define SM_AUTH_CODE_NONCE_LEN     (SM_AUTH_CODE_LEN + SM_AUTH_NONCE_LEN)
#define SM_AUTH_METHOD_FAIL_BUF_SIZE (2)

// SM Key Generation related
#define SM_KEY_GEN_DATA_SIZE            (16)      // 128 bit /8 = 16 bytes
#define SM_PASSKEY_BUF_SIZE             (16)      // 128 bit /8 = 16 bytes
#define SM_AUTH_METHOD_BUF_SIZE         (3)       // 24 bit/8 = 3 bytes

/* re-commissioning status settings */
#define SM_RE_CM_REQUIRED          0x01    /* the device needs to be re-commissioned*/
#define SM_RE_CM_ATTEMPTED         0x02    /* re-CM is started */
#define SM_RE_CM_PENDING           0x03    /* re-CM is attempted but may be sensor
                                           is orphaned and hence not getting response */
#define SM_RE_CM_DEFAULT           0xFF    /* default */

/* key refreshment status settings */
#define SM_KEYRF_REQUIRED       0x10    /* key refresh is required*/
#define SM_KEYRF_ATTEMPTED      0x20    /* key refresh is in progress */
#define SM_KEYRF_FAIL           0x30    /* key refresh attempted and failed */
#define SM_KEYRF_DEFAULT        0xFF    /* default */

// User display messages
#define SM_DISPLAY_MSG_LEN         50   /* display message buffer size */
#define SM_PASSKEY_MSG_LEN         110  /* passkey display message buffer size */

/******************************************************************************
 Structures & Function pointers
 *****************************************************************************/
/*!
 Security Manager type values
 */
typedef enum
{
    /*! 15.4 Coordinator */
    SM_type_coordinator,
    /*! 15.4 Device */
    SM_type_device
} SM_types_t;

/*!
 Security Manager state values
 */
typedef enum
{
    /*! Base state Device association is complete, start commissioning process */
    SM_states_entry =0,

    /*! Coordinator specific state, send commissioning start event */
    SM_states_start,

    /*! Request or send authentication method device supports */
    SM_states_auth_method,

    /*! Set passkey based on authentication method */
    SM_states_passkey,

    /*! Generate ECDH key pair and exchange public key with foreign party */
    SM_states_key_exchange,

    /*! Begin Authentication process */
    SM_states_authentication,

    /*! Generate device key */
    SM_states_key_gen,

    /*! Commissioning successful */
    SM_states_finish_success,

    /*! Commissioning failure */
    SM_states_finish_fail,
} SM_states_t;

/*!
 Security Manager Latest state
 */
typedef enum
{
    /* Commissioning has not been finished*/
    SM_CM_Not_Started =0,
    /* Commissioning is in progress*/
    SM_CM_InProgress,
    /* Commissioning has been successfully finished*/
    SM_CM_Finished_Successfully,
} SM_lastState_t;

/*!
 Key add result
 */
typedef enum
{
    /* Memory allocation Error*/
    SM_Key_Mem_Error =0,
    /* Not a new device*/
    SM_Key_Exist,
    /* New Key*/
    SM_Key_New,
} SM_AddResult_t;


/*!
 Message IDs for Commissioning data message. When sent over-the-air in a message,
 this field is two bytes.
 */
 typedef enum
 {
    SMMsgs_cmdIds_processRequest = 0xCF10,
    SMMsgs_cmdIds_processResponse = 0xCF20,
    SMMsgs_cmdIds_processMethodConfirm = 0xCF30,
    SMMsgs_cmdIds_processPasskeyConfirm = 0xCF40,
    SMMsgs_cmdIds_processPublicKey = 0xCF50,
    SMMsgs_cmdIds_processInfo128SecurityVal = 0xCF60,
    SMMsgs_cmdIds_processInfo128CodeNonce = 0xCF70,
    SMMsgs_cmdIds_processSuccess = 0xCF80,
    SMMsgs_cmdIds_processFail = 0xCF90,
    SMMsgs_cmdIds_keyRefreshRequest = 0xCFA0,
    SMMsgs_noMsg = 0xFFFF,
 } SMMsgs_cmdIds_t;

/*!
 Authentication methods for Commissioning through the security manager.
 */
 typedef enum
 {
    SMMsgs_authMethod_passkey = 0x01,
    SMMsgs_authMethod_defaultCode = 0x02,
    SMMsgs_authMethod_justAllowed = 0x04
 } SMMsgs_authMethod_t;

 /* Structure for a seedKey Entry */
 typedef struct _SM_seedKey_Entry_t{
    List_Elem                        elem;
    uint8_t                          index;
    uint8_t                          lookUpData[APIMAC_KEY_SOURCE_MAX_LEN];
    uint16_t                         shortAddress;
    ApiMac_sAddrExt_t                extAddress;
    uint8_t                          seedKey[SM_ECC_PUBLIC_KEY_SIZE];
 } SM_seedKey_Entry_t;

#ifndef FEATURE_FULL_FUNCTION_DEVICE
 /* Structure to store the device key info in NV*/
 typedef struct {
    uint8_t  lookUpData[APIMAC_KEY_SOURCE_MAX_LEN];
    uint8_t  deviceKey[APIMAC_KEY_MAX_LEN];
    uint8_t  seedKey[SM_ECC_PUBLIC_KEY_SIZE];
    uint8_t  index;
 } nvDeviceKeyInfo_t;
#endif

 /*!
  Passkey callback actions.
  */
  typedef enum
  {
     /*! Passkey entry request */
     SM_passkeyEntryReq = 0,
     /*! Passkey entered */
     SM_passkeyEntered,
     /*! Passkey entry timeout */
     SM_passkeyEntryTimeout,
  } SM_passkeyEntry_t;

/*!
 Error codes for Commissioning through the security manager.
 */
 typedef enum
 {
     /*! No error code */
    SMMsgs_errorCode_none = 0x11,
    /*! Authentication method is not supported by target device */
    SMMsgs_errorCode_unsupportedAuthMethod = 0x12,
    /*! Authentication input does not match */
    SMMsgs_errorCode_noMatchAuthVal = 0x13,
    /*! Key confirmation value does not match */
    SMMsgs_errorCode_noMatchKeyConfirm = 0x14,
    /*! No previously generated key to update */
    SMMsgs_errorCode_missingKey = 0x19,
    /*! Unexpected commissioning message received */
    SMMsgs_errorCode_unexpectedMsg = 0x1A,
    /*! Commissioning process timeout */
    SMMsgs_errorCode_timeout = 0x1B,
    /*! Re-commissioning process - sensor not responding/ orphaned */
    SMMsgs_errorCode_reComm_sensor_notResp = 0x1C,

 } SMMsgs_errorCode_t;

 /*!
  Request passkey callback - callback to request passkey from user input
  */
 typedef void (*SM_requestPassKeyFp_t) (SM_passkeyEntry_t);

/*!
 Commissioning failed callback - passes the status of the commissioning process
 to the application. In this case it indicates that a failure occurred.
 */
typedef void (*SM_failCMProcessFp_t)(ApiMac_deviceDescriptor_t *, bool, bool, SMMsgs_errorCode_t);

/*!
 Commissioning successful callback - passes the status of the commissioning process
 to the application. In this case it indicates that the
 commissioning process was successful.
 */
typedef void (*SM_successCMProcessFp_t) (ApiMac_deviceDescriptor_t *, bool);

typedef struct _SM_callbacks_t
{
    /*! Request passkey callback */
    SM_requestPassKeyFp_t pfnRequestPasskeyCb;
    /*! Security authentication failed callback */
    SM_failCMProcessFp_t pfnFailCMProcessCb;
    /* Security authentication successful callback */
    SM_successCMProcessFp_t pfnSuccessCMProcessCb;
} SM_callbacks_t;

/******************************************************************************
 Global Variables
 *****************************************************************************/
extern uint16_t SM_events;
extern SM_lastState_t SM_Current_State;

extern uint16_t SM_Sensor_SAddress;
extern ApiMac_deviceDescriptor_t commissionDevInfo;
extern volatile bool useSendPktStatus;

#ifdef FEATURE_FULL_FUNCTION_DEVICE
extern bool fCommissionRequired;
extern uint16_t keyRecoverDeviceNumber;
extern bool readySMToRun;
#endif

extern uint8_t SM_cmAttempts;
extern bool SM_forceStopped;

/******************************************************************************
 Function Prototypes
 *****************************************************************************/
/*assert function*/
extern void Main_assertHandler(uint8_t assertReason);
/*!
 * @brief       Initialize this module and update the security manager
 *              callback structure.
 *              <BR>
 *              This function will take the application's MAC callback
 *              structure and override some callbacks with its own callbacks.
 *              The application callbacks that are overridden are saved so that
 *              they can be chain called when processing the callbacks.
 *
 * @param       sem - pointer to semaphore used to sync MAC and APP tasks
 * @param       pCuiHndl - pointer to to Common UI handle for UI interaction
 */
#ifndef CUI_DISABLE
extern void SM_init(void *sem, CUI_clientHandle_t cuiHndl);
#else
extern void SM_init(void *sem);
#endif /* CUI_DISABLE */


/*!
 * @brief       Start the security manager event handler
 *              <BR>
 *              The application will call this function to start the process
 *              of commissioning this device to a new network.
 */
extern void SM_process(void);

/*!
 * @brief       Start Key Refreshment process

 *
 * @param       pDevInfo - pointer to the API MAC device information block
 * @param       pSec - pointer to sensor device security information. The sensor must have already
 *                    associated to the network at the MAC level.
 * @param       FH Enabled - True if FH mode is enabled, false otherwise. Determines
 *                           whether to use extended or short addressing when making
 *                           mac data request
 * @param       RX On Idle - True if RX On Idle is enabled for the sensor device, false
 *                           otherwise. Determines whether the receiver is on when idling.
 */
extern void SM_startKeyRefreshProcess(ApiMac_deviceDescriptor_t *pDevInfo, ApiMac_sec_t *pSec, bool fhEnabled, bool rxOnIdle);

/* Get a device key lookup data from the SeedKeyTable*/
/* input param:     device ext. Address
 *
 * ouput param:    ApiMac_sec_t
 */
extern void SM_getSrcDeviceSecurityInfo(ApiMac_sAddrExt_t extAddress, uint16_t shorAddress, ApiMac_sec_t *pSec);

/*!
 * @brief       Start Security manager commissioning process
 *              <BR>
 *              This function will take the application's MAC device descriptor
 *              structure, security information and a parameter to start the key
 *              refreshment process.
 *
 * @param       devInfo - pointer to the API MAC device information block
 * @param       sec - Sensor device security information. The sensor must have already
 *                    associated to the network at the MAC level.
 * @param       FH Enabled - True if FH mode is enabled, false otherwise. Determines
 *                           whether to use extended or short addressing when making
 *                           mac data request
 * @param       RX On Idle - True if RX On Idle is enabled for the sensor device, false
 *                           otherwise. Determines whether the receiver is on when idling.
 * @param       Device type - Either SM_type_coordinator or SM_type_device
 * @param       authMethods - The authMethods parameter for this function is a bitmask containing
 *                           values of type SMMsgs_authMethod_t.
 */
extern void SM_startCMProcess(ApiMac_deviceDescriptor_t *devInfo, ApiMac_sec_t *sec,
    bool fhEnabled, bool rxOnIdle, SM_types_t devType, uint8_t authMethods);

/*!
 * @brief       Stop the security manager Process
 *              <BR>
 *              The application will call this function to stop the process
 *              of commissioning this device to a new network.
 */
extern void SM_stopCMProcess(void);

/*!
 * @brief       Register Security manager callback functions
 *              refreshment process.
 *
 * @param       pSMCallbacks - pointer to the security manager callback functions
 */
extern void SM_registerCallback(SM_callbacks_t *pSMCallbacks); 

/* Remove a SeedKey Entry from the SeedKeyTable*/
/* input param:     device ext. Address
 *
 * ouput param:     true = removed successfully, false = cannot find the entry
 */
extern bool SM_removeEntryFromSeedKeyTable (ApiMac_sAddrExt_t *extAddress);

/*!
 * @brief       Parse incoming data that has been sent to the application
 *              for the security manager.
 *
 * @param       pDataInd - pointer to a data indication packet to be parsed.
 */
extern void SM_processCommData(ApiMac_mcpsDataInd_t *pDataInd);

/* Add SeedKey Entry to the SeedKeyTable */
/* input param:     device ext. Address, localSecDevice
 *
 * ouput param:     true = added to the table, false = either already exists or mem allocation fail
 */
extern SM_AddResult_t addEntry2SeedKeyTable (ApiMac_sAddrExt_t extAddress, uint16_t shortAddress, uint8_t *seedKey);

#ifndef FEATURE_FULL_FUNCTION_DEVICE
/* Add SeedKey Entry to the SeedKeyTable from NV*/
/* input param:     Device info, Device Key Info
 *
 * ouput param:     true = added to the table, false = either already exists or mem allocation fail
 */
extern SM_AddResult_t updateSeedKeyFromNV (ApiMac_deviceDescriptor_t *pDevInfo,  nvDeviceKeyInfo_t *pDevKeyInfo);
#endif

/* Add a new device key to key table*/
/* input param:     device ext. Address, localSecDevice
 *
 * ouput param:     true = added to the table, false = either already exists or mem allocation fail
 */
extern void addDeviceKey (SM_seedKey_Entry_t* pSeedKeyEntry, uint8_t *deviceKey, bool newkey);
/* Get a SeedKey Entry pointer from the SeedKeyTable*/
/* input param:     device ext. Address + short address
 *
 * ouput param:    pointer of a SeedKey Entry that matches the device ext. address, NULL => cannot find the entry
 */
extern SM_seedKey_Entry_t * getEntryFromSeedKeyTable (ApiMac_sAddrExt_t extAddress, uint16_t shortAddress);

#ifndef FEATURE_FULL_FUNCTION_DEVICE
/*!
 * @brief       recover key
 *
 * @param       devInfo - pointer to the API MAC device information block
 * @param       parentInfo - The LLC parent information block
 * @param       devKeyInfo - Key info
 *
 */
extern void SM_recoverKeyInfo(ApiMac_deviceDescriptor_t devInfo, Llc_netInfo_t parentInfo, nvDeviceKeyInfo_t devKeyInfo);
#endif

/* Set the passkey */
/* input param:     passkey
 *
 * ouput param:     none
 */
extern void SM_setPasskey (uint32_t passkey);

//*****************************************************************************
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif /*FEATURE_SECURE_COMMISSIONING*/
#endif /* SM_TI154_H */
