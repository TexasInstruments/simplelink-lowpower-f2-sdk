RTLS
====

This package includes
 * RTLSManager
 * RTLSNode
 * RTLS SubSystem (ss_rtls.py)

An `RTLSManager` instance controls any number of `RTLSNode` instances,
and funnels the incoming RTLS events into a single queue.

An `RTLSNode` instance is tied to a specific serial port, and depends
on the `unpi.SerialNode` class to receive data, and the RTLS Subsystem
class `RTLS` from `ss_rtls.py` to parse incoming data and build outgoing
frames.


RTLSManager
-----------

```python
node1 = RTLSNode(port='COM23', baudrate=115200, name='NodeOne')
node2 = RTLSNode(port='COM24', baudrate=115200, name='NodeTwo')

# Instantiate RTLSManager with two nodes and no websocket proxy
manager = RTLSManager(nodes=[node1, node2], websocket_port=None)

# Configure automatic distribution of RTLS Coordinator connection parameter messages to RTLS Passive nodes
manager.auto_params = True 

# Get a new subscriber object
subscriber = manager.create_subscriber()

# Start the manager
manager.start()

# Wait until nodes have responded to automatic identify command and get reference
# to single coordinator RTLSNode and list of passive RTLSNode instances
coordinator, passives, connection_monitors, responders, failed = manager.wait_identified()

# Wait for messages
while True:
    try:
        identifier, msg_pri, message = subscriber.pend().as_tuple()
        from_node = manager[identifier]
        # ..
        # do stuff
    except queue.Empty:
        pass
```
