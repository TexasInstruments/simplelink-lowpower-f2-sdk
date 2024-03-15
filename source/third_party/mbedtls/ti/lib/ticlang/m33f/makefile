# Bring in common target-independent definitions
include ../../../defs.mak
# Uncomment to use conan packages instead of installed SDK
include ../../../../imports.conan.mak

# Enable local overrides via a top-directory 'products.mak'
#-include ../../../products.mak

CC = "$(TICLANG_ARMCOMPILER)/bin/tiarmclang"
AR = "$(TICLANG_ARMCOMPILER)/bin/tiarmar"

TICLANG_M33F_CFLAGS = $(POSIX_INCS_TICLANG) \
	-mcpu=cortex-m33 -mfloat-abi=hard -mfpu=fpv5-sp-d16 \
	-g -Oz

CFLAGS_CC13X4 = $(INCS_CC13X4) $(TICLANG_M33F_CFLAGS)

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
	$(CC) -c $(CFLAGS_CC13X4) $< -o $@

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
