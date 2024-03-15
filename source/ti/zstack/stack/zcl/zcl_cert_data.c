/**************************************************************************************************
  Filename:       zcl_cert_data.c
  Revised:        $Date: 2014-06-27 18:15:32 -0700 (Fri, 27 Jun 2014) $
  Revision:       $Revision: 39269 $

  Description:    This file defines actual data for use as in test & debug when
                  instantiating the Certicom Certificate Data for ECC SECURITY.


  Copyright (c) 2019, Texas Instruments Incorporated
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  *  Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

  *  Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  *  Neither the name of Texas Instruments Incorporated nor the names of
      its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "zcomdef.h"

#if defined TEST_CERT_DATA
/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

// The unique Extended Address is mangled into the following 3 certificates using
// the certificate generation tool from Certicom.
#if ZG_BUILD_COORDINATOR_TYPE
#pragma location="IEEE_ADDRESS_SPACE"
const uint8_t certExtendedAddress[Z_EXTADDR_LEN] = {
  0x01, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x12, 0x00
};
#pragma required=certExtendedAddress
#pragma location="IMPLICIT_CERTIFICATE_ADDRESS_SPACE"
const uint8_t ImplicitCertificate[ZCL_KE_IMPLICIT_CERTIFICATE_LEN] = {
  0x03, 0x07, 0x8c, 0x45, 0xde, 0xa5, 0x06, 0xd0,
  0x7f, 0x1b, 0x82, 0x21, 0x22, 0xb5, 0xa3, 0x1e,
  0xb0, 0xa0, 0xd6, 0x29, 0x55, 0xdb, 0x00, 0x12,
  0x4b, 0x00, 0x00, 0x00, 0x00, 0x01, 0x54, 0x45,
  0x53, 0x54, 0x53, 0x45, 0x43, 0x41, 0x01, 0x09,
  0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#pragma required=ImplicitCertificate
#pragma location="CA_PUBLIC_KEY_ADDRESS_SPACE"
const uint8_t CA_Public_Key[ZCL_KE_CA_PUBLIC_KEY_LEN] = {
  0x02, 0x00, 0xfd, 0xe8, 0xa7, 0xf3, 0xd1, 0x08,
  0x42, 0x24, 0x96, 0x2a, 0x4e, 0x7c, 0x54, 0xe6,
  0x9a, 0xc3, 0xf0, 0x4d, 0xa6, 0xb8
};
#pragma required=CA_Public_Key
#pragma location="DEV_PRIVATE_KEY_ADDRESS_SPACE"
const uint8_t DevicePrivateKey[ZCL_KE_DEVICE_PRIVATE_KEY_LEN] = {
  0x02, 0x28, 0x4a, 0x56, 0x3f, 0x02, 0xf2, 0xc8,
  0xbd, 0xa7, 0x57, 0xf9, 0x61, 0xbb, 0x8c, 0xb4,
  0xfb, 0x6e, 0x90, 0xed, 0x42
};
#pragma required=DevicePrivateKey
#elif ZG_BUILD_RTR_TYPE
#pragma location="IEEE_ADDRESS_SPACE"
const uint8_t certExtendedAddress[Z_EXTADDR_LEN] = {
  0x02, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x12, 0x00
};
#pragma required=certExtendedAddress
#pragma location="IMPLICIT_CERTIFICATE_ADDRESS_SPACE"
const uint8_t ImplicitCertificate[ZCL_KE_IMPLICIT_CERTIFICATE_LEN] = {
  0x03, 0x06, 0x9b, 0xc9, 0x8f, 0x75, 0x1f, 0xba,
  0x22, 0xdb, 0xca, 0xd6, 0x4a, 0x7d, 0x46, 0x2c,
  0x20, 0x9c, 0x2e, 0xd9, 0x01, 0x5f, 0x00, 0x12,
  0x4b, 0x00, 0x00, 0x00, 0x00, 0x02, 0x54, 0x45,
  0x53, 0x54, 0x53, 0x45, 0x43, 0x41, 0x01, 0x09,
  0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#pragma required=ImplicitCertificate
#pragma location="CA_PUBLIC_KEY_ADDRESS_SPACE"
const uint8_t CA_Public_Key[ZCL_KE_CA_PUBLIC_KEY_LEN] = {
  0x02, 0x00, 0xfd, 0xe8, 0xa7, 0xf3, 0xd1, 0x08,
  0x42, 0x24, 0x96, 0x2a, 0x4e, 0x7c, 0x54, 0xe6,
  0x9a, 0xc3, 0xf0, 0x4d, 0xa6, 0xb8
};
#pragma required=CA_Public_Key
#pragma location="DEV_PRIVATE_KEY_ADDRESS_SPACE"
const uint8_t DevicePrivateKey[ZCL_KE_DEVICE_PRIVATE_KEY_LEN] = {
  0x03, 0x7a, 0xd5, 0x45, 0xab, 0xa5, 0xf7, 0x47,
  0xe4, 0x0a, 0x24, 0xea, 0x85, 0x50, 0x4c, 0x47,
  0x39, 0x8c, 0x97, 0xf4, 0x96
};
#pragma required=DevicePrivateKey
#else  // if ZG_BUILD_ENDDEVICE_TYPE
#pragma location="IEEE_ADDRESS_SPACE"
const uint8_t certExtendedAddress[Z_EXTADDR_LEN] = {
  0x03, 0x00, 0x00, 0x00, 0x00, 0x4b, 0x12, 0x00
};
#pragma required=certExtendedAddress
#pragma location="IMPLICIT_CERTIFICATE_ADDRESS_SPACE"
const uint8_t ImplicitCertificate[ZCL_KE_IMPLICIT_CERTIFICATE_LEN] = {
  0x02, 0x04, 0xac, 0x2c, 0x26, 0x56, 0xf1, 0xee,
  0xa4, 0xff, 0x5d, 0xac, 0x4e, 0xdd, 0xa1, 0x76,
  0xbf, 0xe4, 0xfa, 0x70, 0xd9, 0x56, 0x00, 0x12,
  0x4b, 0x00, 0x00, 0x00, 0x00, 0x03, 0x54, 0x45,
  0x53, 0x54, 0x53, 0x45, 0x43, 0x41, 0x01, 0x09,
  0x00, 0x01, 0x00, 0x00, 0x01, 0x09, 0x10, 0x03
};
#pragma required=ImplicitCertificate
#pragma location="CA_PUBLIC_KEY_ADDRESS_SPACE"
const uint8_t CA_Public_Key[ZCL_KE_CA_PUBLIC_KEY_LEN] = {
  0x02, 0x00, 0xfd, 0xe8, 0xa7, 0xf3, 0xd1, 0x08,
  0x42, 0x24, 0x96, 0x2a, 0x4e, 0x7c, 0x54, 0xe6,
  0x9a, 0xc3, 0xf0, 0x4d, 0xa6, 0xb8
};
#pragma required=CA_Public_Key
#pragma location="DEV_PRIVATE_KEY_ADDRESS_SPACE"
const uint8_t DevicePrivateKey[ZCL_KE_DEVICE_PRIVATE_KEY_LEN] = {
  0x03, 0x4b, 0xc3, 0x7a, 0x72, 0x10, 0xb7, 0x40,
  0x7a, 0x51, 0xdc, 0x11, 0xe5, 0xae, 0xba, 0xf2,
  0xe1, 0x50, 0x3f, 0x69, 0x55
};
#pragma required=DevicePrivateKey
#endif // ZG_BUILD_COORDINATOR_TYPE

#endif // TEST_CERT_DATA

/**************************************************************************************************
*/
