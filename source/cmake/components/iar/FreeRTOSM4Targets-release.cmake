#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "FreeRTOS::freertos_m4" for configuration "Release"
set_property(TARGET FreeRTOS::freertos_m4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(FreeRTOS::freertos_m4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "ASM"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/freertos/lib/iar/m4/freertos.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS FreeRTOS::freertos_m4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_FreeRTOS::freertos_m4 "${_IMPORT_PREFIX}/kernel/freertos/lib/iar/m4/freertos.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
