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

/**
 * @section rpm-route-main Interface to notify subscribers on network updates
 *
 * Definition of the interface which should be provided to RPM by those who is interested to
 * get notifications on network updates.
 *
 * @subsection rpm-route-problem Problem and requirements definition
 * System components which are responsible to deliver user messages over radio have a need to get information
 * about changes in network, like:
 *  - interface availability
 *  - interface connection quality
 *
 * @subsection rpm-route-tenets Tenets
 * Define a simple and clear API to provide all required information. Interface should not provide any
 * additional information regarding changes, like root causes of errors and so on.
 *
 * @subsection rpm-route-design Design
 * To cover all needs defined in problem sections it is decided to build simple interface which will provide
 * information
 *  - interface availability: @ref route_update_ifc.if_up and @ref route_update_ifc.if_down : notifies
 *      user when interface become available and unavailable
 *  - interface link quality: @ref route_update_ifc.if_update : provides numerical link metric and MTU size
 *
 * For more information about link metric @see netwotk_interface_linkmetric_t
 * and definition of interface @see route_update_ifc_s
 *
 * @note All functions return numerical value with operation status, for more details @see sid_error_t
 */

#ifndef HALO_LIB_NETWORK_INTERFACE_INCLUDE_EXPORT_HALO_LIB_NETWORK_INTERFACE_ROUTE_UPDATE_IFC_H_
#define HALO_LIB_NETWORK_INTERFACE_INCLUDE_EXPORT_HALO_LIB_NETWORK_INTERFACE_ROUTE_UPDATE_IFC_H_

#include <stdint.h>
#include <halo/lib/hall/hallerr.h>
#include <sid_network_data_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct route_update_ifc_s * route_update_ifc;

/**
 * Interface which should be subscribed into RPM to get notification about changes in network.
 */
struct route_update_ifc_s {
    /**
     * Inform that interface is created
     *
     * @param [in] _this    Pointer to the interface instance
     * @param [in] net_ifc  Pointer to the data interface of the network interface
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*if_add   )(const route_update_ifc * _this, const network_interface_data_ifc * net_ifc);

    /**
     * Inform an interface became unavailable
     *
     * @param [in] _this    Pointer to the interface instance
     * @param [in] net_ifc  Pointer to the data interface of the network interface became unavailable
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * When an interface becomes unavailable and it should be removed from the routing table and all rules should be disabled until
     * respective @ref route_update_ifc.if_up will be received. This function must be called to update the routing table information accordingly.
     */
    sid_error_t (*if_down  )(const route_update_ifc * _this, const network_interface_data_ifc * net_ifc);

    /**
     * Inform that interface is destroyed permanently
     *
     * @param [in] _this    Pointer to the interface instance
     * @param [in] net_ifc  Pointer to the data interface of the network interface became unavailable
     *
     * @retval SID_ERROR_NONE in case of success
     *
     * This function should be called in case when NI is permanently removed; after this call all references to specified object
     * must be removed/invalidated.
     */
    sid_error_t (*if_remove)(const route_update_ifc * _this, const network_interface_data_ifc * net_ifc);

    /**
     * Update the routing table information for a network interface
     *
     * @param [in] _this    Pointer to the interface instance
     * @param [in] net_ifc  Pointer to the data interface of the network interface became available
     * @param [in] metric   Value of the routing cost (metric) to be assigned to the @p net_ifc interface in the routing table
     * @param [in] mtu      Size of the Maximum Transmit Unit to be assigned to the @p net_ifc interface in the routing table
     *
     * @retval SID_ERROR_NONE in case of success
     */
    sid_error_t (*if_up)(const route_update_ifc * _this,
                              const network_interface_data_ifc * net_ifc,
                              uint16_t metric,
                              uint16_t mtu);
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_LIB_NETWORK_INTERFACE_INCLUDE_EXPORT_HALO_LIB_NETWORK_INTERFACE_ROUTE_UPDATE_IFC_H_ */
