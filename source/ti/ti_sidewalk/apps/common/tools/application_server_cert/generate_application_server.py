#!/usr/bin/env python3

#
# Copyright 2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.TXT file.This file is a
# Modifiable File, as defined in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#
from __future__ import print_function

import binascii
import argparse

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.asymmetric import ec, ed25519
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import utils

def bin2c(hex_data):
    if isinstance(hex_data, bytes):
        hex_data = hex_data.decode("utf-8")
    ret = ", 0x".join([hex_data[i:i+2] for i in range(0, len(hex_data), 2)])
    return "0x" + ret

# ------------------------------------------------------------------------------
# Process ED25519
# Genearate and extract private key
def generate_ed25519(apid_c, csr_f, serial_c):
    wip = "app-server-ed25519"
    k = ed25519.Ed25519PrivateKey.generate()

    sdata = k.private_bytes(serialization.Encoding.PEM,
                            serialization.PrivateFormat.PKCS8,
                            serialization.NoEncryption())
    with open(wip + "-private.pem", "wb") as f2w:
        f2w.write(sdata)

    sdata = k.private_bytes(serialization.Encoding.Raw,
                            serialization.PrivateFormat.Raw,
                            serialization.NoEncryption())
    txt = binascii.hexlify(sdata).zfill(64)
    print("%s private key: %s" % (wip, str(txt, 'utf-8')))
    with open(wip + "-private.txt", "wb") as f2w:
        f2w.write(txt)

    with open(wip + ".private.bin", "wb") as f2w:
        f2w.write(binascii.unhexlify(txt))

    # Extract public key
    puk = k.public_key()
    sdata = puk.public_bytes(serialization.Encoding.PEM,
                             serialization.PublicFormat.SubjectPublicKeyInfo)
    with open(wip + "-public.pem", "wb") as f2w:
        f2w.write(sdata)

    sdata = puk.public_bytes(serialization.Encoding.Raw,
                             serialization.PublicFormat.Raw)
    with open(wip + ".public.bin", "wb") as f2w:
        f2w.write(sdata)

    t = binascii.hexlify(sdata)
    print("%s public key: %s" % (wip, str(t, 'utf-8')))
    with open(wip + "-public.txt", "wb") as f2w:
        f2w.write(t)

    txt = bin2c(binascii.hexlify(sdata))
    with open(wip + ".c", "w") as f2w:
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_apid[4] = {" + apid_c + "};\n\n")
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_serial[4] = {" + serial_c + "};\n\n")
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_public[32] = {" + txt + "};\n\n")

    csr = sdata.zfill(32) + binascii.unhexlify(csr_f)
    with open(wip + "-csr.bin", "wb") as f2w:
        f2w.write(csr)

    signature = k.sign(csr)
    with open(wip + "-signature.bin", "wb") as f2w:
        f2w.write(signature)

    txt = bin2c(binascii.hexlify(signature).zfill(128))
    with open(wip + ".c", "a") as f2w:
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_signature_apid[64] = {" + txt + "};\n\n")

# ------------------------------------------------------------------------------
# Process P256R1
# Genearate and extract private key
def generate_p256r1(apid_c, csr_f, serial_c):
    wip = "app-server-p256r1"
    k = ec.generate_private_key(ec.SECP256R1(), default_backend())
    sdata = k.private_bytes(serialization.Encoding.PEM,
                            serialization.PrivateFormat.PKCS8,
                            serialization.NoEncryption())
    with open(wip + "-private.pem", "wb") as f2w:
        f2w.write(sdata)

    # Extract private key private bytes
    txt = hex(k.private_numbers().private_value).lstrip("0x").rstrip("L").zfill(64)
    print("%s Private key: %s" % (wip, txt))
    with open(wip + "-private.txt", "w") as f2w:
        f2w.write(txt)

    with open(wip + ".private.bin", "wb") as f2w:
        f2w.write(binascii.unhexlify(txt))

    # Extract public key (pem)
    puk = k.public_key()
    sdata = puk.public_bytes(serialization.Encoding.PEM,
                             serialization.PublicFormat.SubjectPublicKeyInfo)
    with open(wip + "-public.pem", "wb") as f2w:
        f2w.write(sdata)

    # Save public key public_bytes (Uncompressed Point), drop the leading 04, public key identifier
    sdata = puk.public_bytes(serialization.Encoding.X962,
                             serialization.PublicFormat.UncompressedPoint)[1:]
    txt = str(binascii.hexlify(sdata)[0:], 'utf-8')
    print("%s Public key:  %s" % (wip, txt))

    with open(wip + "-public.txt", "w") as f2w:
        f2w.write(txt)

    with open(wip + ".public.bin", "wb") as f2w:
        f2w.write(binascii.unhexlify(txt))

    txt = bin2c(binascii.hexlify(sdata))
    with open(wip + ".c", "w") as f2w:
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_apid[4] = {" + apid_c + "};\n\n")
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_serial[4] = {" + serial_c + "};\n\n")
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_public[64] = {" + txt + "};\n\n")

    csr = sdata.zfill(64) + binascii.unhexlify(csr_f)
    with open(wip + "-csr.bin", "wb") as f2w:
        f2w.write(csr)

    signature = k.sign(csr, ec.ECDSA(hashes.SHA256()))
    with open(wip + "-signature.der", "wb") as f2w:
        f2w.write(signature)

    r_d, s_d = utils.decode_dss_signature(signature)
    r_h = hex(r_d).rstrip("L").lstrip("0x").zfill(64)
    s_h = hex(s_d).rstrip("L").lstrip("0x").zfill(64)
    with open(wip + ".c", "a") as f2w:
        f2w.write("__ALIGN(4) const uint8_t " + wip + "_signature_apid[64] = {" + bin2c(r_h) + ",\n")
        f2w.write("\t\t" + bin2c(s_h) + "};\n\n")
    with open(wip + "-signature.bin", "wb") as f2w:
        f2w.write(binascii.unhexlify(r_h))
        f2w.write(binascii.unhexlify(s_h))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-s',
                        '--app_srv_serial',
                        required=True,
                        help="Certificate Application server #")
    parser.add_argument('-a',
                        '--apid',
                        required=True,
                        help="apid")
    args = parser.parse_args()

    apid = args.apid
    if len(apid) != 4:
        raise ValueError("apid length is not equal to 4")

    csn = int(args.app_srv_serial)

    print("Apid: %s     App Server Serial: %x " % (apid, csn))

    # Process api and serial
    apid_h = apid.encode("utf-8").hex()
    serial_h = hex(csn).lstrip("0x").zfill(8)

    apid_c = bin2c(apid_h)
    csr_f = apid_h + serial_h
    serial_c = bin2c(serial_h)

    generate_ed25519(apid_c, csr_f, serial_c)
    generate_p256r1(apid_c, csr_f, serial_c)

if __name__ == "__main__":
    main()
