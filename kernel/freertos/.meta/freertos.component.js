let displayName = "FreeRTOS";
let description = "FreeRTOS Global Settings";

let topModules = [
    {
        displayName: displayName,
        description: description,
        "modules": [
            "/freertos/FreeRTOS"
        ]
    }
];

let templates = [
    {
        "name": "/freertos/ti_freertos_config.h.xdt",
        "outputPath": "FreeRTOSConfig.h",
        "alwaysRun": false
    },
    {
        "name": "/freertos/ti_freertos_config.c.xdt",
        "outputPath": "ti_freertos_config.c",
        "alwaysRun": false
    },
    {
        "name": "/freertos/ti_freertos_portable_config.c.xdt",
        "outputPath": "ti_freertos_portable_config.c",
        "alwaysRun": false
    }
];

let finalTopModules = [];

/*
 * If nortos or tirtos is configured, omit the FreeRTOS part of the list. Since
 * it is needed for compatibility and tooling, continue to display the modules
 * in the UI if RTOS is "none" - but if the user references the FreeRTOS module,
 * it will throw a validation error demanding the user switch to --rtos freertos.
 */
if (system.getRTOS() == "freertos" || system.getRTOS() == "none")
{
    finalTopModules = topModules;
}

exports = {
    displayName: "FreeRTOS",
    topModules: finalTopModules,
    templates: templates
};
