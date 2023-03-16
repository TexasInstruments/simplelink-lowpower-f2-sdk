#-------------------------------------------------------------------------------
# Copyright (c) 2019-2022 Texas Instruments Incorporated
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/crypto_sp ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/internal_storage_sp ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/tdc ABSOLUTE APPEND)
embedded_target_include_directories(TARGET ${PROJECT_OBJ_LIB} PATH ${REMOTE_GEN_DIR}/interface/include ABSOLUTE APPEND)

include(${CMAKE_CURRENT_LIST_DIR}/crypto_sp/CMakeLists.inc)
include(${CMAKE_CURRENT_LIST_DIR}/internal_storage_sp/CMakeLists.inc)
include(${CMAKE_CURRENT_LIST_DIR}/tdc/CMakeLists.inc)

install(FILES ${REMOTE_GEN_DIR}/interface/include/psa_manifest/sid.h DESTINATION export/tfm/inc/psa_manifest)
install(FILES ${REMOTE_GEN_DIR}/interface/include/psa_manifest/pid.h DESTINATION export/tfm/inc/psa_manifest)
