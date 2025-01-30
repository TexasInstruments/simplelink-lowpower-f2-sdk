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
import queue
import sys
import time

from rtls import RTLSNode, Subscriber

logging.basicConfig(stream=sys.stdout, level=logging.INFO,
                    format='[%(asctime)s] {%(filename)s:%(lineno)d} %(levelname)s - %(message)s')

try:
    passivenode1 = RTLSNode('COM8', 115200, "Passive")
    passivenode1.start()

    coordinatorNode = RTLSNode('COM14', 115200, "Coordinator")
    coordinatorNode.start()

    tofsub = Subscriber(queue=queue.PriorityQueue(), interest=None, transient=False, eventloop=None)
    coordinatorNode.add_subscriber(tofsub)

    passivesub1 = Subscriber(queue=queue.PriorityQueue(), interest=None, transient=False, eventloop=None)
    passivenode1.add_subscriber(passivesub1)

    # Globals
    # Scan list
    scanResultList = list()

    # Modes/Configurations
    rtlsModeTof = 1
    rtlsModeAoa = 0
    responderAddr = '54:6C:0E:9B:66:1D' # Responder addr should be set here

    # Global variables/flags
    seed = 0

    passive1SeedRequired = 0
    coordinatorEnabled = 0
    Average = 0

    # Script start
    #coordinatorNode.rtls.reset_device()
    #passivenode1.rtls.reset_device()

    coordinatorNode.rtls.scan()

    while True:
        try:
            item = tofsub.pend(False)
            msg = item.item
            # logging.info(msg)
            logging.info(" >>COORDINATOR" + msg.as_json())

            if msg.command == 'RTLS_CMD_SCAN' and msg.type == 'AsyncReq':
                scanResultList.append(msg.payload.addr)
                scanResultList.append(msg.payload.addrType)

            if msg.command == 'RTLS_CMD_SCAN_STOP':
                if responderAddr in scanResultList:
                    i = scanResultList.index(responderAddr)
                    coordinatorNode.rtls.connect(scanResultList[i+1], scanResultList[i])
                else:
                    coordinatorNode.rtls.scan()

            if msg.command == 'RTLS_CMD_CONNECT' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SUCCESS':
                if rtlsModeTof == 1:
                    time.sleep(2)
                    coordinatorNode.rtls.tof_set_params('TOF_COORDINATOR',
                                                   64,
                                                   6,
                                                   -55,
                                                   'TOF_MODE_DIST',
                                                   'TOF_MODE_CONT',
                                                   [2408, 2412, 2414, 2418, 2420, 2424])
                if rtlsModeAoa == 1:
                    time.sleep(1)
                    coordinatorNode.rtls.aoa_set_params('AOA_COORDINATOR', 'AOA_MODE_PAIR_ANGLES', 4, 4, 20, 0)
                    time.sleep(1)
                    coordinatorNode.rtls.aoa_start(1)

            if msg.command == 'RTLS_CMD_CONN_PARAMS' and msg.type == 'AsyncReq' and msg.payload.accessAddress is not 0:
                passivenode1.rtls.set_ble_conn_info(msg.payload.accessAddress, msg.payload.connInterval, msg.payload.hopValue, msg.payload.mSCA, msg.payload.currChan, msg.payload.chanMap, msg.payload.crcInit)

            if msg.command == 'RTLS_CMD_TOF_SET_PARAMS' and msg.payload.status == 'RTLS_SUCCESS':
                if rtlsModeTof == 1:
                    coordinatorNode.rtls.tof_get_sec_seed()

            if msg.command == 'RTLS_CMD_TOF_GET_SEC_SEED' and msg.payload.seed is not 0:
                if rtlsModeTof == 1:
                    seed = msg.payload.seed
                    if passive1SeedRequired == 1:
                        passivenode1.rtls.tof_set_sec_seed(seed)

            if msg.command == 'RTLS_CMD_TOF_CALIBRATE' and msg.type == 'AsyncReq':
                if rtlsModeTof == 1:
                    print("Coordinator calibration complete")

        except queue.Empty:
            pass

        # passive node #1
        try:
            item = passivesub1.pend(True, 0.05)
            msg = item.item
            logging.info(" >> PASSIVE1" + msg.as_json())

            if msg.command == 'RTLS_CMD_CONNECT' and msg.type == 'AsyncReq' and msg.payload.status == 'RTLS_SUCCESS':
                time.sleep(1)
                if rtlsModeTof == 1:
                    passivenode1.rtls.tof_set_params('TOF_PASSIVE',
                                                     64,
                                                     6,
                                                     -55,
                                                     'TOF_MODE_DIST',
                                                     'TOF_MODE_CONT',
                                                     [2408, 2412, 2414, 2418, 2420, 2424])
                if rtlsModeAoa == 1:
                    time.sleep(1)
                    passivenode1.rtls.aoa_set_params('AOA_PASSIVE', 'AOA_MODE_ANGLE', 4, 4, 20, 0)
                    time.sleep(1)
                    passivenode1.rtls.aoa_start(1)

            if msg.command == 'RTLS_CMD_TOF_SET_PARAMS' and msg.payload.status == 'RTLS_SUCCESS':
                if rtlsModeTof == 1:
                    if seed is not 0:
                        passivenode1.rtls.tof_set_sec_seed(seed)
                    else:
                        passive1SeedRequired = 1

            if msg.command == 'RTLS_CMD_TOF_SET_SEC_SEED' and msg.payload.status == 'RTLS_SUCCESS':
                if rtlsModeTof == 1:
                    passivenode1.rtls.tof_calib(1, 0, 1)
                    coordinatorNode.rtls.tof_calib(1, 0, 1)

            if msg.command == 'RTLS_CMD_TOF_CALIBRATE' and msg.type == 'SyncRsp' and msg.payload.status == 'RTLS_SUCCESS':
                if rtlsModeTof == 1:
                    if coordinatorEnabled == 0:
                        passivenode1.rtls.tof_start(1)

            if msg.command == 'RTLS_CMD_TOF_ENABLE' and msg.payload.status == 'RTLS_SUCCESS':
                if rtlsModeTof == 1:
                    coordinatorNode.rtls.tof_start(1)
                    coordinatorEnabled = 1
                    time.sleep(5)
                    passivenode1.rtls.tof_calib(0, 0, 1)
                    coordinatorNode.rtls.tof_calib(0, 0, 1)

            if msg.command == 'RTLS_CMD_TOF_CALIBRATE' and msg.type == 'AsyncReq':
                if rtlsModeTof == 1:
                    print("Passive1 calibration complete")

        except queue.Empty:
            pass


finally:
    if coordinatorNode is not None:
        coordinatorNode.stop()
        coordinatorNode.wait_stopped()

    if passivenode1 is not None:
        passivenode1.stop()
        passivenode1.wait_stopped()

