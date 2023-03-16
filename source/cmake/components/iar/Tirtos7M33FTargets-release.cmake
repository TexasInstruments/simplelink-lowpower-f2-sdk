#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Tirtos7::tirtos7_m33f" for configuration "Release"
set_property(TARGET Tirtos7::tirtos7_m33f APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Tirtos7::tirtos7_m33f PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM;C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/tirtos7/packages/ti/sysbios/lib/iar/m33f/sysbios.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Tirtos7::tirtos7_m33f )
list(APPEND _IMPORT_CHECK_FILES_FOR_Tirtos7::tirtos7_m33f "${_IMPORT_PREFIX}/kernel/tirtos7/packages/ti/sysbios/lib/iar/m33f/sysbios.a" )

# Import target "Tirtos7::tirtos7_m33f_ns" for configuration "Release"
set_property(TARGET Tirtos7::tirtos7_m33f_ns APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Tirtos7::tirtos7_m33f_ns PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM;C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/tirtos7/packages/ti/sysbios/lib/iar/m33f/sysbios_ns.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Tirtos7::tirtos7_m33f_ns )
list(APPEND _IMPORT_CHECK_FILES_FOR_Tirtos7::tirtos7_m33f_ns "${_IMPORT_PREFIX}/kernel/tirtos7/packages/ti/sysbios/lib/iar/m33f/sysbios_ns.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
