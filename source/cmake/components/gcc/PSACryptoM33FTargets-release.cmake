#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "PSACrypto::psa_crypto_cc13x4_ns" for configuration "Release"
set_property(TARGET PSACrypto::psa_crypto_cc13x4_ns APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PSACrypto::psa_crypto_cc13x4_ns PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/third_party/psa_crypto/library/lib/gcc/m33f/psa_crypto_cc13x4_ns.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS PSACrypto::psa_crypto_cc13x4_ns )
list(APPEND _IMPORT_CHECK_FILES_FOR_PSACrypto::psa_crypto_cc13x4_ns "${_IMPORT_PREFIX}/source/third_party/psa_crypto/library/lib/gcc/m33f/psa_crypto_cc13x4_ns.a" )

# Import target "PSACrypto::psa_crypto_cc26x4_ns" for configuration "Release"
set_property(TARGET PSACrypto::psa_crypto_cc26x4_ns APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(PSACrypto::psa_crypto_cc26x4_ns PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/source/third_party/psa_crypto/library/lib/gcc/m33f/psa_crypto_cc26x4_ns.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS PSACrypto::psa_crypto_cc26x4_ns )
list(APPEND _IMPORT_CHECK_FILES_FOR_PSACrypto::psa_crypto_cc26x4_ns "${_IMPORT_PREFIX}/source/third_party/psa_crypto/library/lib/gcc/m33f/psa_crypto_cc26x4_ns.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
