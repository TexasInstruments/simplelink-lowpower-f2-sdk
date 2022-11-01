COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

CC = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"
LNK = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"

SYSCFG_CMD_STUB = $(SYSCONFIG_TOOL) --compiler ticlang --product $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_GEN_FILES := $(shell $(SYSCFG_CMD_STUB) --listGeneratedFiles --output . ../../freertos/ti_wisunfan_coap_node.syscfg)
SYSCFG_REF_FILES := $(shell $(SYSCFG_CMD_STUB) --listReferencedFiles --output . ../../freertos/ti_wisunfan_coap_node.syscfg)

ifeq ($(OS),Windows_NT)
	SYSCFG_FILES = $(subst \,/,$(SYSCFG_REF_FILES) $(SYSCFG_GEN_FILES))
else
	SYSCFG_FILES = $(SYSCFG_REF_FILES) $(SYSCFG_GEN_FILES)
endif
SYSCFG_C_FILES = $(filter %.c,$(SYSCFG_FILES))
SYSCFG_H_FILES = $(filter %.h,$(SYSCFG_FILES))

OBJECTS = application_main.obj application_application.obj mbed_ti-rf-driver_source_RFQueue.obj mbed_ti-nv-driver_crc.obj mbed_ti-nv-driver_nvocmp.obj mbed_frameworks_mbed-coap_source_sn_coap_builder.obj mbed_frameworks_mbed-coap_source_sn_coap_header_check.obj mbed_frameworks_mbed-coap_source_sn_coap_parser.obj mbed_frameworks_mbed-coap_source_sn_coap_protocol.obj mbed_nanostack_coap-service_source_coap_connection_handler.obj mbed_nanostack_coap-service_source_coap_message_handler.obj mbed_nanostack_coap-service_source_coap_security_handler.obj mbed_nanostack_coap-service_source_coap_service_api.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_interrupt.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_random.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_timer.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_event_loop.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_event_loop_mutex.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_hal_init.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_fhss_timer.obj mbed_mbednanostack2tirtos_platform_ns_trace.obj software_stack_ti15_4mac_mac_settings_mac_cfg.obj software_stack_ti15_4mac_mac_settings_mac_settings.obj software_stack_ti15_4mac_radio_configuration_mac_user_config.obj software_stack_ti15_4mac_macTask.obj software_stack_ti15_4mac_stack_user_api_api_mac_api_mac.obj software_stack_ti15_4mac_stack_user_api_api_mac_timac_fhss_config.obj software_stack_ti15_4mac_osal_port_osal_port.obj software_stack_ti15_4mac_osal_port_osal_port_timers.obj mbed_nanostack_mbed-mesh-api_source_wisun_tasklet.obj mbed_mbedtls2tirtos_platform_mbed_trng_alt.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_platform_util.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_threading.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_crypto_device_platform.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_aes_alt.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_crypto_misc.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_ecjpake_alt.obj mbed_mbedtls2tirtos_mbedtls_mbed-crypto_sha256_alt.obj application_shell_ns_cmdline.obj mbed_frameworks_mbed-client-randlib_source_randLIB.obj mbed_frameworks_nanostack-libservice_source_IPv6_fcf_lib_ip_fsc.obj mbed_frameworks_nanostack-libservice_source_libBits_common_functions.obj mbed_frameworks_nanostack-libservice_source_libList_ns_list.obj mbed_frameworks_nanostack-libservice_source_libip6string_ip6tos.obj mbed_frameworks_nanostack-libservice_source_libip6string_stoip6.obj mbed_frameworks_nanostack-libservice_source_nsdynmemLIB_nsdynmemLIB.obj mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto.obj mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto_se.obj mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto_slot_management.obj mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto_storage.obj mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_its_file.obj mbed_mbedtls_src_aes.obj mbed_mbedtls_src_aesni.obj mbed_mbedtls_src_arc4.obj mbed_mbedtls_src_aria.obj mbed_mbedtls_src_asn1parse.obj mbed_mbedtls_src_asn1write.obj mbed_mbedtls_src_base64.obj mbed_mbedtls_src_bignum.obj mbed_mbedtls_src_blowfish.obj mbed_mbedtls_src_camellia.obj mbed_mbedtls_src_ccm.obj mbed_mbedtls_src_chacha20.obj mbed_mbedtls_src_chachapoly.obj mbed_mbedtls_src_cipher.obj mbed_mbedtls_src_cipher_wrap.obj mbed_mbedtls_src_cmac.obj mbed_mbedtls_src_ctr_drbg.obj mbed_mbedtls_src_des.obj mbed_mbedtls_src_dhm.obj mbed_mbedtls_src_ecdh.obj mbed_mbedtls_src_ecdsa.obj mbed_mbedtls_src_ecjpake.obj mbed_mbedtls_src_ecp.obj mbed_mbedtls_src_ecp_curves.obj mbed_mbedtls_src_entropy.obj mbed_mbedtls_src_entropy_poll.obj mbed_mbedtls_src_gcm.obj mbed_mbedtls_src_havege.obj mbed_mbedtls_src_hkdf.obj mbed_mbedtls_src_hmac_drbg.obj mbed_mbedtls_src_md.obj mbed_mbedtls_src_md2.obj mbed_mbedtls_src_md4.obj mbed_mbedtls_src_md5.obj mbed_mbedtls_src_memory_buffer_alloc.obj mbed_mbedtls_src_nist_kw.obj mbed_mbedtls_src_oid.obj mbed_mbedtls_src_padlock.obj mbed_mbedtls_src_pem.obj mbed_mbedtls_src_pk.obj mbed_mbedtls_src_pk_wrap.obj mbed_mbedtls_src_pkcs12.obj mbed_mbedtls_src_pkcs5.obj mbed_mbedtls_src_pkparse.obj mbed_mbedtls_src_pkwrite.obj mbed_mbedtls_src_platform.obj mbed_mbedtls_src_poly1305.obj mbed_mbedtls_src_ripemd160.obj mbed_mbedtls_src_rsa.obj mbed_mbedtls_src_rsa_internal.obj mbed_mbedtls_src_sha1.obj mbed_mbedtls_src_sha256.obj mbed_mbedtls_src_sha512.obj mbed_mbedtls_src_timing.obj mbed_mbedtls_src_xtea.obj mbed_mbedtls_src_certs.obj mbed_mbedtls_src_debug.obj mbed_mbedtls_src_error.obj mbed_mbedtls_src_pkcs11.obj mbed_mbedtls_src_ssl_cache.obj mbed_mbedtls_src_ssl_ciphersuites.obj mbed_mbedtls_src_ssl_cli.obj mbed_mbedtls_src_ssl_cookie.obj mbed_mbedtls_src_ssl_srv.obj mbed_mbedtls_src_ssl_msg.obj mbed_mbedtls_src_ssl_ticket.obj mbed_mbedtls_src_ssl_tls.obj mbed_mbedtls_src_version.obj mbed_mbedtls_src_version_features.obj mbed_mbedtls_src_x509.obj mbed_mbedtls_src_x509_create.obj mbed_mbedtls_src_x509_crl.obj mbed_mbedtls_src_x509_crt.obj mbed_mbedtls_src_x509_csr.obj mbed_mbedtls_src_x509write_crt.obj mbed_mbedtls_src_x509write_csr.obj mbed_nanostack_mbed-mesh-api_source_ethernet_tasklet.obj mbed_nanostack_mbed-mesh-api_source_mesh_system.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_network_lib.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_protocol_6lowpan.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_protocol_6lowpan_bootstrap.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_protocol_6lowpan_interface.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Fragmentation_cipv6_fragmenter.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_6lowpan_iphc.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_iphc_compress.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_iphc_decompress.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_lowpan_context.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_beacon_handler.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_data_poll.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_helper.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_ie_lib.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_pairwise_key.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Mesh_mesh.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ND_nd_router_object.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_NVM_nwk_nvm.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_adaptation_interface.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_cfg_settings.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_bootstrap.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_common.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_auth_relay.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_pdu.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_relay.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_relay_lib.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_empty_functions.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_ie_lib.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_llc_data_service.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_management_api.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_mpx_header.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_neighbor_class.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_key_storage.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_time.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_auth.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_controller.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_lib.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_nvm_data.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_nvm_store.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_supp.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_timers.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_stats.obj mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_test_api.obj mbed_nanostack_sal-stack-nanostack_source_BorderRouter_border_router.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_icmpv6.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_icmpv6_prefix.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_icmpv6_radv.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6_flow.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6_fragmentation.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6_resolution.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_mld.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_tcp.obj mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_udp.obj mbed_nanostack_sal-stack-nanostack_source_Core_buffer_dyn.obj mbed_nanostack_sal-stack-nanostack_source_Core_ns_address_internal.obj mbed_nanostack_sal-stack-nanostack_source_Core_ns_monitor.obj mbed_nanostack_sal-stack-nanostack_source_Core_ns_socket.obj mbed_nanostack_sal-stack-nanostack_source_Core_sockbuf.obj mbed_nanostack_sal-stack-nanostack_source_DHCPv6_Server_DHCPv6_Server_service.obj mbed_nanostack_sal-stack-nanostack_source_DHCPv6_client_dhcpv6_client_service.obj mbed_nanostack_sal-stack-nanostack_source_MAC_rf_driver_storage.obj mbed_nanostack_sal-stack-nanostack_source_MLE_mle.obj mbed_nanostack_sal-stack-nanostack_source_MLE_mle_tlv.obj mbed_nanostack_sal-stack-nanostack_source_MPL_mpl.obj mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_core.obj mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_core_sleep.obj mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_stats.obj mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_timer.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_control.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_data.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_downward.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_mrhof.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_objective.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_of0.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_policy.obj mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_upward.obj mbed_nanostack_sal-stack-nanostack_source_Security_Common_security_lib.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_eap_protocol.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_avp.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_client.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_eap_header.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_header.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_relay_table.obj mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_server.obj mbed_nanostack_sal-stack-nanostack_source_Security_TLS_tls_ccm_crypt.obj mbed_nanostack_sal-stack-nanostack_source_Security_TLS_tls_lib.obj mbed_nanostack_sal-stack-nanostack_source_Security_eapol_eapol_helper.obj mbed_nanostack_sal-stack-nanostack_source_Security_eapol_kde_helper.obj mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_addr.obj mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_api.obj mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_eapol_pdu_if.obj mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_socket_if.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_radius_sec_prot_avp_helper.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_radius_sec_prot_radius_client_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_radius_eap_tls_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_auth_eap_tls_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_eap_tls_sec_prot_lib.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_supp_eap_tls_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_fwh_sec_prot_auth_fwh_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_fwh_sec_prot_supp_fwh_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_gkh_sec_prot_auth_gkh_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_gkh_sec_prot_supp_gkh_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_key_sec_prot_key_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_sec_prot_certs.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_sec_prot_keys.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_sec_prot_lib.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_tls_sec_prot_tls_sec_prot.obj mbed_nanostack_sal-stack-nanostack_source_Security_protocols_tls_sec_prot_tls_sec_prot_lib.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_CCM_lib_ccm_security.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_CCM_lib_mbedOS_aes_mbedtls.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_CCM_lib_mbedOS_aes_mbedtls_adapter.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_Neighbor_cache_neighbor_cache.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_SHA256_Lib_ns_sha256.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_SHA256_Lib_shalib.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_Trickle_trickle.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_blacklist_blacklist.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_etx_etx.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_fhss_channel_list.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_fnv_hash_fnv_hash.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_hmac_hmac_md.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_ieee_802_11_ieee_802_11.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_load_balance_load_balance.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mac_neighbor_table_mac_neighbor_table.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_fnet_fnet_stack_services_mdns_fnet_mdns.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_fnet_fnet_stack_services_poll_fnet_poll.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_fnet_fnet_stack_stack_fnet_stdlib.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_ns_fnet_events.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_ns_fnet_port.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_ns_mdns_api.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_buffer.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_frame_counter_table.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_interface.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_security.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_nd_proxy_nd_proxy.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_nist_aes_kw_nist_aes_kw.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_pan_blacklist_pan_blacklist.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_isqrt.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_ns_conf.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_ns_crc.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_ns_file_system.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_whiteboard_whiteboard.obj mbed_nanostack_sal-stack-nanostack_source_Service_Libs_random_early_detection_random_early_detection.obj mbed_frameworks_nanostack-libservice_source_nsdynmemtracker_nsdynmem_tracker_lib.obj mbed_nanostack_sal-stack-nanostack_source_ipv6_stack_ipv6_routing_table.obj mbed_nanostack_sal-stack-nanostack_source_ipv6_stack_protocol_ipv6.obj mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_dhcp_service_api.obj mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_libDHCPv6.obj mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_libDHCPv6_server.obj mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_libDHCPv6_vendordata.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_multicast_api.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_dns.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_6lowpan_parameter_api.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_ipv6.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_load_balance.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_mle.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_rpl.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_short_address_extension.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_test.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_ns_net.obj mbed_nanostack_sal-stack-nanostack_source_libNET_src_socket_api.obj mbed_nanostack_sal-stack-nanostack-eventloop_source_event.obj mbed_nanostack_sal-stack-nanostack-eventloop_source_ns_timeout.obj mbed_nanostack_sal-stack-nanostack-eventloop_source_ns_timer.obj mbed_nanostack_sal-stack-nanostack-eventloop_source_system_timer.obj mbed_mbed-trace_mbed_trace.obj wisun_ncp_src_ncp_changed_props_set.obj wisun_ncp_src_ncp_ncp_base.obj wisun_ncp_src_ncp_ncp_base_dispatcher.obj wisun_ncp_src_ncp_ncp_uart.obj wisun_ncp_src_lib_hdlc_hdlc.obj wisun_ncp_src_lib_spinel_spinel.obj wisun_ncp_src_lib_spinel_spinel_buffer.obj wisun_ncp_src_lib_spinel_spinel_decoder.obj wisun_ncp_src_lib_spinel_spinel_encoder.obj wisun_ncp_src_core_api_message_api.obj wisun_ncp_src_core_api_logging_api.obj wisun_ncp_src_core_api_instance_api.obj wisun_ncp_src_core_common_crc16.obj wisun_ncp_src_core_common_instance.obj wisun_ncp_src_core_common_logging.obj wisun_ncp_src_core_common_message.obj wisun_ncp_src_core_common_notifier.obj wisun_ncp_src_core_common_string.obj wisun_ncp_src_core_common_tasklet.obj wisun_ncp_src_core_common_timer.obj wisun_ncp_src_core_common_tlvs.obj wisun_ncp_platform_misc.obj $(patsubst %.c,%.obj,$(notdir $(SYSCFG_C_FILES)))
NAME = ns_coap_node_src

CFLAGS = -I../.. \
    -I. \
    @${PROJECT_LOC}/application/defines/router.opts \
    -Oz \
    -std=gnu9x \
    -munaligned-access \
    -fshort-enums \
    -fcommon \
    -funsigned-char \
    -mlittle-endian \
    -fdiagnostics-show-option \
    -ffunction-sections \
    -g \
    -I${PROJECT_ROOT} \
    -fno-strict-aliasing \
    -DCOAP_SERVICE_ENABLE \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/low_level/cc13xx/combo \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/ti-rf-driver/ti-rf-driver \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/apps/udp_nanostack_mesh_tirf \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/apps/common/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/common/nv \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_config/ws_router \
    -include mbed_config.h \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/ti-rf-driver/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/nanostack-libservice/mbed-client-libservice \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/nanostack-libservice \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/nanostack-libservice/mbed-client-libservice/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-client-randlib/mbed-client-randlib/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-client-randlib/mbed-client-randlib \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-client-randlib \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-trace \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-coap \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-coap/source/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/nanostack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/stack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/services/serial \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LowPAN \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/nanostack-event-loop \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/nanostack-event-loop/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/nanostack-hal-mbed-cmsis-rtos \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api/source/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api/mbed-mesh-api \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/coap-service/coap-service \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/coap-service/source/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/platform/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/COMPONENT_NSPE \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/shell \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/conf \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/pltfrm/sysbios \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/net \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/services \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/assert \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/appasrt \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/crypto \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/rf \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/high_level \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/low_level \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/osal_port \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/boards \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/util \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/stack/src \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/stack/tirtos/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/heapmgr \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/services/saddr \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/services/sdata \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/fh \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/rom \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/inc/cc13xx \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/low_level/cc13xx \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/tracer \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Thread \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/services/mdns \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/fnv_hash \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/hmac \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/load_balance \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/whiteboard \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/DHCPv6_client \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/libDHCPv6 \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/utils \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/pan_blacklist \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols \
    -DEXCLUDE_TRACE \
    -DxSWITCH_NCP_TO_TRACE \
    -DTLS_SEC_PROT_LIB_USE_MBEDTLS_PLATFORM_MEMORY \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/posix/ticlang \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/core \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/core/utils \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/platforms \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/core/thread \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/ncp \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd/platform/nv \
    -DxWISUN_AUTO_START \
    -DTI_WISUN_FAN_DEBUG \
    -DBoard_EXCLUDE_NVS_EXTERNAL_FLASH \
    -DDeviceFamily_CC13X4 \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4/inc" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4/driverlib" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/ticlang" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/freertos" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/freertos/builds/cc13x4_cc26x4/release/pregenerated_configuration" \
    -DFREERTOS_SUPPORT \
    -DFREERTOS \
    -DNVOCMP_POSIX_MUTEX \
    -I${FREERTOS_INSTALL_DIR}/FreeRTOS/Source/include \
    -I${FREERTOS_INSTALL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM33/non_secure \
    -gdwarf-3 \
    -mcpu=cortex-m33 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16

CPPFLAGS = -I../.. \
    -I. \
    @${PROJECT_LOC}/application/defines/router.opts \
    -Oz \
    -std=gnu9x \
    -munaligned-access \
    -fshort-enums \
    -fcommon \
    -funsigned-char \
    -mlittle-endian \
    -fdiagnostics-show-option \
    -ffunction-sections \
    -g \
    -I${PROJECT_ROOT} \
    -fno-strict-aliasing \
    -DCOAP_SERVICE_ENABLE \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/low_level/cc13xx/combo \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/ti-rf-driver/ti-rf-driver \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/apps/udp_nanostack_mesh_tirf \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/apps/common/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/common/nv \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_config/ws_router \
    -include mbed_config.h \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/ti-rf-driver/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/nanostack-libservice/mbed-client-libservice \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/nanostack-libservice \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/nanostack-libservice/mbed-client-libservice/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-client-randlib/mbed-client-randlib/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-client-randlib/mbed-client-randlib \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-client-randlib \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-trace \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-coap \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/frameworks/mbed-coap/source/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/nanostack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/stack \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/services/serial \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LowPAN \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/nanostack-event-loop \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/nanostack-event-loop/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/nanostack-hal-mbed-cmsis-rtos \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api/source/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api/mbed-mesh-api \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/coap-service/coap-service \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/coap-service/source/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/platform/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/COMPONENT_NSPE \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/shell \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/conf \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/pltfrm/sysbios \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/uip/net \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/services \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/assert \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/appasrt \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/crypto \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/hal/rf \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/high_level \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/low_level \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/osal_port \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/boards \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/util \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/stack/src \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/stack/tirtos/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/common/heapmgr \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/services/saddr \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/services/sdata \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/fh \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/inc \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/rom \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/inc/cc13xx \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/low_level/cc13xx \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/wisunfan_mac/tracer \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/mbed-mesh-api/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Thread \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/services/mdns \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/fnv_hash \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/hmac \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/load_balance \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/source \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/whiteboard \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/DHCPv6_client \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/libDHCPv6 \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/utils \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/pan_blacklist \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols \
    -DEXCLUDE_TRACE \
    -DxSWITCH_NCP_TO_TRACE \
    -DTLS_SEC_PROT_LIB_USE_MBEDTLS_PLATFORM_MEMORY \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/posix/ticlang \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/include \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/core \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/core/utils \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/platforms \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd/platform \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/core/thread \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/src/ncp \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd/platform/nv \
    -DxWISUN_AUTO_START \
    -DTI_WISUN_FAN_DEBUG \
    -DBoard_EXCLUDE_NVS_EXTERNAL_FLASH \
    -DDeviceFamily_CC13X4 \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4/inc" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4/driverlib" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/ticlang" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/freertos" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/freertos/builds/cc13x4_cc26x4/release/pregenerated_configuration" \
    -DFREERTOS_SUPPORT \
    -DFREERTOS \
    -DNVOCMP_POSIX_MUTEX \
    -I${FREERTOS_INSTALL_DIR}/FreeRTOS/Source/include \
    -I${FREERTOS_INSTALL_DIR}/FreeRTOS/Source/portable/GCC/ARM_CM33/non_secure \
    -gdwarf-3 \
    -mcpu=cortex-m33 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16

LFLAGS = "-i$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-i$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel" \
     --diag_suppress=10063 --define=FREERTOS \
    ti_utils_build_linker.cmd.genlibs \
    -lti/devices/cc13x4_cc26x4/driverlib/bin/ticlang/driverlib.lib \
    ../../cc13x4_cc26x4.cmd \
    "-Wl,-m,$(NAME).map" \
    -Wl,--rom_model \
    -Wl,--warn_sections \
    "-L$(TICLANG_ARMCOMPILER)/lib" \
    -llibc.a
all: $(NAME).out

.INTERMEDIATE: syscfg
$(SYSCFG_FILES): syscfg
	@ echo generation complete

syscfg: ../../freertos/ti_wisunfan_coap_node.syscfg
	@ echo Generating configuration files...
	@ $(SYSCFG_CMD_STUB) --output $(@D) $<

define C_RULE
$(basename $(notdir $(1))).obj: $(1) $(SYSCFG_H_FILES)
	@ echo Building $$@
	@ $(CC) $(CFLAGS) -c $$< -o $$@
endef

$(foreach c_file,$(SYSCFG_C_FILES),$(eval $(call C_RULE,$(c_file))))

application_main.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/udp_nanostack_mesh_tirf/main.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

application_application.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/udp_nanostack_mesh_tirf/application.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_ti-rf-driver_source_RFQueue.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/ti-rf-driver/source/RFQueue.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_ti-nv-driver_crc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/common/nv/crc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_ti-nv-driver_nvocmp.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/common/nv/nvocmp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_mbed-coap_source_sn_coap_builder.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/mbed-coap/source/sn_coap_builder.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_mbed-coap_source_sn_coap_header_check.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/mbed-coap/source/sn_coap_header_check.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_mbed-coap_source_sn_coap_parser.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/mbed-coap/source/sn_coap_parser.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_mbed-coap_source_sn_coap_protocol.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/mbed-coap/source/sn_coap_protocol.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_coap-service_source_coap_connection_handler.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/coap-service/source/coap_connection_handler.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_coap-service_source_coap_message_handler.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/coap-service/source/coap_message_handler.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_coap-service_source_coap_security_handler.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/coap-service/source/coap_security_handler.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_coap-service_source_coap_service_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/coap-service/source/coap_service_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_interrupt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/arm_hal_interrupt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_random.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/arm_hal_random.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_timer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/arm_hal_timer.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_event_loop.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/ns_event_loop.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_event_loop_mutex.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/ns_event_loop_mutex.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_hal_init.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/ns_hal_init.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_fhss_timer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/nanostack/nanostack-hal-mbed-cmsis-rtos/arm_hal_fhss_timer.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbednanostack2tirtos_platform_ns_trace.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbednanostack2tirtos/platform/ns_trace.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_mac_settings_mac_cfg.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/high_level/mac_cfg.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_mac_settings_mac_settings.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/low_level/cc13xx/mac_settings.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_radio_configuration_mac_user_config.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/common/boards/mac_user_config.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_macTask.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/common/osal_port/macTask.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_stack_user_api_api_mac_api_mac.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/common/osal_port/api_mac.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_stack_user_api_api_mac_timac_fhss_config.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/common/osal_port/timac_fhss_config.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_osal_port_osal_port.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/common/osal_port/osal_port_tirtos/osal_port.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

software_stack_ti15_4mac_osal_port_osal_port_timers.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/wisunfan_mac/common/osal_port/osal_port_tirtos/osal_port_timers.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_mbed-mesh-api_source_wisun_tasklet.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/mbed-mesh-api/source/wisun_tasklet.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_platform_mbed_trng_alt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/platform/mbed_trng_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_platform_util.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/platform_util.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_threading.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/threading.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_crypto_device_platform.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto/src/crypto_device_platform.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_aes_alt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto/src/aes_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_crypto_misc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto/src/crypto_misc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_ecjpake_alt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto/src/ecjpake_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls2tirtos_mbedtls_mbed-crypto_sha256_alt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/mbed_port/mbedtls2tirtos/mbedtls/mbed-crypto/src/sha256_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

application_shell_ns_cmdline.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/shell/ns_cmdline.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_mbed-client-randlib_source_randLIB.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/mbed-client-randlib/source/randLIB.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_IPv6_fcf_lib_ip_fsc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/IPv6_fcf_lib/ip_fsc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_libBits_common_functions.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/libBits/common_functions.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_libList_ns_list.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/libList/ns_list.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_libip6string_ip6tos.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/libip6string/ip6tos.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_libip6string_stoip6.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/libip6string/stoip6.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_nsdynmemLIB_nsdynmemLIB.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/nsdynmemLIB/nsdynmemLIB.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/psa_crypto.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto_se.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/psa_crypto_se.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto_slot_management.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/psa_crypto_slot_management.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_crypto_storage.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/psa_crypto_storage.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_platform_COMPONENT_PSA_SRV_IMPL_psa_its_file.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/platform/COMPONENT_PSA_SRV_IMPL/psa_its_file.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_aes.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/aes.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_aesni.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/aesni.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_arc4.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/arc4.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_aria.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/aria.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_asn1parse.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/asn1parse.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_asn1write.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/asn1write.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_base64.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/base64.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_bignum.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/bignum.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_blowfish.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/blowfish.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_camellia.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/camellia.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ccm.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ccm.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_chacha20.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/chacha20.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_chachapoly.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/chachapoly.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_cipher.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/cipher.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_cipher_wrap.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/cipher_wrap.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_cmac.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/cmac.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ctr_drbg.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ctr_drbg.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_des.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/des.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_dhm.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/dhm.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ecdh.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ecdh.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ecdsa.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ecdsa.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ecjpake.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ecjpake.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ecp.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ecp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ecp_curves.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ecp_curves.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_entropy.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/entropy.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_entropy_poll.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/entropy_poll.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_gcm.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/gcm.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_havege.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/havege.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_hkdf.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/hkdf.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_hmac_drbg.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/hmac_drbg.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_md.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/md.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_md2.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/md2.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_md4.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/md4.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_md5.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/md5.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_memory_buffer_alloc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/memory_buffer_alloc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_nist_kw.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/nist_kw.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_oid.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/oid.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_padlock.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/padlock.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pem.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pem.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pk.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pk.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pk_wrap.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pk_wrap.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pkcs12.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pkcs12.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pkcs5.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pkcs5.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pkparse.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pkparse.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pkwrite.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pkwrite.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_platform.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/platform.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_poly1305.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/poly1305.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ripemd160.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ripemd160.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_rsa.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/rsa.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_rsa_internal.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/rsa_internal.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_sha1.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/sha1.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_sha256.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/sha256.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_sha512.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/sha512.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_timing.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/timing.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_xtea.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/xtea.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_certs.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/certs.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_debug.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/debug.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_error.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/error.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_pkcs11.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/pkcs11.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_cache.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_cache.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_ciphersuites.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_ciphersuites.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_cli.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_cli.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_cookie.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_cookie.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_srv.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_srv.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_msg.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_msg.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_ticket.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_ticket.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_ssl_tls.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/ssl_tls.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_version.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/version.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_version_features.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/version_features.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509_create.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509_create.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509_crl.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509_crl.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509_crt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509_crt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509_csr.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509_csr.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509write_crt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509write_crt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbedtls_src_x509write_csr.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/ti_wisunfan/mbedtls/src/x509write_csr.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_mbed-mesh-api_source_ethernet_tasklet.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/mbed-mesh-api/source/ethernet_tasklet.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_mbed-mesh-api_source_mesh_system.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/mbed-mesh-api/source/mesh_system.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_network_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Bootstraps/Generic/network_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_protocol_6lowpan.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Bootstraps/Generic/protocol_6lowpan.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_protocol_6lowpan_bootstrap.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Bootstraps/Generic/protocol_6lowpan_bootstrap.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Bootstraps_Generic_protocol_6lowpan_interface.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Bootstraps/Generic/protocol_6lowpan_interface.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Fragmentation_cipv6_fragmenter.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Fragmentation/cipv6_fragmenter.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_6lowpan_iphc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/IPHC_Decode/6lowpan_iphc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_iphc_compress.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/IPHC_Decode/iphc_compress.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_iphc_decompress.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/IPHC_Decode/iphc_decompress.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_IPHC_Decode_lowpan_context.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/IPHC_Decode/lowpan_context.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_beacon_handler.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC/beacon_handler.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_data_poll.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC/mac_data_poll.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_helper.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC/mac_helper.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_ie_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC/mac_ie_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_MAC_mac_pairwise_key.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/MAC/mac_pairwise_key.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_Mesh_mesh.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/Mesh/mesh.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ND_nd_router_object.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ND/nd_router_object.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_NVM_nwk_nvm.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/NVM/nwk_nvm.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_adaptation_interface.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/adaptation_interface.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_cfg_settings.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_cfg_settings.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_bootstrap.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_bootstrap.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_common.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_common.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_auth_relay.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_eapol_auth_relay.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_pdu.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_eapol_pdu.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_relay.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_eapol_relay.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_eapol_relay_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_eapol_relay_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_empty_functions.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_empty_functions.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_ie_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_ie_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_llc_data_service.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_llc_data_service.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_management_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_management_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_mpx_header.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_mpx_header.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_neighbor_class.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_neighbor_class.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_key_storage.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_key_storage.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_time.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_time.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_auth.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_auth.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_controller.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_controller.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_nvm_data.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_nvm_data.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_nvm_store.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_nvm_store.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_supp.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_supp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_pae_timers.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_pae_timers.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_stats.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_stats.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_6LoWPAN_ws_ws_test_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/6LoWPAN/ws/ws_test_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_BorderRouter_border_router.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/BorderRouter/border_router.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_icmpv6.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/icmpv6.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_icmpv6_prefix.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/icmpv6_prefix.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_icmpv6_radv.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/icmpv6_radv.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/ipv6.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6_flow.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/ipv6_flow.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6_fragmentation.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/ipv6_fragmentation.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_ipv6_resolution.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/ipv6_resolution.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_mld.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/mld.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_tcp.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/tcp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Common_Protocols_udp.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Common_Protocols/udp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Core_buffer_dyn.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/buffer_dyn.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Core_ns_address_internal.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/ns_address_internal.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Core_ns_monitor.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/ns_monitor.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Core_ns_socket.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/ns_socket.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Core_sockbuf.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Core/sockbuf.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_DHCPv6_Server_DHCPv6_Server_service.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/DHCPv6_Server/DHCPv6_Server_service.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_DHCPv6_client_dhcpv6_client_service.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/DHCPv6_client/dhcpv6_client_service.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_MAC_rf_driver_storage.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/MAC/rf_driver_storage.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_MLE_mle.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/MLE/mle.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_MLE_mle_tlv.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/MLE/mle_tlv.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_MPL_mpl.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/MPL/mpl.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_core.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/NWK_INTERFACE/protocol_core.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_core_sleep.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/NWK_INTERFACE/protocol_core_sleep.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_stats.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/NWK_INTERFACE/protocol_stats.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_NWK_INTERFACE_protocol_timer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/NWK_INTERFACE/protocol_timer.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_control.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_control.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_data.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_data.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_downward.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_downward.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_mrhof.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_mrhof.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_objective.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_objective.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_of0.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_of0.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_policy.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_policy.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_RPL_rpl_upward.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/RPL/rpl_upward.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_Common_security_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/Common/security_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_eap_protocol.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/eap_protocol.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_avp.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana_avp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_client.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana_client.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_eap_header.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana_eap_header.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_header.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana_header.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_relay_table.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana_relay_table.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_PANA_pana_server.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/PANA/pana_server.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_TLS_tls_ccm_crypt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/TLS/tls_ccm_crypt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_TLS_tls_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/TLS/tls_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_eapol_eapol_helper.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/eapol/eapol_helper.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_eapol_kde_helper.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/eapol/kde_helper.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_addr.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/kmp/kmp_addr.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/kmp/kmp_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_eapol_pdu_if.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/kmp/kmp_eapol_pdu_if.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_kmp_kmp_socket_if.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/kmp/kmp_socket_if.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_radius_sec_prot_avp_helper.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/radius_sec_prot/avp_helper.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_radius_sec_prot_radius_client_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/radius_sec_prot/radius_client_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_radius_eap_tls_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/eap_tls_sec_prot/radius_eap_tls_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_auth_eap_tls_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/eap_tls_sec_prot/auth_eap_tls_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_eap_tls_sec_prot_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/eap_tls_sec_prot/eap_tls_sec_prot_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_eap_tls_sec_prot_supp_eap_tls_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/eap_tls_sec_prot/supp_eap_tls_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_fwh_sec_prot_auth_fwh_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/fwh_sec_prot/auth_fwh_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_fwh_sec_prot_supp_fwh_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/fwh_sec_prot/supp_fwh_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_gkh_sec_prot_auth_gkh_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/gkh_sec_prot/auth_gkh_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_gkh_sec_prot_supp_gkh_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/gkh_sec_prot/supp_gkh_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_key_sec_prot_key_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/key_sec_prot/key_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_sec_prot_certs.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/sec_prot_certs.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_sec_prot_keys.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/sec_prot_keys.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_sec_prot_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/sec_prot_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_tls_sec_prot_tls_sec_prot.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/tls_sec_prot/tls_sec_prot.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Security_protocols_tls_sec_prot_tls_sec_prot_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Security/protocols/tls_sec_prot/tls_sec_prot_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_CCM_lib_ccm_security.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/CCM_lib/ccm_security.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_CCM_lib_mbedOS_aes_mbedtls.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/CCM_lib/mbedOS/aes_mbedtls.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_CCM_lib_mbedOS_aes_mbedtls_adapter.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/CCM_lib/mbedOS/aes_mbedtls_adapter.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_Neighbor_cache_neighbor_cache.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/Neighbor_cache/neighbor_cache.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_SHA256_Lib_ns_sha256.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/SHA256_Lib/ns_sha256.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_SHA256_Lib_shalib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/SHA256_Lib/shalib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_Trickle_trickle.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/Trickle/trickle.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_blacklist_blacklist.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/blacklist/blacklist.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_etx_etx.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/etx/etx.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_fhss_channel_list.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/fhss/channel_list.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_fnv_hash_fnv_hash.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/fnv_hash/fnv_hash.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_hmac_hmac_md.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/hmac/hmac_md.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_ieee_802_11_ieee_802_11.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/ieee_802_11/ieee_802_11.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_load_balance_load_balance.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/load_balance/load_balance.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mac_neighbor_table_mac_neighbor_table.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mac_neighbor_table/mac_neighbor_table.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_fnet_fnet_stack_services_mdns_fnet_mdns.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/services/mdns/fnet_mdns.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_fnet_fnet_stack_services_poll_fnet_poll.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/services/poll/fnet_poll.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_fnet_fnet_stack_stack_fnet_stdlib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/fnet/fnet_stack/stack/fnet_stdlib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_ns_fnet_events.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/ns_fnet_events.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_ns_fnet_port.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/ns_fnet_port.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mdns_ns_mdns_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mdns/ns_mdns_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mle_service/mle_service.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_buffer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mle_service/mle_service_buffer.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_frame_counter_table.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mle_service/mle_service_frame_counter_table.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_interface.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mle_service/mle_service_interface.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_mle_service_mle_service_security.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/mle_service/mle_service_security.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_nd_proxy_nd_proxy.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/nd_proxy/nd_proxy.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_nist_aes_kw_nist_aes_kw.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/nist_aes_kw/nist_aes_kw.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_pan_blacklist_pan_blacklist.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/pan_blacklist/pan_blacklist.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_isqrt.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/utils/isqrt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_ns_conf.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/utils/ns_conf.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_ns_crc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/utils/ns_crc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_utils_ns_file_system.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/utils/ns_file_system.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_whiteboard_whiteboard.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/whiteboard/whiteboard.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_Service_Libs_random_early_detection_random_early_detection.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/Service_Libs/random_early_detection/random_early_detection.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_frameworks_nanostack-libservice_source_nsdynmemtracker_nsdynmem_tracker_lib.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/nanostack-libservice/source/nsdynmemtracker/nsdynmem_tracker_lib.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_ipv6_stack_ipv6_routing_table.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/ipv6_stack/ipv6_routing_table.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_ipv6_stack_protocol_ipv6.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/ipv6_stack/protocol_ipv6.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_dhcp_service_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libDHCPv6/dhcp_service_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_libDHCPv6.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libDHCPv6/libDHCPv6.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_libDHCPv6_server.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libDHCPv6/libDHCPv6_server.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libDHCPv6_libDHCPv6_vendordata.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libDHCPv6/libDHCPv6_vendordata.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_multicast_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/multicast_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_dns.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_dns.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_6lowpan_parameter_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_6lowpan_parameter_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_ipv6.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_ipv6.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_load_balance.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_load_balance.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_mle.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_mle.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_rpl.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_rpl.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_short_address_extension.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_short_address_extension.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_net_test.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/net_test.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_ns_net.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/ns_net.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack_source_libNET_src_socket_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack/source/libNET/src/socket_api.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack-eventloop_source_event.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/source/event.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack-eventloop_source_ns_timeout.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/source/ns_timeout.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack-eventloop_source_ns_timer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/source/ns_timer.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_nanostack_sal-stack-nanostack-eventloop_source_system_timer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/nanostack/sal-stack-nanostack-eventloop/source/system_timer.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

mbed_mbed-trace_mbed_trace.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/third_party/mbed/frameworks/mbed-trace/source/mbed_trace.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

wisun_ncp_src_lib_spinel_spinel.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/lib/spinel/spinel.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

wisun_ncp_platform_misc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/examples/ncp_ftd/platform/misc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

wisun_ncp_src_ncp_changed_props_set.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/ncp/changed_props_set.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_ncp_ncp_base.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/ncp/ncp_base.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_ncp_ncp_base_dispatcher.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/ncp/ncp_base_dispatcher.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_ncp_ncp_uart.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/ncp/ncp_uart.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_lib_hdlc_hdlc.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/lib/hdlc/hdlc.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_lib_spinel_spinel_buffer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/lib/spinel/spinel_buffer.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_lib_spinel_spinel_decoder.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/lib/spinel/spinel_decoder.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_lib_spinel_spinel_encoder.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/lib/spinel/spinel_encoder.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_api_message_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/api/message_api.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_api_logging_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/api/logging_api.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_api_instance_api.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/api/instance_api.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_crc16.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/crc16.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_instance.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/instance.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_logging.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/logging.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_message.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/message.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_notifier.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/notifier.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_string.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/string.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_tasklet.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/tasklet.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_timer.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/timer.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

wisun_ncp_src_core_common_tlvs.obj: $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/ti_wisunfan/apps/../../../../source/ti/ti_wisunfan/ncp_interface/src/core/common/tlvs.cpp $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK) -Wl,-u,_c_int00 $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) $(SYSCFG_GEN_FILES)> $(DEVNULL) 2>&1
