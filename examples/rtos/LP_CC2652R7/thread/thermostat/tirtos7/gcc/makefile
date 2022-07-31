SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

SYSCFG_CMD_STUB = $(SYSCONFIG_TOOL) --compiler gcc --product $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_GEN_FILES := $(shell $(SYSCFG_CMD_STUB) --listGeneratedFiles --output . ../../tirtos7/thermostat.syscfg)
SYSCFG_REF_FILES := $(shell $(SYSCFG_CMD_STUB) --listReferencedFiles --output . ../../tirtos7/thermostat.syscfg)

ifeq ($(OS),Windows_NT)
	SYSCFG_FILES = $(subst \,/,$(SYSCFG_REF_FILES) $(SYSCFG_GEN_FILES))
else
	SYSCFG_FILES = $(SYSCFG_REF_FILES) $(SYSCFG_GEN_FILES)
endif
SYSCFG_C_FILES = $(filter %.c,$(SYSCFG_FILES))
SYSCFG_H_FILES = $(filter %.h,$(SYSCFG_FILES))

OBJECTS = platform_alarm.obj platform_alarm_micro.obj platform_diag.obj platform_entropy.obj platform_misc.obj platform_settings.obj platform_system.obj platform_radio.obj platform_uart.obj platform_nv_crc.obj platform_nv_nvocmp.obj thermostat.obj images.obj tiop_ui.obj cui.obj graphicext.obj disp_utils.obj main.obj otstack.obj otsupport_otrtosapi.obj platform_crypto_aes_alt.obj platform_crypto_sha256_alt.obj platform_crypto_ecjpake_alt.obj platform_crypto_crypto_misc.obj $(patsubst %.c,%.obj,$(notdir $(SYSCFG_C_FILES)))
NAME = thermostat

CFLAGS = -I../.. \
    -I. \
    -fdata-sections \
    -ffunction-sections \
    -Os \
    -g \
    -std=c99 \
    -fno-exceptions \
    -fno-rtti \
    -fno-common \
    -DHAVE_CONFIG_H \
    -DBoard_EXCLUDE_NVS_EXTERNAL_FLASH \
    -DNVOCMP_POSIX_MUTEX \
    -DNVOCMP_NVPAGES=2 \
    -DNDEBUG \
    -DBOARD_DISPLAY_USE_UART=1 \
    -DMBEDTLS_CONFIG_FILE='"mbedtls-config-cc13x2_26x2.h"' \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/thread/cc13x2_26x2/libmbedcrypto/config" \
    -IOPENTHREAD_MAKE_PATH/examples/platforms \
    -IOPENTHREAD_MAKE_PATH/include \
    -IOPENTHREAD_MAKE_PATH/src/core \
    -IOPENTHREAD_MAKE_PATH/src \
    -IOPENTHREAD_MAKE_PATH/third_party/mbedtls/repo/include \
    -IC:/.conan/e89868/1/external/source \
    -I${PROJECT_ROOT}/platform/crypto \
    -DOPENTHREAD_FTD=1 \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/thread/cc13x2_26x2/libopenthread_ftd/config" \
    -DOPENTHREAD_CONFIG_FILE='"openthread-config-cc13x2_26x2-ftd.h"' \
    -DOPENTHREAD_PROJECT_CORE_CONFIG_FILE='"openthread-core-cc13x2_26x2-config.h"' \
    -DTIOP_CUI=1 \
    -DCUI_POSIX=1 \
    -DBOARD_DISPLAY_USE_LCD=1 \
    -DDeviceFamily_CC13X2X7_CC26X2X7 \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/devices/cc13x2_cc26x2" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos7/packages" \
    -mcpu=cortex-m4 \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -gstrict-dwarf \
    -Wall \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include/newlib-nano" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS = -nostartfiles \
    -Wl,--gc-sections \
    --defsym=NVOCMP_NVPAGES=2 \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/thread/cc13x2_26x2/libopenthread_ftd/lib/gcc/m4f" \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/thread/cc13x2_26x2/libopenthread_platform_utils_ftd/lib/gcc/m4f" \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/thread/cc13x2_26x2/libmbedcrypto/lib/gcc/m4f" \
    -lopenthread_ftd \
    -lopenthread_platform_utils_ftd \
    -lmbedcrypto \
    -Wl,-T,../../tirtos7/gcc/cc13x2x7_cc26x2x7_tirtos7.lds \
    "-Wl,-Map,$(NAME).map" \
    -l${PROJECT_BUILD_DIR}/syscfg/ti_utils_build_linker.cmd.genlibs \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    -LC:/.conan/e89868/1/external/source \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos7/packages" \
    -l:ti/devices/cc13x2x7_cc26x2x7/driverlib/bin/gcc/driverlib.lib \
    -march=armv7e-m \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv4-sp-d16 \
    -static \
    "-L$(GCC_ARMCOMPILER)/arm-none-eabi/lib/thumb/v7e-m/hard" \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs
all: $(NAME).out

.INTERMEDIATE: syscfg
$(SYSCFG_FILES): syscfg
	@ echo generation complete

syscfg: ../../tirtos7/thermostat.syscfg
	@ echo Generating configuration files...
	@ $(SYSCFG_CMD_STUB) --output $(@D) $<

define C_RULE
$(basename $(notdir $(1))).obj: $(1) $(SYSCFG_H_FILES)
	@ echo Building $$@
	@ $(CC) $(CFLAGS) $$< -c -o $$@
endef

$(foreach c_file,$(SYSCFG_C_FILES),$(eval $(call C_RULE,$(c_file))))

platform_alarm.obj: ../../platform/alarm.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_alarm_micro.obj: ../../platform/alarm_micro.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_diag.obj: ../../platform/diag.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_entropy.obj: ../../platform/entropy.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_misc.obj: ../../platform/misc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_settings.obj: ../../platform/settings.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_system.obj: ../../platform/system.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_radio.obj: ../../platform/radio.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_uart.obj: ../../platform/uart.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_nv_crc.obj: ../../platform/nv/crc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_nv_nvocmp.obj: ../../platform/nv/nvocmp.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

thermostat.obj: ../../thermostat.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

images.obj: ../../images.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

tiop_ui.obj: ../../tiop_ui.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

cui.obj: ../../cui.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

graphicext.obj: ../../graphicext.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

disp_utils.obj: ../../disp_utils.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

main.obj: ../../main.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

otstack.obj: ../../otstack.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

otsupport_otrtosapi.obj: ../../otsupport/otrtosapi.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_crypto_aes_alt.obj: ../../platform/crypto/aes_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_crypto_sha256_alt.obj: ../../platform/crypto/sha256_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_crypto_ecjpake_alt.obj: ../../platform/crypto/ecjpake_alt.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

platform_crypto_crypto_misc.obj: ../../platform/crypto/crypto_misc.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) $(SYSCFG_GEN_FILES)> $(DEVNULL) 2>&1
