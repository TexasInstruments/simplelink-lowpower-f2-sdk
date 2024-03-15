/*!*****************************************************************************
 *  @file       nwk_discovery.h
 *
 *  @brief      Network Discovery Module Header
 *
 *  The NwkDiscovery interface provides device independent APIs, data types,
 *  and macros for discovery the defices on a ZigBee network.
 *
 *  # Overview #
 *
 *  The The NwkDiscovery builds and maintains a link list of devices on the
 *  ZigBee network.
 *
 *  The APIs in this module serve as an interface for a client
 *  application and offers functionality initiate discovery of devices on the
 *  network.
 *
 *  # Usage #
 *
 *  To use the NwkDiscovery module the application
 *  calls the following APIs:
 *    - NwkDiscovery_init(): Initialize the OADProtocol module/task.
 *    - NwkDiscovery_Params_init():  Initialize a OADProtocol_Params structure
 *      with default values.  Then change the parameters from non-default
 *      values as needed.
 *    - NwkDiscovery_open():  Open an instance of the NwkDiscovery module,
 *      passing the initialized parameters.
 *    - NwkDiscovery_start():  This starts the Network Discovery Process.
 *
 *  The following code example opens OADProtocol, sends a FW version request
 *  and processes the response.
 *
 *  @code
 *
 *  NwkDiscovery_clientFnxs zclSampleSw_nwkDiscoveryCbs = {
 *    zclSampleSw_deviceDiscoveryCb,     //New Device Discovered
 *    zclSampleSw_postNwkDiscoveryEvent, //Post an event for the Nwk Discovery module
 *  };
 *
 * static void zclSampleSw_deviceDiscoveryCb(NwkDiscovery_device_t* newDevice)
 * {
 *   //Do something with the device info
 * }
 *
 * static void zclSampleSw_postNwkDiscoveryEvent()
 * {
    appServiceTaskEvents |= SAMPLEAPP_NWK_DISC_EVT;
    // Wake up the application thread when it waits for clock event
    Semaphore_post(appSemHandle);
 * }
 *
 * void zclSampleSw_Init(void)
 * {
 *   NwkDiscovery_init();
 *
 *   // Set Default parameters structure
 *   static NwkDiscovery_l_Params_t nwkDiscovery_params;
 *
 *   // Initialize and open the Wsn Protocol Task
 *   NwkDiscovery_params_init(&nwkDiscovery_params);
 *   nwkDiscovery_params.nwkDiscoveryPeriod = 2400; //re-discover network every 4hrs
 *   nwkDiscovery_params.deviceDiscoveryPeriod = 500; //leave 500ms between Lqi Discovery messages
 *   NwkDiscovery_open(&nwkDiscovery_params);
 *
 *   NwkDiscovery_start();
 *
 *  }
 *  @endcode
 *
 *
 *  ## Network Discovery Mechanism ##
 *
 *  [TODO: Add description]
 *
 *******************************************************************************
 */
#ifndef NwkDiscovery_H_
#define NwkDiscovery_H_

#include "zstackmsg.h"

/**
 *  @defgroup NwkDiscovery defines
 *  @{
 */

/// Max devices that can be discovered
#define NwkDiscovery_MAX_DEVICES 10 ///< Max number of devices to discover

/// Default to no periodic network discovery
#define NwkDiscovery_DEFAULT_NWK_DISCOVERY_PERIOD   0

/// Default to 500ms delay between device discovery requests
#define NwkDiscovery_DEFAULT_DEVICE_DISCOVERY_PERIOD   500

/// Device Discovery Request Timeout in ms
/*
 * This value needs to account for some worst case poll rate when communicating with
 *  sleepy end devices. The suggestion is for it to be T = 2.5 * MAX_SLEEPY_POLL_RATE
 *
 *  In this case we assume the worst case poll rate for a sleepy device in the network
 *  is 3s
 */
#define NwkDiscovery_DEVICE_DISCOVERY_TIMEOUT   7500

/// Device Discovery Request Max Retries
#define NwkDiscovery_DEVICE_DISCOVERY_MAX_RETRIES   5

/** @}*/

/// NwkDiscovery status codes
typedef enum {
    NwkDiscovery_Status_Success,   ///< Success
    NwkDiscovery_Failed,           ///< Fail
    NwkDiscovery_InvalidParam,     ///< Invalid Parameter
    NwkDiscovery_FailedMaxDevices, ///< Max devices reached
} NwkDiscovery_Status_t;

typedef struct {
    uint16_t nwkAddr;                      ///< Short Address
    zstack_LongAddr_t extendedAddr;        ///< 64 bit extended Address
    uint16_t parentAddress;                ///< Parent Address
    uint8_t rxLqi;                         ///< Receive LQI
    uint8_t lightEndPoint;                 ///< Light End Point
    zstack_LogicalTypes deviceType;        ///< Type of neighbor device
    uint8_t neighborLqiEntries;            ///< The total number of neighbors
    uint8_t neighborLqiEntriesReported;    ///< The total number of neighbors reported
    zstack_RxOnWhenIdleTypes rxOnWhenIdle; ///< Receiver On When Idle status
} NwkDiscovery_device_t;

typedef enum {
    discoveryState_free,
    discoveryState_new,
    discoveryState_lqi_rsp_rcvd,
    discoveryState_discovered,
}discoveryState_t;

typedef struct {
    List_Elem             _elem;       /* Pointer to next and previous elements. */
    NwkDiscovery_device_t discoveredDevice;
    discoveryState_t      discoveryState;
    uint32_t              retryCount;
}NwkDeviceListEntry_t;

/** @brief device info callback, called when a device is discovered
 *
 */
typedef void (*deviceDiscoveryCb_t)(NwkDiscovery_device_t*);

/** @brief function to post a client event which will call NwkDiscovery_process()
 *
 */
typedef void (*postClientNwkDiscoveryEvent_t)(void);

/** @brief OADProtocol callback table
 *
 */
typedef struct
{
    deviceDiscoveryCb_t             pfnDeviceDiscoveryCb; ///< New device discovered
    postClientNwkDiscoveryEvent_t   pfnPostClientNwkDiscoveryEventFxn; ///< Post an Event to run process in task context
} NwkDiscovery_clientFnxs;

/** @brief RF parameter struct
 *  RF parameters are used with the  NwkDiscovery_open() and NwkDiscovery_Params_t() call.
 */
typedef struct {
    uint8_t appServiceTaskId;         //Service Task ID for communication with zstack
    uint32_t nwkDiscoveryPeriod;       //Time in s to re-discover network
    uint32_t deviceDiscoveryPeriod;   //Time in ms between sending zdoMgmtLqiReq's
} NwkDiscovery_Params_t;

/** @brief  Function to initialize the NwkDiscovery_Params struct to its defaults
 *
 *  @param  params      An pointer to RF_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *     appServiceTaskId       = 0
 *     nwkDiscoveryPeriod      = 0 //No Periodic Discovery
 *     deviceDiscoveryPeriod  = 500 //500ms between device discovery messages
 *     endPointOfInterest     = {0} //no Endpoint Of interfest
 */
extern void NwkDiscovery_Params_init(NwkDiscovery_Params_t *params);

/** @brief  Function that initializes the NwkDiscovery module
 *
 */
extern void NwkDiscovery_init(void);

/** @brief  Function to open the NwkDiscovery module
 *
 *  @param  params      An pointer to NwkDiscovery_Params structure for initialization
 */
extern void NwkDiscovery_open(NwkDiscovery_Params_t *params);

/** @brief  Register client functions
 *
 *  @param  nwkDiscovery_clientFnxs  client functions
 */
extern NwkDiscovery_Status_t NwkDiscovery_registerClientFxns(NwkDiscovery_clientFnxs* pClientFnxs);

/** @brief  Function to start Nwk Discovery process
 *
 *  @param  none
 */
extern NwkDiscovery_Status_t NwkDiscovery_start(void);


/** @brief Function to process MatchDesc indications
 *
 *  @param none
 */
extern NwkDiscovery_Status_t NwkDiscovery_processMatchDescRspInd(zstackmsg_zdoMatchDescRspInd_t* pZdoMatchDescRspInd);

/** @brief  Function to process MgmtLqi indications
 *
 *  @param  none
 */
extern NwkDiscovery_Status_t NwkDiscovery_processMgmtLqiRspInd(zstackmsg_zdoMgmtLqiRspInd_t* pZdoMgmtLqiRspInd);

/** @brief  Function to process NwkDiscovery events
 *
 *  @param  none
 */
extern NwkDiscovery_Status_t NwkDiscovery_processEvents(void);

extern NwkDeviceListEntry_t* NwkDiscovery_deviceGet(uint16_t nwkAddr);

#endif /* NwkDiscovery */
