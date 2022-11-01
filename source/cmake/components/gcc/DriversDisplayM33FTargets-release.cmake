#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Drivers::display_cc13x4" for configuration "Release"
set_property(TARGET Drivers::display_cc13x4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Drivers::display_cc13x4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/ti/display/lib/gcc/m33f/display_cc13x4.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Drivers::display_cc13x4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_Drivers::display_cc13x4 "${_IMPORT_PREFIX}/source/ti/display/lib/gcc/m33f/display_cc13x4.a" )

# Import target "Drivers::display_cc26x4" for configuration "Release"
set_property(TARGET Drivers::display_cc26x4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Drivers::display_cc26x4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/ti/display/lib/gcc/m33f/display_cc26x4.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS Drivers::display_cc26x4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_Drivers::display_cc26x4 "${_IMPORT_PREFIX}/source/ti/display/lib/gcc/m33f/display_cc26x4.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
