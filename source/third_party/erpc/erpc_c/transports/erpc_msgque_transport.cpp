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

    assert(NULL != buf);
    uint16_t size = (uint16_t)MsgQue_bufSize(m_recvq);
    assert(size != 0);

    /* assign ownership of transport buffer */
    message->set(buf, size);

    return kErpcStatus_Success;
}

erpc_status_t MsgQueTransport::send(MessageBuffer *message)
{
    /* remove ownership of transport buffer */
    void *buf = (void *)message->get();
    assert(NULL != buf);
    message->set(NULL, 0);

    /* pass buffer ownership to destination node */
    MsgQue_put(m_sendq, buf);

    return kErpcStatus_Success;
}
