/// \addtogroup module_scif_generic_interface
//@{
#include "scif_framework.h"
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_aon_event.h)
#include DeviceFamily_constructPath(inc/hw_aon_pmctl.h)
#include DeviceFamily_constructPath(inc/hw_aon_rtc.h)
#include DeviceFamily_constructPath(inc/hw_aux_sce.h)
#include DeviceFamily_constructPath(inc/hw_aux_smph.h)
#include DeviceFamily_constructPath(inc/hw_aux_evctl.h)
#include DeviceFamily_constructPath(inc/hw_aux_aiodio.h)
#include DeviceFamily_constructPath(inc/hw_aux_anaif.h)
#include DeviceFamily_constructPath(inc/hw_aux_sysif.h)
#include DeviceFamily_constructPath(inc/hw_event.h)
#include DeviceFamily_constructPath(inc/hw_ints.h)
#include DeviceFamily_constructPath(inc/hw_ioc.h)
#include <string.h>
#if defined(__IAR_SYSTEMS_ICC__)
    #include <intrinsics.h>
#endif


/// Driver internal data (located in MCU domain RAM, not shared with the Sensor Controller)
static SCIF_DATA_T scifData;


// OSAL dependencies
static void scifSetMcuwusel(int index, uint32_t wuSignal);


/// Import OSAL
#define SCIF_INCLUDE_OSAL_C_FILE
#include "scif_osal_tirtos.c"




/// Task data structure buffer control: Size (in bytes)
#define SCIF_TASK_STRUCT_CTRL_SIZE                  (3 * sizeof(uint16_t))
/// Task data structure buffer control: Sensor Controller Engine's pointer negative offset (ref. struct start)
#define SCIF_TASK_STRUCT_CTRL_SCE_ADDR_BACK_OFFSET  (3 * sizeof(uint16_t))
/// Task data structure buffer control: Driver/MCU's pointer negative offset (ref. struct start)
#define SCIF_TASK_STRUCT_CTRL_MCU_ADDR_BACK_OFFSET  (2 * sizeof(uint16_t))


/// OPMODEREQ bit-vector: Power-down
#define BV_AUX_SYSIF_OPMODEREQ_PD   AUX_SYSIF_OPMODEREQ_REQ_PDA
/// OPMODEREQ bit-vector: Power-down
#define BV_AUX_SYSIF_OPMODEREQ_LP   AUX_SYSIF_OPMODEREQ_REQ_LP




/** \brief Initializes a single I/O pin for Sensor Controller usage
  *
  * This function must be called for each I/O pin to be used after AUX I/O latching has been set
  * transparent. It configures (in the indicated order):
  * - AIODIO:
  *     - IOMODE
  *     - GPIODOUT
  *     - GPIODIE
  * - IOC:
  *     -IOCFGn (index remapped using \ref scifData.pAuxIoIndexToMcuIocfgOffsetLut[])
  *
  * \param[in]      auxIoIndex
  *     Index of the I/O pin, 0-31, using AUX mapping
  * \param[in]      ioMode
  *     Pin I/O mode, consisting of:
  *     - Bits [31:28]
  *         - Output drive strength
  *             - 0 = Low (2 mA for all I/O pins)
  *             - 1 = Medium (4 mA for all I/O pins)
  *             - 2 = High (4 mA for normal I/O pins / 8 mA for high-drive capability I/O pins)
  *     - Bits [27:0]
  *         - \ref AUXIOMODE_OUTPUT
  *         - \ref AUXIOMODE_INPUT
  *         - \ref AUXIOMODE_OPEN_DRAIN
  *         - \ref AUXIOMODE_OPEN_DRAIN_WITH_INPUT
  *         - \ref AUXIOMODE_OPEN_SOURCE
  *         - \ref AUXIOMODE_OPEN_SOURCE_WITH_INPUT
  *         - \ref AUXIOMODE_ANALOG
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_EV_OBS
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_SPIM_SCLK
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_SPIM_MOSI
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_TIMER2_EV0
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_TIMER2_EV1
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_TIMER2_EV2
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_TIMER2_EV3
  *         - \ref AUXIOMODE_PERIPH_OUTPUT_TIMER2_PULSE
  * \param[in]      pullLevel
  *     Pull level to be used when the pin is configured as input, open-drain or open-source
  *     - No pull: -1
  *     - Pull-down: 0
  *     - Pull-up: 1
  * \param[in]      outputValue
  *     Initial output value when the pin is configured as output, open-drain or open-source
  */
void scifInitIo(uint32_t auxIoIndex, uint32_t ioMode, int pullLevel, uint32_t outputValue) {

    // Calculate access parameters from the AUX I/O index
    uint32_t auxAiodioPin  = auxIoIndex & 0x07;
    uint32_t auxAiodioBase = AUX_AIODIO0_BASE + ((auxIoIndex / 8) * (AUX_AIODIO1_BASE - AUX_AIODIO0_BASE));

    // Setup the AUX I/O controller. The ioMode parameter is packed as follows:
    // - ioMode[31:28] = Output drive strength
    // - ioMode[27:24] = IOPSEL value
    // - ioMode[23:16] = GPIODIE value shifted down to bit 0
    // - ioMode[15:8] = IOPOE value shifted down to bit 0
    // - ioMode[7:0] = IOMODE value shifted down to bit 0
    HWREG(auxAiodioBase + AUX_AIODIO_O_IO0PSEL + ((AUX_AIODIO_O_IO1PSEL - AUX_AIODIO_O_IO0PSEL) * auxAiodioPin)) = (ioMode >> 24) & 0x0F;
    HWREG(auxAiodioBase + AUX_AIODIO_O_IOPOE)    = (HWREG(auxAiodioBase + AUX_AIODIO_O_IOPOE)    & ~(0x01 << (auxAiodioPin)))     | (((ioMode >> 8) & 0xFF) << auxAiodioPin);
    HWREG(auxAiodioBase + AUX_AIODIO_O_IOMODE)   = (HWREG(auxAiodioBase + AUX_AIODIO_O_IOMODE)   & ~(0x03 << (2 * auxAiodioPin))) | ((ioMode & 0xFF) << (2 * auxAiodioPin));
    HWREG(auxAiodioBase + AUX_AIODIO_O_GPIODOUT) = (HWREG(auxAiodioBase + AUX_AIODIO_O_GPIODOUT) & ~(0x01 << (auxAiodioPin)))     | (outputValue << auxAiodioPin);
    HWREG(auxAiodioBase + AUX_AIODIO_O_GPIODIE)  = (HWREG(auxAiodioBase + AUX_AIODIO_O_GPIODIE)  & ~(0x01 << (auxAiodioPin)))     | (((ioMode >> 16) & 0xFF) << auxAiodioPin);
    // Ensure that the settings have taken effect
    HWREG(auxAiodioBase + AUX_AIODIO_O_GPIODIE);

    // Configure pull level and transfer control of the I/O pin to AUX
    scifReinitIo(auxIoIndex, pullLevel, ioMode >> BI_AUXIOMODE_OUTPUT_DRIVE_STRENGTH);

} // scifInitIo




/** \brief Re-initializes a single I/O pin for Sensor Controller usage
  *
  * This function must be called after the AUX AIODIO has been initialized, or when reinitializing I/Os
  * that have been lent temporarily to MCU domain peripherals. It only configures the following:
  * - IOC:
  *     -IOCFGn (index remapped using \ref scifData.pAuxIoIndexToMcuIocfgOffsetLut[])
  *
  * \param[in]      auxIoIndex
  *     Index of the I/O pin, 0-15, using AUX mapping
  * \param[in]      pullLevel
  *     Pull level to be used when the pin is configured as input, open-drain or open-source
  *     - No pull: -1
  *     - Pull-down: 0
  *     - Pull-up: 1
  * \param[in]      driveStrength
  *     Output drive strength
  *     - 0 = Low (2 mA for all I/O pins)
  *     - 1 = Medium (4 mA for all I/O pins)
  *     - 2 = High (4 mA for normal I/O pins / 8 mA for high-drive capability I/O pins)
  */
void scifReinitIo(uint32_t auxIoIndex, int pullLevel, int driveStrength) {

    // Calculate access parameters from the AUX I/O index
    uint32_t mcuIocfgOffset = scifData.pAuxIoIndexToMcuIocfgOffsetLut[auxIoIndex];

    // Setup the MCU I/O controller, making the AUX I/O setup effective
    uint32_t iocfg = IOC_IOCFG0_PORT_ID_AUX_IO | (driveStrength << IOC_IOCFG0_IOCURR_S);
    switch (pullLevel) {
    case -1: iocfg |= IOC_IOCFG0_PULL_CTL_DIS; break;
    case 0:  iocfg |= IOC_IOCFG0_PULL_CTL_DWN; break;
    case 1:  iocfg |= IOC_IOCFG0_PULL_CTL_UP; break;
    }
    HWREG(IOC_BASE + IOC_O_IOCFG0 + mcuIocfgOffset) = iocfg;

} // scifReinitIo




/** \brief Uninitializes a single I/O pin after Sensor Controller usage
  *
  * This detaches the I/O pin from the AUX domain, and configures it as GPIO with input/output disabled
  * and the specified pull level.
  *
  * \param[in]      auxIoIndex
  *     Index of the I/O pin, 0-31, using AUX mapping
  * \param[in]      pullLevel
  *     Pull level
  *     - No pull: -1
  *     - Pull-down: 0
  *     - Pull-up: 1
  */
void scifUninitIo(uint32_t auxIoIndex, int pullLevel) {

    // Calculate access parameters from the AUX I/O index
    uint32_t mcuIocfgOffset = scifData.pAuxIoIndexToMcuIocfgOffsetLut[auxIoIndex];

    // Unconfigure the MCU I/O controller (revert to GPIO with input/output disabled and desired pull
    // level)
    uint32_t iocfg = IOC_IOCFG0_PORT_ID_GPIO;
    switch (pullLevel) {
    case -1: iocfg |= IOC_IOCFG0_PULL_CTL_DIS; break;
    case 0:  iocfg |= IOC_IOCFG0_PULL_CTL_DWN; break;
    case 1:  iocfg |= IOC_IOCFG0_PULL_CTL_UP; break;
    }
    HWREG(IOC_BASE + IOC_O_IOCFG0 + mcuIocfgOffset) = iocfg;

} // scifUninitIo




/** \brief Returns the index of the least significant '1' in the given bit-vector
  *
  * \param[in]      x
  *     The bit-vector
  *
  * \return
  *     The bit index of the least significant '1', for example 2 for 0x0004, or 32 if all bits are '0'
  */
static int scifFindLeastSignificant1(uint32_t x) {
#if defined(__IAR_SYSTEMS_ICC__) || defined(DOXYGEN)
    return __CLZ(__RBIT(x));
#elif defined(__TI_COMPILER_VERSION__) || defined(__CC_ARM)
    return __clz(__rbit(x));
#else
    return __builtin_ctz(x);
#endif
} // scifFindLeastSignificant1




/** \brief Selects SCE operation mode
  *
  * This function can only be called while the SCE is halted. The function ensures that the LP request is
  * not changed while the PD request is asserted.
  *
  * \param[in]      newOpmodereq
  *     The new power mode:
  *     - AUX_SYSIF_OPMODEREQ_REQ_A
  *     - AUX_SYSIF_OPMODEREQ_REQ_LP
  *     - AUX_SYSIF_OPMODEREQ_REQ_PDA
  *     - AUX_SYSIF_OPMODEREQ_REQ_PDLP
  */
void scifSetSceOpmode(uint32_t newOpmodereq) {
    uint32_t currOpmodereq = HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_OPMODEREQ);
    uint32_t opmodeack = HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_OPMODEACK);

    // If running at 24 MHz in active mode ...
    if (scifData.activeModeSceClkFreqMhz == 24) {

        // Reconfigure the Reference DAC clock divider before entering active mode
        if (!(newOpmodereq & BV_AUX_SYSIF_OPMODEREQ_LP)) {
            HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACSMPLCFG0) = 12 - 1;
        }
    }

    // Until the desired power mode has been reached ...
    while (1) {

        // Wait until the acknowledgment matches the request
        while (opmodeack != currOpmodereq) {
            opmodeack = HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_OPMODEACK);
        }

        // We're done when the new power mode has been reached
        if (currOpmodereq == newOpmodereq) {
            break;
        }

        // Exit power-down?
        if (currOpmodereq & BV_AUX_SYSIF_OPMODEREQ_PD) {
            currOpmodereq &= ~BV_AUX_SYSIF_OPMODEREQ_PD;

        // Switch between low-power and active mode?
        } else if ((currOpmodereq ^ newOpmodereq) & BV_AUX_SYSIF_OPMODEREQ_LP) {
            currOpmodereq = newOpmodereq & BV_AUX_SYSIF_OPMODEREQ_LP;

        // Enter power-down?
        } else {
            currOpmodereq = newOpmodereq;
        }

        // Request new mode
        HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_OPMODEREQ) = currOpmodereq;
    }

    // If running at 24 MHz in active mode ...
    if (scifData.activeModeSceClkFreqMhz == 24) {

        // Reconfigure the Reference DAC clock divider after leaving active mode
        if (newOpmodereq & BV_AUX_SYSIF_OPMODEREQ_LP) {
            HWREG(AUX_ANAIF_BASE + AUX_ANAIF_O_DACSMPLCFG0) = 1 - 1;
        }
    }

} // scifSetSceOpmode




/** \brief Selects an MCU domain wake-up source
  *
  * \param[in]      index
  *     The AON_EVENT:MCUWUSEL event index to be modified, i.e. one of the following:
  *     - OSAL_CTRL_READY_MCUWUSEL_WU_EV_INDEX
  *     - OSAL_TASK_ALERT_MCUWUSEL_WU_EV_INDEX
  * \param[in]      wuSignal
  *     The wake-up signal to be selected for \a index, i.e. one of the following:
  *     - AON_EVENT_MCUWUSEL_WU0_EV_AUX_SWEV0
  *     - AON_EVENT_MCUWUSEL_WU0_EV_AUX_SWEV1
  *     - AON_EVENT_MCUWUSEL_WU0_EV_AUX_NONE
  */
static void scifSetMcuwusel(int index, uint32_t wuSignal) {
    uint32_t key;
    key = scifOsalEnterCriticalSection();
    uint32_t mcuwusel = HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL + (index & 0x04));
    mcuwusel &= ~(AON_EVENT_MCUWUSEL_WU0_EV_M << (8 * (index & 0x03)));
    mcuwusel |= wuSignal << (8 * (index & 0x03));
    HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL + (index & 0x04)) = mcuwusel;
    scifOsalLeaveCriticalSection(key);
} // scifSetMcuwusel




/** \brief Initializes the driver
  *
  * This function must be called to enable the driver for operation. The function:
  * - Verifies that the driver is not already active
  * - Stores a local copy of the driver setup data structure, \ref SCIF_DATA_T
  * - Configures AUX domain hardware modules for operation (general and setup-specific). This includes
  *   complete I/O setup for all Sensor Controller tasks.
  * - Loads the generated AUX RAM image into the AUX RAM
  * - Initializes handshaking mechanisms for control, alert interrupt generation and data exchange
  * - Configures use of AUX domain wake-up sources
  * - Starts the Sensor Controller
  *
  * The AUX RAM is powered on by default after chip reset. This function assumes that the AUX RAM is
  * still powered on.
  *
  * \param[in]      *pScifDriverSetup
  *     Driver setup, containing all relevant pointers and parameters for operation
  *
  * \return
  *     \ref SCIF_SUCCESS if successful, or \ref SCIF_ILLEGAL_OPERATION if the Sensor Controller already
  *     is active. The function call has no effect if unsuccessful.
  */
SCIF_RESULT_T scifInit(const SCIF_DATA_T* pScifDriverSetup) {

    // Perform sanity checks: The Sensor Controller cannot already be active
    if (HWREG(AUX_SCE_BASE + AUX_SCE_O_CTL) & AUX_SCE_CTL_CLK_EN_M) {
        return SCIF_ILLEGAL_OPERATION;
    }

    // Copy the driver setup
    memcpy(&scifData, pScifDriverSetup, sizeof(SCIF_DATA_T));

    // Reset wake-up event sources and flags, and setup the Sensor Controller vector table
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_WUGATE) = 0;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU0CFG) = 0x0000;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU1CFG) = 0x0000;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU2CFG) = 0x0000;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU3CFG) = 0x0000;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_WUFLAGSCLR) = 0x00FF;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_WUFLAGSCLR) = 0x0000;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG0) = AUX_SYSIF_VECCFG0_VEC_EV_SW_WU0;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG1) = AUX_SYSIF_VECCFG5_VEC_EV_SW_WU1;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG2) = AUX_SYSIF_VECCFG1_VEC_EV_PROG_WU0;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG3) = AUX_SYSIF_VECCFG2_VEC_EV_PROG_WU1;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG4) = AUX_SYSIF_VECCFG3_VEC_EV_PROG_WU2;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG5) = AUX_SYSIF_VECCFG4_VEC_EV_PROG_WU3;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG6) = AUX_SYSIF_VECCFG6_VEC_EV_SW_WU2;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_VECCFG7) = AUX_SYSIF_VECCFG7_VEC_EV_NONE;
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_WUGATE) = 1;

    // Upload the AUX RAM image
    memcpy((void*) AUX_RAM_BASE, scifData.pAuxRamImage, scifData.auxRamImageSize);

    // Select SW_WU0 as reset vector
    HWREG(AUX_SCE_BASE + AUX_SCE_O_CTL) = 0 << AUX_SCE_CTL_RESET_VECTOR_S;

    // Perform driver setup specific initialization
    scifData.fptrDriverSetupInit();

    // Set the READY event (to the MCU domain)
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_SWEVSET) = AUX_EVCTL_SWEVSET_SWEV0_M;

    // The System CPU shall wake up on the ALERT event
    scifSetMcuwusel(OSAL_TASK_ALERT_MCUWUSEL_WU_EV_INDEX, AON_EVENT_MCUWUSEL_WU0_EV_AUX_SWEV1);

    // Start the Sensor Controller
    HWREG(AUX_SCE_BASE + AUX_SCE_O_CTL) |= AUX_SCE_CTL_CLK_EN_M;

    // Register, clear and enable the interrupts
    osalRegisterCtrlReadyInt();
    osalClearCtrlReadyInt();
    osalEnableCtrlReadyInt();
    osalRegisterTaskAlertInt();
    osalClearTaskAlertInt();
    scifOsalEnableTaskAlertInt();

    return SCIF_SUCCESS;

} // scifInit




/** \brief Uninitializes the driver in order to release hardware resources or switch to another driver
  *
  * All Sensor Controller tasks must have been stopped before calling this function, to avoid leaving
  * hardware modules used in an unknown state. Also, any tick generation must have been stopped to avoid
  * leaving handshaking with the tick source in an unknown state.
  *
  * This function will wait until the Sensor Controller is sleeping before shutting it down.
  */
void scifUninit(void) {

    // Wait until the Sensor Controller is idle (it might still be running, though not for long)
    while (!(HWREG(AUX_SCE_BASE + AUX_SCE_O_CPUSTAT) & AUX_SCE_CPUSTAT_SLEEP_M));

    // Stop and reset the Sensor Controller Engine
    HWREG(AUX_SCE_BASE + AUX_SCE_O_CTL) = AUX_SCE_CTL_CLK_EN_M | AUX_SCE_CTL_RESTART_M;
    scifSetSceOpmode(AUX_SYSIF_OPMODEREQ_REQ_A);
    HWREG(AUX_SCE_BASE + AUX_SCE_O_CTL) = AUX_SCE_CTL_CLK_EN_M | AUX_SCE_CTL_SUSPEND_M;
    HWREG(AUX_SCE_BASE + AUX_SCE_O_CTL) = 0x0000;

    // Power down the Sensor Controller
    scifSetSceOpmode(AUX_SYSIF_OPMODEREQ_REQ_PDA);

    // Disable the Sensor Controller task ALERT event as wake-up source for the MCU domain
    scifSetMcuwusel(OSAL_TASK_ALERT_MCUWUSEL_WU_EV_INDEX, AON_EVENT_MCUWUSEL_WU0_EV_NONE);

    // Disable interrupts
    osalDisableCtrlReadyInt();
    osalUnregisterCtrlReadyInt();
    scifOsalDisableTaskAlertInt();
    osalUnregisterTaskAlertInt();

    // Perform task resource uninitialization
    scifData.fptrDriverSetupUninit();

} // scifUninit




/** \brief Returns a bit-vector indicating the ALERT events associcated with the last ALERT interrupt
  *
  * This function shall be called by the application after it has received an ALERT interrupt, to find
  * which events have occurred.
  *
  * When all the alert events have been handled, the application must call \ref scifAckAlertEvents().
  * After acknowledging, this function must not be called again until the next ALERT event has been
  * received.
  *
  * \return
  *     The event bit-vector contains the following fields:
  *     - [15:8] Task input/output handling failed due to underflow/overflow, one bit per task ID
  *     - [7:0] Task input/output data exchange pending, one bit per task ID
  */
uint32_t scifGetAlertEvents(void) {
    return scifData.pTaskCtrl->bvTaskIoAlert;
} // scifGetAlertEvents




/** \brief Clears the ALERT interrupt source
  *
  * The application must call this function once and only once after reception of an ALERT interrupt,
  * before calling \ref scifAckAlertEvents().
  */
void scifClearAlertIntSource(void) {

    // Clear the source, and wait for it to take effect. The source must be cleared repeatedly in case
    // task code calls fwGenQuickAlertInterrupt() frequently (the flag can be set again, immediately
    // after it has been cleared here).
    do {
        HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGSCLR) = AUX_EVCTL_EVTOAONFLAGS_SWEV1_M;
    } while (HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGS) & AUX_EVCTL_EVTOAONFLAGS_SWEV1_M);

} // scifClearAlertIntSource




/** \brief Acknowledges the ALERT events associcated with the last ALERT interrupt
  *
  * This function shall be called after the handling the events associated with the last ALERT interrupt.
  *
  * The application must call this function once and only once after reception of an ALERT event,
  * after calling \ref scifClearAlertIntSource(). It must not be called unless an ALERT event has
  * occurred.
  *
  * \note Calling this function can delay (by a short period of time) the next task to be executed.
  */
void scifAckAlertEvents(void) {

    // Clear the events that have been handled now. This is needed for subsequent ALERT interrupts
    // generated by fwGenQuickAlertInterrupt(), since that procedure does not update bvTaskIoAlert.
    scifData.pTaskCtrl->bvTaskIoAlert = 0x0000;

    // Make sure that the CPU interrupt has been cleared before reenabling it
    osalClearTaskAlertInt();
    uint32_t key = scifOsalEnterCriticalSection();
    scifOsalEnableTaskAlertInt();

    // Set the ACK event to the Sensor Controller
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_SWWUTRIG) = AUX_SYSIF_SWWUTRIG_SW_WU2_M;
    scifOsalLeaveCriticalSection(key);

} // scifAckAlertEvents




/** \brief Selects whether or not the ALERT interrupt shall wake up the System CPU
  *
  * If the System CPU is in standby mode when the Sensor Controller generates an ALERT interrupt, the
  * System CPU will wake up by default.
  *
  * Call this function to disable or re-enable System CPU wake-up on ALERT interrupt. This can be used to
  * defer ALERT interrupt processing until the System CPU wakes up for other reasons, for example to
  * handle radio events, and thereby avoid unnecessary wake-ups.
  *
  * Note that there can be increased current consumption in System CPU standby mode if the ALERT
  * interrupt is disabled (by calling \ref scifOsalDisableTaskAlertInt()), but wake-up is enabled. This
  * is because the wake-up signal will remain asserted until \ref scifAckAlertEvents() has been called
  * for all pending ALERT events.
  *
  * The behavior resets to enabled when \ref scifInit() is called.
  *
  * \param[in]      enable
  *     Set to false to disable System CPU wake-up on ALERT interrupt, or true to reenable wake-up.
  */
void scifSetWakeOnAlertInt(bool enable) {
    if (enable) {
        scifData.pIntData->alertCanPdAuxMask = 0x0000;
        scifSetMcuwusel(OSAL_TASK_ALERT_MCUWUSEL_WU_EV_INDEX, AON_EVENT_MCUWUSEL_WU0_EV_AUX_SWEV1);
    } else {
        scifData.pIntData->alertCanPdAuxMask = 0xFFFF;
        scifSetMcuwusel(OSAL_TASK_ALERT_MCUWUSEL_WU_EV_INDEX, AON_EVENT_MCUWUSEL_WU0_EV_NONE);
    }
} // scifSetWakeOnAlertInt




/** \brief Sets the initial task startup delay, in ticks
  *
  * This function may be used when starting multiple tasks at once, allowing for either:
  * - Spreading the execution times, for reduced peak current consumption and precise execution timing
  * - Aligning the execution times, for reduced total current consumption but less precise timing for
  *   lower-priority tasks
  *
  * If used, note the following:
  * - It replaces the call to \c fwScheduleTask() from the "Initialization Code"
  * - This function must be used with care when timer-based tasks are already running
  * - This function must always be called when starting the relevant tasks
  *
  * \param[in]      taskId
  *     ID of the task to set startup delay for
  * \param[in]      ticks
  *     Number of timer ticks until the first execution
  */
void scifSetTaskStartupDelay(uint32_t taskId, uint16_t ticks) {
    scifData.pTaskExecuteSchedule[taskId] = ticks;
} // scifSetTaskStartupDelay




/** \brief Resets the task data structures for the specified tasks
  *
  * This function must be called before tasks are restarted. The function resets the following:
  * - The \c state data structure, and optionally the \c cfg, \c input and \c output data structures
  * - The execution schedule table entry, to cancel any last call to \c fwScheduleTask() in task code
  *
  * \param[in]      bvTaskIds
  *     Bit-vector indicating which tasks to reset (where bit N corresponds to task ID N)
  * \param[in]      bvTaskStructs
  *     Bit-vector indicating which task data structure types to reset in addition to \c state for these
  *     tasks. Make a bit-vector of \ref SCIF_STRUCT_CFG, \ref SCIF_STRUCT_INPUT and
  *     \ref SCIF_STRUCT_OUTPUT
  */
void scifResetTaskStructs(uint32_t bvTaskIds, uint32_t bvTaskStructs) {

    // Indicate that the data structure has been cleared
    scifData.bvDirtyTasks &= ~bvTaskIds;

    // Always clean the state data structure
    bvTaskStructs |= (1 << SCIF_STRUCT_STATE);

    // As long as there are more tasks to reset ...
    while (bvTaskIds) {
        uint32_t taskId = scifFindLeastSignificant1(bvTaskIds);
        bvTaskIds &= ~(1 << taskId);

        // Reset the execution schedule entry
        scifData.pTaskExecuteSchedule[taskId] = 0;

        // For each data structure to be reset ...
        uint32_t bvStructs = bvTaskStructs;
        do {
            int n = scifFindLeastSignificant1(bvStructs);
            bvStructs &= ~(1 << n);
            uint32_t taskStructInfo = scifData.pTaskDataStructInfoLut[(taskId * 4) + n];

            // If it exists ...
            if (taskStructInfo) {

                // Split the information
                uint16_t addr   = (taskStructInfo >> 0)  & 0x0FFF; // 11:0
                uint16_t count  = (taskStructInfo >> 12) & 0x00FF; // 19:12
                uint16_t size   = (taskStructInfo >> 20) & 0x0FFF; // 31:20
                uint16_t length = sizeof(uint16_t) * size * count;

                // If multiple-buffered, include the control variables
                if (count > 1) {
                    addr   -= SCIF_TASK_STRUCT_CTRL_SIZE;
                    length += SCIF_TASK_STRUCT_CTRL_SIZE;
                }

                // Reset the data structure
                memcpy((void*) (AUX_RAM_BASE + addr), ((const uint8_t*) scifData.pAuxRamImage) + addr, length);
            }

        } while (bvStructs);
    }

} // scifResetTaskStructs




/** \brief Returns the number of input/output data buffers available for production/consumption
  *
  * When performing task data exchange with multiple-buffered data structures, the application calls this
  * function to get:
  * - The number of input buffers ready to be produced (\a taskStructType = \ref SCIF_STRUCT_INPUT)
  * - The number of output buffers ready to be consumed (\a taskStructType = \ref SCIF_STRUCT_OUTPUT)
  *
  * The application is only allowed to access the returned number of buffers, or less. The function
  * always returns 0 if a buffer overrun or underrun has occurred but has not yet been reported by
  * \ref scifGetAlertEvents(). For each buffer to be produced or consumed, the application must complete
  * these steps:
  * - Call \ref scifGetTaskStruct() to get a pointer to the data structure
  * - If input, populate the data structure. If output, process the data structure contents and reset
  *   contents manually, as needed.
  * - Call \ref scifHandoffTaskStruct() to give/return the buffer to the Sensor Controller Engine
  *
  * For single-buffered data structures, the function has no effect and always returns 0.
  *
  * \param[in]      taskId
  *     Task ID selection
  * \param[in]      taskStructType
  *     Task data structure type selection
  *
  * \return
  *     The number of buffers that can be produced/consumed by the application
  */
uint32_t scifGetTaskIoStructAvailCount(uint32_t taskId, SCIF_TASK_STRUCT_TYPE_T taskStructType) {

    // Fetch the information about the data structure
    uint32_t taskStructInfo = scifData.pTaskDataStructInfoLut[(taskId * 4) + (int) taskStructType];
    uint16_t baseAddr = (taskStructInfo >> 0)  & 0x0FFF; // 11:0
    uint16_t count    = (taskStructInfo >> 12) & 0x00FF; // 19:12
    uint16_t size     = (taskStructInfo >> 20) & 0x0FFF; // 31:20

    // If single-buffered, it's always 0
    if (count < 2) {
        return 0;
    }

    // Fetch the current memory addresses used by SCE and MCU
    uint16_t sceAddr = *((volatile uint16_t*) (AUX_RAM_BASE + baseAddr - SCIF_TASK_STRUCT_CTRL_SCE_ADDR_BACK_OFFSET));
    uint16_t mcuAddr = *((volatile uint16_t*) (AUX_RAM_BASE + baseAddr - SCIF_TASK_STRUCT_CTRL_MCU_ADDR_BACK_OFFSET));

    // Buffer overflow or underflow can occur in the background if the Sensor Controller produces or
    // consumes data too fast for the System CPU application. If this happens, return 0 so that the
    // application can detect the error by calling scifGetAlertEvents() in the next ALERT interrupt
    // before starting to process potentially corrupted or out-of-sync buffers.
    if (scifData.pIntData->bvTaskIoAlert & (0x0100 << taskId)) {
        return 0;
    }

    // Detect all buffers available
    // LSBs are different when none are available -> handled in the calculation further down
    if (mcuAddr == sceAddr) {
        return count;
    }

    // Calculate the number of buffers available
    mcuAddr &= ~0x0001;
    sceAddr &= ~0x0001;
    if (sceAddr < mcuAddr) {
        sceAddr += size * sizeof(uint16_t) * count;
    }
    return (sceAddr - mcuAddr) / (size * sizeof(uint16_t));

} // scifGetTaskIoStructAvailCount




/** \brief Returns a pointer to the specified data structure
  *
  * This function must be used to access multiple-buffered data structures, in which case it finds the
  * correct data structure buffer to be produced/consumed by the application. The application must use
  * \ref scifGetTaskIoStructAvailCount() to get the number of buffers to produce/consume.
  *
  * This function can also be used for single-buffered data structures, but this is less efficient than
  * accessing these data structures directly.
  *
  * \param[in]      taskId
  *     Task ID selection
  * \param[in]      taskStructType
  *     Task data structure type selection
  *
  * \return
  *     Pointer to the data structure (must be casted to correct pointer type)
  */
void* scifGetTaskStruct(uint32_t taskId, SCIF_TASK_STRUCT_TYPE_T taskStructType) {

    // Fetch the information about the data structure
    uint32_t taskStructInfo = scifData.pTaskDataStructInfoLut[(taskId * 4) + (int) taskStructType];
    uint16_t baseAddr = (taskStructInfo >> 0)  & 0x0FFF; // 11:0
    uint16_t count    = (taskStructInfo >> 12) & 0x00FF; // 19:12

    // If single-buffered, just return the base address
    if (count < 2) {
        return (void*) (AUX_RAM_BASE + baseAddr);

    // If multiple-buffered, return the MCU address
    } else {
        uint16_t mcuAddr = *((uint16_t*) (AUX_RAM_BASE + baseAddr - SCIF_TASK_STRUCT_CTRL_MCU_ADDR_BACK_OFFSET));
        return (void*) (AUX_RAM_BASE + (mcuAddr & ~0x0001));
    }

} // scifGetTaskStruct




/** \brief Called to handoff the an input or output data structure to the Sensor Controller Engine
  *
  * For output, this function shall be called after consuming a buffer of a multiple-buffered data
  * structure. This hands over the output buffer back to the Sensor Controller.
  *
  * For input, this function shall be called after producing a buffer of a multiple-buffered data
  * structure. This hands over the input buffer to the Sensor Controller.
  *
  * \param[in]      taskId
  *     Task ID selection
  * \param[in]      taskStructType
  *     Task data structure type selection
  */
void scifHandoffTaskStruct(uint32_t taskId, SCIF_TASK_STRUCT_TYPE_T taskStructType) {

    // Fetch the information about the data structure
    uint32_t taskStructInfo = scifData.pTaskDataStructInfoLut[(taskId * 4) + (int) taskStructType];
    uint16_t baseAddr = (taskStructInfo >> 0)  & 0x0FFF; // 11:0
    uint16_t count    = (taskStructInfo >> 12) & 0x00FF; // 19:12
    uint16_t size     = (taskStructInfo >> 20) & 0x0FFF; // 31:20

    // If multiple-buffered, move on the MCU address to the next buffer
    if (count >= 2) {

        // Move on the address
        uint16_t* pMcuAddr = (uint16_t*) (AUX_RAM_BASE + baseAddr - SCIF_TASK_STRUCT_CTRL_MCU_ADDR_BACK_OFFSET);
        uint16_t newMcuAddr = *pMcuAddr + size * sizeof(uint16_t);

        // If it has wrapped, move it back to the start and invert LSB
        if ((newMcuAddr & ~0x0001) > (baseAddr + (size * sizeof(uint16_t) * (count - 1)))) {
            newMcuAddr = baseAddr | ((newMcuAddr & 0x0001) ^ 0x0001);
        }

        // Write back the new address
        *pMcuAddr = newMcuAddr;
    }

} // scifHandoffTaskStruct




/** \brief Common function for manually starting, executing and terminating tasks
  *
  * \param[in]      bvTaskIds
  *     Bit-vector of task IDs for the tasks to be controlled
  * \param[in]      bvTaskReq
  *     Any legal combination of the following bits:
  *     - 0x01 : Starts the specified tasks
  *     - 0x02 : Executes the specified tasks once
  *     - 0x04 : Stops the specified tasks
  *
  * \return
  *     \ref SCIF_SUCCESS if successful, otherwise \ref SCIF_NOT_READY (last non-blocking call has not
  *     completed) or SCIF_ILLEGAL_OPERATION (attempted to execute an already active task). The function
  *     call has no effect if unsuccessful.
  */
static SCIF_RESULT_T scifCtrlTasksNbl(uint32_t bvTaskIds, uint32_t bvTaskReq) {

    // Prevent interruptions by concurrent scifCtrlTasksNbl() calls
    if (!osalLockCtrlTaskNbl()) {
        return SCIF_NOT_READY;
    }

    // Perform sanity checks: Starting already active or dirty tasks is illegal
    if (bvTaskReq & 0x01) {
        if ((scifData.pTaskCtrl->bvActiveTasks | scifData.bvDirtyTasks) & bvTaskIds) {
            osalUnlockCtrlTaskNbl();
            return SCIF_ILLEGAL_OPERATION;
        }
    }

    // Verify that the control interface is ready, then clear the READY interrupt source
    if (HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGS) & AUX_EVCTL_EVTOAONFLAGS_SWEV0_M) {
        HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGSCLR) = AUX_EVCTL_EVTOAONFLAGS_SWEV0_M;
        while (HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGS) & AUX_EVCTL_EVTOAONFLAGS_SWEV0_M);
    } else {
        osalUnlockCtrlTaskNbl();
        return SCIF_NOT_READY;
    }

    // Initialize tasks?
    if (bvTaskReq & 0x01) {
        scifData.pTaskCtrl->bvTaskInitializeReq = bvTaskIds;
        scifData.bvDirtyTasks |= bvTaskIds;
    } else {
        scifData.pTaskCtrl->bvTaskInitializeReq = 0x0000;
    }

    // Execute tasks?
    if (bvTaskReq & 0x02) {
        scifData.pTaskCtrl->bvTaskExecuteReq = bvTaskIds;
    } else {
        scifData.pTaskCtrl->bvTaskExecuteReq = 0x0000;
    }

    // Terminate tasks? Terminating already inactive tasks is allowed, because tasks may stop
    // spontaneously, and there's no way to know this for sure (it may for instance happen at any moment
    // while calling this function)
    if (bvTaskReq & 0x04) {
        scifData.pTaskCtrl->bvTaskTerminateReq = bvTaskIds;
    } else {
        scifData.pTaskCtrl->bvTaskTerminateReq = 0x0000;
    }

    // Make sure that the CPU interrupt has been cleared before reenabling it
    osalClearCtrlReadyInt();
    osalEnableCtrlReadyInt();

    // Set the REQ event to hand over the request to the Sensor Controller
    HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_SWWUTRIG) = AUX_SYSIF_SWWUTRIG_SW_WU0_M;
    osalUnlockCtrlTaskNbl();
    return SCIF_SUCCESS;

} // scifCtrlTasksNbl




/** \brief Executes the specified tasks once, from an inactive state
  *
  * This triggers the Initialization, Execution and Termination code for each task ID specified in
  * \a bvTaskIds. All selected code is completed for one task before proceeding with the next task. The
  * control READY event is generated when the tasks have been executed.
  *
  * This function should not be used to execute a task that implements the event handler code, because
  * the execution method does not allow for running the Event Handler code.
  *
  * \note This function must not be called for already active tasks.
  *
  * \note Task control does not interrupt ongoing code execution on the Sensor Controller, but it has
  *       priority over other triggers/wake-up sources. Calling this function can therefore delay
  *       upcoming Sensor Controller activities such as RTC-based task execution and Event Handler code
  *       execution.
  *
  * \param[in]      bvTaskIds
  *     Bit-vector indicating which tasks should be executed (where bit N corresponds to task ID N)
  *
  * \return
  *     \ref SCIF_SUCCESS if successful, otherwise \ref SCIF_NOT_READY (last non-blocking call has not
  *     completed) or \ref SCIF_ILLEGAL_OPERATION (attempted to execute an already active task). The
  *     function call has no effect if unsuccessful.
  */
SCIF_RESULT_T scifExecuteTasksOnceNbl(uint32_t bvTaskIds) {
    return scifCtrlTasksNbl(bvTaskIds, 0x07);
} // scifExecuteTasksOnceNbl




/** \brief Starts the specified tasks
  *
  * This triggers the initialization code for each task ID specified in \a bvTaskIds. The READY event
  * is generated when the tasks have been started.
  *
  * \note This function must not be called for already active tasks.
  *
  * \note Task control does not interrupt ongoing code execution on the Sensor Controller, but it has
  *       priority over other triggers/wake-up sources. Calling this function can therefore delay
  *       upcoming Sensor Controller activities such as RTC-based task execution and Event Handler code
  *       execution.
  *
  * \param[in]      bvTaskIds
  *     Bit-vector indicating which tasks to be started (where bit N corresponds to task ID N)
  *
  * \return
  *     \ref SCIF_SUCCESS if successful, otherwise \ref SCIF_NOT_READY (last non-blocking call has not
  *     completed) or \ref SCIF_ILLEGAL_OPERATION (attempted to start an already active task). The
  *     function call has no effect if unsuccessful.
  */
SCIF_RESULT_T scifStartTasksNbl(uint32_t bvTaskIds) {
    return scifCtrlTasksNbl(bvTaskIds, 0x01);
} // scifStartTasksNbl




/** \brief Stops the specified tasks
  *
  * This triggers the termination code for each task ID specified in \a bvTaskIds. The READY event is
  * generated when the tasks have been stopped.
  *
  * \note Task control does not interrupt ongoing code execution on the Sensor Controller, but it has
  *       priority over other triggers/wake-up sources. Calling this function can therefore delay
  *       upcoming Sensor Controller activities such as RTC-based task execution and Event Handler code
  *       execution.
  *
  * \param[in]      bvTaskIds
  *     Bit-vector indicating which tasks to be stopped (where bit N corresponds to task ID N)
  *
  * \return
  *     \ref SCIF_SUCCESS if successful, otherwise \ref SCIF_NOT_READY (last non-blocking call has not
  *     completed). The function call has no effect if unsuccessful.
  */
SCIF_RESULT_T scifStopTasksNbl(uint32_t bvTaskIds) {
    return scifCtrlTasksNbl(bvTaskIds, 0x04);
} // scifStopTasksNbl




/** \brief Triggers manually the Execution code blocks for the specified tasks
  *
  * This triggers the Execution code for each task ID specified in \a bvTaskIds. The READY event
  * is generated when the Execution code has run for all the specified tasks.
  *
  * Calling this function does not interrupt any ongoing activities on the Sensor Controller, and does
  * not affect RTC-based task execution.
  *
  * \note This function should only be called for already active tasks.
  *
  * \note Task control does not interrupt ongoing code execution on the Sensor Controller, but it has
  *       priority over other triggers/wake-up sources. Calling this function can therefore delay
  *       upcoming Sensor Controller activities such as RTC-based task execution and Event Handler code
  *       execution.
  *
  * \param[in]      bvTaskIds
  *     Bit-vector indicating which tasks should be executed (where bit N corresponds to task ID N)
  *
  * \return
  *     \ref SCIF_SUCCESS if successful, otherwise \ref SCIF_NOT_READY (last non-blocking call has not
  *     completed) or \ref SCIF_ILLEGAL_OPERATION (attempted to execute an already active task). The
  *     function call has no effect if unsuccessful.
  */
SCIF_RESULT_T scifSwTriggerExecutionCodeNbl(uint32_t bvTaskIds) {
    return scifCtrlTasksNbl(bvTaskIds, 0x02);
} // scifSwTriggerExecutionCodeNbl




/** \brief Triggers manually the event handler code block
  *
  * This function forces execution of the specified Event Handler code, by trigger index, for the
  * specified task.
  *
  * Calling this function does not interrupt any ongoing activities on the Sensor Controller. It does
  * however cancel any previous trigger setup by a \c evhSetupCompbTrigger(), \c evhSetupGpioTrigger()
  * or \c evhSetupCompbTrigger() procedure call in task code.
  *
  * \note This function should only be called when the task using the event handler code is already
  *       active.
  *
  * \param[in]      taskId
  *     Task ID selection
  * \param[in]      triggerIndex
  *     Event trigger index (as used in the Sensor Controller task code to setup event triggers)
  *
  * \return
  *     \ref SCIF_SUCCESS if successful or \ref SCIF_NOT_READY (last call to this function has
  *     not completed).
  */
SCIF_RESULT_T scifSwTriggerEventHandlerCode(uint32_t taskId, uint32_t triggerIndex) {

    // Calculate the PROGWUnCFG register address
    int triggerBase = (scifData.taskBaseEvTrigIndexLut >> (2 * taskId)) & 0x3;
    uint16_t progwucfgManTrigReg = (uint16_t) ((AUX_SYSIF_BASE + AUX_SYSIF_O_PROGWU1CFG) +
                                               ((triggerBase + triggerIndex) * sizeof(uint32_t)));

    // Generate a trigger unless one is already pending
    uint32_t key = scifOsalEnterCriticalSection();
    SCIF_RESULT_T result;
    if (scifData.pIntData->progwucfgManTrigReg == 0x0000) {
        scifData.pIntData->progwucfgManTrigReg = progwucfgManTrigReg;
        HWREG(AUX_SYSIF_BASE + AUX_SYSIF_O_SWWUTRIG) = AUX_SYSIF_SWWUTRIG_SW_WU1_M;
        result = SCIF_SUCCESS;
    } else {
        result = SCIF_NOT_READY;
    }
    scifOsalLeaveCriticalSection(key);
    return result;

} // scifSwTriggerEventHandlerCode




/** \brief Waits for a non-blocking call to complete, with timeout
  *
  * The non-blocking task control functions, \ref scifExecuteTasksOnceNbl(), \ref scifStartTasksNbl()
  * and \ref scifStopTasksNbl(), may take some time to complete. This wait function can be used to make
  * blocking calls, and allow an operating system to switch context when until the task control interface
  * becomes ready again.
  *
  * The function returns when the last non-blocking call has completed, or immediately if already
  * completed. The function can also return immediately with the \ref SCIF_ILLEGAL_OPERATION error if
  * called from multiple threads of execution with non-zero \a timeoutUs.
  *
  * \b Important: Unlike the ALERT event, the READY event does not generate MCU domain and System CPU
  * wake-up. Depending on the SCIF OSAL implementation, this function might not return before the
  * specified timeout expires, even if the READY event has occurred long before that. To avoid such
  * delays, call \c fwGenAlertInterrupt() from the task code block that \ref scifWaitOnNbl() is waiting
  * for to complete.
  *
  * \param[in]      timeoutUs
  *     Maximum number of microseconds to wait for the non-blocking functions to become available. Use a
  *     timeout of "0" to check whether the interface already is available, or simply call the control
  *     function (which also will return \ref SCIF_NOT_READY if not ready).
  *
  * \return
  *     \ref SCIF_SUCCESS if the last call has completed, otherwise \ref SCIF_NOT_READY (the timeout
  *     expired) or \ref SCIF_ILLEGAL_OPERATION (the OSAL does not allow this function to be called with
  *     non-zero \a timeoutUs from multiple threads of execution).
  */
SCIF_RESULT_T scifWaitOnNbl(uint32_t timeoutUs) {
    if (HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGS) & AUX_EVCTL_EVTOAONFLAGS_SWEV0_M) {
        return SCIF_SUCCESS;
    } else {
        return osalWaitOnCtrlReady(timeoutUs);
    }
} // scifWaitOnNbl




/** \brief Returns a bit-vector indicating which tasks are currently active
  *
  * The bit-vector is maintained by the Sensor Controller. If called before the last non-blocking control
  * operation has completed, the bit-vector may indicate the task states as before or after the non-
  * blocking operations (the bit vector is updated only once per non-blocking call).
  *
  * \return
  *     A bit-vector indicating which tasks are active (bit N corresponds to task N)
  */
uint16_t scifGetActiveTaskIds(void) {
    return scifData.pTaskCtrl->bvActiveTasks;
} // scifGetActiveTaskIds


//@}


// Generated by SANWINA0221118 at 2019-06-13 16:12:30.899
