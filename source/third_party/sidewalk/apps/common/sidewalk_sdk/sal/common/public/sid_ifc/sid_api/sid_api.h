/*
 * Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef SID_API_H
#define SID_API_H

/** @file
 *
 * @defgroup SIDEWALK_API Sidewalk API
 * @brief API for communicating with the Sidewalk network
 * @{
 * @ingroup  SIDEWALK_API
 */

#include <sid_time_types.h>
#include <sid_error.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Describes the link types supported by the Sidewalk library.
 *
 * Note: Previously SID_LINK_TYPE_BLE used here now maps to SID_LINK_TYPE_1.
 * The change is done to abstract link names from physical link types since more
 * combinations are expected to be added in the future.
 */
enum sid_link_type {
    /** Bluetooth Low Energy link */
    SID_LINK_TYPE_1 = 1 << 0,
    /** 900 MHz link for FSK */
    SID_LINK_TYPE_2 = 1 << 1,
    /** 900 MHz link for LORA */
    SID_LINK_TYPE_3 = 1 << 2,
    /** Any Link Type */
    SID_LINK_TYPE_ANY = INT_MAX,
};

/**
 * Describes the link modes supported on each link type.
 *
 * Link mode determines the destination of the messages that the you can send on a link.
 *
 */
enum sid_link_mode {
    /** Messages can be sent to cloud only, when a link type notfies support of this mode */
     SID_LINK_MODE_CLOUD = 1 << 0,
    /** Messages can be sent to mobile only, when a link type notfies support of this mode */
     SID_LINK_MODE_MOBILE = 1 << 1,
    /** Invalid mode */
     SID_LINK_MODE_INVALID = INT_MAX,
};

enum sid_link_type_idx {
    /** Bluetooth Low Energy link */
    SID_LINK_TYPE_1_IDX = 0,
    /** 900 MHz link for FSK */
    SID_LINK_TYPE_2_IDX = 1,
    /** 900 MHz link for LORA */
    SID_LINK_TYPE_3_IDX = 2,
    /** sid max supported links */
    SID_LINK_TYPE_MAX_IDX,
};

/**
 * Describes the Registration status of the Sidewalk library.
 */
enum sid_registration_status {
    /** Used to indicate  Sidewalk library is registered with Sidewalk cloud services */
    SID_STATUS_REGISTERED = 0,

    /** Used to indicate  Sidewalk library is not registered with Sidewalk cloud services */
    SID_STATUS_NOT_REGISTERED = 1,
};

/**
 * Describes the Time synchronization status of the Sidewalk library with the cloud services.
 * Sidewalk security relies on the synchronization of time between the cloud services and the
 * end device.
 *
 * @note Sidewalk library can have time synchronization with cloud service only when the device
 * is registered
 *
 */
enum sid_time_sync_status {
    /** Used to indicate Sidewalk library is registered with sidewalk cloud services */
    SID_STATUS_TIME_SYNCED = 0,

    /** Used to indicate Sidewalk library is not time synchronized with cloud services */
    SID_STATUS_NO_TIME = 1,
};

/**
 * Describes the link connection status with the gateway device
 *
 */
struct sid_status_detail {
    /**
     * Used to indicate which link is up, if the bit corresponding to a link
     * is set it is up otherwise it is down. For example to check if SID_LINK_TYPE_1
     * is up, !!(link_status_mask & SID_LINK_TYPE_1) needs to be true
     *
     * supported link modes indicate the modes supported by each link. A link type may
     * support more than one mode simultaneously
     *
     */
    enum sid_registration_status registration_status;
    enum sid_time_sync_status time_sync_status;
    uint32_t link_status_mask;
    uint32_t supported_link_modes[SID_LINK_TYPE_MAX_IDX];
};

/**
 * Describes the state of the Sidewalk library.
 */
enum sid_state {
    /** Used when the Sidewalk library is ready to send and receive messages */
    SID_STATE_READY = 0,
    /** Used when the Sidewalk library is unable to send or receive messages, such as when the device
     * is not registered or link gets disconnected or time is not synced */
    SID_STATE_NOT_READY = 1,
    /** Used when the Sidewalk library encountered an error. Use sid_get_error() for a diagnostic
     * error code */
    SID_STATE_ERROR = 2,
    /** Used when the Sidewalk library is ready to send and receive messages only with secure channel
     * establishment completed but device is not registered and time is not synced */
    SID_STATE_SECURE_CHANNEL_READY = 3,
};

/**
 * Describes the status of the Sidewalk library.
 */
struct sid_status {
    /** The current state */
    enum sid_state state;
    /** Details of Sidewalk library status */
    struct sid_status_detail detail;
};

/**
 * The Sidewalk library message types.
 * The messages from cloud services to the End device are designated as "Downlink Messages"
 * The messages from End device to Cloud services are designated as "Uplink Messages"
 */
enum sid_msg_type {
    /** #SID_MSG_TYPE_GET is used by the sender to retrieve information from the receiver, the sender
     * expects a mandatory response from the receiver. On reception of #SID_MSG_TYPE_GET, the receiver
     * is expected to send a message with type #SID_MSG_TYPE_RESPONSE with the same message id it received
     * from the message type #SID_MSG_TYPE_GET.
     * This is to ensure the sender can map message type #SID_MSG_TYPE_GET with the received #SID_MSG_TYPE_RESPONSE.
     * Both uplink and downlink messages use this message type.
     * @see sid_put_msg().
     * @see on_msg_received in #sid_event_callbacks.
     */
    SID_MSG_TYPE_GET = 0,
    /** #SID_MSG_TYPE_SET indicates that the sender is expecting the receiver to take an action on receiving the
     * message and the sender does not expect a response.
     * #SID_MSG_TYPE_SET type is used typically by Cloud services to trigger an action to be preformed by the End device.
     * Typical users for this message type are downlink messages.
     */
    SID_MSG_TYPE_SET = 1,
    /** #SID_MSG_TYPE_NOTIFY is used to notify cloud services of any periodic events or events
     * triggered/originated from the device. Cloud services do not typically use #SID_MSG_TYPE_NOTIFY as the
     * nature of messages from cloud services to the devices are explicit commands instead of notifications.
     * Typical users for this message type are uplink messages.
     */
    SID_MSG_TYPE_NOTIFY = 2,
    /** #SID_MSG_TYPE_RESPONSE is sent as a response to the message of type #SID_MSG_TYPE_GET.
     * The sender of #SID_MSG_TYPE_RESPONSE is required to the copy the message id received in
     * the message of type #SID_MSG_TYPE_GET.
     * Both uplink and downlink messages use this message type.
     * @see sid_put_msg().
     * @see on_msg_received in #sid_event_callbacks.
     */
    SID_MSG_TYPE_RESPONSE = 3,
};

/** Attributes applied to the message descriptor on tx */
struct sid_msg_desc_tx_attributes {
    /** Whether this message requests an ack from the AWS IOT service */
    bool request_ack;
    /** Number of retries the Sidewalk stack needs to preform in case the
     * ack is not received. Setting not applicable if request_ack is set to false
     */
    uint8_t num_retries;
    /** Total time the sidewalk stack holds the message in its queue in case
     * the ack is not received. Setting not applicable if request_ack is set to false
     */
    uint16_t ttl_in_seconds;
};

/** Attributes with which the message is received */
struct sid_msg_desc_rx_attributes {
    /** Whether the message received is an acknowledgement. Acknowledgements have the same
     * message id as that of message sent but with zero payload size . See #sid_msg_desc_tx_attributes
     */
    bool is_msg_ack;
    /** Whether the message received is a duplicate. If a message arrives at the sidewalk stack
     * with message id and payload size equal to an already reported message, this message is marked as a duplicate
     * See #SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES
     */
    bool is_msg_duplicate;
    /** Whether the message received has requested an acknowledgement to be sent, Acknowledgements have the
     * same message id as that of the received message. Sidewalk stack immediately queues an acknowledgement
     * to the sender before propagating this message to the user*/
    bool ack_requested;
    /** rssi of the received message */
    int8_t rssi;
    /** snr of the received message */
    int8_t snr;
};

union sid_msg_desc_attributes {
    /** Attributes that are applied only when message is transmitted. See #sid_put_msg */
    struct sid_msg_desc_tx_attributes tx_attr;
    /** Attributes reported per message when the message is reported to the user. See #on_msg_received */
    struct sid_msg_desc_rx_attributes rx_attr;
};

/**
 * A message descriptor given by the Sidewalk library to identify a message.
 */
struct sid_msg_desc {
    /** The link type on which this message is to be sent or was received from */
    uint32_t link_type;
    /** The message type */
    enum sid_msg_type type;
    /** The link mode on which message is sent or received */
    enum sid_link_mode link_mode;
    /** The id associated with a message, generated by the Sidewalk library
     * The maximum value the id can take is 0x3FFF after which the id resets to 1
     */
    uint16_t id;
    /** Attributes applied to the message */
    union sid_msg_desc_attributes msg_desc_attr;
};

/**
 * Describes a message payload.
 */
struct sid_msg {
    void *data;
    size_t size;
};

/**
 * The set of options to be used with sid_option API.
 */
enum sid_option {
    /** Option to configure the advertised battery level. Value is a uint8_t, 0-100 */
    SID_OPTION_BLE_BATTERY_LEVEL = 0,
    /** Option to configure the device profile. Value is of type struct sid_device_profile */
    SID_OPTION_900MHZ_SET_DEVICE_PROFILE = 1,
    /** Option to get the device profile configuration. Value is of type struct sid_device_profile */
    SID_OPTION_900MHZ_GET_DEVICE_PROFILE = 2,
    /** Option to set the message policy to filter duplicates, value is 0 or 1, when set to 0, the default
     * setting, duplicates are filtered and are not propagated using #on_msg_received, when set to 1,
     * duplicates are detected and are propagated using #on_msg_receive, see #sid_msg_desc_rx_attributes */
    SID_OPTION_SET_MSG_POLICY_FILTER_DUPLICATES = 3,
    /** Option to get the configured policy of filtering duplicates, 0 - Filter duplicates, 1 - Allow duplicates */
    SID_OPTION_GET_MSG_POLICY_FILTER_DUPLICATES = 4,
    /** Delimiter to enum sid_option*/
    SID_OPTION_LAST,
};

/**
 * Option to get current time.
 *
 * GPS time is the time in milliseconds since 6/1/1980. GPS time is not
 * adjusted by leap seconds. UTC time is the number of milliseconds since
 * 1/1/1970. Local time is the adjusted time taking timezone into consideration.
 */
enum sid_time_format {
    /** Option to get current gps time */
    SID_GET_GPS_TIME = 0,
    /** Option to get current utc time. NOT SUPPORTED at this time */
    SID_GET_UTC_TIME = 1,
    /** Option to get current local time. NOT SUPPORTED at this time */
    SID_GET_LOCAL_TIME = 2,
};

/**
 * The set of callbacks a user can register through sid_init().
 */
struct sid_event_callbacks {
    /** A place where you can store user data */
    void *context;

    /**
     * Callback to invoke when any Sidewalk event occurs.
     *
     * The Sidewalk library invokes this callback when there is at least one event to process,
     * including internal events. Upon receiving this callback you are required to schedule a call
     * to sid_process() within your main loop or running context.
     *
     * @warning sid_process() MUST NOT be called from within the #on_event callback to avoid
     * re-entrancy and recursion problems.
     *
     * @see sid_process
     *
     * @param[in] in_isr  true if invoked from within an ISR context, false otherwise.
     * @param[in] context The context pointer given in sid_event_callbacks.context
     */
    void (*on_event)(bool in_isr, void *context);

    /**
     * Callback to invoke when a message from the Sidewalk network is received.
     *
     * @warning sid_put_msg() MUST NOT be called from within the #on_msg_received callback
     * to avoid re-entrancy and recursion problems.
     *
     * @param[in] msg_desc A pointer to the received message descriptor, which is never NULL.
     * @param[in] msg      A pointer to the received message payload, which is never NULL.
     * @param[in] context  The context pointer given in sid_event_callbacks.context
     */
    void (*on_msg_received)(const struct sid_msg_desc *msg_desc, const struct sid_msg *msg, void *context);

    /**
     * Callback to invoke when a message was successfully delivered to the Sidewalk network.
     *
     * @param[in] msg_desc A pointer to the sent message descriptor, which is never NULL.
     * @param[in] context  The context pointer given in sid_event_callbacks.context
     */
    void (*on_msg_sent)(const struct sid_msg_desc *msg_desc, void *context);

    /**
     * Callback to invoke when a queued message failed to be delivered to the Sidewalk network.
     *
     * A user can use this notification to schedule retrying sending a message or invoke other error
     * handling.
     *
     * @see sid_put_msg
     *
     * @warning sid_put_msg() MUST NOT be called from within the #on_send_error callback to
     * avoid re-entrancy and recursion problems.
     *
     * @param[in] error    The error code associated with the failure
     * @param[in] msg_desc A pointer to the unsent message descriptor, which is never NULL.
     * @param[in] context  The context pointer given in sid_event_callbacks.context
     */
    void (*on_send_error)(sid_error_t error, const struct sid_msg_desc *msg_desc, void *context);

    /**
     * Callback to invoke when the Sidewalk library status changes.
     *
     * Once sid_start() is called, a #SID_STATE_READY status indicates the library is ready to
     * accept messages sid_put_msg().
     *
     * When receiving #SID_STATE_ERROR, you can call sid_get_error() from within the
     * #on_status_changed callback context to obtain more detail about the error condition.
     * Receiving this status means the Sidewalk library encountered a fatal condition and won't be
     * able to proceed. Hence, this notification is mostly for diagnostic purposes.
     *
     * @param[in] status  The current status, valid until the next invocation of this callback.
     * @param[in] context The context pointer given in sid_event_callbacks.context
     */
    void (*on_status_changed)(const struct sid_status *status, void *context);

    /**
     * Callback to invoke when the Sidewalk library receives a factory reset from the cloud service.
     *
     * On receiving the factory reset from the cloud service, the sidewalk library clears its
     * configuration from the non volatile storage and reset its state accordingly.
     * The sidewalk link status resets to #SID_STATE_DISABLED.
     * This callback is then called by the sidewalk library to notify you to handle the factory reset command
     *
     * The device needs to successfully complete device registration with the cloud services for the sidewalk
     * library to send and receive messages
     *
     * @param[in] context The context pointer given in sid_event_callbacks.context
     */
    void (*on_factory_reset)(void *context);
};

/**
 * Describes the configuration associated with the chosen link.
 */
struct sid_config {
    /** link_mask encodes the sidewalk links that device will initialize and use.
     * For configuring a single link,eg  #SID_LINK_TYPE_1, #link_mask = SID_LINK_TYPE_1,
     * For configuring multi-link, i.e #SID_LINK_TYPE_1 and #SID_LINK_TYPE_3,
     * link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_3
     * #SID_LINK_TYPE_ANY is an invalid option for sid_config
     */
    uint32_t link_mask;
    /** time sync request periodicity. The sidewalk security is based on time and periodically
     * requests Sidewalk time service for time. This config determines the periodicity at which
     * the Sidewalk stack request Sidewalk time service for time
     */
    uint32_t time_sync_periodicity_seconds;
    /** The event callbacks associated with the chosen link.
     *  Must use static const storage as member is accessed by SID API
     *  implementation without being copied.
     */
    struct sid_event_callbacks *callbacks;
    /** BLE link specific configuration.
     *  For all link_type pass a `struct sid_ble_link_config` pointer
     *  This is needed because registration is done over BLE. Must use
     *  static storage as member is accessed by SDI API implementation
     *  without being copied.
     */
    const struct sid_ble_link_config *link_config;
    /** Sub-Ghz link specific configuration
     *  This is needed to enable or disable reporting of metrics to Sidewalk cloud service
     *  and configuration to registration over Sub-Ghz link. Must use static storage
     *  as memer is accessed by SID API implementation without being copied.
     */
    const struct sid_sub_ghz_links_config *sub_ghz_link_config;
};

/**
 * Opaque handle returned by sid_init().
 */
struct sid_handle;

/**
 * Initializes the Sidewalk library for the chosen link type.
 *
 * sid_init() can only be called once for the given sid_config.link_type unless sid_deinit() is
 * called first.
 *
 * @see sid_deinit
 *
 * @param[in] config  The required configuration in order to properly initialize sidewalk for the
 *                    chosen link type.
 * @param[out] handle A pointer where the the opaque handle type will be stored. `handle` is set to
 *                    NULL on error.
 *
 * @returns #SID_ERROR_NONE                on success.
 * @returns #SID_ERROR_ALREADY_INITIALIZED if Sidewalk was already initialized for the given link
 *                                         type.
 */
sid_error_t sid_init(const struct sid_config *config, struct sid_handle **handle);

/**
 * De-initialize the portions of the Sidewalk library associated with the given handle.
 *
 * @see sid_init
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success
 */
sid_error_t sid_deinit(struct sid_handle *handle);

/**
 * Makes the Sidewalk library start operating.
 *
 * The notifications registered during sid_init() are invoked once sid_start() is called.
 *
 * On an unregistered Sidewalk end node sid_start() will put the Sidewalk library into a
 * state ready for device registration, typically the device is registered with a separate
 * mobile application or MCU SDK tool.
 *
 * After registration the Sidewalk end node will receive a time sync message from an accessible
 * Sidewalk gateway. This is a prerequisite to establish up-link or down-link data connectivity.
 * This is visible to the api user by #SID_STATE_READY and #SID_STATUS_TIME_SYNCED.
 *
 * If sid_stop() is called after receiving #SID_STATE_READY the Sidewalk library will cache the
 * time sync for a minimum of SID_TIME_SYNC_MIN_PERIOD seconds. This value should not be
 * modified by the API user.
 *
 * sid_start() can be used to start one or more links at once.
 * To start a single link, eg #SID_LINK_TYPE_1, link_mask = SID_LINK_TYPE_1,
 * To start more than one link, eg #SID_LINK_TYPE_1 and #SID_LINK_TYPE_3 ,
 * link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_3
 *
 * @note User can only start a link_type that was initialized in sid_init()
 *
 * @see sid_stop
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] link_mask The links that need to started.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_start(struct sid_handle *handle, uint32_t link_mask);

/**
 * Makes the Sidewalk library stop operating.
 *
 * No messages will be sent or received and no notifications will occur after sid_stop() is called.
 * Link status will be changed to disconnected and time sync status is cached after sid_stop() is
 * called.
 *
 * sid_stop() can be used to stop one or more links at once.
 * To stop a single link, eg #SID_LINK_TYPE_1, link_mask = SID_LINK_TYPE_1,
 * To stop more than one link, eg #SID_LINK_TYPE_1 and #SID_LINK_TYPE_3 ,
 * link_mask = SID_LINK_TYPE_1 | SID_LINK_TYPE_3
 *
 * @note User can only stop a link_type that was initialized in sid_init()
 *
 * @see sid_start
 *
 * @warning sid_stop() should be called in the same caller context as sid_process().
 *
 * @warning sid_stop() must not be called from within the caller context of any of the
 * sid_event_callbacks registered during sid_init() to avoid re-entrancy and recursion problems.
 *
 * @warning RTC should not be stopped after sid_stop() is called.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[in] link_mask The links that need to be stopped.
 *
 * @returns #SID_ERROR_NONE in case of success.
 */
sid_error_t sid_stop(struct sid_handle *handle, uint32_t link_mask);

/**
 * Process Sidewalk events.
 *
 * When there are no events to process, the function returns immediately.
 * When events are present, sid_process() invokes the sid_event_callbacks registered during
 * sid_init() within its calling context. You may not receive any callbacks for internal events.
 *
 * You are required to schedule sid_process() to run within your main-loop or running context when
 * the sid_event_callbacks.on_event callback is received.
 *
 * Although not recommended for efficiency and power usage reasons, sid_process() can also be called
 * even if sid_event_callbacks.on_event has not been received, to support main loops that operate in
 * a polling manner.
 *
 * @warning sid_process() must not be called from within the caller context of any of the
 * sid_event_callbacks registered during sid_init() to avoid re-entrancy and recursion problems.
 *
 * @see sid_init
 * @see sid_start
 * @see sid_event_callbacks
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns #SID_ERROR_NONE in case of success.
 * @returns #SID_ERROR_STOPPED if sid_start() has not been called.
 */
sid_error_t sid_process(struct sid_handle *handle);

/**
 * Queues a message.
 *
 * @note msg_desc can be used to correlate this message with the sid_event_callbacks.on_msg_sent
 * and sid_event_callbacks.on_send_error callbacks.
 *
 * @note When sending #SID_MSG_TYPE_RESPONSE in response to #SID_MSG_TYPE_GET, the user is expected to fill
 * the id field of message descriptor with id from the corresponding #SID_MSG_TYPE_GET message descriptor.
 * This allows the sid_api to match each unique #SID_MSG_TYPE_RESPONSE with #SID_MSG_TYPE_GET.
 *
 * @param[in]  handle   A pointer to the handle returned by sid_init()
 * @param[in]  msg      The message data to send
 * @param[out] msg_desc The message descriptor this function fills which identifies this message.
 *                      Only valid when #SID_ERROR_NONE is returned.
 *
 * @returns #SID_ERROR_NONE when the message is successfully placed in the transmit queue.
 * @returns #SID_ERROR_TRY_AGAIN when there is no space in the transmit queue.
 */
sid_error_t sid_put_msg(struct sid_handle *handle, const struct sid_msg *msg, struct sid_msg_desc *msg_desc);

/**
 * Get the current error code.
 *
 * When the sid_event_callbacks.on_status_changed callback is called with a #SID_STATE_ERROR
 * you can use this function to retrieve the detailed error code. The error code will only be valid
 * in the calling context of the sid_event_callbacks.on_status_changed callback.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 *
 * @returns The current error code
 */
sid_error_t sid_get_error(struct sid_handle *handle);

/**
 * Gets the MTU associated with the given link_type.
 *
 * @param[in]  handle    A pointer to the handle returned by sid_init()
 * @param[in]  link_type The link type to query
 * @param[out] mtu       A pointer to store the MTU size for the given link_type
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_get_mtu(struct sid_handle *handle, enum sid_link_type link_type, size_t *mtu);

/**
 * Set an option
 *
 * @see sid_option
 * @param[in] handle A pointer to the handle returned by sid_init
 * @param[in] option The option to set
 * @param[in/out] data   A pointer to the memory for input/output data associated with the option
 * @param[in] len    The size of the data array
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_option(struct sid_handle *handle, enum sid_option option, void *data, size_t len);

/**
 * set factory reset
 *
 * Inform the sidewalk stack the factory reset event.
 * The sidewalk library clears its configuration from the non volatile storage and reset its state accordingly.
 * The sidewalk link status resets to #SID_STATE_DISABLED.
 * The device needs to successfully complete device registration with the cloud services for the sidewalk
 * library to sned and receive messages
 *
 * @param[in] handle A pointer to the handle returned by sid_init
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_set_factory_reset(struct sid_handle *handle);

/**
 * Using this API, the device can request that the Sidewalk gateway initiates a connection to the
 * device while the device is advertising via BLE (Sidewalk beaconing). After a connection is
 * dropped the user has to set this beacon state again. Gateways may not always be able to honor
 * this request depending on the number of devices connected to it.
 *
 * @param[in]  handle    A pointer to the handle returned by sid_init().
 * @param[in]  set       Set or clear the connection request in BLE advertising packet.
 *
 * @returns #SID_ERROR_NONE on success.
 * @returns #SID_ERROR_ALREADY_EXISTS when device is already connected to a Sidewalk gateway
 *          and connection request is set.
 */
sid_error_t sid_ble_bcn_connection_request(struct sid_handle *handle, bool set);

/**
 * Set destination ID for messages.
 *
 * By default, the destination ID is set to #SID_MSG_DESTINATION_AWS_IOT_CORE unless
 * changed by sid_set_msg_dest_id(). The destination ID is retained until the device
 * resets or its changed by another invocation of sid_set_msg_dest_id().
 *
 * @see sid_put_msg().
 *
 * @param[in] handle A pointer to the handle returned by sid_init().
 * @param[in] id     The new destination ID.
 *
 * @returns #SID_ERROR_NONE on success.
 */
sid_error_t sid_set_msg_dest_id(struct sid_handle *handle, uint32_t id);

/**
 * Get current status from Sidewalk library.
 *
 * @warning sid_get_status() should be called in the same caller context as sid_process().
 *
 * @warning sid_get_status() must not be called from within the caller context of any of the
 * sid_event_callbacks registered during sid_init() to avoid re-entrancy and recursion problems.
 *
 * @param[in] handle A pointer to the handle returned by sid_init()
 * @param[out] current_status A pointer to store the sdk current status
 *
 * @returns #SID_ERROR_NONE in case of success.
 * @returns #SID_ERROR_INVALID_ARGS when Sidewalk library is not initialized.
 */
sid_error_t sid_get_status(struct sid_handle *handle, struct sid_status *current_status);

/**
 * Get time from the Sidewalk library with the requested format.
 *
 * @warning sid_get_time() should be called in the same caller context as sid_process().
 *
 * @warning sid_get_time() must not be called from within the caller context of any of the
 * sid_event_callbacks registered during sid_init() to avoid re-entrancy and recursion problems.
 *
 * @param[in] handle A pointer to the handle returned by sid_init().
 * @param[in] format The time format to query
 * @param[out] curr_time A pointer to store the current time in sid_timespec format.
 *
 * @returns #SID_ERROR_NONE in case of success.
 * @returns #SID_ERROR_INVALID_ARGS when sidewalk is not initialized or not
 *          registered or invalid format is supplied.
 * @returns #SID_ERROR_UNINITIALIZED when time is not available.
 */
sid_error_t sid_get_time(struct sid_handle *handle, enum sid_time_format format, struct sid_timespec *curr_time);

#ifdef __cplusplus
}
#endif

/** @} */

#endif
