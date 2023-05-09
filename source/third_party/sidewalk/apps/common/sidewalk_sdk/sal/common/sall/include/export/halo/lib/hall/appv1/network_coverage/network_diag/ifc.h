/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_DIAG_IFC_H_
#define RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_DIAG_IFC_H_

#include <halo/lib/hall/appv1/network_coverage/network_diag/types.h>
#include <halo/lib/hall/appv1/message.h>
#include <halo/lib/hall/appv1/generic_command_ifc.h>
#include <halo/lib/hall/interface.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct halo_hall_appv1_network_diagnostics_ifc_s *halo_hall_appv1_network_diagnostics_ifc;

struct halo_hall_appv1_network_diagnostics_ifc_s {
    sid_error_t (*ping)(const halo_hall_appv1_network_diagnostics_ifc *_this,
                        halo_hall_message_descriptor_t *mdesc,
                        const halo_hall_appv1_network_ping_t *val);
    sid_error_t (*pong)(const halo_hall_appv1_network_diagnostics_ifc *_this,
                        halo_hall_message_descriptor_t *mdesc,
                        const halo_hall_appv1_network_pong_t *val);
    sid_error_t (*delay_ping)(const halo_hall_appv1_network_diagnostics_ifc *_this,
                              halo_hall_message_descriptor_t *mdesc,
                              const halo_hall_appv1_network_ping_t *val);
    sid_error_t (*delay_pong)(const halo_hall_appv1_network_diagnostics_ifc *_this,
                              halo_hall_message_descriptor_t *mdesc,
                              const halo_hall_appv1_network_pong_t *val);
    sid_error_t (*status_report_notify)(const halo_hall_appv1_network_diagnostics_ifc *_this,
                                        halo_hall_message_descriptor_t *mdesc,
                                        const uint8_t *data,
                                        const uint8_t size);
};

typedef struct halo_hall_appv1_network_diagnostics_ext_ifc_s halo_hall_appv1_network_diagnostics_ext_ifc;

struct halo_hall_appv1_network_diagnostics_ext_ifc_s {
    halo_hall_appv1_network_diagnostics_ifc send;
    halo_hall_appv1_network_diagnostics_ifc receive;
    halo_hall_appv1_generic_command_ifc generic;
};

typedef struct halo_hall_dispatcher_ifc_s *halo_hall_dispatcher_ifc;

sid_error_t halo_hall_appv1_network_diagnostics_create(const halo_hall_appv1_network_diagnostics_ext_ifc **_this,
                                                       const halo_hall_appv1_network_diagnostics_ifc user_notify_cb,
                                                       halo_hall_appv1_user_ctx_t user_ctx,
                                                       const void *lib_ctx);

#ifdef __cplusplus
}   // extern "C"
#endif

#endif /* RNETC_PUBLIC_HALO_HALL_APPV1_NETWORK_COVERAGE_DIAG_IFC_H_ */
