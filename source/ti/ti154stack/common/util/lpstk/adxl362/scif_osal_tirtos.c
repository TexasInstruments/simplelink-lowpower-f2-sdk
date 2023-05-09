/// \addtogroup module_scif_osal
//@{
#ifdef SCIF_INCLUDE_OSAL_C_FILE

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_nvic.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include "scif_osal_tirtos.h"
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>



/// MCU wakeup source to be used with the Sensor Controller control READY event, must not conflict with OS
#define OSAL_CTRL_READY_MCUWUSEL_WU_EV_INDEX    6
/// MCU wakeup source to be used with the Sensor Controller task ALERT event, must not conflict with OS
#define OSAL_TASK_ALERT_MCUWUSEL_WU_EV_INDEX    7


/// The READY interrupt is implemented using INT_AON_AUX_SWEV0
#define INT_SCIF_CTRL_READY     INT_AON_AUX_SWEV0
/// The ALERT interrupt is implemented using INT_AON_AUX_SWEV1
#define INT_SCIF_TASK_ALERT     INT_AON_AUX_SWEV1


/// Calculates the NVIC register offset for the specified interrupt
#define NVIC_OFFSET(i)          (((i) - 16) / 32)
/// Calculates the bit-vector to be written or compared against for the specified interrupt
#define NVIC_BV(i)              (1 << ((i - 16) % 32))


// Function prototypes
static void osalCtrlReadyIsr(UArg arg);
static void osalTaskAlertIsr(UArg arg);


/// HWI object for the control READY interrupt
static Hwi_Struct hwiCtrlReady;
/// HWI object for the task ALERT interrupt
static Hwi_Struct hwiTaskAlert;


/// Semaphore for control READY waiting
static Semaphore_Struct semCtrlReady;

/// Has the \ref scifOsalInit() function been called?
static volatile bool osalInitDone = false;




/** \brief Registers the control READY interrupt
  *
  * This registers the \ref osalCtrlReadyIsr() function with the \ref INT_SCIF_CTRL_READY interrupt
  * vector.
  *
  * The interrupt occurs at initial startup, and then after each control request has been serviced by the
  * Sensor Controller. The interrupt is pending (including source event flag) and disabled while the task
  * control interface is idle.
  */
static void osalRegisterCtrlReadyInt(void) {
    Hwi_Params hwiParams;
    Hwi_Params_init(&hwiParams);
    // Do not enable interrupt yet
    hwiParams.enableInt = false;
    // Create the HWI object for the control READY interrupt
    Hwi_construct(&hwiCtrlReady, INT_SCIF_CTRL_READY, osalCtrlReadyIsr, &hwiParams, NULL);
} // osalRegisterCtrlReadyInt




/** \brief Unregisters the control READY interrupt
  *
  * This detaches the \ref osalCtrlReadyIsr() function from the \ref INT_SCIF_CTRL_READY interrupt
  * vector.
  */
static void osalUnregisterCtrlReadyInt(void) {
    // Destroy the HWI object
    Hwi_destruct(&hwiCtrlReady);
} // osalUnregisterCtrlReadyInt




/** \brief Enables the control READY interrupt
  *
  * This function is called when sending a control REQ event to the Sensor Controller to enable the READY
  * interrupt. This is done after clearing the event source and then the READY interrupt, using
  * \ref osalClearCtrlReadyInt().
  */
static void osalEnableCtrlReadyInt(void) {
    Hwi_enableInterrupt(INT_SCIF_CTRL_READY);
} // osalEnableCtrlReadyInt




/** \brief Disables the control READY interrupt
  *
  * This function is called when by the control READY ISR, \ref osalCtrlReadyIsr(), so that the interrupt
  * is disabled but still pending (including source event flag) while the task control interface is
  * idle/ready.
  */
static void osalDisableCtrlReadyInt(void) {
    Hwi_disableInterrupt(INT_SCIF_CTRL_READY);
} // osalDisableCtrlReadyInt




/** \brief Clears the task control READY interrupt
  *
  * This is done when sending a control request, after clearing the READY source event.
  */
static void osalClearCtrlReadyInt(void) {
    Hwi_clearInterrupt(INT_SCIF_CTRL_READY);
} // osalClearCtrlReadyInt




/** \brief Registers the task ALERT interrupt
  *
  * This registers the \ref osalTaskAlertIsr() function with the \ref INT_SCIF_TASK_ALERT interrupt
  * vector.
  *
  * The interrupt occurs whenever a sensor controller task alerts the driver, to request data exchange,
  * indicate an error condition or that the task has stopped spontaneously.
  */
static void osalRegisterTaskAlertInt(void) {
    Hwi_Params hwiParams;
    Hwi_Params_init(&hwiParams);
    // Do not enable interrupt yet
    hwiParams.enableInt = false;
    // Create HWI object for the task ALERT interrupt
    Hwi_construct(&hwiTaskAlert, INT_SCIF_TASK_ALERT, osalTaskAlertIsr, &hwiParams, NULL);
} // osalRegisterTaskAlertInt




/** \brief Unregisters the task ALERT interrupt
  *
  * This detaches the \ref osalTaskAlertIsr() function from the \ref INT_SCIF_TASK_ALERT interrupt
  * vector.
  */
static void osalUnregisterTaskAlertInt(void) {
    // Destroy the HWI object
    Hwi_destruct(&hwiTaskAlert);
} // osalUnregisterTaskAlertInt




/** \brief Enables the task ALERT interrupt
  *
  * The interrupt is enabled at startup. It is disabled upon reception of a task ALERT interrupt and re-
  * enabled when the task ALERT is acknowledged.
  */
void scifOsalEnableTaskAlertInt(void) {
    Hwi_enableInterrupt(INT_SCIF_TASK_ALERT);
} // scifOsalEnableTaskAlertInt




/** \brief Disables the task ALERT interrupt
  *
  * The interrupt is enabled at startup. It is disabled upon reception of a task ALERT interrupt and re-
  * enabled when the task ALERT is acknowledged.
  *
  * Note that there can be increased current consumption in System CPU standby mode if the ALERT
  * interrupt is disabled, but wake-up is enabled (see \ref scifSetWakeOnAlertInt()). This is because the
  * wake-up signal will remain asserted until \ref scifAckAlertEvents() has been called for all pending
  * ALERT events.
  */
void scifOsalDisableTaskAlertInt(void) {
    Hwi_disableInterrupt(INT_SCIF_TASK_ALERT);
} // scifOsalDisableTaskAlertInt




/** \brief Clears the task ALERT interrupt
  *
  * This is done when acknowledging the alert, after clearing the ALERT source event.
  */
static void osalClearTaskAlertInt(void) {
    Hwi_clearInterrupt(INT_SCIF_TASK_ALERT);
} // osalClearTaskAlertInt




/** \brief Enters a critical section by disabling hardware interrupts
  *
  * \return
  *     Whether interrupts were enabled at the time this function was called
  */
uint32_t scifOsalEnterCriticalSection(void) {
    return Hwi_disable();
} // scifOsalEnterCriticalSection




/** \brief Leaves a critical section by reenabling hardware interrupts if previously enabled
  *
  * \param[in]      key
  *     The value returned by the previous corresponding call to \ref scifOsalEnterCriticalSection()
  */
void scifOsalLeaveCriticalSection(uint32_t key) {
    Hwi_restore(key);
} // scifOsalLeaveCriticalSection




/// Stores whether task control non-blocking functions have been locked
static volatile bool osalCtrlTaskNblLocked = false;




/** \brief Locks use of task control non-blocking functions
  *
  * This function is used by the non-blocking task control to allow safe operation from multiple threads.
  *
  * The function shall attempt to set the \ref osalCtrlTaskNblLocked flag in a critical section.
  * Implementing a timeout is optional (the task control's non-blocking behavior is not associated with
  * this critical section, but rather with completion of the task control request).
  *
  * \return
  *     Whether the critical section could be entered (true if entered, false otherwise)
  */
static bool osalLockCtrlTaskNbl(void) {
    uint32_t key = Hwi_disable();
    if (osalCtrlTaskNblLocked) {
        Hwi_restore(key);
        return false;
    } else {
        osalCtrlTaskNblLocked = true;
        Hwi_restore(key);
        return true;
    }
} // osalLockCtrlTaskNbl




/** \brief Unlocks use of task control non-blocking functions
  *
  * This function will be called once after a successful \ref osalLockCtrlTaskNbl().
  */
static void osalUnlockCtrlTaskNbl(void) {
    osalCtrlTaskNblLocked = false;
} // osalUnlockCtrlTaskNbl




/// Stores whether \ref semCtrlReady is being pended on in osalWaitOnCtrlReady()
static volatile bool osalWaitOnNblLocked = false;




/** \brief Waits until the task control interface is ready/idle
  *
  * This indicates that the task control interface is ready for the first request or that the last
  * request has been completed.
  *
  * \param[in]      timeoutUs
  *     Minimum timeout, in microseconds
  *
  * \return
  *     \ref SCIF_SUCCESS if the last call has completed, otherwise \ref SCIF_NOT_READY (the timeout
  *     expired) or \ref SCIF_ILLEGAL_OPERATION (the OSAL does not allow this function to be called with
  *     non-zero \a timeoutUs from multiple threads of execution).
  */
static SCIF_RESULT_T osalWaitOnCtrlReady(uint32_t timeoutUs) {
    SCIF_RESULT_T result;

    // If ready now ...
    uint32_t key = Hwi_disable();
    if (HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOAONFLAGS) & AUX_EVCTL_EVTOAONFLAGS_SWEV0_M) {

        // Immediate success
        Hwi_restore(key);
        result = SCIF_SUCCESS;

    // If no timeout has been specified ...
    } else if (timeoutUs == 0) {

        // Immediate failure
        Hwi_restore(key);
        result = SCIF_NOT_READY;

    // If another osalWaitOnCtrlReady() call is in progress ...
    } else if (osalWaitOnNblLocked) {

        // This call to osalWaitOnCtrlReady() has interrupted another call to osalWaitOnCtrlReady()
        Hwi_restore(key);
        result = SCIF_ILLEGAL_OPERATION;

    // Otherwise ...
    } else {

        // The control READY interrupt has not yet occurred, so it is safe to reset the semaphore and
        // pend on it because we know the interrupt will happen.
        Semaphore_reset(Semaphore_handle(&semCtrlReady), 0);
        osalWaitOnNblLocked = true;
        Hwi_restore(key);

        // Enable wake-up on READY interrupt
        scifSetMcuwusel(OSAL_CTRL_READY_MCUWUSEL_WU_EV_INDEX, AON_EVENT_MCUWUSEL_WU0_EV_AUX_SWEV0);

        // Return whether the semaphore was released within the timeout
        if (Semaphore_pend(Semaphore_handle(&semCtrlReady), timeoutUs / Clock_tickPeriod)) {
            osalWaitOnNblLocked = false;
            result = SCIF_SUCCESS;
        } else {
            osalWaitOnNblLocked = false;
            result = SCIF_NOT_READY;
        }

        // Disable wake-up on READY interrupt
        scifSetMcuwusel(OSAL_CTRL_READY_MCUWUSEL_WU_EV_INDEX, AON_EVENT_MCUWUSEL_WU0_EV_NONE);
    }

    return result;

} // osalWaitOnCtrlReady




/// OSAL "TI-RTOS": Application-registered callback function for the task control READY interrupt
static SCIF_VFPTR osalIndicateCtrlReadyCallback = NULL;
/// OSAL "TI-RTOS": Application-registered callback function for the task ALERT interrupt
static SCIF_VFPTR osalIndicateTaskAlertCallback = NULL;




/** \brief Called by \ref osalCtrlReadyIsr() to notify the application
  *
  * This shall trigger a callback, generate a message/event etc.
  */
static void osalIndicateCtrlReady(void) {

    // If the OSAL has been initialized, release the sempahore
    if (osalInitDone) {
        Semaphore_post(Semaphore_handle(&semCtrlReady));
    }

    // Call callback function
    if (osalIndicateCtrlReadyCallback) {
        osalIndicateCtrlReadyCallback();
    }

} // osalIndicateCtrlReady




/** \brief Called by \ref osalTaskAlertIsr() to notify the application
  *
  * This shall trigger a callback, generate a message/event etc.
  */
static void osalIndicateTaskAlert(void) {
    if (osalIndicateTaskAlertCallback) {
        osalIndicateTaskAlertCallback();
    }
} // osalIndicateTaskAlert




/** \brief Sensor Controller READY interrupt service routine
  *
  * The ISR simply disables the interrupt and notifies the application.
  *
  * The interrupt flag is cleared and reenabled when sending the next task control request (by calling
  * \ref scifExecuteTasksOnceNbl(), \ref scifStartTasksNbl() or \ref scifStopTasksNbl()).
  *
  * \param[in]      arg
  *     Unused
  */
static void osalCtrlReadyIsr(UArg arg) {
    osalDisableCtrlReadyInt();
    osalIndicateCtrlReady();
} // osalCtrlReadyIsr




/** \brief Sensor Controller ALERT interrupt service routine
  *
  * The ISR disables further interrupt generation and notifies the application. To clear the interrupt
  * source, the application must call \ref scifClearAlertIntSource. The CPU interrupt flag is cleared and
  * the interrupt is reenabled when calling \ref scifAckAlertEvents() to generate ACK.
  *
  * \param[in]      arg
  *     Unused
  */
static void osalTaskAlertIsr(UArg arg) {
    scifOsalDisableTaskAlertInt();
    osalIndicateTaskAlert();
} // osalTaskAlertIsr




/** \brief OSAL "TI-RTOS": Registers the task control READY interrupt callback
  *
  * Using this callback is normally optional. See \ref osalIndicateCtrlReady() for details.
  *
  * \param[in]      callback
  *     Callback function pointer "void func(void)"
  */
void scifOsalRegisterCtrlReadyCallback(SCIF_VFPTR callback) {
    osalIndicateCtrlReadyCallback = callback;
} // scifOsalRegisterCtrlReadyCallback




/** \brief OSAL "TI-RTOS": Registers the task ALERT interrupt callback
  *
  * Using this callback is normally required. See \ref osalIndicateTaskAlert() for details.
  *
  * \param[in]      callback
  *     Callback function pointer "void func(void)"
  */
void scifOsalRegisterTaskAlertCallback(SCIF_VFPTR callback) {
    osalIndicateTaskAlertCallback = callback;
} // scifOsalRegisterTaskAlertCallback




/** \brief OSAL "TI-RTOS": Initializes the OSAL
  *
  * This creates a binary semaphore used to wait on the task control interface.
  *
  * This function must be called once at startup before using the task control functions:
  * - \ref scifStartTasksNbl()
  * - \ref scifStopTasksNbl()
  * - \ref scifWaitOnNbl()
  */
void scifOsalInit(void) {

    // If the OSAL has not yet been initialized ...
    if (!osalInitDone) {
        osalInitDone = true;

        // Create a binary semaphore, initially blocked.
        Semaphore_Params semParams;
        Semaphore_Params_init(&semParams);
        semParams.mode = Semaphore_Mode_BINARY;
        Semaphore_construct(&semCtrlReady, 0, &semParams);
    }

} // scifOsalInit


#endif
//@}


// Generated by SANWINA0221118 at 2019-06-13 16:12:30.899
