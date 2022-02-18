/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * Copyright (c) 2019-2020 Texas Instruments Incorporated
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"
#include "erpc_basic_codec.h"
#include "erpc_crc16.h"
#include "erpc_manually_constructed.h"
#include "erpc_message_buffer.h"
#include "erpc_simple_server.h"
#include "erpc_transport.h"
#include <cassert>

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global server variables
static ManuallyConstructed<SimpleServer> s_server;
SimpleServer *g_server = NULL;
static ManuallyConstructed<BasicCodecFactory> s_codecFactory;
static ManuallyConstructed<Crc16> s_crc16;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_server_t erpc_server_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory)
{
    assert(transport);

    // Init factories.
    s_codecFactory.construct();

    // Init server with the provided transport.
    s_server.construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(transport);
    s_crc16.construct();
    castedTransport->setCrc16(s_crc16.get());
    s_server->setTransport(castedTransport);
    s_server->setCodecFactory(s_codecFactory);
    s_server->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));
    g_server = s_server;
    return reinterpret_cast<erpc_server_t>(g_server);
}

erpc_server_t erpc_server_init_ref(erpc_transport_t transport, erpc_mbf_t message_buffer_factory,
        void *ss_obj, void *cf_obj, void *crc_obj)
{
    ManuallyConstructed<SimpleServer> *server;
    ManuallyConstructed<BasicCodecFactory> *codecFactory;
    ManuallyConstructed<Crc16> *crc16;

    assert(transport);

    server = static_cast<erpc::ManuallyConstructed<SimpleServer> *>(ss_obj);
    codecFactory = static_cast<erpc::ManuallyConstructed<BasicCodecFactory> *>(cf_obj);
    crc16 = static_cast<erpc::ManuallyConstructed<Crc16> *>(crc_obj);

    // Init factories.
    codecFactory->construct();

    // Init server with the provided transport.
    server->construct();
    Transport *castedTransport = reinterpret_cast<Transport *>(transport);
    crc16->construct();
    castedTransport->setCrc16(crc16->get());
    (*server)->setTransport(castedTransport);
    (*server)->setCodecFactory(*codecFactory);
    (*server)->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));
    return reinterpret_cast<erpc_server_t>(server);
}

void erpc_server_deinit(void)
{
    s_crc16.destroy();
    s_codecFactory.destroy();
    s_server.destroy();
    g_server = NULL;
}

void erpc_server_deinit_ref(void *ss_obj, void *cf_obj, void *crc_obj)
{
    ManuallyConstructed<SimpleServer> *server;
    ManuallyConstructed<BasicCodecFactory> *codecFactory;
    ManuallyConstructed<Crc16> *crc16;

    server = static_cast<erpc::ManuallyConstructed<SimpleServer> *>(ss_obj);
    codecFactory = static_cast<erpc::ManuallyConstructed<BasicCodecFactory> *>(cf_obj);
    crc16 = static_cast<erpc::ManuallyConstructed<Crc16> *>(crc_obj);

    crc16->destroy();
    codecFactory->destroy();
    server->destroy();
}

void erpc_add_service_to_server(void *service)
{
    if (g_server != NULL && service != NULL)
    {
        g_server->addService(static_cast<erpc::Service *>(service));
    }
}

void erpc_add_service_to_server_ref(void *service, void *server_obj)
{
    SimpleServer *ss;

    if (server_obj != NULL && service != NULL)
    {
        ss = static_cast<erpc::SimpleServer *>(server_obj);
        ss->addService(static_cast<erpc::Service *>(service));
    }
}

void erpc_remove_service_from_server(void *service)
{
    if (g_server != NULL && service != NULL)
    {
        g_server->removeService(static_cast<erpc::Service *>(service));
    }
}

void erpc_remove_service_from_server_ref(void *service, void *server_obj)
{
    SimpleServer *ss;

    if (server_obj != NULL && service != NULL)
    {
        ss = static_cast<erpc::SimpleServer *>(server_obj);
        ss->removeService(static_cast<erpc::Service *>(service));
    }
}

void erpc_server_set_crc(uint32_t crcStart)
{
    s_crc16->setCrcStart(crcStart);
}

void erpc_server_set_crc_ref(uint32_t crcStart, void *crc_obj)
{
    Crc16 *crc16;

    crc16 = static_cast<erpc::Crc16 *>(crc_obj);
    crc16->setCrcStart(crcStart);
}

erpc_status_t erpc_server_run(void)
{
    if (g_server != NULL)
    {
        return g_server->run();
    }
    return kErpcStatus_Fail;
}

erpc_status_t erpc_server_run_ref(void *server_obj)
{
    SimpleServer *ss;

    if (server_obj != NULL)
    {
        ss = static_cast<erpc::SimpleServer *>(server_obj);
        return ss->run();
    }
    return kErpcStatus_Fail;
}

erpc_status_t erpc_server_poll(void)
{
    if (g_server != NULL)
    {
        return g_server->poll();
    }
    return kErpcStatus_Fail;
}

erpc_status_t erpc_server_poll_ref(void *server_obj)
{
    SimpleServer *ss;

    if (server_obj != NULL)
    {
        ss = static_cast<erpc::SimpleServer *>(server_obj);
        return ss->poll();
    }
    return kErpcStatus_Fail;
}

void erpc_server_stop(void)
{
    if (g_server != NULL)
    {
        g_server->stop();
    }
}

void erpc_server_stop_ref(void *server_obj)
{
    SimpleServer *ss;

    if (server_obj != NULL)
    {
        ss = static_cast<erpc::SimpleServer *>(server_obj);
        ss->stop();
    }
}

#if ERPC_MESSAGE_LOGGING
bool erpc_server_add_message_logger(erpc_transport_t transport)
{
    if (g_server != NULL)
    {
        return g_server->addMessageLogger(reinterpret_cast<Transport *>(transport));
    }
    return false;
}
#endif
