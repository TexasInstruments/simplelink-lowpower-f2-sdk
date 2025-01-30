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

/*
 *  ======== MCAN.c ========
 */
#include <stdint.h>

#include <third_party/mcan/MCAN.h>
#include <third_party/mcan/inc/MCAN_reg.h>

/*
 * Max values for various register fields.
 */
#define MCAN_DBTP_DSJW_MAX   (0xFU)
#define MCAN_DBTP_DTSEG2_MAX (0xFU)
#define MCAN_DBTP_DTSEG1_MAX (0x1FU)
#define MCAN_DBTP_DBRP_MAX   (0x1FU)

#define MCAN_NBTP_NSJW_MAX   (0x7FU)
#define MCAN_NBTP_NTSEG2_MAX (0x7FU)
#define MCAN_NBTP_NTSEG1_MAX (0xFFU)
#define MCAN_NBTP_NBRP_MAX   (0x1FFU)

#define MCAN_RWD_WDC_MAX (0xFFU)

#define MCAN_TDCR_TDCF_MAX (0x7FU)
#define MCAN_TDCR_TDCO_MAX (0x7FU)

#define MCAN_TSCC_TCP_MAX (0xFU)
#define MCAN_TOCC_TOP_MAX (0xFFFFU)

/*
 * Mask and shift for Tx Buffers elements.
 */
/* Tx Buffer Element Word 0 */
#define MCAN_TX_BUFFER_ELEM_XID_SHIFT (0U)
#define MCAN_TX_BUFFER_ELEM_XID_MASK  (0x1FFFFFFFU)
#define MCAN_TX_BUFFER_ELEM_SID_SHIFT (18U)
#define MCAN_TX_BUFFER_ELEM_SID_MASK  (0x1FFC0000U)
#define MCAN_TX_BUFFER_ELEM_RTR_SHIFT (29U)
#define MCAN_TX_BUFFER_ELEM_RTR_MASK  (0x20000000U)
#define MCAN_TX_BUFFER_ELEM_XTD_SHIFT (30U)
#define MCAN_TX_BUFFER_ELEM_XTD_MASK  (0x40000000U)
#define MCAN_TX_BUFFER_ELEM_ESI_SHIFT (31U)
#define MCAN_TX_BUFFER_ELEM_ESI_MASK  (0x80000000U)
/* Tx Buffer Element Word 1 */
#define MCAN_TX_BUFFER_ELEM_DLC_SHIFT (16U)
#define MCAN_TX_BUFFER_ELEM_DLC_MASK  (0x000F0000U)
#define MCAN_TX_BUFFER_ELEM_BRS_SHIFT (20U)
#define MCAN_TX_BUFFER_ELEM_BRS_MASK  (0x00100000U)
#define MCAN_TX_BUFFER_ELEM_FDF_SHIFT (21U)
#define MCAN_TX_BUFFER_ELEM_FDF_MASK  (0x00200000U)
#define MCAN_TX_BUFFER_ELEM_EFC_SHIFT (23U)
#define MCAN_TX_BUFFER_ELEM_EFC_MASK  (0x00800000U)
#define MCAN_TX_BUFFER_ELEM_MM_SHIFT  (24U)
#define MCAN_TX_BUFFER_ELEM_MM_MASK   (0xFF000000U)

/*
 * Mask and shift for Rx Buffers elements.
 */
/* Rx Buffer Element Word 0 */
#define MCAN_RX_BUFFER_ELEM_XID_SHIFT  (0U)
#define MCAN_RX_BUFFER_ELEM_XID_MASK   (0x1FFFFFFFU)
#define MCAN_RX_BUFFER_ELEM_SID_SHIFT  (18U)
#define MCAN_RX_BUFFER_ELEM_SID_MASK   (0x1FFC0000U)
#define MCAN_RX_BUFFER_ELEM_RTR_SHIFT  (29U)
#define MCAN_RX_BUFFER_ELEM_RTR_MASK   (0x20000000U)
#define MCAN_RX_BUFFER_ELEM_XTD_SHIFT  (30U)
#define MCAN_RX_BUFFER_ELEM_XTD_MASK   (0x40000000U)
#define MCAN_RX_BUFFER_ELEM_ESI_SHIFT  (31U)
#define MCAN_RX_BUFFER_ELEM_ESI_MASK   (0x80000000U)
/* Rx Buffer Element Word 1 */
#define MCAN_RX_BUFFER_ELEM_RXTS_SHIFT (0U)
#define MCAN_RX_BUFFER_ELEM_RXTS_MASK  (0x0000FFFFU)
#define MCAN_RX_BUFFER_ELEM_DLC_SHIFT  (16U)
#define MCAN_RX_BUFFER_ELEM_DLC_MASK   (0x000F0000U)
#define MCAN_RX_BUFFER_ELEM_BRS_SHIFT  (20U)
#define MCAN_RX_BUFFER_ELEM_BRS_MASK   (0x00100000U)
#define MCAN_RX_BUFFER_ELEM_FDF_SHIFT  (21U)
#define MCAN_RX_BUFFER_ELEM_FDF_MASK   (0x00200000U)
#define MCAN_RX_BUFFER_ELEM_FIDX_SHIFT (24U)
#define MCAN_RX_BUFFER_ELEM_FIDX_MASK  (0x7F000000U)
#define MCAN_RX_BUFFER_ELEM_ANMF_SHIFT (31U)
#define MCAN_RX_BUFFER_ELEM_ANMF_MASK  (0x80000000U)

/*
 * Mask and shift for Standard Message ID Filter Elements.
 */
#define MCAN_STD_ID_FILTER_SFID2_SHIFT (0U)
#define MCAN_STD_ID_FILTER_SFID2_MASK  (0x000003FFU)
#define MCAN_STD_ID_FILTER_SFID1_SHIFT (16U)
#define MCAN_STD_ID_FILTER_SFID1_MASK  (0x03FF0000U)
#define MCAN_STD_ID_FILTER_SFEC_SHIFT  (27U)
#define MCAN_STD_ID_FILTER_SFEC_MASK   (0x38000000U)
#define MCAN_STD_ID_FILTER_SFT_SHIFT   (30U)
#define MCAN_STD_ID_FILTER_SFT_MASK    (0xC0000000U)

/*
 * Extended Message ID Filter Element.
 */
#define MCAN_EXT_ID_FILTER_EFID2_SHIFT (0U)
#define MCAN_EXT_ID_FILTER_EFID2_MASK  (0x1FFFFFFFU)
#define MCAN_EXT_ID_FILTER_EFID1_SHIFT (0U)
#define MCAN_EXT_ID_FILTER_EFID1_MASK  (0x1FFFFFFFU)
#define MCAN_EXT_ID_FILTER_EFEC_SHIFT  (29U)
#define MCAN_EXT_ID_FILTER_EFEC_MASK   (0xE0000000U)
#define MCAN_EXT_ID_FILTER_EFT_SHIFT   (30U)
#define MCAN_EXT_ID_FILTER_EFT_MASK    (0xC0000000U)

/*
 * Mask and shift for Tx Event FIFO elements.
 */
#define MCAN_TX_EVENT_FIFO_ELEM_ID_SHIFT  (0U)
#define MCAN_TX_EVENT_FIFO_ELEM_ID_MASK   (0x1FFFFFFFU)
#define MCAN_TX_EVENT_FIFO_ELEM_RTR_SHIFT (29U)
#define MCAN_TX_EVENT_FIFO_ELEM_RTR_MASK  (0x20000000U)
#define MCAN_TX_EVENT_FIFO_ELEM_XTD_SHIFT (30U)
#define MCAN_TX_EVENT_FIFO_ELEM_XTD_MASK  (0x40000000U)
#define MCAN_TX_EVENT_FIFO_ELEM_ESI_SHIFT (31U)
#define MCAN_TX_EVENT_FIFO_ELEM_ESI_MASK  (0x80000000U)

#define MCAN_TX_EVENT_FIFO_ELEM_TXTS_SHIFT (0U)
#define MCAN_TX_EVENT_FIFO_ELEM_TXTS_MASK  (0x0000FFFFU)
#define MCAN_TX_EVENT_FIFO_ELEM_DLC_SHIFT  (16U)
#define MCAN_TX_EVENT_FIFO_ELEM_DLC_MASK   (0x000F0000U)
#define MCAN_TX_EVENT_FIFO_ELEM_BRS_SHIFT  (20U)
#define MCAN_TX_EVENT_FIFO_ELEM_BRS_MASK   (0x00100000U)
#define MCAN_TX_EVENT_FIFO_ELEM_FDF_SHIFT  (21U)
#define MCAN_TX_EVENT_FIFO_ELEM_FDF_MASK   (0x00200000U)
#define MCAN_TX_EVENT_FIFO_ELEM_ET_SHIFT   (22U)
#define MCAN_TX_EVENT_FIFO_ELEM_ET_MASK    (0x00C00000U)
#define MCAN_TX_EVENT_FIFO_ELEM_MM_SHIFT   (24U)
#define MCAN_TX_EVENT_FIFO_ELEM_MM_MASK    (0xFF000000U)

/* Start Address bit shift for any MCAN registers containing a word-aligned
 * start address field.
 */
#define MCAN_START_ADDR_SHIFT (2U)

/*!
 *  @brief Macro to extract a field value. This macro extracts the field value
 *         from a 32-bit variable (which contains the register value).
 *         This macro does not read from actual register address, and only
 *         extracts the field from a variable.
 *
 *  @param regVal         32-bit variable containing the register value.
 *  @param REG_FIELD      Peripheral register bit field name, whose value has to
 *                        be extracted.
 */
#define MCAN_GET_FIELD(regVal, REG_FIELD) (((regVal) & (uint32_t)REG_FIELD##_MASK) >> (uint32_t)REG_FIELD##_SHIFT)

/*!
 *  @brief Macro to set a specific field value. This macro first clears the
 *         specified field value and then performs "OR" of the field value which
 *         is shifted and masked. This will set the field value at its
 *         desired position.
 *
 *  @param regVal         32-bit variable containing the register value.
 *  @param REG_FIELD      Peripheral register bit field name, to which specified
 *                        value has to be set.
 *  @param fieldVal       Value of the field which has to be set.
 */
#define MCAN_SET_FIELD(regVal, REG_FIELD, fieldVal)                    \
    ((regVal) = ((regVal) & (uint32_t)(~(uint32_t)REG_FIELD##_MASK)) | \
                ((((uint32_t)(fieldVal)) << (uint32_t)REG_FIELD##_SHIFT) & (uint32_t)REG_FIELD##_MASK))

/*!
 *  @brief This macro calls read field API for 32 bit register. It also
 *         frames the mask and shift from register field macro.
 *
 *  @param offset         Register address offset.
 *  @param REG_FIELD      Peripheral register bit field name, from which
 *                        specified bit-field value has to be read.
 *  @return Value of the bit-field
 */
#define MCAN_READ_FIELD(offset, REG_FIELD) \
    (MCAN_read_field_raw((uint32_t)(offset), ((uint32_t)REG_FIELD##_MASK), ((uint32_t)REG_FIELD##_SHIFT)))

/*!
 *  @brief This macro calls read-modify-write API for 32 bit register. It also
 *         frames the mask and shift from register field macro.
 *
 *  @param offset         Register address offset.
 *  @param REG_FIELD      Peripheral register bit field name, to which specified
 *                        value has to be set.
 *  @param fieldVal       Value of the field which has to be set.
 */
#define MCAN_MODIFY_FIELD(offset, REG_FIELD, fieldVal)    \
    (MCAN_modify_field_raw((uint32_t)(offset),            \
                           ((uint32_t)REG_FIELD##_MASK),  \
                           ((uint32_t)REG_FIELD##_SHIFT), \
                           (uint32_t)(fieldVal)))

static inline void MCAN_modify_field_raw(uint32_t offset, uint32_t mask, uint32_t shift, uint32_t value);
static inline uint32_t MCAN_read_field_raw(uint32_t offset, uint32_t mask, uint32_t shift);
static inline void MCAN_enableConfigChange(void);
static inline void MCAN_disableConfigChange(void);
static void MCAN_readMsgNoCpy(uint32_t elemAddr, MCAN_RxBufElementNoCpy *elem);
static void MCAN_readMsg(uint32_t elemAddr, MCAN_RxBufElement *elem);
static void MCAN_writeMsgNoCpy(uint32_t elemAddr, const MCAN_TxBufElementNoCpy *elem);
static void MCAN_writeMsg(uint32_t elemAddr, const MCAN_TxBufElement *elem);

/*! Payload bytes indexed by Data Length Code (DLC) field. */
static const size_t MCAN_dataSize[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};

/*! Element Size (RAM words) indexed by TXESC.TBDS, RXESC.RBDS or RXESC.FnDS */
static const uint32_t MCAN_elementSizeWords[8] = {4, 5, 6, 7, 8, 10, 14, 18};

/*!
 *  @brief   This function reads a 32 bit register, modifies specific set of
 *           bits and writes back to the register.
 *
 *  @param   offset  Register address offset.
 *  @param   mask    Mask for the bit field.
 *  @param   shift   Bit field shift from LSB.
 *  @param   value   Value to be written to bit-field.
 *
 *  @note    Use MCAN_MODIFY_FIELD macro instead of this function directly
 *           to reduce static analysis violations.
 */
static inline void MCAN_modify_field_raw(uint32_t offset, uint32_t mask, uint32_t shift, uint32_t value)
{
    uint32_t regVal = MCAN_readReg(offset);
    regVal &= (uint32_t)~mask;
    regVal |= (value << shift) & mask;
    MCAN_writeReg(offset, regVal);
}

/*!
 *  @brief   This function reads a 32 bit register, masks specific set of bits
 *           and right shifts the value to LSB position.
 *
 *  @param   offset  Register address offset.
 *  @param   mask    Mask for the bit field.
 *  @param   shift   Bit field shift from LSB.
 *
 *  @return  Bit-field value (absolute value shifted to LSB position)
 *
 *  @note    Use MCAN_READ_FIELD macro instead of this function directly
 *           to reduce static analysis violations.
 */
static inline uint32_t MCAN_read_field_raw(uint32_t offset, uint32_t mask, uint32_t shift)
{
    uint32_t regVal = MCAN_readReg(offset);
    regVal          = (regVal & mask) >> shift;
    return (regVal);
}

/*
 *  ======== MCAN_enableConfigChange ========
 */
static inline void MCAN_enableConfigChange(void)
{
    uint32_t regVal;

    regVal = MCAN_readReg(MCAN_CCCR);

    /* Clock stop request must be cleared when written in Standby mode */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_CSR, 0U);

    MCAN_SET_FIELD(regVal, MCAN_CCCR_CCE, 1U);

    MCAN_writeReg(MCAN_CCCR, regVal);
}

/*
 *  ======== MCAN_disableConfigChange ========
 */
static inline void MCAN_disableConfigChange(void)
{
    uint32_t regVal;

    regVal = MCAN_readReg(MCAN_CCCR);

    /* Clock stop request must be cleared when written in Standby mode */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_CSR, 0U);

    MCAN_SET_FIELD(regVal, MCAN_CCCR_CCE, 0U);

    MCAN_writeReg(MCAN_CCCR, regVal);
}

/*
 *  ======== MCAN_readMsgNoCpy ========
 */
static void MCAN_readMsgNoCpy(uint32_t elemAddr, MCAN_RxBufElementNoCpy *elem)
{
    size_t dataSize;
    uint32_t regVal;
    uint32_t tempElemAddr = elemAddr;

    regVal    = MCAN_readReg(tempElemAddr);
    elem->rtr = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_RTR);
    elem->xtd = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_XTD);
    elem->esi = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_ESI);

    if (0U != elem->xtd)
    {
        elem->id = MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_XID);
    }
    else
    {
        elem->id = MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_SID);
    }

    tempElemAddr += 4U;

    regVal     = MCAN_readReg(tempElemAddr);
    elem->rxts = (uint16_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_RXTS);
    elem->dlc  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_DLC);
    elem->brs  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_BRS);
    elem->fdf  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_FDF);
    elem->fidx = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_FIDX);
    elem->anmf = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_ANMF);

    tempElemAddr += 4U;

    dataSize = MCAN_dataSize[elem->dlc];

    MCAN_readMsgRam(elem->data, tempElemAddr, dataSize);
}

/*
 *  ======== MCAN_readMsg ========
 */
static void MCAN_readMsg(uint32_t elemAddr, MCAN_RxBufElement *elem)
{
    size_t dataSize;
    uint32_t regVal;
    uint32_t tempElemAddr = elemAddr;

    regVal    = MCAN_readReg(tempElemAddr);
    elem->rtr = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_RTR);
    elem->xtd = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_XTD);
    elem->esi = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_ESI);

    if (0U != elem->xtd)
    {
        elem->id = MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_XID);
    }
    else
    {
        elem->id = MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_SID);
    }

    tempElemAddr += 4U;

    regVal     = MCAN_readReg(tempElemAddr);
    elem->rxts = (uint16_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_RXTS);
    elem->dlc  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_DLC);
    elem->brs  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_BRS);
    elem->fdf  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_FDF);
    elem->fidx = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_FIDX);
    elem->anmf = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_RX_BUFFER_ELEM_ANMF);

    tempElemAddr += 4U;

    dataSize = MCAN_dataSize[elem->dlc];

    MCAN_readMsgRam(elem->data, tempElemAddr, dataSize);
}

/*
 *  ======== MCAN_writeMsgNoCpy ========
 */
static void MCAN_writeMsgNoCpy(uint32_t elemAddr, const MCAN_TxBufElementNoCpy *elem)
{
    size_t dataSize;
    uint32_t regVal;
    uint32_t tempElemAddr = elemAddr;

    regVal = 0U;
    if (0U != elem->xtd)
    {
        MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_XID, elem->id);
    }
    else
    {
        MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_SID, elem->id);
    }
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_RTR, elem->rtr);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_XTD, elem->xtd);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_ESI, elem->esi);
    MCAN_writeReg(tempElemAddr, regVal);
    tempElemAddr += 4U;

    regVal = 0U;
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_DLC, elem->dlc);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_BRS, elem->brs);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_FDF, elem->fdf);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_EFC, elem->efc);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_MM, elem->mm);
    MCAN_writeReg(tempElemAddr, regVal);
    tempElemAddr += 4U;

    dataSize = MCAN_dataSize[elem->dlc];

    MCAN_writeMsgRam(tempElemAddr, elem->data, dataSize);
}

/*
 *  ======== MCAN_writeMsg ========
 */
static void MCAN_writeMsg(uint32_t elemAddr, const MCAN_TxBufElement *elem)
{
    size_t dataSize;
    uint32_t regVal;
    uint32_t tempElemAddr = elemAddr;

    regVal = 0U;
    if (0U != elem->xtd)
    {
        MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_XID, elem->id);
    }
    else
    {
        MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_SID, elem->id);
    }
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_RTR, elem->rtr);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_XTD, elem->xtd);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_ESI, elem->esi);
    MCAN_writeReg(tempElemAddr, regVal);
    tempElemAddr += 4U;

    regVal = 0U;
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_DLC, elem->dlc);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_BRS, elem->brs);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_FDF, elem->fdf);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_EFC, elem->efc);
    MCAN_SET_FIELD(regVal, MCAN_TX_BUFFER_ELEM_MM, elem->mm);
    MCAN_writeReg(tempElemAddr, regVal);
    tempElemAddr += 4U;

    dataSize = MCAN_dataSize[elem->dlc];

    MCAN_writeMsgRam(tempElemAddr, elem->data, dataSize);
}

/*
 *  ======== MCAN_setOpMode ========
 */
void MCAN_setOpMode(MCAN_OperationMode mode)
{
    MCAN_MODIFY_FIELD(MCAN_CCCR, MCAN_CCCR_INIT, mode);
}

/*
 *  ======== MCAN_getOpMode ========
 */
MCAN_OperationMode MCAN_getOpMode(void)
{
    return (MCAN_READ_FIELD(MCAN_CCCR, MCAN_CCCR_INIT));
}

/*
 *  ======== MCAN_init ========
 */
int_fast16_t MCAN_init(const MCAN_InitParams *initParams)
{
    int_fast16_t status = MCAN_STATUS_ERROR;
    uint32_t regVal;

    if (MCAN_RWD_WDC_MAX >= initParams->wdcPreload)
    {
        MCAN_enableConfigChange();

        /* Configure MCAN mode (FD vs Classic CAN operation) and controls */
        regVal = MCAN_readReg(MCAN_CCCR);
        /* Clock stop request must be cleared when written in Standby mode */
        MCAN_SET_FIELD(regVal, MCAN_CCCR_CSR, 0U);
        MCAN_SET_FIELD(regVal, MCAN_CCCR_FDOE, initParams->fdMode);
        MCAN_SET_FIELD(regVal, MCAN_CCCR_BRSE, initParams->brsEnable);
        MCAN_SET_FIELD(regVal, MCAN_CCCR_TXP, initParams->txpEnable);
        MCAN_SET_FIELD(regVal, MCAN_CCCR_EFBI, initParams->efbi);
        MCAN_SET_FIELD(regVal, MCAN_CCCR_PXHD, initParams->pxhDisable);
        MCAN_SET_FIELD(regVal, MCAN_CCCR_DAR, initParams->darEnable);
        MCAN_writeReg(MCAN_CCCR, regVal);

        /* Configure MSG RAM watchdog counter preload value */
        MCAN_MODIFY_FIELD(MCAN_RWD, MCAN_RWD_WDC, initParams->wdcPreload);

        MCAN_disableConfigChange();

        status = MCAN_STATUS_SUCCESS;
    }

    return status;
}

/*
 *  ======== MCAN_config ========
 */
int_fast16_t MCAN_config(const MCAN_ConfigParams *config)
{
    int_fast16_t status = MCAN_STATUS_ERROR;
    uint32_t regVal;

    MCAN_enableConfigChange();

    /* Configure MCAN control registers */
    regVal = MCAN_readReg(MCAN_CCCR);
    /* Clock stop request must be cleared when written in Standby mode */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_CSR, 0U);
    MCAN_SET_FIELD(regVal, MCAN_CCCR_MON, config->monEnable);
    MCAN_SET_FIELD(regVal, MCAN_CCCR_ASM, config->asmEnable);
    MCAN_writeReg(MCAN_CCCR, regVal);

    /* Configure Global Filter */
    regVal = 0;
    MCAN_SET_FIELD(regVal, MCAN_GFC_RRFE, config->filterConfig.rrfe);
    MCAN_SET_FIELD(regVal, MCAN_GFC_RRFS, config->filterConfig.rrfs);
    MCAN_SET_FIELD(regVal, MCAN_GFC_ANFE, config->filterConfig.anfe);
    MCAN_SET_FIELD(regVal, MCAN_GFC_ANFS, config->filterConfig.anfs);
    MCAN_writeReg(MCAN_GFC, regVal);

    if ((MCAN_TSCC_TCP_MAX >= config->tsPrescaler) && (MCAN_TOCC_TOP_MAX >= config->timeoutPreload))
    {
        /* Configure timestamp counter */
        regVal = 0;
        MCAN_SET_FIELD(regVal, MCAN_TSCC_TSS, config->tsSelect);
        MCAN_SET_FIELD(regVal, MCAN_TSCC_TCP, config->tsPrescaler);
        MCAN_writeReg(MCAN_TSCC, regVal);

        /* Configure timeout counter */
        regVal = 0;
        MCAN_SET_FIELD(regVal, MCAN_TOCC_TOS, config->timeoutSelect);
        MCAN_SET_FIELD(regVal, MCAN_TOCC_TOP, config->timeoutPreload);
        MCAN_writeReg(MCAN_TOCC, regVal);

        /* Enable timeout counter */
        MCAN_MODIFY_FIELD(MCAN_TOCC, MCAN_TOCC_ETOC, config->timeoutCntEnable);

        status = MCAN_STATUS_SUCCESS;
    }

    MCAN_disableConfigChange();

    return status;
}

/*
 *  ======== MCAN_getBitTime ========
 */
void MCAN_getBitTime(MCAN_BitTimingParams *bitTiming)
{
    uint32_t regVal;

    /* Read Nominal Bit Timing and Prescaler */
    regVal                       = MCAN_readReg(MCAN_NBTP);
    bitTiming->nomRatePrescaler  = MCAN_GET_FIELD(regVal, MCAN_NBTP_NBRP);
    bitTiming->nomTimeSeg1       = MCAN_GET_FIELD(regVal, MCAN_NBTP_NTSEG1);
    bitTiming->nomTimeSeg2       = MCAN_GET_FIELD(regVal, MCAN_NBTP_NTSEG2);
    bitTiming->nomSynchJumpWidth = MCAN_GET_FIELD(regVal, MCAN_NBTP_NSJW);

    /* Read Data Bit Timing and Prescaler */
    regVal                        = MCAN_readReg(MCAN_DBTP);
    bitTiming->dataRatePrescaler  = MCAN_GET_FIELD(regVal, MCAN_DBTP_DBRP);
    bitTiming->dataTimeSeg1       = MCAN_GET_FIELD(regVal, MCAN_DBTP_DTSEG1);
    bitTiming->dataTimeSeg2       = MCAN_GET_FIELD(regVal, MCAN_DBTP_DTSEG2);
    bitTiming->dataSynchJumpWidth = MCAN_GET_FIELD(regVal, MCAN_DBTP_DSJW);

    /* Read Transceiver Delay Compensation */
    regVal                    = MCAN_readReg(MCAN_TDCR);
    bitTiming->tdcConfig.tdcf = MCAN_GET_FIELD(regVal, MCAN_TDCR_TDCF);
    bitTiming->tdcConfig.tdco = MCAN_GET_FIELD(regVal, MCAN_TDCR_TDCO);
}

/*
 *  ======== MCAN_setBitTime ========
 */
int_fast16_t MCAN_setBitTime(const MCAN_BitTimingParams *bitTiming)
{
    int_fast16_t status = MCAN_STATUS_ERROR;
    uint32_t regVal;

    MCAN_enableConfigChange();

    if ((MCAN_NBTP_NSJW_MAX >= bitTiming->nomSynchJumpWidth) && (MCAN_NBTP_NTSEG2_MAX >= bitTiming->nomTimeSeg2) &&
        (MCAN_NBTP_NTSEG1_MAX >= bitTiming->nomTimeSeg1) && (MCAN_NBTP_NBRP_MAX >= bitTiming->nomRatePrescaler))
    {
        regVal = 0;
        MCAN_SET_FIELD(regVal, MCAN_NBTP_NBRP, bitTiming->nomRatePrescaler);
        MCAN_SET_FIELD(regVal, MCAN_NBTP_NTSEG1, bitTiming->nomTimeSeg1);
        MCAN_SET_FIELD(regVal, MCAN_NBTP_NTSEG2, bitTiming->nomTimeSeg2);
        MCAN_SET_FIELD(regVal, MCAN_NBTP_NSJW, bitTiming->nomSynchJumpWidth);
        MCAN_writeReg(MCAN_NBTP, regVal);

        status = MCAN_STATUS_SUCCESS;
    }

    if ((MCAN_STATUS_SUCCESS == status) && (MCAN_DBTP_DSJW_MAX >= bitTiming->dataSynchJumpWidth) &&
        (MCAN_DBTP_DTSEG2_MAX >= bitTiming->dataTimeSeg2) && (MCAN_DBTP_DTSEG1_MAX >= bitTiming->dataTimeSeg1) &&
        (MCAN_DBTP_DBRP_MAX >= bitTiming->dataRatePrescaler))
    {
        regVal = 0;
        MCAN_SET_FIELD(regVal, MCAN_DBTP_DBRP, bitTiming->dataRatePrescaler);
        MCAN_SET_FIELD(regVal, MCAN_DBTP_DTSEG1, bitTiming->dataTimeSeg1);
        MCAN_SET_FIELD(regVal, MCAN_DBTP_DTSEG2, bitTiming->dataTimeSeg2);
        MCAN_SET_FIELD(regVal, MCAN_DBTP_DSJW, bitTiming->dataSynchJumpWidth);

        if (0U < bitTiming->tdcConfig.tdco)
        {
            /* Data rate prescaler must be <= 1 to enable TDC */
            if (1U >= bitTiming->dataRatePrescaler)
            {
                /* Enable Transceiver Delay Compensation */
                MCAN_SET_FIELD(regVal, MCAN_DBTP_TDC, 1U);
            }
            else
            {
                status = MCAN_STATUS_ERROR;
            }
        }

        MCAN_writeReg(MCAN_DBTP, regVal);
    }
    else
    {
        status = MCAN_STATUS_ERROR;
    }

    if ((MCAN_STATUS_SUCCESS == status) && (MCAN_TDCR_TDCF_MAX >= bitTiming->tdcConfig.tdcf) &&
        (MCAN_TDCR_TDCO_MAX >= bitTiming->tdcConfig.tdco))
    {
        /* Configure Transceiver Delay Compensation */
        regVal = 0;
        MCAN_SET_FIELD(regVal, MCAN_TDCR_TDCF, bitTiming->tdcConfig.tdcf);
        MCAN_SET_FIELD(regVal, MCAN_TDCR_TDCO, bitTiming->tdcConfig.tdco);
        MCAN_writeReg(MCAN_TDCR, regVal);
    }
    else
    {
        status = MCAN_STATUS_ERROR;
    }

    MCAN_disableConfigChange();
    return status;
}

/*
 *  ======== MCAN_configMsgRam ========
 */
void MCAN_configMsgRam(const MCAN_MsgRamConfig *msgRamConfig)
{
    uint32_t regVal;
    uint32_t regValRXESC = 0U;

    MCAN_enableConfigChange();

    /* Configure Standard Message Filters section */
    if (0U != msgRamConfig->sidFilterListSize)
    {
        regVal = 0U;
        MCAN_SET_FIELD(regVal, MCAN_SIDFC_FLSSA, (msgRamConfig->sidFilterStartAddr >> MCAN_START_ADDR_SHIFT));
        MCAN_SET_FIELD(regVal, MCAN_SIDFC_LSS, msgRamConfig->sidFilterListSize);
        MCAN_writeReg(MCAN_SIDFC, regVal);
    }

    /* Configure Extended Message Filters section */
    if (0U != msgRamConfig->xidFilterListSize)
    {
        regVal = 0U;
        MCAN_SET_FIELD(regVal, MCAN_XIDFC_FLESA, (msgRamConfig->xidFilterStartAddr >> MCAN_START_ADDR_SHIFT));
        MCAN_SET_FIELD(regVal, MCAN_XIDFC_LSE, msgRamConfig->xidFilterListSize);
        MCAN_writeReg(MCAN_XIDFC, regVal);
    }

    /* Configure Rx FIFO 0 section */
    if (0U != msgRamConfig->rxFifo0Size)
    {
        regVal = 0U;
        MCAN_SET_FIELD(regVal, MCAN_RXF0C_F0SA, (msgRamConfig->rxFifo0StartAddr >> MCAN_START_ADDR_SHIFT));
        MCAN_SET_FIELD(regVal, MCAN_RXF0C_F0S, msgRamConfig->rxFifo0Size);
        MCAN_SET_FIELD(regVal, MCAN_RXF0C_F0WM, msgRamConfig->rxFifo0Watermark);
        MCAN_SET_FIELD(regVal, MCAN_RXF0C_F0OM, msgRamConfig->rxFifo0OpMode);
        MCAN_writeReg(MCAN_RXF0C, regVal);

        /* Configure Rx FIFO0 elements size */
        MCAN_SET_FIELD(regValRXESC, MCAN_RXESC_F0DS, msgRamConfig->rxFifo0ElemSize);
    }

    /* Configure Rx FIFO 1 section */
    if (0U != msgRamConfig->rxFifo1Size)
    {
        regVal = 0U;
        MCAN_SET_FIELD(regVal, MCAN_RXF1C_F1SA, (msgRamConfig->rxFifo1StartAddr >> MCAN_START_ADDR_SHIFT));
        MCAN_SET_FIELD(regVal, MCAN_RXF1C_F1S, msgRamConfig->rxFifo1Size);
        MCAN_SET_FIELD(regVal, MCAN_RXF1C_F1WM, msgRamConfig->rxFifo1Watermark);
        MCAN_SET_FIELD(regVal, MCAN_RXF1C_F1OM, msgRamConfig->rxFifo1OpMode);
        MCAN_writeReg(MCAN_RXF1C, regVal);

        /* Configure Rx FIFO1 elements size */
        MCAN_SET_FIELD(regValRXESC, MCAN_RXESC_F1DS, msgRamConfig->rxFifo1ElemSize);
    }

    /* Configure Rx Buffer Start Address */
    MCAN_MODIFY_FIELD(MCAN_RXBC, MCAN_RXBC_RBSA, (msgRamConfig->rxBufStartAddr >> MCAN_START_ADDR_SHIFT));
    /* Configure Rx Buffer elements size */
    MCAN_SET_FIELD(regValRXESC, MCAN_RXESC_RBDS, msgRamConfig->rxBufElemSize);
    MCAN_writeReg(MCAN_RXESC, regValRXESC);

    /* Configure Tx Event FIFO section */
    if (0U != msgRamConfig->txEventFifoSize)
    {
        regVal = 0U;
        MCAN_SET_FIELD(regVal, MCAN_TXEFC_EFSA, (msgRamConfig->txEventFifoStartAddr >> MCAN_START_ADDR_SHIFT));
        MCAN_SET_FIELD(regVal, MCAN_TXEFC_EFS, msgRamConfig->txEventFifoSize);
        MCAN_SET_FIELD(regVal, MCAN_TXEFC_EFWM, msgRamConfig->txEventFifoWatermark);
        MCAN_writeReg(MCAN_TXEFC, regVal);
    }

    /* Configure Tx Buffer and FIFO/Q section */
    if ((0U != msgRamConfig->txFifoQSize) || (0U != msgRamConfig->txBufNum))
    {
        regVal = 0U;
        MCAN_SET_FIELD(regVal, MCAN_TXBC_TBSA, (msgRamConfig->txBufStartAddr >> MCAN_START_ADDR_SHIFT));
        MCAN_SET_FIELD(regVal, MCAN_TXBC_NDTB, msgRamConfig->txBufNum);
        MCAN_SET_FIELD(regVal, MCAN_TXBC_TFQS, msgRamConfig->txFifoQSize);
        MCAN_SET_FIELD(regVal, MCAN_TXBC_TFQM, msgRamConfig->txFifoQMode);
        MCAN_writeReg(MCAN_TXBC, regVal);

        /* Configure Tx Buffer element size */
        MCAN_MODIFY_FIELD(MCAN_TXESC, MCAN_TXESC_TBDS, msgRamConfig->txBufElemSize);
    }

    MCAN_disableConfigChange();
}

/*
 *  ======== MCAN_getTxBufAddr ========
 */
static uint32_t MCAN_getTxBufAddr(uint32_t bufIdx)
{
    uint32_t elemAddr;
    uint32_t elemSize;
    uint32_t elemSizeIdx;
    uint32_t startAddr;

    startAddr = MCAN_READ_FIELD(MCAN_TXBC, MCAN_TXBC_TBSA);
    /* Shift address field to correct position */
    startAddr = (startAddr << MCAN_START_ADDR_SHIFT);

    elemSizeIdx = MCAN_READ_FIELD(MCAN_TXESC, MCAN_TXESC_TBDS);
    /* Get element size words and convert to bytes */
    elemSize    = MCAN_elementSizeWords[elemSizeIdx] << 2U;
    elemAddr    = startAddr + (elemSize * bufIdx);
    elemAddr += MCAN_getMRAMOffset();

    return elemAddr;
}

/*
 *  ======== MCAN_writeTxMsgNoCpy ========
 */
void MCAN_writeTxMsgNoCpy(uint32_t bufIdx, const MCAN_TxBufElementNoCpy *elem)
{
    uint32_t elemAddr = MCAN_getTxBufAddr(bufIdx);

    MCAN_writeMsgNoCpy(elemAddr, elem);
}

/*
 *  ======== MCAN_writeTxMsg ========
 */
void MCAN_writeTxMsg(uint32_t bufIdx, const MCAN_TxBufElement *elem)
{
    uint32_t elemAddr = MCAN_getTxBufAddr(bufIdx);

    MCAN_writeMsg(elemAddr, elem);
}

/*
 *  ======== MCAN_setTxBufAddReq ========
 */
void MCAN_setTxBufAddReq(uint32_t bufIdx)
{
    MCAN_writeReg(MCAN_TXBAR, ((uint32_t)1U << bufIdx));
}

/*
 *  ======== MCAN_getTxFifoQStatus ========
 */
void MCAN_getTxFifoQStatus(MCAN_TxFifoQStatus *fifoQStatus)
{
    uint32_t regVal = MCAN_readReg(MCAN_TXFQS);

    fifoQStatus->fifoFull = MCAN_GET_FIELD(regVal, MCAN_TXFQS_TFQF);
    fifoQStatus->putIdx   = MCAN_GET_FIELD(regVal, MCAN_TXFQS_TFQPI);
    fifoQStatus->getIdx   = MCAN_GET_FIELD(regVal, MCAN_TXFQS_TFGI);
    fifoQStatus->freeLvl  = MCAN_GET_FIELD(regVal, MCAN_TXFQS_TFFL);
}

/*
 *  ======== MCAN_getNewDataStatus ========
 */
void MCAN_getNewDataStatus(MCAN_RxNewDataStatus *newDataStatus)
{
    newDataStatus->statusLow  = MCAN_readReg(MCAN_NDAT1);
    newDataStatus->statusHigh = MCAN_readReg(MCAN_NDAT2);
}

/*
 *  ======== MCAN_clearNewDataStatus ========
 */
void MCAN_clearNewDataStatus(const MCAN_RxNewDataStatus *newDataStatus)
{
    MCAN_writeReg(MCAN_NDAT1, newDataStatus->statusLow);
    MCAN_writeReg(MCAN_NDAT2, newDataStatus->statusHigh);
}

/*
 *  ======== MCAN_readRxMsgNoCpy ========
 */
void MCAN_readRxMsgNoCpy(MCAN_MemType memType, uint32_t num, MCAN_RxBufElementNoCpy *elem)
{
    uint32_t elemAddr;
    uint32_t elemSize;
    uint32_t elemSizeIdx;
    uint32_t enableRead = 0U;
    uint32_t idx;
    uint32_t startAddr;

    if (MCAN_MEM_TYPE_BUF == memType)
    {
        startAddr   = MCAN_READ_FIELD(MCAN_RXBC, MCAN_RXBC_RBSA);
        elemSizeIdx = MCAN_READ_FIELD(MCAN_RXESC, MCAN_RXESC_RBDS);
        idx         = num;
        enableRead  = 1U;
    }
    else /* (MCAN_MEM_TYPE_FIFO == memType) */
    {
        switch (num)
        {
            case MCAN_RX_FIFO_NUM_0:
                startAddr   = MCAN_READ_FIELD(MCAN_RXF0C, MCAN_RXF0C_F0SA);
                elemSizeIdx = MCAN_READ_FIELD(MCAN_RXESC, MCAN_RXESC_F0DS);
                idx         = MCAN_READ_FIELD(MCAN_RXF0S, MCAN_RXF0S_F0GI);
                enableRead  = 1U;
                break;

            case MCAN_RX_FIFO_NUM_1:
                startAddr   = MCAN_READ_FIELD(MCAN_RXF1C, MCAN_RXF1C_F1SA);
                elemSizeIdx = MCAN_READ_FIELD(MCAN_RXESC, MCAN_RXESC_F1DS);
                idx         = MCAN_READ_FIELD(MCAN_RXF1S, MCAN_RXF1S_F1GI);
                enableRead  = 1U;
                break;

            default:
                /* Invalid option */
                break;
        }
    }

    if (0U != enableRead)
    {
        /* Shift address field to correct position */
        startAddr = (startAddr << MCAN_START_ADDR_SHIFT);
        elemSize  = MCAN_elementSizeWords[elemSizeIdx] << 2U; /* convert to bytes */
        elemAddr  = startAddr + (elemSize * idx);
        elemAddr += MCAN_getMRAMOffset();
        MCAN_readMsgNoCpy(elemAddr, elem);
    }
}

/*
 *  ======== MCAN_readRxMsg ========
 */
void MCAN_readRxMsg(MCAN_MemType memType, uint32_t num, MCAN_RxBufElement *elem)
{
    uint32_t elemAddr;
    uint32_t elemSize;
    uint32_t elemSizeIdx;
    uint32_t enableRead = 0U;
    uint32_t idx;
    uint32_t startAddr;

    if (MCAN_MEM_TYPE_BUF == memType)
    {
        startAddr   = MCAN_READ_FIELD(MCAN_RXBC, MCAN_RXBC_RBSA);
        elemSizeIdx = MCAN_READ_FIELD(MCAN_RXESC, MCAN_RXESC_RBDS);
        idx         = num;
        enableRead  = 1U;
    }
    else /* (MCAN_MEM_TYPE_FIFO == memType) */
    {
        switch (num)
        {
            case MCAN_RX_FIFO_NUM_0:
                startAddr   = MCAN_READ_FIELD(MCAN_RXF0C, MCAN_RXF0C_F0SA);
                elemSizeIdx = MCAN_READ_FIELD(MCAN_RXESC, MCAN_RXESC_F0DS);
                idx         = MCAN_READ_FIELD(MCAN_RXF0S, MCAN_RXF0S_F0GI);
                enableRead  = 1U;
                break;

            case MCAN_RX_FIFO_NUM_1:
                startAddr   = MCAN_READ_FIELD(MCAN_RXF1C, MCAN_RXF1C_F1SA);
                elemSizeIdx = MCAN_READ_FIELD(MCAN_RXESC, MCAN_RXESC_F1DS);
                idx         = MCAN_READ_FIELD(MCAN_RXF1S, MCAN_RXF1S_F1GI);
                enableRead  = 1U;
                break;

            default:
                /* Invalid option */
                break;
        }
    }

    if (0U != enableRead)
    {
        /* Shift address field to correct position */
        startAddr = (uint32_t)(startAddr << MCAN_START_ADDR_SHIFT);
        elemSize  = MCAN_elementSizeWords[elemSizeIdx] << 2U; /* convert to bytes */
        elemAddr  = startAddr + (elemSize * idx);
        elemAddr += MCAN_getMRAMOffset();
        MCAN_readMsg(elemAddr, elem);
    }
}

/*
 *  ======== MCAN_readTxEventFifo ========
 */
int_fast16_t MCAN_readTxEventFifo(MCAN_TxEventFifoElement *elem)
{
    int_fast16_t status = MCAN_STATUS_ERROR;
    uint32_t elemAddr;
    uint32_t elemSize;
    uint32_t fillLevel;
    uint32_t idx;
    uint32_t regVal;
    uint32_t startAddr;

    regVal    = MCAN_readReg(MCAN_TXEFS);
    fillLevel = MCAN_GET_FIELD(regVal, MCAN_TXEFS_EFFL);

    if (0U != fillLevel)
    {
        idx = MCAN_GET_FIELD(regVal, MCAN_TXEFS_EFGI);

        startAddr = MCAN_READ_FIELD(MCAN_TXEFC, MCAN_TXEFC_EFSA);
        elemSize  = MCAN_TX_EVENT_ELEM_SIZE;

        /* Shift address field to correct position */
        startAddr = (startAddr << MCAN_START_ADDR_SHIFT);
        elemAddr  = startAddr + (elemSize * idx);
        elemAddr += MCAN_getMRAMOffset();

        regVal    = MCAN_readReg(elemAddr);
        elem->id  = MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_ID);
        elem->rtr = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_RTR);
        elem->xtd = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_XTD);
        elem->esi = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_ESI);

        elemAddr += 4U;
        regVal     = MCAN_readReg(elemAddr);
        elem->txts = (uint16_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_TXTS);
        elem->dlc  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_DLC);
        elem->brs  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_BRS);
        elem->fdf  = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_FDF);
        elem->et   = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_ET);
        elem->mm   = (uint8_t)MCAN_GET_FIELD(regVal, MCAN_TX_EVENT_FIFO_ELEM_MM);

        /* Write the Event FIFO Acknowledge Index to increment the Tx Event FIFO
         * get index.
         */
        MCAN_MODIFY_FIELD(MCAN_TXEFA, MCAN_TXEFA_EFAI, idx);

        status = MCAN_STATUS_SUCCESS;
    }

    return status;
}

/*
 *  ======== MCAN_addStdMsgIDFilter ========
 */
void MCAN_addStdMsgIDFilter(uint32_t filtNum, const MCAN_StdMsgIDFilterElement *elem)
{
    uint32_t elemAddr;
    uint32_t regVal;
    uint32_t startAddr;

    startAddr = MCAN_READ_FIELD(MCAN_SIDFC, MCAN_SIDFC_FLSSA);
    /* Shift address field to correct position */
    startAddr = (startAddr << MCAN_START_ADDR_SHIFT);
    elemAddr  = startAddr + (filtNum * MCAN_STD_ID_FILTER_ELEM_SIZE);
    elemAddr += MCAN_getMRAMOffset();

    regVal = 0U;
    regVal |= (elem->sfid2 << MCAN_STD_ID_FILTER_SFID2_SHIFT);
    regVal |= (elem->sfid1 << MCAN_STD_ID_FILTER_SFID1_SHIFT);
    regVal |= (elem->sfec << MCAN_STD_ID_FILTER_SFEC_SHIFT);
    regVal |= (elem->sft << MCAN_STD_ID_FILTER_SFT_SHIFT);
    MCAN_writeReg(elemAddr, regVal);
}

/*
 *  ======== MCAN_addExtMsgIDFilter ========
 */
void MCAN_addExtMsgIDFilter(uint32_t filtNum, const MCAN_ExtMsgIDFilterElement *elem)
{
    uint32_t elemAddr;
    uint32_t regVal;
    uint32_t startAddr;

    startAddr = MCAN_READ_FIELD(MCAN_XIDFC, MCAN_XIDFC_FLESA);
    /* Shift address field to correct position */
    startAddr = (startAddr << MCAN_START_ADDR_SHIFT);
    elemAddr  = startAddr + (filtNum * MCAN_EXT_ID_FILTER_ELEM_SIZE);
    elemAddr += MCAN_getMRAMOffset();

    regVal = 0U;
    regVal |= (elem->efid1 << MCAN_EXT_ID_FILTER_EFID1_SHIFT);
    regVal |= (elem->efec << MCAN_EXT_ID_FILTER_EFEC_SHIFT);
    MCAN_writeReg(elemAddr, regVal);

    elemAddr += 4U;
    regVal = 0U;
    regVal |= (elem->efid2 << MCAN_EXT_ID_FILTER_EFID2_SHIFT);
    regVal |= (elem->eft << MCAN_EXT_ID_FILTER_EFT_SHIFT);
    MCAN_writeReg(elemAddr, regVal);
}

/*
 *  ======== MCAN_enableLoopbackMode ========
 */
void MCAN_enableLoopbackMode(MCAN_LpbkMode lpbkMode)
{
    uint32_t regVal;

    MCAN_enableConfigChange();

    regVal = MCAN_readReg(MCAN_CCCR);

    /* Clock stop request must be cleared when written in Standby mode */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_CSR, 0U);

    if (MCAN_LPBK_MODE_INTERNAL == lpbkMode)
    {
        MCAN_SET_FIELD(regVal, MCAN_CCCR_MON, 1U);
    }
    else
    {
        MCAN_SET_FIELD(regVal, MCAN_CCCR_MON, 0U);
    }

    /* Enable write access to TEST reg */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_TEST, 1U);

    MCAN_writeReg(MCAN_CCCR, regVal);

    MCAN_MODIFY_FIELD(MCAN_TEST, MCAN_TEST_LBCK, 1U);

    MCAN_disableConfigChange();
}

/*
 *  ======== MCAN_disableLoopbackMode ========
 */
void MCAN_disableLoopbackMode(void)
{
    uint32_t regVal;

    MCAN_enableConfigChange();

    /* Disable loopback mode */
    MCAN_MODIFY_FIELD(MCAN_TEST, MCAN_TEST_LBCK, 0U);

    regVal = MCAN_readReg(MCAN_CCCR);

    /* Clock stop request must be cleared when written in Standby mode */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_CSR, 0U);

    /* Disable write access to TEST reg */
    MCAN_SET_FIELD(regVal, MCAN_CCCR_TEST, 0U);
    MCAN_SET_FIELD(regVal, MCAN_CCCR_MON, 0U);

    MCAN_writeReg(MCAN_CCCR, regVal);

    MCAN_disableConfigChange();
}

/*
 *  ======== MCAN_getProtocolStatus ========
 */
void MCAN_getProtocolStatus(MCAN_ProtocolStatus *protStatus)
{
    uint32_t regVal = MCAN_readReg(MCAN_PSR);

    protStatus->lastErrCode   = MCAN_GET_FIELD(regVal, MCAN_PSR_LEC);
    protStatus->act           = MCAN_GET_FIELD(regVal, MCAN_PSR_ACT);
    protStatus->errPassive    = MCAN_GET_FIELD(regVal, MCAN_PSR_EP);
    protStatus->warningStatus = MCAN_GET_FIELD(regVal, MCAN_PSR_EW);
    protStatus->busOffStatus  = MCAN_GET_FIELD(regVal, MCAN_PSR_BO);
    protStatus->dlec          = MCAN_GET_FIELD(regVal, MCAN_PSR_DLEC);
    protStatus->resi          = MCAN_GET_FIELD(regVal, MCAN_PSR_RESI);
    protStatus->rbrs          = MCAN_GET_FIELD(regVal, MCAN_PSR_RBRS);
    protStatus->rfdf          = MCAN_GET_FIELD(regVal, MCAN_PSR_RFDF);
    protStatus->pxe           = MCAN_GET_FIELD(regVal, MCAN_PSR_PXE);
    protStatus->tdcv          = MCAN_GET_FIELD(regVal, MCAN_PSR_TDCV);
}

/*
 *  ======== MCAN_enableInt ========
 */
void MCAN_enableInt(uint32_t intMask)
{
    uint32_t regVal = MCAN_readReg(MCAN_IE);

    regVal |= intMask;

    MCAN_writeReg(MCAN_IE, regVal);
}

/*
 *  ======== MCAN_disableInt ========
 */
void MCAN_disableInt(uint32_t intMask)
{
    uint32_t regVal = MCAN_readReg(MCAN_IE);

    regVal &= (uint32_t)~intMask;

    MCAN_writeReg(MCAN_IE, regVal);
}

/*
 *  ======== MCAN_setIntLineSel ========
 */
void MCAN_setIntLineSel(uint32_t intMask, MCAN_IntLineNum lineNum)
{
    uint32_t regVal;

    regVal = MCAN_readReg(MCAN_ILS);

    if (MCAN_INT_LINE_NUM_0 == lineNum)
    {
        /* 0 = Interrupt assigned to interrupt line 0 */
        regVal &= (uint32_t)~intMask;
    }
    else
    {
        /* 1 = Interrupt assigned to interrupt line 0 */
        regVal |= intMask;
    }

    MCAN_writeReg(MCAN_ILS, regVal);
}

/*
 *  ======== MCAN_enableIntLine ========
 */
void MCAN_enableIntLine(MCAN_IntLineNum lineNum)
{
    uint32_t mask   = ((uint32_t)1U << lineNum);
    uint32_t regVal = MCAN_readReg(MCAN_ILE);

    regVal |= mask;

    MCAN_writeReg(MCAN_ILE, regVal);
}

/*
 *  ======== MCAN_disableIntLine ========
 */
void MCAN_disableIntLine(MCAN_IntLineNum lineNum)
{
    uint32_t mask   = ((uint32_t)1U << lineNum);
    uint32_t regVal = MCAN_readReg(MCAN_ILE);

    regVal &= (uint32_t)~mask;

    MCAN_writeReg(MCAN_ILE, regVal);
}

/*
 *  ======== MCAN_getIntStatus ========
 */
uint32_t MCAN_getIntStatus(void)
{
    return (MCAN_readReg(MCAN_IR));
}

/*
 *  ======== MCAN_clearIntStatus ========
 */
void MCAN_clearIntStatus(uint32_t intMask)
{
    MCAN_writeReg(MCAN_IR, intMask);
}

/*
 *  ======== MCAN_getRxFifoStatus ========
 */
void MCAN_getRxFifoStatus(MCAN_RxFifoNum fifoNum, MCAN_RxFifoStatus *fifoStatus)
{
    uint32_t regVal;

    if (MCAN_RX_FIFO_NUM_0 == fifoNum)
    {
        regVal = MCAN_readReg(MCAN_RXF0S);
    }
    else
    {
        regVal = MCAN_readReg(MCAN_RXF1S);
    }

    /* All Rx FIFO1 status fields match Rx FIFO0 with the exception of the
     * Debug Message Status field (bits 31:30) which only exist in RXF1S
     */
    fifoStatus->fillLvl  = MCAN_GET_FIELD(regVal, MCAN_RXF0S_F0FL);
    fifoStatus->getIdx   = MCAN_GET_FIELD(regVal, MCAN_RXF0S_F0GI);
    fifoStatus->putIdx   = MCAN_GET_FIELD(regVal, MCAN_RXF0S_F0PI);
    fifoStatus->fifoFull = MCAN_GET_FIELD(regVal, MCAN_RXF0S_F0F);
    fifoStatus->msgLost  = MCAN_GET_FIELD(regVal, MCAN_RXF0S_RF0L);
}

/*
 *  ======== MCAN_setRxFifoAck ========
 */
int_fast16_t MCAN_setRxFifoAck(MCAN_RxFifoNum fifoNum, uint32_t idx)
{
    int_fast16_t status = MCAN_STATUS_ERROR;
    uint32_t numElements;

    if (MCAN_RX_FIFO_NUM_0 == fifoNum)
    {
        numElements = MCAN_READ_FIELD(MCAN_RXF0C, MCAN_RXF0C_F0S);

        if (numElements >= idx)
        {
            MCAN_MODIFY_FIELD(MCAN_RXF0A, MCAN_RXF0A_F0AI, idx);
            status = MCAN_STATUS_SUCCESS;
        }
    }
    else if (MCAN_RX_FIFO_NUM_1 == fifoNum)
    {
        numElements = MCAN_READ_FIELD(MCAN_RXF1C, MCAN_RXF1C_F1S);

        if (numElements >= idx)
        {
            MCAN_MODIFY_FIELD(MCAN_RXF1A, MCAN_RXF1A_F1AI, idx);
            status = MCAN_STATUS_SUCCESS;
        }
    }
    else
    {
        /* Do nothing */
    }

    return status;
}

/*
 *  ======== MCAN_getTxEventFifoStatus ========
 */
void MCAN_getTxEventFifoStatus(MCAN_TxEventFifoStatus *fifoStatus)
{
    uint32_t regVal = MCAN_readReg(MCAN_TXEFS);

    fifoStatus->fillLvl  = MCAN_GET_FIELD(regVal, MCAN_TXEFS_EFFL);
    fifoStatus->getIdx   = MCAN_GET_FIELD(regVal, MCAN_TXEFS_EFGI);
    fifoStatus->putIdx   = MCAN_GET_FIELD(regVal, MCAN_TXEFS_EFPI);
    fifoStatus->fifoFull = MCAN_GET_FIELD(regVal, MCAN_TXEFS_EFF);
    fifoStatus->eleLost  = MCAN_GET_FIELD(regVal, MCAN_TXEFS_TEFL);
}

/*
 *  ======== MCAN_getTxBufReqPend ========
 */
uint32_t MCAN_getTxBufReqPend(void)
{
    return (MCAN_readReg(MCAN_TXBRP));
}

/*
 *  ======== MCAN_cancelTxBufReq ========
 */
void MCAN_cancelTxBufReq(uint32_t bufIdx)
{
    uint32_t regVal = ((uint32_t)1U << bufIdx);

    MCAN_writeReg(MCAN_TXBCR, regVal);
}

/*
 *  ======== MCAN_getTxBufTransmissionStatus ========
 */
uint32_t MCAN_getTxBufTransmissionStatus(void)
{
    return (MCAN_readReg(MCAN_TXBTO));
}

/*
 *  ======== MCAN_getTxBufCancellationStatus ========
 */
uint32_t MCAN_getTxBufCancellationStatus(void)
{
    return (MCAN_readReg(MCAN_TXBCF));
}

/*
 *  ======== MCAN_enableTxBufTransInt ========
 */
void MCAN_enableTxBufTransInt(uint32_t bufMask)
{
    uint32_t regVal = MCAN_readReg(MCAN_TXBTIE);

    regVal |= bufMask;

    MCAN_writeReg(MCAN_TXBTIE, regVal);
}

/*
 *  ======== MCAN_disableTxBufTransInt ========
 */
void MCAN_disableTxBufTransInt(uint32_t bufMask)
{
    uint32_t regVal = MCAN_readReg(MCAN_TXBTIE);

    regVal &= (uint32_t)~bufMask;

    MCAN_writeReg(MCAN_TXBTIE, regVal);
}

/*
 *  ======== MCAN_getClkStopAck ========
 */
uint32_t MCAN_getClkStopAck(void)
{
    return (MCAN_READ_FIELD(MCAN_CCCR, MCAN_CCCR_CSA));
}

/*
 *  ======== MCAN_getTimestampCounter ========
 */
uint16_t MCAN_getTimestampCounter(void)
{
    return (uint16_t)MCAN_readReg(MCAN_TSCV);
}
