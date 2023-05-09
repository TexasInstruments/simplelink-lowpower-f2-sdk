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
#

import threading
import asyncio


class BluetoothManager(object):

    def __init__(self):
        self.__loop = None

    def run(self):
        self.__bleak_thread = threading.Thread(target=self.run_bleak_loop)
        # Discard thread quietly on exit.
        self.__bleak_thread.daemon = True
        self.__bleak_thread_ready = threading.Event()
        self.__bleak_thread.start()
        # Wait for thread to start.
        self.__bleak_thread_ready.wait()

    def await_bleak(self, coroutine, timeout=None):
        future = asyncio.run_coroutine_threadsafe(coroutine, self.__loop)
        return future.result(timeout)

    def run_bleak_loop(self):
        self.__loop = asyncio.new_event_loop()
        self.__bleak_thread_ready.set()
        self.__loop.run_forever()

    def stop(self):
        self.__loop.stop()
        self.__bleak_thread_ready.clear()
