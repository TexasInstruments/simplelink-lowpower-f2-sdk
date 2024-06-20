/*
 * Copyright (c) 2019 ARM Limited. All rights reserved.
 */
#include <string.h>
#include <stdlib.h>
#include <mbed_assert.h>

#include "eventOS_event.h"
#include "eventOS_event_timer.h"
#include "eventOS_scheduler.h"
#include "platform/arm_hal_timer.h"
#include "borderrouter_tasklet.h"
#include "borderrouter_helpers.h"
#include "net_interface.h"
#include "fhss_api.h"
#include "fhss_config.h"
#include "ws_management_api.h"
#include "ws_bbr_api.h"
#include "ip6string.h"
#include "mac_api.h"
#include "ethernet_mac_api.h"
#include "sw_mac.h"
#include "nwk_stats_api.h"
#include "randLIB.h"
#include "NanostackTiRfPhy.h"
#include "borderrouter_tasklet.h"
#include "mesh_system.h"
#include "mbed-mesh-api/mesh_interface_types.h"
#include "NWK_INTERFACE/Include/protocol_abstract.h"
#include "nsconfig.h"
#include "NWK_INTERFACE/Include/protocol.h"
#include "Common_Protocols/ipv6_constants.h"
#include "6LoWPAN/ws/ws_common.h"
#include "wisun_tasklet.h"

#include "net_rpl.h"
#include "RPL/rpl_protocol.h"
#include "RPL/rpl_policy.h"
#include "RPL/rpl_control.h"
#include "RPL/rpl_objective.h"
#include "RPL/rpl_upward.h"
#include "RPL/rpl_downward.h"
#include "RPL/rpl_structures.h"

#include "application.h"
#include "mbed_config_app.h"

#include "ti_drivers_config.h"
#include <ti/drivers/GPIO.h>

#ifdef WISUN_NCP_ENABLE
#include "openthread/error.h"
#endif

#ifdef MBED_CONF_APP_CERTIFICATE_HEADER
#include MBED_CONF_APP_CERTIFICATE_HEADER
#endif

#include "ns_trace.h"
#define TRACE_GROUP "brro"

#define NR_BACKHAUL_INTERFACE_PHY_DRIVER_READY 2
#define NR_BACKHAUL_INTERFACE_PHY_DOWN  3
#define MESH_LINK_TIMEOUT 100
#define MESH_METRIC 1000

#define WS_DEFAULT_REGULATORY_DOMAIN 255
#define WS_DEFAULT_OPERATING_CLASS 255
#define WS_DEFAULT_OPERATING_MODE 255
#define WS_DEFAULT_UC_CHANNEL_FUNCTION 255
#define WS_DEFAULT_BC_CHANNEL_FUNCTION 255
#define WS_DEFAULT_UC_DWELL_INTERVAL 0
#define WS_DEFAULT_BC_INTERVAL 0
#define WS_DEFAULT_BC_DWELL_INTERVAL 0
#define WS_DEFAULT_UC_FIXED_CHANNEL 0xffff
#define WS_DEFAULT_BC_FIXED_CHANNEL 0xffff

extern configurable_props_t cfg_props;

static mac_api_t *mac_api;
static eth_mac_api_t *eth_mac_api;

typedef enum {
    STATE_UNKNOWN,
    STATE_DISCONNECTED,
    STATE_LINK_READY,
    STATE_BOOTSTRAP,
    STATE_CONNECTED,
    STATE_MAX_VALUE
} connection_state_e;

typedef struct {
    int8_t prefix_len;
    uint8_t prefix[16];
    uint8_t next_hop[16];
} route_info_t;

typedef struct {
    int8_t  ws_interface_id;
    int8_t  net_interface_id;
} ws_br_handler_t;

ws_br_handler_t ws_br_handler;

/* Backhaul prefix */
static uint8_t backhaul_prefix[16] = {0};

/* Backhaul default route information */
static route_info_t backhaul_route;
static int8_t br_tasklet_id = -1;

/* Network statistics */
static nwk_stats_t nwk_stats;

/* variables to help fetch rssi of neighbor nodes */
uint8_t cur_num_nbrs;
uint8_t nbr_idx = 0;
nbr_node_metrics_t nbr_nodes_metrics[SIZE_OF_NEIGH_LIST];

extern ti_wisun_config_t ti_wisun_config;

/* Function forward declarations */

#ifdef FEATURE_TIMAC_SUPPORT
extern void timacExtaddressRegister();
#endif

static void network_interface_event_handler(arm_event_s *event);
static void mesh_network_up(void);
static void eth_network_data_init(void);
static net_ipv6_mode_e backhaul_bootstrap_mode = NET_IPV6_BOOTSTRAP_STATIC;
static void borderrouter_tasklet(arm_event_s *event);
static int wisun_interface_up(void);
static void wisun_interface_event_handler(arm_event_s *event);
static void network_interface_event_handler(arm_event_s *event);
static int backhaul_interface_down(void);
static void borderrouter_backhaul_phy_status_cb(uint8_t link_up, int8_t driver_id);
extern mesh_error_t nanostack_wisunInterface_configure(void);
extern fhss_timer_t fhss_functions;
//bool is_net_if_up(void);

typedef struct {
    char *network_name;
    uint8_t regulatory_domain;
    uint8_t operating_class;
    uint8_t operating_mode;
    uint8_t uc_channel_function;
    uint8_t bc_channel_function;
    uint8_t uc_dwell_interval;
    uint32_t bc_interval;
    uint8_t bc_dwell_interval;
    uint16_t uc_fixed_channel;
    uint16_t bc_fixed_channel;
} ws_config_t;
static ws_config_t ws_conf;

static void mesh_network_up()
{
    tr_debug("Create Mesh Interface");

    int status;
    int8_t wisun_if_id = ws_br_handler.ws_interface_id;

#ifdef FIXED_GTK_KEYS
    ti_wisun_config.use_fixed_gtk_keys = true;
#endif

    status = arm_nwk_interface_configure_6lowpan_bootstrap_set(
                 wisun_if_id,
                 NET_6LOWPAN_BORDER_ROUTER,
                 NET_6LOWPAN_WS);

    if (status < 0) {
        tr_error("arm_nwk_interface_configure_6lowpan_bootstrap_set() failed");
        return;
    }

    status = wisun_interface_up();
    MBED_ASSERT(!status);
    if (status) {
        tr_error("wisun_interface_up() failed: %d", status);
    }
}

static void eth_network_data_init()
{
    memset(&backhaul_prefix[8], 0, 8);

    /* Bootstrap mode for the backhaul interface */
#if MBED_CONF_APP_BACKHAUL_DYNAMIC_BOOTSTRAP == 1
    backhaul_bootstrap_mode = NET_IPV6_BOOTSTRAP_AUTONOMOUS;
    tr_info("NET_IPV6_BOOTSTRAP_AUTONOMOUS");

#else
    tr_info("NET_IPV6_BOOTSTRAP_STATIC");
    backhaul_bootstrap_mode = NET_IPV6_BOOTSTRAP_STATIC;
    // done like this so that prefix can be left out in the dynamic case.
    const char *param = MBED_CONF_APP_BACKHAUL_PREFIX;
    stoip6(param, strlen(param), backhaul_prefix);
    tr_info("backhaul_prefix: %s", print_ipv6(backhaul_prefix));

    /* Backhaul route configuration*/
    memset(&backhaul_route, 0, sizeof(backhaul_route));
#ifdef MBED_CONF_APP_BACKHAUL_NEXT_HOP
    param = MBED_CONF_APP_BACKHAUL_NEXT_HOP;
    stoip6(param, strlen(param), backhaul_route.next_hop);
    tr_info("next hop: %s", print_ipv6(backhaul_route.next_hop));
#endif
    param = MBED_CONF_APP_BACKHAUL_DEFAULT_ROUTE;
    char *prefix, route_buf[255] = {0};
    /* copy the config value to a non-const buffer */
    strncpy(route_buf, param, sizeof(route_buf) - 1);
    prefix = strtok(route_buf, "/");
    backhaul_route.prefix_len = atoi(strtok(NULL, "/"));
    stoip6(prefix, strlen(prefix), backhaul_route.prefix);
    tr_info("backhaul route prefix: %s", print_ipv6(backhaul_route.prefix));
#endif
}

void load_config(void)
{
    ws_conf.network_name = malloc(MAX_NETWORK_NAME_SIZE);

    if(ws_conf.network_name)
    {
        strcpy(ws_conf.network_name, cfg_props.network_name);
    }
    else
    {
        tr_info("load_config: ws_conf.network_name pointer is NULL");
    }

    ws_conf.regulatory_domain = cfg_props.config_reg_domain;
    ws_conf.operating_class = cfg_props.operating_class;
    ws_conf.operating_mode = cfg_props.operating_mode;
    ws_conf.uc_channel_function = cfg_props.uc_channel_function;
    ws_conf.bc_channel_function = cfg_props.bc_channel_function;
    ws_conf.uc_dwell_interval = cfg_props.uc_dwell_interval;
    ws_conf.bc_interval = cfg_props.bc_interval;
    ws_conf.bc_dwell_interval = cfg_props.bc_dwell_interval;
    ws_conf.uc_fixed_channel = cfg_props.uc_fixed_channel;
    ws_conf.bc_fixed_channel = cfg_props.bc_fixed_channel;
}


void wisun_rf_init()
{
    int8_t rf_driver_id = 0;

    mac_description_storage_size_t storage_sizes;
    //storage_sizes.device_decription_table_size = 32;
    storage_sizes.device_decription_table_size = 4;
    storage_sizes.key_description_table_size = 4;
    storage_sizes.key_lookup_size = 1;
    storage_sizes.key_usage_size = 1;

    // donot lock/ unlock mutex when NCP is enabled as
    // the interface tasklet is run by nanostack scheduler
#ifndef WISUN_NCP_ENABLE
    nanostack_lock();
#endif

#ifndef FEATURE_TIMAC_SUPPORT
    NanostackTiRfPhy_init();
    rf_driver_id = NanostackTiRfPhy_rf_register();
#else
    timacExtaddressRegister();
#endif

    if(rf_driver_id >= 0)
    {
        // After the RF is up, we can seed the random from it.
        randLIB_seed_random();

        if (!mac_api) {
            mac_api = ns_sw_mac_create(rf_driver_id, &storage_sizes);
        }

        ws_br_handler.ws_interface_id = arm_nwk_interface_lowpan_init(mac_api, ws_conf.network_name);

        if (ws_br_handler.ws_interface_id < 0) {
            tr_error("Wi-SUN interface creation failed");
#ifndef WISUN_NCP_ENABLE
            nanostack_unlock();
#endif
            return ;
        }

        if (ws_br_handler.ws_interface_id > -1 &&
                ws_br_handler.net_interface_id > -1) {
            //ignoring return value of bbr start operation as backhaul is not implemented yet
            ws_bbr_start(ws_br_handler.ws_interface_id, ws_br_handler.net_interface_id);
        }
    }

#ifndef WISUN_NCP_ENABLE
    nanostack_unlock();
#endif

}


static int wisun_interface_up(void)
{
    int32_t ret;

    fhss_timer_t *fhss_timer_ptr = NULL;

    fhss_timer_ptr = &fhss_functions;

    ret = ws_management_node_init(ws_br_handler.ws_interface_id, ws_conf.regulatory_domain, ws_conf.network_name, fhss_timer_ptr);
    if (0 != ret) {
        tr_error("WS node init fail - code %"PRIi32"", ret);
        return -1;
    }

    if (ws_conf.uc_channel_function != WS_DEFAULT_UC_CHANNEL_FUNCTION) {
        ret = ws_management_fhss_unicast_channel_function_configure(ws_br_handler.ws_interface_id, ws_conf.uc_channel_function, ws_conf.uc_fixed_channel, ws_conf.uc_dwell_interval);
        if (ret != 0) {
            tr_error("Unicast channel function configuration failed %"PRIi32"", ret);
            return -1;
        }
    }
    if (ws_conf.bc_channel_function != WS_DEFAULT_BC_CHANNEL_FUNCTION ||
            ws_conf.bc_dwell_interval != WS_DEFAULT_BC_DWELL_INTERVAL ||
            ws_conf.bc_interval != WS_DEFAULT_BC_INTERVAL) {
        ret = ws_management_fhss_broadcast_channel_function_configure(ws_br_handler.ws_interface_id, ws_conf.bc_channel_function, ws_conf.bc_fixed_channel, ws_conf.bc_dwell_interval, ws_conf.bc_interval);
        if (ret != 0) {
            tr_error("Broadcast channel function configuration failed %"PRIi32"", ret);
            return -1;
        }
    }

    if (ws_conf.uc_dwell_interval != WS_DEFAULT_UC_DWELL_INTERVAL ||
            ws_conf.bc_dwell_interval != WS_DEFAULT_BC_DWELL_INTERVAL ||
            ws_conf.bc_interval != WS_DEFAULT_BC_INTERVAL) {
        ret = ws_management_fhss_timing_configure(ws_br_handler.ws_interface_id, ws_conf.uc_dwell_interval, ws_conf.bc_interval, ws_conf.bc_dwell_interval);
        if (ret != 0) {
            tr_error("fhss configuration failed %"PRIi32"", ret);
            return -1;
        }
    }
    if (ws_conf.regulatory_domain != WS_DEFAULT_REGULATORY_DOMAIN ||
            ws_conf.operating_mode != WS_DEFAULT_OPERATING_MODE ||
            ws_conf.operating_class != WS_DEFAULT_OPERATING_CLASS) {
        ret = ws_management_regulatory_domain_set(ws_br_handler.ws_interface_id, ws_conf.regulatory_domain, ws_conf.operating_class, ws_conf.operating_mode);
        if (ret != 0) {
            tr_error("Regulatory domain configuration failed %"PRIi32"", ret);
            return -1;
        }
    }

#ifdef MBED_CONF_APP_CERTIFICATE_HEADER
    /** Add Trusted Root Certificate/s ***/
    arm_network_trusted_certificates_remove();

    arm_certificate_entry_s trusted_cert = {
        .cert = MBED_CONF_APP_ROOT_CERTIFICATE,
        .key = NULL,
        .cert_len = 0,
        .key_len = 0
    };

    trusted_cert.cert_len = strlen((const char *) MBED_CONF_APP_ROOT_CERTIFICATE) + 1;
    arm_network_trusted_certificate_add((const arm_certificate_entry_s *)&trusted_cert);

    /*** Add Own Certificate and Private Key ***/
    arm_network_own_certificates_remove();

    arm_certificate_entry_s own_cert = {
        .cert = MBED_CONF_APP_OWN_CERTIFICATE,
        .key =  MBED_CONF_APP_OWN_CERTIFICATE_KEY,
        .cert_len = 0,
        .key_len = 0
    };
    own_cert.cert_len = strlen((const char *) MBED_CONF_APP_OWN_CERTIFICATE) + 1;
    own_cert.key_len = strlen((const char *) MBED_CONF_APP_OWN_CERTIFICATE_KEY) + 1;
    arm_network_own_certificate_add((const arm_certificate_entry_s *)&own_cert);

#endif //MBED_CONF_APP_CERTIFICATE_HEADER


    ret = arm_nwk_interface_up(ws_br_handler.ws_interface_id);
    if (ret != 0) {
        tr_error("mesh0 up Fail with code: %"PRIi32"", ret);
        return ret;
    }
    tr_info("mesh0 bootstrap ongoing..");
    return 0;
}

#undef ETH
#undef SLIP
#undef EMAC
#undef CELL
#define ETH 1
#define SLIP 2
#define EMAC 3
#define CELL 4


static int backhaul_interface_up(int8_t driver_id)
{
    int retval = -1;
    tr_debug("backhaul_interface_up: %i", driver_id);
    if (ws_br_handler.net_interface_id != -1) {
        tr_debug("Border RouterInterface already at active state");
        return retval;
    }

    if (!eth_mac_api) {
        //eth_mac_api = ethernet_mac_create(driver_id);
    }

#if MBED_CONF_APP_BACKHAUL_DRIVER == CELL
    if (eth_mac_api->iid64_get) {
        ws_br_handler.net_interface_id = arm_nwk_interface_ppp_init(eth_mac_api, "ppp0");
    } else
#endif
    {
        ws_br_handler.net_interface_id = arm_nwk_interface_ethernet_init(eth_mac_api, "bh0");
    }

    MBED_ASSERT(ws_br_handler.net_interface_id >= 0);
    if (ws_br_handler.net_interface_id >= 0) {
        tr_debug("Backhaul interface ID: %d", ws_br_handler.net_interface_id);
        if (ws_br_handler.ws_interface_id > -1) {
            ws_bbr_start(ws_br_handler.ws_interface_id, ws_br_handler.net_interface_id);
        }
        arm_nwk_interface_configure_ipv6_bootstrap_set(
            ws_br_handler.net_interface_id, backhaul_bootstrap_mode, backhaul_prefix);
        arm_nwk_interface_up(ws_br_handler.net_interface_id);
        retval = 0;
    } else {
        tr_error("Could not init ethernet");
    }

    return retval;
}

#undef ETH
#undef SLIP
#undef EMAC
#undef CELL

static int backhaul_interface_down(void)
{
    int retval = -1;
    if (ws_br_handler.net_interface_id != -1) {
        arm_nwk_interface_down(ws_br_handler.net_interface_id);
        ws_br_handler.net_interface_id = -1;
        retval = 0;
    } else {
        tr_debug("Could not set eth down");
    }
    return retval;
}

static void print_interface_addr(int id)
{
    uint8_t address_buf[128];
    int address_count = 0;
    char buf[128];

    if (arm_net_address_list_get(id, 128, address_buf, &address_count) == 0) {
        uint8_t *t_buf = address_buf;
        for (int i = 0; i < address_count; ++i) {
            ip6tos(t_buf, buf);
            tr_info(" [%d] %s", i, buf);
            t_buf += 16;
        }
    }
}

#if MBED_CONF_APP_DEBUG_TRACE
static void print_interface_addresses(void)
{
    tr_info("Backhaul interface addresses:");
    print_interface_addr(ws_br_handler.net_interface_id);

    tr_info("RF interface addresses:");
    print_interface_addr(ws_br_handler.ws_interface_id);
}
#endif

/**
  * \brief Border Router Main Tasklet
  *
  *  Tasklet Handle next items:
  *
  *  - EV_INIT event: Set Certificate Chain, RF Interface Boot UP, multicast Init
  *  - SYSTEM_TIMER event: For RF interface Handshake purpose
  *
  */
static void borderrouter_tasklet(arm_event_s *event)
{
    arm_library_event_type_e event_type;
    event_type = (arm_library_event_type_e)event->event_type;

    switch (event_type) {
        case ARM_LIB_NWK_INTERFACE_EVENT:
            if (event->event_id == ws_br_handler.net_interface_id) {
                network_interface_event_handler(event);
            } else {
                wisun_interface_event_handler(event);
            }

            break;
        // comes from the backhaul_driver_init.
        case APPLICATION_EVENT:
            if (event->event_id == NR_BACKHAUL_INTERFACE_PHY_DRIVER_READY) {
                int8_t net_backhaul_id = (int8_t) event->event_data;

                if (backhaul_interface_up(net_backhaul_id) != 0) {
                    tr_debug("Backhaul bootstrap start failed");
                } else {
                    tr_debug("Backhaul bootstrap started");
                }
            } else if (event->event_id == NR_BACKHAUL_INTERFACE_PHY_DOWN) {
                if (backhaul_interface_down() == 0) {
                    tr_debug("Backhaul interface is down");
                }
            }
            break;

        case ARM_LIB_TASKLET_INIT_EVENT:
            br_tasklet_id = event->receiver;
            //eth_network_data_init();
            //backhaul_driver_init(borderrouter_backhaul_phy_status_cb);
            GPIO_write(CONFIG_GPIO_RLED, 0);
            mesh_network_up();
            eventOS_event_timer_request(9, ARM_LIB_SYSTEM_TIMER_EVENT, br_tasklet_id, 20000);
            break;

        case ARM_LIB_SYSTEM_TIMER_EVENT:
            eventOS_event_timer_cancel(event->event_id, event->receiver);

            if (event->event_id == 9) {
#if MBED_CONF_APP_DEBUG_TRACE
#ifndef WISUN_DEMO
                arm_print_routing_table();
                arm_print_neigh_cache();
                print_memory_stats();
                // Trace interface addresses. This trace can be removed if nanostack prints added/removed
                // addresses.
                print_interface_addresses();
#endif
/* endif for WISUN_DEMO */
#endif
/* endif for MBED_CONF_APP_DEBUG_TRACE */
                eventOS_event_timer_request(9, ARM_LIB_SYSTEM_TIMER_EVENT, br_tasklet_id, 20000);
            }
            break;

        default:
            break;
    }
}

static void borderrouter_backhaul_phy_status_cb(uint8_t link_up, int8_t driver_id)
{
    arm_event_s event = {
        .sender = br_tasklet_id,
        .receiver = br_tasklet_id,
        .priority = ARM_LIB_MED_PRIORITY_EVENT,
        .event_type = APPLICATION_EVENT,
        .event_data = driver_id
    };

    if (link_up) {
        event.event_id = NR_BACKHAUL_INTERFACE_PHY_DRIVER_READY;
    } else {
        event.event_id = NR_BACKHAUL_INTERFACE_PHY_DOWN;
    }

    eventOS_event_send(&event);
}

// ethernet interface
static void network_interface_event_handler(arm_event_s *event)
{
    arm_nwk_interface_status_type_e status = (arm_nwk_interface_status_type_e)event->event_data;
    switch (status) {
        case (ARM_NWK_BOOTSTRAP_READY): { // Interface configured Bootstrap is ready

            tr_info("BR interface_id: %d", ws_br_handler.net_interface_id);
            if (-1 != ws_br_handler.net_interface_id) {
                // metric set to high priority
                if (0 != arm_net_interface_set_metric(ws_br_handler.net_interface_id, 0)) {
                    tr_warn("Failed to set metric for eth0.");
                }

                if (backhaul_bootstrap_mode == NET_IPV6_BOOTSTRAP_STATIC) {
                    uint8_t *next_hop_ptr;

                    if (memcmp(backhaul_route.next_hop, (const uint8_t[16]) {
                    0
                }, 16) == 0) {
                        next_hop_ptr = NULL;
                    } else {
                        next_hop_ptr = backhaul_route.next_hop;
                    }
                    tr_debug("Default route prefix: %s/%d", print_ipv6(backhaul_route.prefix),
                             backhaul_route.prefix_len);
                    tr_debug("Default route next hop: %s", print_ipv6(backhaul_route.next_hop));
                    arm_net_route_add(backhaul_route.prefix,
                                      backhaul_route.prefix_len,
                                      next_hop_ptr, 0xffffffff, 128,
                                      ws_br_handler.net_interface_id);
                }
                tr_info("Backhaul interface addresses:");
                print_interface_addr(ws_br_handler.net_interface_id);
            }
            break;
        }
        case (ARM_NWK_RPL_INSTANCE_FLOODING_READY): // RPL instance have been flooded
            tr_info("RPL instance have been flooded");
            break;
        case (ARM_NWK_SET_DOWN_COMPLETE): // Interface DOWN command successfully
            break;
        case (ARM_NWK_NWK_SCAN_FAIL):   // Interface have not detect any valid network
            tr_warning("mesh0 haven't detect any valid nwk");
            break;
        case (ARM_NWK_IP_ADDRESS_ALLOCATION_FAIL): // IP address allocation fail(ND, DHCPv4 or DHCPv6)
            tr_error("NO GP address detected");
            break;
        case (ARM_NWK_DUPLICATE_ADDRESS_DETECTED): // User specific GP16 was not valid
            tr_error("Ethernet IPv6 Duplicate addr detected!");
            break;
        case (ARM_NWK_AUHTENTICATION_START_FAIL): // No valid Authentication server detected behind access point ;
            tr_error("No valid ath server detected behind AP");
            break;
        case (ARM_NWK_AUHTENTICATION_FAIL): // Network authentication fail by Handshake
            tr_error("Network authentication fail");
            break;
        case (ARM_NWK_NWK_CONNECTION_DOWN): // No connection between Access point or Default Router
            tr_warning("Prefix timeout");
            break;
        case (ARM_NWK_NWK_PARENT_POLL_FAIL): // Sleepy host poll fail 3 time
            tr_warning("Parent poll fail");
            break;
        case (ARM_NWK_PHY_CONNECTION_DOWN): // Interface PHY cable off or serial port interface not respond anymore
            tr_error("eth0 down");
            break;
        default:
            tr_warning("Unknown nwk if event (type: %02x, id: %02x, data: %02x)", event->event_type, event->event_id, (unsigned int)event->event_data);
            break;
    }
}

static void wisun_interface_event_handler(arm_event_s *event)
{
    arm_nwk_interface_status_type_e status = (arm_nwk_interface_status_type_e)event->event_data;
    switch (status) {
        case (ARM_NWK_BOOTSTRAP_READY): { // Interface configured Bootstrap is ready
            tr_info("Wisun bootstrap ready");

            if (arm_net_interface_set_metric(ws_br_handler.ws_interface_id, MESH_METRIC) != 0) {
                tr_warn("Failed to set metric for mesh0.");
            }

            tr_info("RF interface addresses:");
            print_interface_addr(ws_br_handler.ws_interface_id);

            break;
        }
        case (ARM_NWK_SET_DOWN_COMPLETE):
            tr_info("Wisun interface down");
            break;
        default:
            tr_warning("Unknown nwk if event (type: %02x, id: %02x, data: %02x)", event->event_type, event->event_id, (unsigned int)event->event_data);
            break;
    }

}

/* function to initialize the br_handlers */
void ws_br_handler_init()
{
    ws_br_handler.ws_interface_id = -1;
    ws_br_handler.net_interface_id = -1;
}

void border_router_loadcfg_init(void)
{
    load_config();
    wisun_rf_init();
}

/*!
 * Check if bringing up of network interface is done or not
 * return value: true if already done, false if not
 */
bool is_net_if_up(void)
{
    //ignoring net_interface_id as back haul is not implemented yet
    if(ws_br_handler.ws_interface_id < 0)
    {
        return(false);
    }
    else
    {
        return(true);
    }
}

/*!
 * Initialize and run border router tasklet
 */
void border_router_tasklet_start(void)
{
    protocol_stats_start(&nwk_stats);

    eventOS_event_handler_create(
        &borderrouter_tasklet,
        ARM_LIB_TASKLET_INIT_EVENT);
}

#ifdef WISUN_NCP_ENABLE

/* Below functions are invoked on receiving
 * the appropriate commands from host */

/*!
 * Bring up the network interface
 */
otError nanostack_net_if_up()
{
    if(MESH_ERROR_NONE != nanostack_wisunInterface_configure())
    {
        return(OT_ERROR_FAILED);
    }

    border_router_loadcfg_init();

    if(!is_net_if_up())
    {
        return(OT_ERROR_FAILED);
    }
    else
    {
        return(OT_ERROR_NONE);
    }
}

/*!
 * Check if wisun stack on border router has been brought up or not
 */
bool is_net_stack_up(void)
{
    protocol_interface_info_entry_t *cur;

    cur = protocol_stack_interface_info_get_by_id(ws_br_handler.ws_interface_id);

    if (!cur) {
        return false;
    }

    if(ER_BOOTSRAP_DONE == cur->nwk_bootstrap_state)
    {
        return(true);
    }
    else
    {
        return(false);
    }
}

/*!
 * Start the process to bring up wisun stack
 */
otError nanostack_net_stack_up(void)
{

    border_router_tasklet_start();
#ifdef FEATURE_TIMAC_SUPPORT
    wisun_tasklet_statistics_start();
#endif
    return(OT_ERROR_NONE);
}

uint8_t get_current_net_state(void)
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    uint8_t curNetState = 0;

    if(!cur)
    {
        tr_debug("get_current_net_state: NULL pointer");
        return curNetState;
    }

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
        default:
            break;
    }

    return(curNetState);
}

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
 * Helper function - returns the Link Local address of the node
 */
if_address_entry_t *get_linkLocal_address()
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    if(!cur)
    {
        tr_debug("get_linkLocal_address: NULL pointer");
        return NULL;
    }

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
if_address_entry_t *get_globalUnicast_address()
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    if(cur)
    {
        ns_list_foreach(if_address_entry_t, entry, &cur->ip_addresses)
        {
           if (entry->source != ADDR_SOURCE_UNKNOWN)
           {
               return entry;
           }
        }
    }

    /* if we are here could not find the address */
    return NULL;

}

/*!
 * Helper function - get current multicast groups a node is subscribed to
 */
if_group_list_t *get_multicast_ip_groups()
{
    protocol_interface_info_entry_t *cur;
    if_group_list_t *multicast_list = NULL;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);

    if(cur)
    {
        multicast_list = &cur->ip_groups;
    }

    return multicast_list;
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
    if (!cur || !addr_is_ipv6_multicast(address) ||
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
    return rpl_instance_match_dao_target(instance, addr, 128);
}

int revoke_gtk_hwaddr(uint8_t *eui64)
{
    protocol_interface_info_entry_t *cur;
    cur = protocol_stack_interface_info_get(IF_6LoWPAN);
    if (!cur || !cur->rpl_domain) {
        return -1;
    }

    if(ws_bbr_node_keys_remove(cur->id, eui64) != 0)
    {
        /* failed to remove EUI64. Either invalid entry or other issues */
        return -1;
    }

    ws_bbr_node_access_revoke_start(cur->id);

    /* Currently if a device is revoked access, all roote cert are removed to prevent
     * device from joining again. This will also block other devices to join.
     * Custom APIs to add specific root certificates or use of CRL needs to be implemented for finer
     * device management
     */
    arm_network_trusted_certificates_remove();


    return 0;
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


#endif //WISUN_NCP_ENABLE

