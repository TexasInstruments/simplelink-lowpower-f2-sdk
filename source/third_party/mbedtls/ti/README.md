# README for TI Mbed TLS

TI provides hardware accelerated cryptographic primitives using Mbed TLS v3.5.0 for several cryptographic algorithms, including AES-ECB, AES-CBC, AES-CMAC, AES-CBC-MAC, AES-CCM, AES-GCM, TRNG (entropy), ECDH, ECDSA, ECJPAKE, SHA-2-256, and SHA-2-512. 

## Using pre-built libraries
TI provides pre-built libraries for CC13X4_CC26X4 devices using GCC, IAR and TICLANG compilers tools. With the following steps, the user can use TI Mbed TLS library in their project:
* Add include paths: 
    * `[SDK]/source/third_party/mbedtls/include`,
    * `[SDK]/source/third_party/mbedtls/ti`
    * `[SDK]/source/third_party/mbedtls/ti/port`
* Add predefined symbol: 
    * `"MBEDTLS_CONFIG_FILE="<configs/config-cc13x4.h>"`
* Link the library: 
    * `[SDK]/source/third_party/mbedtls/ti/lib/[TOOL]/m33f/mbdetls.a`

## Customizing the Mbed TLS support
As an alternative to consuming pre-built libraries from the SDK, the user may customize the configuration in Mbed TLS and rebuild the libraries using the following steps:
* Enable/disable the required configurations in `[SDK]/source/third_party/mbedtls/ti/configs/config-cc13x4.h`
* Re-build the library using the makefile in `[SDK]/source/third_party/mbedtls/ti/makefile`. From the SDK root directory, perform the following steps: 
>       cd source/third_party/mbedtls/ti/
>       make


Once the library is built with the desired configurations, the user can use the libraries as described above in Using pre-built libraries.