#!/usr/bin/env python3
#
# Copyright 2020-2022 Amazon.com, Inc. or its affiliates. All rights reserved.
#
# AMAZON PROPRIETARY/CONFIDENTIAL
#
# You may not use this file except in compliance with the terms and conditions
# set forth in the accompanying LICENSE.txt file. This file is a
# Modifiable File, as defined in the accompanying LICENSE.txt file.
#
# THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
# DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
# IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
#

from __future__ import annotations
import argparse
import binascii
import json
import base64
import sys
from ctypes import Structure, c_ubyte
import traceback
import yaml
from prodict import Prodict  # type: ignore
from enum import Enum
from pathlib import Path
from typing import Tuple
from typing import Union
from typing import List
from typing import Any
from typing import Optional
from typing import Iterator

SMSN_SIZE: int = 32
SERIAL_SIZE: int = 4
PRK_SIZE: int = 32
ED25519_PUB_SIZE: int = 32
P256R1_PUB_SIZE: int = 64
SIG_SIZE: int = 64

# pylint: disable=C0114,C0115,C0116


class SidMfgValueId(Enum):
    """
    Please note that these values have to be in sync at alls times with
    projects/sid/sal/common/public/sid_pal_ifc/mfg_store/sid_pal_mfg_store_ifc.h
    sid_pal_mfg_store_value_t
    """

    """
    Format
    SID_PAL_MFG_STORE_XXXX = (<VALUE>, <SIZE>)
    """

    SID_PAL_MFG_STORE_MAGIC: Tuple[int, int] = (0, 4)
    SID_PAL_MFG_STORE_DEVID: Tuple[int, int] = (1, 5)
    SID_PAL_MFG_STORE_VERSION: Tuple[int, int] = (2, 4)
    SID_PAL_MFG_STORE_SERIAL_NUM: Tuple[int, int] = (3, 17)
    SID_PAL_MFG_STORE_SMSN: Tuple[int, int] = (4, 32)
    SID_PAL_MFG_STORE_APP_PUB_ED25519: Tuple[int, int] = (5, 32)
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519: Tuple[int, int] = (6, 32)
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519: Tuple[int, int] = (7, 32)
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE: Tuple[int, int] = (8, 64)
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1: Tuple[int, int] = (9, 32)
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1: Tuple[int, int] = (10, 64)
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE: Tuple[int, int] = (11, 64)
    SID_PAL_MFG_STORE_DAK_PUB_ED25519: Tuple[int, int] = (12, 32)
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE: Tuple[int, int] = (13, 64)
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL: Tuple[int, int] = (14, 4)
    SID_PAL_MFG_STORE_DAK_PUB_P256R1: Tuple[int, int] = (15, 64)
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE: Tuple[int, int] = (16, 64)
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL: Tuple[int, int] = (17, 4)
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519: Tuple[int, int] = (18, 32)
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE: Tuple[int, int] = (19, 64)
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL: Tuple[int, int] = (20, 4)
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1: Tuple[int, int] = (21, 64)
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE: Tuple[int, int] = (22, 64)
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL: Tuple[int, int] = (23, 4)
    SID_PAL_MFG_STORE_MAN_PUB_ED25519: Tuple[int, int] = (24, 32)
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE: Tuple[int, int] = (25, 64)
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL: Tuple[int, int] = (26, 4)
    SID_PAL_MFG_STORE_MAN_PUB_P256R1: Tuple[int, int] = (27, 64)
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE: Tuple[int, int] = (28, 64)
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL: Tuple[int, int] = (29, 4)
    SID_PAL_MFG_STORE_SW_PUB_ED25519: Tuple[int, int] = (30, 32)
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE: Tuple[int, int] = (31, 64)
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL: Tuple[int, int] = (32, 4)
    SID_PAL_MFG_STORE_SW_PUB_P256R1: Tuple[int, int] = (33, 64)
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE: Tuple[int, int] = (34, 64)
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL: Tuple[int, int] = (35, 4)
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519: Tuple[int, int] = (36, 32)
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1: Tuple[int, int] = (37, 64)
    SID_PAL_MFG_STORE_APID: Tuple[int, int] = (38, 4)
    SID_PAL_MFG_STORE_CORE_VALUE_MAX: Tuple[int, None] = (4000, None)

    def __init__(self, value: int, size: int) -> None:
        # Overload the value so that the enum value corresponds to the
        # Mfg value
        self._value_ = value
        self.size = size


class StructureHelper:
    def __repr__(self) -> str:
        repr_str = f"{self.__class__.__name__}\n"
        repr_str += f" device_prk-{binascii.hexlify(self.device_prk).upper()}\n"  # type: ignore
        for _ in self.__class__._fields_:  # type: ignore
            field_name = _[0]
            repr_str += f" {field_name}: {binascii.hexlify(getattr(self, field_name)).upper()}\n"  # type: ignore
        return repr_str


class SidCertMfgP256R1Chain(Structure, StructureHelper):

    # pylint: disable=C0326
    _fields_ = [
        ("smsn", c_ubyte * SMSN_SIZE),
        ("device_pub", c_ubyte * P256R1_PUB_SIZE),
        ("device_sig", c_ubyte * SIG_SIZE),
        ("dak_serial", c_ubyte * SERIAL_SIZE),
        ("dak_pub", c_ubyte * P256R1_PUB_SIZE),
        ("dak_sig", c_ubyte * SIG_SIZE),
        ("product_serial", c_ubyte * SERIAL_SIZE),
        ("product_pub", c_ubyte * P256R1_PUB_SIZE),
        ("product_sig", c_ubyte * SIG_SIZE),
        ("man_serial", c_ubyte * SERIAL_SIZE),
        ("man_pub", c_ubyte * P256R1_PUB_SIZE),
        ("man_sig", c_ubyte * SIG_SIZE),
        ("sw_serial", c_ubyte * SERIAL_SIZE),
        ("sw_pub", c_ubyte * P256R1_PUB_SIZE),
        ("sw_sig", c_ubyte * SIG_SIZE),
        ("root_serial", c_ubyte * SERIAL_SIZE),
        ("root_pub", c_ubyte * P256R1_PUB_SIZE),
        ("root_sig", c_ubyte * SIG_SIZE),
    ]

    def __new__(cls, cert_buffer: bytes, priv: bytes):  # type: ignore
        return cls.from_buffer_copy(cert_buffer)

    def __init__(self: SidCertMfgP256R1Chain, cert_buffer: bytes, priv: bytes) -> None:
        self._cert_buffer = cert_buffer
        _device_prk = bytearray(binascii.unhexlify(priv))
        """
        Sometimes cloud generates p256r1 private key with an invalid preceding
        00, handle that case
        """
        if len(_device_prk) == PRK_SIZE + 1 and _device_prk[0] == 00:
            print(f"P256R1 private key size is {PRK_SIZE+1}, truncate to {PRK_SIZE}")
            del _device_prk[0]

        self.device_prk = bytes(_device_prk)

        assert len(self.device_prk) == PRK_SIZE, "Invalid P256R1 private key size -{} Expected Size -{}".format(
            len(self.device_prk), PRK_SIZE
        )


class SidCertMfgED25519Chain(Structure, StructureHelper):

    # pylint: disable=C0326
    _fields_ = [
        ("smsn", c_ubyte * SMSN_SIZE),
        ("device_pub", c_ubyte * ED25519_PUB_SIZE),
        ("device_sig", c_ubyte * SIG_SIZE),
        ("dak_serial", c_ubyte * SERIAL_SIZE),
        ("dak_pub", c_ubyte * ED25519_PUB_SIZE),
        ("dak_sig", c_ubyte * SIG_SIZE),
        ("product_serial", c_ubyte * SERIAL_SIZE),
        ("product_pub", c_ubyte * ED25519_PUB_SIZE),
        ("product_sig", c_ubyte * SIG_SIZE),
        ("man_serial", c_ubyte * SERIAL_SIZE),
        ("man_pub", c_ubyte * ED25519_PUB_SIZE),
        ("man_sig", c_ubyte * SIG_SIZE),
        ("sw_serial", c_ubyte * SERIAL_SIZE),
        ("sw_pub", c_ubyte * ED25519_PUB_SIZE),
        ("sw_sig", c_ubyte * SIG_SIZE),
        ("root_serial", c_ubyte * SERIAL_SIZE),
        ("root_pub", c_ubyte * ED25519_PUB_SIZE),
        ("root_sig", c_ubyte * SIG_SIZE),
    ]

    def __new__(cls, cert_buffer: bytes, priv: bytes):  # type: ignore
        return cls.from_buffer_copy(cert_buffer)

    def __init__(self: SidCertMfgED25519Chain, cert_buffer: bytes, priv: bytes):
        self._cert_buffer = cert_buffer
        self.device_prk = binascii.unhexlify(priv)
        assert len(self.device_prk) == PRK_SIZE, "Invalid ED25519 private key size -{} Expected Size -{}".format(
            len(self.device_prk), PRK_SIZE
        )


class SidCertMfgCert:
    @staticmethod
    def from_base64(
        cert: bytes, priv: bytes, is_p256r1: bool = False
    ) -> Union[SidCertMfgP256R1Chain, SidCertMfgED25519Chain]:
        if is_p256r1:
            return SidCertMfgP256R1Chain(base64.b64decode(cert), priv)
        return SidCertMfgED25519Chain(base64.b64decode(cert), priv)


class SidMfgObj:
    def __init__(
        self: SidMfgObj,
        mfg_enum: SidMfgValueId,
        value: Any,
        info: dict[str, int],
        skip: bool = False,
        word_size: int = 4,
        is_network_order: bool = True,
    ):
        assert isinstance(word_size, int)
        assert (word_size > 0 and info) or (word_size == 0 and not info)

        _info = Prodict.from_dict(info) if isinstance(info, dict) else info

        self._name: str = mfg_enum.name
        self._value: Any = value
        self._start: int = 0 if not _info else _info.start
        self._end: int = 0 if not _info else _info.end
        self._word_size: int = word_size
        self._id_val: int = mfg_enum.value
        self._skip: bool = skip

        if info:
            assert self._start < self._end, "Invalid {}  end offset: {} < start offset: {}".format(
                self._name, self._end, self._start
            )
            byte_len = self.end - self.start
        else:
            byte_len = len(value)

        self._encoded: bytes = bytes(bytearray())
        if isinstance(self._value, int):
            self._encoded = (self._value).to_bytes(byte_len, byteorder="big" if is_network_order else "little")
        elif isinstance(self._value, bytes):
            self._encoded = self._value
        elif isinstance(self._value, bytearray):
            self._encoded = bytes(self._value)
        elif isinstance(self._value, str):
            self._encoded = bytes(self._value, "ascii")
        else:
            try:
                self._encoded = bytes(self._value)
            except TypeError as ex:
                raise ValueError("{} Cannot convert value {} to bytes".format(self._name, self._value)) from ex

        if len(self._encoded) < byte_len:
            self._encoded = self._encoded.ljust(byte_len, b"\x00")

        if len(self._encoded) != byte_len:
            ex_str = "Field {} value {} len {} mismatch expected field value len {}".format(
                self._name, self._value, len(self._encoded), byte_len
            )
            raise ValueError(ex_str)

        if byte_len != mfg_enum.size:
            print(f"{self} has incorrect size {byte_len} expected {mfg_enum.size}")

    @property
    def name(self: SidMfgObj) -> str:
        return self._name

    @property
    def start(self: SidMfgObj) -> int:
        return self._start * self._word_size

    @property
    def end(self: SidMfgObj) -> int:
        return self._end * self._word_size

    @property
    def encoded(self: SidMfgObj) -> bytes:
        return self._encoded

    @property
    def id_val(self: SidMfgObj) -> int:
        return self._id_val

    @property
    def skip(self: SidMfgObj) -> bool:
        return self._skip

    def __repr__(self: SidMfgObj) -> str:
        val: Any = None
        if isinstance(self._value, str):
            val = self._value
        elif isinstance(self._value, int):
            val = self._value
        else:
            val = binascii.hexlify(self._encoded).upper()

        return f"{self._name}[{self.start}:{self.end}] : {val}"


class SidMfg:
    def __init__(self: SidMfg, app_pub: Union[None, bytes], config: Prodict, is_network_order: bool) -> None:
        self._config = config
        self._app_pub: Optional[bytes] = app_pub
        self._apid: Optional[str] = None
        self._is_network_order: bool = is_network_order
        self._mfg_objs: List[SidMfgObj] = []
        self._word_size: int = 0 if not self._config else self._config.offset_size

    def __iter__(self: SidMfg) -> Iterator[SidMfgObj]:
        return iter(sorted(self._mfg_objs, key=lambda mfg_obj: mfg_obj.id_val))

    def __repr__(self: SidMfg) -> str:
        value = f"{str(self._ed25519)} \n{str(self._p256r1)} \n"  # type: ignore
        value += "SID Values\n"
        value += "\n".join([f" {str(_)}" for _ in self._mfg_objs])
        value += "\n"
        return value

    def append(self: SidMfg, mfg_enum: SidMfgValueId, value: Any, can_skip: bool = False) -> None:
        try:
            offset_config = 0 if not self._config else self._config.mfg_offsets[mfg_enum.name]
            mfg_obj = SidMfgObj(
                mfg_enum,
                value,
                offset_config,
                skip=can_skip,
                word_size=self._word_size,
                is_network_order=self._is_network_order,
            )
            self._mfg_objs.append(mfg_obj)
        except KeyError as ex:
            if can_skip:
                print("Skipping {}".format(mfg_enum.name))
            else:
                raise ex
        except Exception:
            traceback.print_exc()
            exit(1)


class SidMfgBBJson(SidMfg):
    def __init__(self: SidMfgBBJson, bb_json: Any, config: Prodict, is_network_order: bool = True) -> None:
        super().__init__(app_pub=None, config=config, is_network_order=is_network_order)

        _bb_json = Prodict.from_dict(bb_json)

        def unhex(unhex_val: str) -> bytes:
            return binascii.unhexlify(unhex_val)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, "SID0", can_skip=True)
        if self._config:
            self.append(SidMfgValueId.SID_PAL_MFG_STORE_VERSION, self._config.mfg_page_version, can_skip=True)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVID, unhex(_bb_json.ringNetDevId))
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519, unhex(_bb_json.PKI.device_cert.ed25519_priv))
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519, unhex(_bb_json.PKI.device_cert.ed25519_pub))
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE,
            unhex(_bb_json.PKI.device_cert.ed25519_signature),
        )
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, unhex(_bb_json.PKI.device_cert.p256r1_priv))
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, unhex(_bb_json.PKI.device_cert.p256r1_pub))
        self.append(
            SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE,
            unhex(_bb_json.PKI.device_cert.p256r1_signature),
        )

        for cert in _bb_json.PKI.intermediate_certs:
            _cert = Prodict.from_dict(cert)
            if _cert.cert_name == "AMZN":
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_ED25519, unhex(_cert.ed25519_pub))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_P256R1, unhex(_cert.p256r1_pub))
            elif _cert.cert_name == "MAN":
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519, unhex(_cert.ed25519_pub))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE, unhex(_cert.ed25519_signature))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_ED25519_SERIAL, unhex(_cert.ed25519_serial))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1, unhex(_cert.p256r1_pub))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE, unhex(_cert.p256r1_signature))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_P256R1_SERIAL, unhex(_cert.p256r1_serial))
            elif _cert.cert_name == "MODEL":
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519, unhex(_cert.ed25519_pub))
                self.append(
                    SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE, unhex(_cert.ed25519_signature)
                )
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL, unhex(_cert.ed25519_serial))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1, unhex(_cert.p256r1_pub))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE, unhex(_cert.p256r1_signature))
                self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL, unhex(_cert.p256r1_serial))


class SidMfgAcsJson(SidMfg):
    def __init__(
        self: SidMfgAcsJson, acs_json: Any, app_pub: bytes, config: Prodict, is_network_order: bool = True
    ) -> None:
        super().__init__(app_pub=app_pub, config=config, is_network_order=is_network_order)

        _acs_json = Prodict.from_dict(acs_json)
        self._ed25519 = SidCertMfgCert.from_base64(_acs_json.eD25519, _acs_json.metadata.devicePrivKeyEd25519)
        self._p256r1 = SidCertMfgCert.from_base64(
            _acs_json.p256R1, _acs_json.metadata.devicePrivKeyP256R1, is_p256r1=True
        )
        self._apid = _acs_json.metadata.apid
        self._smsn = binascii.unhexlify(_acs_json.metadata.smsn)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, "SID0", can_skip=True)
        if self._config:
            self.append(SidMfgValueId.SID_PAL_MFG_STORE_VERSION, self._config.mfg_page_version, can_skip=True)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SMSN, self._smsn)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APID, self._apid)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APP_PUB_ED25519, self._app_pub)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519, self._ed25519.device_prk)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519, self._ed25519.device_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE, self._ed25519.device_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, self._p256r1.device_prk)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, self._p256r1.device_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE, self._p256r1.device_sig)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519, self._ed25519.dak_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE, self._ed25519.dak_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_ED25519_SERIAL, self._ed25519.dak_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1, self._p256r1.dak_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE, self._p256r1.dak_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_P256R1_SERIAL, self._p256r1.dak_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519, self._ed25519.product_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE, self._ed25519.product_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL, self._ed25519.product_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1, self._p256r1.product_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE, self._p256r1.product_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL, self._p256r1.product_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519, self._ed25519.man_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE, self._ed25519.man_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_ED25519_SERIAL, self._ed25519.man_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1, self._p256r1.man_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE, self._p256r1.man_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_P256R1_SERIAL, self._p256r1.man_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519, self._ed25519.sw_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE, self._ed25519.sw_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_ED25519_SERIAL, self._ed25519.sw_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1, self._p256r1.sw_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE, self._p256r1.sw_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_P256R1_SERIAL, self._p256r1.sw_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_ED25519, self._ed25519.root_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_P256R1, self._p256r1.root_pub)


class SidMfgAwsJson(SidMfg):
    def __init__(
        self: SidMfgAwsJson,
        aws_wireless_device_json: Any,
        aws_device_profile_json: Any,
        config: Prodict,
        is_network_order: bool = True,
    ) -> None:
        super().__init__(app_pub=None, config=config, is_network_order=is_network_order)

        _aws_wireless_device_json = Prodict.from_dict(aws_wireless_device_json)
        _aws_device_profile_json = Prodict.from_dict(aws_device_profile_json)

        def get_value(crypt_keys: Any, key_type: str) -> Any:
            for _ in crypt_keys:
                _ = Prodict.from_dict(_)
                if _.SigningAlg == key_type:
                    return _.Value
            return None

        def unhex(unhex_val: str) -> bytes:
            return binascii.unhexlify(unhex_val)

        self._ed25519 = SidCertMfgCert.from_base64(
            get_value(_aws_wireless_device_json.Sidewalk.DeviceCertificates, "Ed25519"),
            get_value(_aws_wireless_device_json.Sidewalk.PrivateKeys, "Ed25519"),
        )
        self._p256r1 = SidCertMfgCert.from_base64(
            get_value(_aws_wireless_device_json.Sidewalk.DeviceCertificates, "P256r1"),
            get_value(_aws_wireless_device_json.Sidewalk.PrivateKeys, "P256r1"),
            is_p256r1=True,
        )

        self._apid = _aws_device_profile_json.Sidewalk.ApId
        self._smsn = unhex(_aws_wireless_device_json.Sidewalk.SidewalkManufacturingSn)
        self._app_pub = unhex(_aws_device_profile_json.Sidewalk.ApplicationServerPublicKey)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAGIC, "SID0", can_skip=True)
        if self._config:
            self.append(SidMfgValueId.SID_PAL_MFG_STORE_VERSION, self._config.mfg_page_version, can_skip=True)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SMSN, self._smsn)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APID, self._apid)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_APP_PUB_ED25519, self._app_pub)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519, self._ed25519.device_prk)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519, self._ed25519.device_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE, self._ed25519.device_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1, self._p256r1.device_prk)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1, self._p256r1.device_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE, self._p256r1.device_sig)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519, self._ed25519.dak_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE, self._ed25519.dak_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_ED25519_SERIAL, self._ed25519.dak_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1, self._p256r1.dak_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE, self._p256r1.dak_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_DAK_P256R1_SERIAL, self._p256r1.dak_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519, self._ed25519.product_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE, self._ed25519.product_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL, self._ed25519.product_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1, self._p256r1.product_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE, self._p256r1.product_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL, self._p256r1.product_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519, self._ed25519.man_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE, self._ed25519.man_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_ED25519_SERIAL, self._ed25519.man_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1, self._p256r1.man_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE, self._p256r1.man_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_MAN_P256R1_SERIAL, self._p256r1.man_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519, self._ed25519.sw_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE, self._ed25519.sw_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_ED25519_SERIAL, self._ed25519.sw_serial)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1, self._p256r1.sw_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE, self._p256r1.sw_sig)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_SW_P256R1_SERIAL, self._p256r1.sw_serial)

        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_ED25519, self._ed25519.root_pub)
        self.append(SidMfgValueId.SID_PAL_MFG_STORE_AMZN_PUB_P256R1, self._p256r1.root_pub)


class SidMfgOutBin:
    def __init__(self: SidMfgOutBin, file_name: str, config: Prodict) -> None:
        self._file_name = file_name
        self._config = config
        self._encoded = bytearray()

    def __enter__(self: SidMfgOutBin) -> SidMfgOutBin:
        path = Path(self._file_name)
        self._file = open(self._file_name, "rb+") if path.is_file() else open(self._file_name, "wb+")
        self._encoded = bytearray(self._file.read())
        _encoded_size = self._config.mfg_page_size * self._config.offset_size
        if len(self._encoded) < _encoded_size:
            self._encoded.extend(bytearray(b"\xff") * (_encoded_size - len(self._encoded)))
        return self

    def __exit__(self: SidMfgOutBin, type: Any, value: Any, traceback: Any) -> None:
        self._file.seek(0)
        self._file.write(self._encoded)
        self._file.close()

    def write(self: SidMfgOutBin, sid_mfg: SidMfg, config: Prodict) -> None:
        encoded_len = len(self._encoded)
        for _ in sid_mfg:
            if encoded_len <= _.end:
                ex_str = "Cannot fit Field-{} in mfg page, mfg_page_size has to be at least {}".format(
                    _.name, int(_.end / self._config.offset_size) + 1
                )
                raise Exception(ex_str)
            self._encoded[_.start : _.end] = _.encoded

            if encoded_len != len(self._encoded):
                raise Exception("Encoded Length Changed")


class SidMfgOutNVM3:
    def __init__(self: SidMfgOutNVM3, file_name: str):
        self._file_name = file_name
        self._objs: List[str] = list()

    def __enter__(self: SidMfgOutNVM3) -> SidMfgOutNVM3:
        self._file = open(self._file_name, "w+")
        return self

    def __exit__(self: SidMfgOutNVM3, type: Any, value: Any, traceback: Any) -> None:
        self._file.write("\n".join(self._objs))
        self._file.close()

    def write(self: SidMfgOutNVM3, sid_mfg: SidMfg) -> None:
        if sid_mfg is None:
            raise Exception("mfg is not valid")
        for obj in sid_mfg:
            if not obj.skip:
                self._objs.append(
                    "0x{:04x}:OBJ:{}".format(SidMfgValueId[obj.name].value, binascii.hexlify(obj.encoded).decode())
                )


def main() -> None:
    def str2bool(val: Union[bool, str]) -> bool:
        if isinstance(val, bool):
            return val
        if val.lower() in ("yes", "true", "t", "y", "1"):
            return True
        if val.lower() in ("no", "false", "f", "n", "0"):
            return False
        raise argparse.ArgumentTypeError("Boolean value expected.")

    class _HelpAction(argparse._HelpAction):
        def __call__(self, parser, namespace, values, option_string=None):  # type: ignore
            parser.print_help()
            subparsers_actions = [
                action for action in parser._actions if isinstance(action, argparse._SubParsersAction)
            ]
            for subparsers_action in subparsers_actions:
                for choice, subparser in subparsers_action.choices.items():
                    print(subparser.format_help())
            parser.exit()

    common_args = argparse.ArgumentParser(add_help=False)

    common_args.add_argument(
        "--config",
        type=argparse.FileType("r"),
        required=False,
        help="Config Yaml that defines the mfg page offsets",
    )
    common_args.add_argument(
        "--output_bin",
        type=str,
        required=False,
        help="""Output bin file, if this file does not exist
-                             it will be created, if it does exist the data at
-                             the offsets defined in the config file will be
-                             overwritten by provision data""",
    )
    common_args.add_argument(
        "--output_sl_nvm3",
        type=str,
        required=False,
        help="""Create Silabs NVM3 file""",
    )
    common_args.add_argument(
        "--is_network_order",
        type=str2bool,
        required=False,
        default=True,
        nargs="?",
        const=True,
        help="Controls endianess in which integers are stored",
    )
    common_args.add_argument(
        "--dump_raw_values",
        action="store_true",
        default=False,
        help="Dump the raw values for debugging",
    )

    parser = argparse.ArgumentParser(
        description="""Generate bin file with
                                     sidewalk certificates""",
        add_help=False,
    )
    parser.add_argument(
        "-h", "--help", action=_HelpAction, help="help for help if you need some help"
    )  # add custom help

    subparsers = parser.add_subparsers()

    acs_args = subparsers.add_parser("acs", help="Arguments for ACS Console Input", parents=[common_args])
    acs_args.set_defaults(group="acs")
    acs_args.add_argument(
        "--json",
        type=argparse.FileType("r"),
        required=True,
        help="ACS Console JSON file",
    )
    acs_args.add_argument(
        "--app_srv_pub",
        type=argparse.FileType("rb"),
        required=True,
        help="App server public key",
    )

    bb_args = subparsers.add_parser("bb", help="Arguments for Black Box Input", parents=[common_args])
    bb_args.set_defaults(group="bb")
    bb_args.add_argument(
        "--json",
        type=argparse.FileType("r"),
        required=True,
        help="Black Box Sidewalk Response JSON file",
    )

    aws_args = subparsers.add_parser("aws", help="Arguments for Aws Input", parents=[common_args])
    aws_args.set_defaults(group="aws")
    aws_args.add_argument(
        "--wireless_device_json",
        type=argparse.FileType("r"),
        required=True,
        help="Json Response of 'aws iotwireless get-wireless-device' ",
    )
    aws_args.add_argument(
        "--device_profile_json",
        type=argparse.FileType("r"),
        required=True,
        help="Json response of 'aws iotwireless get-device-profile ...' ",
    )

    args = parser.parse_args()
    if args == argparse.Namespace():
        parser.print_help()
        parser.exit()

    config = Prodict.from_dict(yaml.safe_load(args.config)) if args.config else None

    if args.group == "acs":
        app_srv_pub = args.app_srv_pub.read()
        acs_json = json.load(args.json)
        sid_mfg = SidMfgAcsJson(
            acs_json=acs_json,
            app_pub=app_srv_pub,
            config=config,
            is_network_order=args.is_network_order,
        )
    elif args.group == "bb":
        bb_json = json.load(args.json)
        sid_mfg = SidMfgBBJson(
            bb_json=bb_json,
            config=config,
            is_network_order=args.is_network_order,
        )
    elif args.group == "aws":
        aws_wireless_device_json = json.load(args.wireless_device_json)
        aws_device_profile_json = json.load(args.device_profile_json)
        sid_mfg = SidMfgAwsJson(
            aws_wireless_device_json=aws_wireless_device_json,
            aws_device_profile_json=aws_device_profile_json,
            config=config,
            is_network_order=args.is_network_order,
        )
    else:
        print("Specified group unsupported!")
        sys.exit()

    if args.dump_raw_values:
        print(sid_mfg)

    outfile = None
    if args.output_bin and not args.config:
        print("output_bin and config need to be given at the same time")
        sys.exit()
    elif not args.output_bin and not args.output_sl_nvm3:
        print("At least one output needs to be given")
        sys.exit()
    elif args.output_bin and args.output_sl_nvm3:
        print("Two outputs cannot be given at the same time")
        sys.exit()
    elif args.output_sl_nvm3:
        outfile = args.output_sl_nvm3
        with SidMfgOutNVM3(outfile) as out:
            out.write(sid_mfg)
    elif args.output_bin and args.config:
        outfile = args.output_bin
        with SidMfgOutBin(outfile, config) as out:
            out.write(sid_mfg, config)
    else:
        print("Specified output options unsupported!")
        sys.exit()

    print("WRITE DONE - {}".format(outfile))


if __name__ == "__main__":
    main()
