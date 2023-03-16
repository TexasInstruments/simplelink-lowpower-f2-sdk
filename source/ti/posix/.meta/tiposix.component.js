
let topModules = [];
let displayName = "";

/* Only show the Settings module for tirtos7 */
if (system.getRTOS() == "tirtos7") {
    displayName = "TI RTOS";
    topModules = [
        {
            displayName: "TI RTOS",
            description: "POSIX API support for TI-RTOS",
            "modules": [
                "/ti/posix/tirtos/Settings"
            ]
        }
    ];
} else if (system.getRTOS() == "freertos") {
    displayName = "FreeRTOS";
    topModules = [
        {
            displayName: "FreeRTOS",
            description: "POSIX API support for FreeRTOS",
            "modules": [
                "/ti/posix/freertos/Settings"
            ]
        }
    ];
}

exports = {
    displayName: displayName,
    topModules: topModules
};
