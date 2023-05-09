#!/usr/bin/env python3

#
# Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

import os
import sys
import time
import json
import logging
import binascii
import argparse
import hashlib
import traceback
from prodict import Prodict

from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import utils
from cryptography.hazmat.backends import default_backend

SCRIPT_PATH = os.path.dirname(os.path.realpath(__file__))
WORKSPACE_ROOT = os.path.abspath('{base}/../..'.format(base=SCRIPT_PATH))

sys.path.append(WORKSPACE_ROOT)

# Load the app configuration
from apps.remote_cert_provision import app_config

from transports.transport import DeviceTransport
from http_client.remote_cert_provision_client import RemoteCertProvisionClient

import srcp

logging.basicConfig(
    level=logging.INFO,
    format='%(levelname)7s %(asctime)s [%(filename)13s:%(lineno)4d] %(message)s'
)
logger = logging.getLogger(__name__)

SRCP_CHARACTERISTICS = {
    'mailbox_write_char': 'c7df0002-33c0-11e8-b467-0ed5f89f718b',
    'mailbox_notify_char': 'c7df0003-33c0-11e8-b467-0ed5f89f718b'
}

MAX_FRAG_LEN = 128

#pylint: disable=C0114,C0115,C0116

def write(device, data):
    while len(data):
        if len(data) < MAX_FRAG_LEN:
            pkt = bytes(bytearray([1]))
        else:
            pkt = bytes(bytearray([0]))

        device.write(pkt + data[:MAX_FRAG_LEN])
        time.sleep(0.05)
        data = data[MAX_FRAG_LEN:]


def read_with_timeout(device, timeout=None):
    if timeout is None:
        timeout = int(os.getenv('COMMAND_TIMEOUT', 60))  # 60 seconds timeout
    t_init = time.time()
    while time.time() - t_init < timeout:
        packet = device.read()
        if packet is not None:
            break
    return packet


def read(device, timeout=5):
    more_data = True
    accum = bytes(bytearray([]))
    while more_data:
        data = read_with_timeout(device, timeout)
        if not data:
            break
        more_data = data[0]
        accum += data[1:]
    return accum


class SRCPInvalidRegisrationEnvironmentError(Exception):
    pass


class SRCPDeviceCsrError(Exception):
    pass


class SRCPDeviceCsrSidewalkCloudError(Exception):
    pass


class SRCPDeviceCsrStatusError(Exception):
    pass


class SRCPInvalidPacketType(Exception):
    pass


class SRCPInvalidDeviceCsrResponse(Exception):
    pass


class CustRCPCloudError(Exception):
    pass


class CustRCPCloud:
    def __init__(self, config):
        self._config = config
        self._attest_device_csr = None
        self._payload = None

    def get_device_smsn(self):
        hash_msg = str.encode(self._config.device.type) \
                    + b'-REMOTEPROV' \
                    + str.encode(self._config.device.dsn) \
                    + str.encode(self._config.device.apid)
        return str.encode(hashlib.sha256(hash_msg).hexdigest())

    def attest_device_csr(self, payload):
        smsn = self.get_device_smsn()
        payload_smsn = binascii.hexlify(payload.smsn)
        if smsn != payload_smsn:
            raise CustRCPCloudError(
                f'SMSN do not match {smsn} != {payload_smsn}!')

        device_msg = payload.transaction_id \
                     + payload.smsn \
                     + payload.ed25519_puk \
                     + payload.p256r1_puk

        CustRCPCloud.p256k1_verify(device_msg, payload.signature,
                                   str.encode(self._config.device.pub_key))

        self._payload = payload

    def generate_attested_device_csr(self):
        attested_device_csr = Prodict.from_dict({
            "device_details": {},
            "server_details": {}
        })

        attested_device_csr.endpoint_id = self._payload.smsn
        attested_device_csr.device_details.ed25519_puk = self._payload.ed25519_puk
        attested_device_csr.device_details.p256r1_puk = self._payload.p256r1_puk
        attested_device_csr.device_details.signature = self._payload.signature
        attested_device_csr.device_details.transaction_id = self._payload.transaction_id

        attested_device_csr.server_details.apid = str.encode(
            self._config.device.apid)
        attested_device_csr.server_details.device_type = str.encode(
            self._config.device.type)
        attested_device_csr.server_details.dsn = str.encode(
            self._config.device.dsn)
        attested_device_csr.server_details.device_key = str.encode(
            self._config.device.pub_key)
        attested_device_csr.server_details.signature_issue_time = int(
            time.time())

        attest_csr_sig_msg = attested_device_csr.device_details.transaction_id \
                            + attested_device_csr.endpoint_id \
                            + attested_device_csr.device_details.ed25519_puk \
                            + attested_device_csr.device_details.p256r1_puk \
                            + attested_device_csr.device_details.signature \
                            + attested_device_csr.server_details.apid \
                            + attested_device_csr.server_details.device_type \
                            + attested_device_csr.server_details.dsn \
                            + binascii.unhexlify(attested_device_csr.server_details.device_key) \
                            + attested_device_csr.server_details.signature_issue_time.to_bytes(4, byteorder="big")

        attested_device_csr.server_details.signature = CustRCPCloud.p384r_sign(
            attest_csr_sig_msg, str.encode(self._config.cust_prk))

        self._attest_device_csr = attested_device_csr

        return attested_device_csr

    def verify_device_csr_status(self, payload):
        if payload.status != srcp.cust_srcp_device_csr_status.STATUS.CS_SUCCESS:
            raise SRCPDeviceCsrStatusError(
                f'RCP FAILED with error {payload.status}')

        # Check attested_csr_hash
        #assert(0)

        msg = payload.transaction_id + bytes(bytearray([payload.status.value
                                                        ])) + payload.hash
        CustRCPCloud.p256k1_verify(msg, payload.signature,
                                   str.encode(self._config.device.pub_key))

        smsn = self.get_device_smsn().decode('utf-8').upper()
        logger.info(
            f'DEVICE Successfully remote service provisioned with SMSN: {smsn}'
        )

    @staticmethod
    def p256k1_verify(hash_msg, sig, verifying_pub):
        hash_msg = bytes(hashlib.sha256(hash_msg).digest())

        verifying_pub = bytes([0x4]) + binascii.unhexlify(bytes(verifying_pub))
        p256k1_pub = ec.EllipticCurvePublicKey.from_encoded_point(
            ec.SECP256K1(), verifying_pub)
        _r, _s = int.from_bytes(sig[:32],
                              byteorder="big"), int.from_bytes(sig[32:],
                                                               byteorder="big")
        sig = utils.encode_dss_signature(_r, _s)
        p256k1_pub.verify(bytes(sig), hash_msg,
                          ec.ECDSA(utils.Prehashed(hashes.SHA256())))

    @staticmethod
    def p384r_sign(hash_msg, sig_prk):
        sig_prk = int(sig_prk, 16)
        p384r1_prk = ec.derive_private_key(sig_prk, ec.SECP384R1(),
                                           default_backend())
        sig = p384r1_prk.sign(hash_msg, ec.ECDSA(hashes.SHA512()))
        r, s = utils.decode_dss_signature(sig)

        def to_padded_hex(val, pad_len):
            return val.to_bytes(pad_len, byteorder='big')

        return to_padded_hex(r, 64) + to_padded_hex(s, 64)


class SrcpOrchestrator:
    def __init__(self,
                 client,
                 device,
                 config,
                 registration_environment='beta'):
        self._client = client
        self._device = device
        self._registration_environment = registration_environment
        self._cust_cloud = CustRCPCloud(config[registration_environment])
        self.__validate_init_environment()

    def __validate_init_environment(self):
        lwa_token = os.getenv('LWA_TOKEN')
        auth_token = os.getenv('AUTH_TOKEN')
        gateway_id = os.getenv('GATEWAY_ID')
        if lwa_token is not None:
            logger.info('LWA Token was provided')
        else:
            logger.info(
                'LWA Token was not provided. Checking for Z-Asset Auth token and Gateway ID'
            )
            if auth_token is None or gateway_id is None:
                raise AuthenticationError(
                    'Z-Asset Token or Gateway ID were not provided')

    def _start_srcp(self):
        pkt = srcp.cust_srcp_pkt()
        pkt.type = srcp.cust_srcp_pkt.TYPE.BEGIN
        try:
            pkt.srcp_begin.stage = {
                'beta': srcp.cust_srcp_device_begin.STAGE.BETA,
                'gamma': srcp.cust_srcp_device_begin.STAGE.QA,
                'prod': srcp.cust_srcp_device_begin.STAGE.PROD
            }[self._registration_environment]
        except KeyError as error:
            raise SRCPInvalidRegisrationEnvironmentError(
                f'Invalid registration_environment {self._registration_environment}'
            )

        write(self._device, pkt.encode_to_bytes())

    def _verify_srcp_device_csr(self):
        data = read(self._device)
        if not data:
            raise SRCPDeviceCsrError(f'Failed to receive device csr')

        pkt = srcp.cust_srcp_pkt()
        pkt.parse_from_bytes(data)
        if pkt.type != srcp.cust_srcp_pkt.TYPE.DEVICE_CSR:
            raise SRCPInvalidPacketType(
                f'Expected DEVICE_CSR, received {pkt.type}')

        self._cust_cloud.attest_device_csr(pkt.device_csr)

    def _send_attested_device_csr_to_sidewalk(self):
        attested_device_csr = self._cust_cloud.generate_attested_device_csr()

        def get_json_attested_device_csr(csr):
            return {
                "endpointId":
                binascii.hexlify(csr.endpoint_id).decode("utf-8"),
                "deviceDetails": {
                    "ed25519Puk":
                    binascii.hexlify(
                        csr.device_details.ed25519_puk).decode("utf-8"),
                    "p256r1Puk":
                    binascii.hexlify(
                        csr.device_details.p256r1_puk).decode("utf-8"),
                    "signature":
                    binascii.hexlify(
                        csr.device_details.signature).decode("utf-8"),
                    "transactionID":
                    binascii.hexlify(
                        csr.device_details.transaction_id).decode("utf-8"),
                },
                "serverDetails": {
                    "apid":
                    csr.server_details.apid.decode("utf-8"),
                    "deviceType":
                    csr.server_details.device_type.decode("utf-8"),
                    "serialNumber":
                    csr.server_details.dsn.decode("utf-8"),
                    "deviceKey":
                    csr.server_details.device_key.decode("utf-8"),
                    "signatureIssueTime":
                    csr.server_details.signature_issue_time,
                    "signature":
                    binascii.hexlify(csr.server_details.signature).decode(
                        "utf-8").zfill(256),
                }
            }

        response = self._client.post_authenticate_remote_device(
            data=get_json_attested_device_csr(attested_device_csr),
            lwa_token=os.getenv('LWA_TOKEN'),
            gateway_id=os.getenv('GATEWAY_ID'),
            auth=os.getenv('AUTH_TOKEN'))
        if response.ok:
            resp = response.json()
        else:
            raise SRCPDeviceCsrSidewalkCloudError(
                f'Request to Sidewalk RCP endpoint failed'
                f'Status: {response.status_code}'
                f'Reason: {response.reason}'
                f'Body: {response.text}')

        return resp

    def _send_store_cred_to_device(self, resp):
        def get_store_cred_from_device_csr_resp_json(resp):
            pkt = srcp.cust_srcp_pkt()
            pkt.type = srcp.cust_srcp_pkt.TYPE.STORE_CRED
            try:
                pkt.store_cred.transaction_id = binascii.unhexlify(
                    resp['transactionID'].encode())
                pkt.store_cred.ed25519_sig = binascii.unhexlify(
                    resp['ed25519Cert'].encode())
                pkt.store_cred.p256r1_sig = binascii.unhexlify(
                    resp['p256r1Cert'].encode())
                pkt.store_cred.signature = binascii.unhexlify(
                    resp['signature'].encode())
                pkt.store_cred.attested_csr_hash = binascii.unhexlify(
                    resp['requestHash'].encode())
            except KeyError as error:
                raise SRCPInvalidDeviceCsrResponse(str(error))
            return pkt.encode_to_bytes()

        write(self._device, get_store_cred_from_device_csr_resp_json(resp))

    def _verify_srcp_device_csr_status(self):
        data = read(self._device)
        if not data:
            raise SRCPDeviceCsrStatusError(
                'Failed to receive device csr Status')

        pkt = srcp.cust_srcp_pkt()
        pkt.parse_from_bytes(data)
        if pkt.type != srcp.cust_srcp_pkt.TYPE.CSR_STATUS:
            raise SRCPInvalidPacketType(
                f'Expected CSR_STATUS, received {pkt.type}')

        self._cust_cloud.verify_device_csr_status(pkt.status)

    def run(self):
        self._start_srcp()
        self._verify_srcp_device_csr()
        resp = self._send_attested_device_csr_to_sidewalk()
        self._send_store_cred_to_device(resp)
        self._verify_srcp_device_csr_status()


def main(cert_info):
    registration_config = app_config.get_config()
    client = RemoteCertProvisionClient()
    device = DeviceTransport(
        device_identifier=registration_config.get("DEVICE_IDENTIFIER"),
        characteristics=SRCP_CHARACTERISTICS)
    device.open()

    try:
        orch = SrcpOrchestrator(
            client, device, Prodict.from_dict(cert_info),
            registration_config.get('REGISTRATION_ENVIRONMENT'))
        orch.run()
    except Exception as error:
        logger.error('An error occurred during device registration')
        logger.error(f'Error: {error}')
        traceback.print_exc()

    device.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--cert_info',
        help="Cert json to be processed (defaults to default_cert_info.json)",
        type=argparse.FileType('r'),
        default='default_cert_info.json',
        required=False)

    args = parser.parse_args()
    main(cert_info=json.load(args.cert_info))
