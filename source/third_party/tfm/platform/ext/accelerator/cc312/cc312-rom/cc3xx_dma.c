/*
 * Copyright (c) 2021-2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_dma.h"

#include "cc3xx_dev.h"
#include "cc3xx_engine_state.h"
#include "cc3xx_config.h"

#include <assert.h>

struct cc3xx_dma_state_t dma_state;

#ifdef CC3XX_CONFIG_DMA_REMAP_ENABLE
static cc3xx_dma_remap_region_t remap_regions[CC3XX_CONFIG_DMA_REMAP_REGION_AM] = {0};

void cc3xx_dma_remap_region_init(uint32_t remap_region_idx,
                                        cc3xx_dma_remap_region_t *region)
{
    memcpy(&remap_regions[remap_region_idx], region, sizeof(*region));
}

void cc3xx_dma_remap_region_clear(uint32_t remap_region_idx)
{
    memset(&remap_regions[remap_region_idx], 0, sizeof(cc3xx_dma_remap_region_t));
}

void cc3xx_dma_tcm_cpusel(uint32_t cpuid)
{
    dma_state.remap_cpusel = cpuid;
}

static uintptr_t remap_addr(uintptr_t addr)
{
    uint32_t idx;
    cc3xx_dma_remap_region_t *region;

    for (idx = 0; idx < CC3XX_CONFIG_DMA_REMAP_REGION_AM; idx++) {
        region = &remap_regions[idx];
        if (addr >= region->region_base
            && addr < region->region_base + region->region_size) {
            return (addr - region->region_base) + region->remap_base
                    + (region->remap_cpusel_offset * dma_state.remap_cpusel);
        }
    }

    return addr;
}

#else

static uintptr_t remap_addr(uintptr_t addr) {
    return addr;
}

#endif /* CC3XX_CONFIG_DMA_REMAP_ENABLE */

static void process_data(const void* buf, size_t length)
{
    uintptr_t remapped_buf;

    /* Enable the DMA clock */
    P_CC3XX->misc.dma_clk_enable = 0x1U;

    /* Mask a sensible set of the host interrupts */
    P_CC3XX->host_rgf.host_rgf_imr = 0x7F0U;

    /* Reset the AXI_ERROR and SYM_DMA_COMPLETED interrupts */
    P_CC3XX->host_rgf.host_rgf_icr |= 0xFF0U;

    /* remap the address, particularly for TCMs */
    remapped_buf = remap_addr((uintptr_t)buf);

    if (dma_state.block_buf_needs_output) {
        /* Set the data target */
        P_CC3XX->dout.dst_lli_word0 = dma_state.output_addr;
        /* And the length */
        P_CC3XX->dout.dst_lli_word1 = length;

        dma_state.output_addr += length;
        dma_state.current_bytes_output += length;
    }

    /* Set the data source */
    P_CC3XX->din.src_lli_word0 = remapped_buf;
    /* Writing the length triggers the DMA */
    P_CC3XX->din.src_lli_word1 = length;

    /* Wait for the DMA to complete (The SYM_DMA_COMPLETED interrupt to be
     * asserted)
     */
    while (!(P_CC3XX->host_rgf.host_rgf_irr & 0x800U)) {
#ifdef CC3XX_CONFIG_DMA_WFI_WAIT_ENABLE
        __asm("WFI");
#endif /* CC3XX_CONFIG_WFI_WAIT_ENABLE */
    }

    /* Reset the SYM_DMA_COMPLETED interrupt */
    P_CC3XX->host_rgf.host_rgf_icr = 0x800U;

    /* Disable the DMA clock */
    P_CC3XX->misc.dma_clk_enable = 0x0U;
}

void cc3xx_dma_copy_data(void* dest, const void* src, size_t length)
{
    /* Set to PASSTHROUGH engine */
    cc3xx_set_engine(CC3XX_ENGINE_NONE);

    /* Set output target */
    cc3xx_dma_set_output(dest, length);

    /* This starts the copy */
    cc3xx_dma_buffered_input_data(src, length, true);
    cc3xx_dma_flush_buffer(false);
}

cc3xx_err_t cc3xx_dma_buffered_input_data(const void* buf, size_t length,
                                          bool write_output)
{
    size_t block_buf_size_free =
        dma_state.block_buf_size - dma_state.block_buf_size_in_use;
    size_t data_to_process_length = 0;
    size_t dma_input_length = 0;

    if (write_output) {
        if (length > dma_state.output_size) {
            return CC3XX_ERR_DMA_OUTPUT_BUFFER_TOO_SMALL;
        }
        dma_state.output_size -= length;
    }

    /* The DMA block buf will hold a block (to allow GCM and Hashing which both
     * require a last-block special case to work). First, fill this block.
     */
    if (dma_state.block_buf_size_in_use != 0) {
        /* If we need to output the block buffer, and then new data shouldn't be
         * output, then the block buffer needs to be flushed
         */
        if (dma_state.block_buf_needs_output != write_output) {
            cc3xx_dma_flush_buffer(false);
        } else {
            data_to_process_length =
                length < block_buf_size_free ? length : block_buf_size_free;
            memcpy(dma_state.block_buf + dma_state.block_buf_size_in_use, buf,
                   block_buf_size_free);
            dma_state.block_buf_size_in_use += data_to_process_length;
            buf += data_to_process_length;
            length -= data_to_process_length;
        }
    }

    if (length == 0) {
        return CC3XX_ERR_SUCCESS;
    }

    dma_state.block_buf_needs_output = write_output;

    /* The block buf is now full, and we have remaining data. First dispatch the
     * block buf. If the buffer is empty, this is a no-op.
     */
    cc3xx_dma_flush_buffer(false);

    /* If we have any whole blocks left, flush them (but make sure at least some
     * data always remains to insert into the block buf.
     */
    data_to_process_length = ((length - 1) / dma_state.block_buf_size) * dma_state.block_buf_size;
    while (data_to_process_length > 0) {
        dma_input_length = data_to_process_length < 0x10000 ? data_to_process_length
                                                            : 0x10000 - dma_state.block_buf_size;
        process_data(buf, dma_input_length);
        data_to_process_length -= dma_input_length;
        length -= dma_input_length;
        buf += dma_input_length;
    }

    /* Write the remaining data into the block buffer. The previous flush means
     * the buffer is empty, and we have less than a block of input data left, so
     * this can't overflow.
     */
    memcpy(dma_state.block_buf, buf, length);
    dma_state.block_buf_size_in_use += length;

    return CC3XX_ERR_SUCCESS;
}

void cc3xx_dma_flush_buffer(bool zero_pad_first)
{
    if (dma_state.block_buf_size_in_use > 0) {
        if (zero_pad_first) {
            memset(dma_state.block_buf + dma_state.block_buf_size_in_use, 0,
                   sizeof(dma_state.block_buf) - dma_state.block_buf_size_in_use);
            dma_state.block_buf_size_in_use = dma_state.block_buf_size;
        }

        process_data(dma_state.block_buf, dma_state.block_buf_size_in_use);
        dma_state.block_buf_size_in_use = 0;
    }
}

void cc3xx_dma_set_buffer_size(size_t size) {
    dma_state.block_buf_size = size;
    assert(size <= CC3XX_DMA_BLOCK_BUF_MAX_SIZE);
}

void cc3xx_dma_set_output(void* buf, size_t length)
{
    if (buf != NULL) {
        /* If we're swapping the buffer location, flush remaining data, but only
         * if the data in the block buffer is marked as needed to be output.
         */
        if (dma_state.block_buf_needs_output) {
            cc3xx_dma_flush_buffer(false);
        }

        /* remap the address, particularly for TCMs */
        dma_state.output_addr = remap_addr((uintptr_t)buf);
    }

    dma_state.output_size = length;
}

void cc3xx_dma_uninit(void)
{
    memset(&dma_state, 0, sizeof(dma_state));
}
