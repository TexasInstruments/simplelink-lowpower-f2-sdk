#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Tirtos7::tirtos7_m4" for configuration "Release"
set_property(TARGET Tirtos7::tirtos7_m4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Tirtos7::tirtos7_m4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM;C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/tirtos7/packages/ti/sysbios/lib/iar/m4/sysbios.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Tirtos7::tirtos7_m4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_Tirtos7::tirtos7_m4 "${_IMPORT_PREFIX}/kernel/tirtos7/packages/ti/sysbios/lib/iar/m4/sysbios.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
