/*
 * Copyright (c) 2018-2019 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_msgque_transport.h"
#include "erpc_transport_setup.h"
#include <ti/pcl/MsgQue.h>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<MsgQueTransport> s_transport_client;
static ManuallyConstructed<MsgQueTransport> s_transport_server;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_msgque_init_client(void *sendq, void *recvq)
{
    s_transport_client.construct((MsgQue_Handle)sendq, (MsgQue_Handle)recvq);
    return reinterpret_cast<erpc_transport_t>(s_transport_client.get());
}

erpc_transport_t erpc_transport_msgque_init_server(void *sendq, void *recvq)
{
    s_transport_server.construct((MsgQue_Handle)sendq, (MsgQue_Handle)recvq);
    return reinterpret_cast<erpc_transport_t>(s_transport_server.get());
}

erpc_transport_t erpc_transport_msgque_init_ref(void *sendq, void *recvq, void *transport_obj)
{
    ManuallyConstructed<MsgQueTransport> *t;

    t = static_cast<erpc::ManuallyConstructed<MsgQueTransport> *>(transport_obj);
    t->construct((MsgQue_Handle)sendq, (MsgQue_Handle)recvq);
    return reinterpret_cast<erpc_transport_t>(t->get());
}
