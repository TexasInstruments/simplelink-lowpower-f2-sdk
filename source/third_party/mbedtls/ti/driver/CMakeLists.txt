set(ti_sl_target ${MBEDTLS_TARGET_PREFIX}ti_sl)

set(src_ti_sl_driver
  ti_sl_transparent_key_management.c
  ti_sl_transparent_builtin_key_support.c
)
add_library(${ti_sl_target} STATIC ${src_ti_sl_driver})

target_include_directories(${ti_sl_target}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
         $<BUILD_INTERFACE:${MBEDTLS_DIR}/include>
         $<BUILD_INTERFACE:${MBEDTLS_DIR}/ti>
         $<BUILD_INTERFACE:${MBEDTLS_DIR}/../../>
)

# Enable floating point here to help with TFM builds
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfloat-abi=hard -mfpu=fpv5-sp-d16")

# Pass-through MBEDTLS_CONFIG_FILE and MBEDTLS_USER_CONFIG_FILE
# This must be duplicated from library/CMakeLists.txt because
# ti_sl is not directly linked against any mbedtls targets
# so does not inherit the compile definitions.

target_compile_definitions(${MBEDTLS_TARGET_PREFIX}mbedcrypto
        INTERFACE
            MBEDTLS_PSA_CRYPTO_KEY_ID_ENCODES_OWNER
            MBEDTLS_CONFIG_FILE="$<BUILD_INTERFACE:${MBEDTLS_DIR}/ti/configs/config-tidrivers.h>"
    )

if(MBEDTLS_CONFIG_FILE)
    target_compile_definitions(ti_sl
        PUBLIC MBEDTLS_CONFIG_FILE="${MBEDTLS_CONFIG_FILE}")
endif()
if(MBEDTLS_USER_CONFIG_FILE)
    target_compile_definitions(ti_sl
        PUBLIC MBEDTLS_USER_CONFIG_FILE="${MBEDTLS_USER_CONFIG_FILE}")
endif()

install(TARGETS ${ti_sl_target}
EXPORT MbedTLSTargets
DESTINATION ${CMAKE_INSTALL_LIBDIR}
PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
