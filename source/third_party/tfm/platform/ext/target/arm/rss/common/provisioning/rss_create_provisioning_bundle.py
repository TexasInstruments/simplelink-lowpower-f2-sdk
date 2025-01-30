#-------------------------------------------------------------------------------
# Copyright (c) 2021-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

import argparse
import struct
import secrets
import binascii
from cryptography.hazmat.primitives.ciphers.aead import AESCCM
from cryptography.hazmat.primitives import cmac
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
import hashlib

def struct_pack(objects, pad_to=0):
    defstring = "<"
    for obj in objects:
        defstring += str(len(obj)) + "s"

    size = struct.calcsize(defstring)
    if size < pad_to:
        defstring += str(pad_to - size) + "x"
    elif size > pad_to and pad_to != 0:
        print("Error padding struct of size {} to {}".format(size, pad_to))
        exit(1);

    return (bytes(struct.pack(defstring, *objects)))

def derive_encryption_key(input_key):

    # Every element of the boot state is 0
    boot_state = struct_pack([
        int(args.provisioning_lcs).to_bytes(4, byteorder='little'),
        bytes(32 + 4),
    ])
    hash = hashlib.sha256()
    hash.update(boot_state)
    context = hash.digest()

    output_key = bytes(0);
    # The KDF outputs 16 bytes per iteration, so we need 2 for an AES-256 key
    for i in range(2):
        state = struct_pack([(i + 1).to_bytes(4, byteorder='little'),
                             # C keeps the null byte, python removes it, so we add
                             # it back manually.
                             args.krtl_derivation_label.encode('ascii') + bytes(1),
                             bytes(1), context,
                             (32).to_bytes(4, byteorder='little')])
        c = cmac.CMAC(algorithms.AES(input_key))
        c.update(state)
        output_key += c.finalize()
    return output_key

parser = argparse.ArgumentParser()
parser.add_argument("--provisioning_code", help="the input provisioning code", required=True)
parser.add_argument("--provisioning_data", help="the input provisioning data", required=True)
parser.add_argument("--provisioning_values", help="the input provisioning values", required=True)
parser.add_argument("--magic", help="the magic constant to insert at the start and end", required=True)
parser.add_argument("--bl1_2_padded_hash_input_file", help="the hash of the final bl1_2 image", required=False)
parser.add_argument("--bl1_2_input_file", help="the final bl1_2 image", required=False)
parser.add_argument("--rss_id", help="the ID of the RSS", required=False)
parser.add_argument("--otp_dma_ics_input_file", help="OTP DMA ICS input file", required=False)
parser.add_argument("--bundle_output_file", help="bundle output file", required=False)
parser.add_argument("--key_file", help="the AES-CCM key file", required=True)
parser.add_argument("--krtl_derivation_label", help="The provisioning key derivation label", required=True)
parser.add_argument("--provisioning_lcs", help="The LCS in which provisioning will be run", required=True)
args = parser.parse_args()

with open(args.provisioning_code, "rb") as in_file:
    code = in_file.read()

# It's technically possible to have provisioning code that requires no DATA
# section, so this is optional.
try:
    with open(args.provisioning_data, "rb") as in_file:
        data = in_file.read()
except FileNotFoundError:
    data = bytes(0);

with open(args.provisioning_values, "rb") as in_file:
    values = in_file.read()

with open(args.key_file, "rb") as in_file:
    input_key = in_file.read()

if args.bl1_2_padded_hash_input_file:
    with open(args.bl1_2_padded_hash_input_file, "rb") as in_file:
        bl1_2_padded_hash = in_file.read()
else:
    bl1_2_padded_hash = bytes(0)

if args.bl1_2_input_file:
    with open(args.bl1_2_input_file, "rb") as in_file:
        bl1_2 = in_file.read()
else:
    bl1_2 = bytes(0)

if args.otp_dma_ics_input_file:
    with open(args.otp_dma_ics_input_file, "rb") as in_file:
        otp_dma_ics = in_file.read()
    otp_dma_ics = struct_pack([ otp_dma_ics], pad_to=0x400 - 4)
    otp_ics_crc = binascii.crc32(otp_dma_ics).to_bytes(4, byteorder='little')
    otp_dma_ics = struct_pack([otp_ics_crc, otp_dma_ics])
else:
    otp_dma_ics = bytes(0)

if args.rss_id != None:
    rss_id = int(args.rss_id, 0).to_bytes(4, 'little')
else:
    rss_id = bytes(0)

patch_bundle = struct_pack([
    bl1_2_padded_hash,
    bl1_2,
    otp_dma_ics,
    rss_id,
])

code = struct_pack([code], pad_to=0x8000)
values = struct_pack([patch_bundle, values[len(patch_bundle):]], pad_to=0x3800)
data = struct_pack([values, data], pad_to=0x3E00)

ccm_iv = secrets.token_bytes(12)

to_auth = struct_pack([
    int(args.magic, 16).to_bytes(4, 'little'),
])

to_encrypt = struct_pack([
    code,
    data,
])

encrypted_data = AESCCM(derive_encryption_key(input_key),
                        tag_length=16).encrypt(ccm_iv, to_encrypt, to_auth)
tag = encrypted_data[-16:]
encrypted_data = encrypted_data[:-16]

bundle = struct_pack([
    to_auth,
    encrypted_data,
    ccm_iv,
    tag,
    int(args.magic, 16).to_bytes(4, 'little'),
])

with open(args.bundle_output_file, "wb") as out_file:
    out_file.write(bundle)
