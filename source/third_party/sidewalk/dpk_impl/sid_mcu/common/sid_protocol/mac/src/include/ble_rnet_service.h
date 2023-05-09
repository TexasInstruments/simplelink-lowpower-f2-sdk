/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef BLE_RNET_H__
#define BLE_RNET_H__

#include <stdint.h>
#include <stdbool.h>
#include <sid_protocol_opts.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@brief   Macro for defining a ble_rnet_service instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_RNET_SERVICE_DEF(_name)                                                                          \
static ble_rnet_service_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_RNET_OBSERVER_PRIO,                                                     \
                     ble_rnet_on_ble_evt, &_name)

#define BLE_UUID_RNET_SERVICE 0x0001                      /**< The UUID of the RingNet Service. */

#define HEADER_LENGTH 3

/**@brief   Maximum length of data (in bytes) that can be transmitted to the peer by the RingNet service module. */
#if defined(NRF_SDH_BLE_GATT_MAX_MTU_SIZE) && (NRF_SDH_BLE_GATT_MAX_MTU_SIZE != 0)
    #define BLE_RNET_MTU_LEN (NRF_SDH_BLE_GATT_MAX_MTU_SIZE - HEADER_LENGTH)
#else
    #define BLE_RNET_MTU_LEN (BLE_GATT_MTU_SIZE_DEFAULT - HEADER_LENGTH)
    #warning NRF_SDH_BLE_GATT_MAX_MTU_SIZE is not defined.
#endif


/**@brief   Nordic UART Service event types. */
typedef enum
{
    BLE_RNET_EVT_RX_DATA,               /**< Data received. */
    BLE_RNET_EVT_TX_READY,              /**< Service is ready to accept new data to be transmitted. */
    BLE_RNET_EVT_OUTBOX_READY,          /**< Notification has been enabled. */
    BLE_RNET_EVT_OUTBOX_NOT_READY,       /**< Notification has been disabled. */
} ble_rnet_evt_type_t;


/* Forward declaration of the ble_rnet_service_t type. */
typedef struct ble_rnet_s ble_rnet_service_t;


/**@brief   RingNet Service @ref BLE_RNET_EVT_RX_DATA event data.
 *
 * @details This structure is passed to an event when @ref BLE_RNET_EVT_RX_DATA occurs.
 */
typedef struct
{
    uint8_t const * p_data;           /**< A pointer to the buffer with received data. */
    uint16_t        length;           /**< Length of received data. */
} ble_rnet_evt_rx_data_t;


/**@brief   RingNet Service event structure.
 *
 * @details This structure is passed to an event coming from service.
 */
typedef struct
{
    ble_rnet_evt_type_t type;           /**< Event type. */
    ble_rnet_service_t * p_rnet_s;                 /**< A pointer to the instance. */
    union
    {
        ble_rnet_evt_rx_data_t rx_data; /**< @ref BLE_RNET_EVT_RX_DATA event data. */
    } params;
} ble_rnet_evt_t;


/**@brief   RingNet Service event handler type. */
typedef void (*ble_rnet_data_handler_t) (ble_rnet_evt_t * p_evt);

/**@brief   RingNet Service initialization structure.
 *
 * @details This structure contains the initialization information for the service. The application
 * must fill this structure and pass it to the service using the @ref ble_rnet_service_init
 *          function.
 */
typedef struct
{
    ble_rnet_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_rnet_init_t;

/**@brief   RingNet Service structure.
 *
 * @details This structure contains status information related to the service.
 */
struct ble_rnet_s {
    uint8_t                  uuid_type;               /**< UUID type for RingNet Service Base UUID. */
    uint16_t                 service_handle;          /**< Handle of RingNet Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t outbox_handles;          /**< Handles related to OUTBOX characteristic (as provided by the SoftDevice). */
    ble_gatts_char_handles_t inbox_handles;           /**< Handles related to INBOX characteristic (as provided by the SoftDevice). */
    uint16_t                 conn_handle;             /**< Handle of the current connection (as provided by the SoftDevice). BLE_CONN_HANDLE_INVALID if not in a connection. */
    bool                     is_notification_enabled; /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
    ble_rnet_data_handler_t  data_handler;            /**< Event handler to be called for handling received data. */
};


/**@brief   Function for initializing the RingNet Service.
 *
 * @param[out] p_rnet_s   RingNet Service structure. This structure must be supplied
 *                        by the application. It is initialized by this function and will
 *                        later be used to identify this particular service instance.
 * @param[in] p_rnet_init Information needed to initialize the service.
 *
 * @retval NRF_SUCCESS If the service was successfully initialized. Otherwise, an error code is returned.
 * @retval NRF_ERROR_NULL If either of the pointers p_rnet_s or p_rnet_init is NULL.
 */
uint32_t ble_rnet_service_init(ble_rnet_service_t * p_rnet_s, ble_rnet_init_t const * p_rnet_init);


/**@brief   Function for handling the RingNet Service's BLE events.
 *
 * @details This function should be called each time an event is received 
 * from the SoftDevice. This function processes the event if it is relevant
 * and calls the RingNet Service event handler if necessary.
 * 
 *
 * @param[in] p_ble_evt     Event received from the SoftDevice.
 * @param[in] p_context     RingNet Service structure.
 */
void ble_rnet_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);


/**@brief   Function for sending data to the peer.
 *
 * @details This function sends data as an OUTBOX characteristic notification to the
 *          peer.
 *
 * @param[in] p_rnet_s       Pointer to the RingNet Service structure.
 * @param[in] p_data         Data to be sent.
 * @param[in] p_length       Pointer to length of data(B) to be sent.
 *
 * @retval NRF_SUCCESS       If data was sent successfully. Otherwise, an error code is returned.
 */
uint32_t ble_rnet_send_data(ble_rnet_service_t * p_rnet_s, uint8_t * p_data, uint16_t * p_length);


#ifdef __cplusplus
}
#endif

#endif // BLE_RNET_SERVICE_H__

/** @} */
