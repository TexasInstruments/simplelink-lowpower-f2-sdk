# Bring in common target-independent definitions
include ../../../defs.mak

# Uncomment to use conan packages instead of installed SDK
include ../../../../imports.conan.mak

# Enable local overrides via a top-directory 'products.mak'
# -include ../../../products.mak

CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
AR = "$(IAR_ARMCOMPILER)/bin/iarchive"

IAR_M33F_CFLAGS = "-I$(IAR_ARMCOMPILER)/inc" --silent --aeabi --cpu=Cortex-M33.fp \
        --diag_suppress=Pa050 --endian=little -e --fpu=FPv5_sp --thumb

CFLAGS_CC13X4 = $(INCS_CC13X4) $(IAR_M33F_CFLAGS)
DEFAULT_OBJ_DIR = default
OBJS_DEFAULTDIR = $(addprefix $(DEFAULT_OBJ_DIR)/,$(OBJS_CC13X4))

vpath %.c ../../../../library ../../../port $(TFM_INSTALL_DIR)/tfm_s/cc26x4/production_full/crypto_sp \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage/flash \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage/flash_fs \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/common/ \
		  $(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/target/ti/cc26x4/cmsis_drivers/

$(DEFAULT_OBJ_DIR)/%.obj: %.c | $(DEFAULT_OBJ_DIR)
	@ echo "#"
	@ echo "# Building $@"
	$(CC) $(POSIX_INCS_IAR) $(CFLAGS_CC13X4) $< -o $@

all: mbedtls.a

$(DEFAULT_OBJ_DIR):
	mkdir $@

clean:
	@ rm -rf $(DEFAULT_OBJ_DIR)
	@ $(RM) *.a

mbedtls.a: $(OBJS_DEFAULTDIR)
	@ echo "#"
	@ echo "# Building $@"
	@ $(RM) $@
	$(AR) $@ $^
