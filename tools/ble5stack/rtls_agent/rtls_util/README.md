# RTLS Util
This package is a layer between the user and internal modules.
 
This package based on: 
* rtls
* unpi

RTLS package used for RTLS Manager and RTLS Node see [README](../rtls/README.md).
UNPI package user for comunincation and parsing data between RTLS package and devices see [README](../unpi/README.md).
 
 
 ## Code example
```python
    import queue
    import json
    
    from rtls_util import RtlsUtil, RtlsUtilLoggingLevel, RtlsUtilException

    logging_file = "log.txt"
    ## Initialize RTLS Util instance
    rtlsUtil = RtlsUtil(logging_file, RtlsUtilLoggingLevel.INFO)
    
    devices = [
        {"com_port": "COM37", "baud_rate": 460800, "name": "CC26x2 Coordinator"},
        {"com_port": "COM29", "baud_rate": 460800, "name": "CC26x2 Passive"}
    ]
    ## Setup devices
    coordinator_node, passive_nodes, all_nodes = rtlsUtil.set_devices(devices)
    print(f"Coordinator : {coordinator_node} \nPassives : {passive_nodes} \nAll : {all_nodes}")

    ## Reset devices for initial state of devices
    rtlsUtil.reset_devices()
    print("Devices Reset")
    
    ## Scan for slave for 15 sec 
    scan_results = rtlsUtil.scan(15)
    print(f"Scan Results: {scan_results}")

    ## Connect with connection interval 100 mSec
    rtlsUtil.ble_connect(scan_results[0], 100)
    print("Connection Success")    

    aoa_params = {
        "aoa_run_mode": "AOA_MODE_RAW",
        "aoa_cc26x2": {
            "aoa_slot_durations": 2,
            "aoa_sample_rate": 1,
            "aoa_sample_size": 1,
            "aoa_sampling_control": 0x10,
            "aoa_sampling_enable": 1,
            "aoa_pattern_len": 36,
            "aoa_ant_pattern": [0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 6, 7,
                                8, 9, 10, 11, 6, 7, 8, 9, 10, 11]
       }
    }
    rtlsUtil.aoa_set_params(aoa_params)
    print("AOA Paramas Set")

    rtlsUtil.aoa_start(cte_length=20, cte_interval=1)
    print("AOA Started")
    
    while True:
        try:
            data = rtlsUtil.aoa_results_queue.get(block=True, timeout=0.5)
            print(json.dumps(data))
        except queue.Empty:
            continue
            
    rtlsUtil.aoa_stop()
    print("AOA Stopped")
    
    if rtlsUtil.ble_connected:
        rtlsUtil.ble_disconnect()
        print("Master Disconnected")

    rtlsUtil.done()
    print("Done")
```

## More Examples 

Look at [rtls_connected.py](../examples/rtls_connected.py) or [rtls_connectionless.py](../examples/rtls_connectionless.py) 