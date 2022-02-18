/*
 * Copyright (c) 2019 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EMBEDDED_RPC__MBF_MSGQUE_H_
#define _EMBEDDED_RPC__MBF_MSGQUE_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "erpc_config_internal.h"
#include "erpc_manually_constructed.h"
#include "erpc_mbf_setup.h"
#include "erpc_message_buffer.h"
#include <ti/pcl/MsgQue.h>

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace erpc {

/*!
 * @brief MsgQue message buffer factory
 */
class MsgQueMessageBufferFactory : public MessageBufferFactory
{
public:
    /*!
     * @brief Constructor
     */
    MsgQueMessageBufferFactory(MsgQue_Handle sendq, MsgQue_Handle recvq)
    : m_sendq(sendq)
    , m_recvq(recvq)
    {
    }

    /*!
     * @brief Destructor
     */
    virtual ~MsgQueMessageBufferFactory(void) {}

    /*!
     * @brief Create MessageBuffer object
     *
     * @return MessageBuffer reference
     */
    virtual MessageBuffer create(void)
    {
        uint8_t *buf = (uint8_t *)MsgQue_alloc(m_sendq);
        assert(NULL != buf);
        uint16_t size = (uint16_t)MsgQue_bufSize(m_sendq);
        assert(size != 0);

        return MessageBuffer(buf, size);
    }

    /*!
     * @brief Inform server if it has to create buffer for received message
     *
     * @return Must return TRUE when server needs to create a buffer
     * for receiving a message.
     */
    virtual bool createServerBuffer(void) { return false; }

    /*!
     * @brief Dispose MessageBuffer object
     *
     * @param[in] message MessageBuffer reference
     */
    virtual void dispose(MessageBuffer *message)
    {
        assert(message);

        /* remove ownership of transport buffer */
        uint8_t *buf = message->get();

        if (NULL != buf) {
            message->set(NULL, 0);
            MsgQue_free(m_recvq, buf);
        }
    }

    /*!
     * @brief Prepare server reply message buffer
     *
     * @param[in] message MessageBuffer which needs a new out-going buffer
     */
    virtual erpc_status_t prepareServerBufferForSend(MessageBuffer *message)
    {
        /* remove ownership of invoke buffer, free buffer */
        void *invoke_buf = (void *)message->get();

        if (NULL != invoke_buf) {
            message->set(NULL, 0);
            MsgQue_free(m_recvq, invoke_buf);
        }

        /* allocate reply buffer, assign ownership */
        uint8_t *reply_buf = (uint8_t *)MsgQue_alloc(m_sendq);
        assert(NULL != reply_buf);
        uint16_t size = (uint16_t)MsgQue_bufSize(m_sendq);
        assert(size != 0);
        message->set(reply_buf, size);

        return kErpcStatus_Success;
    }

protected:
    MsgQue_Handle m_sendq;      /*!< MsgQue instance handle, send queue */
    MsgQue_Handle m_recvq;      /*!< MsgQue instance handle, receive queue */
};

} // namespace erpc

#endif // _EMBEDDED_RPC__MBF_MSGQUE_H_
