#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

find_package(Python3)

add_executable(dm_provisioning_bundle)

set_target_properties(dm_provisioning_bundle
    PROPERTIES
        SUFFIX ".axf"
)

target_add_scatter_file(dm_provisioning_bundle
    $<$<C_COMPILER_ID:ARMClang>:${CMAKE_CURRENT_SOURCE_DIR}/../bundle_common/provisioning_bundle.sct>
    $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/../bundle_common/provisioning_bundle.ld>
)

target_link_options(dm_provisioning_bundle
    PRIVATE
        $<$<C_COMPILER_ID:GNU>:-Wl,-Map=${CMAKE_BINARY_DIR}/bin/dm_provisioning_bundle.map>
        $<$<C_COMPILER_ID:ARMClang>:--map>
        $<$<C_COMPILER_ID:IAR>:--map\;${CMAKE_BINARY_DIR}/bin/dm_provisioning_bundle.map>
)

target_link_options(dm_provisioning_bundle
    PRIVATE
        --entry=do_provision
)

target_sources(dm_provisioning_bundle
    PRIVATE
        ./dm_provisioning_code.c
        $<$<BOOL:${TFM_DUMMY_PROVISIONING}>:./dm_dummy_provisioning_data.c>
        $<$<BOOL:${CONFIG_GNU_SYSCALL_STUB_ENABLED}>:${CMAKE_SOURCE_DIR}/platform/ext/common/syscalls_stub.c>
)

target_include_directories(dm_provisioning_bundle
    PRIVATE
        ../bundle_common
)

target_link_libraries(dm_provisioning_bundle
    PRIVATE
        platform_bl1_1_interface
        bl1_1_shared_lib_interface
)

target_link_shared_code(dm_provisioning_bundle
    bl1_1
)

add_custom_target(encrypted_dm_provisioning_bundle
    ALL
    SOURCES encrypted_dm_provisioning_bundle.bin
)

add_custom_command(OUTPUT encrypted_dm_provisioning_bundle.bin
    DEPENDS $<TARGET_FILE_DIR:dm_provisioning_bundle>/dm_provisioning_bundle.axf
    DEPENDS $<TARGET_FILE_DIR:bl1_2>/bl1_2_padded_hash.bin
    DEPENDS $<TARGET_FILE_DIR:bl1_2>/bl1_2_padded.bin
    bl1_2_padded_bin bl2_signed_bin dm_provisioning_bundle
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../rss_create_provisioning_bundle.py
    COMMAND ${CMAKE_OBJCOPY} $<TARGET_FILE_DIR:dm_provisioning_bundle>/dm_provisioning_bundle.axf
                    --dump-section CODE=dm_provisioning_code.bin
    COMMAND ${CMAKE_OBJCOPY} $<TARGET_FILE_DIR:dm_provisioning_bundle>/dm_provisioning_bundle.axf
                    --dump-section DATA=dm_provisioning_data.bin
    COMMAND ${CMAKE_OBJCOPY} $<TARGET_FILE_DIR:dm_provisioning_bundle>/dm_provisioning_bundle.axf
                    --dump-section VALUES=dm_provisioning_values.bin
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../rss_create_provisioning_bundle.py
                    --provisioning_code dm_provisioning_code.bin
                    --provisioning_data dm_provisioning_data.bin
                    --provisioning_values dm_provisioning_values.bin
                    --bundle_output_file encrypted_dm_provisioning_bundle.bin
                    --magic "0xBEEFFEED"
                    --key_file=${RSS_RTL_KEY_PATH}
                    --krtl_derivation_label="DM_PROVISIONING"
                    --provisioning_lcs 1
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/encrypted_dm_provisioning_bundle.bin ${CMAKE_BINARY_DIR}/bin/encrypted_dm_provisioning_bundle.bin
)
