#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Driverlib::cc13x4_cc26x3_cc26x4" for configuration "Release"
set_property(TARGET Driverlib::cc13x4_cc26x3_cc26x4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Driverlib::cc13x4_cc26x3_cc26x4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/ti/devices/cc13x4_cc26x4/driverlib/bin/ticlang/driverlib.lib"
  )

list(APPEND _IMPORT_CHECK_TARGETS Driverlib::cc13x4_cc26x3_cc26x4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_Driverlib::cc13x4_cc26x3_cc26x4 "${_IMPORT_PREFIX}/source/ti/devices/cc13x4_cc26x4/driverlib/bin/ticlang/driverlib.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
