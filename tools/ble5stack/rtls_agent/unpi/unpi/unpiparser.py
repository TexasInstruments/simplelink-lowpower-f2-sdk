#
#  Copyright (c) 2018-2019, Texas Instruments Incorporated
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#  *  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#  *  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  *  Neither the name of Texas Instruments Incorporated nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
#  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
#  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
#  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
#  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

import enum
import inspect
import logging
import copy

logger = logging.getLogger('unpiparser')

from dataclasses import dataclass, field
from typing import Iterable, Any

import construct
from construct import Const, Struct, Rebuild, Byte, BitStruct, Nibble, Int8ul, Int16ul, Checksum, RawCopy, Enum, Union, \
    Switch, len_, this, BitsInteger, ConstError, Tell, StreamError, Rebuffered, ChecksumError, EnumInteger, \
    EnumIntegerString, GreedyBytes, Container, ListContainer
from functools import reduce, singledispatch

import json


@singledispatch
def to_serializable(val):
    return str(val)


@dataclass(order=True)
class QMessage:
    priority: int
    item: Any = field(compare=False)

    def __repr__(self):
        return f'QMessage(pri={self.priority}, item={self.item}'


class NpiOriginator(enum.IntEnum):
    Ap = 1
    Nwp = 2
    Any = 3


class UNPITypes(enum.IntEnum):
    SyncReq = 1
    AsyncReq = 2
    SyncRsp = 3


class NpiSubSystems(enum.IntEnum):
    SNP = 0x15
    CM = 0x17
    RTLS = 0x19
    UTIL = 0x07

@dataclass
class UNPIError:
    module: str
    message: str

    def as_json(self):
        return dict(module=self.module, message=self.message)


@dataclass
class UNPIMessage:
    type: UNPITypes
    subsystem: int
    command: int
    data: Iterable
    originator: NpiOriginator = NpiOriginator.Ap
    _construct: Any = None
    payload: Container = None
    dict: dict = None

    @staticmethod
    def from_construct(frame, originator=NpiOriginator.Ap, node_identifier='unknown', node_name='unknown'):
        typ = frame.cmd0.type  # frame.cmd0.type.intvalue
        subcmd = frame.cmd0.subcmd  # frame.cmd0.subcmd.intvalue if type(frame.cmd0.subcmd) == EnumIntegerString else int(frame.cmd0.subcmd)
        cmd = frame.cmd1  # frame.cmd1.intvalue if type(frame.cmd1) == EnumIntegerString else int(frame.cmd1)
        return UNPIMessage(typ, subcmd, cmd, bytes(frame.data), originator, frame)

    def as_construct(self):
        if self._construct: return self._construct
        return None

    def as_json(self):
        return json.dumps(dict(originator=self.originator.name, type=str(self.type), subsystem=self.subsystem, command=self.command, payload=self.payload))

    @staticmethod
    def from_dict(dct):  # A dict resulting from JSON parsing
        if dct['type'] not in [item.name for item in UNPITypes]:
            error = 'Could not find request type %s in available types {%s}' % (dct['type'], [item for item in UNPITypes])
            logging.error(error)
            return UNPIError(__name__, error)

        if dct['subsystem'] not in [item.name for item in NpiSubSystems]:
            error = 'Could not find subsystem %s in available {%s}' % (dct['subsystem'], [item for item in NpiSubSystems])
            logging.error(error)
            return UNPIError(__name__, error)

        return UNPIMessage(originator=NpiOriginator.Ap, type=UNPITypes[dct['type']], subsystem=NpiSubSystems[dct['subsystem']], command=dct['command'], data=[], dict=dct['payload'])

    @staticmethod
    def from_json(js):
        return __class__.from_dict(json.loads(js))

    def __repr__(self):
        return "UNPIMessage(originator={} type={}, subsystem={}, command={}, data={})".format(
            self.originator.name,
            self.type.name if isinstance(self.type, UNPITypes) else self.type,
            self.subsystem.name if isinstance(self.subsystem, NpiSubSystems) else self.subsystem,
            self.command.name if hasattr(self.command, 'name') else self.command,
            ':'.join(['%02X' % x for x in self.data])
        )

    @property
    def header(self):
        return UNPIHeader.from_message(self)


@to_serializable.register(UNPIMessage)
def ts_unpimessage(val):
    return dict(originator=val.originator.name, type=str(val.type), subsystem=val.subsystem, command=val.command, payload=val.payload)


@dataclass
class UNPIHeader:
    type: UNPITypes
    subsystem: int
    command: int

    @staticmethod
    def from_message(message):
        return UNPIHeader(message.type, message.subsystem, message.command)

    def as_int(self):
        return UNPIHeader(UNPITypes(int(self.type)), int(self.subsystem), int(self.command))

    def __hash__(self):
        return hash((self.type, self.subsystem, self.command))


class NiceBytes(construct.Adapter):
    def _decode(self, obj, ctx, path):
        return ":".join(map(lambda x: "%02X" % x, obj))

    def _encode(self, obj, context, path):
        return list(map(lambda x: int(x, 16), obj.split(":")))


class ReverseBytes(construct.SymmetricAdapter):
    def _decode(self, obj, ctx, path):
        return list(reversed(obj))


class NpiRequest:
    """
    Abstract container class/mixin for NpiRequests. A class that subclasses this class should also
    subclass the other mixin classes like AsyncReq|SyncReq|SyncRsp, FromNwp|FromAp|FromAny.

    Provides some helper functions such as
    parse  - call parse on the `struct` class member
    header - returns the UNPIHeader signature of the NpiRequest subclass
    build  - given the arguments in the same order as the `struct` member, calls build
    """

    @classmethod
    def parse(cls, payload):
        """
        Boilerplate function for NpiRequest classes to call parse on the payload struct

        :param payload: [bytes()] that should be parsed according to the classes struct
        :return: [Container]
        """
        ret = {}
        if not hasattr(cls, 'struct'):
            return ret
        try:
            ret = cls.struct.parse(payload)
        except StreamError as e:
            logging.error("Unexpected payload received for message %s: %s.", cls.__name__, e)
        finally:
            return ret


    @classmethod
    def header(cls):
        """
        :return: The uNpi header signature for this request
        """
        return UNPIHeader(cls.type, cls.subsystem, cls.command)

    @classmethod
    def build(cls, *args, **kwargs):
        """
        Generic build function, takes a list of arguments and maps them to the struct fields. Should also be able
        to accept keyword arguments.

        :param args: Arguments to each of the struct fields, in order.
        :param kwargs: Possibly same as the above, but named
        :return: [bytes()] complete serial uNpi frame
        """
        # Give higher priority to Sync messages
        priority = 2 if cls.type == UNPITypes.AsyncReq else 1

        if cls.struct:
            # Build a dict of the arg values based on the order of the payload fields
            arg_names = [x.name for x in cls.struct.subcons][:len(args)]
            argsdict = {k: v for k, v in zip(arg_names, args)}
            # Build message based on type, ss, command and payload in concrete class
            data = cls.struct.build({**argsdict, **kwargs})
            payload = cls.struct.parse(data)
        else:
            data = b''
            payload = None

        msg = UNPIMessage(cls.type, cls.subsystem, cls.command, data, payload=payload)
        return QMessage(priority, msg)


class SubSysMeta(type):
    """
    Metaclass to give inner NpiRequest classes the subsystem type of the containing class.
    """

    def __init__(cls, name, bases, dct):
        if any([b._NpiSubSystem__is_subsystem for b in bases]):  # Then it's a concrete subsystem class
            ss_type = cls.type
            commands = cls.responses()

            for c in commands:
                c.subsystem = ss_type
                c.subsystemclass = cls


class NpiSubSystem(metaclass=SubSysMeta):
    """
    Container class for NPI subsystems, gives a namespace and a couple of helper functions such as parse.
    """

    __is_subsystem = True

    defaultParse = Struct(
        'raw_payload' / NiceBytes(GreedyBytes)
    )

    @classmethod
    def responses(cls):
        """
        :return: List of NpiRequest inner classes in the NpiSubSystem class
        """
        return [attr[1] for attr in inspect.getmembers(cls, inspect.isclass) if issubclass(attr[1], NpiRequest)]

    @classmethod
    def parse(cls, msg: UNPIMessage):
        """
        Calls the parse function of an inner NpiRequest class based on the NPI header "signature"
        :param msg: An NPI frame with the payload unparsed (data field exists)
        :return: Parsed payload [class Container from construct]
        """
        responses = {c.header().as_int(): c for c in cls.responses() if c.originator == NpiOriginator.Nwp}
        parser = responses.get(UNPIHeader.from_message(msg).as_int(), cls.defaultParse)
        parsed = parser.parse(msg.data)
        # Delete all underscored things
        # We don't want to pass this on at this point. Why is it even there after parsing is done?
        def delete_io_recursive(container):
            if hasattr(container, 'keys'):
                kk = list(filter(lambda x: x[0] == '_', filter(lambda x: x[:2] != '__', container.keys())))
                for k in kk:
                    container.pop(k)  # delete all "special" keys starting with _Xxx
                for k, v in container.items():
                    if type(v) == Container:
                        delete_io_recursive(v)
                    if type(v) == ListContainer:
                        for item in [x for x in v if type(x) == Container]:
                            delete_io_recursive(item)
        delete_io_recursive(parsed)
        return parsed

    @classmethod
    def build_from_json(cls, msg: UNPIMessage):
        responses = {c.header().as_int(): c for c in cls.responses()}  # if c.originator == NpiOriginator.Ap}
        # Hacky way of finding the proper Command enum from the string
        try:
            command = next((v.command for v in responses.values() if v.command.name == msg.command))
        except StopIteration:
            error = "Could not find command %s from available {%s}" % (msg.command, [v.command.name for v in responses.values()])
            logging.error(error)
            return UNPIError(__name__, error)
        msg.command = command
        builder = responses.get(UNPIHeader.from_message(msg).as_int(), cls.defaultParse)
        built = builder.build(**msg.dict)
        msg.data = built.item.data
        return QMessage(built.priority, msg)


class UNPIParser:
    def __init__(self, subsystems_and_commands_dict, types=None, max_pkt_len=1024, only_known_subsys=True):
        """
        Constructor for a uNPI parser instance. Need a dictionary of NpiSubSystems enum keys where each value is
        an enum class containing the commands of the subsystem

        :param subsystems_and_commands_dict: {[NpiSubSystems]: Commands enum}
        :param types: Optional, to replace the NPI asyncReq, syncReq, syncRsp types
        :param max_pkt_len: Reject message if length is more than this, assuming bad data in that case
        :param only_known_subsys: Reject message if header includes an unknown uNPI SybSystem, assuming bad data.
        """
        if not types:
            types = {t.name: t.value for t in list(UNPITypes)}

        self.maxLen = max_pkt_len
        self.only_known_ss = only_known_subsys
        self.req = Enum(BitsInteger(3), **types)
        self.cmd_class = Enum(BitsInteger(5), **{ss.name: ss.value for ss in subsystems_and_commands_dict})
        # self.command = Enum(Int8ul, **commands)

        commandswitch = Switch(lambda ctx: int(ctx.cmd0.subcmd), {int(ss): Enum(Int8ul, cmds) for ss, cmds in subsystems_and_commands_dict.items()}, default=Int8ul)
        self.subsystems_and_commands = subsystems_and_commands_dict
        self.unpi_frame = Struct(
            "sof" / Const(0xFE, Int8ul),
            "length" / Rebuild(Int16ul, len_(this.data)),
            "cmd0" / BitStruct(
                "type" / self.req,
                "subcmd" / self.cmd_class
            ),
            "cmd1" / commandswitch,
            "hdroffset" / Tell,
            "data" / Byte[this.length],
            "fcs" / Checksum(Byte,
                             lambda data: reduce((lambda cur, prev: cur ^ prev), data),
                             self.read_stream_for_fcs(offset=1))
        )

    def read_stream_for_fcs(self, offset):
        """
        Helper function that accesses the raw bytestream in order to calculate the FCS field correctly

        :param offset: Where to start reading
        :return: func that returns a byte string that FCS should be calculated on
        """
        def read_stream_offset(ctx):
            ctx._io.seek(offset)
            readLen = ctx.hdroffset + ctx.length  # Don't read more from stream than needed
            return ctx._io.read(readLen) if ctx._building else ctx._io.read(readLen-1)

        return read_stream_offset

    def parse(self, data):
        """
        Parse a bytestring into a uNPI frame [Container], does not parse payload

        :param data: Input bytes
        :return: Parsed container
        """
        return self.unpi_frame.parse(bytes(data))

    def build(self, req_type, cmd_type, cmd, data):
        """
        Build an UNPI frame given command type, subsystem, command and data

        :param req_type: 3 bits
        :param cmd_type: 5 bits
        :param cmd: 8 bits
        :param data: Anything that can be cast to bytes()
        :return: bytes() serial frame
        """
        return self.unpi_frame.build(dict(cmd0=dict(type=req_type, subcmd=cmd_type), cmd1=cmd, data=bytes(data)))

    def parse_stream(self, data):
        """
        Tries to parse incoming bytes into an uNPI frame. Returns a tuple of [Container], [bytes] where [Container] can
        be None and [bytes] is the input data that was not consumed.

        Will consume bytes up until SOF in any case.

        :param data: Received bytes, could contain partial or complete or multiple actual frames
        :return: (Container|None, bytes)
        """
        if not data or len(data) == 0:
            return None, data

        unpi_header = Struct(
            "sof" / Const(0xFE, Int8ul),
            "length" / Int16ul,
            "cmd0" / Int8ul,
            "cmd1" / Int8ul,
        )

        temp = data[:]
        while len(temp):
            try:
                hdr = unpi_header.parse(bytes(temp))
                if (hdr.length > self.maxLen)\
                        or (hdr.cmd0 >> 5 not in [1, 2, 3])\
                        or (self.only_known_ss and (hdr.cmd0 & 0x1f) not in self.subsystems_and_commands.keys()):
                    temp = temp[1:]
                    continue

                p = self.parse(temp)
                msg = UNPIMessage.from_construct(p, originator=NpiOriginator.Nwp)
                temp = temp[p.length + p.hdroffset + 1:]
                return msg, temp
            except ConstError as e:  # Did not find Start-of-frame
                temp = temp[1:]
                logging.warning(repr(e))
                # logging.error("Expected Start-of-Frame character, got something else. Skipping.")
            except StreamError as e:
                logging.debug(repr(e) + "  -- Likely not enough bytes received in this read, waiting for more.")
                return None, temp
            except ChecksumError as e:
                temp = temp[1:]
                logging.error(repr(e))
            except Exception as e:
                logging.error("Unexpected error")
                logging.error(repr(e))
                raise e
        return None, temp


if __name__ == '__main__':
    class MySubSystems(enum.IntEnum):
        SNP = 0x15

    class MySNPCommands(enum.IntEnum):
        GAP_START_ADV = 0x42
        GAP_STOP_ADV = 0x44
        GATT_ADD_SERVICE = 0x81

    parser = UNPIParser({MySubSystems.SNP: MySNPCommands})

    # print(parser.parse([0xFE, 0x01, 0x00, 0x15 | 0x2 << 5, 0x81, 0x12, 0xC7]))
    #
    # print(parser.build(parser.req.SyncReq, parser.cmd_class.SNP, parser.command.GAP_START_ADV, data=bytes(b'\x08\x09')))
    #
    # print(parser.parse(b'\xfe\x02\x005B\x08\tt'))

    print("==== Parsing stream")
    stream = [0x81, 0x12, 0xC7] + [0xFE, 0x01, 0x00, 0x15 | 0x2 << 5, 0x81, 0xFF, 0x2A] + [0xAB, 0xBA] + [0xFE, 0x01, 0x00, 0x15 | 0x2 << 5] + [0x81, 0xEE, 0x3B]
    s = stream[:]

    #stream = 'FE:06:00:57:04:01:D9:00:15:DD:0C:49:FE:06:00:57:04:01:C6:97:18:DD:0C:CC'
    #s = [int(x, 16) for x in stream.split(':')]

    # unpi_header = Struct(
    #     "sof" / Const(0xFE, Int8ul),
    #     "length" / Int16ul,
    #     "cmd0" / Int8ul,
    #     "cmd1" / Int8ul,
    # )
    #hdr = unpi_header.parse(bytes(s))
    #print(hdr)



    p = None
    while True:
        p, s = parser.parse_stream(s)
        if p is None:
            break
        g = p
        print(p)


