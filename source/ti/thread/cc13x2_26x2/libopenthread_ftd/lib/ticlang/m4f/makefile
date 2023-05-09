SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

CC = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"
AR = "$(TICLANG_ARMCOMPILER)/bin/tiarmar"

OBJECTS = src_core_api_backbone_router_api.obj src_core_api_backbone_router_ftd_api.obj src_core_api_border_agent_api.obj src_core_api_border_router_api.obj src_core_api_border_routing_api.obj src_core_api_channel_manager_api.obj src_core_api_channel_monitor_api.obj src_core_api_child_supervision_api.obj src_core_api_coap_api.obj src_core_api_coap_secure_api.obj src_core_api_commissioner_api.obj src_core_api_crypto_api.obj src_core_api_dataset_api.obj src_core_api_dataset_ftd_api.obj src_core_api_dataset_updater_api.obj src_core_api_diags_api.obj src_core_api_dns_api.obj src_core_api_dns_server_api.obj src_core_api_error_api.obj src_core_api_heap_api.obj src_core_api_history_tracker_api.obj src_core_api_icmp6_api.obj src_core_api_instance_api.obj src_core_api_ip6_api.obj src_core_api_jam_detection_api.obj src_core_api_joiner_api.obj src_core_api_link_api.obj src_core_api_link_metrics_api.obj src_core_api_link_raw_api.obj src_core_api_logging_api.obj src_core_api_message_api.obj src_core_api_nat64_api.obj src_core_api_multi_radio_api.obj src_core_api_netdata_api.obj src_core_api_netdata_publisher_api.obj src_core_api_netdiag_api.obj src_core_api_network_time_api.obj src_core_api_ping_sender_api.obj src_core_api_random_crypto_api.obj src_core_api_random_noncrypto_api.obj src_core_api_server_api.obj src_core_api_sntp_api.obj src_core_api_srp_client_api.obj src_core_api_srp_client_buffers_api.obj src_core_api_srp_server_api.obj src_core_api_tasklet_api.obj src_core_api_tcp_api.obj src_core_api_thread_api.obj src_core_api_thread_ftd_api.obj src_core_api_trel_api.obj src_core_api_udp_api.obj src_core_backbone_router_backbone_tmf.obj src_core_backbone_router_bbr_leader.obj src_core_backbone_router_bbr_local.obj src_core_backbone_router_bbr_manager.obj src_core_backbone_router_multicast_listeners_table.obj src_core_backbone_router_ndproxy_table.obj src_core_border_router_infra_if.obj src_core_border_router_routing_manager.obj src_core_coap_coap.obj src_core_coap_coap_message.obj src_core_coap_coap_secure.obj src_core_common_appender.obj src_core_common_binary_search.obj src_core_common_crc16.obj src_core_common_data.obj src_core_common_error.obj src_core_common_frame_builder.obj src_core_common_frame_data.obj src_core_common_heap.obj src_core_common_heap_data.obj src_core_common_heap_string.obj src_core_common_instance.obj src_core_common_log.obj src_core_common_message.obj src_core_common_notifier.obj src_core_common_random.obj src_core_common_settings.obj src_core_common_string.obj src_core_common_tasklet.obj src_core_common_time_ticker.obj src_core_common_timer.obj src_core_common_tlvs.obj src_core_common_trickle_timer.obj src_core_common_uptime.obj src_core_crypto_aes_ccm.obj src_core_crypto_aes_ecb.obj src_core_crypto_crypto_platform.obj src_core_crypto_ecdsa.obj src_core_crypto_ecdsa_tinycrypt.obj src_core_crypto_hkdf_sha256.obj src_core_crypto_hmac_sha256.obj src_core_crypto_mbedtls.obj src_core_crypto_pbkdf2_cmac.obj src_core_crypto_sha256.obj src_core_crypto_storage.obj src_core_diags_factory_diags.obj src_core_mac_channel_mask.obj src_core_mac_data_poll_handler.obj src_core_mac_data_poll_sender.obj src_core_mac_link_raw.obj src_core_mac_mac.obj src_core_mac_mac_filter.obj src_core_mac_mac_frame.obj src_core_mac_mac_links.obj src_core_mac_mac_types.obj src_core_mac_sub_mac.obj src_core_mac_sub_mac_callbacks.obj src_core_meshcop_announce_begin_client.obj src_core_meshcop_border_agent.obj src_core_meshcop_commissioner.obj src_core_meshcop_dataset.obj src_core_meshcop_dataset_local.obj src_core_meshcop_dataset_manager.obj src_core_meshcop_dataset_manager_ftd.obj src_core_meshcop_dataset_updater.obj src_core_meshcop_dtls.obj src_core_meshcop_energy_scan_client.obj src_core_meshcop_extended_panid.obj src_core_meshcop_joiner.obj src_core_meshcop_joiner_router.obj src_core_meshcop_meshcop.obj src_core_meshcop_meshcop_leader.obj src_core_meshcop_meshcop_tlvs.obj src_core_meshcop_network_name.obj src_core_meshcop_panid_query_client.obj src_core_meshcop_timestamp.obj src_core_net_checksum.obj src_core_net_dhcp6_client.obj src_core_net_dhcp6_server.obj src_core_net_dns_client.obj src_core_net_dns_dso.obj src_core_net_dns_types.obj src_core_net_dnssd_server.obj src_core_net_icmp6.obj src_core_net_ip4_types.obj src_core_net_ip6.obj src_core_net_ip6_address.obj src_core_net_ip6_filter.obj src_core_net_ip6_headers.obj src_core_net_ip6_mpl.obj src_core_net_nd_agent.obj src_core_net_nd6.obj src_core_net_nat64_translator.obj src_core_net_netif.obj src_core_net_sntp_client.obj src_core_net_socket.obj src_core_net_srp_client.obj src_core_net_srp_server.obj src_core_net_tcp6.obj src_core_net_udp6.obj src_core_radio_radio.obj src_core_radio_radio_callbacks.obj src_core_radio_radio_platform.obj src_core_radio_trel_interface.obj src_core_radio_trel_link.obj src_core_radio_trel_packet.obj src_core_thread_address_resolver.obj src_core_thread_announce_begin_server.obj src_core_thread_announce_sender.obj src_core_thread_anycast_locator.obj src_core_thread_child_table.obj src_core_thread_csl_tx_scheduler.obj src_core_thread_discover_scanner.obj src_core_thread_dua_manager.obj src_core_thread_energy_scan_server.obj src_core_thread_indirect_sender.obj src_core_thread_key_manager.obj src_core_thread_link_metrics.obj src_core_thread_link_quality.obj src_core_thread_lowpan.obj src_core_thread_mesh_forwarder.obj src_core_thread_mesh_forwarder_ftd.obj src_core_thread_mesh_forwarder_mtd.obj src_core_thread_mle.obj src_core_thread_mle_router.obj src_core_thread_mle_types.obj src_core_thread_mlr_manager.obj src_core_thread_neighbor_table.obj src_core_thread_network_data.obj src_core_thread_network_data_leader.obj src_core_thread_network_data_leader_ftd.obj src_core_thread_network_data_local.obj src_core_thread_network_data_notifier.obj src_core_thread_network_data_publisher.obj src_core_thread_network_data_service.obj src_core_thread_network_data_tlvs.obj src_core_thread_network_data_types.obj src_core_thread_network_diagnostic.obj src_core_thread_panid_query_server.obj src_core_thread_radio_selector.obj src_core_thread_router_table.obj src_core_thread_src_match_controller.obj src_core_thread_thread_netif.obj src_core_thread_time_sync_service.obj src_core_thread_tmf.obj src_core_thread_topology.obj src_core_thread_uri_paths.obj src_core_utils_channel_manager.obj src_core_utils_channel_monitor.obj src_core_utils_child_supervision.obj src_core_utils_flash.obj src_core_utils_heap.obj src_core_utils_history_tracker.obj src_core_utils_jam_detector.obj src_core_utils_otns.obj src_core_utils_parse_cmdline.obj src_core_utils_ping_sender.obj src_core_utils_slaac_address.obj src_core_utils_srp_client_buffers.obj third_party_tcplp_lib_bitmap.obj third_party_tcplp_lib_cbuf.obj third_party_tcplp_lib_lbuf.obj third_party_tcplp_bsdtcp_tcp_input.obj third_party_tcplp_bsdtcp_tcp_output.obj third_party_tcplp_bsdtcp_tcp_reass.obj third_party_tcplp_bsdtcp_tcp_sack.obj third_party_tcplp_bsdtcp_tcp_subr.obj third_party_tcplp_bsdtcp_tcp_timer.obj third_party_tcplp_bsdtcp_tcp_timewait.obj third_party_tcplp_bsdtcp_tcp_usrreq.obj third_party_tcplp_bsdtcp_cc_cc_newreno.obj src_lib_platform_exit_code.obj
NAME = libopenthread_ftd

CFLAGS = -I../../.. \
    -Oz \
    -g \
    -gdwarf-3 \
    -Wall \
    -Wno-ti-macros \
    -ffunction-sections \
    -Wextra \
    -Wshadow \
    -Werror \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/examples/platforms \
    -DHAVE_CONFIG_H -D_BSD_SOURCE=1 -D_DEFAULT_SOURCE=1 \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/third_party/mbedtls \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/third_party/mbedtls/repo/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/include \
    -DMBEDTLS_CONFIG_FILE='"mbedtls-config-cc13x2_26x2.h"' \
    -I${WorkspaceDirPath}/libmbedcrypto_cc13x2_26x2_ticlang/config \
    -DDeviceFamily_CC13X2_CC26X2 \
    -DOPENTHREAD_FTD=1 \
    -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-cc13x2_26x2-config.h"' \
    -DOPENTHREAD_CONFIG_FILE='"openthread-config-cc13x2_26x2-ftd.h"' \
    -I${WorkspaceDirPath}/libopenthread_ftd_cc13x2_26x2_ticlang/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src/core \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16

CPPFLAGS = -I../../.. \
    -Oz \
    -g \
    -gdwarf-3 \
    -Wall \
    -Wno-ti-macros \
    -ffunction-sections \
    -Wextra \
    -Wshadow \
    -Werror \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/examples/platforms \
    -DHAVE_CONFIG_H -D_BSD_SOURCE=1 -D_DEFAULT_SOURCE=1 \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/third_party/mbedtls \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/third_party/mbedtls/repo/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/include \
    -DMBEDTLS_CONFIG_FILE='"mbedtls-config-cc13x2_26x2.h"' \
    -I${WorkspaceDirPath}/libmbedcrypto_cc13x2_26x2_ticlang/config \
    -DDeviceFamily_CC13X2_CC26X2 \
    -DOPENTHREAD_FTD=1 \
    -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-cc13x2_26x2-config.h"' \
    -DOPENTHREAD_CONFIG_FILE='"openthread-config-cc13x2_26x2-ftd.h"' \
    -I${WorkspaceDirPath}/libopenthread_ftd_cc13x2_26x2_ticlang/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src/core \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16

all: $(NAME).a

third_party_tcplp_lib_bitmap.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/lib/bitmap.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_lib_cbuf.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/lib/cbuf.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_lib_lbuf.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/lib/lbuf.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_input.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_input.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_output.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_output.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_reass.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_reass.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_sack.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_sack.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_subr.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_subr.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_timer.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_timer.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_timewait.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_timewait.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_tcp_usrreq.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/tcp_usrreq.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

third_party_tcplp_bsdtcp_cc_cc_newreno.obj: OPENTHREAD_MAKE_PATH/third_party/tcplp/bsdtcp/cc/cc_newreno.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

src_lib_platform_exit_code.obj: OPENTHREAD_MAKE_PATH/src/lib/platform/exit_code.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

src_core_api_backbone_router_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/backbone_router_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_backbone_router_ftd_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/backbone_router_ftd_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_border_agent_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/border_agent_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_border_router_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/border_router_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_border_routing_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/border_routing_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_channel_manager_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/channel_manager_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_channel_monitor_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/channel_monitor_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_child_supervision_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/child_supervision_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_coap_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/coap_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_coap_secure_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/coap_secure_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_commissioner_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/commissioner_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_crypto_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/crypto_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_dataset_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/dataset_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_dataset_ftd_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/dataset_ftd_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_dataset_updater_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/dataset_updater_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_diags_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/diags_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_dns_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/dns_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_dns_server_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/dns_server_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_error_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/error_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_heap_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/heap_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_history_tracker_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/history_tracker_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_icmp6_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/icmp6_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_instance_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/instance_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_ip6_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/ip6_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_jam_detection_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/jam_detection_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_joiner_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/joiner_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_link_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/link_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_link_metrics_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/link_metrics_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_link_raw_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/link_raw_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_logging_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/logging_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_message_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/message_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_nat64_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/nat64_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_multi_radio_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/multi_radio_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_netdata_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/netdata_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_netdata_publisher_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/netdata_publisher_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_netdiag_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/netdiag_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_network_time_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/network_time_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_ping_sender_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/ping_sender_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_random_crypto_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/random_crypto_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_random_noncrypto_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/random_noncrypto_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_server_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/server_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_sntp_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/sntp_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_srp_client_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/srp_client_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_srp_client_buffers_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/srp_client_buffers_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_srp_server_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/srp_server_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_tasklet_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/tasklet_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_tcp_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/tcp_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_thread_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/thread_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_thread_ftd_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/thread_ftd_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_trel_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/trel_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_udp_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/udp_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_backbone_router_backbone_tmf.obj: OPENTHREAD_MAKE_PATH/src/core/backbone_router/backbone_tmf.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_backbone_router_bbr_leader.obj: OPENTHREAD_MAKE_PATH/src/core/backbone_router/bbr_leader.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_backbone_router_bbr_local.obj: OPENTHREAD_MAKE_PATH/src/core/backbone_router/bbr_local.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_backbone_router_bbr_manager.obj: OPENTHREAD_MAKE_PATH/src/core/backbone_router/bbr_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_backbone_router_multicast_listeners_table.obj: OPENTHREAD_MAKE_PATH/src/core/backbone_router/multicast_listeners_table.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_backbone_router_ndproxy_table.obj: OPENTHREAD_MAKE_PATH/src/core/backbone_router/ndproxy_table.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_border_router_infra_if.obj: OPENTHREAD_MAKE_PATH/src/core/border_router/infra_if.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_border_router_routing_manager.obj: OPENTHREAD_MAKE_PATH/src/core/border_router/routing_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_coap_coap.obj: OPENTHREAD_MAKE_PATH/src/core/coap/coap.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_coap_coap_message.obj: OPENTHREAD_MAKE_PATH/src/core/coap/coap_message.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_coap_coap_secure.obj: OPENTHREAD_MAKE_PATH/src/core/coap/coap_secure.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_appender.obj: OPENTHREAD_MAKE_PATH/src/core/common/appender.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_binary_search.obj: OPENTHREAD_MAKE_PATH/src/core/common/binary_search.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_crc16.obj: OPENTHREAD_MAKE_PATH/src/core/common/crc16.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_data.obj: OPENTHREAD_MAKE_PATH/src/core/common/data.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_error.obj: OPENTHREAD_MAKE_PATH/src/core/common/error.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_frame_builder.obj: OPENTHREAD_MAKE_PATH/src/core/common/frame_builder.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_frame_data.obj: OPENTHREAD_MAKE_PATH/src/core/common/frame_data.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_heap.obj: OPENTHREAD_MAKE_PATH/src/core/common/heap.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_heap_data.obj: OPENTHREAD_MAKE_PATH/src/core/common/heap_data.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_heap_string.obj: OPENTHREAD_MAKE_PATH/src/core/common/heap_string.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_instance.obj: OPENTHREAD_MAKE_PATH/src/core/common/instance.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_log.obj: OPENTHREAD_MAKE_PATH/src/core/common/log.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_message.obj: OPENTHREAD_MAKE_PATH/src/core/common/message.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_notifier.obj: OPENTHREAD_MAKE_PATH/src/core/common/notifier.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_random.obj: OPENTHREAD_MAKE_PATH/src/core/common/random.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_settings.obj: OPENTHREAD_MAKE_PATH/src/core/common/settings.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_string.obj: OPENTHREAD_MAKE_PATH/src/core/common/string.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_tasklet.obj: OPENTHREAD_MAKE_PATH/src/core/common/tasklet.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_time_ticker.obj: OPENTHREAD_MAKE_PATH/src/core/common/time_ticker.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_timer.obj: OPENTHREAD_MAKE_PATH/src/core/common/timer.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_tlvs.obj: OPENTHREAD_MAKE_PATH/src/core/common/tlvs.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_trickle_timer.obj: OPENTHREAD_MAKE_PATH/src/core/common/trickle_timer.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_uptime.obj: OPENTHREAD_MAKE_PATH/src/core/common/uptime.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_aes_ccm.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/aes_ccm.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_aes_ecb.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/aes_ecb.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_crypto_platform.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/crypto_platform.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_ecdsa.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/ecdsa.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_ecdsa_tinycrypt.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/ecdsa_tinycrypt.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_hkdf_sha256.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/hkdf_sha256.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_hmac_sha256.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/hmac_sha256.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_mbedtls.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/mbedtls.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_pbkdf2_cmac.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/pbkdf2_cmac.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_sha256.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/sha256.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_crypto_storage.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/storage.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_diags_factory_diags.obj: OPENTHREAD_MAKE_PATH/src/core/diags/factory_diags.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_channel_mask.obj: OPENTHREAD_MAKE_PATH/src/core/mac/channel_mask.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_data_poll_handler.obj: OPENTHREAD_MAKE_PATH/src/core/mac/data_poll_handler.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_data_poll_sender.obj: OPENTHREAD_MAKE_PATH/src/core/mac/data_poll_sender.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_link_raw.obj: OPENTHREAD_MAKE_PATH/src/core/mac/link_raw.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_mac.obj: OPENTHREAD_MAKE_PATH/src/core/mac/mac.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_mac_filter.obj: OPENTHREAD_MAKE_PATH/src/core/mac/mac_filter.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_mac_frame.obj: OPENTHREAD_MAKE_PATH/src/core/mac/mac_frame.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_mac_links.obj: OPENTHREAD_MAKE_PATH/src/core/mac/mac_links.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_mac_types.obj: OPENTHREAD_MAKE_PATH/src/core/mac/mac_types.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_sub_mac.obj: OPENTHREAD_MAKE_PATH/src/core/mac/sub_mac.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_sub_mac_callbacks.obj: OPENTHREAD_MAKE_PATH/src/core/mac/sub_mac_callbacks.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_announce_begin_client.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/announce_begin_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_border_agent.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/border_agent.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_commissioner.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/commissioner.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_dataset.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/dataset.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_dataset_local.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/dataset_local.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_dataset_manager.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/dataset_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_dataset_manager_ftd.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/dataset_manager_ftd.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_dataset_updater.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/dataset_updater.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_dtls.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/dtls.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_energy_scan_client.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/energy_scan_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_extended_panid.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/extended_panid.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_joiner.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/joiner.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_joiner_router.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/joiner_router.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_meshcop.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/meshcop.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_meshcop_leader.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/meshcop_leader.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_meshcop_tlvs.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/meshcop_tlvs.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_network_name.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/network_name.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_panid_query_client.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/panid_query_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_meshcop_timestamp.obj: OPENTHREAD_MAKE_PATH/src/core/meshcop/timestamp.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_checksum.obj: OPENTHREAD_MAKE_PATH/src/core/net/checksum.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_dhcp6_client.obj: OPENTHREAD_MAKE_PATH/src/core/net/dhcp6_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_dhcp6_server.obj: OPENTHREAD_MAKE_PATH/src/core/net/dhcp6_server.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_dns_client.obj: OPENTHREAD_MAKE_PATH/src/core/net/dns_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_dns_dso.obj: OPENTHREAD_MAKE_PATH/src/core/net/dns_dso.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_dns_types.obj: OPENTHREAD_MAKE_PATH/src/core/net/dns_types.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_dnssd_server.obj: OPENTHREAD_MAKE_PATH/src/core/net/dnssd_server.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_icmp6.obj: OPENTHREAD_MAKE_PATH/src/core/net/icmp6.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_ip4_types.obj: OPENTHREAD_MAKE_PATH/src/core/net/ip4_types.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_ip6.obj: OPENTHREAD_MAKE_PATH/src/core/net/ip6.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_ip6_address.obj: OPENTHREAD_MAKE_PATH/src/core/net/ip6_address.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_ip6_filter.obj: OPENTHREAD_MAKE_PATH/src/core/net/ip6_filter.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_ip6_headers.obj: OPENTHREAD_MAKE_PATH/src/core/net/ip6_headers.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_ip6_mpl.obj: OPENTHREAD_MAKE_PATH/src/core/net/ip6_mpl.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_nd_agent.obj: OPENTHREAD_MAKE_PATH/src/core/net/nd_agent.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_nd6.obj: OPENTHREAD_MAKE_PATH/src/core/net/nd6.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_nat64_translator.obj: OPENTHREAD_MAKE_PATH/src/core/net/nat64_translator.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_netif.obj: OPENTHREAD_MAKE_PATH/src/core/net/netif.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_sntp_client.obj: OPENTHREAD_MAKE_PATH/src/core/net/sntp_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_socket.obj: OPENTHREAD_MAKE_PATH/src/core/net/socket.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_srp_client.obj: OPENTHREAD_MAKE_PATH/src/core/net/srp_client.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_srp_server.obj: OPENTHREAD_MAKE_PATH/src/core/net/srp_server.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_tcp6.obj: OPENTHREAD_MAKE_PATH/src/core/net/tcp6.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_net_udp6.obj: OPENTHREAD_MAKE_PATH/src/core/net/udp6.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_radio.obj: OPENTHREAD_MAKE_PATH/src/core/radio/radio.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_radio_callbacks.obj: OPENTHREAD_MAKE_PATH/src/core/radio/radio_callbacks.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_radio_platform.obj: OPENTHREAD_MAKE_PATH/src/core/radio/radio_platform.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_trel_interface.obj: OPENTHREAD_MAKE_PATH/src/core/radio/trel_interface.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_trel_link.obj: OPENTHREAD_MAKE_PATH/src/core/radio/trel_link.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_trel_packet.obj: OPENTHREAD_MAKE_PATH/src/core/radio/trel_packet.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_address_resolver.obj: OPENTHREAD_MAKE_PATH/src/core/thread/address_resolver.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_announce_begin_server.obj: OPENTHREAD_MAKE_PATH/src/core/thread/announce_begin_server.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_announce_sender.obj: OPENTHREAD_MAKE_PATH/src/core/thread/announce_sender.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_anycast_locator.obj: OPENTHREAD_MAKE_PATH/src/core/thread/anycast_locator.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_child_table.obj: OPENTHREAD_MAKE_PATH/src/core/thread/child_table.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_csl_tx_scheduler.obj: OPENTHREAD_MAKE_PATH/src/core/thread/csl_tx_scheduler.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_discover_scanner.obj: OPENTHREAD_MAKE_PATH/src/core/thread/discover_scanner.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_dua_manager.obj: OPENTHREAD_MAKE_PATH/src/core/thread/dua_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_energy_scan_server.obj: OPENTHREAD_MAKE_PATH/src/core/thread/energy_scan_server.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_indirect_sender.obj: OPENTHREAD_MAKE_PATH/src/core/thread/indirect_sender.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_key_manager.obj: OPENTHREAD_MAKE_PATH/src/core/thread/key_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_link_metrics.obj: OPENTHREAD_MAKE_PATH/src/core/thread/link_metrics.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_link_quality.obj: OPENTHREAD_MAKE_PATH/src/core/thread/link_quality.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_lowpan.obj: OPENTHREAD_MAKE_PATH/src/core/thread/lowpan.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mesh_forwarder.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mesh_forwarder.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mesh_forwarder_ftd.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mesh_forwarder_ftd.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mesh_forwarder_mtd.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mesh_forwarder_mtd.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mle.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mle.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mle_router.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mle_router.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mle_types.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mle_types.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_mlr_manager.obj: OPENTHREAD_MAKE_PATH/src/core/thread/mlr_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_neighbor_table.obj: OPENTHREAD_MAKE_PATH/src/core/thread/neighbor_table.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_leader.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_leader.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_leader_ftd.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_leader_ftd.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_local.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_local.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_notifier.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_notifier.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_publisher.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_publisher.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_service.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_service.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_tlvs.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_tlvs.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_data_types.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_data_types.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_network_diagnostic.obj: OPENTHREAD_MAKE_PATH/src/core/thread/network_diagnostic.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_panid_query_server.obj: OPENTHREAD_MAKE_PATH/src/core/thread/panid_query_server.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_radio_selector.obj: OPENTHREAD_MAKE_PATH/src/core/thread/radio_selector.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_router_table.obj: OPENTHREAD_MAKE_PATH/src/core/thread/router_table.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_src_match_controller.obj: OPENTHREAD_MAKE_PATH/src/core/thread/src_match_controller.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_thread_netif.obj: OPENTHREAD_MAKE_PATH/src/core/thread/thread_netif.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_time_sync_service.obj: OPENTHREAD_MAKE_PATH/src/core/thread/time_sync_service.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_tmf.obj: OPENTHREAD_MAKE_PATH/src/core/thread/tmf.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_topology.obj: OPENTHREAD_MAKE_PATH/src/core/thread/topology.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_uri_paths.obj: OPENTHREAD_MAKE_PATH/src/core/thread/uri_paths.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_channel_manager.obj: OPENTHREAD_MAKE_PATH/src/core/utils/channel_manager.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_channel_monitor.obj: OPENTHREAD_MAKE_PATH/src/core/utils/channel_monitor.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_child_supervision.obj: OPENTHREAD_MAKE_PATH/src/core/utils/child_supervision.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_flash.obj: OPENTHREAD_MAKE_PATH/src/core/utils/flash.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_heap.obj: OPENTHREAD_MAKE_PATH/src/core/utils/heap.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_history_tracker.obj: OPENTHREAD_MAKE_PATH/src/core/utils/history_tracker.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_jam_detector.obj: OPENTHREAD_MAKE_PATH/src/core/utils/jam_detector.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_otns.obj: OPENTHREAD_MAKE_PATH/src/core/utils/otns.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_parse_cmdline.obj: OPENTHREAD_MAKE_PATH/src/core/utils/parse_cmdline.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_ping_sender.obj: OPENTHREAD_MAKE_PATH/src/core/utils/ping_sender.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_slaac_address.obj: OPENTHREAD_MAKE_PATH/src/core/utils/slaac_address.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_srp_client_buffers.obj: OPENTHREAD_MAKE_PATH/src/core/utils/srp_client_buffers.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

$(NAME).a: $(OBJECTS)
	@ echo creating library $@
	@ $(AR) r $@ $^

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).a > $(DEVNULL) 2>&1
