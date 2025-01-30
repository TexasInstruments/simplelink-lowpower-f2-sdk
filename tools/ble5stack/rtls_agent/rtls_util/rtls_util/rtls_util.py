import weakref
import sys
import queue
import time
import threading
import json
import logging
from logging.handlers import RotatingFileHandler
import copy

from .rtls_util_exception import *

from rtls import RTLSManager, RTLSNode

from dataclasses import dataclass


@dataclass
class RtlsUtilLoggingLevel():
    INFO = 20
    DEBUG = 10
    ALL = 0
    NONE = -1


class ConnectionlessAOASync():
    def __init__(self, sync_created, sync_established, sync_report_enable, cl_aoa_enable, in_padv_list, sync_handle):
        self.sync_created = sync_created
        self.sync_established = sync_established
        self.sync_report_enable = sync_report_enable
        self.cl_aoa_enable = cl_aoa_enable
        self.sync_handle = sync_handle
        self.in_padv_list = in_padv_list

    def __repr__(self):
        return "%s(%r)" % (self.__class__, self.__dict__)


class RtlsUtil():
    def __init__(self, logging_file, logging_level, websocket_port=None, max_log_file_size=100):
        self.logger = None
        self.logger_fh = None

        if not logging_level == RtlsUtilLoggingLevel.NONE:
            self.logger = logging.getLogger()
            self.logger.setLevel(logging_level)

            self.logger_fh = RotatingFileHandler(logging_file, maxBytes=max_log_file_size * 1024 * 1024,
                                                 backupCount=100)
            self.logger_fh.setLevel(logging_level)

            # formatter = logging.Formatter('[%(asctime)s] %(filename)-18sln %(lineno)3d %(threadName)-10s %(name)s - %(levelname)8s - %(message)s')
            formatter = logging.Formatter('[%(asctime)s] %(name)9s - %(levelname)8s - %(message)s')
            self.logger_fh.setFormatter(formatter)

            # Messages can be filter by logger name
            # blank means all all messages
            # filter = logging.Filter()
            # self.logger_fh.addFilter(filter)

            self.logger.addHandler(self.logger_fh)

        self._coordinator_node = None
        self._passive_nodes = []
        self._connection_monitor_nodes = []
        self._responder_nodes = []
        self._all_nodes = []

        self._rtls_manager = None
        self._rtls_manager_subscriber = None

        self._message_receiver_th = None
        self._message_receiver_stop = False

        self._scan_results = []
        self._scan_stopped = threading.Event()
        self._scan_stopped.clear()

        self._ble_connected = False
        self._connected_responder = []

        self._coordinator_disconnected = threading.Event()
        self._coordinator_disconnected.clear()

        self._coordinator_seed = None

        self._timeout = 30
        self._conn_handle = None
        self._responder_attempt_to_connect = None

        self._is_cci_started = False
        self._is_aoa_started = False

        self.padv_list_size = None
        self.sync_failed_to_be_est = False
        # TODO: Change this flags to be threading.event and use clear, is_set, set to toggle
        self._padv_read_list_size_complete = threading.Event()
        self._padv_read_list_size_complete.clear()
        self._padv_clear_adv_list_complete = threading.Event()
        self._padv_clear_adv_list_complete.clear()

        self._read_heap_size_complete = threading.Event()
        self._read_heap_size_complete.clear()

        self._padv_current_responder = None
        self._padv_sync_dict = {}

        self.aoa_results_queue = queue.Queue()
        self.conn_info_queue = queue.Queue()
        self.padv_event_queue = queue.Queue()
        self.cl_aoa_results_queue = queue.Queue()
        self.custom_message_queue = queue.Queue()

        self.custom_message_filter = None

        self.evt_assert = False
        self.total_heap = None
        self.free_heap = None

        self.websocket_port = websocket_port

        self.on_ble_disconnected_queue = queue.Queue()

    def __del__(self):
        self.done()

    @property
    def timeout(self):
        return self._timeout

    @timeout.setter
    def timeout(self, value):
        self._timeout = value

    def _rtls_wait(self, true_cond_func, nodes, timeout_message):
        timeout = time.time() + self._timeout
        timeout_reached = time.time() > timeout

        while not true_cond_func(nodes) and not timeout_reached:
            time.sleep(0.1)
            timeout_reached = time.time() > timeout

        if timeout_reached:
            raise RtlsUtilTimeoutException(
                f"Timeout reached while waiting for : {timeout_message}")

    def done(self):
        if self._message_receiver_th is not None:
            self._message_receiver_stop = True
            self._message_receiver_th.join()
            self._message_receiver_th = None

        if self._rtls_manager:
            self._rtls_manager.stop()

            self._rtls_manager_subscriber = None
            self._rtls_manager = None

        if self.logger_fh:
            self.logger_fh.close()
            self.logger.removeHandler(self.logger_fh)

    def _log_incoming_msg(self, item, identifier):
        json_item = json.loads(item.as_json())

        json_item["type"] = "Response" if json_item["type"] == "SyncRsp" else "Event"

        # Filtering out "originator" and "subsystem" fields
        new_dict = {k: v for (k, v) in json_item.items()
                    if k != "originator" if k != "subsystem"}

        # Get reference to RTLSNode based on identifier in message
        sending_node = self._rtls_manager[identifier]

        if sending_node in self._passive_nodes:
            self.add_user_log(f"PASSIVE : {identifier} --> {new_dict}")
        elif sending_node in self._connection_monitor_nodes:
            self.add_user_log(f"CONNECTION_MONITOR : {identifier} --> {new_dict}")
        elif sending_node in self._responder_nodes:
            self.add_user_log(f"RESPONDER : {identifier} --> {new_dict}")
        else:
            self.add_user_log(f"COORDINATOR  : {identifier} --> {new_dict}")

    def _message_receiver(self):
        while not self._message_receiver_stop:
            # Get messages from manager
            try:
                identifier, msg_pri, msg = self._rtls_manager_subscriber.pend(
                    block=True, timeout=0.05).as_tuple()

                self._log_incoming_msg(msg, identifier)

                sending_node = self._rtls_manager[identifier]

                if self.custom_message_filter and msg.command in self.custom_message_filter:
                    self.custom_message_queue.put({
                        "name": sending_node.name,
                        "identifier": identifier,
                        "msg": msg
                    })

                if msg.command == "RTLS_EVT_DEBUG" and msg.type == "AsyncReq":
                    self.add_user_log(msg.payload)

                if msg.command == "RTLS_CMD_SCAN" and msg.type == "AsyncReq":
                    self._add_scan_result({
                        'addr': msg.payload.addr,
                        'addrType': msg.payload.addrType,
                        'rssi': msg.payload.rssi,
                        'advSID': msg.payload.advSID,
                        'periodicAdvInt': msg.payload.periodicAdvInt
                    })

                if msg.command == "RTLS_CMD_SCAN_STOP" and msg.type == "AsyncReq":
                    self._scan_stopped.set()

                if msg.command == "RTLS_CMD_CONNECT" and msg.type == "AsyncReq" and msg.payload.status == "RTLS_SUCCESS":
                    if self._responder_attempt_to_connect and not sending_node.connection_in_progress \
                            and sending_node.capabilities.get('RTLS_COORDINATOR', False):
                        ## Lines below should be activate for active responders
                        self._connected_responder.append(copy.copy(self._responder_attempt_to_connect))
                        self._responder_attempt_to_connect = None

                    sending_node.connection_in_progress = False
                    sending_node.connected = True

                    if sending_node.identifier == self._coordinator_node.identifier:
                        self._conn_handle = msg.payload.connHandle if 'connHandle' in msg.payload else -1
                        self._coordinator_disconnected.clear()

                if msg.command == "RTLS_CMD_CONN_PARAMS" and msg.type == "AsyncReq":
                    self._responder_attempt_to_connect = {
                        'addr': msg.payload.peerAddr,
                        'conn_handle': msg.payload.connHandle,
                        'connRole': msg.payload.connRole
                    }

                if msg.command == "RTLS_CMD_CONNECT" and msg.type == "AsyncReq" \
                        and msg.payload.status in ["RTLS_LINK_TERMINATED", "RTLS_FAIL"]:
                    sending_node.connected = False
                    sending_node.connection_in_progress = False

                    if sending_node.identifier == self._coordinator_node.identifier:
                        self._coordinator_disconnected.set()

                        if 'connHandle' in msg.payload:
                            ## Cleaning connected responders
                            for _responder in self._connected_responder[:]:
                                if _responder['conn_handle'] == msg.payload.connHandle:
                                    self._connected_responder.remove(_responder)
                                    break

                            ## Sending terminate link to passives and connection monitors nodes
                            need_to_update_nodes = self._passive_nodes + self._connection_monitor_nodes
                            for n in need_to_update_nodes:
                                n.rtls.terminate_link(msg.payload.connHandle)

                        # TODO:
                        #     Make disocnnect wait for all required responder to disconnect
                        #     if len(self._connected_responder) == 0:
                        #         self._coordinator_disconnected.set()
                        #     else:
                        #         self._coordinator_disconnected.set()

                    elif 'connHandle' in msg.payload and sending_node in self._passive_nodes:
                        responder = self._get_responder_by_conn_handle(msg.payload.connHandle)
                        # In this point if responder is none it means connection fail while we in connection process
                        if responder is not None:
                            self.on_ble_disconnected_queue.put({
                                'node_identifier': sending_node.identifier,
                                'responder_addr': responder['addr'],
                                'isCciStarted': self._is_cci_started,
                                'isAoaStarted': self._is_aoa_started
                            })

                    elif sending_node in self._responder_nodes and msg.payload.status == "RTLS_FAIL":
                        self._coordinator_node.connected = False
                        self._coordinator_node.connection_in_progress = False

                    else:
                        pass

                if msg.command == 'RTLS_CMD_AOA_SET_PARAMS' and msg.payload.status == 'RTLS_SUCCESS':
                    sending_node.aoa_initialized = True

                if msg.command == 'RTLS_CMD_AOA_ENABLE' and msg.payload.status == 'RTLS_SUCCESS':
                    sending_node.aoa_started = True

                if msg.command in ["RTLS_CMD_AOA_RESULT_ANGLE",
                                   "RTLS_CMD_AOA_RESULT_RAW",
                                   "RTLS_CMD_AOA_RESULT_PAIR_ANGLES"] and msg.type == "AsyncReq":
                    self.aoa_results_queue.put({
                        "name": sending_node.name,
                        "type": str(msg.command),
                        "identifier": identifier,
                        "payload": msg.payload
                    })

                if msg.command == 'RTLS_CMD_RESET_DEVICE' and msg.type == 'AsyncReq':
                    sending_node.device_resets = True

                if msg.command == 'RTLS_CMD_CONN_INFO' and msg.type == 'SyncRsp':
                    sending_node.cci_started = True

                if msg.command == 'RTLS_EVT_CONN_INFO' and msg.type == 'AsyncReq':
                    self.conn_info_queue.put({
                        "name": sending_node.name,
                        "type": "conn_info",
                        "identifier": identifier,
                        "payload": msg.payload
                    })

                # print(msg.payload)
                if msg.command == 'RTLS_CMD_SET_RTLS_PARAM' and msg.payload.rtlsParamType == "RTLS_PARAM_CONNECTION_INTERVAL" and msg.payload.status == "RTLS_SUCCESS":
                    sending_node.conn_interval_updated = True

                if msg.command == 'RTLS_CMD_IDENTIFY' and msg.type == 'SyncRsp':
                    sending_node.identified = True
                    sending_node.identifier = msg.payload.identifier
                    sending_node.capabilities = msg.payload.capabilities
                    sending_node.devId = msg.payload.devId
                    sending_node.revNum = msg.payload.revNum

                if msg.command == 'RTLS_CMD_CREATE_SYNC' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                    self._padv_sync_dict[self._padv_current_responder].sync_created = True

                if msg.command == 'RTLS_EVT_TERMINATE_SYNC' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SUCCESS':
                    if self._padv_current_responder is not None and self._padv_current_responder in self._padv_sync_dict.keys():
                        self._padv_sync_dict[self._padv_current_responder].sync_created = False

                if msg.command == 'RTLS_EVT_SYNC_LOST' and msg.type == 'AsyncReq':
                    responder = self._padv_find_address_by_handle(msg.payload.syncHandle)
                    if responder:
                        padv_current_responder = (responder[0], responder[1])
                        self._padv_sync_dict[padv_current_responder].sync_created = False
                        self._padv_sync_dict[padv_current_responder].sync_established = False
                        self._padv_sync_dict[padv_current_responder].sync_report_enable = False
                        self._padv_sync_dict[padv_current_responder].cl_aoa_enable = False
                        self._padv_sync_dict[padv_current_responder].sync_handle = -1

                if msg.command == 'RTLS_CMD_ADD_DEVICE_ADV_LIST' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                    self._padv_sync_dict[self._padv_current_responder].in_padv_list = True

                if msg.command == 'RTLS_CMD_REMOVE_DEVICE_ADV_LIST' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                    self._padv_sync_dict[self._padv_current_responder].in_padv_list = False

                if msg.command == 'RTLS_CMD_CLEAR_ADV_LIST' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                    self._padv_clear_adv_list_complete.set()

                if msg.command == 'RTLS_CMD_CL_AOA_ENABLE' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                    self._padv_sync_dict[self._padv_current_responder].cl_aoa_enable = not self._padv_sync_dict[
                        self._padv_current_responder].cl_aoa_enable

                if msg.command == 'RTLS_EVT_SYNC_EST' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SUCCESS':
                    current_responder = (msg.payload.advAddress, msg.payload.advSid)
                    if current_responder in self._padv_sync_dict.keys():
                        self._padv_sync_dict[current_responder].sync_established = True
                        self._padv_sync_dict[current_responder].sync_handle = msg.payload.syncHandle

                if msg.command == 'RTLS_EVT_SYNC_EST' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SYNC_CANCELED_BY_HOST':
                    self._padv_sync_dict[self._padv_current_responder].sync_created = False

                if msg.command == 'RTLS_EVT_SYNC_EST' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SYNC_FAILED_TO_BE_EST':
                    self.sync_failed_to_be_est = True

                if msg.command == 'RTLS_CMD_PERIODIC_RECEIVE_ENABLE' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                    self._padv_sync_dict[self._padv_current_responder].sync_report_enable = not self._padv_sync_dict[
                        self._padv_current_responder].sync_report_enable

                if msg.command == 'RTLS_CMD_READ_ADV_LIST_SIZE' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SUCCESS':
                    self.padv_list_size = msg.payload.listSize
                    self._padv_read_list_size_complete.set()

                if msg.command == 'RTLS_CMD_HEAP_SIZE' and msg.type == 'SyncRsp':
                    self.total_heap = msg.payload.totalHeap
                    self.free_heap = msg.payload.freeHeap
                    self._read_heap_size_complete.set()

                if msg.command == 'RTLS_EVT_PERIODIC_ADV_RPT' and msg.type == 'AsyncReq':
                    self.padv_event_queue.put({
                        "name": sending_node.name,
                        "type": "padv_event",
                        "identifier": identifier,
                        "payload": msg.payload
                    })

                if msg.command in ['RTLS_CMD_CL_AOA_RESULT_RAW',
                                   'RTLS_CMD_CL_AOA_RESULT_ANGLE',
                                   'RTLS_CMD_CL_AOA_RESULT_PAIR_ANGLES'] and msg.type == 'AsyncReq':
                    self.cl_aoa_results_queue.put({
                        "name": sending_node.name,
                        "type": str(msg.command),
                        "identifier": identifier,
                        "payload": msg.payload
                    })

                if msg.command == 'RTLS_EVT_ASSERT' and msg.type == 'AsyncReq':
                    self.evt_assert = True

            except queue.Empty:
                pass

    def _start_message_receiver(self):
        self._message_receiver_stop = False
        self._message_receiver_th = threading.Thread(
            target=self._message_receiver)
        self._message_receiver_th.setDaemon(True)
        self._message_receiver_th.start()

    def _empty_queue(self, q):
        while True:
            try:
                q.get_nowait()
            except queue.Empty:
                break

    def _is_passive_in_nodes(self, nodes):
        for node in nodes:
            if not node.capabilities.get('RTLS_COORDINATOR', False):
                return True

        return False

    # User Function

    def add_user_log(self, msg, print_to_terminal=False):
        if self.logger:
            self.logger.info(msg)

        if print_to_terminal:
            print(msg)

    def set_custom_message_filter(self, list_of_cmd):
        if not isinstance(list_of_cmd, (list, tuple)):
            raise Exception(" list_of_cmd must be type of list or tuple")

        self.custom_message_filter = list_of_cmd

    def get_custom_message_filter(self):
        return self.custom_message_filter

    def clear_custom_message_filter(self):
        self.custom_message_filter = None

    # Devices API

    def indentify_devices(self, devices_setting):
        self.add_user_log("Setting nodes : ".format(
            json.dumps(devices_setting)))
        nodes = [RTLSNode(node["com_port"], node["baud_rate"],
                          node["name"]) for node in devices_setting]

        _rtls_manager = RTLSManager(nodes, websocket_port=None)
        _rtls_manager_subscriber = _rtls_manager.create_subscriber()
        _rtls_manager.auto_params = False

        _rtls_manager.start()
        self.add_user_log("RTLS Manager started")
        time.sleep(2)

        _all_nodes = _rtls_manager.nodes

        _rtls_manager.stop()
        while not _rtls_manager.stopped:
            time.sleep(0.1)

        _rtls_manager_subscriber = None
        _rtls_manager = None

        return _all_nodes

    def set_devices(self, devices_setting):
        self.add_user_log("Setting nodes : ".format(json.dumps(devices_setting)))
        nodes = [RTLSNode(node["com_port"], node["baud_rate"], node["name"]) for node in devices_setting]

        self._rtls_manager = RTLSManager(nodes, websocket_port=self.websocket_port)
        self._rtls_manager_subscriber = self._rtls_manager.create_subscriber()
        self._rtls_manager.auto_params = True

        self._start_message_receiver()
        self.add_user_log("Message receiver started")

        self._rtls_manager.start()
        self.add_user_log("RTLS Manager started")
        time.sleep(2)
        self._coordinator_node, self._passive_nodes, self._connection_monitor_nodes, self._responder_nodes, failed = self._rtls_manager.wait_identified()

        if self._coordinator_node is None:
            raise RtlsUtilCoordinatorNotFoundException("No one of the nodes identified as RTLS COORDINATOR")
        # elif len(self._passive_nodes) == 0:
        #     raise RtlsUtilPassiveNotFoundException("No one of the nodes identified as RTLS PASSIVE")
        elif len(failed) > 0:
            raise RtlsUtilNodesNotIdentifiedException(
                "{} nodes not identified at all".format(len(failed)), failed)
        else:
            pass

        self._all_nodes = []
        self._all_nodes.extend(copy.copy(self._passive_nodes))
        self._all_nodes.extend(copy.copy(self._connection_monitor_nodes))
        self._all_nodes.extend(copy.copy(self._responder_nodes))
        self._all_nodes.extend([self._coordinator_node])

        for node in self._all_nodes:
            node.cci_started = False
            node.aoa_initialized = False

            node.ble_connected = False
            node.device_resets = False

        self.add_user_log("Done setting node")
        return self._coordinator_node, self._passive_nodes, self._connection_monitor_nodes, self._responder_nodes, self._all_nodes

    def get_devices_capability(self, nodes=None):
        nodes_to_set = self._all_nodes
        if nodes is not None:
            if isinstance(nodes, list):
                nodes_to_set = nodes
            else:
                raise RtlsUtilException("nodes input must be from list type")

        for node in nodes_to_set:
            node.identified = False
            node.rtls.identify()

        true_cond_func = lambda nodes: all([n.identified for n in nodes])
        self._rtls_wait(true_cond_func, nodes_to_set, "All device to identified")

        ret = []
        for node in nodes_to_set:
            dev_info = {
                "node_mac_address": node.identifier,
                "capabilities": node.capabilities
            }

            ret.append(dev_info)

        return ret

    ######

    # Common BLE API

    def _get_responder_by_addr(self, addr, advSID=None):
        for _scan_result in self._scan_results:
            if advSID is None:
                if _scan_result['addr'].lower() == addr.lower():
                    return _scan_result
            else:
                if _scan_result['addr'].lower() == addr.lower() and _scan_result['advSID'] == advSID:
                    return _scan_result

        return None

    def _get_responder_by_conn_handle(self, conn_handle):
        for _responder in self._connected_responder:
            if _responder['conn_handle'] == conn_handle:
                return _responder

        return None

    def _add_scan_result(self, scan_result):
        if self._get_responder_by_addr(scan_result['addr'], scan_result['advSID']) is None:
            self._scan_results.append(scan_result)

    def scan(self, scan_time_sec, expected_responder_bd_addr=None, advSID=None, node=None):
        if node is None:
            node = self._coordinator_node

        self._scan_results = []

        timeout = time.time() + scan_time_sec
        timeout_reached = time.time() > timeout

        while not timeout_reached:
            self._scan_stopped.clear()

            node.rtls.scan()
            scan_start_time = time.time()

            scan_timeout_reached = time.time() > (scan_start_time + 10)
            while not self._scan_stopped.isSet() and not scan_timeout_reached:
                time.sleep(0.1)
                scan_timeout_reached = time.time() > (scan_start_time + 10)

                if scan_timeout_reached:
                    raise RtlsUtilEmbeddedFailToStopScanException(
                        "Embedded side didn't finished due to timeout")

            if len(self._scan_results) > 0:
                if expected_responder_bd_addr is not None and self._get_responder_by_addr(expected_responder_bd_addr,
                                                                                          advSID) is not None:
                    break

            timeout_reached = time.time() > timeout
        else:
            if len(self._scan_results) > 0:
                if expected_responder_bd_addr is not None and self._get_responder_by_addr(expected_responder_bd_addr,
                                                                                          advSID) is not None:
                    raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
            else:
                raise RtlsUtilScanNoResultsException(
                    "No device with responder capability found")

        return self._scan_results

    @property
    def ble_connected(self):
        return len(self._connected_responder) > 0

    def ble_connected_to(self, responder):
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        else:
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")

        for s in self._connected_responder:
            if s['addr'] == responder['addr'] and s['conn_handle'] == responder['conn_handle']:
                return True

        return False

    def ble_connect(self, responder, connect_interval_mSec, node=None):
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        else:
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")

        if node is None:
            node = self._coordinator_node

        interval = int(connect_interval_mSec / 1.25)

        self._conn_handle = None
        self._coordinator_node.connection_in_progress = True
        self._coordinator_node.connected = False

        node.rtls.connect(responder['addrType'], responder['addr'], interval)

        true_cond_func = lambda coordinator_node: coordinator_node.connection_in_progress == False
        self._rtls_wait(true_cond_func, self._coordinator_node, "All node to connect")

        if self._coordinator_node.connected:
            responder['conn_handle'] = self._conn_handle

            self._connected_responder.append(responder)

            self._ble_connected = True
            self.add_user_log("Connection process done")

            return self._conn_handle

        return None

    def ble_disconnect(self, conn_handle=None, nodes=None):
        nodes_to_set = self._all_nodes
        if nodes is not None:
            if isinstance(nodes, list):
                nodes_to_set = nodes
            else:
                raise RtlsUtilException("Nodes input must be from list type")

        ## Remove RTLS RESPONDERS
        nodes_work_with = [n for n in nodes_to_set if not n.capabilities.get('RTLS_RESPONDER', False)]

        self._coordinator_disconnected.clear()

        for node in nodes_work_with:
            if conn_handle is None:
                for responder in self._connected_responder[:]:
                    node.rtls.terminate_link(responder['conn_handle'])
            else:
                node.rtls.terminate_link(conn_handle)

        true_cond_func = lambda event: event.isSet()
        self._rtls_wait(true_cond_func, self._coordinator_disconnected, "Coordinator disconnect")

        self._ble_connected = False
        self.add_user_log("Disconnect process done")

    def set_connection_interval(self, connect_interval_mSec, conn_handle=None):
        conn_interval = int(connect_interval_mSec / 1.25)
        data_len = 2
        data_bytes = conn_interval.to_bytes(data_len, byteorder='little')

        true_cond_func = lambda node: node.conn_interval_updated

        if str(self._coordinator_node.devId) == "DeviceFamily_ID_CC26X0R2":
            self._coordinator_node.conn_interval_updated = False

            self._coordinator_node.rtls.set_rtls_param('RTLS_PARAM_CONNECTION_INTERVAL', data_len, data_bytes)

            self._rtls_wait(true_cond_func, self._coordinator_node, "Coordinator node set connection interval")

        else:
            if conn_handle is None:
                for s in self._connected_responder:
                    self._coordinator_node.conn_interval_updated = False

                    self._coordinator_node.rtls.set_rtls_param(s['conn_handle'],
                                                               'RTLS_PARAM_CONNECTION_INTERVAL',
                                                               data_len,
                                                               data_bytes)

                    self._rtls_wait(true_cond_func, self._coordinator_node, "Coordinator node set connection interval")


            else:
                self._coordinator_node.conn_interval_updated = False

                self._coordinator_node.rtls.set_rtls_param(conn_handle,
                                                           'RTLS_PARAM_CONNECTION_INTERVAL',
                                                           data_len,
                                                           data_bytes)

                self._rtls_wait(true_cond_func, self._coordinator_node, "Coordinator node set connection interval")

        self.add_user_log("Connection Interval Updated")

    def reset_devices(self, nodes=None):
        nodes_to_set = self._all_nodes
        if nodes is not None:
            if isinstance(nodes, list):
                nodes_to_set = nodes
            else:
                raise RtlsUtilException("nodes input must be from list type")

        for node in nodes_to_set:
            node.device_resets = False
            node.rtls.reset_device()

        true_cond_func = lambda nodes: all([n.device_resets for n in nodes])
        self._rtls_wait(true_cond_func, nodes_to_set, "All node to reset")

    def is_multi_connection_supported(self, nodes):
        return True

    ######

    # CCI - Continuous Connection Info

    def _cci_set_state(self, state, nodes=None, conn_handle=None):
        nodes_to_set = self._all_nodes
        if nodes is not None:
            if isinstance(nodes, list):
                nodes_to_set = nodes
            else:
                raise RtlsUtilException("Nodes input must be from list type")

        true_cond_func = lambda n: n.cci_started
        if state == True:  ## Error Message for CCI start
            error_msg = "Node {} [{}] fail to start Continue Connection Information [CCI]"
        else:  ## Error Message for CCI stop
            error_msg = """
Exception has occurs, 
this can be related to the BLE_AGAMA-3540. 
For more information and workaround, please find BLE_AGAMA-3540 in BLE5-STACK release note"""

        ## Remove RTLS RESPONDERS
        nodes_work_with = [n for n in nodes_to_set if not n.capabilities.get('RTLS_RESPONDER', False)]

        for node in nodes_work_with:
            node.cci_started = False

            for responder in self._connected_responder:
                if conn_handle is not None:
                    if not (conn_handle == responder['conn_handle']):
                        continue

                node.rtls.get_conn_info(responder['conn_handle'], state)
                self._rtls_wait(true_cond_func, node, error_msg.format(node.identifier, node.port))

    def cci_start(self, nodes=None, conn_handle=None):
        self._cci_set_state(True, nodes, conn_handle)
        self._is_cci_started = True
        self.add_user_log("CCI Started")

    def cci_stop(self, nodes=None, conn_handle=None):
        self._cci_set_state(False, nodes, conn_handle)
        self._is_cci_started = False
        self.add_user_log("CCI Stopped")

    ######

    # AOA - Angle of Arrival

    def is_aoa_supported(self, nodes):
        devices_capab = self.get_devices_capability(nodes)
        for device_capab in devices_capab:
            if not (device_capab['capabilities'].AOA_TX == True or device_capab['capabilities'].AOA_RX == True):
                return False

        return True

    def _aoa_set_params(self, node, aoa_params, conn_handle):
        try:
            node.aoa_initialized = False
            node_role = 'AOA_COORDINATOR' if node.capabilities.get('RTLS_COORDINATOR', False) else 'AOA_PASSIVE'
            node.rtls.aoa_set_params(
                node_role,
                aoa_params['aoa_run_mode'],
                conn_handle,
                aoa_params['aoa_cc26x2']['aoa_slot_durations'],
                aoa_params['aoa_cc26x2']['aoa_sample_rate'],
                aoa_params['aoa_cc26x2']['aoa_sample_size'],
                aoa_params['aoa_cc26x2']['aoa_sampling_control'],
                aoa_params['aoa_cc26x2']['aoa_sampling_enable'],
                aoa_params['aoa_cc26x2']['aoa_pattern_len'],
                aoa_params['aoa_cc26x2']['aoa_ant_pattern']
            )

        except KeyError as ke:
            raise RtlsUtilException("Invalid key : {}".format(str(ke)))

    def aoa_set_params(self, aoa_params, nodes=None, conn_handle=None):
        nodes_to_set = self._all_nodes
        if nodes is not None:
            if isinstance(nodes, list):
                nodes_to_set = nodes
            else:
                raise RtlsUtilException("Nodes input must be from list type")

        true_cond_func = lambda n: n.aoa_initialized

        ## Remove RTLS RESPONDERS and RTLS_CONNECTION_MONITOR
        nodes_work_with = [n for n in nodes_to_set if not n.capabilities.get('RTLS_RESPONDER', False)]
        nodes_work_with = [n for n in nodes_work_with if not n.capabilities.get('RTLS_CONNECTION_MONITOR', False)]

        for node in nodes_work_with:
            for responder in self._connected_responder:
                if conn_handle is not None:
                    if not (conn_handle == responder['conn_handle']):
                        continue

                node.aoa_initialized = False

                self._aoa_set_params(node, aoa_params, responder['conn_handle'])
                self._rtls_wait(true_cond_func, node, f"node {responder['addr']} to set AoA params")

    def _aoa_set_state(self, start, cte_interval=1, cte_length=20, nodes=None, conn_handle=None):
        nodes_to_set = self._all_nodes
        if nodes is not None:
            if isinstance(nodes, list):
                nodes_to_set = nodes
            else:
                raise RtlsUtilException("Nodes input must be from list type")

        true_cond_func = lambda n: n.aoa_started

        ## Remove RTLS RESPONDERS
        nodes_work_with = [n for n in nodes_to_set if not n.capabilities.get('RTLS_RESPONDER', False)]
        nodes_work_with = [n for n in nodes_work_with if not n.capabilities.get('RTLS_CONNECTION_MONITOR', False)]

        for node in nodes_work_with:
            node_role = 'AOA_COORDINATOR' if node.capabilities.get('RTLS_COORDINATOR', False) else 'AOA_PASSIVE'

            for responder in self._connected_responder:
                if conn_handle is not None:
                    if not (conn_handle == responder['conn_handle']):
                        continue

                node.aoa_started = False

                node.rtls.aoa_start(responder['conn_handle'], start, cte_interval, cte_length)
                self._rtls_wait(true_cond_func, node, f"node {responder['addr']} fail to start AoA")

    def aoa_start(self, cte_length, cte_interval, nodes=None, conn_handle=None):
        self._aoa_set_state(start=True, cte_length=cte_length, cte_interval=cte_interval, nodes=nodes,
                            conn_handle=conn_handle)
        self.add_user_log("AOA Started")

    def aoa_stop(self, nodes=None, conn_handle=None):
        self._aoa_set_state(start=False, nodes=nodes, conn_handle=conn_handle)
        self.add_user_log("AOA Stopped")

    ######

    # Periodic adv/Connectionless AOA

    def cl_aoa_start(self, cl_aoa_params, responder):
        if not self._padv_sync_dict[(responder['addr'], responder['advSID'])].cl_aoa_enable:
            self._cl_aoa_set_state(cl_aoa_params, responder, True)
            self.add_user_log("Connectionless AOA Started")
        else:
            raise RtlsUtilException("Connectionless AOA has already enabled.")

    def cl_aoa_stop(self, cl_aoa_params, responder):
        if self._padv_sync_dict[(responder['addr'], responder['advSID'])].cl_aoa_enable:
            self._cl_aoa_set_state(cl_aoa_params, responder, False)
            self.add_user_log("Connectionless AOA Stopped")
        else:
            self.add_user_log("Connectionless AOA has already disabled.")

    def _cl_aoa_set_state(self, cl_aoa_params, responder, enable):
        # Check responder type(str or dict)
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        elif isinstance(responder, dict):
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys() or 'advSID' not in responder.keys() or 'periodicAdvInt' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")
        else:
            raise RtlsUtilException("Input responder is nor string neither dict - invalid input")

        self._padv_current_responder = (responder['addr'], responder['advSID'])
        # Get sync handle
        sync_handle = self.padv_get_sync_handle_by_responder(responder)

        if sync_handle > -1:
            self._coordinator_node.rtls.connectionless_aoa_enable(cl_aoa_params['cl_aoa_role'],
                                                                  cl_aoa_params['cl_aoa_result_mode'],
                                                                  sync_handle,
                                                                  int(enable),
                                                                  cl_aoa_params['cl_aoa_slot_durations'],
                                                                  cl_aoa_params['cl_aoa_sample_rate'],
                                                                  cl_aoa_params['cl_aoa_sample_size'],
                                                                  cl_aoa_params['cl_aoa_sampling_control'],
                                                                  cl_aoa_params['max_sample_cte'],
                                                                  cl_aoa_params['cl_aoa_pattern_len'],
                                                                  cl_aoa_params['cl_aoa_ant_pattern']
                                                                  )

            true_cond_func = lambda sync_dict: sync_dict[self._padv_current_responder].cl_aoa_enable == enable
            self._rtls_wait(true_cond_func, self._padv_sync_dict, "Coordinator node to set connectionless aoa state")

    def padv_create_sync(self, responder, options, skip, syncTimeout, syncCteType):
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        elif isinstance(responder, dict):
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys() or 'advSID' not in responder.keys() or 'periodicAdvInt' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")
        else:
            raise RtlsUtilException("Input responder is nor string neither dict - invalid input")

        self._padv_current_responder = (responder['addr'], responder['advSID'])
        # Extract bit0 and bit 1 for later usage
        b0 = options & 1
        b1 = options >> 1 & 1

        if b0 == 1 \
                and self._padv_current_responder[0].lower() == "ff:ff:ff:ff:ff:ff" \
                and self._padv_current_responder in self._padv_sync_dict.keys():  # Connect from periodic advertiser list
            self._padv_sync_dict.pop(
                self._padv_current_responder)  # remove BD address of responder from list to start again

        if self._padv_current_responder not in self._padv_sync_dict.keys():
            self._padv_sync_dict[self._padv_current_responder] = ConnectionlessAOASync(False, False,
                                                                                       True if b1 == 0 else False,
                                                                                       False,
                                                                                       False, -1)
        else:
            self._padv_sync_dict[self._padv_current_responder].sync_report_enable = True if b1 == 0 else False

        self._coordinator_node.rtls.create_sync(responder['advSID'],
                                                options,
                                                responder['addrType'],
                                                responder['addr'],
                                                skip,
                                                syncTimeout,
                                                syncCteType
                                                )

        true_cond_func = lambda sync_dict: sync_dict[self._padv_current_responder].sync_created == True
        self._rtls_wait(true_cond_func, self._padv_sync_dict, "Coordinator node to create sync")

    def padv_create_sync_cancel(self):
        if self._padv_current_responder:
            if self._padv_current_responder in self._padv_sync_dict.keys():
                if not self._padv_sync_dict[self._padv_current_responder].sync_established:
                    self._coordinator_node.rtls.create_sync_cancel()

                    true_cond_func = lambda x: x[self._padv_current_responder].sync_created is False
                    self._rtls_wait(true_cond_func, self._padv_sync_dict, "Coordinator node to cancel sync")

                    self._padv_sync_dict.pop(self._padv_current_responder)
                else:
                    raise RtlsUtilException("Can not cancel sync after it has been established")
            else:
                raise RtlsUtilException(f"Responder address does not exist : {self._padv_current_responder}")
        else:
            raise RtlsUtilException(f"No current responder indicated")

        self._padv_current_responder = None

    def padv_terminate_sync(self, syncHandle):
        responder = self._padv_find_address_by_handle(syncHandle)
        self._padv_current_responder = (responder[0], responder[1])

        if self._padv_sync_dict[self._padv_current_responder].sync_established:
            self._coordinator_node.rtls.terminate_sync(syncHandle)

            true_cond_func = lambda x: x[self._padv_current_responder].sync_created is False
            self._rtls_wait(true_cond_func, self._padv_sync_dict, "Coordinator node to terminate sync")

            if not self._padv_sync_dict[self._padv_current_responder].in_padv_list:
                self._padv_sync_dict.pop(self._padv_current_responder)

            self._padv_current_responder = None

    def _padv_find_address_by_handle(self, syncHandle):
        ret_address = [key for key in self._padv_sync_dict if
                       syncHandle == self._padv_sync_dict[key].sync_handle]
        ret_address = ret_address[0] if len(ret_address) > 0 else None

        if ret_address is None:
            raise RtlsUtilException(f"No address matches to the wanted handle: {syncHandle}")

        return ret_address

    def padv_periodic_receive_enable(self, syncHandle):
        responder = self._padv_find_address_by_handle(syncHandle)
        self._padv_current_responder = (responder[0], responder[1])

        self._padv_sync_dict[self._padv_current_responder].sync_report_enable = False

        self._coordinator_node.rtls.periodic_receive_enable(syncHandle, 1)

        true_cond_func = lambda x: x.sync_report_enable
        self._rtls_wait(true_cond_func, self._padv_sync_dict[self._padv_current_responder],
                        "Coordinator node to enable periodic report receive")

    def padv_periodic_receive_disable(self, syncHandle):
        responder = self._padv_find_address_by_handle(syncHandle)
        self._padv_current_responder = (responder[0], responder[1])

        self._padv_sync_dict[self._padv_current_responder].sync_report_enable = True

        self._coordinator_node.rtls.periodic_receive_enable(syncHandle, 0)

        true_cond_func = lambda x: x.sync_report_enable is False
        self._rtls_wait(true_cond_func, self._padv_sync_dict[self._padv_current_responder],
                        "Coordinator node to disable periodic report receive")

    def padv_add_device_to_periodic_adv_list(self, responder):
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        elif isinstance(responder, dict):
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys() or 'advSID' not in responder.keys() or 'periodicAdvInt' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")
        else:
            raise RtlsUtilException("Input responder is nor string neither dict - invalid input")

        self._padv_current_responder = (responder['addr'], responder['advSID'])
        if self._padv_current_responder not in self._padv_sync_dict.keys():
            self._padv_sync_dict[self._padv_current_responder] = ConnectionlessAOASync(
                False, False, False, False, False, -1)

        self._coordinator_node.rtls.add_device_to_periodic_adv_list(responder['addrType'], responder['addr'],
                                                                    responder['advSID'])

        true_cond_func = lambda x: x.in_padv_list is True
        self._rtls_wait(true_cond_func, self._padv_sync_dict[(responder['addr'], responder['advSID'])],
                        "Add device to list event")

    def padv_remove_device_from_periodic_adv_list(self, responder):
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        elif isinstance(responder, dict):
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys() or 'advSID' not in responder.keys() or 'periodicAdvInt' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")
        else:
            raise RtlsUtilException("Input responder is nor string neither dict - invalid input")

        if (responder['addr'], responder['advSID']) not in self._padv_sync_dict.keys():
            raise RtlsUtilException(f"Input responder does not exits in responders dictionary: {responder['addr']}")
        elif not self._padv_sync_dict[(responder['addr'], responder['advSID'])].in_padv_list:
            raise RtlsUtilException(
                f"Input responder does not exits in embedded periodic advertisers list: {responder['addr']}")
        else:
            self._padv_current_responder = (responder['addr'], responder['advSID'])

            self._coordinator_node.rtls.remove_device_from_periodic_adv_list(responder['addrType'], responder['addr'],
                                                                             responder['advSID'])

            true_cond_func = lambda x: x.in_padv_list is False
            self._rtls_wait(true_cond_func, self._padv_sync_dict[(responder['addr'], responder['advSID'])],
                            "Remove device from list event")

    def padv_read_periodic_adv_list_size(self):
        self._padv_read_list_size_complete.clear()

        self._coordinator_node.rtls.read_periodic_adv_list_size()

        true_cond_func = lambda x: x.is_set()
        self._rtls_wait(true_cond_func, self._padv_read_list_size_complete, "Read list size event")

        return self.padv_list_size

    def padv_clear_periodic_adv_list(self):
        self._padv_clear_adv_list_complete.clear()

        self._coordinator_node.rtls.clear_periodic_adv_list()

        true_cond_func = lambda x: x.is_set()
        self._rtls_wait(true_cond_func, self._padv_clear_adv_list_complete, "Clear adv list event")

    def padv_get_sync_handle_by_responder(self, responder):
        if isinstance(responder, str):
            responder = self._get_responder_by_addr(responder)
            if responder is None:
                raise RtlsUtilScanResponderNotFoundException("Expected responder not found in scan list")
        elif isinstance(responder, dict):
            if 'addr' not in responder.keys() or 'addrType' not in responder.keys() or 'advSID' not in responder.keys() or 'periodicAdvInt' not in responder.keys():
                raise RtlsUtilException("Input responder not a string and not contains required keys")
        else:
            raise RtlsUtilException("Input responder is nor string neither dict - invalid input")

        if (responder['addr'], responder['advSID']) in self._padv_sync_dict.keys():
            return self._padv_sync_dict[(responder['addr'], responder['advSID'])].sync_handle
        else:
            return -1

    def padv_get_responder_by_sync_handle(self, sync_handle):
        for key, responder in self._padv_sync_dict.items():
            if responder.sync_handle == sync_handle:
                return key[0]

        return None

    def sync(self, responder, options, skip, syncTimeout, syncCteType, scan_time_sec):
        if skip != 0:
            timeout_condition = syncTimeout >= skip * responder['periodicAdvInt']
        else:
            timeout_condition = syncTimeout >= responder['periodicAdvInt']

        if timeout_condition:
            self.padv_create_sync(responder,
                                  options,
                                  skip,
                                  syncTimeout,
                                  syncCteType)
            # Scan again for sync established event
            self.scan(scan_time_sec, responder['addr'], responder['advSID'])

            sync_handle = self.padv_get_sync_handle_by_responder(responder)
        else:
            raise RtlsUtilException("Timeout condition does not satisfied")

        return sync_handle

    def read_heap_size(self):

        self._read_heap_size_complete.clear()

        self._coordinator_node.rtls.heap_req()

        true_cond_func = lambda x: x.is_set()
        self._rtls_wait(true_cond_func, self._read_heap_size_complete, "Read heap size event")

        return {'total_heap': self.total_heap, 'free_heap': self.free_heap}

    ######
