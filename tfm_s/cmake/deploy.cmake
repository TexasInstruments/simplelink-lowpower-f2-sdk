# Install files required to build drivers library. Destination is relative to CMAKE_INSTALL_PREFIX.
install(FILES ${CMAKE_BINARY_DIR}/generated/interface/include/psa/framework_feature.h DESTINATION export/tfm/inc/psa)
install(FILES ${CMAKE_BINARY_DIR}/generated/interface/include/psa_manifest/sid.h DESTINATION export/tfm/inc/psa_manifest)
install(FILES ${CMAKE_BINARY_DIR}/generated/interface/include/psa_manifest/pid.h DESTINATION export/tfm/inc/psa_manifest)
install(FILES ${CMAKE_BINARY_DIR}/secure_fw/s_veneers.o DESTINATION export/tfm/veneers)
install(FILES ${GENERATED_FLASH_LAYOUT_DIR}/flash_layout.h DESTINATION export/tfm/inc)
install(FILES ${CMAKE_BINARY_DIR}/generated/interface/include/config_impl.h DESTINATION export/tfm/inc)

# Install SysConfig module used by Code Composer Studio to load the native C Runtime Object Viewer (ROV) capsules
install(FILES ${CMAKE_SOURCE_DIR}/../../ti/tfm/rov_theia/syscfg_c.rov.xs DESTINATION bin)
