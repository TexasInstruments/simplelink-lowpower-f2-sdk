#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Loggers::loggers_m4" for configuration "Release"
set_property(TARGET Loggers::loggers_m4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Loggers::loggers_m4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/ti/loggers/drivers/lib/gcc/m4/loggers.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Loggers::loggers_m4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_Loggers::loggers_m4 "${_IMPORT_PREFIX}/source/ti/loggers/drivers/lib/gcc/m4/loggers.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
