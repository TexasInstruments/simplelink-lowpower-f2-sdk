/*
 * Copyright (c) 2018-2019 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__MSGQUE_TRANSPORT_H_
#define _EMBEDDED_RPC__MSGQUE_TRANSPORT_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <erpc_message_buffer.h>
#include <erpc_transport.h>
#include <ti/pcl/MsgQue.h>

/*!
 * @addtogroup msgque_transport
 * @{
 * @file
 */

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {
/*!
 * @brief Transport to send and receive messages via ti.pcl.MsgQue
 *
 * @ingroup msgque_transport
 */
class MsgQueTransport : public Transport
{
public:
    /*!
     * @brief Constructor.
     */
    MsgQueTransport(MsgQue_Handle sendq, MsgQue_Handle recvq)
    : m_sendq(sendq)
    , m_recvq(recvq)
    {
    }

    /*!
     * @brief Destructor.
     */
    virtual ~MsgQueTransport(void) {}

    /*!
     * @brief poll for new message
     */
    virtual bool hasMessage(void)
    {
        return MsgQue_check(m_recvq);
    }

    /*!
     * @brief receive a message
     *
     * @param[out] pointer to received message buffer
     */
    virtual erpc_status_t receive(MessageBuffer *message);

    /*!
     * @brief send a message
     *
     * @param[in] message buffer to send
     */
    virtual erpc_status_t send(MessageBuffer *message);

private:
    MsgQue_Handle m_sendq;      /*!< MsgQue instance handle, send queue */
    MsgQue_Handle m_recvq;      /*!< MsgQue instance handle, receive queue */
};

} // namespace erpc

/*! @} */

#endif // _EMBEDDED_RPC__MSGQUE_TRANSPORT_H_
