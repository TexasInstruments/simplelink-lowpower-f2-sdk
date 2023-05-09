SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

CC = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"
AR = "$(TICLANG_ARMCOMPILER)/bin/tiarmar"

OBJECTS = src_core_api_diags_api.obj src_core_api_error_api.obj src_core_api_instance_api.obj src_core_api_link_raw_api.obj src_core_api_logging_api.obj src_core_api_random_noncrypto_api.obj src_core_api_tasklet_api.obj src_core_common_instance.obj src_core_common_log.obj src_core_common_random.obj src_core_common_string.obj src_core_common_tasklet.obj src_core_common_timer.obj src_core_common_uptime.obj src_core_crypto_aes_ccm.obj src_core_crypto_aes_ecb.obj src_core_crypto_crypto_platform.obj src_core_crypto_storage.obj src_core_diags_factory_diags.obj src_core_mac_link_raw.obj src_core_mac_mac_frame.obj src_core_mac_mac_types.obj src_core_mac_sub_mac.obj src_core_mac_sub_mac_callbacks.obj src_core_radio_radio.obj src_core_radio_radio_callbacks.obj src_core_radio_radio_platform.obj src_core_thread_link_quality.obj src_core_utils_parse_cmdline.obj src_lib_platform_exit_code.obj
NAME = libopenthread_radio

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
    -DMBEDTLS_CONFIG_FILE='"mbedtls-config-cc13x4_26x4.h"' \
    -I${WorkspaceDirPath}/libmbedcrypto_cc13x4_26x4_ticlang/config \
    -DDeviceFamily_CC13X4_CC26X4 \
    -DOPENTHREAD_RADIO=1 \
    -DOPENTHREAD_CONFIG_FILE='"openthread-config-cc13x4_26x4-rcp-spi.h"' \
    -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-cc13x4_26x4-config.h"' \
    -I${WorkspaceDirPath}/libopenthread_radio_spi_cc13x4_26x4_ticlang/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src/core \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src \
    -mcpu=cortex-m33 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16

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
    -DMBEDTLS_CONFIG_FILE='"mbedtls-config-cc13x4_26x4.h"' \
    -I${WorkspaceDirPath}/libmbedcrypto_cc13x4_26x4_ticlang/config \
    -DDeviceFamily_CC13X4_CC26X4 \
    -DOPENTHREAD_RADIO=1 \
    -DOPENTHREAD_CONFIG_FILE='"openthread-config-cc13x4_26x4-rcp-spi.h"' \
    -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-cc13x4_26x4-config.h"' \
    -I${WorkspaceDirPath}/libopenthread_radio_spi_cc13x4_26x4_ticlang/config \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src/core \
    -I${COM_TI_SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR}/source/third_party/openthread/src \
    -mcpu=cortex-m33 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16

all: $(NAME).a

src_lib_platform_exit_code.obj: OPENTHREAD_MAKE_PATH/src/lib/platform/exit_code.c
	@ echo Building $@
	@ $(CC) $(CFLAGS) -c $< -o $@

src_core_api_diags_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/diags_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_error_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/error_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_instance_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/instance_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_link_raw_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/link_raw_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_logging_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/logging_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_random_noncrypto_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/random_noncrypto_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_api_tasklet_api.obj: OPENTHREAD_MAKE_PATH/src/core/api/tasklet_api.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_instance.obj: OPENTHREAD_MAKE_PATH/src/core/common/instance.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_log.obj: OPENTHREAD_MAKE_PATH/src/core/common/log.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_random.obj: OPENTHREAD_MAKE_PATH/src/core/common/random.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_string.obj: OPENTHREAD_MAKE_PATH/src/core/common/string.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_tasklet.obj: OPENTHREAD_MAKE_PATH/src/core/common/tasklet.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_common_timer.obj: OPENTHREAD_MAKE_PATH/src/core/common/timer.cpp
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

src_core_crypto_storage.obj: OPENTHREAD_MAKE_PATH/src/core/crypto/storage.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_diags_factory_diags.obj: OPENTHREAD_MAKE_PATH/src/core/diags/factory_diags.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_link_raw.obj: OPENTHREAD_MAKE_PATH/src/core/mac/link_raw.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_mac_mac_frame.obj: OPENTHREAD_MAKE_PATH/src/core/mac/mac_frame.cpp
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

src_core_radio_radio.obj: OPENTHREAD_MAKE_PATH/src/core/radio/radio.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_radio_callbacks.obj: OPENTHREAD_MAKE_PATH/src/core/radio/radio_callbacks.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_radio_radio_platform.obj: OPENTHREAD_MAKE_PATH/src/core/radio/radio_platform.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_thread_link_quality.obj: OPENTHREAD_MAKE_PATH/src/core/thread/link_quality.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

src_core_utils_parse_cmdline.obj: OPENTHREAD_MAKE_PATH/src/core/utils/parse_cmdline.cpp
	@ echo Building $@
	@ $(CC) $(CPPFLAGS) -c $< -o $@

$(NAME).a: $(OBJECTS)
	@ echo creating library $@
	@ $(AR) r $@ $^

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).a > $(DEVNULL) 2>&1
