/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated
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
 *  @file    MCAN.h
 *
 *  @brief   Hardware abstraction layer for M_CAN Controller v3.2.1
 *
 *******************************************************************************
 */

/* Structs were designed to avoid bitfields, unions, and casting
 * for portability and to avoid static analysis violations.
 */
#ifndef __MCAN_H__
#define __MCAN_H__

#include <stdint.h>
#include <stddef.h>
#include <third_party/mcan/inc/MCAN_reg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * MCAN register and message RAM R/W functions are defined in the
 * device-specific driver since access methods will vary for internal vs
 * external devices as will the base MCAN register address.
 */
extern void MCAN_writeReg(uint32_t offset, uint32_t value);
extern void MCAN_writeMsgRam(uint32_t offset, const uint8_t *src, size_t numBytes);
extern uint32_t MCAN_readReg(uint32_t offset);
extern void MCAN_readMsgRam(uint8_t *dst, uint32_t offset, size_t numBytes);
extern uint32_t MCAN_getMRAMOffset(void);

/*!
 *  @brief   Successful status code.
 *
 *  Functions return MCAN_STATUS_SUCCESS if the function was executed
 *  successfully.
 */
#define MCAN_STATUS_SUCCESS ((int_fast16_t)0)

/*!
 *  @brief   Generic error status code.
 *
 *  Functions return MCAN_STATUS_ERROR if the function was not executed
 *  successfully and no more specific error is applicable.
 */
#define MCAN_STATUS_ERROR ((int_fast16_t)-1)

/*! @brief  Maximum payload supported by CAN-FD protocol in bytes */
#define MCAN_MAX_PAYLOAD_SIZE (64U)

/*! @brief  Standard Message ID filter element size in bytes */
#define MCAN_STD_ID_FILTER_ELEM_SIZE (4U)

/*! @brief  Extended Message ID filter element size in bytes */
#define MCAN_EXT_ID_FILTER_ELEM_SIZE (8U)

/*! @brief  Tx Event FIFO element size in bytes */
#define MCAN_TX_EVENT_ELEM_SIZE (8U)

/*! @brief  Tx/Rx Element Header Size in bytes */
#define MCAN_TX_RX_ELEMENT_HEADER_SIZE (8U)

/*!
 *  @anchor MCAN_IntLineNum
 *  @name MCAN Interrupt Line Number
 *  @{
 */
/*!
 *  @brief    Enum to select the MCAN interrupt lines
 */
typedef uint32_t MCAN_IntLineNum;

#define MCAN_INT_LINE_NUM_0 (0U)
/*!< MCAN interrupt line 0 */
#define MCAN_INT_LINE_NUM_1 (1U)
/*!< MCAN interrupt line 1 */
/* @} */

/*!
 *  @anchor MCAN_OperationMode
 *  @name MCAN Operation Mode
 *  @{
 */
/*!
 *  @brief    Enum to represent the MCAN mode of operation
 */
typedef uint32_t MCAN_OperationMode;

#define MCAN_OPERATION_MODE_NORMAL  (0U)
/*!< MCAN normal mode */
#define MCAN_OPERATION_MODE_SW_INIT (1U)
/*!< MCAN SW initialization mode */
/* @} */

/*!
 *  @anchor MCAN_MemType
 *  @name MCAN Mem type
 *  @{
 */
/*!
 *  @brief    Enum to represent the MCAN Message RAM type.
 */
typedef uint32_t MCAN_MemType;

#define MCAN_MEM_TYPE_BUF  (0U)
/*!< MCAN Msg RAM buffers */
#define MCAN_MEM_TYPE_FIFO (1U)
/*!< MCAN Msg RAM FIFO/Queue */
/* @} */

/*!
 *  @anchor MCAN_RxFifoNum
 *  @name MCAN Rx FIFO Number
 *  @{
 */
/*!
 *  @brief    Enum to represent the MCAN Rx FIFO number
 */
typedef uint32_t MCAN_RxFifoNum;

#define MCAN_RX_FIFO_NUM_0 (0U)
/*!< MCAN Rx FIFO 0 */
#define MCAN_RX_FIFO_NUM_1 (1U)
/*!< MCAN Rx FIFO 1 */
/* @} */

/*!
 *  @anchor MCAN_ElemSize
 *  @name MCAN Element Size
 *  @{
 */
/*!
 *  @brief    Enum to represent FIFO/Buffer element Size
 */
typedef uint32_t MCAN_ElemSize;

#define MCAN_ELEM_SIZE_8BYTES  (0U)
/*!< 8 byte data field */
#define MCAN_ELEM_SIZE_12BYTES (1U)
/*!< 12 byte data field */
#define MCAN_ELEM_SIZE_16BYTES (2U)
/*!< 16 byte data field */
#define MCAN_ELEM_SIZE_20BYTES (3U)
/*!< 20 byte data field */
#define MCAN_ELEM_SIZE_24BYTES (4U)
/*!< 24 byte data field */
#define MCAN_ELEM_SIZE_32BYTES (5U)
/*!< 32 byte data field */
#define MCAN_ELEM_SIZE_48BYTES (6U)
/*!< 48 byte data field */
#define MCAN_ELEM_SIZE_64BYTES (7U)
/*!< 64 byte data field */
/* @} */

/*!
 *  @anchor MCAN_TimeOutSelect
 *  @name MCAN Timeout select
 *  @{
 */
/*!
 *  @brief    Enum to represent the MCAN time-out counter configuration
 */
typedef uint32_t MCAN_TimeOutSelect;

#define MCAN_TIMEOUT_SELECT_CONT          (0U)
/*!< Continuous operation Mode */
#define MCAN_TIMEOUT_SELECT_TX_EVENT_FIFO (1U)
/*!< Timeout controlled by Tx Event FIFO */
#define MCAN_TIMEOUT_SELECT_RX_FIFO0      (2U)
/*!< Timeout controlled by Rx FIFO 0 */
#define MCAN_TIMEOUT_SELECT_RX_FIFO1      (3U)
/*!< Timeout controlled by Rx FIFO 1 */
/* @} */

/*!
 *  @anchor MCAN_IntSrc
 *  @name MCAN Interrupt Source
 *  @{
 */
/*!
 *  @brief  Enum for MCAN interrupts.
 */
typedef uint32_t MCAN_IntSrc;

#define MCAN_INT_SRC_RX_FIFO0_NEW_MSG       (MCAN_IR_RF0N_MASK)
/*!< Rx FIFO 0 New Message interrupt */
#define MCAN_INT_SRC_RX_FIFO0_WATERMARK     (MCAN_IR_RF0W_MASK)
/*!< Rx FIFO 0 Watermark Reached interrupt */
#define MCAN_INT_SRC_RX_FIFO0_FULL          (MCAN_IR_RF0F_MASK)
/*!< Rx FIFO 0 Full interrupt */
#define MCAN_INT_SRC_RX_FIFO0_MSG_LOST      (MCAN_IR_RF0L_MASK)
/*!< Rx FIFO 0 Message Lost interrupt */
#define MCAN_INT_SRC_RX_FIFO1_NEW_MSG       (MCAN_IR_RF1N_MASK)
/*!< Rx FIFO 1 New Message interrupt */
#define MCAN_INT_SRC_RX_FIFO1_WATERMARK     (MCAN_IR_RF1W_MASK)
/*!< Rx FIFO 1 Watermark Reached interrupt */
#define MCAN_INT_SRC_RX_FIFO1_FULL          (MCAN_IR_RF1F_MASK)
/*!< Rx FIFO 1 Full interrupt */
#define MCAN_INT_SRC_RX_FIFO1_MSG_LOST      (MCAN_IR_RF1L_MASK)
/*!< Rx FIFO 1 Message Lost interrupt */
#define MCAN_INT_SRC_HIGH_PRIO_MSG          (MCAN_IR_HPM_MASK)
/*!< High Priority Message interrupt */
#define MCAN_INT_SRC_TRANS_COMPLETE         (MCAN_IR_TC_MASK)
/*!< Transmission Completed interrupt */
#define MCAN_INT_SRC_TRANS_CANCEL_FINISH    (MCAN_IR_TCF_MASK)
/*!< Transmission Cancellation Finished interrupt */
#define MCAN_INT_SRC_TX_FIFO_EMPTY          (MCAN_IR_TFE_MASK)
/*!< Tx FIFO Empty interrupt */
#define MCAN_INT_SRC_TX_EVT_FIFO_NEW_ENTRY  (MCAN_IR_TEFN_MASK)
/*!< Tx Event FIFO New Entry interrupt */
#define MCAN_INT_SRC_TX_EVT_FIFO_WATERMARK  (MCAN_IR_TEFW_MASK)
/*!< Tx Event FIFO Watermark Reached interrupt */
#define MCAN_INT_SRC_TX_EVT_FIFO_FULL       (MCAN_IR_TEFF_MASK)
/*!< Tx Event FIFO Full interrupt */
#define MCAN_INT_SRC_TX_EVT_FIFO_ELEM_LOST  (MCAN_IR_TEFL_MASK)
/*!< Tx Event FIFO Element Lost interrupt */
#define MCAN_INT_SRC_TIMESTAMP_WRAPAROUND   (MCAN_IR_TSW_MASK)
/*!< Timestamp Wraparound interrupt */
#define MCAN_INT_SRC_MSG_RAM_ACCESS_FAILURE (MCAN_IR_MRAF_MASK)
/*!< Message RAM Access Failure interrupt */
#define MCAN_INT_SRC_TIMEOUT                (MCAN_IR_TOO_MASK)
/*!< Timeout Occurred interrupt */
#define MCAN_INT_SRC_DEDICATED_RX_BUFF_MSG  (MCAN_IR_DRX_MASK)
/*!< Message stored to Dedicated Rx Buffer interrupt */
#define MCAN_INT_SRC_BIT_ERR_CORRECTED      (MCAN_IR_BEC_MASK)
/*!< Bit Error Corrected interrupt */
#define MCAN_INT_SRC_BIT_ERR_UNCORRECTED    (MCAN_IR_BEU_MASK)
/*!< Bit Error Uncorrected interrupt */
#define MCAN_INT_SRC_ERR_LOG_OVERFLOW       (MCAN_IR_ELO_MASK)
/*!< Error Logging Overflow interrupt */
#define MCAN_INT_SRC_ERR_PASSIVE            (MCAN_IR_EP_MASK)
/*!< Error Passive interrupt */
#define MCAN_INT_SRC_WARNING_STATUS         (MCAN_IR_EW_MASK)
/*!< Warning Status interrupt */
#define MCAN_INT_SRC_BUS_OFF_STATUS         (MCAN_IR_BO_MASK)
/*!< Bus_Off Status interrupt */
#define MCAN_INT_SRC_WATCHDOG               (MCAN_IR_WDI_MASK)
/*!< Watchdog Interrupt interrupt */
#define MCAN_INT_SRC_PROTOCOL_ERR_ARB       (MCAN_IR_PEA_MASK)
/*!< Protocol Error in Arbitration Phase interrupt */
#define MCAN_INT_SRC_PROTOCOL_ERR_DATA      (MCAN_IR_PED_MASK)
/*!< Protocol Error in Data Phase interrupt */
#define MCAN_INT_SRC_RES_ADDR_ACCESS        (MCAN_IR_ARA_MASK)
/*!< Access to Reserved Address interrupt */
/* @} */

/*!
 *  @anchor MCAN_LpbkMode
 *  @name MCAN Loopback Mode
 *  @{
 */
/*!
 *  @brief    Enum to select the MCAN Loopback mode
 */
typedef uint32_t MCAN_LpbkMode;

#define MCAN_LPBK_MODE_INTERNAL (0U)
/*!< Internal Loop Back Mode.
 *   This mode can be used for hot self-test and this mode will not
 *   affect bus state.
 */
#define MCAN_LPBK_MODE_EXTERNAL (1U)
/*!< External Loop Back Mode.
 *   In this mode, the M_CAN controller treats its own transmitted messages as
 *   received messages and stores them (if they pass acceptance filtering)
 *   into an Rx Buffer or an Rx FIFO. This mode will affect bus state.
 */
/* @} */

/*!
 *  @anchor MCAN_ComState
 *  @name MCAN Com State
 *  @{
 */
/*!
 *  @brief    Enum to represent MCAN communication state
 */
typedef uint32_t MCAN_ComState;

#define MCAN_COM_STATE_SYNCHRONIZING (0U)
/*!< MCAN is synchronizing on CAN communication */
#define MCAN_COM_STATE_IDLE          (1U)
/*!< MCAN is neither receiver nor transmitter */
#define MCAN_COM_STATE_RECEIVER      (2U)
/*!< MCAN is operating as receiver */
#define MCAN_COM_STATE_TRANSMITTER   (3U)
/*!< MCAN is operating as transmitter */
/* @} */

/*!
 *  @anchor MCAN_GFC_NM
 *  @name MCAN General Filter Control Non-Matching
 *  @{
 */
/*!
 *  @brief    Enum to represent the MCAN general filter configuration for non-matching frames
 */
typedef uint32_t MCAN_GFCNonMatching;

#define MCAN_GFC_NM_ACCEPT_INTO_RXFIFO0 (0U)
/*!< Accept non-matching frames into Rx FIFO0 */
#define MCAN_GFC_NM_ACCEPT_INTO_RXFIFO1 (1U)
/*!< Accept non-matching frames into Rx FIFO0 */
#define MCAN_GFC_NM_REJECT              (3U)
/*!< Reject non-matching frames */
/* @} */

/*!
 *  @anchor MCAN_TSCC_TS_SEL
 *  @name MCAN timestamp select
 *  @{
 */
/*!
 *  @brief    Enum to represent the MCAN timestamp select
 */
typedef uint32_t MCAN_TSCCTimestampSel;

#define MCAN_TSCC_COUNTER_ALWAYS_0 (0U)
/*!< Timestamp counter value always 0x0000 */
#define MCAN_TSCC_COUNTER_USE_TCP  (1U)
/*!< Timestamp counter value incremented according to Timestamp Counter Prescaler */
#define MCAN_TSCC_COUNTER_EXTERNAL (2U)
/*!< External timestamp counter value used - req'd for CAN FD */
/* @} */

/*!
 *  @anchor MCAN_ErrCode
 *  @name MCAN Error Code
 *  @{
 */
/*!
 *  @brief    Enum to represent M_CAN Error Code
 */
typedef uint32_t MCAN_ErrCode;

#define MCAN_ERR_CODE_NO_ERROR    (0U)
/*!< No error occurred since LEC has been reset by
 *   successful reception or transmission.
 */
#define MCAN_ERR_CODE_STUFF_ERROR (1U)
/*!< More than 5 equal bits in a sequence have occurred in a part of
 *   a received message where this is not allowed.
 */
#define MCAN_ERR_CODE_FORM_ERROR  (2U)
/*!< A fixed format part of a received frame has the wrong format. */
#define MCAN_ERR_CODE_ACK_ERROR   (3U)
/*!< The message transmitted by the M_CAN was not acknowledged
 *   by another node.
 */
#define MCAN_ERR_CODE_BIT1_ERROR  (4U)
/*!< During the transmission of a message (with the exception of
 *   the arbitration field), the device wanted to send a
 *   recessive level (bit of logical value 1))
 *   but the monitored bus value was dominant.
 */
#define MCAN_ERR_CODE_BIT0_ERROR  (5U)
/*!< During the transmission of a message (or acknowledge bit,
 *   or active error flag, or overload flag), the device wanted to send
 *   a dominant level (data or identifier bit logical value 0),
 *   but the monitored bus value was recessive. During Bus_Off recovery
 *   this status is set each time a sequence of 11 recessive bits has been
 *   monitored. This enables the CPU to monitor the proceeding of
 *   the Bus_Off recovery sequence (indicating the bus is not stuck at
 *   dominant or continuously disturbed).
 */
#define MCAN_ERR_CODE_CRC_ERROR   (6U)
/*!< The CRC check sum of a received message was incorrect.
 *   The CRC of an incoming message does not match with the
 *   CRC calculated from the received data.
 */
#define MCAN_ERR_CODE_NO_CHANGE   (7U)
/*! < Any read access to the Protocol Status Register re-initializes the LEC to
 *    7. When the LEC shows the value 7, no CAN bus event was detected since the
 *    last CPU read access to the Protocol Status Register.
 */
/* @} */

/*!
 *  @brief  Structure for MCAN Transmitter Delay Compensation parameters.
 */
typedef struct
{
    uint32_t tdcf;
    /*!< Transmitter Delay Compensation Filter Window Length.
     *   Defines the minimum value for the Secondary Sample Point (SSP) position;
     *   dominant edges on M_CAN Rx that would result in an earlier SSP position
     *   are ignored for transmitter delay measurement. The feature is enabled when
     *   \c TDCF is configured to a value greater than \c TDCO.
     *   Range:[0x0-0x7F]
     */
    uint32_t tdco;
    /*!< Transmitter Delay Compensation Offset.
     *   Defines the distance between the measured delay from M_CAN Tx to
     *   M_CAN Rx and the secondary sample point. Transmitter Delay
     *   Compensation is enabled when this value is non-zero and the
     *   \c dataRatePrescaler is 0 or 1. If the \c dataRatePrescaler is > 1,
     *   Transmitter Delay Compensation cannot be enabled.
     *   Range:[0x0-0x7F]
     */
} MCAN_TDCConfig;

/*!
 *  @brief  Structure for bit timing parameters.
 *
 *  @note   Bit timing related to data phase and transmitter delay compensation
 *          are only valid if CAN-FD mode and bit rate switching are enabled.
 */
typedef struct
{
    uint32_t nomRatePrescaler;
    /*!< Nominal Bit Rate Prescaler.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0x1FF]
     */
    uint32_t nomTimeSeg1;
    /*!< Nominal Time segment before sample point.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0xFF]
     */
    uint32_t nomTimeSeg2;
    /*!< Nominal Time segment after sample point.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0x7F]
     */
    uint32_t nomSynchJumpWidth;
    /*!< Nominal (Re)Synchronization Jump Width.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0x7F]
     */
    uint32_t dataRatePrescaler;
    /*!< Data Bit Rate Prescaler.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0x01] if Transmitter Delay Compensation is enabled.
     *   Range:[0x0-0x1F] if Transmitter Delay Compensation is disabled.
     */
    uint32_t dataTimeSeg1;
    /*!< Data Time segment before sample point.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0x1F]
     */
    uint32_t dataTimeSeg2;
    /*!< Data Time segment after sample point.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0xF]
     */
    uint32_t dataSynchJumpWidth;
    /*!< Data (Re)Synchronization Jump Width.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0xF]
     */
    MCAN_TDCConfig tdcConfig;
    /*!< Transmitter Delay Compensation parameters.
     *   Refer struct MCAN_TDCConfig.
     */
} MCAN_BitTimingParams;

/*!
 *  @brief  Structure for MCAN Global Filter Configuration parameters.
 */
typedef struct
{
    uint32_t rrfe;
    /*!< Reject Remote Frames Extended
     *   <br> 0 = Filter remote frames with 29-bit extended IDs
     *   <br> 1 = Reject all remote frames with 29-bit extended IDs
     */
    uint32_t rrfs;
    /*!< Reject Remote Frames Standard
     *   <br> 0 = Filter remote frames with 11-bit standard IDs
     *   <br> 1 = Reject all remote frames with 11-bit standard IDs
     */
    MCAN_GFCNonMatching anfe;
    /*!< Accept Non-matching Frames Extended.
     *   Refer enum MCAN_GFCNonMatching.
     */
    MCAN_GFCNonMatching anfs;
    /*!< Accept Non-matching Frames Standard.
     *   Refer enum MCAN_GFCNonMatching.
     */
} MCAN_GlobalFiltConfig;

/*!
 *  @brief  Structure for MCAN initialization parameters.
 */
typedef struct
{
    uint32_t fdMode;
    /*!< FD Operation Enable
     *   <br> 0 = FD operation disabled
     *   <br> 1 = FD operation enabled
     */
    uint32_t brsEnable;
    /*!< Bit Rate Switch Enable
     *   This is valid only when fdMode = 1.
     *   <br> 0 = Bit rate switching for transmissions disabled
     *   <br> 1 = Bit rate switching for transmissions enabled
     */
    uint32_t txpEnable;
    /*!< Transmit Pause
     *   <br> 0 = Transmit pause disabled
     *   <br> 1 = Transmit pause enabled
     */
    uint32_t efbi;
    /*!< Edge Filtering during Bus Integration
     *   <br> 0 = Edge filtering disabled
     *   <br> 1 = Two consecutive dominant tq required to detect an edge for
     *            hard synchronization
     */
    uint32_t pxhDisable;
    /*!< Protocol Exception Handling Disable
     *   <br> 0 = Protocol exception handling enabled
     *   <br> 1 = Protocol exception handling disabled
     */
    uint32_t darEnable;
    /*!< Disable Automatic Retransmission
     *   <br> 0 = Automatic retransmission of messages not transmitted successfully
     *            enabled
     *   <br> 1 = Automatic retransmission disabled
     */
    uint32_t wdcPreload;
    /*!< Start value of the Message RAM Watchdog Counter.
     *   Range:[0x0-0xFF]
     */
} MCAN_InitParams;

/*!
 *  @brief  Structure for MCAN configuration parameters.
 */
typedef struct
{
    uint32_t monEnable;
    /*!< Bus Monitoring Mode
     *   <br> 0 = Bus Monitoring Mode is disabled
     *   <br> 1 = Bus Monitoring Mode is enabled
     */
    uint32_t asmEnable;
    /*!< Restricted Operation Mode
     *   <br> 0 = Normal CAN operation
     *   <br> 1 = Restricted Operation Mode active
     *   <br> This mode should not be combined with test modes.
     */
    uint32_t tsPrescaler;
    /*!< Timestamp Counter Prescaler.
     *   This prescaler is only used when tsSelect is MCAN_TSCC_COUNTER_USE_TCP
     *   which means the internal timestamp counter source is selected.
     *   Interpreted by M_CAN as the value in this field + 1.
     *   Range:[0x0-0xF]
     */
    MCAN_TSCCTimestampSel tsSelect;
    /*!< Timestamp source selection.
     *   Refer enum MCAN_TSCCTimestampSel
     */
    MCAN_TimeOutSelect timeoutSelect;
    /*!< Time-out counter source select.
     *   Refer enum MCAN_TimeOutSelect.
     */
    uint32_t timeoutPreload;
    /*!< Start value of the Timeout Counter (down-counter).
     *   The Timeout Counter is decremented in multiples of CAN bit times [1-16]
     *   depending on the configuration of the tsPrescaler.
     *   Range:[0x0-0xFFFF]
     */
    uint32_t timeoutCntEnable;
    /*!< Time-out Counter Enable
     *   <br> 0 = Timeout Counter is disabled
     *   <br> 1 = Timeout Counter is enabled
     */
    MCAN_GlobalFiltConfig filterConfig;
    /*!< Global Filter Configuration parameters.
     *   Refer struct MCAN_GlobalFiltConfig.
     */
} MCAN_ConfigParams;

/*!
 *  @brief  Structure for MCAN error logging counters status.
 */
typedef struct
{
    uint32_t transErrLogCnt;
    /*!< Transmit Error Counter */
    uint32_t recErrCnt;
    /*!< Receive Error Counter */
    uint32_t rpStatus;
    /*!< Receive Error Passive
     *   <br> 0 = The Receive Error Counter is below the error passive level (128)
     *   <br> 1 = The Receive Error Counter has reached the error passive level (128)
     */
    uint32_t canErrLogCnt;
    /*!< CAN Error Logging */
} MCAN_ErrCntStatus;

/*!
 *  @brief  Structure for MCAN protocol status.
 */
typedef struct
{
    MCAN_ErrCode lastErrCode;
    /*!< Last Error Code.
     *   Refer enum MCAN_ErrCode
     */
    MCAN_ComState act;
    /*!< Activity - Monitors the module's CAN communication state.
     *   Refer enum MCAN_ComState
     */
    uint32_t errPassive;
    /*!< Error Passive
     *   <br> 0 = The M_CAN is in the Error_Active state
     *   <br> 1 = The M_CAN is in the Error_Passive state
     */
    uint32_t warningStatus;
    /*!< Warning Status
     *   <br> 0 = Both error counters are below the Error_Warning limit of 96
     *   <br> 1 = At least one of error counter has reached the Error_Warning
     *            limit of 96
     */
    uint32_t busOffStatus;
    /*!< Bus_Off Status
     *   <br> 0 = The M_CAN is not Bus_Off
     *   <br> 1 = The M_CAN is in Bus_Off state
     */
    MCAN_ErrCode dlec;
    /*!< Data Phase Last Error Code.
     *   Refer enum MCAN_ErrCode
     */
    uint32_t resi;
    /*!< ESI flag of last received CAN FD Message
     *   <br> 0 = Last received CAN FD message did not have its ESI flag set
     *   <br> 1 = Last received CAN FD message had its ESI flag set
     */
    uint32_t rbrs;
    /*!< BRS flag of last received CAN FD Message
     *   <br> 0 = Last received CAN FD message did not have its BRS flag set
     *   <br> 1 = Last received CAN FD message had its BRS flag set
     */
    uint32_t rfdf;
    /*!< Received a CAN FD Message
     *   <br> 0 = Since this bit was reset by the CPU, no CAN FD message has been
     *            received
     *   <br> 1 = Message in CAN FD format with FDF flag set has been received
     */
    uint32_t pxe;
    /*!< Protocol Exception Event
     *   <br> 0 = No protocol exception event occurred since last read access
     *   <br> 1 = Protocol exception event occurred
     */
    uint32_t tdcv;
    /*!< Transmitter Delay Compensation Value */
} MCAN_ProtocolStatus;

/*!
 *  @brief  Structure for MCAN Message RAM Configuration Parameters.
 *
 *  Message RAM can contain following sections:
 *  - Standard ID filters,
 *  - Extended ID filters,
 *  - Rx FIFO0,
 *  - Rx FIFO1,
 *  - Rx Buffers,
 *  - Tx Event FIFO,
 *  - Tx Buffers,
 *  - Tx FIFO (or Tx Queue)
 *
 *  @note   If any section in the message RAM is not used then its size should
 *          be initialized to '0'. (Number of buffers in case of Tx/Rx buffer).
 */
typedef struct
{
    uint32_t sidFilterStartAddr;
    /*!< Standard ID Filter List Start Address */
    uint32_t sidFilterListSize;
    /*!< List Size: Standard ID
     *   <br> 0 = No standard Message ID filter
     *   <br> 1-128 = Number of standard Message ID filter elements
     */
    uint32_t xidFilterStartAddr;
    /*!< Extended ID Filter List Start Address */
    uint32_t xidFilterListSize;
    /*!< List Size: Extended ID
     *   <br> 0 = No standard Message ID filter
     *   <br> 1-64 = Number of standard Message ID filter elements
     */

    uint32_t rxFifo0StartAddr;
    /*!< Rx FIFO0 Start Address */
    uint32_t rxFifo0Size;
    /*!< Rx FIFO0 Size
     *   <br> 0 = No Rx FIFO
     *   <br> 1-64 = Number of Rx FIFO elements
     */
    uint32_t rxFifo0Watermark;
    /*!< Rx FIFO0 Watermark
     *   <br> 0 = Watermark interrupt disabled
     *   <br> 1-64 = Level for Rx FIFO 0 watermark interrupt
     */
    uint32_t rxFifo0OpMode;
    /*!< Rx FIFO0 Operation Mode
     *   <br> 0 = FIFO blocking mode
     *   <br> 1 = FIFO overwrite mode
     */
    uint32_t rxFifo1StartAddr;
    /*!< Rx FIFO1 Start Address */
    uint32_t rxFifo1Size;
    /*!< Rx FIFO1 Size
     *   <br> 0 = No Rx FIFO
     *   <br> 1-64 = Number of Rx FIFO elements
     */
    uint32_t rxFifo1Watermark;
    /*!< Rx FIFO1 Watermark
     *   <br> 0 = Watermark interrupt disabled
     *   <br> 1-64 = Level for Rx FIFO 1 watermark interrupt
     */
    uint32_t rxFifo1OpMode;
    /*!< Rx FIFO1 Operation Mode
     *   <br> 0 = FIFO blocking mode
     *   <br> 1 = FIFO overwrite mode
     */
    uint32_t rxBufStartAddr;
    /*!< Rx Buffer Start Address */
    uint32_t rxBufElemSize;
    /*!< Rx Buffer Element Size */
    uint32_t rxFifo0ElemSize;
    /*!< Rx FIFO0 Element Size */
    uint32_t rxFifo1ElemSize;
    /*!< Rx FIFO1 Element Size */

    uint32_t txEventFifoStartAddr;
    /*!< Tx Event FIFO Start Address */
    uint32_t txEventFifoSize;
    /*!< Event FIFO Size
     *   <br> 0 = Tx Event FIFO disabled
     *   <br> 1-32 = Number of Tx Event FIFO elements
     */
    uint32_t txEventFifoWatermark;
    /*!< Tx Event FIFO Watermark
     *   <br> 0 = Watermark interrupt disabled
     *   <br> 1-32 = Level for Tx Event FIFO watermark interrupt
     */

    uint32_t txBufStartAddr;
    /*!< Tx Buffers Start Address */
    uint32_t txBufNum;
    /*!< Number of Dedicated Transmit Buffers
     *   <br> 0 = No Dedicated Tx Buffers
     *   <br> 1-32 = Number of Dedicated Tx Buffers
     */
    uint32_t txFifoQSize;
    /*!< Transmit FIFO/Queue Size
     *   <br> 0 = No Tx FIFO/Queue
     *   <br> 1-32 = Number of Tx Buffers used for Tx FIFO/Queue
     */
    uint32_t txFifoQMode;
    /*!< Tx FIFO/Queue Mode
     *   <br> 0 = Tx FIFO operation
     *   <br> 1 = Tx Queue operation
     */
    uint32_t txBufElemSize;
    /*!< Tx Buffer Element Size */
} MCAN_MsgRamConfig;

/*!
 *  @brief  Structure for MCAN High Priority Message.
 */
typedef struct
{
    uint32_t bufIdx;
    /*!< Buffer Index.
     *   Only valid when MSI[1] = 1.
     */
    uint32_t msi;
    /*!< Message Storage Indicator
     *   <br> 0 = No FIFO selected
     *   <br> 1 = FIFO message lost
     *   <br> 2 = Message stored in FIFO 0
     *   <br> 3 = Message stored in FIFO 1
     */
    uint32_t filterIdx;
    /*!< Filter Index */
    uint32_t filterList;
    /*!< Indicates the filter list of the matching filter element
     *   <br> 0 = Standard Filter List
     *   <br> 1 = Extended Filter List
     */
} MCAN_HighPriorityMsgInfo;

/*!
 *  @brief  Structure for MCAN new data flag for Rx buffer.
 */
typedef struct
{
    uint32_t statusLow;
    /*!< New data flags for Rx buffers 0 to 31. Set bit to clear flag. */
    uint32_t statusHigh;
    /*!< New data flags for Rx buffers 32 to 63. Set bit to clear flag. */
} MCAN_RxNewDataStatus;

/*!
 *  @brief  Structure for MCAN Rx FIFO Status.
 */
typedef struct
{
    uint32_t fillLvl;
    /*!< Rx FIFO Fill Level - number of elements stored in Rx FIFO */
    uint32_t getIdx;
    /*!< Rx FIFO Get Index */
    uint32_t putIdx;
    /*!< Rx FIFO Put Index */
    uint32_t fifoFull;
    /*!< Rx FIFO Full
     *   <br> 0 = Rx FIFO not full
     *   <br> 1 = Rx FIFO full
     */
    uint32_t msgLost;
    /*!< Rx FIFO Message Lost */
} MCAN_RxFifoStatus;

/*!
 *  @brief  Structure for MCAN Tx FIFO/Queue Status.
 */
typedef struct
{
    uint32_t freeLvl;
    /*!< Tx FIFO Free Level */
    uint32_t getIdx;
    /*!< Tx FIFO Get Index.
     *   Read as zero when Tx Queue operation is configured.
     */
    uint32_t putIdx;
    /*!< Tx FIFO/Queue Put Index */
    uint32_t fifoFull;
    /*!< Tx FIFO/Queue Full
     *   <br> 0 = Tx FIFO/Queue not full
     *   <br> 1 = Tx FIFO/Queue full
     */
} MCAN_TxFifoQStatus;

/*!
 *  @brief  Structure for MCAN Tx Event FIFO Status.
 */
typedef struct
{
    uint32_t fillLvl;
    /*!< Event FIFO Fill Level */
    uint32_t getIdx;
    /*!< Event FIFO Gut Index */
    uint32_t putIdx;
    /*!< Event FIFO Put Index */
    uint32_t fifoFull;
    /*!< Event FIFO Full
     *   <br> 0 = Tx Event FIFO not full
     *   <br> 1 = Tx Event FIFO full
     */
    uint32_t eleLost;
    /*!< Tx Event FIFO Element Lost
     *   <br> 0 = No Tx Event FIFO element lost
     *   <br> 1 = Tx Event FIFO element lost, also set after write attempt to
     *            Tx Event FIFO of size zero.
     */
} MCAN_TxEventFifoStatus;

/*!
 *  @brief  Structure for MCAN Tx Buffer element.
 */
typedef struct
{
    uint8_t data[MCAN_MAX_PAYLOAD_SIZE];
    /*!< Data bytes.
     *   Only first DLC number of bytes are valid.
     */
    uint32_t id;
    /*!< Identifier */
    uint32_t rtr;
    /*!< Remote Transmission Request
     *   <br> 0 = Transmit data frame
     *   <br> 1 = Transmit remote frame
     */
    uint32_t xtd;
    /*!< Extended Identifier
     *   <br> 0 = 11-bit standard identifier
     *   <br> 1 = 29-bit extended identifier
     */
    uint32_t esi;
    /*!< Error State Indicator
     *   <br> 0 = ESI bit in CAN FD format depends only on error passive flag
     *   <br> 1 = ESI bit in CAN FD format transmitted recessive
     */
    uint32_t dlc;
    /*!< Data Length Code
     *   <br> 0-8  = CAN + CAN FD: transmit frame has 0-8 data bytes
     *   <br> 9-15 = CAN: transmit frame has 8 data bytes
     *   <br> 9-15 = CAN FD: transmit frame has 12/16/20/24/32/48/64 data bytes
     */
    uint32_t brs;
    /*!< Bit Rate Switching
     *   <br> 0 = CAN FD frame transmitted without bit rate switching
     *   <br> 1 = CAN FD frame transmitted with bit rate switching
     */
    uint32_t fdf;
    /*!< FD Format
     *   <br> 0 = Frame transmitted in Classic CAN format
     *   <br> 1 = Frame transmitted in CAN FD format
     */
    uint32_t efc;
    /*!< Event FIFO Control
     *   <br> 0 = Don't store Tx events
     *   <br> 1 = Store Tx events
     */
    uint32_t mm;
    /*!< Message Marker - 8 bits */
} MCAN_TxBufElement;

/*!
 *  @brief  Structure for MCAN Tx Buffer element which takes data as a pointer.
 */
typedef struct
{
    uint32_t id;
    /*!< Identifier */
    uint32_t rtr;
    /*!< Remote Transmission Request
     *   <br> 0 = Transmit data frame
     *   <br> 1 = Transmit remote frame
     */
    uint32_t xtd;
    /*!< Extended Identifier
     *   <br> 0 = 11-bit standard identifier
     *   <br> 1 = 29-bit extended identifier
     */
    uint32_t esi;
    /*!< Error State Indicator
     *   <br> 0 = ESI bit in CAN FD format depends only on error passive flag
     *   <br> 1 = ESI bit in CAN FD format transmitted recessive
     */
    uint32_t dlc;
    /*!< Data Length Code
     *   <br> 0-8  = CAN + CAN FD: transmit frame has 0-8 data bytes
     *   <br> 9-15 = CAN: transmit frame has 8 data bytes
     *   <br> 9-15 = CAN FD: transmit frame has 12/16/20/24/32/48/64 data bytes
     */
    uint32_t brs;
    /*!< Bit Rate Switching
     *   <br> 0 = CAN FD frame transmitted without bit rate switching
     *   <br> 1 = CAN FD frame transmitted with bit rate switching
     */
    uint32_t fdf;
    /*!< FD Format
     *   <br> 0 = Frame transmitted in Classic CAN format
     *   <br> 1 = Frame transmitted in CAN FD format
     */
    uint32_t efc;
    /*!< Event FIFO Control
     *   <br> 0 = Don't store Tx events
     *   <br> 1 = Store Tx events
     */
    uint32_t mm;
    /*!< Message Marker - 8 bits */
    const uint8_t *data;
    /*!< Pointer to data. Must be word-aligned for best performance.
     *   Only the first DLC number of bytes will be transmitted.
     */
} MCAN_TxBufElementNoCpy;

/*!
 *  @brief  Structure for MCAN Rx Buffer element.
 */
typedef struct
{
    uint8_t data[MCAN_MAX_PAYLOAD_SIZE];
    /*!< Data bytes.
     *   Only first DLC number of bytes are valid.
     */
    uint32_t id;
    /*!< Identifier */
    uint16_t rxts;
    /*!< Rx Timestamp */
    uint8_t rtr;
    /*!< Remote Transmission Request
     *   <br> 0 = Received frame is a data frame
     *   <br> 1 = Received frame is a remote frame
     */
    uint8_t xtd;
    /*!< Extended Identifier
     *   <br> 0 = 11-bit standard identifier
     *   <br> 1 = 29-bit extended identifier
     */
    uint8_t esi;
    /*!< Error State Indicator
     *   <br> 0 = Transmitting node is error active
     *   <br> 1 = Transmitting node is error passive
     */
    uint8_t dlc;
    /*!< Data Length Code
     *   <br> 0-8  = CAN + CAN FD: received frame has 0-8 data bytes
     *   <br> 9-15 = CAN: received frame has 8 data bytes
     *   <br> 9-15 = CAN FD: received frame has 12/16/20/24/32/48/64 data bytes
     */
    uint8_t brs;
    /*!< Bit Rate Switching
     *   <br> 0 = Frame received without bit rate switching
     *   <br> 1 = Frame received with bit rate switching
     */
    uint8_t fdf;
    /*!< FD Format
     *   <br> 0 = Standard frame format
     *   <br> 1 = CAN FD frame format (new DLC-coding and CRC)
     */
    uint8_t fidx;
    /*!< Filter Index */
    uint8_t anmf;
    /*!< Accepted Non-matching Frame
     *   <br> 0 = Received frame matching filter index FIDX
     *   <br> 1 = Received frame did not match any Rx filter element
     */
} MCAN_RxBufElement;

/*!
 *  @brief  Structure for MCAN Rx Buffer element which takes data as a pointer.
 *          Please note that as the data field is a pointer so wrong value for
 *          payload size passed from application will lead to data corruption.
 */
typedef struct
{
    uint32_t id;
    /*!< Identifier */
    uint16_t rxts;
    /*!< Rx Timestamp */
    uint8_t rtr;
    /*!< Remote Transmission Request
     *   <br> 0 = Received frame is a data frame
     *   <br> 1 = Received frame is a remote frame
     */
    uint8_t xtd;
    /*!< Extended Identifier
     *   <br> 0 = 11-bit standard identifier
     *   <br> 1 = 29-bit extended identifier
     */
    uint8_t esi;
    /*!< Error State Indicator
     *   <br> 0 = Transmitting node is error active
     *   <br> 1 = Transmitting node is error passive
     */
    uint8_t dlc;
    /*!< Data Length Code
     *   <br> 0-8  = CAN + CAN FD: received frame has 0-8 data bytes
     *   <br> 9-15 = CAN: received frame has 8 data bytes
     *   <br> 9-15 = CAN FD: received frame has 12/16/20/24/32/48/64 data bytes
     */
    uint8_t brs;
    /*!< Bit Rate Switching
     *   <br> 0 = Frame received without bit rate switching
     *   <br> 1 = Frame received with bit rate switching
     */
    uint8_t fdf;
    /*!< FD Format
     *   <br> 0 = Standard frame format
     *   <br> 1 = CAN FD frame format (new DLC-coding and CRC)
     */
    uint8_t fidx;
    /*!< Filter Index */
    uint8_t anmf;
    /*!< Accepted Non-matching Frame
     *   <br> 0 = Received frame matching filter index FIDX
     *   <br> 1 = Received frame did not match any Rx filter element
     */
    uint8_t *data;
    /*!< Pointer to data. Must be word-aligned for best performance.
     *   Only first DLC number of bytes are valid.
     */
} MCAN_RxBufElementNoCpy;

/*!
 *  @brief  Structure for MCAN Tx Event FIFO element.
 */
typedef struct
{
    uint32_t id;
    /*!< Identifier */
    uint16_t txts;
    /*!< Tx Timestamp */
    uint8_t rtr;
    /*!< Remote Transmission Request
     *   <br> 0 = Data frame transmitted
     *   <br> 1 = Remote frame transmitted
     */
    uint8_t xtd;
    /*!< Extended Identifier
     *   <br> 0 = 11-bit standard identifier
     *   <br> 1 = 29-bit extended identifier
     */
    uint8_t esi;
    /*!< Error State Indicator
     *   <br> 0 = Transmitting node is error active
     *   <br> 1 = Transmitting node is error passive
     */
    uint8_t dlc;
    /*!< Data Length Code
     *   <br> 0-8  = CAN + CAN FD: frame with 0-8 data bytes transmitted
     *   <br> 9-15 = CAN: frame with 8 data bytes transmitted
     *   <br> 9-15 = CAN FD: frame with 12/16/20/24/32/48/64 data bytes transmitted
     */
    uint8_t brs;
    /*!< Bit Rate Switching
     *   <br> 0 = Frame transmitted without bit rate switching
     *   <br> 1 = Frame transmitted with bit rate switching
     */
    uint8_t fdf;
    /*!< FD Format
     *   <br> 0 = Standard frame format
     *   <br> 1 = CAN FD frame format (new DLC-coding and CRC)
     */
    uint8_t et;
    /*!< Event Type
     *   <br> 0 = Reserved
     *   <br> 1 = Tx event
     *   <br> 2 = Transmission in spite of cancellation
     *        (always set for transmissions in DAR mode)
     *   <br> 3 = Reserved
     */
    uint8_t mm;
    /*!< Message Marker */
} MCAN_TxEventFifoElement;

/*!
 *  @brief  Structure for MCAN Standard Message ID Filter Element.
 */
typedef struct
{
    uint32_t sfec;
    /*!< Standard Filter Element Configuration (3-bits)
     *   <br> 0 = Disable filter element
     *   <br> 1 = Store in Rx FIFO 0 if filter matches
     *   <br> 2 = Store in Rx FIFO 1 if filter matches
     *   <br> 3 = Reject ID if filter matches
     *   <br> 4 = Set priority if filter matches
     *   <br> 5 = Set priority and store in FIFO 0 if filter matches
     *   <br> 6 = Set priority and store in FIFO 1 if filter matches
     *   <br> 7 = Store into Rx Buffer or as debug message, configuration of
     *            SFT field is ignored as the ID must match SFID1 exactly.
     *   <br> If SFEC = 4-6, a match sets high priority
     *        message status and generates an interrupt.
     */
    uint32_t sft;
    /*!< Standard Filter Type (2-bits)
     *   <br> 0 = Range filter from SFID1 to SFID2 (SFID2 >= SFID1)
     *   <br> 1 = Dual ID filter for SFID1 or SFID2
     *   <br> 2 = Classic filter: SFID1 = filter, SFID2 = mask
     *   <br> 3 = Filter element disabled
     */
    uint32_t sfid1;
    /*!< Standard Filter ID 1 (11-bits).
     *   First ID of standard ID filter element.
     *   When filtering for Rx Buffers or for debug messages this field defines
     *   the ID of a standard message to be stored. The received identifiers
     *   must match exactly, no masking mechanism is used.
     */
    uint32_t sfid2;
    /*!< Standard Filter ID 2 (11-bits).
     *   This field has a different meaning depending on the value of SFEC:
     *   <br> SFEC = 1-6: Second ID of standard ID filter element
     *   <br> SFEC = 7: Filter for Rx Buffers or for debug messages
     *   <br>
     *   <br> SFID2[10:9] decides whether the received message is stored into
     *        an Rx Buffer or treated as message A, B, or C of the debug
     *        message sequence.
     *   <br> 0 = Store message into an Rx Buffer
     *   <br> 1 = Debug Message A
     *   <br> 2 = Debug Message B
     *   <br> 3 = Debug Message C
     *   <br>
     *   <br> SFID2[8:6] is used to control the filter event pins at the
     *        Extension Interface.
     *   <br>
     *   <br> SFID2[5:0] defines the offset to the Rx Buffer Start Address
     *        RXBC.RBSA for storage of a matching message.
     */
} MCAN_StdMsgIDFilterElement;

/*!
 *  @brief  Structure for MCAN Extended Message ID Filter Element.
 */
typedef struct
{
    uint32_t efec;
    /*!< Extended Filter Element Configuration
     *   <br> 0 = Disable filter element
     *   <br> 1 = Store in Rx FIFO 0 if filter matches
     *   <br> 2 = Store in Rx FIFO 1 if filter matches
     *   <br> 3 = Reject ID if filter matches
     *   <br> 4 = Set priority if filter matches
     *   <br> 5 = Set priority and store in FIFO 0 if filter matches
     *   <br> 6 = Set priority and store in FIFO 1 if filter matches
     *   <br> 7 = Store into Rx Buffer or as debug message, configuration of
     *            EFT field is ignored as the ID must match EFID1 exactly.
     *   <br> If EFEC = 4-6, a match sets high priority
     *        message status and generates an interrupt.
     */
    uint32_t eft;
    /*!< Extended Filter Type
     *   <br> 0 = Range filter from EFID1 to EFID2 (EFID2 >= EFID1)
     *   <br> 1 = Dual ID filter for EFID1 or EFID2
     *   <br> 2 = Classic filter: EFID1 = filter, EFID2 = mask
     *   <br> 3 = Range filter from EFID1 to EFID2 (EFID2 >= EFID1),
     *            XIDAM mask not applied
     */
    uint32_t efid1;
    /*!< Extended Filter ID 1 (29-bits).
     *   First ID of extended ID filter element.
     *   When filtering for Rx Buffers or for debug messages this field defines
     *   the ID of a extended message to be stored. The received identifiers
     *   must match exactly, no masking mechanism is used.
     */
    uint32_t efid2;
    /*!< Extended Filter ID 2 (29-bits)
     *   This field has a different meaning depending on the value of EFEC:
     *   <br> EFEC = 1-6: Second ID of extended ID filter element
     *   <br> EFEC = 7: Filter for Rx Buffers or for debug messages
     *   <br>
     *   <br> EFID2[10:9] decides whether the received message is stored into
     *        an Rx Buffer or treated as message A, B, or C of the debug
     *        message sequence.
     *   <br> 0 = Store message into an Rx Buffer
     *   <br> 1 = Debug Message A
     *   <br> 2 = Debug Message B
     *   <br> 3 = Debug Message C
     *   <br>
     *   <br> EFID2[8:6] is used to control the filter event pins at the
     *        Extension Interface.
     *   <br>
     *   <br> EFID2[5:0] defines the offset to the Rx Buffer Start Address
     *        RXBC.RBSA for storage of a matching message. */
} MCAN_ExtMsgIDFilterElement;

/*!
 *  @brief   Initializes M_CAN controller.
 *
 *  @param   initParams      Pointer to initialization parameters.
 *                           Refer struct MCAN_InitParams.
 *
 *  @retval  MCAN_STATUS_SUCCESS if successful.
 *  @retval  MCAN_STATUS_ERROR if config has invalid time delay compensation or WDT preload.
 */
int_fast16_t MCAN_init(const MCAN_InitParams *initParams);

/*!
 *  @brief   Configures M_CAN controller.
 *
 *  @param   config          Pointer to configuration parameters.
 *                           Refer struct MCAN_ConfigParams.
 *
 *  @retval  MCAN_STATUS_SUCCESS if successful.
 *  @retval  MCAN_STATUS_ERROR if config has invalid prescaler or timeout preload.
 */
int_fast16_t MCAN_config(const MCAN_ConfigParams *config);

/*!
 *  @brief   Sets M_CAN controller mode of operation.
 *
 *  @param   mode            Mode of operation.
 *                           Refer enum MCAN_OperationMode.
 *
 *  @return  None.
 */
void MCAN_setOpMode(MCAN_OperationMode mode);

/*!
 *  @brief   Returns M_CAN controller mode of operation.
 *
 *
 *  @return  mode            Mode of operation.
 *                           Refer enum MCAN_OperationMode.
 */
MCAN_OperationMode MCAN_getOpMode(void);

/*!
 *  @brief   Reads the M_CAN controller bit timings.
 *
 *  @param   bitTiming       Pointer to MCAN bit timing parameters.
 *                           Refer struct MCAN_BitTimingParams.
 *
 *  @return  None.
 */
void MCAN_getBitTime(MCAN_BitTimingParams *bitTiming);

/*!
 *  @brief   Sets M_CAN controller bit timings.
 *
 *  @param   bitTiming       Pointer to MCAN bit timing parameters.
 *                           Refer struct MCAN_BitTimingParams.
 *
 *  @retval  MCAN_STATUS_SUCCESS if successful.
 *  @retval  MCAN_STATUS_ERROR if config has invalid timing values.
 */
int_fast16_t MCAN_setBitTime(const MCAN_BitTimingParams *bitTiming);

/*!
 *  @brief   Configures the various sections of Message RAM.
 *
 *  @warning Message RAM configuration is not validated by this API.
 *
 *  @param   msgRamConfig    Pointer to Message RAM configuration.
 *                           Refer struct MCAN_MsgRamConfig.
 *
 *  @return  None.
 */
void MCAN_configMsgRam(const MCAN_MsgRamConfig *msgRamConfig);

/*!
 *  @brief   Writes Tx message to message RAM.
 *
 *  @param   bufIdx          Tx Buffer index [0-31] where to write message. Must be valid per
 *                           the message RAM configuration.
 *  @param   elem            Pointer to Tx element.
 *                           Refer struct MCAN_TxBufElementNoCpy.
 *
 *  @return  None.
 *
 *  @post    #MCAN_setTxBufAddReq()
 *
 *  @sa      #MCAN_writeTxMsg()
 */
void MCAN_writeTxMsgNoCpy(uint32_t bufIdx, const MCAN_TxBufElementNoCpy *elem);

/*!
 *  @brief   Writes Tx message to message RAM.
 *
 *  @param   bufIdx          Tx Buffer index [0-31] where to write message. Must be valid per
 *                           the message RAM configuration.
 *  @param   elem            Pointer to Tx element.
 *                           Refer struct MCAN_TxBufElement.
 *
 *  @return  None.
 *
 *  @post    #MCAN_setTxBufAddReq()
 *
 *  @sa      #MCAN_writeTxMsgNoCpy()
 */
void MCAN_writeTxMsg(uint32_t bufIdx, const MCAN_TxBufElement *elem);

/*!
 *  @brief   Sets Tx Buffer Add Request.
 *
 *  @pre     #MCAN_writeTxMsgNoCpy() or #MCAN_writeTxMsg()
 *
 *  @param   bufIdx          Tx Buffer index [0-31] for which request is to be added.
 *                           Must be valid per the message RAM configuration.
 *
 *  @return  None.
 */
void MCAN_setTxBufAddReq(uint32_t bufIdx);

/*!
 *  @brief   Reads New Data Message Status.
 *
 *  @param   newDataStatus   Pointer to Rx Buffer new data status.
 *                           Refer struct MCAN_RxNewDataStatus.
 *
 *  @return  None.
 *
 *  @post    #MCAN_clearNewDataStatus()
 */
void MCAN_getNewDataStatus(MCAN_RxNewDataStatus *newDataStatus);

/*!
 *  @brief   Clears New Data Message Status.
 *
 *  @pre     #MCAN_getNewDataStatus()
 *
 *  @param   newDataStatus   Pointer to Rx Buffer new data status.
 *                           Refer struct MCAN_RxNewDataStatus.
 *
 *  @return  None.
 */
void MCAN_clearNewDataStatus(const MCAN_RxNewDataStatus *newDataStatus);

/*!
 *  @brief   Reads received message from message RAM.
 *
 *  This function uses the MCAN_RxBufElementNoCpy structure element which has
 *  data as pointer instead of an array. Note that as the data is a pointer here
 *  hence corruption of data is possible when the payload size is exceeded.
 *
 *  @param   memType         Part of message ram to which given message to write.
 *                           Refer enum MCAN_MemType.
 *  @param   num             Buffer number or FIFO number from where message is to read.
 *                           Refer enum MCAN_RxFifoNum if FIFO number. Must be valid per
 *                           the message RAM configuration.
 *  @param   elem            Pointer to Rx element.
 *                           Refer struct MCAN_RxBufElementNoCpy.
 *
 *  @return  None.
 *
 *  @post    #MCAN_setRxFifoAck()
 *
 *  @sa      #MCAN_readRxMsg()
 */
void MCAN_readRxMsgNoCpy(MCAN_MemType memType, uint32_t num, MCAN_RxBufElementNoCpy *elem);

/*!
 *  @brief   Reads received message from message RAM.
 *
 *  @param   memType         Part of message ram to which given message to write.
 *                           Refer enum MCAN_MemType.
 *  @param   num             Buffer number or FIFO number from where message is to read.
 *                           Refer enum MCAN_RxFifoNum if FIFO number. Must be valid per
 *                           the message RAM configuration.
 *  @param   elem            Pointer to Rx element.
 *                           Refer struct MCAN_RxBufElement.
 *
 *  @return  None.
 *
 *  @post    #MCAN_setRxFifoAck()
 *
 *  @sa      #MCAN_readRxMsgNoCpy()
 */
void MCAN_readRxMsg(MCAN_MemType memType, uint32_t num, MCAN_RxBufElement *elem);

/*!
 *  @brief   Reads next available element from Tx Event FIFO.
 *
 *  This function writes the Tx Event FIFO acknowledge index after successfully
 *  reading the Tx Event FIFO element.
 *
 *  @param   elem            Pointer to Tx Event FIFO Element Object.
 *                           Refer struct MCAN_TxEventFifoElement.
 *
 *  @retval  MCAN_STATUS_SUCCESS if successful.
 *  @retval  MCAN_STATUS_ERROR if the Tx Event FIFO is empty.
 */
int_fast16_t MCAN_readTxEventFifo(MCAN_TxEventFifoElement *elem);

/*!
 *  @brief   Adds Standard Message ID Filter Element.
 *
 *  @param   filtNum         Filter number (0-based).
 *  @param   elem            Pointer to standard ID Filter Object.
 *                           Refer struct MCAN_StdMsgIDFilterElement.
 *
 *  @return  None.
 */
void MCAN_addStdMsgIDFilter(uint32_t filtNum, const MCAN_StdMsgIDFilterElement *elem);

/*!
 *  @brief   Adds Extended Message ID Filter Element.
 *
 *  @param   filtNum         Filter number (0-based).
 *  @param   elem            Pointer to extended ID Filter Object.
 *                           Refer struct MCAN_ExtMsgIDFilterElement.
 *
 *  @return  None.
 */
void MCAN_addExtMsgIDFilter(uint32_t filtNum, const MCAN_ExtMsgIDFilterElement *elem);

/*!
 *  @brief   Enables Loopback Test Mode for M_CAN controller.
 *
 *  @param   lpbkMode        Loopback mode for MCAN.
 *                           Refer enum MCAN_LpbkMode.
 *
 *  @return  None.
 *
 *  @note    This API can be called only when MCAN is in Software
 *           Initialization mode of operation.
 */
void MCAN_enableLoopbackMode(MCAN_LpbkMode lpbkMode);

/*!
 *  @brief   Disables Loopback Test Mode for M_CAN controller.
 *
 *  @pre     #MCAN_enableLoopbackMode()
 *
 *  @return  None.
 *
 *  @note    This API can be called only when MCAN is in Software
 *           Initialization mode of operation.
 */
void MCAN_disableLoopbackMode(void);

/*!
 *  @brief   Reads protocol status for M_CAN controller.
 *
 *  @param   protStatus      Pointer to Protocol Status.
 *                           Refer struct MCAN_ProtocolStatus.
 *
 *  @return  None.
 */
void MCAN_getProtocolStatus(MCAN_ProtocolStatus *protStatus);

/*!
 *  @brief   Enables interrupts.
 *
 *  @param   intMask         Bit mask of interrupts to enable.
 *                           Refer enum MCAN_IntSrc.
 *
 *  @return  None.
 */
void MCAN_enableInt(uint32_t intMask);

/*!
 *  @brief   Disables interrupts.
 *
 *  @param   intMask         Bit mask of interrupts to disable.
 *                           Refer enum MCAN_IntSrc.
 *
 *  @return  None.
 */
void MCAN_disableInt(uint32_t intMask);

/*!
 *  @brief   Sets interrupt line select.
 *
 *  @param   intMask         Interrupt Number for which interrupt
 *                           line is to be selected. Refer enum MCAN_IntSrc.
 *  @param   lineNum         Interrupt Line to select.
 *                           Refer enum MCAN_IntLineNum.
 *
 *  @return  None.
 */
void MCAN_setIntLineSel(uint32_t intMask, MCAN_IntLineNum lineNum);

/*!
 *  @brief   Enables selected interrupt line.
 *
 *  @param   lineNum         Interrupt Line to enable.
 *                           Refer enum MCAN_IntLineNum.
 *
 *  @return  None.
 */
void MCAN_enableIntLine(MCAN_IntLineNum lineNum);

/*!
 *  @brief   Disables selected interrupt line.
 *
 *  @param   lineNum         Interrupt Line to disable.
 *                           Refer enum MCAN_IntLineNum.
 *
 *  @return  None.
 */
void MCAN_disableIntLine(MCAN_IntLineNum lineNum);

/*!
 *  @brief   Returns interrupt status.
 *
 *  @return  Interrupt Status.
 */
uint32_t MCAN_getIntStatus(void);

/*!
 *  @brief   Clears the interrupt status.
 *
 *  @param   intMask         Interrupts to clear status.
 *
 *  @return  None.
 */
void MCAN_clearIntStatus(uint32_t intMask);

/*!
 *  @brief   Reads Rx FIFO status.
 *
 *  @param   fifoNum         Rx FIFO number
 *                           Refer enum MCAN_RxFifoNum
 *  @param   fifoStatus      Rx FIFO Status.
 *                           Refer struct MCAN_RxFifoStatus.
 *
 *  @return  None.
 */
void MCAN_getRxFifoStatus(MCAN_RxFifoNum fifoNum, MCAN_RxFifoStatus *fifoStatus);

/*!
 *  @brief   Reads Tx Event FIFO status.
 *
 *  @param   fifoStatus      Pointer to Tx Event FIFO Status.
 *                           Refer struct TxEventFifoStatus.
 *
 *  @return  None.
 */
void MCAN_getTxEventFifoStatus(MCAN_TxEventFifoStatus *fifoStatus);

/*!
 *  @brief   Reads Tx FIFO/Queue status.
 *
 *  @param   fifoQStatus     Pointer to Tx FIFO/Queue Status.
 *                           Refer struct MCAN_TxFifoQStatus.
 *
 *  @return  None.
 */
void MCAN_getTxFifoQStatus(MCAN_TxFifoQStatus *fifoQStatus);

/*!
 *  @brief   Sets Rx FIFO Acknowledgement.
 *
 *  @param   fifoNum         FIFO Number.
 *                           Refer enum MCAN_RxFifoNum.
 *  @param   idx             Rx FIFO Acknowledge Index
 *
 *  @retval  MCAN_STATUS_SUCCESS if successful.
 *  @retval  MCAN_STATUS_ERROR if \c fifoNum or \c idx is invalid.
 */
int_fast16_t MCAN_setRxFifoAck(MCAN_RxFifoNum fifoNum, uint32_t idx);

/*!
 *  @brief   Returns Tx Buffer Request Pending status.
 *
 *  @return  Tx Buffer Request Pending status.
 */
uint32_t MCAN_getTxBufReqPend(void);

/*!
 *  @brief   Cancels a Tx Buffer Request.
 *
 *  @param   bufIdx         Tx Buffer index [0-31] for which request is to be cancelled.
 *
 *  @return  None.
 */
void MCAN_cancelTxBufReq(uint32_t bufIdx);

/*!
 *  @brief   Returns Tx Buffer Transmission Occurred status.
 *
 *  @return  Tx Buffer Transmission Occurred status.
 */
uint32_t MCAN_getTxBufTransmissionStatus(void);

/*!
 *  @brief   Returns Transmit Buffer Cancellation Finished status.
 *
 *  @return  Transmit Buffer Cancellation Finished status.
 */
uint32_t MCAN_getTxBufCancellationStatus(void);

/*!
 *  @brief   Enables Tx Buffer Transmission Interrupt.
 *
 *  @param   bufMask         Tx Buffer mask for which interrupt(s) to enable.
 *
 *  @return  None.
 */
void MCAN_enableTxBufTransInt(uint32_t bufMask);

/*!
 *  @brief   Disables Tx Buffer Transmission Interrupt.
 *
 *  @param   bufMask         Tx Buffer mask for which interrupt(s) to disable.
 *
 *  @return  None.
 */
void MCAN_disableTxBufTransInt(uint32_t bufMask);

/*!
 *  @brief   Returns clock stop acknowledgement for M_CAN controller.
 *
 *  @return  Clock Stop Acknowledge:
 *           <br> 0 = No clock stop acknowledged,
 *           <br> 1 = M_CAN may be set in power down
 */
uint32_t MCAN_getClkStopAck(void);

/*!
 *  @brief   Returns the 16-bit timestamp counter value.
 *
 *  @return  Timestamp counter value
 */
uint16_t MCAN_getTimestampCounter(void);

#ifdef __cplusplus
}
#endif

#endif /* __MCAN_H__ */
