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
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/dpl/ClockP.h>

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
#include "6LoWPAN/ws/ws_config.h"
#include "udp.h"

#include "Core/include/ns_address_internal.h"
#include "NWK_INTERFACE/Include/protocol_abstract.h"
#include "NWK_INTERFACE/Include/protocol.h"
#include "6LoWPAN/ws/ws_bootstrap.h"

#include "nsdynmemLIB.h"

#include "net_rpl.h"
#include "RPL/rpl_protocol.h"
#include "RPL/rpl_policy.h"
#include "RPL/rpl_control.h"
#include "RPL/rpl_objective.h"
#include "RPL/rpl_upward.h"
#include "RPL/rpl_downward.h"
#include "RPL/rpl_structures.h"

#ifdef WISUN_NCP_ENABLE
/* OpenThread Internal/Example Header files */
#include "otsupport/otrtosapi.h"
#include "openthread/ncp.h"
#include "platform/system.h"
#elif defined(COAP_SERVICE_ENABLE)
#include "coap_service_api.h"
#include "eventOS_event_timer.h"
#endif

#include "application.h"
#include "ti_wisunfan_features.h"

#ifdef COAP_OAD_ENABLE
#include "oad.h"
#endif

#include "fh_map_direct.h"
#include "fh_pib.h"
#include "fh_nt.h"

/******************************************************************************
Defines & enums
 *****************************************************************************/
#define TRACE_GROUP "main"

#define SEND_BUF_SIZE 20
#define multicast_addr_str "ff15::810a:64d1"
//#define multicast_addr_str "fd00:7283:7e00:0:212:4b00:1ca1:72ff"
#define UDP_PORT 1234
#define UDP_PORT_ECHO              7        /* Echo Protocol - RFC 862 */
#define MASTER_GROUP 0
#define MY_GROUP 1
#define NOT_INITIALIZED -1

#ifdef COAP_SERVICE_ENABLE
#define COAP_JOIN_URI "join"
#define COAP_LED_URI "led"
#define COAP_RSSI_URI "rssi"
#define COAP_TEST_METRICS_URI "metrics"
#ifdef COAP_PANID_LIST
#define COAP_PANID_LIST_ALLOW_URI "panid/allow"
#define COAP_PANID_LIST_DENY_URI "panid/deny"
#define COAP_PANID_LIST_BULK_URI "panid/bulk"
#define COAP_PANID_REDISCOVER_URI "panid/rediscover"

#define PANID_LIST_ACTION_ADD 0
#define PANID_LIST_ACTION_DEL 1

#endif //COAP_PANID_LIST

#define COAP_RLED_ID 0
#define COAP_GLED_ID 1

#ifdef COAP_PANID_LIST
#define PAN_REDISCOVER_BLINK_TIMER_ID 0
#define PAN_REDISCOVER_JOIN_RESP_TIMER_ID 1

#define PAN_REDISCOVER_JOIN_TIMEOUT_SEC     1800 // 30 min
#define PAN_REDISCOVER_JOIN_REQ_TIMEOUT_SEC 2    // 2 sec

typedef enum pan_rediscover_evt {
    PAN_REDISCOVER_INIT_EVT    = 0,
    PAN_REDISCOVER_START_EVT   = 1,
    PAN_REDISCOVER_WAIT_EVT    = 2,
    PAN_REDISCOVER_JOIN_EVT    = 3,
    PAN_REDISCOVER_RESTART_EVT = 4,
} pan_rediscover_evt_t;
#endif //COAP_PANID_LIST

#endif // COAP_SERVICE_ENABLE

typedef enum connection_status {
    CON_STATUS_LOCAL_UP           = 0,        /*!< local IP address set */
    CON_STATUS_GLOBAL_UP          = 1,        /*!< global IP address set */
    CON_STATUS_DISCONNECTED       = 2,        /*!< no connection to network */
    CON_STATUS_CONNECTING         = 3,        /*!< connecting to network */
    CON_STATUS_ERROR_UNSUPPORTED  = 4
} connection_status_t;


/******************************************************************************
 Static & Global Variables
 *****************************************************************************/
static connection_status_t _connect_status;
/*static*/ volatile bool connectedFlg = false;
/*static*/ int8_t interface_id = NOT_INITIALIZED;
static bool _blocking = false;
static bool _configured = false;

static int8_t socket_id;
static uint8_t send_buf[SEND_BUF_SIZE] = {0};
static uint8_t send_buf_unicast[SEND_BUF_SIZE] = {0};

static uint8_t recv_buffer[SEND_BUF_SIZE] = {0};
static uint8_t multi_cast_addr[16] = {0};
static uint8_t uni_cast_addr[16] = {0};
static ns_address_t send_addr = {0};
static ns_address_t send_addr_unicast = {0};
extern bool sent_dao;
extern uint8_t root_unicast_addr[16];
static bool bcast_send = false;
static bool unicast_send = true;

#ifdef NWK_TEST
uint32_t ticks_before_joining = 0;
uint32_t ticks_after_joining = 0;
#endif //NWK_TEST

#ifdef COAP_SERVICE_ENABLE
int8_t service_id = -1;
static uint8_t led_state[2];

#ifdef COAP_PANID_LIST
static int8_t pan_rediscover_tasklet_id = -1;
extern uint16_t panid_list_clear_timeout_sec;
#endif

#endif

#ifdef WISUN_NCP_ENABLE
int8_t ncp_tasklet_id = -1;
otInstance *OtStack_instance = NULL;

otError nanostack_net_if_up(void);
otError nanostack_net_stack_up(void);
#ifdef WISUN_AUTO_START
static inline void auto_start_assert_led();
static inline void autoStartSignal();
#endif //WISUN_AUTO_START
#endif //WISUN_NCP_ENABLE

#ifdef WISUN_TEST_METRICS
static uint32_t num_pkts = 0;
static void handle_message(char* msg);

#ifdef WISUN_TEST_MPL_UDP
static void get_txFrameInfo(char* msg, uint16_t* txIdx, uint32_t* txbfio);
extern void timac_GetBC_Slot_BFIO(uint16_t *slot, uint32_t *bfio);
#endif

#endif

/* variables to help fetch rssi of neighbor nodes */
uint8_t cur_num_nbrs;
uint8_t nbr_idx = 0;
nbr_node_metrics_t nbr_nodes_metrics[SIZE_OF_NEIGH_LIST];

uint8_t get_current_net_state(void);

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

// Unused by non-border routers
ti_br_config_t ti_br_config = {};

configurable_props_t cfg_props = { .phyTxPower = CONFIG_TRANSMIT_POWER, \
                                   .ccaDefaultdBm = CONFIG_CCA_THRESHOLD, \
                                   .uc_channel_function = CONFIG_CHANNEL_FUNCTION, \
                                   .uc_fixed_channel = CONFIG_UNICAST_FIXED_CHANNEL_NUM, \
                                   .uc_dwell_interval = CONFIG_UNICAST_DWELL_TIME,\
                                   .bc_channel_function = 0, \
                                   .bc_fixed_channel = 0, \
                                   .bc_interval = 0,\
                                   .bc_dwell_interval = 0, \
                                   .pan_id = CONFIG_PAN_ID, \
                                   .network_name = CONFIG_NETNAME, \
                                   .bc_channel_list = CONFIG_BROADCAST_CHANNEL_MASK, \
                                   .uc_channel_list = CONFIG_UNICAST_CHANNEL_MASK, \
                                   .async_channel_list = CONFIG_ASYNC_CHANNEL_MASK, \
                                   .wisun_device_type = CONFIG_WISUN_DEVICE_TYPE, \
                                   .ch0_center_frequency = CONFIG_CENTER_FREQ * 1000, \
                                   .config_channel_spacing = CONFIG_CHANNEL_SPACING, \
                                   .config_phy_id = CONFIG_PHY_ID, \
                                   .config_reg_domain = CONFIG_REG_DOMAIN, \
                                   .operating_class = CONFIG_OP_MODE_CLASS, \
                                   .operating_mode = CONFIG_OP_MODE_ID, \
                                   .hwaddr = CONFIG_INVALID_HWADDR};


/******************************************************************************
Function declarations Local & Global
 *****************************************************************************/
#ifdef FEATURE_TIMAC_SUPPORT
extern void timacExtaddressRegister();
#endif

mesh_error_t nanostack_wisunInterface_bringup();
mesh_error_t nanostack_wisunInterface_connect(bool blocking);
void nanostack_wait_till_connect();

#ifdef WISUN_NCP_ENABLE
extern void platformNcpSendProcess();
extern void platformNcpSendAsyncProcess();
#endif

#ifdef COAP_SERVICE_ENABLE
static void pan_rediscover_tasklet_start(void);
static void pan_rediscover_update(uint8_t event_type);
static void pan_rediscover_tasklet(arm_event_s *event);
static int coap_recv_cb(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr);
static int coap_recv_cb_rssi(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr);
static int coap_panid_list_cb(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr);

#ifdef WISUN_TEST_METRICS
static int coap_recv_cb_tstmetrics(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr);
#endif
#endif

#ifdef WISUN_TEST_METRICS
void get_test_metrics(test_metrics_s *test_metrics);
#endif

/******************************************************************************
Function definitions
 *****************************************************************************/

/*!
 * Callback for handling any status while a node is joining a network.
 */
void nanostackNetworkHandler(mesh_connection_status_t status)
{
    tr_debug("nanostackNetworkHandler: %x", status);

    if (_blocking) {
        if (_connect_status == CON_STATUS_CONNECTING
                && (status == MESH_CONNECTED || status == MESH_CONNECTED_LOCAL
                    || status == MESH_CONNECTED_GLOBAL)) {
            connectedFlg = true;
        } else if (status == MESH_DISCONNECTED) {
            connectedFlg = false;
        }
    }

    if (status == MESH_CONNECTED) {
        uint8_t temp_ipv6_global[16];
        uint8_t temp_ipv6_local[16];
        if (arm_net_address_get(interface_id, ADDR_IPV6_LL, temp_ipv6_local) == 0) {
            tr_info("nanostackNetworkHandler: CON_STATUS_LOCAL_UP, IP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                temp_ipv6_local[0], temp_ipv6_local[1],
                temp_ipv6_local[2], temp_ipv6_local[3],
                temp_ipv6_local[4], temp_ipv6_local[5],
                temp_ipv6_local[6], temp_ipv6_local[7],
                temp_ipv6_local[8], temp_ipv6_local[9],
                temp_ipv6_local[10], temp_ipv6_local[11],
                temp_ipv6_local[12], temp_ipv6_local[13],
                temp_ipv6_local[14], temp_ipv6_local[15]);
            _connect_status = CON_STATUS_LOCAL_UP;
        }
        if (arm_net_address_get(interface_id, ADDR_IPV6_GP, temp_ipv6_global) == 0
                && (memcmp(temp_ipv6_global, temp_ipv6_local, 16) != 0)) {
            tr_info("nanostackNetworkHandler: CON_STATUS_GLOBAL_UP, IP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                temp_ipv6_global[0], temp_ipv6_global[1],
                temp_ipv6_global[2], temp_ipv6_global[3],
                temp_ipv6_global[4], temp_ipv6_global[5],
                temp_ipv6_global[6], temp_ipv6_global[7],
                temp_ipv6_global[8], temp_ipv6_global[9],
                temp_ipv6_global[10], temp_ipv6_global[11],
                temp_ipv6_global[12], temp_ipv6_global[13],
                temp_ipv6_global[14], temp_ipv6_global[15]);
            _connect_status = CON_STATUS_GLOBAL_UP;
        }
    } else if (status == MESH_CONNECTED_LOCAL) {
        tr_info("nanostackNetworkHandler: CON_STATUS_LOCAL_UP");
        _connect_status = CON_STATUS_LOCAL_UP;
    } else if (status == MESH_CONNECTED_GLOBAL) {
        tr_info("nanostackNetworkHandler: CON_STATUS_GLOBAL_UP");
        _connect_status = CON_STATUS_GLOBAL_UP;
    } else if (status == MESH_BOOTSTRAP_STARTED || status == MESH_BOOTSTRAP_FAILED) {
        tr_info("nanostackNetworkHandler: CON_STATUS_CONNECTING");
        _connect_status = CON_STATUS_CONNECTING;
    } else {
        _connect_status = CON_STATUS_DISCONNECTED;
        tr_info("nanostackNetworkHandler: CON_STATUS_DISCONNECTED");
    }
}

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
//            tr_mpl("socket_callback: SOCKET_DATA, sock=%d, bytes=%d", sock_cb->socket_id, sock_cb->d_len);

            /* Convert string addr to ipaddr array */
            len = socket_recvfrom(socket_id, recv_buffer, sizeof(recv_buffer), 0, &source_addr);
            if(len > 0)
              {
                  num_pkts++;
#ifdef WISUN_TEST_MPL_UDP
                  uint16_t slotIdx_tx, slotIdx_rx;
                  uint32_t bfio_tx, bfio_rx, latency, macBcInterval;
                  get_txFrameInfo ((char*)recv_buffer, &slotIdx_tx, &bfio_tx);
                  timac_GetBC_Slot_BFIO(&slotIdx_rx, &bfio_rx);
                  MAP_FHPIB_get(FHPIB_BC_INTERVAL, &macBcInterval);
                  latency = (bfio_rx - bfio_tx) + (slotIdx_rx - slotIdx_tx) * macBcInterval;
                  tr_mpl("Latency: %d L:%d B1:%d S1:%d B2:%d S2:%d BI:%d\r\n", latency, len, bfio_tx, slotIdx_tx, bfio_rx, slotIdx_rx, macBcInterval);
#else
                  tr_mpl("Recv[%d]: %s, Pkts:%d", len, recv_buffer, num_pkts);
#endif
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
    static const int16_t multicast_hops = 10;
    socket_setsockopt(socket_id, SOCKET_IPPROTO_IPV6, SOCKET_IPV6_MULTICAST_HOPS, &multicast_hops, sizeof(multicast_hops));

    static const int32_t buf_size = 20;
    int32_t rtn = 20;
//    rtn = socket_setsockopt(socket_id, SOCKET_SOL_SOCKET, SOCKET_SO_RCVBUF, &buf_size, sizeof buf_size);
    tr_info("set rx buffer len %x, status %x", buf_size, rtn);
/*
    rtn = socket_setsockopt(socket_id, SOCKET_SOL_SOCKET, SOCKET_SO_SNDBUF, &buf_size, sizeof buf_size);
    tr_info("set Tx buffer len %x, status %x", buf_size, rtn);
*/

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
 * Configure the network settings like network name,
 * regulatory domain etc. before starting the network
 */
extern const char *ti154stack_lib_version;
extern const char *ti154stack_lib_date;
extern const char *ti154stack_lib_time;
mesh_error_t nanostack_wisunInterface_configure(void)
{
    int ret;

    tr_info("Library info | Date: %s, Time: %s, Version: %s", ti154stack_lib_date, ti154stack_lib_time,
            ti154stack_lib_version);

    if (_configured) {
        // Already configured
        return MESH_ERROR_NONE;
    }
#ifndef TI_WISUN_FAN_OPT
    ret = ws_management_network_name_set(interface_id, cfg_props.network_name);
    if (ret < 0) {
        return MESH_ERROR_PARAM;
    }


    ret = ws_management_regulatory_domain_set(interface_id, CONFIG_REG_DOMAIN,
                                                            CONFIG_OP_MODE_CLASS,
                                                            CONFIG_OP_MODE_ID);
    if (ret < 0) {
        return MESH_ERROR_PARAM;
    }


    ret = ws_management_fhss_unicast_channel_function_configure(interface_id, cfg_props.uc_channel_function,
                                                                              cfg_props.uc_fixed_channel,
                                                                              cfg_props.uc_dwell_interval);
    if (ret < 0) {
        return MESH_ERROR_PARAM;
    }


    ret = ws_management_fhss_broadcast_channel_function_configure(interface_id, cfg_props.bc_channel_function,
                                                                                cfg_props.bc_fixed_channel,
                                                                                cfg_props.bc_dwell_interval,
                                                                                cfg_props.bc_interval);
    if (ret < 0) {
        return MESH_ERROR_PARAM;
    }

    ret = ws_management_network_size_set(interface_id, MBED_CONF_MBED_MESH_APP_WISUN_NETWORK_SIZE);
    if (ret < 0) {
        return MESH_ERROR_PARAM;
    }
#else
    ws_cfg_set_intferface_all();
#endif //TI_WISUN_FAN_OPT

    _configured = true;
    return MESH_ERROR_NONE;
}

/*!
 *
 */
mesh_error_t nanostack_wisunInterface_bringup()
{
    int8_t device_id = 0;

#ifndef FEATURE_TIMAC_SUPPORT
    NanostackTiRfPhy_init();
    device_id = NanostackTiRfPhy_rf_register();
#else
    timacExtaddressRegister();
#endif
    // After the RF is up, we can seed the random from it.
    randLIB_seed_random();

    wisun_tasklet_init();

    interface_id =  wisun_tasklet_network_init(device_id);
    if (interface_id < 0) {
        return MESH_ERROR_UNKNOWN;
    }

    return MESH_ERROR_NONE;
}

/*!
 * Connect to the Wi-SUN network. Should be called only after calling
 * nanostack_wisunInterface_bringup(). This function also submits
 * the network handler function for handling different events
 * during the connection process, while calling "connect".
 * Only blocking mode has been tested. So, it is recommended for
 * the input parameter blocking to be set to true.
 */
mesh_error_t nanostack_wisunInterface_connect(bool blocking)
{

    int8_t tasklet_id;

    _blocking = blocking;

    tasklet_id = wisun_tasklet_connect(nanostackNetworkHandler, interface_id);

    if (tasklet_id < 0) {
        return MESH_ERROR_UNKNOWN;
    }

   return MESH_ERROR_NONE;
}

/*!
 * Connect to the Wi-SUN network. Should be called only after calling
 * nanostack_wisunInterface_bringup().
 * It is recommended to start the node join process in blocking mode
 */
void nanostack_wait_till_connect()
{
    uint8_t _net_state;
    if (_blocking)
    {
#ifdef NWK_TEST
        ticks_before_joining = ClockP_getSystemTicks();
#endif //NWK_TEST

        // wait till connection goes through
        while(connectedFlg == false)
            {
                /* Toggle red LED at rate of state*100 ms. Slower the blinking, closer it is to joining */
                _net_state = get_current_net_state();
                usleep((_net_state + 1) * 100000);
                // max usleep value possible is 1000000
                GPIO_toggle(CONFIG_GPIO_RLED);
            }
        /* Solid Green to Indicate that node has Joined */
        GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);
        GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_ON);

#ifdef NWK_TEST
        ticks_after_joining = ClockP_getSystemTicks();
#endif //NWK_TEST
    }
}

#if !defined(WISUN_NCP_ENABLE) && !defined(NWK_TEST)
/*!
 * Interrupt handler for handling button presses on the
 * Launch pad.
 * Press button 1 to start sending of a broadcast packets.
 * To disable sending of broadcast packets, press button 2
 * again.
 * Press button 2 to trigger sending of a unicast packet.
 * To trigger sending of the next unicast packet, press
 * button 2 again.
 * These button presses should be exercised only after the
 * node joins the network.
 */
static void btn_interrupt_handler(uint8_t index)
{
    if(index == CONFIG_GPIO_BTN1)
    {
        if(bcast_send == true)
        {
            bcast_send = false;
        }
        else
        {
            bcast_send = true;
        }
    } 
    else if(index == CONFIG_GPIO_BTN2)
    {
        if(unicast_send == true)
        {
            unicast_send = false;
        }
        else
        {
            unicast_send = true;
        }

    }
}
#endif //WISUN_NCP_ENABLE, NWK_TEST


/*!
 * Process received message
 */
static void handle_message(char* msg) {
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

#ifdef WISUN_TEST_MPL_UDP
/*!
 * Retrieve Idx and BFIO from received message
 */
void get_txFrameInfo(char* msg, uint16_t* txIdx, uint32_t* txbfio) {
    char *txinfo;

    // Format from Border Router for the payload is as follows
    // snprintf(send_buf, sizeof(send_buf), "Id:%d:bfio:%u", slotIdx,bfio);
    txinfo = strstr(msg, "Id:");
    if (txinfo == NULL)
    {
        // Idx and BFIO for tx is not in the message payload
        *txIdx = 0;
        *txbfio = 0;
    }
    else
    {
        char *ptr;
        // Retrieve slot ID index
        *txIdx = strtol(txinfo+3, &ptr, 10);

        // Retrieve BFIO
        txinfo = strstr(ptr, "bfio:");
        *txbfio = strtol(txinfo+5, &ptr, 10);
    }
    return;
}
#endif

#ifdef COAP_SERVICE_ENABLE
#ifdef COAP_PANID_LIST
static void pan_rediscover_tasklet_start(void)
{
    pan_rediscover_tasklet_id = eventOS_event_handler_create(
        &pan_rediscover_tasklet,
        PAN_REDISCOVER_INIT_EVT);
}

static void pan_rediscover_update(uint8_t event_type)
{
    arm_event_s event = {
           .sender = 0,
           .receiver = pan_rediscover_tasklet_id,
           .priority = ARM_LIB_LOW_PRIORITY_EVENT,
           .event_type = event_type,
           .event_id = 0,
           .event_data = 0
    };
    eventOS_event_send(&event);
}

extern void ccfg_read_mac_addr(uint8_t *mac_addr);
static void pan_rediscover_tasklet(arm_event_s *event)
{
    pan_rediscover_evt_t event_type;
    protocol_interface_info_entry_t *cur;
    uint8_t net_state = 0;
    uint16_t blink_rate;
    int ret;
    uint8_t hwAddr[8];

    event_type = (pan_rediscover_evt_t) event->event_type;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    switch (event_type) {
        // Init event called after tasklet creation
        case PAN_REDISCOVER_INIT_EVT:
            break;
        case PAN_REDISCOVER_START_EVT:
            GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);
            GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_OFF);
            pan_rediscover_update(PAN_REDISCOVER_WAIT_EVT);
            break;
        case PAN_REDISCOVER_WAIT_EVT:
            net_state = get_current_net_state();
            if(net_state != 5)
            {
                // Toggle red LED at rate of state*100 ms. Slower the blinking, closer it is to joining
                blink_rate = (net_state + 1) * 100;
                GPIO_toggle(CONFIG_GPIO_RLED);

                // Timer to recheck net_state (toggles LED just as in initial join)
                eventOS_event_timer_request(PAN_REDISCOVER_BLINK_TIMER_ID, PAN_REDISCOVER_WAIT_EVT,
                                            pan_rediscover_tasklet_id, blink_rate);
            }
            else
            {
                // Solid Green to Indicate that node has joined
                GPIO_write(CONFIG_GPIO_RLED, CONFIG_GPIO_LED_OFF);
                GPIO_write(CONFIG_GPIO_GLED, CONFIG_GPIO_LED_ON);
                // Cancel the timeout timer as we have successfully joined
                pan_rediscover_update(PAN_REDISCOVER_JOIN_EVT);
            }
            break;
        case PAN_REDISCOVER_JOIN_EVT:
            // Get current HW address and add to payload
            ccfg_read_mac_addr(hwAddr);
            // CoAP message to indicate to BR that new device has joined
            coap_service_request_send(service_id, 0, cur->border_router_setup->border_router_gp_adr,
                                      COAP_PORT, COAP_MSG_TYPE_NON_CONFIRMABLE, COAP_MSG_CODE_REQUEST_POST,
                                      COAP_JOIN_URI, COAP_CT_TEXT_PLAIN, hwAddr, 8, NULL);
            // Timer to initiate panid list clear and restart joining process (in case join is lost)
            // Cancelled if bulk panid list set is sent to COAP_PANID_LIST_BULK_URI
            eventOS_event_timer_request(PAN_REDISCOVER_JOIN_RESP_TIMER_ID, PAN_REDISCOVER_RESTART_EVT,
                                        pan_rediscover_tasklet_id, 1000*PAN_REDISCOVER_JOIN_REQ_TIMEOUT_SEC);
            break;
        case PAN_REDISCOVER_RESTART_EVT:
            // Cancel lingering wait event if it is still in the event queue
            eventOS_event_timer_cancel(PAN_REDISCOVER_BLINK_TIMER_ID, pan_rediscover_tasklet_id);
            api_panid_filter_list_remove(PANID_ALLOW_LIST_E, 0, true);
            api_panid_filter_list_remove(PANID_DENY_LIST_E, 0, true);
            nanostack_net_stack_restart(true);
            pan_rediscover_update(PAN_REDISCOVER_START_EVT);
            break;
        default:
            break;
    } // switch(event_type)
}
#endif // COAP_PANID_LIST


/*!
 * Callback for processing received coap message
 */
static int coap_recv_cb(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr)
{

    if (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_GET)
    {
        // Send LED states as [RLED_STATE, GLED_STATE] in CoAP response payload
        led_state[COAP_RLED_ID] = GPIO_read(CONFIG_GPIO_RLED);
        led_state[COAP_GLED_ID] = GPIO_read(CONFIG_GPIO_GLED);
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CONTENT,
                                   COAP_CT_TEXT_PLAIN, (uint8_t *) &led_state, sizeof(led_state));
    }
    else if (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_POST ||
             request_ptr->msg_code == COAP_MSG_CODE_REQUEST_PUT)
    {
        bool success = true;
        if (request_ptr->payload_len != 2 || request_ptr->payload_ptr == NULL)
        {
            // Invalid payload length
            success = false;
        }
        else if ((request_ptr->payload_ptr[0] != 0 && request_ptr->payload_ptr[0] != 1) ||
                 (request_ptr->payload_ptr[1] != 0 && request_ptr->payload_ptr[1] != 1))
        {
            // Invalid payload contents
            success = false;
        }
        else
        {
            // Set LED state from payload: [target LED, LED state]
            if (request_ptr->payload_ptr[0] == COAP_RLED_ID)
            {
                GPIO_write(CONFIG_GPIO_RLED, request_ptr->payload_ptr[1]);
            }
            else if (request_ptr->payload_ptr[0] == COAP_GLED_ID)
            {
                GPIO_write(CONFIG_GPIO_GLED, request_ptr->payload_ptr[1]);
            }
            else
            {
                success = false;
            }
        }

        if (success)
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CHANGED,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
        else
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_BAD_REQUEST,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
    }
    else
    {
        // Delete resource not supported
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED,
                                   COAP_CT_TEXT_PLAIN, NULL, 0);
    }
    return 0;
}

#ifdef WISUN_TEST_METRICS
/*!
 * Callback for processing received coap message for Test metrics
 */
static int coap_recv_cb_tstmetrics(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr)
{
    if (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_GET)
    {
        test_metrics_s test_metrics;
        // Send test metrics data
        get_test_metrics(&test_metrics);
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CONTENT,
                                   COAP_CT_TEXT_PLAIN, (uint8_t *) &test_metrics, sizeof(test_metrics_s));
    }
    else
    {
        // Post, Put, and Delete resource are not supported for now
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED,
                                   COAP_CT_TEXT_PLAIN, NULL, 0);
    }
    return 0;
}
#endif

#ifdef COAP_PANID_LIST
static int coap_panid_list_cb(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr)
{
    uint16_t i, panid_list_len, payload_index;
    panid_list_type_e list_type;
    sn_coap_msg_code_e resp_code;
    uint16_t *panid_list;
    uint16_t *payload;
    uint16_t panid;
    int ret;

    // Determine whether to use allowlist, denylist, do bulk update, or do rediscovery
    if (memcmp(request_ptr->uri_path_ptr, COAP_PANID_LIST_ALLOW_URI, request_ptr->uri_path_len) == 0)
    {
        list_type = PANID_ALLOW_LIST_E;
        panid_list = panid_allow_list;
        panid_list_len = MAX_PANID_ALLOW_LIST_LEN;
    }
    else if (memcmp(request_ptr->uri_path_ptr, COAP_PANID_LIST_DENY_URI, request_ptr->uri_path_len) == 0)
    {
        list_type = PANID_DENY_LIST_E;
        panid_list = panid_deny_list;
        panid_list_len = MAX_PANID_DENY_LIST_LEN;
    }
    else if (memcmp(request_ptr->uri_path_ptr, COAP_PANID_REDISCOVER_URI, request_ptr->uri_path_len) == 0)
    {
        // Start PAN rediscover process
        ret = nanostack_net_stack_restart(false);
        if (ret == PANID_STACK_RESTART_SUCCESS)
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CHANGED,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
            pan_rediscover_update(PAN_REDISCOVER_START_EVT);

        }
        else if (ret == PANID_STACK_RESTART_NO_RESTART)
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_VALID,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
        else
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
        return 0;
    }
    else if (memcmp(request_ptr->uri_path_ptr, COAP_PANID_LIST_BULK_URI, request_ptr->uri_path_len) == 0)
    {
        uint8_t err = 0;
        uint16_t allowlist_len, denylist_len;

        // Cancel timer set by PAN_REDISCOVER_JOIN_EVT
        eventOS_event_timer_cancel(PAN_REDISCOVER_JOIN_RESP_TIMER_ID, pan_rediscover_tasklet_id);

        /*
         * Format of CoAP PANID bulk update:
         * <2 byte allow/deny list timeout (seconds) +
         * <2 byte allowlist len> + <2 byte denylist len> +
         * <2 byte allowlist entry>*(allowlist len) + <2 byte denylist entry>*(denylist len)
         */
        // Length checking
        if (request_ptr->payload_len < 6) // Min length considering 0 entries in both lists
        {
            err = 1;
        }
        else
        {
            uint16_t total_len = 6;
            allowlist_len = *((uint16_t *) (request_ptr->payload_ptr + 2));
            denylist_len = *((uint16_t *) (request_ptr->payload_ptr + 4));
            total_len += ((allowlist_len + denylist_len)*2);
            if (total_len != request_ptr->payload_len)
            {
                err = 1;
            }
        }
        if (err == 1)
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_BAD_REQUEST,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
            return 0;
        }

        // Populate timeout value (sec) if nonzero
        uint16_t payload_timeout = *(uint16_t *)(request_ptr->payload_ptr);
        if (payload_timeout != 0)
        {
            panid_list_clear_timeout_sec = payload_timeout;
        }

        // Populate allowlist/denylist
        uint16_t panid_list_start;
        panid_list_type_e remove_list_type;
        // Start counting payload after timeout/size info, and cast to 16 bit array
        uint16_t *panid_list_payload = (uint16_t *)(request_ptr->payload_ptr + 6);
        for (list_type = PANID_ALLOW_LIST_E; list_type <= PANID_DENY_LIST_E; list_type++)
        {
            if (list_type == PANID_ALLOW_LIST_E)
            {
                remove_list_type = PANID_DENY_LIST_E;
                panid_list_start = 0;
                panid_list_len = allowlist_len;
            }
            else // PANID_DENY_LIST_E
            {
                remove_list_type = PANID_ALLOW_LIST_E;
                panid_list_start = allowlist_len;
                panid_list_len = denylist_len;
            }

            for (i = 0; i < panid_list_len; i++)
            {
                panid = *(panid_list_payload + panid_list_start + i);
                // Add to list
                ret = api_panid_filter_list_add(list_type, panid);
                if (ret != PANID_FLTR_UPDATE_SUCCESS)
                {
                    coap_service_response_send(service_id, 0, request_ptr,
                                               COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR,
                                               COAP_CT_TEXT_PLAIN, NULL, 0);
                    return 0;
                }
                // Remove from opposite list
                ret = api_panid_filter_list_remove(remove_list_type, panid, false);
                if (ret != PANID_FLTR_UPDATE_SUCCESS && ret != PANID_FLTR_UPDATE_NO_MATCH)
                {
                    coap_service_response_send(service_id, 0, request_ptr,
                                               COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR,
                                               COAP_CT_TEXT_PLAIN, NULL, 0);
                    return 0;
                }
            }
        }

        // Start PAN redicover process
        ret = nanostack_net_stack_restart(false);
        if (ret == PANID_STACK_RESTART_SUCCESS)
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CHANGED,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
            pan_rediscover_update(PAN_REDISCOVER_START_EVT);

        }
        else if (ret == PANID_STACK_RESTART_NO_RESTART)
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_VALID,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
        else
        {
            coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
        return 0;
    }
    else
    {
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_BAD_REQUEST,
                                   COAP_CT_TEXT_PLAIN, NULL, 0);
        return 0;
    }

    // Handle GET request
    if (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_GET)
    {
        payload_index = 1; // Reserve index 0 for payload length
        payload = (uint16_t *) malloc(panid_list_len * sizeof(uint16_t));
        memset((uint8_t *)payload, 0, sizeof(panid_list_len * sizeof(uint16_t)));

        // Populate payload with all used PAN IDs
        for(i = 0; i < panid_list_len; i++)
        {
            if (panid_list[i] != PANID_UNUSED)
            {
                payload[payload_index] = panid_list[i];
                payload_index++;
            }
        }
        payload[0] = payload_index - 1; // First element of payload is size of panid list

        // Send used PAN IDs
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CONTENT,
                                   COAP_CT_TEXT_PLAIN, (uint8_t *)payload, sizeof(uint16_t)*(payload_index));
    }
    // Handle POST/PUT request
    else if (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_POST ||
             request_ptr->msg_code == COAP_MSG_CODE_REQUEST_PUT)
    {
        ret = -1;
        if (request_ptr->payload_len == 3)
        {
            panid = *((uint16_t *) (request_ptr->payload_ptr + 1));
            if (request_ptr->payload_ptr[0] == PANID_LIST_ACTION_ADD)
            {
                ret = api_panid_filter_list_add(list_type, panid);
                resp_code = COAP_MSG_CODE_RESPONSE_CREATED;
            }
            else if (request_ptr->payload_ptr[0] == PANID_LIST_ACTION_DEL)
            {
                ret = api_panid_filter_list_remove(list_type, panid, false);
                resp_code = COAP_MSG_CODE_RESPONSE_DELETED;
            }
        }

        if (ret == PANID_FLTR_UPDATE_SUCCESS)
        {
            coap_service_response_send(service_id, 0, request_ptr, resp_code,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
        else
        {
            coap_service_response_send(service_id, 0, request_ptr,
                                       COAP_MSG_CODE_RESPONSE_INTERNAL_SERVER_ERROR,
                                       COAP_CT_TEXT_PLAIN, NULL, 0);
        }
    }
    else
    {
        // Delete resource not supported
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED,
                                   COAP_CT_TEXT_PLAIN, NULL, 0);
    }
    return 0;
}
#endif // COAP_PANID_LIST

void fetch_neighbor_details();

static int coap_recv_cb_rssi(int8_t service_id, uint8_t source_address[static 16],
                 uint16_t source_port, sn_coap_hdr_s *request_ptr)
{

    if (request_ptr->msg_code == COAP_MSG_CODE_REQUEST_GET)
    {
        fetch_neighbor_details();

        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_CONTENT,
                                   COAP_CT_TEXT_PLAIN, (uint8_t *) nbr_nodes_metrics, sizeof(nbr_nodes_metrics));
    }
    else
    {
        // Delete resource not supported
        coap_service_response_send(service_id, 0, request_ptr, COAP_MSG_CODE_RESPONSE_METHOD_NOT_ALLOWED,
                                   COAP_CT_TEXT_PLAIN, NULL, 0);
    }
    return 0;
}

#endif // COAP_SERVICE_ENABLE

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
    /* default user LED's */
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

    extern const char *ti154stack_lib_version;
    extern const char *ti154stack_lib_date;
    extern const char *ti154stack_lib_time;
    tr_info("Library info | Date: %s, Time: %s, Version: %s", ti154stack_lib_date, ti154stack_lib_time,
            ti154stack_lib_version);

    nanostack_wisunInterface_configure();

    // Release mutex before blocking
    nanostack_lock();

    if( MESH_ERROR_NONE != nanostack_wisunInterface_bringup())
    {
        // release mutex
        nanostack_unlock();
        // do not proceed further
        while(1);
    }

    if(MESH_ERROR_NONE != nanostack_wisunInterface_connect(true))
    {
        // release mutex
        nanostack_unlock();
        // do not proceed further
        while(1);
    }

    // if here all good so far: proceed further
    // Release mutex before blocking
    nanostack_unlock();

#ifdef NWK_TEST

    /* this should only be initialized once */
    nwkTest_init();

    while (1)
    {
        nwkTest_run();
        usleep(100000);
    }
#elif defined(COAP_SERVICE_ENABLE)
    service_id = coap_service_initialize(interface_id, COAP_PORT, 0, NULL, NULL);
    coap_service_register_uri(service_id, COAP_LED_URI,
                              COAP_SERVICE_ACCESS_GET_ALLOWED |
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_recv_cb);
    coap_service_register_uri(service_id, COAP_RSSI_URI,
                                  COAP_SERVICE_ACCESS_GET_ALLOWED,
                                  coap_recv_cb_rssi);
#ifdef WISUN_TEST_METRICS
    coap_service_register_uri(service_id, COAP_TEST_METRICS_URI,
                              COAP_SERVICE_ACCESS_GET_ALLOWED,
                              coap_recv_cb_tstmetrics);
#endif

#ifdef COAP_PANID_LIST
    coap_service_register_uri(service_id, COAP_PANID_LIST_ALLOW_URI,
                              COAP_SERVICE_ACCESS_GET_ALLOWED |
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_panid_list_cb);
    coap_service_register_uri(service_id, COAP_PANID_LIST_DENY_URI,
                              COAP_SERVICE_ACCESS_GET_ALLOWED |
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_panid_list_cb);
    coap_service_register_uri(service_id, COAP_PANID_REDISCOVER_URI,
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_panid_list_cb);
    coap_service_register_uri(service_id, COAP_PANID_LIST_BULK_URI,
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_panid_list_cb);

    pan_rediscover_tasklet_start();
#endif // COAP_PANID_LIST

#if defined(COAP_OAD_ENABLE)
    // Initialize oad tasklet
    oad_tasklet_start();

    /* Initialize CoAP OAD services */
    coap_service_register_uri(service_id, OAD_FWV_REQ_URI,
                              COAP_SERVICE_ACCESS_GET_ALLOWED |
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_oad_cb);
    coap_service_register_uri(service_id, OAD_NOTIF_URI,
                              COAP_SERVICE_ACCESS_GET_ALLOWED |
                              COAP_SERVICE_ACCESS_PUT_ALLOWED |
                              COAP_SERVICE_ACCESS_POST_ALLOWED,
                              coap_oad_cb);
#endif // COAP_OAD_ENABLE

#else
    /* Convert string addr to ipaddr array */
    stoip6(multicast_addr_str, strlen(multicast_addr_str), multi_cast_addr);

    if(udpSocketSetup() == false)
    {
        tr_debug("Socket setup failed");
    }

    /* Set multicast send address */
    send_addr.type = ADDRESS_IPV6;
    send_addr.identifier = UDP_PORT;
    memcpy(send_addr.address, multi_cast_addr, 16);

    /* Set unicast send address */
    send_addr_unicast.type = ADDRESS_IPV6;
    send_addr_unicast.identifier = UDP_PORT_ECHO;

    while (1) {
        int16_t len;
        static uint32_t bcast_delay_count = 0;
        static uint32_t unicast_delay_count = 0;
        static bool prev_bcast_send = false;
        static bool prev_unicast_send = false;
        static ns_address_t source_addr = {0};

        if(prev_bcast_send != bcast_send)
        {
            if(bcast_send)
            {
                tr_info("bcast_send enabled, sending light toggle every 10s");
                bcast_delay_count = 100;
            }
            else
            {
                tr_info("bcast_send disabled");
            }
        }
        prev_bcast_send = bcast_send;

        if(bcast_send)
        {
            if(bcast_delay_count==100)
            {
                static bool light_state = false;

                /* toggle light state */
                light_state ^= 1;
                /**
                * Multicast control message is a NUL terminated string of semicolon separated
                * <field identifier>:<value> pairs.
                *
                * Light control message format:
                * t:lights;g:<group_id>;s:<1|0>;\0
                */
                snprintf(send_buf, sizeof(send_buf), "t:lights;g:%03d;s:%s;", MY_GROUP, (light_state ? "1" : "0"));
                tr_debug("Sending lightcontrol message: %s", send_buf);
                /* send every 20s */
                tr_info("Sending multicast packet");
                ret = socket_sendto(socket_id, &send_addr, send_buf, sizeof(send_buf));
                tr_info("Sendto returned: %d", ret);

                GPIO_toggle(CONFIG_GPIO_GLED);

                bcast_delay_count = 0;
            }
            bcast_delay_count++;
        }
        if(unicast_send)
        {
            if(sent_dao == true) //&& !addr_ipv6_equal((const uint8_t*)root_unicast_addr, ns_in6addr_any))
             {
                memcpy(send_addr_unicast.address, root_unicast_addr, 16);
                tr_info("Sending unicast packet from Router to Border Router");
                ret = socket_sendto(socket_id, &send_addr_unicast, send_buf_unicast, sizeof(send_buf_unicast));
                tr_info("Sendto returned: %d", ret);
                unicast_send = false;
             }
        }

        len = socket_recvfrom(socket_id, recv_buffer, sizeof(recv_buffer), 0, &source_addr);
        if(len > 0)
        {
            tr_info("Recv[%d]: %s, ", len, recv_buffer);
            handle_message((char*)recv_buffer);
        }
        else if(NS_EWOULDBLOCK != len)
        {
            tr_info("Recv error %x", len);
        }

        usleep(100000);
    }
#endif /* endif for NWK_TEST not defined */
    nanostack_wait_till_connect();
#if defined(COAP_SERVICE_ENABLE) && defined(COAP_PANID_LIST)
    pan_rediscover_update(PAN_REDISCOVER_JOIN_EVT);
#endif
    return NULL;
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
 * Callback from the UART module indicating need for processing.
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

     } //end of switch case
}

/*!
 * Create the NCP tasklet whose core logic is defined in ncp_tasklet()
 * Also, post the event to initialize it after creating the tasklet
 */
void ncp_tasklet_start(void)
{
        eventOS_event_handler_create(
        &ncp_tasklet,
        ARM_LIB_TASKLET_INIT_EVENT);
}


/* Below functions are invoked on receiving
 * the appropriate commands from host */

/*!
 * Check if bringing up of network interface is done or not
 * return value: true if already done, false if not
 */
bool is_net_if_up(void)
{
    if(interface_id < 0)
    {
        return (false);
    }
    else
    {
        return (true);
    }
}

/*!
 * Bring up the network interface
 */
otError nanostack_net_if_up(void)
{
    if( MESH_ERROR_NONE != nanostack_wisunInterface_configure())
    {
        return (OT_ERROR_FAILED);
    }
    if (MESH_ERROR_NONE != nanostack_wisunInterface_bringup())
    {
        return (OT_ERROR_FAILED);
    }

    //everything is ok; return error_none
    return(OT_ERROR_NONE);

}

/*!
 * Check if the node joined the network or not on starting
 * the connection to network
 * return value: true if connected, false if not or in the
 * process of connecting
 */
bool is_net_stack_up(void)
{
    return(connectedFlg);
}

/*!
 * Start the process to connect node to the network
 */
otError nanostack_net_stack_up(void)
{
    GPIO_write(CONFIG_GPIO_RLED, 0);

    if(MESH_ERROR_NONE != nanostack_wisunInterface_connect(true))
    {
        return (OT_ERROR_FAILED);
    }
    //everything is ok; return error_none
    return(OT_ERROR_NONE);

    // it is expected that host will keep monitoring
    // if connection succeeded or not by giving
    // command to check if is_net_stack_up()
    // or reading current_net_state()
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

/*!
 * Helper function - returns the Link Local address of the node
 */
if_address_entry_t *get_linkLocal_address(void)
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    ns_list_foreach(if_address_entry_t, entry, &cur->ip_addresses )
    {
       if (entry->source == ADDR_SOURCE_UNKNOWN)
       {
           return entry;
       }
    }

    /* if we are here could not find the address */
    return NULL;

}

/*!
 * Helper function - returns the global unicast address of the node
 */
if_address_entry_t *get_globalUnicast_address(void)
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    ns_list_foreach(if_address_entry_t, entry, &cur->ip_addresses )
    {
       if (entry->source == ADDR_SOURCE_DHCP)
       {
           return entry;
       }
    }

    /* if we are here could not find the address */
    return NULL;
}

#endif //WISUN_NCP_ENABLE

/*!
 * Helper function to get neighbor node metrics like rssi_in, rssi_out
 * Metrics are copied over to a global structure instance.
 */
void fetch_neighbor_details()
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);
    if(!cur || !cur->mac_parameters || !cur->mac_parameters->mac_neighbor_table)
    {
        tr_debug("fetch_neighbor_details: NULL pointer");
        return;
    }

    uint8_t max_nbrs, nbr_idx = 0;

    max_nbrs = cur->mac_parameters->mac_neighbor_table->list_total_size;
    cur_num_nbrs = (cur->mac_parameters->mac_neighbor_table->neighbour_list_size) - 1;

    for(uint8_t i = 0; i < max_nbrs; i++)
    {
        if(cur->mac_parameters->mac_neighbor_table->neighbor_entry_buffer[i].trusted_device == 1)
        {
            //found a valid neighbor
            //copy mac address
            memcpy(nbr_nodes_metrics[nbr_idx].mac_eui, cur->mac_parameters->mac_neighbor_table->neighbor_entry_buffer[i].mac64, sizeof(sAddrExt_t));

            //fetch and copy rssi_in rssi_out (Shift back the WS_RSL_SCALING amount)
            nbr_nodes_metrics[nbr_idx].rssi_in = cur->ws_info->neighbor_storage.neigh_info_list[i].rsl_in >> WS_RSL_SCALING;
            nbr_nodes_metrics[nbr_idx].rssi_out = cur->ws_info->neighbor_storage.neigh_info_list[i].rsl_out >> WS_RSL_SCALING;

            nbr_idx++;

            if(nbr_idx == cur_num_nbrs)
            {
                // found all entries
                break;
            }//end of inner if

        } //end of outer if

    }//end of for
}

/*!
 * Returns the current state of the node based on
 * at what state is the node during the network joining process
 * Return value used to vary the rate of red led blinking
 * while the node is joining the network
 */
uint8_t get_current_net_state(void)
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    uint8_t curNetState = 0;

    switch(cur->nwk_bootstrap_state)
    {
        case ER_IDLE:
            curNetState = 0;
            break;
        case ER_ACTIVE_SCAN:
            curNetState = 1;
            break;
        case ER_PANA_AUTH:
            curNetState = 2;
            break;
        case ER_SCAN:
            curNetState = 3;
            break;
        case ER_RPL_SCAN:
            curNetState = 4;
            break;
        case ER_BOOTSRAP_DONE:
            curNetState = 5;
            break;
        case ER_WAIT_RESTART:
            curNetState = 6;
            break;
        default:
            break;
    }

    return(curNetState);
}

/*!
 * Helper function - get current multicast groups a node is subscribed to
 */
if_group_list_t *get_multicast_ip_groups()
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    return &cur->ip_groups;
}

/*!
 * Helper function - add multicast address
 * return 0 if successful or -1 when an error occurs
 */
int8_t add_multicast_addr(const uint8_t *address)
{
    protocol_interface_info_entry_t *cur;

    cur = protocol_stack_interface_info_get(IF_6LoWPAN);
    if (!addr_is_ipv6_multicast(address) ||
        addr_ipv6_multicast_scope(address) <= IPV6_SCOPE_REALM_LOCAL)
    {
        return -1; // Non-multicast or realm/link/interface local scope address
    }

    if (addr_add_group(cur, address) == NULL)
    {
        return -1; // Address group add failed
    }
    return 0;
}

/*!
 * Helper function - remove multicast address
 * return 0 if successful or -1 when an error occurs
 */
int8_t remove_multicast_addr(const uint8_t *address)
{
    protocol_interface_info_entry_t *cur;

    cur = protocol_stack_interface_info_get(IF_6LoWPAN);
    if (!addr_is_ipv6_multicast(address) ||
        addr_ipv6_multicast_scope(address) <= IPV6_SCOPE_REALM_LOCAL)
    {
        return -1; // Non-multicast or realm/link/interface local scope address
    }

    addr_remove_group(cur, address);
    return 0;
}

rpl_instance_t *get_rpl_instance()
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);
    if (!cur || !cur->rpl_domain) {
        return NULL;
    }

    rpl_instance_t *instance = ns_list_get_first(&cur->rpl_domain->instances);

    return instance;
}

rpl_dao_target_t *get_dao_target_from_addr(rpl_instance_t *instance, const uint8_t *addr)
{
    // Always return NULL, only valid for BR devices
    return NULL;
}

uint16_t get_network_panid(void)
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);
    if (!cur || !cur->ws_info) {
        return 0xFFFF;
    }
    return cur->ws_info->network_pan_id;
}


#ifdef WISUN_TEST_METRICS
extern JOIN_TIME_s node_join_time;

/*
 * Get latest test metrics
 */
void get_test_metrics(test_metrics_s *test_metrics)
{
    rpl_instance_t *rpl_inst;

    test_metrics->revision = 4;
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
    test_metrics->udpPktCnt = num_pkts;
    // Reset UDP packet count to 0 for next test
    num_pkts = 0;

    // Get current rank
    rpl_inst = get_rpl_instance();
    test_metrics->current_rank = rpl_inst->current_rank;

    // Populate length
    test_metrics->length = (uint16_t) sizeof(test_metrics_s);
}
#endif
