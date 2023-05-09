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

/*
 *
 *
 *  Component responsibilities:
 *   1. manage bindings
 *   2. dispatch messages from network (MDM) to classes
 *   3. dispatch messages from classes to network (using MDM)
 *
 *  This component is passive and activated by user of MDM
 *
 * API
 *  bind
 *      Arguments:
 *          command class ID
 *          notification callback and specific command class implementation
 *          local and remote HALO (former RNET) network addresses
 *
 *    Binding policies:
 *      Bind defines routing for messages based on user addresses provided by user in bind call
 *
 *      # | local | remote | description
 *      --+-------+--------+---------------------------------------------------------------------
 *      1 | NULL  | NULL   | in: class will receive all messages send to specific command class
 *        |       |        | out: messages send by class will be broadcasted to network
 *      --+-------+--------+---------------------------------------------------------------------
 *      2 | ADDR  | NULL   | in: class will receive all messages send to specific command class and local ADDR
 *        |       |        | out: messages send by class will be broadcasted to network
 *      --+-------+--------+---------------------------------------------------------------------
 *      3 | NULL  | ADDR   | in: class will receive all messages send to specific command class
 *        |       |        | out: messages send by class will be send to specified address
 *      --+-------+--------+---------------------------------------------------------------------
 *      4 | ADDR  | ADDR   | in: class will receive all messages send to specific command class
 *        |       |        |        and local ADDR and specific remote ADDR
 *        |       |        | out: messages send by class will be send to specified remote (dst)
 *        |       |        |        address from specific local ADDR
 *      --+-------+--------+---------------------------------------------------------------------
 *           NOTE: send operation allow user to specify destination, then rules defined by bind operation
 *                  will be overridden
 *
 *           NULL - equivalent of broadcast address
 *                  local NULL - accept all incoming
 *                  remote NULL - send broadcast message
 *
 *  send_to
 *      Arguments
 *          Prepared command to be send out
 *          remote address (can be NULL, then address and rules specified in bind will be used)
 *      Note: after send user MUST NOT change internals of command which is passes to dispatcher
 *
 *  on_message
 *      Arguments
 *          message which need to be dispatched to subscribers
 *          local and remote addresses must be used to dispatch messages according rules specified in bind
 */

#ifndef HALO_HALL_DISPATCHER_H_
#define HALO_HALL_DISPATCHER_H_

#include <sid_network_address.h>
#include <halo/lib/hall/hallerr.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/cmd.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/core/resource_manager.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct halo_hall_dispatcher_ifc_s* halo_hall_dispatcher_ifc;
typedef const struct halo_hall_mdm_ifc_s* halo_hall_mdm_ifc;
typedef struct hall_cmd_descriptor_s hall_cmd_descriptor_t;

typedef struct msg_config_s {
    const halo_hall_message_descriptor_t* msg_desc;
    hall_cmd_descriptor_t cmd_decriptor;
    uint16_t size;
} msg_config_t;

struct halo_hall_dispatcher_ifc_s {
    void (*destroy)(const halo_hall_dispatcher_ifc* _this);

    // up-link communications
    sid_error_t (*bind)(const halo_hall_dispatcher_ifc* _this,
                         const uint16_t cmd_class_id,
                         const halo_hall_appv1_generic_command_ifc* generic_ifc,
                         const network_interface_data_ifc* route,
                         const struct sid_address* remote);

    sid_error_t (*unbind)(const halo_hall_dispatcher_ifc* _this,
                           const uint16_t cmd_class_id,
                           const halo_hall_appv1_generic_command_ifc* generic_ifc);

    sid_error_t (*set_downlink)(const halo_hall_dispatcher_ifc* _this, const halo_hall_mdm_ifc* mdm);

    sid_error_t (*send)(const halo_hall_dispatcher_ifc* _this,
                         halo_all_cmd_t* msg,
                         const halo_hall_appv1_generic_command_ifc* generic_ifc);

    sid_error_t (*get_msg_buff)(const halo_hall_dispatcher_ifc* _this, const msg_config_t* config, halo_all_cmd_t** msg);

    sid_error_t (*free_msg_buff)(const halo_hall_dispatcher_ifc* _this, halo_all_cmd_t* msg);
    // down-link notifications
    sid_error_t (*on_message)(const halo_hall_dispatcher_ifc* _this, halo_all_cmd_t* msg);
    sid_error_t (*get_class_version)(const halo_hall_dispatcher_ifc* _this,
                                      uint16_t cls_id,
                                      halo_hall_appv1_class_versions_t* vestions);
};

sid_error_t halo_hall_dispatcher_create(const halo_hall_dispatcher_ifc** dispatcher_this);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* HALO_HALL_DISPATCHER_H_ */
