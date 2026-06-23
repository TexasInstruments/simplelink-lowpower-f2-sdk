/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== application.c ========
 */

#ifndef WISUN_NCP_ENABLE
#undef EXCLUDE_TRACE
#endif
#include "mbed_config_app.h"

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
/* Driver configuration */
#include "ti_drivers_config.h"
#include "ti_wisunfan_config.h"

#include "nsconfig.h"
#include "mesh_system.h"
#include "socket_api.h"
#include "ip6string.h"
#include "net_interface.h"
#include "wisun_tasklet.h"
#include "ns_trace.h"
#include "fhss_config.h"
#include "randLIB.h"
#include "ws_management_api.h"

#include "6LoWPAN/ws/ws_common_defines.h"
#include "Common_Protocols/ipv6_constants.h"
#include "6LoWPAN/ws/ws_common.h"
#include "Common_Protocols/udp.h"

#if (WISUN_APP_TCP_MODE > 0)
#include "Common_Protocols/tcp.h"
#endif

#include "mbed-mesh-api/mesh_interface_types.h"
#include "NanostackTiRfPhy.h"
#include "borderrouter_tasklet.h"
#include "nsdynmemLIB.h"
#include "6LoWPAN/ws/ws_config.h"
#include "ti_wisunfan_features.h"

/* TCP Configuration Override - ensures TCP is enabled even if SysConfig doesn't support it */
#if __has_include("tcp_config_override.h")
#include "tcp_config_override.h"
#endif

#include "application.h"
#include "eventOS_event_timer.h"
#ifdef WISUN_NCP_ENABLE
/* OpenThread Internal/Example Header files */
#include "otsupport/otrtosapi.h"
#include "openthread/ncp.h"
#include "platform/system.h"
#else
#endif //WISUN_NCP_ENABLE

/******************************************************************************
Defines & enums
 *****************************************************************************/

#define TRACE_GROUP "main"

#define NOT_INITIALIZED -1

/******************************************************************************
 Static & Global Variables
 *****************************************************************************/

static int8_t interface_id = NOT_INITIALIZED;
static bool _configured = false;

#ifdef NWK_TEST
uint32_t ticks_before_joining = 0;
uint32_t ticks_after_joining = 0;
#endif //NWK_TEST

#if (WISUN_APP_TCP_MODE > 0)
/* TCP Socket Configuration */
#define TCP_PORT                   (5678U)     /* TCP port for LED demo */
#define TCP_RECV_BUF_SIZE          (2048U)     /* TCP receive buffer size (matches SOCKET_DEFAULT_STREAM_RCVBUF) */
#define TCP_SEND_BUF_SIZE          (2048U)     /* TCP send buffer size (matches SOCKET_DEFAULT_STREAM_SNDBUF) */
#define TCP_BACKLOG                (1U)        /* TCP listen backlog */

/* Reconnect delay — OS timer in ms, not a loop-tick counter */
#define TCP_RECONNECT_DELAY_MS     (5000U)    /* ms before client retries after disconnect */

/* socket_connect() returns this when handshake is in progress (wait for callback) */
#define TCP_CONNECT_IN_PROGRESS    (-4)


/******************************************************************************
 TCP State — unified enum for both connection state machine and NCP
 notification vocabulary.  Values 1-6 match the PySpinel event protocol.
 *****************************************************************************/
typedef enum {
    TCP_STATE_IDLE            = 0,  /*!< No socket open */
    TCP_STATE_CONNECTED       = 1,  /*!< Data path open; NCP: CONNECTED */
    TCP_STATE_DISCONNECTED    = 2,  /*!< Event: connection lost; NCP: DISCONNECTED */
    TCP_STATE_LISTENING       = 3,  /*!< Server: bound+listening; NCP: LISTEN_STARTED */
    TCP_STATE_CLIENT_ACCEPTED = 4,  /*!< Event: server accepted client; NCP: CLIENT_ACCEPTED */
    TCP_STATE_CONNECT_FAILED  = 5,  /*!< Event: connect failed; NCP: CONNECT_FAILED */
    TCP_STATE_DATA_RECEIVED   = 6,  /*!< NCP informational */
    TCP_STATE_CONNECTING      = 7,  /*!< Client: SYN sent (internal only) */
    TCP_STATE_RECONNECT_WAIT  = 8   /*!< Client: OS timer armed (internal only) */
} tcp_state_t;

/* TCP connection context — all runtime state in one place */
typedef struct {
    int8_t               socket_id;
#if (WISUN_APP_TCP_MODE == 1)
    int8_t               client_socket_id;
#endif
    volatile tcp_state_t conn_state;
#if (WISUN_APP_TCP_MODE == 2)
    uint8_t              target_peer_addr[16];
    bool                 peer_addr_valid;
#endif
    ns_address_t         peer_addr;
    uint8_t              recv_buffer[TCP_RECV_BUF_SIZE];
    uint8_t              send_buffer[TCP_SEND_BUF_SIZE];
} socket_info_t;

/* Single TCP context instance — explicitly pre-initialised */
static socket_info_t g_tcp_state = {
    .socket_id       = NOT_INITIALIZED,
#if (WISUN_APP_TCP_MODE == 1)
    .client_socket_id = NOT_INITIALIZED,
#endif
    .conn_state      = TCP_STATE_IDLE,
#if (WISUN_APP_TCP_MODE == 2)
    .peer_addr_valid = false,
#endif
};

/* One-shot reconnect timer (NULL when not armed) */
#if (WISUN_APP_TCP_MODE == 2)
static timeout_t *tcp_reconnect_timeout = NULL;
#endif

/* NCP ABI: ncp_base_mtd.cpp reads this as extern bool tcp_connected.
 * Kept in sync at the end of tcp_socket_callback() and in tcp_disconnect(). */
bool tcp_connected = false;

#endif /* WISUN_APP_TCP_MODE */

#ifdef WISUN_NCP_ENABLE
int8_t ncp_tasklet_id = -1;
otInstance *OtStack_instance = NULL;

#ifdef WISUN_TEST_METRICS
extern JOIN_TIME_s node_join_time;
#endif

#endif //WISUN_NCP_ENABLE

#ifdef WISUN_TEST_MPL_EMBEDDED
#define SEND_BUF_SIZE 24
#define multicast_addr_str "ff15::810a:64d1"
#define UDP_PKT_INTERVAL 1000
#ifndef MASTER_GROUP
#define MASTER_GROUP 0
#define MY_GROUP     1
#endif
#endif

#ifdef TI_WISUN_FAN_DEBUG
bool disable_ns_messages = false;
#endif

ti_wisun_config_t ti_wisun_config =
{
    .rapid_join = FEATURE_RAPID_JOIN_ENABLE,
    .network_size_config = FEATURE_NETWORK_PROFILE,
    .mpl_low_latency = FEATURE_MPL_LOW_LATENCY_ENABLE,
    .rapid_disconnect_detect_br = FEATURE_RAPID_DISCONNECT_DETECT_BR_SEC,
    .rapid_disconnect_detect_rn = FEATURE_RAPID_DISCONNECT_DETECT_RN_SEC,
    .auth_type  = NETWORK_AUTH_TYPE,
    .use_fixed_gtk_keys = false,
    .force_star_topology = FEATURE_FORCE_STAR_TOPOLOGY,
    .use_dhcp_solicit_for_renew = true,
    .fixed_gtk_keys = {
        FIXED_GTK_KEY_1,
        FIXED_GTK_KEY_2,
        FIXED_GTK_KEY_3,
        FIXED_GTK_KEY_4,
    }
};

ti_br_config_t ti_br_config =
{
    .use_external_dhcp_server = FEATURE_EXTERNAL_DHCP_SERVER_ENABLE,
    .external_dhcp_server_addr = FEATURE_EXTERNAL_DHCP_SERVER_ADDR,
    .use_external_radius_server = FEATURE_EXTERNAL_RADIUS_SERVER_ENABLE,
    .external_radius_server_addr = FEATURE_EXTERNAL_RADIUS_SERVER_ADDR,
    .external_radius_server_shared_secret = FEATURE_EXTERNAL_RADIUS_SERVER_SHARED_SECRET,
    .external_radius_server_shared_secret_length = FEATURE_EXTERNAL_RADIUS_SERVER_SHARED_SECRET_LENGTH,
};

configurable_props_t cfg_props =
{
    .phyTxPower = CONFIG_TRANSMIT_POWER,
    .ccaDefaultdBm = CONFIG_CCA_THRESHOLD,
    .uc_channel_function = CONFIG_CHANNEL_FUNCTION,
    .uc_fixed_channel = CONFIG_UNICAST_FIXED_CHANNEL_NUM,
    .uc_dwell_interval = CONFIG_UNICAST_DWELL_TIME,
    .bc_channel_function = CONFIG_CHANNEL_FUNCTION,
    .bc_fixed_channel = CONFIG_BROADCAST_FIXED_CHANNEL_NUM,
    .bc_interval = CONFIG_BROADCAST_INTERVAL,
    .bc_dwell_interval = CONFIG_BROADCAST_DWELL_TIME,
    .pan_id = CONFIG_PAN_ID,
    .network_name = CONFIG_NETNAME,
    .bc_channel_list = CONFIG_BROADCAST_CHANNEL_MASK,
    .uc_channel_list = CONFIG_UNICAST_CHANNEL_MASK,
    .async_channel_list = CONFIG_ASYNC_CHANNEL_MASK,
    .wisun_device_type = CONFIG_WISUN_DEVICE_TYPE,
    .ch0_center_frequency = CONFIG_CENTER_FREQ * 1000,
    .config_channel_spacing = CONFIG_CHANNEL_SPACING,
    .config_number_of_channels = CONFIG_TOTAL_CHANNELS,
    .config_phy_id = CONFIG_PHY_ID,
    .config_reg_domain = CONFIG_REG_DOMAIN,
    .operating_class = CONFIG_OP_MODE_CLASS,
    .operating_mode = CONFIG_OP_MODE_ID,
    .fan_support_version = 1,
    .config_chan_plan = 0,
    .config_chan_plan_id = 255,
    .hwaddr = CONFIG_INVALID_HWADDR,
#ifdef WISUN_FAN_CORE_1_1
    .mdr_enable = 0 ,
    .num_phy_mode = 1,
    .Phy_Mode_Id = {CONFIG_PHY_ID},
#endif
    .channel_page = CONFIG_CHANNEL_PAGE,
    .rx_on_when_idle = true,
#ifdef FEATURE_FULL_FUNCTION_DEVICE
    .ffd = true,
#else
    .ffd = false,
#endif
    .regulatory_channel_list = CONFIG_REGULATION_CHANNEL_MASK,
};

/******************************************************************************
Function declarations Local & Global
 *****************************************************************************/
extern void ws_br_handler_init();
extern void border_router_loadcfg_init(void);
extern bool is_net_if_up(void);

#ifdef WISUN_NCP_ENABLE
extern void platformNcpSendProcess();
extern void platformNcpSendAsyncProcess();
#ifdef WISUN_AUTO_START
static inline void auto_start_assert_led();
static inline void autoStartSignal();
extern otError nanostack_net_if_up();
extern otError nanostack_net_stack_up(void);
#endif //WISUN_AUTO_START

#if (WISUN_APP_TCP_MODE > 0)
/******************************************************************************
 TCP — private (static) forward declarations
 *****************************************************************************/
static void tcp_reset_state(void);
static void tcp_socket_callback(void *cb);
#if (WISUN_APP_TCP_MODE == 2)
static bool tcp_init_peer_address(void);
static void tcp_reconnect_timer_cb(void *arg);
#endif

/* NCP externs — implemented in ncp_base_mtd.cpp */
extern void nanostack_notify_tcp_status(uint8_t event, uint8_t connected);
extern void nanostack_notify_tcp_data(const uint8_t *data, uint16_t len);

/******************************************************************************
 TCP — public API
 *****************************************************************************/

/*! (MODE 1) Open, bind, and listen on TCP_PORT. */
bool tcp_socket_setup(uint16_t port);

/*! (MODE 2) Connect to configured peer. addr_bytes=NULL uses TCP_PEER_ADDR_STR. */
bool tcp_client_connect(const char *addr, uint16_t port);

/*! Send data to connected peer (both modes). */
bool tcp_send_data(const uint8_t *data, uint16_t len);

/*! Full shutdown: close all sockets, cancel timers. */
void tcp_disconnect(void);

/*! Returns true if conn_state == TCP_STATE_CONNECTED. */
bool tcp_is_connected(void);

void tcp_init_br(void);
bool tcp_send_data_to(int slot, const uint8_t *data, uint16_t len);
int tcp_get_mode(void);
const char *tcp_get_mode_str(void);
int tcp_client_count(void);

#endif /* WISUN_APP_TCP_MODE */
#endif //WISUN_NCP_ENABLE

#ifdef WISUN_TEST_MPL_EMBEDDED
#define SEND_BUF_SIZE 24
#define multicast_addr_str "ff15::810a:64d1"
#define UDP_PKT_INTERVAL 1000
#define MASTER_GROUP 0
#define MY_GROUP 1

static uint8_t recv_buffer[SEND_BUF_SIZE] = {0};
static uint32_t num_pkts = 0;

int8_t socket_id;
uint8_t multi_cast_addr[16] = {0};

uint32_t gPktCount = 0;
timeout_t *gUDP_pkt_timeout;
void handle_message(char* msg);
extern void timac_GetBC_Slot_BFIO(uint16_t *slot, uint32_t *bfio);
uint16_t startUDPArgs[2];
#endif

bool is_in_eapol_eui_allow_list(uint8_t *euiAddress);
bool insert_eapol_eui_allow_list(uint8_t* euiAddress);
bool remove_eapol_eui_allow_list(uint8_t* euiAddress);


/******************************************************************************
Function definitions
 *****************************************************************************/

/*!
 * Configure the network size - pre step to configuring and
 * starting the network.
 * MBED_CONF_MBED_MESH_API_WISUN_NETWORK_SIZE macro defined in
 * application/mbed_config_app.h is used in this function to
 * determine the network size.
 * In the Out of Box example, this macro is set to a small
 * network i.e less than or around 100 nodes
 */
#ifndef WISUN_RCP_HOST
extern const char *ti154stack_lib_version;
extern const char *ti154stack_lib_date;
extern const char *ti154stack_lib_time;
extern const char *wisun_stack_version;
extern const char *wisun_protocol_version;
#endif
mesh_error_t nanostack_wisunInterface_configure(void)
{
    int ret;
#ifndef WISUN_RCP_HOST
    tr_info("Library info | Date: %s, Time: %s, Version: %s", ti154stack_lib_date, ti154stack_lib_time,
            ti154stack_lib_version);
    tr_info("Wi-SUN stack version: %s, Wi-SUN protocol version: %s", wisun_stack_version, wisun_protocol_version);
#endif

    if (_configured) {
        // Already configured
        return MESH_ERROR_NONE;
    }
#ifdef MBED_CONF_MBED_MESH_API_WISUN_NETWORK_SIZE
#ifndef TI_WISUN_FAN_OPT
    ret = ws_management_network_size_set(interface_id, MBED_CONF_MBED_MESH_API_WISUN_NETWORK_SIZE);
    if (ret < 0) {
        return MESH_ERROR_PARAM;
    }
#else
    ws_cfg_set_intferface_all();
#endif //TI_WISUN_FAN_OPT
#endif

    _configured = true;
    return MESH_ERROR_NONE;
}

#ifndef WISUN_NCP_ENABLE
/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    int16_t ret;

    /* Configure the LED pins */
    GPIO_setConfig(CONFIG_GPIO_GLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_RLED, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    /* defualt user LED's */
    GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
    GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);

#ifndef NWK_TEST
    /* Configure the button pins */
    GPIO_setConfig(CONFIG_GPIO_BTN1, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);
    GPIO_setConfig(CONFIG_GPIO_BTN2, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Setup callback for btn int */
    GPIO_setCallback(CONFIG_GPIO_BTN1, btn_interrupt_handler);
    GPIO_enableInt(CONFIG_GPIO_BTN1);
    GPIO_setCallback(CONFIG_GPIO_BTN2, btn_interrupt_handler);
    GPIO_enableInt(CONFIG_GPIO_BTN2);
#endif //NWK_TEST

    tr_info("Border Router Example: US 915MHz, Channel 11, security disabled");

    if(MESH_ERROR_NONE != nanostack_wisunInterface_configure())
    {
        // error occurred; do not proceed further
        while(1);
    }

    ws_br_handler_init();

    border_router_loadcfg_init();

    if(!is_net_if_up())
    {
        // error occurred; do not proceed further
        while(1);
    }

    // if here: all ok - kick start border router tasklet
    border_router_tasklet_start();

#ifdef NWK_TEST
    /* this should only be initialized once */
    nwkTest_init();
#endif

    while (1) {

#ifdef NWK_TEST
        nwkTest_run();
#endif
        //sleep for 100ms
        usleep(30000);
    }
}

#else //WISUN_NCP_ENABLE

/*!
 * Signal NCP tasklet with the event NCP_SEND_RESPONSE_EVENT,
 * so that NCP_tasklet can process the sending of a response
 * back to the host, when the host sends a command.
 * e.g. Response to a command from host to set/get configuration.
 */
void platformNcpSendRspSignal()
{
    //post an event to ncp_tasklet
    arm_event_s event = {
           .sender = 0,
           .receiver = ncp_tasklet_id,
           .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
           .event_type = NCP_SEND_RESPONSE_EVENT,
           .event_id = 0,
           .event_data = 0
       };

   eventOS_event_send(&event);
}

/*!
 * Signal NCP tasklet with the event NCP_SEND_ASYNC_RSPONSE_EVENT
 * so that NCP tasket can process the sending of an async response
 * back to the host - e.g. reception of a packet by the NWP
 */
void platformNcpSendAsyncRspSignal()
{
    //post an event to ncp_tasklet
    arm_event_s event = {
           .sender = 0,
           .receiver = ncp_tasklet_id,
           .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
           .event_type = NCP_SEND_ASYNC_RSPONSE_EVENT,
           .event_id = 0,
           .event_data = 0
       };

   eventOS_event_send(&event);
}

/*!
 * Callback from the UART module indicating need for processing
 * incoming message to NCP.
 */
void platformUartSignal(uintptr_t arg)
{
    //post an event to ncp_tasklet
    arm_event_s event = {
           .sender = 0,
           .receiver = ncp_tasklet_id,
           .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
           .event_type = NCP_UART_EVENT,
           .event_id = 0,
           .event_data = arg
       };

   eventOS_event_send(&event);
}

#ifdef WISUN_AUTO_START
/*!
 * Blink Leds continuously when an assert occurs
 */
static inline void auto_start_assert_led()
{
    while(1)
    {
        sleep(2);
        GPIO_toggle(CONFIG_GPIO_GLED);
        GPIO_toggle(CONFIG_GPIO_RLED);
    }
}

/*!
 * Post event to NCP tasklet to do net interface configuration
 * and start wisun stack without having to receive commands on
 * the NCP interface
 */
static inline void autoStartSignal()
{
    arm_event_s auto_event = {
                           .sender = 0,
                           .receiver = ncp_tasklet_id,
                           .priority = ARM_LIB_HIGH_PRIORITY_EVENT,
                           .event_type = NCP_AUTO_START_EVENT,
                           .event_id = 0,
                           .event_data = 0
                         };

    eventOS_event_send(&auto_event);
}
#endif //WISUN_AUTO_START

#ifdef WISUN_TEST_MPL_EMBEDDED
/*!
 * Callback for handling any activity on the udp socket
 */
void socket_callback(void *cb)
{
    socket_callback_t *sock_cb = (socket_callback_t *) cb;

#ifdef WISUN_TEST_METRICS
    int16_t len;
    ns_address_t source_addr;
#endif
    tr_debug("socket_callback() sock=%d, event=0x%x, interface=%d, data len=%d",
             sock_cb->socket_id, sock_cb->event_type, sock_cb->interface_id, sock_cb->d_len);

    switch (sock_cb->event_type & SOCKET_EVENT_MASK) {
        case SOCKET_DATA:
            tr_info("socket_callback: SOCKET_DATA, sock=%d, bytes=%d", sock_cb->socket_id, sock_cb->d_len);

#ifdef WISUN_TEST_METRICS
            tr_mpl("socket_callback: SOCKET_DATA, sock=%d, bytes=%d", sock_cb->socket_id, sock_cb->d_len);

            /* Convert string addr to ipaddr array */
            len = socket_recvfrom(socket_id, recv_buffer, sizeof(recv_buffer), 0, &source_addr);
            if(len > 0)
              {
                  num_pkts++;
                  tr_mpl("Recv[%d]: %s, Pkts:%d", len, recv_buffer, num_pkts);
                  handle_message((char*)recv_buffer);
              }
              else if(NS_EWOULDBLOCK != len)
              {
                  tr_mpl("Recv error %x", len);
              }
#endif
            break;
        case SOCKET_CONNECT_DONE:
            tr_info("socket_callback: SOCKET_CONNECT_DONE");
            break;
        case SOCKET_CONNECT_FAIL:
            tr_info("socket_callback: SOCKET_CONNECT_FAIL");
            break;
        case SOCKET_CONNECT_AUTH_FAIL:
            tr_info("socket_callback: SOCKET_CONNECT_AUTH_FAIL");
            break;
        case SOCKET_INCOMING_CONNECTION:
            tr_info("socket_callback: SOCKET_INCOMING_CONNECTION");
            break;
        case SOCKET_TX_FAIL:
            tr_info("socket_callback: SOCKET_TX_FAIL");
            break;
        case SOCKET_CONNECT_CLOSED:
            tr_info("socket_callback: SOCKET_CONNECT_CLOSED");
            break;
        case SOCKET_CONNECTION_RESET:
            tr_info("socket_callback: SOCKET_CONNECTION_RESET");
            break;
        case SOCKET_NO_ROUTE:
            tr_info("socket_callback: SOCKET_NO_ROUTE");
            break;
        case SOCKET_TX_DONE:
            tr_info("socket_callback: SOCKET_TX_DONE");
            break;
        case SOCKET_NO_RAM:
            tr_info("socket_callback: SOCKET_NO_RAM");
            break;
        case SOCKET_CONNECTION_PROBLEM:
            tr_info("socket_callback: SOCKET_CONNECTION_PROBLEM");
            break;
        default:
            break;
    }
}

/*!
 * Setup udp socket and bind to a specific port number
 */
bool udpSocketSetup(void)
{
    int8_t ret;
    ns_ipv6_mreq_t mreq;
    ns_address_t bind_addr;

    tr_info("opening udp socket");
    socket_id = socket_open(SOCKET_UDP, 0, socket_callback);
    if (socket_id < 0) {
        tr_debug("socket open failed with error %d", socket_id);
        return false;
    }

    // how many hops the multicast message can go
    static const int16_t multicast_hops = 2;
    socket_setsockopt(socket_id, SOCKET_IPPROTO_IPV6, SOCKET_IPV6_MULTICAST_HOPS, &multicast_hops, sizeof(multicast_hops));

    static const int32_t buf_size = 20;
    int32_t rtn = 20;
//    rtn = socket_setsockopt(socket_id, SOCKET_SOL_SOCKET, SOCKET_SO_RCVBUF, &buf_size, sizeof buf_size);
//    tr_info("set rx buffer len %x, status %x", buf_size, rtn);
/*
    rtn = socket_setsockopt(socket_id, SOCKET_SOL_SOCKET, SOCKET_SO_SNDBUF, &buf_size, sizeof buf_size);
    tr_info("set Tx buffer len %x, status %x", buf_size, rtn);
*/
    /* Convert string addr to ipaddr array */
    stoip6(multicast_addr_str, strlen(multicast_addr_str), multi_cast_addr);

    memcpy(mreq.ipv6mr_multiaddr, multi_cast_addr, 16);
    mreq.ipv6mr_interface = 0;
    socket_setsockopt(socket_id, SOCKET_IPPROTO_IPV6, SOCKET_IPV6_JOIN_GROUP, &mreq, sizeof(mreq));

    bind_addr.type = ADDRESS_IPV6;
    memcpy(bind_addr.address, ns_in6addr_any, 16);
    bind_addr.identifier = UDP_PORT_TEST;
    ret = socket_bind(socket_id, &bind_addr);
    if (ret < 0) {
        tr_error("socket bind failed with error %d", ret);
        return false;
    }
    return true;
}

/*!
 * Process received message
 */
void handle_message(char* msg) {
    uint8_t state=0;
    uint16_t group=0xffff;

    if (strstr(msg, "t:lights;") == NULL) {
       return;
    }

    if (strstr(msg, "s:1;") != NULL) {
        state = 1;
    }
    else if (strstr(msg, "s:0;") != NULL) {
        state = 0;
    }

    // 0==master, 1==default group
    char *msg_ptr = strstr(msg, "g:");
    if (msg_ptr) {
        char *ptr;
        group = strtol(msg_ptr, &ptr, 10);
    }

    // in this example we only use one group
    if (group==MASTER_GROUP || group==MY_GROUP) {
        GPIO_write(CONFIG_GPIO_RLED, state);
    }
}

/*!
 * Send UDP Traffic to configured Socket ID
 * This function gets called by the eventOS timer
 * Timer is canceled at the completion of sending
 * desired UDP packets.
 */
void sendUDPTraffic () {
//    uint16_t count;
    int16_t ret;
    ns_address_t send_addr = {0};
    uint16_t slotIdx;
    uint32_t bfio;
    uint8_t hopCount = 1;
    uint16_t pktLen = 20;
    int16_t multicast_hops;
    uint8_t* send_buf;

    /* Set multicast send address */
    send_addr.type = ADDRESS_IPV6;
    send_addr.identifier = UDP_PORT_TEST;
    memcpy(send_addr.address, multi_cast_addr, 16);

    hopCount = (uint8_t) startUDPArgs[0];
    pktLen   = (uint16_t) startUDPArgs[1];

    send_buf = malloc(pktLen);

    timac_GetBC_Slot_BFIO(&slotIdx, &bfio);
//    tr_mpl("UDP payload slot(%d), BFIO(%u)", slotIdx, bfio);

    if (send_buf)
    {
        snprintf((char*)send_buf, pktLen, "Id:%d:bfio:%u:%d:%d", slotIdx,bfio, hopCount, pktLen);
    }

    // Send UDP Packet until the desired numbers
    if (gPktCount--) {
        // Set #of hops for multicast message
        multicast_hops = hopCount;
        socket_setsockopt(socket_id, SOCKET_IPPROTO_IPV6, SOCKET_IPV6_MULTICAST_HOPS, &multicast_hops, sizeof(multicast_hops));
        ret = socket_sendto(socket_id, &send_addr, send_buf, pktLen);
    } else {
       eventOS_timeout_cancel(gUDP_pkt_timeout);
    }

    if (send_buf)
    {
        free(send_buf);
    }
}

/*!
 * Set up number of UDP packets to be send
 * and set up timer to send the packets.
 */
void startUDPTraffic (uint32_t numPkts, uint8_t pktInterval, uint8_t hopCount, uint16_t pktLen) {
    uint32_t updPktInterval;

    if (numPkts){
        gPktCount = numPkts;
    } else {
        gPktCount = 0xFFFFFFFF;
    }
    startUDPArgs[0] = hopCount;
    startUDPArgs[1] = pktLen;
    // Convert seconds to mSec
    updPktInterval = pktInterval * 1000;
    gUDP_pkt_timeout = eventOS_timeout_every_ms(sendUDPTraffic, updPktInterval, NULL);
}
#endif

/*!
 * Core logic for NCP tasklet. Helps process incoming, outgoing
 * messages on NCP interface based on the received event
 */
void ncp_tasklet(arm_event_s *event)
{
    arm_library_event_type_e event_type;
    event_type = (arm_library_event_type_e)event->event_type;
    switch (event_type)
    {
            case ARM_LIB_TASKLET_INIT_EVENT:

                ncp_tasklet_id = event->receiver;

                OtStack_instance = otInstanceInitSingle();
                assert(OtStack_instance);

                otNcpInit(OtStack_instance);

                GPIO_write(CONFIG_GPIO_RLED, 1);

                for(int i = 0; i< 3; i++)
                {
                    GPIO_toggle(CONFIG_GPIO_GLED);
                    usleep(300000);
                }

/* TCP has no button handling on the border router */

#ifdef WISUN_AUTO_START
                //post an event to ncp_tasklet
                autoStartSignal();
#endif //WISUN_AUTO_START

                break;

#ifdef WISUN_AUTO_START
            case NCP_AUTO_START_EVENT:

                GPIO_write(CONFIG_GPIO_RLED, 1);

                /* Automatically  bring the interface up & initiate Joining */
                /*  Equivalent to running ifconfig up and wisunstack start by default at startup */
                if(nanostack_net_if_up() != OT_ERROR_NONE)
                {
                    //assert here and blink leds in loop
                    auto_start_assert_led();
                }
                if(nanostack_net_stack_up() != OT_ERROR_NONE)
                {
                    //assert here and blink leds in loop
                    auto_start_assert_led();
                }
                /* Note: TCP initialization happens automatically in borderrouter_ws.c
                 * when the bootstrap is ready (ARM_NWK_BOOTSTRAP_READY event).
                 * This ensures TCP is only started after the network is fully operational.
                 */
                break;
#endif //WISUN_AUTO_START

            case NCP_UART_EVENT:
                platformUartProcess(event->event_data);
                break;

            case NCP_SEND_RESPONSE_EVENT:
                platformNcpSendProcess();
                break;

            case NCP_SEND_ASYNC_RSPONSE_EVENT:
                platformNcpSendAsyncProcess();
                break;
            default:
                break;
     }
}

/*!
 * Create the NCP tasklet whose core logic is defined in ncp_tasklet()
 * Also, post the event to initialize it after creating the tasklet
 *
 */
void ncp_tasklet_start(void)
{
        eventOS_event_handler_create(
        &ncp_tasklet,
        ARM_LIB_TASKLET_INIT_EVENT);
}

/*!
 * Helper function - returns the first non-zero channel from a list of
 * channels that is sent as input.
 */
uint8_t get_first_fixed_channel(uint8_t * channel_list)
{
    uint8_t idx, sizeOfChannelMask;
    uint8_t fixedChannelNum = 0;
    uint8_t bit_location = 0;
    uint8_t byteEntry = 0;
    sizeOfChannelMask = CHANNEL_BITMAP_SIZE;

    for(idx = 0; idx < sizeOfChannelMask; idx++)
    {
        byteEntry = channel_list[idx];
        bit_location = 0;
        while (bit_location < 8)
        {
            if (byteEntry & 0x01) {
                return fixedChannelNum;
            }
            else {
               fixedChannelNum++;
            }

            bit_location++;
            // shift byteEntry 1 to the right to pop off last bit
            byteEntry = byteEntry >> 1;
        }
    }
    return fixedChannelNum;
}

sAddrExt_t zeroExtAddr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
sAddrExt_t eapol_eui_allow_list[EAPOL_EUI_LIST_SIZE];
bool is_in_eapol_eui_allow_list(uint8_t *euiAddress)
{
    uint16_t index = 0;

    for(index = 0; index < EAPOL_EUI_LIST_SIZE; index++)
    {
      if(sAddrExtCmp(euiAddress, eapol_eui_allow_list[index]))
      {
          /* specified dest address found in list */
          return true;
      }
    }
    return false;
}

bool insert_eapol_eui_allow_list(uint8_t* euiAddress)
{
    uint8_t index = 0;
    bool retVal = false;
    if (is_in_eapol_eui_allow_list(euiAddress))
    {
        // Already in allow list
        return true;
    }

    for(index = 0; index < EAPOL_EUI_LIST_SIZE; index++)
    {
        if(sAddrExtCmp(eapol_eui_allow_list[index], zeroExtAddr))
        {
            /* copy the string over if there is nothing stored there */
            sAddrExtCpy(eapol_eui_allow_list[index], euiAddress);
            retVal = true;
            break;
        }
    }

    return(retVal);
}
bool remove_eapol_eui_allow_list(uint8_t* euiAddress)
{
    uint8_t index = 0;
    bool retVal = false;

    for(index = 0; index < EAPOL_EUI_LIST_SIZE; index++)
    {
        if(sAddrExtCmp(eapol_eui_allow_list[index], euiAddress))
        {
            /* copy the string over if there is nothing stored there */
            sAddrExtCpy(eapol_eui_allow_list[index], zeroExtAddr);
            retVal = true;
            break;
        }
    }

    return(retVal);
}

#ifdef WISUN_TEST_METRICS
/*
 * Get latest test metrics
 */
void get_test_metrics(test_metrics_s *test_metrics)
{
    test_metrics->revision = 3;
    // Populate join time
    memcpy(&test_metrics->join_time, &node_join_time, sizeof(JOIN_TIME_s));

    // Populate MAC debug
    timac_getMACDebugCounts(&test_metrics->mac_debug);

    // Populate heap debug
    const mem_stat_t *heap_stats = ns_dyn_mem_get_mem_stat();

    test_metrics->heap_debug.heap_sector_size = heap_stats->heap_sector_size;
    test_metrics->heap_debug.heap_sector_allocated_bytes =
            heap_stats->heap_sector_allocated_bytes;
    test_metrics->heap_debug.heap_sector_allocated_bytes_max =
            heap_stats->heap_sector_allocated_bytes_max;

    timac_getMACPerfData(&test_metrics->mac_perf_data);

    // Populate length
    test_metrics->length = (uint16_t) sizeof(test_metrics_s);
}
#endif

#endif /* WISUN_NCP_ENABLE — closes #else at line 512 */

#ifdef WISUN_NCP_ENABLE
#if (WISUN_APP_TCP_MODE > 0)

/******************************************************************************
 TCP Internal helpers
 *****************************************************************************/

/*!
 * Session reset: disconnect the active data path while preserving infra.
 * MODE 1: closes client_socket_id only; listen socket stays alive (→ LISTENING).
 * MODE 2: closes socket_id, cancels reconnect timer (→ IDLE).
 * For full shutdown call tcp_disconnect() instead.
 */
static void tcp_reset_state(void)
{

#if (WISUN_APP_TCP_MODE == 1)
    if (g_tcp_state.client_socket_id >= 0)
    {
        socket_close(g_tcp_state.client_socket_id);
        g_tcp_state.client_socket_id = NOT_INITIALIZED;
    }
    g_tcp_state.conn_state = (g_tcp_state.socket_id >= 0)
                             ? TCP_STATE_LISTENING : TCP_STATE_IDLE;
#else
    if (g_tcp_state.socket_id >= 0)
    {
        socket_close(g_tcp_state.socket_id);
        g_tcp_state.socket_id = NOT_INITIALIZED;
    }
    g_tcp_state.conn_state = TCP_STATE_IDLE;
    if (tcp_reconnect_timeout != NULL)
    {
        eventOS_timeout_cancel(tcp_reconnect_timeout);
        tcp_reconnect_timeout = NULL;
    }
#endif
}

#if (WISUN_APP_TCP_MODE == 2)
/*!
 * One-shot OS timer callback: fired after TCP_RECONNECT_DELAY_MS.
 * Issues a fresh connect attempt, preserving the existing peer address.
 */
static void tcp_reconnect_timer_cb(void *arg)
{
    (void)arg;
    tcp_reconnect_timeout = NULL;
    tr_info("TCP: Reconnect timer fired — attempting connect");
    tcp_client_connect(NULL, 0);
}
#endif /* WISUN_APP_TCP_MODE == 2 */

/******************************************************************************
 TCP Socket callback
 *****************************************************************************/

/*!
 * Callback for handling any activity on the TCP socket
 */
static void tcp_socket_callback(void *cb)
{
    socket_callback_t *sock_cb = (socket_callback_t *) cb;
    int16_t len;

    tr_info("tcp_socket_callback() sock=%d event=0x%x", sock_cb->socket_id, sock_cb->event_type);

    switch (sock_cb->event_type & SOCKET_EVENT_MASK)
    {
        case SOCKET_DATA:
            tr_info("tcp_socket_callback: SOCKET_DATA sock=%d bytes=%d",
                    sock_cb->socket_id, sock_cb->d_len);
            len = socket_recv(sock_cb->socket_id, g_tcp_state.recv_buffer,
                              TCP_RECV_BUF_SIZE, 0);
            if (len > 0)
            {
                tr_info("TCP Recv[%d bytes]", len);
                nanostack_notify_tcp_data(g_tcp_state.recv_buffer, (uint16_t)len);
            }
            else if (len == 0)
            {
                tr_info("TCP: Connection closed by peer (EOF)");
                tcp_reset_state();
                nanostack_notify_tcp_status((uint8_t)TCP_STATE_DISCONNECTED, 0);
            }
            else
            {
                tr_warn("TCP: socket_recv error: %d (sock=%d)", len, sock_cb->socket_id);
            }
            break;

        case SOCKET_CONNECT_DONE:
            tr_info("tcp_socket_callback: SOCKET_CONNECT_DONE");
            g_tcp_state.conn_state = TCP_STATE_CONNECTED;
            nanostack_notify_tcp_status((uint8_t)TCP_STATE_CONNECTED, 1);
            break;

        case SOCKET_CONNECT_FAIL:
            tr_info("tcp_socket_callback: SOCKET_CONNECT_FAIL");
            tcp_reset_state();
            nanostack_notify_tcp_status((uint8_t)TCP_STATE_CONNECT_FAILED, 0);
#if (WISUN_APP_TCP_MODE == 2)
            tr_info("TCP: Connect failed — retrying in %u ms", TCP_RECONNECT_DELAY_MS);
            g_tcp_state.conn_state = TCP_STATE_RECONNECT_WAIT;
            tcp_reconnect_timeout = eventOS_timeout_ms(tcp_reconnect_timer_cb,
                                                       TCP_RECONNECT_DELAY_MS, NULL);
#endif
            break;

        case SOCKET_CONNECT_AUTH_FAIL:
            tr_info("tcp_socket_callback: SOCKET_CONNECT_AUTH_FAIL");
            break;

        case SOCKET_INCOMING_CONNECTION:
            tr_info("tcp_socket_callback: SOCKET_INCOMING_CONNECTION sock=%d", sock_cb->socket_id);
#if (WISUN_APP_TCP_MODE == 1)
            tr_info("TCP SERVER: Accepting incoming connection");
            /* Close any stale accepted socket before accepting a new one */
            if (g_tcp_state.client_socket_id >= 0)
            {
                socket_close(g_tcp_state.client_socket_id);
                g_tcp_state.client_socket_id = NOT_INITIALIZED;
                g_tcp_state.conn_state = TCP_STATE_IDLE;
            }
            {
                int8_t new_sock = socket_accept(sock_cb->socket_id,
                                                &g_tcp_state.peer_addr,
                                                tcp_socket_callback);
                if (new_sock >= 0)
                {
                    g_tcp_state.client_socket_id = new_sock;
                    g_tcp_state.conn_state = TCP_STATE_CONNECTED;
                    nanostack_notify_tcp_status((uint8_t)TCP_STATE_CLIENT_ACCEPTED, 1);
                    tr_info("TCP: Connection accepted client_socket=%d", g_tcp_state.client_socket_id);
                }
                else
                {
                    tr_error("TCP: socket_accept failed: %d", new_sock);
                }
            }
#else
            tr_warn("TCP: Got INCOMING_CONNECTION in CLIENT mode — ignoring");
#endif
            break;

        case SOCKET_TX_FAIL:
            tr_warn("tcp_socket_callback: SOCKET_TX_FAIL sock=%d", sock_cb->socket_id);
            break;

        case SOCKET_CONNECT_CLOSED:
            tr_info("tcp_socket_callback: SOCKET_CONNECT_CLOSED sock=%d", sock_cb->socket_id);
            tcp_reset_state();
            nanostack_notify_tcp_status((uint8_t)TCP_STATE_DISCONNECTED, 0);
            break;

        case SOCKET_CONNECTION_RESET:
            tr_info("tcp_socket_callback: SOCKET_CONNECTION_RESET");
            tcp_reset_state();
            nanostack_notify_tcp_status((uint8_t)TCP_STATE_DISCONNECTED, 0);
            break;

        case SOCKET_NO_ROUTE:
            tr_info("tcp_socket_callback: SOCKET_NO_ROUTE");
            break;

        case SOCKET_TX_DONE:
            tr_debug("tcp_socket_callback: SOCKET_TX_DONE sock=%d", sock_cb->socket_id);
            break;

        case SOCKET_NO_RAM:
            tr_warn("tcp_socket_callback: SOCKET_NO_RAM sock=%d", sock_cb->socket_id);
            break;

        case SOCKET_CONNECTION_PROBLEM:
            tr_info("tcp_socket_callback: SOCKET_CONNECTION_PROBLEM");
#if (WISUN_APP_TCP_MODE == 2)
            /* Reset unconditionally — a problem means the connection is broken */
            tr_info("TCP: Connection problem — retrying in %u ms", TCP_RECONNECT_DELAY_MS);
            tcp_reset_state();
            nanostack_notify_tcp_status((uint8_t)TCP_STATE_DISCONNECTED, 0);
            g_tcp_state.conn_state = TCP_STATE_RECONNECT_WAIT;
            tcp_reconnect_timeout = eventOS_timeout_ms(tcp_reconnect_timer_cb,
                                                       TCP_RECONNECT_DELAY_MS, NULL);
#endif
            break;

        default:
            tr_warn("tcp_socket_callback: unhandled event 0x%x", sock_cb->event_type);
            break;
    }
    tcp_connected = (g_tcp_state.conn_state == TCP_STATE_CONNECTED);
}

#if (WISUN_APP_TCP_MODE == 1)
/*!
 * Setup TCP socket for server mode
 * @param port The port number to bind to (IGNORED - always uses TCP_PORT 5678)
 * Note: Port parameter is kept for NCP API compatibility but the application
 *       is hardcoded to use TCP_PORT (5678) for all operations
 */
bool tcp_socket_setup(uint16_t port)
{
    int8_t ret;
    ns_address_t bind_addr;

    tr_info("TCP SERVER SETUP: opening socket on port %d", TCP_PORT);
    g_tcp_state.socket_id = socket_open(SOCKET_TCP, 0, tcp_socket_callback);
    if (g_tcp_state.socket_id < 0)
    {
        tr_error("TCP: socket_open failed: %d", g_tcp_state.socket_id);
        return false;
    }

    bind_addr.type = ADDRESS_IPV6;
    memcpy(bind_addr.address, ns_in6addr_any, 16);
    bind_addr.identifier = (port != 0) ? port : TCP_PORT;
    ret = socket_bind(g_tcp_state.socket_id, &bind_addr);
    if (ret < 0)
    {
        tr_error("TCP: socket_bind failed: %d", ret);
        tcp_reset_state();
        return false;
    }

    ret = socket_listen(g_tcp_state.socket_id, TCP_BACKLOG);
    if (ret < 0)
    {
        tr_error("TCP: socket_listen failed: %d", ret);
        tcp_reset_state();
        return false;
    }

    g_tcp_state.conn_state = TCP_STATE_LISTENING;
    tr_info("TCP SERVER READY: listening on port %d socket_id=%d", TCP_PORT, g_tcp_state.socket_id);
    nanostack_notify_tcp_status((uint8_t)TCP_STATE_LISTENING, 0);
    return true;
}
#else /* WISUN_APP_TCP_MODE == 2 */

/*!
 * Initialize TCP peer address from configuration string
 * Parses TCP_PEER_ADDR_STR and sets up g_tcp_state.target_peer_addr
 * Must be called before tcp_client_connect()
 *
 * @return true if peer address was successfully parsed and set
 */
static bool tcp_init_peer_address(void)
{
    if (strlen(TCP_PEER_ADDR_STR) > 0) {
        /* Parse configured peer address */
        if (stoip6(TCP_PEER_ADDR_STR, strlen(TCP_PEER_ADDR_STR), g_tcp_state.target_peer_addr)) {
            g_tcp_state.peer_addr_valid = true;
            tr_info("TCP: Configured peer address: %s", TCP_PEER_ADDR_STR);
            return true;
        } else {
            tr_error("TCP: Failed to parse TCP_PEER_ADDR_STR: %s", TCP_PEER_ADDR_STR);
            return false;
        }
    } else {
        tr_error("TCP CLIENT: TCP_PEER_ADDR_STR must be configured for client mode!");
        return false;
    }
}

/*!
 * Connect TCP socket as client to peer router node.
 * @param addr  IPv6 address string (e.g. "2020:abcd::1"), or NULL to use TCP_PEER_ADDR_STR.
 * @param port  Destination port (0 defaults to TCP_PORT)
 */
bool tcp_client_connect(const char *addr, uint16_t port)
{
    int8_t ret;
    ns_address_t connect_addr;

    /* Guard: don't start a second attempt if already connecting */
    if (g_tcp_state.conn_state == TCP_STATE_CONNECTING)
    {
        tr_warn("TCP: already CONNECTING — ignoring");
        return true;
    }

    /* Parse string address if provided; fall back to stored or config address */
    if (addr != NULL)
    {
        if (!stoip6(addr, strlen(addr), g_tcp_state.target_peer_addr))
        {
            tr_error("TCP: invalid IPv6 address string: %s", addr);
            return false;
        }
        g_tcp_state.peer_addr_valid = true;
    }
    else if (!g_tcp_state.peer_addr_valid)
    {
        if (!tcp_init_peer_address())
        {
            return false;
        }
    }

    /* Close any leftover socket from a previous attempt */
    if (g_tcp_state.socket_id >= 0)
    {
        socket_close(g_tcp_state.socket_id);
        g_tcp_state.socket_id = NOT_INITIALIZED;
    }

    tr_info("TCP CLIENT CONNECT START");
    g_tcp_state.socket_id = socket_open(SOCKET_TCP, 0, tcp_socket_callback);
    if (g_tcp_state.socket_id < 0)
    {
        tr_error("TCP: socket_open failed: %d", g_tcp_state.socket_id);
        return false;
    }

    connect_addr.type       = ADDRESS_IPV6;
    connect_addr.identifier = (port != 0) ? port : TCP_PORT;
    memcpy(connect_addr.address, g_tcp_state.target_peer_addr, 16);

    ret = socket_connect(g_tcp_state.socket_id, &connect_addr, 0);
    tr_info("TCP CLIENT: socket_connect() returned %d (%d=in_progress)", ret, TCP_CONNECT_IN_PROGRESS);

    if (ret < 0 && ret != TCP_CONNECT_IN_PROGRESS)
    {
        tr_error("TCP: socket_connect failed: %d", ret);
        tcp_reset_state();
        return false;
    }

    g_tcp_state.conn_state = TCP_STATE_CONNECTING;
    tr_info("TCP CLIENT: SYN sent — waiting for SOCKET_CONNECT_DONE");
    return true;
}
#endif /* WISUN_APP_TCP_MODE == 2 */

/* NCP API compatibility functions - provide stubs for the mode not in use */
#if (WISUN_APP_TCP_MODE == 1)
/*!
 * Stub implementation of tcp_client_connect for server mode
 * Server mode doesn't support connecting as a client
 */
bool tcp_client_connect(const char *addr, uint16_t port)
{
    (void)addr; (void)port;
    tr_warn("TCP: tcp_client_connect called in SERVER mode - not supported");
    return false;
}
#else /* WISUN_APP_TCP_MODE == 2 */
/*!
 * Stub implementation of tcp_socket_setup for client mode
 * Client mode doesn't support server listen functionality
 */
bool tcp_socket_setup(uint16_t port)
{
    /* Suppress unused parameter warning */
    (void)port;

    tr_warn("TCP: tcp_socket_setup called in CLIENT mode - not supported");
    return false;
}
#endif /* WISUN_APP_TCP_MODE == 1 */

/*!
 * General-purpose TCP send function
 * Sends data on the appropriate socket based on current role:
 * - Server mode: sends on accepted client socket (g_tcp_state.client_socket_id)
 * - Client mode: sends on connected socket (g_tcp_state.socket_id)
 * @return true on success, false on failure
 */
bool tcp_send_data(const uint8_t* data, uint16_t len)
{
    int8_t send_socket;

    if (data == NULL || len == 0) {
        tr_warn("TCP: Invalid data or length");
        return false;
    }

    if (g_tcp_state.conn_state != TCP_STATE_CONNECTED) {
        tr_warn("TCP: Not connected, cannot send");
        return false;
    }

    /* Determine which socket to use based on compile-time role */
#if (WISUN_APP_TCP_MODE == 1)
    if (g_tcp_state.client_socket_id >= 0) {
        send_socket = g_tcp_state.client_socket_id;
    } else {
        tr_warn("TCP: No valid client socket for sending");
        return false;
    }
#else
    if (g_tcp_state.socket_id >= 0) {
        send_socket = g_tcp_state.socket_id;
    } else {
        tr_warn("TCP: No valid socket for sending");
        return false;
    }
#endif

    tr_debug("TCP: Sending %d bytes on socket %d", len, send_socket);
    int16_t ret = socket_send(send_socket, data, len);
    return (ret >= 0);
}

/*!
 * Initialize TCP for Border Router
 * Should be called after the network is up and BR is operational
 */
void tcp_init_br(void)
{
    if (g_tcp_state.conn_state != TCP_STATE_IDLE)
    {
        tr_info("TCP: Already initialized (state=%d)", g_tcp_state.conn_state);
        return;
    }

    tr_info("==== TCP INIT FOR BORDER ROUTER ====");

#if (WISUN_APP_TCP_MODE == 1)
    if (tcp_socket_setup(0) == false)
    {
        tr_error("TCP: Server socket setup failed");
        return;
    }
    tr_info("TCP SERVER mode enabled — listening on port %d", TCP_PORT);
#else
    if (!tcp_client_connect(NULL, 0))
    {
        tr_debug("TCP Client connect failed — will retry via OS timer");
    }
    else
    {
        tr_info("TCP CLIENT mode enabled — connecting to %s", TCP_PEER_ADDR_STR);
    }
#endif

    tr_info("TCP: BR forwards IPv6 packets between nodes.");
}


/*!
 * Get TCP connection status
 * Returns true if TCP is connected, false otherwise
 */
bool tcp_is_connected(void)
{
    return (g_tcp_state.conn_state == TCP_STATE_CONNECTED);
}

/*!
 * Get TCP peer address
 * Copies the peer IPv6 address to the provided buffer
 */
void tcp_get_peer_address(uint8_t* addr_out)
{
    if (addr_out != NULL && g_tcp_state.conn_state == TCP_STATE_CONNECTED) {
        memcpy(addr_out, g_tcp_state.peer_addr.address, 16);
    }
}

/*!
 * Get TCP peer port
 * Returns the port number of the connected peer
 */
uint16_t tcp_get_peer_port(void)
{
    if (g_tcp_state.conn_state == TCP_STATE_CONNECTED) {
        return g_tcp_state.peer_addr.identifier;
    }
    return 0;
}

/*!
 * Disconnect TCP connection
 * Closes all TCP sockets and resets connection state
 */
void tcp_disconnect(void)
{
    tr_info("TCP: full disconnect");

#if (WISUN_APP_TCP_MODE == 2)
    if (tcp_reconnect_timeout != NULL)
    {
        eventOS_timeout_cancel(tcp_reconnect_timeout);
        tcp_reconnect_timeout = NULL;
    }
#endif

#if (WISUN_APP_TCP_MODE == 1)
    if (g_tcp_state.client_socket_id >= 0)
    {
        socket_close(g_tcp_state.client_socket_id);
        g_tcp_state.client_socket_id = NOT_INITIALIZED;
    }
#endif
    if (g_tcp_state.socket_id >= 0)
    {
        socket_close(g_tcp_state.socket_id);
        g_tcp_state.socket_id = NOT_INITIALIZED;
    }

    g_tcp_state.conn_state = TCP_STATE_IDLE;
    tcp_connected = false;
    nanostack_notify_tcp_status((uint8_t)TCP_STATE_DISCONNECTED, 0);
    tr_info("TCP: Disconnected");
}

/*!
 * Get TCP mode
 * @return 0 for server, 1 for client
 */
int tcp_get_mode(void)
{
#if (WISUN_APP_TCP_MODE == 1)
    return 0;  /* Server mode */
#else
    return 1;  /* Client mode */
#endif
}

/*!
 * Get TCP mode as string
 * @return "server" or "client"
 */
const char *tcp_get_mode_str(void)
{
#if (WISUN_APP_TCP_MODE == 1)
    return "server";
#else
    return "client";
#endif
}

/*!
 * Get number of connected TCP clients
 * @return Number of connected clients (0 or 1 for current implementation)
 */
int tcp_client_count(void)
{
#if (WISUN_APP_TCP_MODE == 1)
    /* Server mode: check if we have a connected client */
    if (g_tcp_state.conn_state == TCP_STATE_CONNECTED && g_tcp_state.client_socket_id >= 0) {
        return 1;
    }
    return 0;
#else
    /* Client mode: we don't accept connections */
    return 0;
#endif
}

/*!
 * Send data to a specific client slot (proxy function)
 * This is a proxy function for compatibility with NCP interface.
 * Current implementation only supports single client, so slot is ignored.
 *
 * @param slot Client slot number (ignored - only slot 0 supported)
 * @param data Data to send
 * @param len Length of data
 * @return true on success, false on failure
 */
bool tcp_send_data_to(int slot, const uint8_t *data, uint16_t len)
{
    /* Current implementation only supports single client */
    if (slot != 0) {
        tr_warn("TCP: send_data_to slot %d not supported (only slot 0)", slot);
        return false;
    }

    /* Just use regular send function */
    return tcp_send_data(data, len);
}

#endif /* WISUN_APP_TCP_MODE */

#endif //WISUN_NCP_ENABLE
