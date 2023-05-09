/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
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

/* @section ni-data-main Network Data interface
 * Definition of interfaces for data exchange between HALL and Network Interface.
 *
 * @subsection ni-data-problem Problem and requirements definition
 * TBD
 * @subsection ni-data-tenets Tenets
 * TBD
 * @subsection ni-data-design Design
 * TBD
 *
 * @note All functions return numerical value with operation status, for more details @see sid_error_t
 */

#ifndef SID_NETWORK_DATA_IFC_H
#define SID_NETWORK_DATA_IFC_H

#include <sid_network_address.h>

#include <sid_error.h>
#include <sall_app_error.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct halo_all_cmd_s halo_all_cmd_t;
typedef struct halo_hall_msg_status_s halo_hall_msg_status_t;
typedef struct network_interface_data_ifc_s* network_interface_data_ifc;
typedef struct network_interface_data_notify_ifc_s* network_interface_data_notify_ifc;

/**
 * Notification interface towards HALL from Network Interface
 */
struct network_interface_data_notify_ifc_s {

    /**
     * Notifies HALL that incoming messages are ready to receive
     *
     * @param [in] _this - pointer to instance of notification interface
     * @param [in] nif - pointer to Network Adapter data interface which is ready to receive operation
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * Should be called when incoming messages are ready for processing. Expected that receive function of
     * corresponding network_interface_data_ifc will return message for processing.
     */
    sid_error_t (*on_recv_ready)(const network_interface_data_notify_ifc* _this, const network_interface_data_ifc* nif);

    /**
     * Notifies HALL that network interface is ready to accept outgoing messages
     *
     * @param [in] _this - pointer to instance of notification interface
     * @param [in] nif - pointer to Network Adapter data interface which is ready to send operation
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * Should be called when Network Interface is ready to accept outgoing messages. Expected that send function of
     * corresponding network_interface_data_ifc will accept at least one message successfully.
     */
    sid_error_t (*on_send_ready)(const network_interface_data_notify_ifc* _this, const network_interface_data_ifc* nif);

    /**
     * Notifies HALL on status of outgoing message
     *
     * @param [in] _this - pointer to instance of notification interface
     * @param [in] msg_status - delivery status of the message
     * @retval SID_ERROR_NONE in case of success
     *
     * Should be called to notify HALL on delivery status of outgoing message
     */
    sid_error_t (*on_data_sent)(const network_interface_data_notify_ifc* _this, halo_hall_msg_status_t *msg_status);
};

/**
 * Network Data Interface used to communicate with HALL Components
 */
struct network_interface_data_ifc_s {
    /**
     * Destroys the interface an the underlying network interface instance
     * @param _this                 - reference to the interface
     */
    void (*destroy)(const network_interface_data_ifc* _this);

    /**
     * Function to specify notify interface.
     *
     * User of data interface should use this function to specify data notification interface in order to get notifications.
     * Otherwise user will need to pull data interface constantly to read data. To update notification interface just call
     * function with new value or with NULL to disable notification
     *
     * @param [in] _this        reference to current data interface
     * @param [in] notify_ifc   notification interface pointer or NULL to disable notifications
     *
     * @retval SID_ERROR_NONE
     * @note Only one subscriber is supported and function call just override previously specified value
     */
    sid_error_t (* set_notify)(const network_interface_data_ifc* _this, const network_interface_data_notify_ifc* notify_ifc);

    /**
     * Used by HALL to push data to Network Interface
     *
     * @param [in] _this - pointer to interface
     * @param [in] msg - pointer to outgoing message
     *
     * @retval SID_ERROR_NONE in case of success
     * @retval SID_ERROR_NO_MEM - if no resources available to accept message (XXX : clarify error code)
     */
    sid_error_t (*send)(const network_interface_data_ifc* _this, halo_all_cmd_t* msg);
    /**
     * Used by HALL to retrieve incoming messages from Network Interface
     *
     * @param [in] _this - pointer to interface
     * @param [out] msg - reference to pointer to incoming message
     *
     * @retval SID_ERROR_NONE in case of success
     * @retval SID_ERROR_NO_DATA - if no more incoming messages available (XXX : clarify error code)
     */
    sid_error_t (*recv)(const network_interface_data_ifc* _this, halo_all_cmd_t** msg);

    /**
     * Used by HALL to discard outgoing message and generate NOT ACK
     *
     * @param [in] _this - pointer to interface
     * @param [in] msg - message which should be discarded
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * After that call HALL must receive incoming message with corresponding error code (TIMEOUT)
     */
    sid_error_t (*generate_nack)(const network_interface_data_ifc* _this,
                                 halo_all_cmd_t* msg,
                                 sall_app_error_t status);

    /**
     * Used by HALL to access Network Interface network address
     *
     * @param [in] _this - pointer to interface
     * @param [out] resource_manager - reference to pointer to put resource manager interface into
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * Returns reference to network address associated with specified adapter
     */
    sid_error_t (*get_local_addr)(const network_interface_data_ifc* _this, const struct sid_address** local_addr);
};

#ifdef __cplusplus
}
#endif

#endif
