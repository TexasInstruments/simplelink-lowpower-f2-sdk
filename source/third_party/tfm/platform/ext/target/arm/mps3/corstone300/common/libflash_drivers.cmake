#-------------------------------------------------------------------------------
# Copyright (c) 2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# The flash_drivers_s has to be static libraries because they have
# to be built as a standalone lib and loaded to a predefined
# address in SRAM memory. The linker places it to the
# ER_CODE_SRAM address.

target_link_libraries(flash_drivers_s
    PRIVATE
        flash_drivers
        cmsis_includes_s
)
target_link_libraries(flash_drivers_bl2
    PRIVATE
        flash_drivers
        cmsis_includes_bl2
)

target_compile_options(flash_drivers_s
    PRIVATE
        ${COMPILER_CP_FLAG}
)

target_link_options(flash_drivers_s
    PRIVATE
        ${LINKER_CP_OPTION}
)

target_compile_options(flash_drivers_bl2
    PRIVATE
        ${BL2_COMPILER_CP_FLAG}
)

target_link_options(flash_drivers_bl2
    PRIVATE
        ${BL2_LINKER_CP_OPTION}
)

target_link_libraries(platform_bl2
    PRIVATE
        flash_drivers_bl2
)

target_link_libraries(platform_s
    PRIVATE
        flash_drivers_s
)
