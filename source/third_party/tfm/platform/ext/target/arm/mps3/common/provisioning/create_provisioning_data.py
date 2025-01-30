#! /usr/bin/env python3
#
# -----------------------------------------------------------------------------
# Copyright (c) 2020-2023, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# -----------------------------------------------------------------------------

import os
import sys
import click
from jinja2 import Environment, FileSystemLoader
from cryptography.hazmat.primitives.hashes import Hash, SHA256

# Add the cwd to the path so that if there is a version of imgtool in there then
# it gets used over the system imgtool. Used so that imgtool from upstream
# mcuboot is preferred over system imgtool
cwd = os.getcwd()
sys.path = [cwd] + sys.path
import imgtool
import imgtool.main

parser_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../'))
sys.path.append(parser_path)

# This works around Python 2 and Python 3 handling character encodings
# differently. More information about this issue at
# https://click.palletsprojects.com/en/5.x/python3
os.environ['LC_ALL'] = 'C.UTF-8'
os.environ['LANG'] = 'C.UTF-8'


def get_key_hash_c_array(key_file):
    key = imgtool.main.load_key(key_file)
    digest = Hash(SHA256())
    digest.update(key.get_public_bytes())
    return hex_to_c_array(digest.finalize())


@click.argument('outfile')
@click.option('--bl2_rot_priv_key_0', metavar='filename', required=True)
@click.option('--bl2_rot_priv_key_1', metavar='filename', required=False)
@click.option('--bl2_rot_priv_key_2', metavar='filename', required=False)
@click.option('--bl2_rot_priv_key_3', metavar='filename', required=False)
@click.option('--template_path', metavar='filename', required=True)
@click.option('--secure_debug_pk', metavar='key', required=False)
@click.option('--huk', metavar='key', required=False)
@click.option('--iak', metavar='key', required=False)
@click.option('--boot_seed', metavar='seed', required=False)
@click.option('--implementation_id', metavar='id', required=False)
@click.option('--certification_reference', metavar='reference', default="",
              required=False)
@click.option('--verification_service_url', metavar='url', default="",
              required=False)
@click.option('--entropy_seed', metavar='seed', required=False)
@click.command(help='''Create a signed or unsigned image\n
               INFILE and OUTFILE are parsed as Intel HEX if the params have
               .hex extension, otherwise binary format is used''')
def generate_provisioning_data_c(outfile, bl2_rot_priv_key_0,
                                 bl2_rot_priv_key_1,
                                 template_path, bl2_rot_priv_key_2,
                                 bl2_rot_priv_key_3,
                                 secure_debug_pk, huk, iak, boot_seed,
                                 implementation_id,
                                 certification_reference,
                                 verification_service_url,
                                 entropy_seed):

    environment = Environment(loader=FileSystemLoader(template_path))
    template = environment.get_template("provisioning_data_template.jinja2")

    # getpubhash cannot be called directly because of its click decorators
    bl2_rot_pub_key_0_hash = ""
    if bool(bl2_rot_priv_key_0) is True:
        bl2_rot_pub_key_0_hash = get_key_hash_c_array(bl2_rot_priv_key_0)

    bl2_rot_pub_key_1_hash = ""
    if bool(bl2_rot_priv_key_1) is True:
        bl2_rot_pub_key_1_hash = get_key_hash_c_array(bl2_rot_priv_key_1)

    bl2_rot_pub_key_2_hash = ""
    if bool(bl2_rot_priv_key_2) is True:
        bl2_rot_pub_key_2_hash = get_key_hash_c_array(bl2_rot_priv_key_2)

    bl2_rot_pub_key_3_hash = ""
    if bool(bl2_rot_priv_key_3) is True:
        bl2_rot_pub_key_3_hash = get_key_hash_c_array(bl2_rot_priv_key_3)

    if bool(huk) is False:
        huk = hex_to_c_array(os.urandom(32))

    if bool(iak) is False:
        iak = hex_to_c_array(os.urandom(32))

    if bool(boot_seed) is False:
        boot_seed = hex_to_c_array(os.urandom(32))

    if bool(implementation_id) is False:
        implementation_id = hex_to_c_array(os.urandom(32))

    if bool(entropy_seed) is False:
        entropy_seed = hex_to_c_array(os.urandom(64))

    key_arrays = {
        "bl2_rotpk_0": bl2_rot_pub_key_0_hash,
        "bl2_rotpk_1": bl2_rot_pub_key_1_hash,
        "bl2_rotpk_2": bl2_rot_pub_key_2_hash,
        "bl2_rotpk_3": bl2_rot_pub_key_3_hash,
        "secure_debug_pk": secure_debug_pk,
        "huk": huk,
        "iak": iak,
        "iak_len": "32",
        "boot_seed": boot_seed,
        "implementation_id": implementation_id,
        "certification_reference": certification_reference,
        "verification_service_url": verification_service_url,
        "entropy_seed": entropy_seed
    }

    with open(outfile, "w") as F:
        F.write(template.render(key_arrays))


def hex_to_c_array(hex_val):
    c_array = ""
    for count, b in enumerate(hex_val):
        if count % 8 == 0 and count != 0:
            c_array = c_array + '\n'
        c_array = c_array + "0x{:02x}, ".format(b)

    return c_array


if __name__ == '__main__':
    generate_provisioning_data_c()
