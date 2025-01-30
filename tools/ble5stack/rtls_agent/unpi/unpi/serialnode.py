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

from queue import PriorityQueue, Empty

from serial import Serial, SerialException
from .unpiparser import UNPIParser, UNPIMessage, QMessage
import threading
import logging
logger = logging.getLogger('serialnode')


def b2ascii(b):
    return ':'.join(["%02X" % y for y in b])


def builder_class(builderclass):
    def builder_dec(func):
        def call_builder(self, *args, **kwargs):
            return self.sender(builderclass.build(*args, **kwargs))

        def wrapper(self, *args, **kwargs):
            return call_builder(self, *args, **kwargs)
        return wrapper
    return builder_dec


class SerialNode(threading.Thread):
    def __init__(self, port, speed: int, inQ: PriorityQueue, outQ: PriorityQueue, ss_commands_dict, name=None):
        super(SerialNode, self).__init__(name=name)
        self.ser = None
        self.exception = None
        self.port = port
        self.speed = speed
        self.inQ = inQ
        self.outQ = outQ
        self.stopEvent = threading.Event()
        self.startedEvent = threading.Event()
        self.parser = UNPIParser(ss_commands_dict)
        self.inBuffer = bytes()

    @property
    def stopped(self):
        return self.stopEvent.is_set()

    def stop(self):
        self.stopEvent.set()

    def run(self):
        try:
            self.ser = Serial(self.port, baudrate=self.speed, timeout=0.05)
            self.startedEvent.set()

            while not self.stopped:
                try:
                    outMsg = self.outQ.get_nowait()
                    m = outMsg
                    if type(outMsg) is QMessage:
                        m = outMsg.item
                    outframe = self.parser.build(m.type, m.subsystem, m.command, data=bytes(m.data))
                    # print(">> " + ':'.join(['%02X' % x for x in outframe]))
                    logger.debug("[{}] >>> {}".format(self.port, m))
                    logger.debug("[{}] >>> ".format(self.port) + b2ascii(outframe))
                    self.ser.write(outframe)
                except Empty:
                    pass

                # _in = self.ser.read(4096)
                # if _in:
                #     self.inBuffer += _in
                if self.ser.inWaiting() > 0:
                    self.inBuffer += self.ser.read(self.ser.inWaiting())
                    # print(_in)
                    logger.debug("<<< " + b2ascii(self.inBuffer))
                    while True:
                        p, self.inBuffer = self.parser.parse_stream(self.inBuffer)
                        logger.debug("[{}] <<< ".format(self.port) + b2ascii(self.inBuffer))
                        if p is None:
                            break
                        logger.debug("[{}] <<< {}".format(self.port, p))
                        self.inQ.put(QMessage(1, p), block=True)

                        if self.outQ.qsize() > 0:
                            logger.debug(f"&&& {self.port} We found something in outQ while we parsing input messages, we will break and return")
                            break



        except SerialException as e:
            self.exception = e
            logger.error(str(e))

        finally:
            if self.ser is not None:
                self.ser.close()
            self.stop()


if __name__ == '__main__':
    cmd_types = dict(SNP=0x15)
    commands = dict(DEVICE_POWERUP=0x1, GAP_START_ADV=0x42, GAP_STOP_ADV=0x44, GATT_ADD_SERVICE=0x81)

    inQueue = PriorityQueue()
    outQueue = PriorityQueue()

    hciReset = QMessage(1, UNPIMessage(1, 0x15, 4, bytes([0x1D, 0xFC, 0x01])))

    node = SerialNode('COM48', 115200, inQueue, outQueue, cmd_types, commands)
    node.start()

    # inQueue.get(block=True)
    outQueue.put(hciReset)

    try:
        while True:
            pass
    finally:
        node.stop()
        node.join()
