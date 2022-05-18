/*
 * Copyright (c) 2018-2019 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.h"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.h"

#include <ti/pcl/MsgQue.h>
#include "erpc_setup_mbf_msgque.h"


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

using namespace erpc;

static ManuallyConstructed<MsgQueMessageBufferFactory> s_msgFactory_client;

erpc_mbf_t erpc_mbf_msgque_init_client(void *sendq, void *recvq)
{
    s_msgFactory_client.construct((MsgQue_Handle)sendq, (MsgQue_Handle)recvq);
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory_client.get());
}
static ManuallyConstructed<MsgQueMessageBufferFactory> s_msgFactory_server;

erpc_mbf_t erpc_mbf_msgque_init_server(void *sendq, void *recvq)
{
    s_msgFactory_server.construct((MsgQue_Handle)sendq, (MsgQue_Handle)recvq);
    return reinterpret_cast<erpc_mbf_t>(s_msgFactory_server.get());
}

erpc_mbf_t erpc_mbf_msgque_init_ref(void *sendq, void *recvq, void *mbf_obj)
{
    ManuallyConstructed<MsgQueMessageBufferFactory> *mbf;

    mbf = static_cast<erpc::ManuallyConstructed<MsgQueMessageBufferFactory> *>(mbf_obj);
    mbf->construct((MsgQue_Handle)sendq, (MsgQue_Handle)recvq);
    return reinterpret_cast<erpc_mbf_t>(mbf->get());
}
