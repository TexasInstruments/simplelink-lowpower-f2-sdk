

/* ******* Add SYS/BIOS 7.x to the configuration ********* */
/* use bigInt for zero padding hex numbers */
let bigInt = system.utils.bigInt;

/* ================ Kernel (SYS/BIOS) configuration ================ */
const BIOS = scripting.addModule("/ti/sysbios/BIOS");

/*
 * Enable the TrustZone PSA extensions
 */
BIOS.psaEnabled = true;

/* ================ Hwi configuration ================ */
const HWI = scripting.addModule("/ti/sysbios/family/arm/v8m/Hwi");

/*
 * Both these values must match the SPE expectations for the vector table. The
 * base of RAM is reserved for the secure image. These values do not affect
 * TICLANG or GCC, as they place vector tables using the linker file.
 */
HWI.vectorTableAddress = bigInt(0x2000C000);
HWI.resetVectorAddress = bigInt(0x2000C000);

/* ================ Boot configuration ================ */
const Boot = scripting.addModule("/ti/sysbios/family/arm/cc26xx/Boot");

/*
 * Disable the execution of SetupTrimDevice(), which is already called by the
 * SPE before the application boots and requires access to secure resources.
 */
Boot.trimDevice = false;
