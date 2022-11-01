

/* ******* Add FreeRTOS to the configuration ********* */


/* ================ Kernel (FreeRTOS) configuration ================ */
const FreeRTOS = scripting.addModule("/freertos/FreeRTOS");

const POSIX = scripting.addModule("/ti/posix/freertos/Settings");

if (system.modules["/ti/utils/TrustZone"]) {
    FreeRTOS.psaEnabled = true;

    /*
     * Enable the secure callback module
     */
    const SecureCallback = scripting.addModule("/ti/drivers/spe/SecureCallback");
}
