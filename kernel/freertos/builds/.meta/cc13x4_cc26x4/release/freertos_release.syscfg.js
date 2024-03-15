

/* ******* Add FreeRTOS to the configuration ********* */

/*eslint-disable no-unused-vars*/

/* ================ Kernel (FreeRTOS) configuration ================ */
const FreeRTOS = scripting.addModule("/freertos/FreeRTOS");

const POSIX = scripting.addModule("/ti/posix/freertos/Settings");

if (system.modules["/ti/utils/TrustZone"]) {
    /*
     * Enable the secure callback module
     */
    const SecureCallback = scripting.addModule("/ti/drivers/tfm/SecureCallback");

    /* Set Secure image reset vector table address */
    const CCFG = scripting.addModule("/ti/devices/CCFG");

    CCFG.setFlashVectorTable = true;
    CCFG.addressFlashVectorTable = 0x0000D100;
}

/*eslint-enable no-unused-vars*/