/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * Copyright (c) 2018-2019 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ERPC_MBF_SETUP_H_
#define _ERPC_MBF_SETUP_H_

#include "erpc_transport_setup.h"

/*!
 * @addtogroup message_buffer_factory_setup
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////

//! @brief Opaque MessageBufferFactory object type.
typedef struct ErpcMessageBufferFactory *erpc_mbf_t;

////////////////////////////////////////////////////////////////////////////////
// API
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

//! @name MessageBufferFactory setup
//@{

/*!
 * @brief Create MessageBuffer factory which is using static allocated buffers.
 */
erpc_mbf_t erpc_mbf_static_init(void);

/*!
 * @brief Create MessageBuffer factory which is using dynamic allocated buffers.
 */
erpc_mbf_t erpc_mbf_dynamic_init(void);

/*!
 * @brief Create MessageBuffer factory which is using RPMSG LITE zero copy buffers.
 *
 * Has to be used with RPMSG lite zero copy transport.
 */
erpc_mbf_t erpc_mbf_rpmsg_init(erpc_transport_t transport);

/*!
 * @brief Create MessageBuffer factory which is using RPMSG LITE TTY buffers.
 *
 * Has to be used with RPMSG lite TTY transport.
 */
erpc_mbf_t erpc_mbf_rpmsg_tty_init(erpc_transport_t transport);

/*!
 * @brief Create MessageBuffer factory for ti.pcl.MsgQue
 */
erpc_mbf_t erpc_mbf_msgque_init_client(void *sendq, void *recvq);
erpc_mbf_t erpc_mbf_msgque_init_server(void *sendq, void *recvq);
erpc_mbf_t erpc_mbf_msgque_init_ref(void *sendq, void *recvq, void *mbf_obj);

//@}

#ifdef __cplusplus
}
#endif

/*! @} */

#endif // _ERPC_MBF_SETUP_H_
