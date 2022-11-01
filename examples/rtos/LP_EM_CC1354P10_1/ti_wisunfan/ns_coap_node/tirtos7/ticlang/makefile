PROJECT_LOC = $(abspath ../../)
PROJECT_ROOT = $(PROJECT_LOC)
COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

CC = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"
LNK = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"

SYSCFG_CMD_STUB = $(SYSCONFIG_TOOL) --compiler ticlang --product $(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_GEN_FILES := $(shell $(SYSCFG_CMD_STUB) --listGeneratedFiles --output . ../../tirtos7/ti_wisunfan_coap_node.syscfg)
SYSCFG_REF_FILES := $(shell $(SYSCFG_CMD_STUB) --listReferencedFiles --output . ../../tirtos7/ti_wisunfan_coap_node.syscfg)

ifeq ($(OS),Windows_NT)
	SYSCFG_FILES = $(subst \,/,$(SYSCFG_REF_FILES) $(SYSCFG_GEN_FILES))
else
	SYSCFG_FILES = $(SYSCFG_REF_FILES) $(SYSCFG_GEN_FILES)
endif
SYSCFG_C_FILES = $(filter %.c,$(SYSCFG_FILES))
SYSCFG_H_FILES = $(filter %.h,$(SYSCFG_FILES))

OBJECTS = application_main.obj application_application.obj mbed_ti-rf-driver_source_RFQueue.obj mbed_ti-nv-driver_crc.obj mbed_ti-nv-driver_nvocmp.obj mbed_frameworks_mbed-coap_source_sn_coap_builder.obj mbed_frameworks_mbed-coap_source_sn_coap_header_check.obj mbed_frameworks_mbed-coap_source_sn_coap_parser.obj mbed_frameworks_mbed-coap_source_sn_coap_protocol.obj mbed_nanostack_coap-service_source_coap_connection_handler.obj mbed_nanostack_coap-service_source_coap_message_handler.obj mbed_nanostack_coap-service_source_coap_security_handler.obj mbed_nanostack_coap-service_source_coap_service_api.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_interrupt.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_random.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_timer.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_event_loop.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_event_loop_mutex.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_ns_hal_init.obj mbed_mbednanostack2tirtos_nanostack_nanostack-hal-mbed-cmsis-rtos_arm_hal_fhss_timer.obj mbed_mbednanostack2tirtos_platform_ns_trace.obj software_stack_ti15_4mac_mac_settings_mac_cfg.obj software_stack_ti15_4mac_mac_settings_mac_settings.obj software_stack_ti15_4mac_radio_configuration_mac_user_config.obj software_stack_ti15_4mac_macTask.obj software_stack_ti15_4mac_stack_user_api_api_mac_api_mac.obj software_stack_ti15_4mac_stack_user_api_api_mac_timac_fhss_config.obj software_stack_ti15_4mac_osal_port_osal_port.obj software_stack_ti15_4mac_osal_port_osal_port_timers.obj mbed_nanostack_mbed-mesh-api_source_wisun_tasklet.obj $(patsubst %.c,%.obj,$(notdir $(SYSCFG_C_FILES)))
NAME = ns_coap_node

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
    @${PROJECT_LOC}/${ConfigName}/syscfg/ti_wisunfan_config.opts \
    -DBoard_EXCLUDE_NVS_EXTERNAL_FLASH \
    -DDeviceFamily_CC13X4 \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4/inc" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x4_cc26x4/driverlib" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos7/packages" \
    "-I$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/ticlang" \
    -gdwarf-3 \
    -mcpu=cortex-m33 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16

LFLAGS = "-i$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-i$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel" \
    "-i$(COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos7/packages" \
    --define=TIRTOS7 \
    --unused_section_elimination=on \
    --diag_wrap=off \
    --xml_link_info=${ProjName}_linkInfo.xml \
    -x \
    --display_error_number \
    --rom_model \
    --warn_sections \
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

syscfg: ../../tirtos7/ti_wisunfan_coap_node.syscfg
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

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK) -Wl,-u,_c_int00 $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) $(SYSCFG_GEN_FILES)> $(DEVNULL) 2>&1
