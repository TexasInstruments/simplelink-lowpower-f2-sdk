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

#ifndef HALO_HALS_ROUTER_H_
#define HALO_HALS_ROUTER_H_


#include <halo/lib/hall/radio_policy_manager/route_update_ifc.h>
#include <halo/lib/hall/core/resource_manager.h>
#include <sid_network_data_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ADAPTERS_COUNT 1

typedef struct halo_hall_route_config_s {
    size_t      arp_size;           //!< Size of ARP-like table for best route lookup based on "last seen" information for specific destination.
    uint32_t    ttl_s;              //!< ARP-like table record Time To Live (TTL).
    size_t      route_table_size;   //!< Initial size of routing rules table size.
    size_t      net_ifc_table_size; //!< Initial size of net interfaces table size.
    struct sid_address *local;          //!< Local address
    const halo_hall_resource_manager_ifc* resource_manager; //!< link to resource manager
} halo_hall_router_config_t;

/**
 * Enumeration defines destinations for message under processing
 */
typedef enum halo_hall_route_destination_type_e {
    HALO_HALL_ROUTE_DESTINATION_TYPE_DEFAULT,  //!< Deliver to default adapter
    HALO_HALL_ROUTE_DESTINATION_TYPE_ADAPTER,  //!< Delivery to a specific adapter
    HALO_HALL_ROUTE_DESTINATION_TYPE_LOCAL,    //!< Deliver to higher level(MDM) for local processing
} halo_hall_route_destination_type;

/**
 * Defines routing destination
 *
 * @see halo_hall_route_destination_type for more details
 */
typedef struct halo_hall_route_destination_s {
    halo_hall_route_destination_type type;
    const network_interface_data_ifc* adapter; /**! used in combination with @ref HALO_HALL_ROUTE_DESTINATION_TYPE_ADAPTER*/
} halo_hall_route_destination_t;


/**
 * Source description of routing rule.
 */
typedef struct halo_hall_route_source_s {
    const network_interface_data_ifc* adapters[MAX_ADAPTERS_COUNT];
} halo_hall_route_source_t;

typedef bool (*halo_hall_filter_predicate_t)(const halo_all_cmd_t* msg);

typedef struct halo_hall_route_ifc_s* halo_hall_route_ifc;
/**
 * Route configuration interface.
 *
 * Provides API to configure routing rules.
 */
struct halo_hall_route_ifc_s {
    sid_error_t (*add)(
        const halo_hall_route_ifc* _this,       //!< Router object instance.
        const struct sid_address* destination,      //!< Message destination address.
        const halo_hall_route_source_t* src,    //!< Adapter(s) which can be a message source.
        const halo_hall_filter_predicate_t predicate, //!< user defined filtering rule
        const halo_hall_route_destination_t* dst//!< Message destination definition.
    );
    sid_error_t (*remove)(
        const halo_hall_route_ifc* _this,       //!< Router object instance.
        const struct sid_address* destination,      //!< Message destination address.
        const halo_hall_route_source_t* src,    //!< Adapter(s) which can be a message source.
        const halo_hall_filter_predicate_t predicate, //!< user defined filtering rule
        const halo_hall_route_destination_t* dst//!< Message destination definition.
    );
};




typedef struct halo_hall_router_ext_ifc_s* halo_hall_router_ext_ifc;
/**
 * Router interface definition.
 *
 * This is aggregated set of interfaces involved into routing configuration and operation.
 */
struct halo_hall_router_ext_ifc_s {
    void (*destroy)(const halo_hall_router_ext_ifc* _this);
    const network_interface_data_notify_ifc*  uplink_notify;   //!< To be subscribed to data_ifc in layer below to get notifications.

    //!< Need to be provided to upper layer to send outgoing messages and read incoming messages.
    const network_interface_data_ifc*         (*get_uplink_data)(const halo_hall_router_ext_ifc* _this);
    //!< Interface which must be provided to RPM.
    const route_update_ifc*                   (*get_route_update)(const halo_hall_router_ext_ifc* _this);
    //!< Interface to configure routing rules.
    const halo_hall_route_ifc*                (*get_route_config)(const halo_hall_router_ext_ifc* _this);

    //!< set the internal local address of the router.
    void (* set_local_addr)(const halo_hall_router_ext_ifc* _this, const struct sid_address *const addr);
};

/**
 * Router constructor.
 *
 * @param [out] ifc  return value, reference to new instance of router will be returned into given value.
 * @param [in] cfg          router configuration, @see halo_hall_router_config_t.
 * @param [in] mdm_notify   reference to.
 *
 * @return Numeric operation status, @see  sid_error_t for more information.
 * @retval SID_ERROR_NONE
 */
sid_error_t halo_hall_router_create(
    const halo_hall_router_ext_ifc** ifc,
    const halo_hall_router_config_t* cfg,
    const network_interface_data_notify_ifc* mdm_notify
    );

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALS_ROUTER_H_ */
