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

import logging

logger = logging.getLogger('rtlsmanager')

import queue
import sys
import threading
import time
import json
from collections import UserDict
from typing import List, Tuple

from .rtlsnode import RTLSNode, Subscriber, NodeMessage

import asyncio

from .ss_rtls import RTLS
from unpi.unpiparser import QMessage, UNPIError
from .websocket import WSServer


def b2ascii(b):
    return ':'.join(["%02X" % y for y in b])


class RTLSManager(threading.Thread, UserDict):
    """
    Wraps several RTLSNodes, combines incoming data and routes outgoing data.
    """

    def __init__(self, nodes: List[RTLSNode], websocket_port: int or None = 8766):
        """
        Creates an RTLSManager instance, but does not start the node threads
        or the websocket server until `start()` is called on the instance.

        :param nodes: A list of RTLSNode instances, started or not started
        :param websocket_port: The port at which to serve websocket requests
                               or None if no websocket server should be started
        """
        super().__init__(name="RTLSManager")
        self.nodes = nodes
        for node in self.nodes:
            node.manager = self
        self.inQueue = queue.Queue()
        self.outQueue = queue.PriorityQueue()
        self.stopEvent = threading.Event()
        self.subscribers = []

        self.wss = None
        self.wssloop = None
        if websocket_port is not None:
            self.wssloop = asyncio.new_event_loop()
            self.wssloop.set_debug(True)
            self.wss = WSServer(self.wssloop, on_connect=self._on_socket_connect,
                                on_disconnect=self._on_socket_disconnect, port=websocket_port)
        self.input_queues = []  # Websocket queues
        self.on_ws_connect = None
        self.on_ws_disconnect = None
        self.on_ws_message = None
        self.on_node_message = None

        self.subscriptions = {}  # Subscriber objects that this class holds on nodes

        self.data = {}  # UserDict backing dict

        self.auto_params = False

    def __eq__(self, other):
        return self is other

    def __hash__(self):
        return hash(id(self))

    def identify_node(self, node, identifier):
        self[identifier] = node

    def message_from_node(self, node, message):
        self.inQueue.put(NodeMessage(identifier=node.identifier, message=message), block=True)

    @property
    def stopped(self):
        return self.stopEvent.is_set()

    def stop(self) -> None:
        """
        Stops the associated RTLSNode instances' threads and websocket server if any. Pends on threads stop.
        :return: None
        """
        self.stopEvent.set()
        # logging.shutdown()
        for node in self.nodes:
            node.stop()
            node.join()
        if self.wss:
            self.wss.stop()
            self.wss.join()

    def run(self):
        if self.wss: self.wss.start()
        for node in self.nodes:
            if not node.is_alive():
                node.start()
            else:
                if node.identifier:
                    self[node.identifier] = node

        try:
            while not self.stopped:
                #
                # Input from nodes
                #
                try:
                    item = self.inQueue.get(block=True, timeout=0.5)
                    self.inQueue.task_done()
                    msg = item.message.item

                    # Relay conn params message automatically from coordinator to passives
                    if self.auto_params and msg.command == 'RTLS_CMD_CONN_PARAMS' and self[item.identifier].capabilities.get('RTLS_COORDINATOR', False):
                        nodes = [n for n in self.nodes if n.capabilities.get('RTLS_PASSIVE', False) or n.capabilities.get('RTLS_CONNECTION_MONITOR', False)]
                        for node in nodes:
                            node.rtls.set_ble_conn_info(**msg.payload)

                    parsedItem = item  # msg  # QMessage(item.priority, msg)
                    # logger.debug("Have %d subscribers for %s" % (len(self.subscribers), msg))

                    # Send this off to subscribers and callback
                    if self.on_node_message:
                        self.on_node_message(parsedItem)

                    for subscriber in self.subscribers:
                        if subscriber.eventloop:  # Then it's a websocket
                            asyncio.run_coroutine_threadsafe(subscriber.queue.put(item), subscriber.eventloop)
                        else:
                            subscriber.queue.put(parsedItem)
                        if subscriber.transient:
                            self.subscribers.remove(subscriber)
                except queue.Empty:
                    pass

                #
                # Input from socket
                #
                for inQ in self.input_queues:
                    try:
                        item = inQ.get_nowait()
                        # Check if it's a meta-command
                        try:
                            js_dict = json.loads(item)
                            if js_dict.get('control', None):
                                self.handle_control(js_dict)
                            else:
                                msg = self._msg_from_json(item)
                                if isinstance(msg.message, UNPIError):
                                    wss_subscriber = next((sub for sub in self.subscribers if sub.eventloop))
                                    asyncio.run_coroutine_threadsafe(
                                        wss_subscriber.queue.put(json.dumps({'error': msg.message.as_json()})),
                                        wss_subscriber.eventloop)
                                    logger.error("Could not parse websocket message")
                                else:
                                    logger.debug(msg)
                                    self.outQueue.put(QMessage(priority=1, item=msg))
                                    if self.on_ws_message is not None: self.on_ws_message(msg)
                        except json.decoder.JSONDecodeError as e:
                            self.send_to_ws('{"error": "Invalid JSON"}')

                    except queue.Empty:
                        pass

                #
                # Output to nodes
                #
                try:
                    out_msg = self.outQueue.get_nowait()
                    item = out_msg.item  # Item is of type NodeMessage
                    identifier = item.identifier

                    # Find destination node based on identifier match
                    dst_node = next((node for node in self.nodes if
                                     node.identifier == identifier))  # , None) -- accept StopIteration exception if not found
                    # Send just the UNPIMessage part to the node, with same priority as the NodeMessage
                    dst_node.send(item.message)
                except queue.Empty:
                    pass
        finally:
            self.stop()

    def wait_identified(self) -> Tuple[RTLSNode, List[RTLSNode], List[RTLSNode], List[RTLSNode], List[RTLSNode]]:
        """
        Waits up to 500 msec for associated RTLSNode instances to connect and get a response
        :return: Tuple (coordinator, [passives], [failed]) of successful and failed nodes.
        """
        target_time = time.time() + 0.5
        failed = [n for n in self.nodes]
        while target_time > time.time() and len(failed):
            for n in failed:
                if n.identifyEvent.isSet():
                    failed.remove(n)

        coordinator, passives, connection_monitors, responders = self.get_coordinator_passives()

        return coordinator, passives, connection_monitors, responders, failed

    def get_coordinator_passives(self) -> Tuple[RTLSNode, List[RTLSNode], List[RTLSNode], List[RTLSNode]]:
        """
        Returns a tuple of coordinator and list of passives, determined by the capabilities reported
        :return: (coordinator, [passives])
        """
        coordinator_node = None
        try:
            coordinator_node = next((n for n in self.nodes if n.capabilities.get('RTLS_COORDINATOR', False)))
        except StopIteration:
            pass

        passive_nodes = [n for n in self.nodes if n.capabilities.get('RTLS_PASSIVE', False)]

        connection_monitor_nodes = [n for n in self.nodes if n.capabilities.get('RTLS_CONNECTION_MONITOR', False)]

        responders_nodes = [n for n in self.nodes if n.capabilities.get('RTLS_RESPONDER', False)]

        return coordinator_node, passive_nodes, connection_monitor_nodes, responders_nodes

    def _on_socket_connect(self, ws, inQ, outQ):
        sub = Subscriber(queue=outQ, interest=None, transient=False, eventloop=self.wssloop)
        if self.on_ws_connect is not None: self.on_ws_connect(ws)
        self.add_subscriber(sub)
        self.input_queues.append(inQ)
        return sub

    def _on_socket_disconnect(self, ws, sub, inQ, outQ):
        if self.on_ws_disconnect is not None: self.on_ws_disconnect(ws)
        for node in self.nodes:
            node.remove_subscriber(sub)
        self.input_queues.remove(inQ)
        for sub in self.subscribers[:]:
            if sub.eventloop == ws.loop:
                self.subscribers.remove(sub)

    def send(self, msg: NodeMessage) -> None:
        """
        Puts a NodeMessage on the output queue to be sent to the appropriate RTLSNode child.
        :param msg: NodeMessage containing recipient identifier and an UNPIMessage object
        :return: None
        """
        self.outQueue.put(msg)

    def recv(self, block=False, timeout=None) -> NodeMessage or None:
        """
        Adds and pends on a transient queue subscriber. The subscriber is removed on timeout or message receipt.
        :param block: Whether to block when waiting for incoming message
        :param timeout: How long to block
        :return: NodeMessage or None
        """
        sub = Subscriber(queue.PriorityQueue(), interest=None, transient=True, eventloop=None)
        self.add_subscriber(sub)

        try:
            item = sub.queue.get(block=block, timeout=timeout)
            sub.queue.task_done()
            msg = item.item
            return msg
        except queue.Empty:
            # Since it's a transient subscriber it is removed if a message is received
            # Must remove here if no message received
            self.remove_subscriber(sub)
        return None

    def create_subscriber(self) -> Subscriber:
        """
        Creates a subscriber and subscribes to messages from connected RTLS nodes.
        :return: Subscriber
        """
        sub = Subscriber(queue=queue.PriorityQueue(), interest=None, transient=False, eventloop=None)
        self.add_subscriber(sub)
        return sub

    def add_subscriber(self, subscriber: Subscriber) -> None:
        """
        Append a subscriber. It will receive all messages from serial nodes.
        :param subscriber:
        :return: None
        """
        self.subscribers.append(subscriber)

    def remove_subscriber(self, subscriber):
        self.subscribers.remove(subscriber)

    def _msg_from_json(self, js):
        msg_unparsed_payload = NodeMessage.from_json(js)
        msg = msg_unparsed_payload.message

        if isinstance(msg, UNPIError):
            return msg_unparsed_payload  # pass it along

        # Find which node should get this
        node = self.get(msg_unparsed_payload.identifier, None)
        if node is None:
            error = "Could not find node with identifier %s" % msg_unparsed_payload.identifier
            logger.error(error)
            msg_unparsed_payload.message = UNPIError(__name__, error)
            return msg_unparsed_payload

        # Find out which subsystem should parse it
        if not hasattr(msg.subsystem, 'name'): return None  # Was not parsed

        try:
            ss = next((s for s in node.subsystems if s.type == msg.subsystem.value))
        except StopIteration:
            error = "Could not find subsystem %s in initialized list %s" % (
                msg.subsystem.name, ','.join([s.__name__ for s in node.subsystems]))
            logger.error(error)
            msg_unparsed_payload.message = UNPIError(__name__, error)
            return msg_unparsed_payload

        # Make it build the construct
        unpimsg = ss.build_from_json(msg_unparsed_payload.message)
        # Swap it out (technically the variable name is now a lie)
        msg_unparsed_payload.message = unpimsg
        return msg_unparsed_payload

    def send_to_ws(self, msg):
        wss_subscriber = next((sub for sub in self.subscribers if sub.eventloop))
        asyncio.run_coroutine_threadsafe(wss_subscriber.queue.put(msg), wss_subscriber.eventloop)

    def handle_control(self, dct):
        control = dct['control']
        req = control.get('req', None)
        logger.debug("Got control message: " + req)
        if req == 'LIST_DEVICES':
            rsp = {'control':
                       {'req': 'LIST_DEVICES',
                        'devices': [
                            {'name': n.name,
                             'port': n.port,
                             'identifier': n.identifier,
                             'caps': [str(c) for c, e in n.capabilities.items() if e]
                             }
                            for n in self.nodes
                        ]}}
            self.send_to_ws(json.dumps(rsp))

        if req == 'LIST_SUBSYSTEMS':
            ident = control.get('identifier', None)
            if ident is None:
                self.send_to_ws(json.dumps({'error': 'Missing identifier'}))
            else:
                node = self.get(ident, None)
                if node is None:
                    self.send_to_ws(json.dumps({'error': 'Could not find node with identifier'}))
                else:
                    rsp = {'control':
                               {'req': 'LIST_SUBSYSTEMS',
                                'subsystems': [
                                    {'name': ss.__name__,
                                     'val': ss.type,
                                     'requests': [
                                         {'originator': r.originator.name, 'cmd': r.command.name, 'type': r.type.name}
                                         for r in ss.responses()]
                                     }
                                    for ss in node.subsystems
                                ]}}
                    self.send_to_ws(json.dumps(rsp))


if __name__ == '__main__':
    logging.basicConfig(stream=sys.stdout, level=logging.DEBUG,
                        format='[%(asctime)s] %(filename)-18sln %(lineno)3d %(threadName)-10s %(levelname)8s - %(message)s')

    jsonmsg = '{"identifier": "54:6C:0E:A0:50:6A", "message": {"type": "SyncRsp", "subsystem": "RTLS", "command": "RTLS_BLE_SCAN", "payload": {"status": "SUCCESS"}}}'

    msg_from_json = NodeMessage.from_json(jsonmsg)
    out = RTLS.build_from_json(msg_from_json.message)
    logging.info(out)

    my_nodes = [RTLSNode('COM17', 460800, "Coordinator")]

    manager = None
    managerSub = Subscriber(queue=queue.PriorityQueue(), interest=None, transient=False, eventloop=None)
    try:
        manager = RTLSManager(my_nodes)

        manager.add_subscriber(managerSub)
        manager.start()
        time.sleep(2)
        for node in manager.nodes:
            logging.info(node.identifier)

        tofCoordinatorId = '54:6C:0E:A0:50:6A'

        logging.info(manager._msg_from_json(jsonmsg))

        manager[tofCoordinatorId].rtls.scan()
        scanMsg = QMessage(1, NodeMessage(tofCoordinatorId, RTLS.ScanReq.build()))
        manager.send(scanMsg)

        while True:
            try:
                node_msg = managerSub.pend(block=True, timeout=0.5)
                from_node = node_msg.identifier
                pri = node_msg.message.priority
                msg = node_msg.message.item
                # logging.info(msg)
                logging.info(node_msg.as_json())
                # logging.info(node_msg.from_json(node_msg.as_json()))

            except queue.Empty:
                pass

    finally:
        if manager:
            manager.stop()
