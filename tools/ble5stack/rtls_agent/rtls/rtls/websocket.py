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
logger = logging.getLogger('rtls_websocket')
import queue
import threading

import websockets


class WSServer(threading.Thread):
    def __init__(self, loop, on_connect, on_disconnect, port=8766):
        super().__init__()
        self.loop = loop  # asyncio.new_event_loop()
        self.connected = dict()
        self.on_connect = on_connect
        self.on_disconnect = on_disconnect
        self.port = port
        self.stop_future = None
        self.server = None
        self.stopEvent = threading.Event()

    async def consumer_handler(self, websocket, path, inQ):
        try:
            async for message in websocket:
                # await inQ.put(message)
                inQ.put(message)
        except websockets.exceptions.ConnectionClosed as e:
            pass

    async def producer_handler(self, websocket, path, outQ):
        while True:
            message = await outQ.get()
            outQ.task_done()
            if hasattr(message, 'item'):
                msg = message.item
                await websocket.send(msg.as_json())
            elif hasattr(message, 'as_json'):
                await websocket.send(message.as_json())
            elif isinstance(message, str):
                await websocket.send(message)

    async def handler(self, websocket, path):
        # socket_in_queue = asyncio.Queue(loop=self.loop)
        socket_in_queue = queue.Queue()
        socket_out_queue = asyncio.Queue(loop=self.loop)

        arg0 = self.on_connect(websocket, socket_in_queue, socket_out_queue)

        consumer_task = asyncio.ensure_future(self.consumer_handler(websocket, path, socket_in_queue))
        producer_task = asyncio.ensure_future(self.producer_handler(websocket, path, socket_out_queue))
        pending = None
        try:
            done, pending = await asyncio.wait([consumer_task, producer_task], return_when=asyncio.FIRST_COMPLETED)
        except Exception as e:
            logger.error(e)
        finally:
            if pending:
                for task in pending:
                    task.cancel()
            self.on_disconnect(websocket, arg0, socket_in_queue, socket_out_queue)

    def run(self):
        asyncio.set_event_loop(self.loop)

        loop = self.loop

        start_server = websockets.serve(self.handler, 'localhost', self.port)
        self.server = loop.run_until_complete(start_server)

        self.stop_future = loop.create_future()
        loop.run_until_complete(self.stop_future)

        self.server.close()
        loop.run_until_complete(self.server.wait_closed())
        loop.run_until_complete(loop.shutdown_asyncgens())
        loop.close()

    def stop(self):
        if not self.stopEvent.is_set():
            self.stopEvent.set()
            try:
                self.loop.call_soon_threadsafe(self.stop_future.set_result, None)

            except RuntimeError as e:
                raise e
                pass  # If event loop is closed already, who cares at this point

