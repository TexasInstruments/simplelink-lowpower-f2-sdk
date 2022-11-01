#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TiUtils::json_m33f" for configuration "Release"
set_property(TARGET TiUtils::json_m33f APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TiUtils::json_m33f PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/ti/utils/json/lib/ticlang/m33f/json_release.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS TiUtils::json_m33f )
list(APPEND _IMPORT_CHECK_FILES_FOR_TiUtils::json_m33f "${_IMPORT_PREFIX}/source/ti/utils/json/lib/ticlang/m33f/json_release.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
