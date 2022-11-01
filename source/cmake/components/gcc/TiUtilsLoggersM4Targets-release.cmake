#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "TiUtils::tiutils_loggers_m4" for configuration "Release"
set_property(TARGET TiUtils::tiutils_loggers_m4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(TiUtils::tiutils_loggers_m4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/ti/loggers/utils/lib/gcc/m4/loggers_release.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS TiUtils::tiutils_loggers_m4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_TiUtils::tiutils_loggers_m4 "${_IMPORT_PREFIX}/source/ti/loggers/utils/lib/gcc/m4/loggers_release.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
