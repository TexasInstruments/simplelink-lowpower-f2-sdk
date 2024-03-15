# Bring in common target-independent definitions
include ../../../defs.mak
# Uncomment to use conan packages instead of installed SDK
include ../../../../imports.conan.mak

# Enable local overrides via a top-directory 'products.mak'
# -include ../../../products.mak

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
AR = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc-ar"

GCC_M33_CFLAGS= $(POSIX_INCS_GCC) -mcpu=cortex-m33 -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
				-Wno-builtin-requires-header -g -Os -ffunction-sections \
        		-fdata-sections -g -Wall -std=c99 -c

CFLAGS_CC13X4 = $(INCS_CC13X4) $(GCC_M33_CFLAGS)

ARFLAGS = -c -q

DEFAULT_OBJ_DIR = default
OBJS_DEFAULT = $(addprefix $(DEFAULT_OBJ_DIR)/,$(OBJS_CC13X4))

vpath %.c ../../../../library ../../../port $(TFM_INSTALL_DIR)/tfm_s/cc26x4/production_full/crypto_sp \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage/flash \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage/flash_fs \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/common/ \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/target/ti/cc26x4/cmsis_drivers/

$(DEFAULT_OBJ_DIR)/%.obj: %.c | $(DEFAULT_OBJ_DIR)
	@ echo "#"
	@ echo "# Building $@"
	$(CC) $(POSIX_INCS_GCC) $(CFLAGS_CC13X4) $< -o $@

all: mbedtls.a

$(DEFAULT_OBJ_DIR):
	mkdir $@

clean:
	@ rm -rf $(DEFAULT_OBJ_DIR)
	@ $(RM) *.a

mbedtls.a: $(OBJS_DEFAULT)
	@ echo "#"
	@ echo "# Building $@"
	$(AR) $(ARFLAGS) $@ $^