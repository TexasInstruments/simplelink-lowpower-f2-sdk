#-------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

add_library(cc312_rom STATIC)

target_sources(cc312_rom
    PRIVATE
        ./cc3xx_lcs.c
        ./cc3xx_otp.c
        ./cc3xx_rng.c
        ./cc3xx_hash.c
        ./cc3xx_aes.c
        ./cc3xx_dma.c
        ./cc3xx_init.c
        ./cc3xx_engine_state.c
        ./cc3xx_stdlib.c
        ./cc3xx_chacha.c
        ./cc3xx_kdf.c
        ./cc3xx_pka.c
        ./cc3xx_poly1305.c
        ./cc3xx_hmac.c
        ./cc3xx_drbg_ctr.c
        ./cc3xx_drbg_hash.c
        ./cc3xx_drbg_hmac.c
        ./cc3xx_drbg.c
)

target_include_directories(cc312_rom
    PUBLIC
        .
)

target_link_libraries(cc312_rom
    PRIVATE
        platform_bl1_1_interface
)
