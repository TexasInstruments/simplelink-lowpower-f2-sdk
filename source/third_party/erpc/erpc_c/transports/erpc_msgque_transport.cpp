/*
 * Copyright (c) 2018-2019 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_msgque_transport.h"
#include <cstdio>
#include <assert.h>
#include <ti/pcl/MsgQue.h>
#include <ti/utils/runtime/Bench.h>

/* bench declarations */
Bench_decl(CONFIG_ERPC_MsgQueTransport_recv_t0);
Bench_decl(CONFIG_ERPC_MsgQueTransport_send_t0);
Bench_decl(CONFIG_ERPC_MsgQueTransport_send_t1);

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_status_t MsgQueTransport::receive(MessageBuffer *message)
{
    /* receive transport buffer */
    uint8_t *buf = (uint8_t *)MsgQue_get(m_recvq);

    Bench_logTS(CONFIG_ERPC_MsgQueTransport_recv_t0);

    assert(NULL != buf);
    uint16_t size = (uint16_t)MsgQue_bufSize(m_recvq);
    assert(size != 0);

    /* assign ownership of transport buffer */
    message->set(buf, size);

    return kErpcStatus_Success;
}

erpc_status_t MsgQueTransport::send(MessageBuffer *message)
{
    Bench_logTS(CONFIG_ERPC_MsgQueTransport_send_t0);

    /* remove ownership of transport buffer */
    void *buf = (void *)message->get();
    assert(NULL != buf);
    message->set(NULL, 0);

    /* pass buffer ownership to destination node */
    MsgQue_put(m_sendq, buf);

    Bench_logTS(CONFIG_ERPC_MsgQueTransport_send_t1);

    return kErpcStatus_Success;
}
