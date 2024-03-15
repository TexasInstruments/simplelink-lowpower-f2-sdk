#
# By default, look for an SDK-provided imports.mak file to provide
# dependency locations (e.g. toolchains).
#
# Note, this SDK's imports.mak inclusion is optional (the leading '-'
# when including).  If exists, we'll use variables in it, else the
# user will have to set these variables themselves.  The variables
# this build system may use, depending on what you're trying to build, include:
#    CCS_ARMCOMPILER     - CCS ARM toolchain
#    IAR_ARMCOMPILER     - IAR ARM toolchain
#    GCC_ARMCOMPILER     - GCC ARM toolchain
#    GCC_ARM64COMPILER   - GCC 64-bit ARM toolchain
#    CCS_C6XCOMPILER     - CCS C6x toolchain
#    TICLANG_ARMCOMPILER - TI ARM CLANG toolchain
#    RM                  - delete a file in an OS-independent way
#    RMDIR               - delete a directory in an OS-independent way
#
# Note that this SDK_INSTALL_DIR path is relative to the
# makefiles including this defs.mak, which are 3 directories deeper
# (lib/<toolchain>/<isa>) than this file.
#SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)
SDK_INSTALL_DIR ?= $(CORESDK_CC13XX_CC26XX_INSTALL_DIR)

# -include $(SDK_INSTALL_DIR)/imports.mak

# Default POSIX is in the SDK's source/ directory, but users can
# override this
POSIX_ROOT = $(SDK_INSTALL_DIR)/source

OBJS_CRYPTO=    aes.obj    aesni.obj    aria.obj    \
        asn1parse.obj asn1write.obj base64.obj    \
        bignum.obj bignum_core.obj    camellia.obj  \
        ccm.obj       chacha20.obj  chachapoly.obj \
        cipher.obj    cipher_wrap.obj   cmac.obj   \
        constant_time.obj  ctr_drbg.obj  des.obj    \
        dhm.obj       ecdh.obj      ecdsa.obj     \
        ecjpake.obj   ecp.obj               \
        ecp_curves.obj    entropy.obj   entropy_poll.obj  \
        error.obj     gcm.obj   hash_info.obj \
        hmac_drbg.obj    md.obj    md5.obj       \
        memory_buffer_alloc.obj       oid.obj       \
        padlock.obj   pem.obj       poly1305.obj     pk.obj        \
        pk_wrap.obj   pkcs12.obj    pkcs5.obj     \
        pkparse.obj   pkwrite.obj   platform.obj     platform_util.obj \
        ripemd160.obj rsa.obj    rsa_alt_helpers.obj   sha1.obj \
        sha256.obj    sha512.obj    threading.obj \
        timing.obj    version.obj           \
        version_features.obj

# Objects required for PSA Crypto API support
OBJS_PSACRYPTO_CC13X4 = psa_crypto_slot_management.obj psa_crypto.obj \
                psa_crypto_driver_wrappers.obj psa_crypto_hash.obj \
                psa_crypto_aead.obj psa_crypto_cipher.obj psa_crypto_ecp.obj \
                psa_crypto_mac.obj psa_crypto_rsa.obj \
                psa_crypto_storage.obj  tfm_its_local_api.obj \
                tfm_internal_trusted_storage.obj its_utils.obj its_flash.obj \
                its_flash_nor.obj its_flash_fs.obj its_flash_fs_dblock.obj \
                its_flash_fs_mblock.obj tfm_hal_its.obj psa_crypto_client.obj \
                Driver_Flash.obj
                
OBJS_X509=  x509.obj      \
        x509_create.obj   x509_crl.obj  x509_crt.obj  \
        x509_csr.obj  x509write_crt.obj x509write_csr.obj

OBJS_TLS=   debug.obj     net_sockets.obj       \
        ssl_cache.obj ssl_ciphersuites.obj  \
        ssl_client.obj   ssl_cookie.obj        \
        ssl_ticket.obj        \
        ssl_tls.obj   ssl_tls12_client.obj   \
        ssl_tls12_server.obj

OBJS_PORT= entropy_alt.obj threading_alt.obj

OBJS_PORT_CC13X4 = $(OBJS_PORT) aes_alt.obj ccm_alt.obj cmac_alt.obj \
                                ecdh_alt.obj ecdsa_alt.obj ecjpake_alt.obj \
                                entropy_alt.obj gcm_alt.obj sha256_alt.obj sha512_alt.obj

OBJS= $(OBJS_CRYPTO) \
        $(OBJS_X509) \
        $(OBJS_TLS) \
        $(OBJS_PORT)

OBJS_CC13X4 = $(OBJS_CRYPTO) \
        $(OBJS_X509) \
        $(OBJS_TLS) \
        $(OBJS_PORT_CC13X4) 
        # $(OBJS_PSACRYPTO_CC13X4)

INCS = -I../../../../include -I../../../../ti/port \
        -I../../../../ti/configs \
        -DMBEDTLS_CONFIG_FILE='"config-ti.h"'

INCS_CC13X4 = -I../../../../include -I../../../../ti/port \
        -I../../../../ti/configs \
        -DMBEDTLS_CONFIG_FILE='"config-cc13x4.h"' \
        -DDeviceFamily_CC13X4

# Include options required for PSA Crypto API support
# INCS_CC13X4 = -I../../../../include -I../../../../ti/port \
#         -I../../../../ti/configs \
#         -DMBEDTLS_CONFIG_FILE='"config-cc13x4-psa-alt.h"' \
#         -DMBEDTLS_PSA_CRYPTO_CONFIG_FILE='"config-psa-crypto-cc13x4.h"' \
#         -DDeviceFamily_CC13X4 \
#         -DITS_CREATE_FLASH_LAYOUT=1 \
#         -DTFM_PARTITION_LOG_LEVEL=0 \
#         -DTFM_SPM_LOG_LEVEL=0 \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/interface/include \
#         -I$(TFM_INSTALL_DIR)/source/ \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/include/ \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/config \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/common \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/lib/runtime/include \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/include \
#         -I$(SDK_INSTALL_DIR)/tfm_s/cc26x4/build_dependencies \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/crypto \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage/flash \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/internal_trusted_storage/flash_fs \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/partitions/protected_storage \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/secure_fw/spm/include \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/cmsis \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/driver \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/target/ti/cc26x4/retarget \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/target/ti/cc26x4/cmsis_core \
#         -I$(TFM_INSTALL_DIR)/source/third_party/tfm/platform/ext/target/ti/cc26x4
#         # -I$(TFM_INSTALL_DIR)/tfm_s/build/cc26x4/production_full/Release/export/tfm/inc

POSIX_INCS_CCS = -I$(POSIX_ROOT)/ti/posix/ccs -I$(POSIX_ROOT)
POSIX_INCS_IAR = -I$(POSIX_ROOT)/ti/posix/iar -I$(POSIX_ROOT)
POSIX_INCS_GCC = -I$(POSIX_ROOT)/ti/posix/gcc -I$(POSIX_ROOT)
POSIX_INCS_TICLANG = -I$(POSIX_ROOT)/ti/posix/ticlang -I$(POSIX_ROOT)
