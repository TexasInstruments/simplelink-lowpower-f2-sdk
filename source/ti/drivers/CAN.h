/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*!*****************************************************************************
 *  @file       CAN.h
 *
 *  @brief      Controller Area Network (CAN) Driver Interface
 *
 *  @anchor ti_drivers_CAN_Overview
 *  # Overview
 *  The Controller Area Network (CAN) driver is a single instance driver
 *  that provides a simple interface to transmit and receive messages on a CAN bus.
 *  Messages are broadcast to the entire CAN network and each device is responsible
 *  for filtering and handling the received messages as necessary.
 *  The application is responsible for interpreting the received data.
 *
 *  <hr>
 *  @anchor ti_drivers_CAN_Usage
 *  # Usage
 *
 *  To use the CAN driver to send and receive messages over the CAN bus, the
 *  application calls the following APIs:
 *    - CAN_open(): Open the CAN driver instance and configure the CAN
 *      controller, placing it in normal operational mode.
 *    - CAN_write(): Transmit a message using the Tx FIFO/Queue. This is the
 *      typical method of transmission.
 *    - CAN_writeBuffer(): Transmit a message using a dedicated Tx Buffer. This
 *      method of transmission requires a custom message RAM configuration and
 *      should only be used if there is an application-specific need that cannot
 *      be met by using the Tx FIFO/Queue.
 *    - CAN_read(): Receive a message. This should be called in a task context
 *      and triggered by the event callback when CAN_EVENT_RX_DATA_AVAIL occurs.
 *    - CAN_close(): Close the CAN driver instance and reset the CAN controller,
 *      placing it in standby operational mode.
 *
 *  @anchor ti_drivers_CAN_Synopsis
 *  ## Synopsis
 *  The following code example initializes the CAN driver with the default
 *  configuration, transmits a CAN FD message, and waits to read any received
 *  messages.
 *
 *  @code
 *  // Payload data size indexed by Data Length Code (DLC) field.
 *  static const uint32_t DLCtoDataSize[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
 *
 *  // Rx semaphore.
 *  static SemaphoreP_Handle rxSemHandle;
 *
 *  void eventCallback(CAN_Handle handle, uint32_t event, uint32_t data, void *userArg)
 *  {
 *      if (event == CAN_EVENT_RX_DATA_AVAIL)
 *      {
 *          SemaphoreP_post(rxSemHandle);
 *      }
 *      // Handle more events here if enabled via the event mask...
 *  }
 *
 *  void thread(arg0, arg1)
 *  {
 *     int_fast16_t status;
 *     CAN_RxBufElement rxElem;
 *     CAN_TxBufElement txElem;
 *
 *     // Initialize driver(s).
 *     CAN_init();
 *
 *     // Create callback semaphore.
 *     SemaphoreP_Params semParams;
 *     SemaphoreP_Params_init(&semParams);
 *     semParams.mode    = SemaphoreP_Mode_BINARY;
 *     callbackSemHandle = SemaphoreP_create(0, &(semParams));
 *
 *     if (callbackSemHandle == NULL)
 *     {
 *         // SemaphoreP_create() failed.
 *         while (1) {}
 *     }
 *
 *     // Open CAN driver with default configuration.
 *     CAN_Params_init(&canParams);
 *     canParams.eventCbk  = eventCallback;
 *     // Setup event mask for events the application is interested in receiving
 *     // the callback for. Typically, only the CAN_EVENT_RX_DATA_AVAIL is required.
 *     canParams.eventMask = CAN_EVENT_RX_DATA_AVAIL;
 *
 *     canHandle = CAN_open(CONFIG_CAN_0, &canParams);
 *     if (canHandle == NULL)
 *     {
 *         // CAN_open() failed.
 *         while (1) {}
 *     }
 *
 *     // Setup Tx buffer element:
 *     //   CAN FD without Bit Rate Switching
 *     //   Extended Message ID = 0x12345678
 *     //   Data Length of 64-bytes
 *     //   Message marker = 5
 *     txElem.id  = 0x12345678U;
 *     txElem.rtr = 0U;
 *     txElem.xtd = 1U;
 *     txElem.esi = 0U;
 *     txElem.brs = 1U;
 *     txElem.dlc = CAN_DLC_64B;
 *     txElem.fdf = 1U;
 *     txElem.efc = 0U;
 *     txElem.mm  = 5U;
 *
 *     // Fill data payload with incrementing values.
 *     for (i = 0; i < DLCtoDataSize[txElem.dlc]; i++)
 *     {
 *         txElem.data[i] = i;
 *     }
 *
 *     // Transmit message.
 *     CAN_write(canHandle, &txElem);
 *
 *     while (1)
 *     {
 *         // Wait for Rx data available event.
 *         SemaphoreP_pend(rxSemHandle, (uint32_t)SemaphoreP_WAIT_FOREVER);
 *
 *         // Read all available messages.
 *         while (CAN_read(canHandle, &rxElem) == CAN_STATUS_SUCCESS)
 *         {
 *             // Process received message.
 *         }
 *     }
 *  }
 *
 *  @endcode
 *
 *  More details on usage are provided in the following subsections.
 *
 *  @anchor ti_drivers_CAN_Examples
 *  ## Examples #
 *  * @ref ti_drivers_CAN_Synopsis "Usage Synopsis"
 *  * @ref ti_drivers_CAN_Example_initMsgRAM "Initialize with custom message RAM configuration"
 *  * @ref ti_drivers_CAN_Example_initRawBitRate "Initialize with raw bit rate timing"
 *
 *  ## Initializing the CAN Driver
 *
 *  CAN_init() must be called before any other CAN APIs.  This function
 *  initializes common driver resources and calls the device-specific
 *  initialization function to configure the bit rate and message RAM.
 *
 *  ## Opening the CAN Driver
 *  After initializing the CAN driver by calling CAN_init(), the application
 *  can open a CAN instance by calling CAN_open().  This function
 *  takes an index into the @p CAN_config[] array, and a CAN parameters data
 *  structure. The CAN instance is specified by the index of the CAN in
 *  @p CAN_config[]. Calling CAN_open() a second time with the same index
 *  previously passed to CAN_open() will result in an error.  You can,
 *  though, re-use the index if the instance is closed via CAN_close().
 *
 *  If no #CAN_Params structure is passed to CAN_open(), default values are
 *  used. If the open call is successful, it returns a non-NULL value.
 *  The CAN driver APIs are non-blocking; there is no configurable return behavior.
 *
 *  @anchor ti_drivers_CAN_Example_initMsgRAM
 *  Example initializing the CAN driver with a custom message RAM configuration
 *  to receive only filtered message IDs:
 *
 *  @note CAN driver SysConfig must be setup with 'Reject Non-Matching Messages' enabled.
 *
 *  @code
 *  #define STD_MSG_FILTER_NUM 2U
 *  #define EXT_MSG_FILTER_NUM 1U
 *
 *  static MCAN_StdMsgIDFilterElement stdMsgIDFilter[STD_MSG_FILTER_NUM] =
 *      {{.sfid1 = 0x555, .sfid2 = 0x444, .sfec = CAN_FEC_STORE_RXFIFO0, .sft = CAN_FILTER_DUAL_ID},
 *       {.sfid1 = 0x123, .sfid2 = 0U, .sfec = CAN_FEC_STORE_RXBUF, .sft = 0U}};
 *
 *  static MCAN_ExtMsgIDFilterElement extMsgIDFilter[EXT_MSG_FILTER_NUM] =
 *      {{.efid1 = 0x1234578, .efid2 = 0x1234600, .efec = CAN_FEC_STORE_RXFIFO1, .eft = CAN_FILTER_RANGE}};
 *
 *  const CAN_MsgRAMConfig msgRAMConfig = {
 *      .stdFilterNum       = STD_MSG_FILTER_NUM,
 *      .extFilterNum       = EXT_MSG_FILTER_NUM,
 *      .stdMsgIDFilterList = &stdMsgIDFilter[0],
 *      .extMsgIDFilterList = &extMsgIDFilter[0],
 *
 *      .rxFIFONum[0] = 10U,
 *      .rxFIFONum[1] = 2U,
 *      .rxBufNum     = 1U,
 *      .txBufNum     = 1U,
 *      .txFIFOQNum   = 5U,
 *      .txFIFOQMode  = 1U,
 *  };
 *
 *  int_fast16_t status;
 *
 *  // Initialize driver(s).
 *  CAN_init();
 *
 *  // Open CAN driver with custom message filters.
 *  CAN_Params_init(&canParams);
 *  canParams.msgRAMConfig = &msgRAMConfig;
 *  canParams.eventCbk  = eventCallback;
 *  // Setup event mask for events the application is interested in receiving
 *  // the callback for. Typically, only the CAN_EVENT_RX_DATA_AVAIL is required.
 *  canParams.eventMask = CAN_EVENT_RX_DATA_AVAIL;
 *
 *  canHandle = CAN_open(CONFIG_CAN_0, &canParams);
 *  if (canHandle == NULL)
 *  {
 *      // CAN_open() failed.
 *      while (1) {}
 *  }
 *  @endcode
 *
 *  @anchor ti_drivers_CAN_Example_initRawBitRate
 *  Example initializing the CAN driver with a specific raw bit rate timing:
 *  @note For this example, CAN driver SysConfig should be setup with 'CAN FD
 *  Operation' and 'Bit Rate Switching' enabled. The nominal and data bit rates
 *  selected in SysConfig will be ignored since raw bit rate timing parameters
 *  are provided to CAN_open().
 *
 *  @code
 *  const CAN_DataBitRateTimingRaw rawDataBitRateTiming = {
 *      // 1Mbps with 40MHz clk and 80% sample point ((40E6 / 2) / (15 + 4 + 1) = 1E6)
 *      // Add 1 to each programmed bit time to get functional value and +1 for for prop segment
 *      .dbrp            = 1U,
 *      .dtSeg1          = 14U,
 *      .dtSeg2          = 3U,
 *      .dsjw            = 3U,
 *      .tdcOffset       = 1U,
 *      .tdcFilterWinLen = 2U
 *  };
 *
 *  const CAN_BitRateTimingRaw rawBitTiming = {
 *      // 500kbps nominal with 40MHz clk and 87.5% sample point ((40E6 / 1) / (70 + 9 + 1) = 500E3)
 *      // Add 1 to each programmed bit time to get functional value and +1 for for prop segment
 *      .nbrp       = 0U,
 *      .ntSeg1     = 69U,
 *      .ntSeg2     = 8U,
 *      .nsjw       = 8U,
 *      .dataTiming = &rawDataBitRateTiming
 *  };
 *
 *  int_fast16_t status;
 *
 *  // Initialize driver(s).
 *  CAN_init();
 *
 *  // Open CAN with specific raw bit timing.
 *  CAN_Params_init(&canParams);
 *  canParams.bitTiming = &rawBitTiming;
 *  canParams.eventCbk  = eventCallback;
 *  // Setup event mask for events the application is interested in receiving
 *  // the callback for. Typically, only the CAN_EVENT_RX_DATA_AVAIL is required.
 *  canParams.eventMask = CAN_EVENT_RX_DATA_AVAIL;
 *
 *  canHandle = CAN_open(CONFIG_CAN_0, &canParams);
 *  if (canHandle == NULL)
 *  {
 *      // CAN_open() failed.
 *      while (1) {}
 *  }
 *  @endcode
 *
 *  ## CAN Message RAM Configuration
 *
 *  The default message RAM configuration is as follows:
 *  - No Rx filters.
 *  - Tx Queue (message with lowest ID in the queue will be transmitted first)
 *    with a fixed device-specific number of Tx buffers.
 *  - No Tx event FIFO.
 *  - Fixed device-specific number of Rx FIFO0 buffers.
 *  - No Rx FIFO1 buffers.
 *  - No dedicated Rx buffers.
 *
 *  The number of default Tx and Rx buffers varies depending on the size of the
 *  device's message RAM. Check the doxygen for the device-specific CAN
 *  implementation to find the message RAM size. If using a custom message RAM
 *  configuration, utilize the entire space by maximizing the number of Rx/Tx
 *  buffers for optimal performance.
 *
 *  ## CAN Write Behavior
 *  CAN_write() will return immediately after the message is loaded into the CAN
 *  controller's message RAM and pending transfer; it does not wait for the CAN
 *  message to be transmitted on the bus before returning. The CAN controller will
 *  automatically handle transmission retries in the event of a failure.
 *
 *  ## CAN Read Behavior
 *  When a message is received in Rx FIFO0/1 or a dedicated Rx buffer, the CAN
 *  driver's IRQ handler automatically reads the Rx buffer element from the CAN
 *  controller's message RAM and stores it in a ring buffer whose size is
 *  configurable in SysConfig. When CAN_read() is called, the Rx buffer element
 *  is copied from the ring buffer to the application. If the ring buffer becomes
 *  full, any new messages received will be lost until the application frees
 *  space in the ring buffer by calling CAN_read().
 *******************************************************************************
 */
#ifndef ti_drivers_can__include
#define ti_drivers_can__include

#include <stdint.h>
#include <stddef.h>

#include <ti/drivers/utils/StructRingBuf.h>
#include <ti/devices/DeviceFamily.h>

#include <third_party/mcan/MCAN.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! @addtogroup CAN_STATUS
 *  @{
 */

/*!
 *  @brief   Successful status code.
 *
 *  Functions return CAN_STATUS_SUCCESS if the function was executed
 *  successfully.
 */
#define CAN_STATUS_SUCCESS ((int_fast16_t)0)

/*!
 *  @brief   Generic error status code.
 *
 *  Functions return CAN_STATUS_ERROR if the function was not executed
 *  successfully and no more pertinent error code could be returned.
 */
#define CAN_STATUS_ERROR ((int_fast16_t)-1)

/*!
 *  @brief   Not supported status code.
 *
 *  Functions return CAN_STATUS_NOT_SUPPORTED if the function is not supported
 *  either by the target hardware or the driver implementation.
 */
#define CAN_STATUS_NOT_SUPPORTED ((int_fast16_t)-2)

/*!
 *  @brief  Tx buffer full status code.
 *
 *  Functions return CAN_STATUS_TXBUF_FULL if the Tx buffers are full.
 */
#define CAN_STATUS_TX_BUF_FULL ((int_fast16_t)-3)

/*!
 *  @brief  No received message available status code.
 *
 *  Functions return CAN_STATUS_NO_RX_MSG_AVAIL if there are no messages
 *  available in the Rx message ring buffer.
 */
#define CAN_STATUS_NO_RX_MSG_AVAIL ((int_fast16_t)-4)

/*! @}*/

/*! @addtogroup CAN_EVENT
 *  @{
 */

/*!
 *  @brief   A SPI transfer error occurred.
 *
 *  @note    This error is only relevant for SPI-backed external CAN controllers.
 *
 *  If a SPI transfer error occurs, the CAN driver behavior is undefined and
 *  message transmit and receive may be unreliable. This error is an indication
 *  that further debug of the system HW & SW is required.
 */
#define CAN_EVENT_SPI_XFER_ERROR (0x200U)

/*!
 *  @brief   An uncorrected bit error occurred.
 *
 *  If an uncorrected bit error occurs, the driver should be closed and
 *  re-opened to attempt recovery.
 */
#define CAN_EVENT_BIT_ERR_UNCORRECTED (0x100U)

/*!
 *  @brief   The driver's Rx ring buffer was full.
 *
 *  The driver's Rx ring buffer was full resulting in a message being discarded.
 *  The application must call #CAN_read() in a more timely manner in response
 *  to CAN_EVENT_RX_DATA_AVAIL or the size of the Rx ring buffer should be
 *  increased.
 */
#define CAN_EVENT_RX_RING_BUFFER_FULL (0x80U)

/*!
 *  @brief   A message was lost for hardware Rx FIFO.
 *
 *  See event data for FIFO number.
 */
#define CAN_EVENT_RX_FIFO_MSG_LOST (0x40U)

/*!
 *  @brief   State change to error passive.
 */
#define CAN_EVENT_ERR_PASSIVE (0x20U)

/*!
 *  @brief   State change to error active.
 */
#define CAN_EVENT_ERR_ACTIVE (0x10U)

/*!
 *  @brief   State change to bus off.
 */
#define CAN_EVENT_BUS_OFF (0x08U)

/*!
 *  @brief   State change to bus on.
 */
#define CAN_EVENT_BUS_ON (0x04U)

/*!
 *  @brief   A CAN message transmission was completed.
 */
#define CAN_EVENT_TX_FINISHED (0x02U)

/*!
 *  @brief   Received CAN message data is available.
 *
 * This event is used as an indication that #CAN_read() can be called to read
 * the received message(s).
 */
#define CAN_EVENT_RX_DATA_AVAIL (0x01U)

/*! @}*/

/*!
 *  @brief  A CAN Rx buffer element struct for #CAN_read().
 */
typedef MCAN_RxBufElement CAN_RxBufElement;

/*!
 *  @brief  A CAN Tx buffer element struct for #CAN_write() and #CAN_writeBuffer().
 */
typedef MCAN_TxBufElement CAN_TxBufElement;

/*!
 *  @brief  A handle that is returned from a #CAN_open() call.
 */
typedef struct CAN_Config_ *CAN_Handle;

/*!
 *  @brief      The definition of a callback function used by the CAN driver.
 *
 *  @note       The callback can occur in task or interrupt context.
 *
 *  @param[in]  handle        A #CAN_Handle returned from #CAN_open
 *  @param[in]  event         @ref CAN_EVENT that has occurred.
 *  @param[in]  data          Data is event dependent:
 *                            - #CAN_EVENT_RX_DATA_AVAIL: number of Rx buffers
 *                              available to read.
 *                            - #CAN_EVENT_RX_FIFO_MSG_LOST: FIFO number.
 *                            - #CAN_EVENT_RX_RING_BUFFER_FULL: number of times
 *                              the Rx ring buffer became full resulting in message
 *                              loss.
 *                            - CAN_EVENT_TX_FINISHED: mask of buffers for which
 *                              transmission has occurred.
 *                            - CAN_EVENT_SPI_XFER_ERROR: SPI transfer status code.
 *                            - other: unused
 *  @param[in]  userArg       A user supplied argument specified
 *                            in CAN_Params.
 */
typedef void (*CAN_EventCbk)(CAN_Handle handle, uint32_t event, uint32_t data, void *userArg);

/*!
 *  @brief CAN Message RAM configuration
 *
 *  The #CAN_MsgRAMConfig structure contains information used to configure
 *  the message RAM.
 *
 *  This structure needs to be defined before calling CAN_init() and it must
 *  not be changed thereafter.
 *
 *  @sa     CAN_init()
 */
typedef struct
{
    uint32_t stdFilterNum;
    /*!< Number of Standard ID filter elements [0-128] */
    uint32_t extFilterNum;
    /*!< Number of Extended ID filter elements [0-64] */
    MCAN_StdMsgIDFilterElement *stdMsgIDFilterList;
    /*!< Points to Standard ID filter elements. Set to NULL if zero elements. */
    MCAN_ExtMsgIDFilterElement *extMsgIDFilterList;
    /*!< Points to Extended ID filter elements. Set to NULL if zero elements. */

    /*
     * Note: All Rx and Tx buffer elements include a 64-byte payload if CAN FD
     *       is enabled. Otherwise, they include an 8-byte payload.
     */

    uint32_t rxFIFONum[2];
    /*!< Number of Rx FIFO elements [0-64] for FIFO0 and FIFO1 respectively */
    uint32_t rxBufNum;
    /*!< Number of dedicated Rx Buffer elements */

    uint32_t txBufNum;
    /*!< Number of dedicated Tx Buffer elements */
    uint32_t txFIFOQNum;
    /*!< Number of Tx buffer elements for Tx FIFO or Queue */
    uint32_t txFIFOQMode;
    /*!< Tx FIFO/Queue Mode: 0 = Tx FIFO mode, 1 = Tx Queue mode */
} CAN_MsgRAMConfig;

/*!
 *  @brief Structure defining the raw MCAN CAN FD data phase bit rate configuration
 *
 *  The length of the bit time is [dtSeg1 + dtSeg2 + 3] tq.
 *  Bit Rate = (MCAN system clock / (dbrp + 1)) / (length of bit time)
 */
typedef struct
{
    uint32_t dbrp;
    /*!< DBRP: Data Bit Rate Prescaler value for the MCAN system clock.
     * Interpreted by MCAN as the value is this field + 1. If Transceiver Delay
     * Compensation is used, valid range: 0 to 1. Otherwise, valid range: 0 to
     * 31. */
    uint32_t dtSeg1;
    /*!< DTSEG1: Data time segment 1 + prop segment. Interpreted by MCAN as the
     * value in this field + 1. Valid range: 0 to 31. */
    uint32_t dtSeg2;
    /*!< DTSEG2: Data time segment 2. Interpreted by MCAN as the value is this
     * field + 1. Valid range: 0 to 15. */
    uint32_t dsjw;
    /*!< DSJW: Data Resynchronization Jump Width. Interpreted by MCAN as the
     * value is this field + 1. Valid range: 0 to 15. Typically set equal to tSeg2. */
    uint32_t tdcOffset;
    /*!< Transceiver Delay Compensation offset. Valid range: 0 to 127 mtq. */
    uint32_t tdcFilterWinLen;
    /*!< Transceiver Delay Compensation filter window length. Valid range: 0 to
     * 127 mtq. */
} CAN_DataBitRateTimingRaw;

/*!
 *  @brief Structure defining the raw MCAN bit rate configuration
 *
 *  The length of the bit time is [ntSeg1 + ntSeg2 + 3] tq.
 *  Bit Rate = (MCAN system clock / (nbrp + 1)) / (length of bit time)
 */
typedef struct
{
    uint32_t nbrp;
    /*!< NBRP: Nominal Bit Rate Prescaler value for the MCAN system clock.
     * Interpreted by MCAN as the value is this field + 1. Valid range: 0 to
     * 511. */
    uint32_t ntSeg1;
    /*!< NTSEG1: Nominal time segment 1 + prop segment. Interpreted by MCAN as the
     * value in this field + 1. Valid range: 1 to 255. */
    uint32_t ntSeg2;
    /*!< NTSEG2: Nominal time segment 2. Interpreted by MCAN as the value is this
     * field + 1. Valid range: 1 to 127. */
    uint32_t nsjw;
    /*!< NSJW: Nominal Resynchronization Jump Width. Interpreted by MCAN as the
     * value is this field + 1. Valid range: 0 to 127. Typically set equal to
     * tSeg2. */

    const CAN_DataBitRateTimingRaw *dataTiming;
    /*!< CAN FD data phase bit rate configuration. May be set to NULL if CAN FD
     * bit rate switching is not enabled. */
} CAN_BitRateTimingRaw;

/*!
 *  @brief    CAN Parameters
 *
 *  CAN parameters are used with the CAN_open() call. Default values for
 *  these parameters are set using CAN_Params_init().
 *
 *  @sa       CAN_Params_init()
 */
typedef struct
{
    const CAN_MsgRAMConfig *msgRAMConfig;  /*!< Pointer to message RAM configuration */
    const CAN_BitRateTimingRaw *bitTiming; /*!< Pointer to raw bit timing values */
    CAN_EventCbk eventCbk;                 /*!< Pointer to event callback function */
    uint32_t eventMask;                    /*!< Mask of events to call event callback function for */
    void *userArg;                         /*!< User supplied arg for callback */
} CAN_Params;

/*!
 *  @brief  CAN Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct
{
    CAN_EventCbk eventCbk; /*!< User supplied event callback */
    uint32_t eventMask;    /*!< User supplied event mask */
    void *userArg;         /*!< User supplied arg for callback */
    uint32_t intMask;      /*!< MCAN interrupt mask */

    uint32_t txBufNum;     /*!< Copy of the number of dedicated Tx Buffer elements */
    uint32_t txFIFOQNum;   /*!< Copy of the number of Tx buffer elements to use for Tx FIFO or Queue */
    uint32_t rxBufNum;     /*!< Copy of the number of dedicated Rx Buffer elements */
    uint32_t rxFIFONum[2]; /*!< Copy of the number of Rx FIFO elements [0-64] for FIFO 0 and FIFO 1 respectively */

    StructRingBuf_Object rxStructRingBuf; /*!< Receive ring buffer */
    StructRingBuf_Object txStructRingBuf; /*!< Transmit ring buffer */

    bool isOpen;
} CAN_Object;

/*!
 *  @brief CAN hardware attributes
 *
 *  CAN hardware attributes provides constant HW-specific configuration and
 *  should be included in the board file or SysConfig generated file and pointed
 *  to by the #CAN_config struct.
 */
typedef struct
{
    bool enableCANFD;           /*!< Set to true to enable CAN FD */
    bool enableBRS;             /*!< Set to true to enable CAN FD bit rate switching */
    bool rejectNonMatchingMsgs; /*!< Set to true to reject incoming messages that do not match a filter */
    uint32_t nominalBitRate;    /*!< Bit rate for arbitration */
    uint32_t dataBitRate;       /*!< Bit rate for CAN-FD data phase */

    void *rxRingBufPtr;   /*!< Pointer to Rx ring buffer */
    void *txRingBufPtr;   /*!< Pointer to Tx ring buffer */
    size_t rxRingBufSize; /*!< Number of Rx ring buffer elements */
    size_t txRingBufSize; /*!< Number of Tx ring buffer elements */
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    uint32_t intPriority; /*!< Interrupt priority */
    uint32_t rxPinMux;    /*!< Receive pin mux */
    uint32_t txPinMux;    /*!< Transmit pin mux */
    uint_least8_t rxPin;  /*!< Receive pin */
    uint_least8_t txPin;  /*!< Transmit pin */
#endif
} CAN_HWAttrs;

/*!
 *  @brief  CAN Global configuration
 *
 *  The CAN_Config structure contains a set of pointers used to characterize
 *  the CAN driver implementation.
 *
 */
typedef struct CAN_Config_
{
    /*! Pointer to a driver specific data object */
    CAN_Object *object;

    /*! Pointer to a driver specific hardware attributes structure */
    const CAN_HWAttrs *hwAttrs;
} CAN_Config;

extern const CAN_Config CAN_config[];
extern const uint_least8_t CAN_count;

/* Data Length Code for use with CAN_RxBufElement & CAN_TxBufElement */
#define CAN_DLC_0B  ((uint32_t)0U)
#define CAN_DLC_1B  ((uint32_t)1U)
#define CAN_DLC_2B  ((uint32_t)2U)
#define CAN_DLC_3B  ((uint32_t)3U)
#define CAN_DLC_4B  ((uint32_t)4U)
#define CAN_DLC_5B  ((uint32_t)5U)
#define CAN_DLC_6B  ((uint32_t)6U)
#define CAN_DLC_7B  ((uint32_t)7U)
#define CAN_DLC_8B  ((uint32_t)8U)
#define CAN_DLC_12B ((uint32_t)9U)  /*!< Equivalent to CAN_DLC_8B for classic CAN */
#define CAN_DLC_16B ((uint32_t)10U) /*!< Equivalent to CAN_DLC_8B for classic CAN */
#define CAN_DLC_20B ((uint32_t)11U) /*!< Equivalent to CAN_DLC_8B for classic CAN */
#define CAN_DLC_24B ((uint32_t)12U) /*!< Equivalent to CAN_DLC_8B for classic CAN */
#define CAN_DLC_32B ((uint32_t)13U) /*!< Equivalent to CAN_DLC_8B for classic CAN */
#define CAN_DLC_48B ((uint32_t)14U) /*!< Equivalent to CAN_DLC_8B for classic CAN */
#define CAN_DLC_64B ((uint32_t)15U) /*!< Equivalent to CAN_DLC_8B for classic CAN */

/*
 * Filter Element Configuration
 *   0 = Disable filter element
 *   1 = Store in Rx FIFO 0 if filter matches
 *   2 = Store in Rx FIFO 1 if filter matches
 *   3 = Reject ID if filter matches
 *   4 = Set priority if filter matches
 *   5 = Set priority and store in FIFO 0 if filter matches
 *   6 = Set priority and store in FIFO 1 if filter matches
 *   7 = Store into Rx Buffer or as debug message,
 *       configuration of SFT[1:0] ignored.
 *   If SFEC = 4-6, a match sets high priority
 *   message status and generates an interrupt.
 */
#define CAN_FEC_DISABLE_FILTER         0U
#define CAN_FEC_STORE_RXFIFO0          1U
#define CAN_FEC_STORE_RXFIFO1          2U
#define CAN_FEC_REJECT_ID              3U
#define CAN_FEC_SET_PRIO               4U
#define CAN_FEC_SET_PRIO_STORE_RXFIFO0 5U
#define CAN_FEC_SET_PRIO_STORE_RXFIFO1 6U
#define CAN_FEC_STORE_RXBUF            7U

/*
 * Filter Type
 *   0 = Range filter from SFID1 to SFID2 (SFID2 >= SFID1)
 *   1 = Dual ID filter for SFID1 or SFID2
 *   2 = Classic filter: SFID1 = filter, SFID2 = mask
 *   3 = Filter element disabled
 */
#define CAN_FILTER_RANGE     0U
#define CAN_FILTER_DUAL_ID   1U
#define CAN_FILTER_WITH_MASK 2U
#define CAN_FILTER_DISABLE   3U

/*!
 *  @brief  This function initializes the CAN module.
 *
 *  @pre    The CAN_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other CAN driver APIs. This function call does not modify any
 *          peripheral registers.
 *
 *  @sa     #CAN_open
 */
void CAN_init(void);

/*!
 *  @brief  Initializes the #CAN_Params struct to its default values
 *
 *  @param  params      An pointer to #CAN_Params structure for
 *                      initialization
 *
 *  Defaults values are:
 *     .msgRAMConfig = NULL
 *     .bitTiming    = NULL
 *     .eventCbk     = NULL
 *     .eventMask    = 0U
 *     .userArg      = NULL
 *
 *  @sa     #CAN_open
 */
void CAN_Params_init(CAN_Params *params);

/*!
 *  @brief  Initializes a CAN driver instance and returns a handle.
 *
 *  Initializes a CAN driver instance, configures the CAN device in normal
 *  operational mode, and returns a handle. Since the MCAN IP is highly
 *  configurable, the message RAM configuration and raw bit timings may be
 *  provided in the parameter block. Raw bit timings are required to use
 *  transmitter delay compensation. Invalid message RAM configuration or
 *  bit timing parameters will cause this function to fail.
 *
 *  @pre    CAN controller has been initialized using #CAN_init
 *
 *  @param  index         Logical peripheral number for the CAN indexed into
 *                        the #CAN_config table
 *
 *  @param  params        Pointer to a parameter block, if NULL it will use
 *                        default values.
 *
 *  @return A #CAN_Handle on success, otherwise, NULL upon error or if it has been
 *          opened already.
 *
 *  @pre    #CAN_init
 *  @sa     #CAN_close
 */
CAN_Handle CAN_open(uint_least8_t index, CAN_Params *params);

/*!
 *  @brief  Closes a CAN peripheral specified by \a handle.
 *
 *  @pre    #CAN_open() has to be called first.
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *
 *  @pre     #CAN_open
 */
void CAN_close(CAN_Handle handle);

/*!
 *  @brief  Reads a received CAN message
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *  @param  elem    A pointer to a #CAN_RxBufElement.
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_NO_RX_MSG_AVAIL if no messages are available.
 *
 *  @pre    #CAN_open
 */
int_fast16_t CAN_read(CAN_Handle handle, CAN_RxBufElement *elem);

/*!
 *  @brief  Sends CAN message using the Tx FIFO/Queue
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *  @param  elem    A pointer to a #CAN_TxBufElement.
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_ERROR if no Tx FIFO/Queue is configured.
 *  @retval CAN_STATUS_TX_BUF_FULL if the Tx buffer is full.
 *
 *  @pre    #CAN_open
 */
int_fast16_t CAN_write(CAN_Handle handle, const CAN_TxBufElement *elem);

/*!
 *  @brief  Sends CAN message using a dedicated Tx Buffer
 *
 *  Dedicated Tx buffers are intended for message transmission under complete
 *  control of the application. A custom message RAM config with dedicated Tx
 *  buffer(s) must be provided during #CAN_init in order to utilize this
 *  function.
 *
 *  @note   The default message RAM configuration uses a Tx Queue in which the
 *          message with the highest priority in the queue is transmitted first.
 *          If a custom message RAM configuration with Tx FIFO is used, messages
 *          are transmitted out in the order they are placed in the FIFO. Adding
 *          a dedicated Tx buffer to the custom configuration and calling this
 *          function can allow a new higher priority message to be transmitted
 *          before the Tx FIFO is empty.
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *  @param  bufIdx  Index of the dedicated Tx buffer.
 *  @param  elem    A pointer to a #CAN_TxBufElement.
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_ERROR if the Tx buffer index is invalid or
 *          the buffer already has a Tx request pending.
 *
 *  @pre    #CAN_open
 */
int_fast16_t CAN_writeBuffer(CAN_Handle handle, uint32_t bufIdx, const CAN_TxBufElement *elem);

/*!
 *  @brief  Enables external loopback test mode
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *
 *  @pre    #CAN_open
 *  @sa     #CAN_enableLoopbackInt
 *  @sa     #CAN_disableLoopback
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_NOT_SUPPORTED if this feature is not supported.
 */
int_fast16_t CAN_enableLoopbackExt(CAN_Handle handle);

/*!
 *  @brief  Enables internal loopback test mode
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *
 *  @pre    #CAN_open
 *  @sa     #CAN_enableLoopbackExt
 *  @sa     #CAN_disableLoopback
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_NOT_SUPPORTED if this feature is not supported.
 */
int_fast16_t CAN_enableLoopbackInt(CAN_Handle handle);

/*!
 *  @brief  Disables loopback test mode
 *
 *  @param  handle  A #CAN_Handle returned from #CAN_open
 *
 *  @pre    #CAN_open
 *  @pre    #CAN_enableLoopbackExt or #CAN_enableLoopbackInt
 *
 *  @retval CAN_STATUS_SUCCESS if successful.
 *  @retval CAN_STATUS_NOT_SUPPORTED if this feature is not supported.
 */
int_fast16_t CAN_disableLoopback(CAN_Handle handle);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_can__include */
