/*
 * Copyright 2019-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

/*
 * @section ni-control-main Network Control interface
 * Interfaces exposed by Network Interface required to control network status
 *  and operates accordingly.
 *
 * @subsection ni-control-problem Problem and requirements definition
 * TBD
 * @subsection ni-control-tenets Tenets
 * TBD
 * @subsection ni-control-design Design
 * TBD
 *
 * @note All functions return numerical value with operation status, for more details @see sid_error_t
 */

#ifndef SID_NETWORK_CONTROL_IFC_H
#define SID_NETWORK_CONTROL_IFC_H

#include <sid_error.h>
#include <sid_network_data_ifc.h>

#include <lk/list.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Link_Type_1 connection statistics */
struct sid_link_1_connection_statistics {
    /** time spent by link advertising its presence */
    uint32_t advertisement_duration_seconds;
};

/** Link_Type_2 connection statistics */
struct sid_link_2_connection_statistics {
    /** time spent by link in establishing connection to the gateway */
    uint32_t seconds_spent_in_connection_establishment;
};

/** Link_Type_2 connection statistics */
struct sid_link_3_connection_statistics {
    /** Number of control packets sent to check for link connection status */
    uint32_t control_packets_sent;
    /** Number of responses received to the control packets sent */
    uint32_t control_packets_resp_received;
};

/** Link specific connection statistics */
union sid_link_specific_connection_statistics {
    /** Link Type_1 connection statistics */
    struct sid_link_1_connection_statistics link1_stats;
    /** Link Type_2 connection statistics */
    struct sid_link_2_connection_statistics link2_stats;
    /** Link Type_3 connection statistics */
    struct sid_link_3_connection_statistics link3_stats;
};

/** Link connection statistics */
struct sid_link_connection_statistics {
    /** link in connected state duration */
    uint32_t connection_duration_seconds;
    /** link in disconnected state duration */
    uint32_t disconnected_duration_seconds;
    /** Number of successful connection attempts */
    uint32_t connection_success;
    /** Number of connection attempts resulted in failure */
    uint32_t connection_fail;
    /** link specific connection statistics*/
    union sid_link_specific_connection_statistics link_stats;
};

/** link statistics for the link */
struct sid_link_statistics {
    /** link statistics for the link type */
    uint8_t link_type;
    /** number of messages sent over the link type */
    uint32_t tx;
    /** number of messages failed to sent over the link type */
    uint32_t tx_fail;
    /** number of messages retries */
    uint32_t retries;
    /** number of messages received */
    uint32_t rx;
    /** number of messages that failed to be received over the link type */
    uint32_t rx_fail;
    /** number of link layer retries */
    uint32_t link_retries;
    /** number of duplicates received */
    uint32_t duplicates;
    /** Link connection statistics */
    struct sid_link_connection_statistics link_conn_stats;
};

// Forward declarations, details are below
typedef struct network_interface_control_ifc_s* network_interface_control_ifc;

/**
 * States which can be reported from Network Interface to subscriber (Policy Manager)
 */
typedef enum network_interface_control_connection_state_e {
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_DISCONNECTED,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_CONNECTED,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_TIMEOUT,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_SYNCED,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_GCS_REQUEST,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_DEVICE_PARAM_CHANGE,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_NW_SYNC_REQUEST,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_PROV_SYNCED_FAILED,
    NETWORK_INTERFACE_CONTROL_CONNECTION_STATE_MAX, /** This has to be last */
} network_interface_control_connection_state;

enum network_interface_info_req_type {
    NETWORK_INTERFACE_INFO_REQ_TYPE_SCH_LOOKUP,
    NETWORK_INTERFACE_INFO_REQ_TYPE_SCH_ASSIGN,
    NETWORK_INTERFACE_INFO_REQ_TYPE_SCH_EXT,
    NETWORK_INTERFACE_INFO_REQ_TYPE_SCH_TERM_NTFY,
    NETWORK_INTERFACE_INFO_REQ_TYPE_FSK_WAN_GW_JOIN_RESP,
    NETWORK_INTERFACE_INFO_REQ_TYPE_SEND_NACK,
    NETWORK_INTERFACE_INFO_REQ_TYPE_MAX,   // This has to be last
};

typedef struct {
    list_node_t node;
    enum network_interface_info_req_type type;
    uint8_t *data;
    uint16_t data_sz;
} sid_network_ctrl_req_msg_t;

/**
 * Enumeration of possible power operation modes for Networking subsystem.
 *
 * Each value will request Network Policy Manager to change behavior accordingly. To get or update operation in RPM please
 * check @ref radio_policy_manager_ifc.set_mode and @ref radio_policy_manager_ifc.get_mode
 * or @ref network_interface_control_ifc.set_mode
 */
typedef enum network_power_operation_mode_e {
    NETWORK_POWER_OPERATION_MODE_POWERCRITICAL,    /**< Put RPM in mode with highest power save mode, reducing all operations to minimum
                                                        to reduce power consumption as much as possible*/
    NETWORK_POWER_OPERATION_MODE_POWERSAVE,        /**< Requests from RPM to operate in most power efficient mode: choose connection which
                                                        will provide connectivity with minimal power consumption. Other activities on
                                                        reconnection and lookup for better connection should be minimized to save power*/
    NETWORK_POWER_OPERATION_MODE_BALANCED,         /**< RPM should choose network which will provide best performance with reasonable power
                                                        consumption, run moderate activity to connect to more efficient network */
    NETWORK_POWER_OPERATION_MODE_NETWORKPIORITY,   /**< NMP Should provide maximum connectivity performance and availability despite of
                                                        power consumption */
    NETWORK_POWER_OPERATION_MODE_JOIN,             /**< A defined operation mode to provides connection that allows join procedure to
                                                        complete. The operation mode will be restored upon join completion */
    _NETWORK_POWER_OPERATION_MODE_COUNT
} network_power_operation_mode;

enum {
    NETWORK_INTERFACE_LINK_TYPE_UNDEFINED,
    NETWORK_INTERFACE_LINK_TYPE_900MHZ,
    NETWORK_INTERFACE_LINK_TYPE_BLE,
    NETWORK_INTERFACE_LINK_TYPE_HOST,
};

/**
 * Enumeration of operations that might be supported by NI
 */
typedef enum {
    NETWORK_INTERFACE_IOCTL_OPERATION_CHANNEL_SURVEY = 0x1,
    NETWORK_INTERFACE_IOCTL_OPERATION_LINK_TYPE_GET = 0x2,
    NETWORK_INTERFACE_IOCTL_OPERATION_READ_STATS = 0x3,
    NETWORK_INTERFACE_IOCTL_OPERATION_ON_STATS_ROLLOVER_SET = 0x4,
    NETWORK_INTERFACE_IOCTL_OPERATION_GCS_STAT_WRITE = 0x5,
    NETWORK_INTERFACE_IOCTL_OPERATION_HANDLE_TIMESYNC = 0x6,
    NETWORK_INTERFACE_IOCTL_OPERATION_START_ADV = 0x7,
    NETWORK_INTERFACE_IOCTL_OPERATION_STOP_ADV = 0x8,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_PROV_STATE = 0x9,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_BATTERY_LEVEL = 0x10,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_LOCAL_ADDRESS = 0x11,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_DEDICATED_LINK_PARAMS = 0x12,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_NW_SYNC_EVENT = 0x13,
    NETWORK_INTERFACE_IOCTL_OPERATION_GCS_TIME_RESPONSE = 0x14,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_FSK_WAN_LINK_PARAMS = 0x15,
    NETWORK_INTERFACE_IOCTL_OPERATION_HANDLE_FSK_WAN = 0x16,
    NETWORK_INTERFACE_IOCTL_OPERATION_GWD_START_FILTER_SESSION = 0x17,
    NETWORK_INTERFACE_IOCTL_OPERATION_GWD_STOP_FILTER_SESSION = 0x18,
    NETWORK_INTERFACE_IOCTL_OPERATION_GWD_ADD_BLOCKLIST = 0x19,
    NETWORK_INTERFACE_IOCTL_OPERATION_GWD_SDB_MAC_SET_PARAM = 0x1a,
    NETWORK_INTERFACE_IOCTL_OPERATION_GWD_SDB_MAC_SET_STATE = 0x1b,
    NETWORK_INTERFACE_IOCTL_OPERATION_SET_DEV_PARAM_SYNC_STATUS = 0x1c,
    NETWORK_INTERFACE_IOCTL_OPERATION_GET_STATS = 0x1d,
    NETWORK_INTERFACE_IOCTL_OPERATION_UPDATE_NODE_GROUP = 0x1e,
    NETWORK_INTERFACE_IOCTL_OPERATION_DISABLE_INACTIVITY_TIMER = 0x1f,
    // Extend if required
} network_interface_ioctl_operation;

struct sid_network_interface_connect_config {
    uint32_t timeout_ms;
    bool is_soft;
};

/**
 * Numerical value to present link quality.
 *
 * It is expected that network interface will report link metric in range of values: [0, 1000].
 * Lower number is responsible for lower quality; higher - better quality.
 *
 * Metric calculation should be based on message delivery/receive statistic, ratio of
 * successful to total operations (send, receive, including number of retries and amount of transfered data).
 * Probably calculation algorithm should include time distribution: most recent statistics should provide more
 * value for metric then statistic calculated, ie, 1 or 5 minutes ago.
 */
typedef uint16_t network_interface_linkmetric_t;

/**
 * Notification configuration which should be supplied as part of Network interface configuration
 */
typedef struct network_interface_linkmetric_param_s {
    network_interface_linkmetric_t threshold; /**< absolute change of link metric value from previous value when NI should call
                                                   @ref network_interface_control_notify_ifc.on_metric_update with updated value */
    network_interface_linkmetric_t min;       /**< minimal value of link metric to start notification about changes */
    network_interface_linkmetric_t max;       /**< maximal value of link metric to stop notification about changes */
} network_interface_linkmetric_param_t;

typedef struct network_interface_app_list_s {
    uint32_t count;
    struct sid_address * addresses_array;
} network_interface_app_list_t;

typedef struct network_interface_control_notify_ifc_s* network_interface_control_notify_ifc;

/**
 * Interface which should be provided by control application (Policy Manager) to get notified with updates from Network Interface
 */
struct network_interface_control_notify_ifc_s {
    /**
     * Notifies about a network interface connection state change
     * @param [in] _this            - reference to the interface
     * @param [in] nif              - pointer to the interface the notification is coming from
     * @param [in] connection_state - state of the connection the interface has changed to
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_connection_state_change)(const network_interface_control_notify_ifc *     _this,
                                              const network_interface_control_ifc *            nif,
                                              const network_interface_control_connection_state connection_state);
    /**
     * Notifies about update of the scan list
     *
     * @param [in] _this    reference to the interface
     * @param [in] nif      pointer to the interface the notification is coming from
     * @param [in] status   status of scan
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_scan_update)(const network_interface_control_notify_ifc * _this,
                                  const network_interface_control_ifc *        nif);
    /**
     * Notifies about finishing of the last configuration change request
     *
     * @param [in] _this    reference to the interface
     * @param [in] nif      pointer to the interface the notification is coming from
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_cfg_applied)(const network_interface_control_notify_ifc * _this,
                                  const network_interface_control_ifc *        nif);

    /**
     * Method which will be called from Network Interface to MPM with updated statistic information.
     * @see network_interface_connection_stats_t definition for more details. Method will be called
     * every time when at least one field is changed. In structure user will be provided only with
     * delta from last time.
     *
     * to avoid trembling in RPM due to very often notifications, updated should be provided if:
     *  -# change from previous value is larger than 7% of range
     *  -# once per minute if change is less then 7% or total range
     *
     * @param [in] _this    reference to the interface
     * @param [in] nif      reference to control interface which is calling RPM callback
     * @param [in] metric   numerical value which presents current link quality, @see netwotk_interface_linkmetric_t
     *                      for details
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_metric_update)(const network_interface_control_notify_ifc * _this,
                                    const network_interface_control_ifc *          nif,
                                    const network_interface_linkmetric_t           metric);

    /**
     * Notifies about Radio TX was done
     *
     * @param [in] _this       reference to the interface
     * @param [in] nif         pointer to the interface the notification is coming from
     * @param [in] info        pointer to structure with additional information
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_radio_tx)(const network_interface_control_notify_ifc* _this,
                               const network_interface_control_ifc* nif,
                               const void* info);

    /**
     * Notifies before transmitting a message
     *
     * @param [in] _this       reference to the interface
     * @param [in] nif         pointer to the interface the notification is coming from
     * @param [in] info        pointer to structure with additional information
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_tx_prepare)(const network_interface_control_notify_ifc* _this,
                                 const network_interface_control_ifc* nif,
                                 const void* info);

    /**
     * Notifies about a control packet sent up by the interface
     *
     * @param [in] _this    reference to the interface
     * @param [in] nif      pointer to the interface the notification is coming from
     * @param [in] req_type numeric value presents the control request type
     * @param [in] data     opaque data
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*on_ctrl_info_req)(const network_interface_control_notify_ifc *_this,
                                    const network_interface_control_ifc *nif,
                                    enum network_interface_info_req_type req_type,
                                    const void *data);
};

/**
 * Interface to manipulate with instance of Network Interface.
 *
 * Provides all required functionality to control NI.
 */
struct network_interface_control_ifc_s {
    /**
     * Destroys the interface an the underlying network interface instance
     * @param _this                 - reference to the interface
     */
    void (*destroy)(const network_interface_control_ifc* _this);

    /**
     * Enables or disables the interface operation
     * @param [in] _this            - reference to the interface
     * @param [in] on               - boolean value indicating the new state: true states for enable and false for disable
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*enable)(const network_interface_control_ifc* _this, bool on);

    /**
     * Enables or disables the scan operation on the interface
     * @param [in] _this            - reference to the interface
     * @param [in] on               - boolean value indicating the new state: true states for enable and false for disable
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*scan)(const network_interface_control_ifc* _this, bool on);

    /**
     * Retrieves the scan result list
     * @param [in] _this            - reference to the interface
     * @param [out] ap_list          - pointer to the list the scan results to be pushed to
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * @note The @p ap_list type and format is to be defined later
     */
    sid_error_t (*get_ap_list)(const network_interface_control_ifc* _this, network_interface_app_list_t* ap_list);

    /**
     * Requests for a connection to an entry of the scan result
     * @param [in] _this            - reference to the interface
     * @param [in] ap_list_entry    - pointer to the ap_list entry to connect to,
     *                                  NULL in case we want to reconnect to last known
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * @note The @p ap_list type and format is to be defined later
     */
    sid_error_t (*connect)(const network_interface_control_ifc *_this,
                           const struct sid_network_interface_connect_config *const config);

    /**
     * Disconnect from an ap entry value
     * @param [in] _this            - reference to the interface
     * @param [in] ap_list_entry    - pointer to the ap_list entry to dis-connect from,
     *                                  NULL in case we want to reconnect to last known
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * @note The @p ap_list type and format is to be defined later
     */
    sid_error_t (*disconnect)(const network_interface_control_ifc *_this,
                              const struct sid_network_interface_connect_config *const config);

    /**
     * Retrieves the Max Transmit Unit on an interface
     * @param [in] _this            - reference to the interface
     * @param [in] mtu              - pointer the variable the result to be provided in
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*get_mtu)(const network_interface_control_ifc* _this, uint16_t* mtu);

    /**
     * Retrieves the Network metric on an interface
     * @param [in] _this            - reference to the interface
     * @param [in] metric           - pointer the variable the result to be provided in
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*get_metric)(const network_interface_control_ifc* _this, uint16_t* metric);

    /**
     * Retrieves statistics on an interface
     * @param [in] _this            - reference to the interface
     * @param [in] statistics       - pointer the variable the result to be provided in
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * @note The @p statistics type and format is to be defined later
     */
    sid_error_t (*get_stats)(const network_interface_control_ifc* _this, void* statistics);

    /**
     * Retrieves a data path operation readiness on an interface
     * @param [in] _this            - reference to the interface
     * @param [in] ready            - pointer the variable the result to be provided in
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*is_ready)(const network_interface_control_ifc* _this, bool * ready);

    /**
     * Requests a generic part of the configuration to be applied on a network interface
     * @param [in] _this            - reference to the interface
     * @param [in] new_config       - pointer the new configuration to be applied
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * @note The @p new_config type and format is to be defined later
     */
    sid_error_t (*set_cfg)(const network_interface_control_ifc* _this, void* new_config);

    /**
     * Function to be called by RPM to update power operation mode in network interface
     *
     * @param [in] _this    Pointer to the interface instance object
     * @param [in] mode     RPM operation mode, @see network_power_operation_mode for details.
     * @param [in] ni_mode  Denotes the mode of operation (FSK PAN or LDR WAN)
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*set_mode)(const network_interface_control_ifc * _this, const network_power_operation_mode mode, void *ni_mode);

    /**
     * Function to query the mode of operation of the interface
     *
     * @param [in] _this    Pointer to the interface instance object
     * @param [out] mode_ptr     RPM operation mode, @see network_power_operation_mode for details.
     * @param [out] ni_mode  Denotes the mode of operation (FSK PAN or LDR WAN)
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*get_mode)(const network_interface_control_ifc * _this, network_power_operation_mode *mode_ptr, void *ni_mode);

    /**
     * Function to handle NI specific data.
     *  @note: not all type of operations declared in @see network_interface_ioctl_operation are required to be supported
     * and NI will return error code SID_ERROR_NOSUPPORT
     *
     * @param _this [in] reference to NI instance
     * @param type  [in] type of operation requested by caller
     * @param data  [in/out] user provided buffer with data or to store output data
     * @param size  [in/out] size of user specified buffer, returns count of bytes used/consumed by fuction
     *
     * @retval SID_ERROR_NONE - operation completed successfully
     * @retval SID_ERROR_NOSUPPORT - operation is not supported by current implementation
     * @retval SID_ERROR_OOM - in case if user data buffer is not enough to parse input data or store output, parameter size will return required buffer size
     * @retval SID_ERROR_GENERIC - something goes wrong during execution, please contact developer of a specific NI implementation
     */
    sid_error_t (*ioctl)(const network_interface_control_ifc * _this,
                         const network_interface_ioctl_operation type,
                         void* opaque_data, size_t size);

    /**
     * Activate or deactivate network synchronization
     * @param [in] _this            - reference to the interface
     * @param [in] activate         - boolean value indicating activate or deactivate
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*activate_network_sync)(const network_interface_control_ifc* _this, bool activate);
};

typedef struct network_interface_ifc_s network_interface_ifc;
/**
 * Generalized network interface
 */
struct network_interface_ifc_s {
    /**
     * Instance destructor
     * @param _this
     */
    void (*destroy) (const network_interface_ifc* _this);

    network_interface_data_ifc data_ifc;                 /**< data API*/
    network_interface_control_ifc control_ifc;           /**< control API*/
};

/**
 * Generalized type of the network interface constructor.
 *
 * @param opaque_implementation_specific_config  pointer to the opaque implementation-specific configuration for the network interface to
 *                                                 be created with
 * @param ctrl_ntfy_interface                    pointer to the control notification interface to be bound to the network interface for
 *                                                 control notifications
 * @return
 */
typedef sid_error_t (*network_interface_ctor_t)(
        const network_interface_ifc ** network_interface,
        const void * opaque_implementation_specific_config,
        const network_interface_control_notify_ifc * ctrl_ntfy_interface);

#ifdef __cplusplus
}
#endif

#endif
