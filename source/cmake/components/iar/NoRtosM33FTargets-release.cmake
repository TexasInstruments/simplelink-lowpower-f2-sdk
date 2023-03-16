#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "NoRtos::nortos_cc13x4" for configuration "Release"
set_property(TARGET NoRtos::nortos_cc13x4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NoRtos::nortos_cc13x4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc13x4.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS NoRtos::nortos_cc13x4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_NoRtos::nortos_cc13x4 "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc13x4.a" )

# Import target "NoRtos::nortos_cc26x4" for configuration "Release"
set_property(TARGET NoRtos::nortos_cc26x4 APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NoRtos::nortos_cc26x4 PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc26x4.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS NoRtos::nortos_cc26x4 )
list(APPEND _IMPORT_CHECK_FILES_FOR_NoRtos::nortos_cc26x4 "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc26x4.a" )

# Import target "NoRtos::nortos_cc13x4_ns" for configuration "Release"
set_property(TARGET NoRtos::nortos_cc13x4_ns APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NoRtos::nortos_cc13x4_ns PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc13x4_ns.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS NoRtos::nortos_cc13x4_ns )
list(APPEND _IMPORT_CHECK_FILES_FOR_NoRtos::nortos_cc13x4_ns "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc13x4_ns.a" )

# Import target "NoRtos::nortos_cc26x4_ns" for configuration "Release"
set_property(TARGET NoRtos::nortos_cc26x4_ns APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(NoRtos::nortos_cc26x4_ns PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc26x4_ns.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS NoRtos::nortos_cc26x4_ns )
list(APPEND _IMPORT_CHECK_FILES_FOR_NoRtos::nortos_cc26x4_ns "${_IMPORT_PREFIX}/kernel/nortos/lib/iar/m33f/nortos_cc26x4_ns.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
