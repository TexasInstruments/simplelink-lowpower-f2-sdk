import os
import queue
import sys
import threading
import time
import datetime
import json
import subprocess

import pandas as pd

## Uncomment line below for local debug of packages
# sys.path.append(r"../unpi")
# sys.path.append(r"../rtls")
# sys.path.append(r"../rtls_util")

from rtls_util import RtlsUtil, RtlsUtilLoggingLevel, RtlsUtilException, RtlsUtilTimeoutException, \
    RtlsUtilCoordinatorNotFoundException, RtlsUtilEmbeddedFailToStopScanException, \
    RtlsUtilScanResponderNotFoundException, RtlsUtilScanNoResultsException, RtlsUtilNodesNotIdentifiedException


class RtlsConnectedExample():
    def __init__(self, coordinator_comport, passive_comports, connection_monitor_comports, active_responder_comports,
                 responder_bd_addrs, scan_time, connection_interval, continues_connection_info, angle_of_arrival,
                 angle_of_arrival_params, data_collection_duration, data_collection_iteration, logging_directory=None,
                 post_analyze_func=None, timeout=30):
        self.headers = ['pkt', 'sample_idx', 'rssi', 'ant_array', 'channel', 'i', 'q', 'slot_duration', 'sample_rate',
                        'filtering', 'identifier', 'connHandle']

        self.coordinator_comport = coordinator_comport
        self.passive_comports = passive_comports
        self.connection_monitor_comports = connection_monitor_comports
        self.active_responder_comports = active_responder_comports
        self.responder_bd_addrs = responder_bd_addrs

        self.devices = None
        self.coordinator_node = None
        self.passive_nodes = None
        self.connection_monitor_nodes = None
        self.responders_nodes = None
        self.all_nodes = None

        self.scan_time = scan_time
        self.connection_interval = connection_interval
        self.all_conn_handles = []

        self.enable_continues_connection_info = continues_connection_info

        self.enable_angle_of_arrival = angle_of_arrival
        self.angle_of_arrival_params = angle_of_arrival_params

        self.data_collection_duration = data_collection_duration
        self._data_collection_iteration = data_collection_iteration
        self._current_iteration = -1
        self.csv_files = []

        self.logging_directory = logging_directory
        self.post_analyze_func = post_analyze_func
        self.timeout = timeout

        self._data_collection_in_process = threading.Event()
        self._data_collection_in_process.clear()

        self._continues_connection_info_thread = None
        self._angle_of_arrival_thread = None

    @property
    def data_collection_iteration(self):
        return self._data_collection_iteration

    @property
    def current_iteration(self):
        return self._current_iteration

    @current_iteration.setter
    def current_iteration(self, valuw):
        print(f"\n {'=' * 10} Starting Loop {valuw + 1} {'=' * 10}")
        self._current_iteration = valuw

    def _get_example_parameters(self):
        example_parameters = "\n"
        example_parameters += "Example Input Parameters\n"
        example_parameters += "----------------------------------------------------------------------\n"
        example_parameters += f"Coordinator comport                     : {self.coordinator_comport}\n"
        example_parameters += f"Passive comports                        : {self.passive_comports}\n"
        example_parameters += f"Connection monitor comports             : {self.connection_monitor_comports}\n"
        example_parameters += f"Active responder comports               : {self.active_responder_comports}\n"
        example_parameters += f"Responder BD Address                    : {self.responder_bd_addrs}\n"
        example_parameters += f"Scan time                               : {self.scan_time} Sec\n"
        example_parameters += f"Connection Interval                     : {self.connection_interval} mSec\n"
        example_parameters += f"Enable Continues Connection Interval    : {'Yes' if self.enable_continues_connection_info else 'No'}\n"
        example_parameters += f"Enable Angle Of Arrival (AoA)           : {'Yes' if self.enable_angle_of_arrival else 'No'}\n"
        example_parameters += f"    AoA Enable Filter                   : {'Yes' if self.angle_of_arrival_params.get('enable_filter', True) else 'No'}\n"
        example_parameters += f"    AoA Slot Duration                   : {self.angle_of_arrival_params.get('slot_duration', 2)}\n"
        example_parameters += f"    AoA Sample Rate                     : {self.angle_of_arrival_params.get('sample_rate', 1)}\n"
        example_parameters += f"    AoA Sample Size                     : {self.angle_of_arrival_params.get('sample_size', 1)}\n"
        example_parameters += f"    AoA CTE Length                      : {self.angle_of_arrival_params.get('cte_length', 20)}\n"
        example_parameters += f"    AoA CTE Interval                    : {self.angle_of_arrival_params.get('cte_interval', 2)}\n"
        example_parameters += f"Data Collection Duration                : {self.data_collection_duration}\n"
        example_parameters += f"Data Collection iteration               : {self.data_collection_iteration}\n"
        example_parameters += f"Provide Post Analyze Function           : {'Yes' if self.post_analyze_func else 'No'}\n"
        example_parameters += f"Example Log Dir                         : {os.path.dirname(self.logging_file)}\n"
        example_parameters += f"Example Log File Name                   : {os.path.basename(self.logging_file)}\n"
        example_parameters += "----------------------------------------------------------------------\n"
        example_parameters += "\n"

        return example_parameters

    def _get_date_time(self):
        return datetime.datetime.now().strftime("%m_%d_%Y_%H_%M_%S")

    def _get_logging_file_path(self):
        data_time = self._get_date_time()

        if self.logging_directory is not None:
            logging_file_path = self.logging_directory
        else:
            logging_file_path = os.path.join(os.path.curdir, os.path.basename(__file__).replace('.py', '_log'))

        if not os.path.isdir(logging_file_path):
            os.makedirs(logging_file_path)

        logging_file = os.path.join(logging_file_path,
                                    f"{data_time}_{os.path.basename(__file__).replace('.py', '.log')}")

        return os.path.abspath(logging_file)

    def _get_csv_file_name_for(self, identifier, conn_handle, loop_count, with_date=True):
        data_time = self._get_date_time()
        logging_file_path = self._get_logging_file_path()
        logging_file_dir = os.path.dirname(logging_file_path)

        identifier = identifier.lower().replace(":", "")
        if with_date:
            filename = os.path.join(logging_file_dir,
                                    f"{data_time}_rtls_raw_iq_samples_{identifier}_{conn_handle}_loop{loop_count}.csv")
        else:
            filename = os.path.join(logging_file_dir,
                                    f"rtls_raw_iq_samples_{identifier}_{conn_handle}_loop{loop_count}.csv")

        return filename

    def initialize(self, logging_level=RtlsUtilLoggingLevel.INFO):
        try:
            self.logging_file = self._get_logging_file_path()

            self.rtls_util = RtlsUtil(self.logging_file, logging_level)
            self.rtls_util.timeout = self.timeout

            self.rtls_util.add_user_log(self._get_example_parameters(), print_to_terminal=True)

            print("Coordinator set into device list")
            self.devices = [{"com_port": self.coordinator_comport, "baud_rate": 460800, "name": "Coordinator"}]

            if self.passive_comports and isinstance(self.passive_comports, list):
                print("Passive comports added to into device list")
                for index, passive_comport in enumerate(self.passive_comports):
                    self.devices.append({"com_port": passive_comport, "baud_rate": 460800, "name": f"Passive {index}"})

            if self.connection_monitor_comports and isinstance(self.connection_monitor_comports, list):
                print("Connection Monitor comports added to into device list")
                for index, connection_monitor_comport in enumerate(self.connection_monitor_comports):
                    self.devices.append({"com_port": connection_monitor_comport, "baud_rate": 460800,
                                         "name": f"Connection Monitor {index}"})

            if self.active_responder_comports and isinstance(self.active_responder_comports, list):
                print("Responder comports added to into device list")
                for index, responder_comport in enumerate(self.active_responder_comports):
                    self.devices.append(
                        {"com_port": responder_comport, "baud_rate": 460800, "name": f"Responder {index}"})

            ## Setup devices
            self.coordinator_node, self.passive_nodes, self.connection_monitor_nodes, self.responders_nodes, \
            self.all_nodes = self.rtls_util.set_devices(self.devices)
            # print(f"Coordinator : {self.coordinator_node} \nPassives : {self.passive_nodes}
            #                                       \mRespondes : {self.responders_nodes} \nAll : {self.all_nodes}")

            self.rtls_util.reset_devices()
            print("Devices Reset")

            return True

        except RtlsUtilCoordinatorNotFoundException:
            print('No one of given devices identified as RTLS Coordinator')
            return False

        except RtlsUtilNodesNotIdentifiedException:
            print('One of given devices not identified as RTLS type')
            return False

    def scan_and_connect_to_responders(self):
        # self.all_conn_handles = []
        if self.responder_bd_addrs is None:
            return False

        try:
            print(f"\nCoordinator start scan for {self.scan_time} sec")
            scan_results = self.rtls_util.scan(self.scan_time)
            suitable_responders = [scan_result for scan_result in scan_results if scan_result['periodicAdvInt'] == 0]
            print("Founded Responders: \n\t{}".format(
                '\n\t'.join([str(suitable_responders) for suitable_responders in suitable_responders])))

            ## Code below demonstrates how to connect to multiple responders
            if not self.responder_bd_addrs:
                # Sort Scan Results by RSSI and take only 8 first responders
                self.responder_bd_addrs = sorted(suitable_responders, key=lambda s: s['rssi'], reverse=True)[:8]

            for responder_bd_addr in self.responder_bd_addrs:
                try:
                    if type(responder_bd_addr) == dict:
                        responder_bd_addr = responder_bd_addr['addr']

                    print(f"\nTrying connect to {responder_bd_addr}")
                    conn_handle = self.rtls_util.ble_connect(responder_bd_addr, self.connection_interval)
                    if conn_handle is not None:
                        self.all_conn_handles.append(conn_handle)
                        print(f"Connected to {responder_bd_addr} with connection handle {conn_handle}")
                    else:
                        print(f"Failed to connect to {responder_bd_addr}")
                except RtlsUtilScanResponderNotFoundException:
                    print(f"Failed to connect to {responder_bd_addr}")
                    continue
                except RtlsUtilTimeoutException:
                    print(f"Failed to connect to {responder_bd_addr}")
                    continue

            return len(self.all_conn_handles) > 0

        except RtlsUtilEmbeddedFailToStopScanException:
            return False
        except RtlsUtilScanNoResultsException:
            return False
        except RtlsUtilException:
            return False

    def disconnect(self):
        for conn_handle in self.all_conn_handles[:]:
            try:
                self.rtls_util.ble_disconnect(conn_handle=conn_handle)
                print(f"\nCoordinator disconnected from responder with conn handle {conn_handle}")
                self.all_conn_handles.remove(conn_handle)
            except RtlsUtilTimeoutException:
                print(f"\nCoordinator fail to disconnect from responder with conn handle {conn_handle}")
                continue

    def scan_and_connect_to_coordinator(self):
        if self.responders_nodes is None or self.responders_nodes == []:
            return False

        coordinator_addr = self.rtls_util._coordinator_node.identifier

        for responder in self.responders_nodes:
            try:
                print(f"\nResponder {responder.identifier} [{responder.port}] start scan for {self.scan_time} sec")
                scan_results = self.rtls_util.scan(self.scan_time, node=responder)

                print("\nFounded Coordinators: \n\t{}".format('\n\t'.join([str(result) for result in scan_results])))

                print(f"\nResponder {responder.identifier} [{responder.port}] trying connect to {coordinator_addr}")
                conn_handle = self.rtls_util.ble_connect(coordinator_addr, self.connection_interval, node=responder)

                if conn_handle is not None:
                    self.all_conn_handles.append(conn_handle)
                    print(f"Connected to {coordinator_addr} success with connection handle {conn_handle}")
                else:
                    print(f"Failed connect to {coordinator_addr}")

            except RtlsUtilEmbeddedFailToStopScanException:
                print(f"\nResponder {responder.identifier} [{responder.port}] fail to stop scan")
                continue
            except RtlsUtilScanNoResultsException:
                print(f"\nResponder {responder.identifier} [{responder.port}] fail to find any coordinator")
                continue
            except RtlsUtilException:
                print(f"\nResponder {responder.identifier} [{responder.port}] failed connect to {coordinator_addr}")
                continue

        return True

    # Thread for Continuous Connection Info  monitoring
    # Should not be called by user's application.
    # Please use functions start_continues_connection_info and stop_continues_connection_info instead.
    def _continues_connection_info_result(self):
        while self._data_collection_in_process.is_set():
            data_time = datetime.datetime.now().strftime("[%m:%d:%Y %H:%M:%S:%f] :")

            try:
                cci_data = self.rtls_util.conn_info_queue.get_nowait()
                print(f"{data_time} {json.dumps(cci_data)}")
            except queue.Empty:
                pass

    def start_continues_connection_info(self):
        self._continues_connection_info_thread = None

        if self.enable_continues_connection_info:
            self._continues_connection_info_thread = threading.Thread(target=self._continues_connection_info_result)
            self._continues_connection_info_thread.daemon = True

            self._continues_connection_info(True)

    def stop_continues_connection_info(self):
        if self.enable_continues_connection_info:
            self._continues_connection_info(False)

        self._continues_connection_info_thread = None
        self.rtls_util._empty_queue(self.rtls_util.conn_info_queue)

    def _continues_connection_info(self, enable):
        if enable:
            self.rtls_util.cci_start()
            print("Continues Connection Information Started\n")
        else:
            self.rtls_util.cci_stop()
            print("\nContinues Connection Information Stopped")

    def _execute_post_analyze_on_generated_csv(self):
        if self.post_analyze_func:
            for csv_file in self.csv_files:
                print(f"\nExecuting post analyze script on {csv_file}")
                output = self.post_analyze_func(csv_file)
                self.rtls_util.add_user_log(output)

    # Thread for AoA sampling collection and storage
    # Should not be called by user's application.
    # Please use functions start_angle_of_arrival and stop_angle_of_arrival instead.
    def _angle_of_arrival_result(self):
        TABLE = None
        pkt_count = 0
        current_csv_files = set()
        self.csv_files = []

        while self._data_collection_in_process.is_set():
            data_time = datetime.datetime.now().strftime("[%m:%d:%Y %H:%M:%S:%f] :")

            try:
                aoa_data = self.rtls_util.aoa_results_queue.get_nowait()
                # print(f"{data_time} {json.dumps(aoa_data)}")

                if 'type' in aoa_data and aoa_data['type'] == "RTLS_CMD_AOA_RESULT_RAW":
                    calc_sample_count = 624
                    enable_filter = self.angle_of_arrival_params.get('enable_filter', True)
                    filtering = int(not enable_filter)
                    slot_duration = self.angle_of_arrival_params.get('slot_duration', 2)
                    sample_rate = self.angle_of_arrival_params.get('sample_rate', 1)
                    if enable_filter:
                        calc_sample_count = int((8 + 74 * (filtering + 1) / slot_duration) * sample_rate)

                    if TABLE is None:
                        TABLE = pd.DataFrame(columns=self.headers)

                    identifier = aoa_data["identifier"]
                    connHandle = aoa_data['payload']['connHandle']
                    channel = int(aoa_data['payload']['channel'])
                    offset = int(aoa_data['payload']['offset'])
                    rssi = aoa_data['payload']['rssi']
                    antenna = 6
                    samplesLength = aoa_data['payload']["samplesLength"]

                    if not calc_sample_count == int(samplesLength):
                        self.rtls_util.add_user_log(
                            f"RTLS_CMD_AOA_RESULT_RAW was dropped because received samplesLength {samplesLength} is not as expected {calc_sample_count}")
                        continue

                    df_by_channel = TABLE.loc[(TABLE['channel'] == channel) & (TABLE['identifier'] == identifier) & (
                            TABLE['connHandle'] == connHandle)]

                    rows = []
                    for indx, sample in enumerate(aoa_data['payload']['samples']):
                        sample_idx = offset + indx
                        if sample_idx in list(df_by_channel['sample_idx']):
                            TABLE = TABLE.drop(
                                TABLE[(TABLE['channel'] == channel) & (
                                        TABLE['sample_idx'] == sample_idx)].index.values
                            )

                        sample_i = sample['i']
                        sample_q = sample['q']

                        row = {
                            'pkt': 0,
                            'sample_idx': sample_idx,
                            'rssi': rssi,
                            'ant_array': antenna,
                            'channel': channel,
                            'i': sample_i,
                            'q': sample_q,
                            'slot_duration': slot_duration,
                            'sample_rate': sample_rate,
                            'filtering': filtering,
                            'identifier': identifier,
                            'connHandle': connHandle
                        }
                        rows.append(row)

                    TABLE = TABLE.append(rows, ignore_index=True)

                    df_by_channel = TABLE.loc[(TABLE['channel'] == channel) & (TABLE['identifier'] == identifier) & (
                            TABLE['connHandle'] == connHandle)]

                    if len(df_by_channel) == samplesLength:
                        df_by_channel = df_by_channel.drop('identifier', 1)
                        df_by_channel = df_by_channel.drop('connHandle', 1)

                        df_by_channel = df_by_channel.sort_values(by=['sample_idx'])
                        df_by_channel.loc[:, "pkt"] = df_by_channel.loc[:, "pkt"].replace(to_replace=0,
                                                                                          value=pkt_count)

                        csv_file_name = self._get_csv_file_name_for(identifier, connHandle, self.current_iteration + 1,
                                                                    with_date=False)
                        current_csv_files.add(csv_file_name)

                        if os.path.isfile(csv_file_name):
                            df_by_channel.to_csv(csv_file_name, mode='a', index=False, header=False)
                        else:
                            df_by_channel.to_csv(csv_file_name, index=False)
                        print(f"{data_time} Added new set of IQ into {csv_file_name}")

                        pkt_count += 1
                        TABLE = TABLE.loc[(TABLE['channel'] != channel) & (TABLE['identifier'] == identifier) & (
                                TABLE['connHandle'] == connHandle)]

            except queue.Empty:
                pass

        for current_csv_file in list(current_csv_files):
            current_csv_file_dirname = os.path.dirname(current_csv_file)
            current_csv_file_basename = os.path.basename(current_csv_file)
            dt = self._get_date_time()

            new_csv_file = os.path.join(current_csv_file_dirname, f"{dt}_{current_csv_file_basename}")
            os.rename(current_csv_file, new_csv_file)
            print(f"Rename \"{os.path.basename(current_csv_file)}\" into \"{os.path.basename(new_csv_file)}\"")

            self.csv_files.append(new_csv_file)

    def start_angle_of_arrival(self):
        self._angle_of_arrival_thread = None

        if self.enable_angle_of_arrival:
            self._angle_of_arrival_thread = threading.Thread(target=self._angle_of_arrival_result)
            self._angle_of_arrival_thread.daemon = True

            self._angle_of_arrival(True)

    def stop_angle_of_arrival(self):
        if self.enable_angle_of_arrival:
            self._angle_of_arrival(False)

        self._angle_of_arrival_thread = None
        self.rtls_util._empty_queue(self.rtls_util.aoa_results_queue)

        self._execute_post_analyze_on_generated_csv()

    def _angle_of_arrival(self, enable):
        if enable:
            aoa_params = {
                "aoa_run_mode": "AOA_MODE_RAW",
                "aoa_cc26x2": {
                    "aoa_slot_durations": self.angle_of_arrival_params.get('slot_duration', 2),
                    "aoa_sample_rate": self.angle_of_arrival_params.get('sample_rate', 1),
                    "aoa_sample_size": self.angle_of_arrival_params.get('sample_size', 1),
                    "aoa_sampling_control": int(
                        '0x10' if self.angle_of_arrival_params.get('enable_filter', True) else '0x11', 16),
                    "aoa_sampling_enable": 1,
                    "aoa_pattern_len": 36,
                    "aoa_ant_pattern": [0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 6, 7,
                                        8, 9, 10, 11, 6, 7, 8, 9, 10, 11]
                }
            }
            self.rtls_util.aoa_set_params(aoa_params)
            print("AOA Params Set")

            self.rtls_util.aoa_start(cte_length=self.angle_of_arrival_params.get('cte_length', 20),
                                     cte_interval=self.angle_of_arrival_params.get('cte_interval', 2))
            print("AOA Started\n")
        else:
            self.rtls_util.aoa_stop()
            print("\nAOA Stopped")

    def sleep(self):
        self._data_collection_in_process.set()

        if self._continues_connection_info_thread:
            self._continues_connection_info_thread.start()

        if self._angle_of_arrival_thread:
            self._angle_of_arrival_thread.start()

        print(f"\nExample will now wait for result for {self.data_collection_duration} sec\n")
        timeout = time.time() + self.data_collection_duration
        while timeout >= time.time():
            time.sleep(0.1)

        self._data_collection_in_process.clear()

        if self._continues_connection_info_thread:
            self._continues_connection_info_thread.join()

        if self._angle_of_arrival_thread:
            self._angle_of_arrival_thread.join()

    def done(self):
        self.rtls_util.done()
        # print("Done done!")


def post_analyze_of_IQ_data(csv_file):
    print(f"Analyzing file : {csv_file}")
    # Place here you code for IQ data post-analyze algorithms
    return "Return here output of your post-analyze algorithm"


def main():
    example = RtlsConnectedExample(
        coordinator_comport="COM5",  # "/dev/cu.usbmodemL1100KKT1",
        passive_comports=None,  # None - Disable this option, ['COMx'] - at least one comport
        connection_monitor_comports=None,  # None - Disable this option, ['COMx'] - at least one comport
        active_responder_comports=None,  # None - Disable this option, ['COMx'] - at least one comport
        responder_bd_addrs=[],
        # None - Disable this option, [] - Connect All, ['80:6F:B0:1E:39:02', '80:6F:B0:1E:38:C3'] - connect to specific bd addresses
        scan_time=10,
        connection_interval=100,
        continues_connection_info=False,
        angle_of_arrival=True,
        angle_of_arrival_params={
            'enable_filter': True,
            'slot_duration': 2,
            'sample_rate': 1,
            'sample_size': 1,
            'cte_length': 20,
            'cte_interval': 2
        },
        data_collection_duration=30,
        data_collection_iteration=1,
        post_analyze_func=post_analyze_of_IQ_data
    )

    # Perform initialization of RTLS Util, setup devices and reset them
    # Parameters used: coordinator_comport, passive_comports
    if example.initialize():

        # Perform Scan and Connect with required responders
        # Parameters used: responder_bd_addrs, scan_time, connection_interval
        if any([example.scan_and_connect_to_coordinator(), example.scan_and_connect_to_responders()]):

            # Execute next action in loop
            # Parameters used: data_collection_iteration
            for example.current_iteration in range(example.data_collection_iteration):
                # Start RSSI vs Channel report (if enabled)
                # Parameters used: continues_connection_info
                example.start_continues_connection_info()

                # Start Angle of Arrival report (if enabled)
                # Parameters used: angle_of_arrival, angle_of_arrival_params
                example.start_angle_of_arrival()

                # Example start all inner threads for result collecting and sleep for data_collection_duration
                # Parameters used: data_collection_duration
                example.sleep()

                # Stop RSSI vs Channel report (if enabled)
                # Parameters used: continues_connection_info
                example.stop_continues_connection_info()

                # Stop Angle of Arrival report (if enabled) and analyze collected data using post_analyze_func
                # Parameters used: angle_of_arrival, post_analyze_func
                example.stop_angle_of_arrival()

            # Disconnect from all connected responders
            example.disconnect()

    # Close all open inner threads of RTLS Util
    example.done()
    print("Example ended")


if __name__ == '__main__':
    main()
