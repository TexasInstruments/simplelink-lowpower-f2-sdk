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

import asyncio
import logging

logger = logging.getLogger('rtlsnode')

import queue
import sys
import threading
import time
import typing
from typing import Tuple
from dataclasses import dataclass, field
from queue import PriorityQueue

from unpi.serialnode import SerialNode

from .ss_rtls import RTLS, Commands as RTLSCommands, DeviceFamily
from .ss_assert import UTIL, Commands as UTILCommands

import json
import copy

from unpi.unpiparser import UNPIHeader, NpiSubSystems, UNPIMessage, QMessage, to_serializable


@dataclass(order=True)
class NodeMessage:
    identifier: typing.Any
    message: QMessage = field(compare=False)

    def as_json(self):
        return json.dumps(self, default=to_serializable)

    @staticmethod
    def from_json(js):
        dct = json.loads(js)
        return NodeMessage(identifier=dct['identifier'],
                           message=UNPIMessage.from_dict(dct['message']))

    def as_tuple(self) -> Tuple[str, int, UNPIMessage]:
        return self.identifier, self.message.priority, self.message.item

    def __repr__(self):
        return "NodeMessage(identifier={} message={})".format(self.identifier, self.message.item)


@to_serializable.register(NodeMessage)
def ts_node_message(val):
    return dict(identifier=val.identifier, message=val.message.item)


class SendReceipt:
    def __init__(self, rtlsnode, header):
        self.node = rtlsnode
        self.header = header

    def pend(self, block=True, timeout=2):
        return self.node.recv(block, timeout)

    def __repr__(self):
        return repr(self.header)


@dataclass
class Subscriber:
    queue: typing.Union[queue.PriorityQueue, asyncio.PriorityQueue]
    interest: UNPIHeader = None
    transient: bool = False
    eventloop: asyncio.BaseEventLoop or None = None

    def pend(self, block=True, timeout=2):
        return self.queue.get(block, timeout)


class RTLSNode(threading.Thread):
    subsystems = [RTLS, UTIL]  # [class(NpiSubSystem)]

    subsystem_map = {
        NpiSubSystems.RTLS: RTLSCommands,  # Enum(NpiSubSystems): [Enum(Commands)]
        NpiSubSystems.UTIL: UTILCommands
    }

    def __init__(self, port, speed, name=None):
        super().__init__(name=name)
        self.name = name
        self.inQueue = PriorityQueue()
        self.outQueue = PriorityQueue()
        self.serial = SerialNode(port, speed, self.inQueue, self.outQueue, RTLSNode.subsystem_map, name=name)
        self.exception = None

        self.stopEvent = threading.Event()
        self.identifyEvent = threading.Event()
        self.subscribers = []

        self.identifier = None
        self.capabilities = {}
        self.devId = None
        self.revNum = None
        self.port = port

        self.manager = None

        self.cci_started = False

        self.tof_calib_info = None
        self.tof_calib_read_return = False
        self.tof_calibration_configured = False
        self.tof_calibrated = False
        self.tof_initialized = False
        self.seed_initialized = False
        self.tof_started = False

        self.hadm_initialized = False

        self.aoa_initialized = False
        self.aoa_started = False

        self.connection_in_progress = False
        self.device_resets = False

        # Subsystem helpers
        self.rtls = RTLS(lambda x: self.send(x))

    @property
    def stopped(self):
        return self.stopEvent.is_set()

    def stop(self):
        self.stopEvent.set()
        self.serial.stop()

    def run(self):
        try:
            self.serial.start()
            if not self.serial.startedEvent.wait(10):
                self.stop()
                if self.serial.exception:
                    self.exception = self.serial.exception
                return

            # Identify
            self.rtls.identify()
            timeout = time.time() + 0.5
            while time.time() < timeout:
                try:
                    item = self.inQueue.get(block=True, timeout=0.5)
                    # msg = __class__.parse(item.item)
                    msg = self.parse(item.item)
                    if msg.command == RTLSCommands.RTLS_CMD_IDENTIFY.name:
                        self.identifier = msg.payload.identifier
                        self.capabilities = msg.payload.capabilities
                        self.devId = msg.payload.devId
                        self.revNum = msg.payload.revNum
                        if self.manager is not None: self.manager.identify_node(self, self.identifier)
                        self.identifyEvent.set()
                        break
                    else:
                        pass  # discard anything coming while we wait for identify
                except queue.Empty:
                    pass

            # Wait for messages
            while not self.stopped:
                try:
                    item = self.inQueue.get(block=True, timeout=0.5)
                    self.inQueue.task_done()
                    msg = item.item
                    # msg = __class__.parse(msg)
                    msg = self.parse(msg)
                    msg.node_identifier = self.identifier
                    msg.node_name = self.name
                    parsedItem = QMessage(item.priority, msg)
                    logger.debug("Have %d subscribers for %s" % (len(self.subscribers), msg))
                    for subscriber in self.subscribers:
                        if subscriber.eventloop:
                            asyncio.run_coroutine_threadsafe(subscriber.queue.put(parsedItem), subscriber.eventloop)
                        else:
                            subscriber.queue.put(parsedItem)
                        if subscriber.transient:
                            self.subscribers.remove(subscriber)
                    if self.manager is not None:
                        self.manager.message_from_node(self, parsedItem)
                except queue.Empty:
                    pass
        finally:
            self.stop()

    def send(self, msg):
        self.parse_outgoing_msg(msg)
        self.outQueue.put(msg)
        return SendReceipt(self, msg.item.header)

    def parse_outgoing_msg(self, msg):
        item = copy.deepcopy(msg.item)
        node_id = self.identifier

        if item.command.name == RTLSCommands.RTLS_CMD_IDENTIFY.name and node_id is None:
            return

        payload = item.payload
        if payload is not None:
            # Filtering out "_io" for future dumps option
            item.payload = {k: v for (k, v) in payload.items() if k != "_io"}

        self.log_outgoing_msg(item)

    def log_outgoing_msg(self, item):
        node_id = self.identifier
        json_item = json.loads(item.as_json())
        json_item["command"] = item.command.name
        # json_item["type"] = json_item["type"].replace("UNPITypes.", "")
        json_item["type"] = "Command"

        # Filtering out "originator" and "subsystem" fields
        new_dict = {k: v for (k, v) in json_item.items() if k != "originator" if k != "subsystem"}

        if self.capabilities.get('RTLS_COORDINATOR', False):
            role = "COORDINATOR  "
        elif self.capabilities.get('RTLS_PASSIVE', False):
            role = "PASSIVE  "
        elif self.capabilities.get('RTLS_RESPONDER', False):
            role = "RESPONDER  "
        elif self.capabilities.get('RTLS_CONNECTION_MONITOR', False):
            role = "CONNECTION_MONITOR  "
        else:
            role = "UNCLEAR  "

        logger.info(f"{role}: {node_id} <-- {new_dict}")

    def recv(self, block=False, timeout=None):
        logger.debug("Waiting for data")

        sub = Subscriber(queue.PriorityQueue(), interest=None, transient=True, eventloop=None)
        self.add_subscriber(sub)

        try:
            item = sub.queue.get(block=block, timeout=timeout)
            sub.queue.task_done()
            msg = item.item
            return msg
        except queue.Empty:
            self.remove_subscriber(sub)
        return None

    def add_subscriber(self, subscriber):
        self.subscribers.append(subscriber)

    def remove_subscriber(self, subscriber):
        if subscriber in self.subscribers:
            self.subscribers.remove(subscriber)

    # @classmethod
    # def parse(cls, msg):
    #     header = UNPIHeader.from_message(msg)
    #     sss = {s.type: s for s in cls.subsystems}
    #     parserclass = sss.get(int(header.subsystem), None)
    #     if parserclass:
    #         parsed = parserclass.parse(msg)
    #     else:
    #         parsed = {}
    #     msg.payload = parsed
    #     return msg

    def parse(self, msg):
        header = UNPIHeader.from_message(msg)
        sss = {s.type: s for s in self.subsystems}
        parserclass = sss.get(int(header.subsystem), None)
        if parserclass:
            parsed = parserclass.parse(msg)
        else:
            parsed = {}
        msg.payload = parsed
        return msg

    def wait_stopped(self):
        self.serial.join()
