/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cc3xx_pka.h"

#include "cc3xx_dev.h"
#include "cc3xx_config.h"
#include "cc3xx_rng.h"
#include "cc3xx_endian_helpers.h"

#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#define PKA_WORD_SIZE  8
#define PKA_WORD_BIT_SIZE  (PKA_WORD_SIZE * 8)
#define PKA_SRAM_SIZE 0x1800 /* 6KiB */

/* The hardware requires and extra word and byte to deal with carries etc
 * (which would then later be removed by a reduction operation). The TRM
 * suggests this should be only a word, but the extra byte is required for
 * mod_exp to function correctly.
 */
#define PKA_MAX_OVERFLOW_SIZE     (PKA_WORD_SIZE + 1)
#define PKA_MAX_OVERFLOW_BIT_SIZE (PKA_MAX_OVERFLOW_SIZE * 8)

/* Signed immediates use a two's complement encoding in 5 bits */
#define PKA_MAX_SIGNED_IMMEDIATE 15
#define PKA_MIN_SIGNED_IMMEDIATE (-16)

#define PKA_MAX_UNSIGNED_IMMEDIATE 31

#define PKA_PHYS_REG_TEMP_0 30
#define PKA_PHYS_REG_TEMP_1 31

#define CC3XX_PKA_PHYS_REG_AMOUNT 32
#define PKA_RESERVED_PHYS_REG_AMOUNT 4
#define PKA_PHYS_REG_FIRST_MAPPABLE (CC3XX_PKA_REG_NP + 1)
#define PKA_VIRT_REG_FIRST_ALLOCATABLE (CC3XX_PKA_REG_NP + 1)

#define CC3XX_PKA_RANDOM_BUF_SIZE 32

enum pka_op_size_t {
    PKA_OP_SIZE_N = 0,
    PKA_OP_SIZE_REGISTER = 1,
};

enum pka_arg_mapping_slots_t {
    PKA_MAPPING_SLOT_R0 = 0,
    PKA_MAPPING_SLOT_R1,
    PKA_MAPPING_SLOT_R2,
    PKA_MAPPING_SLOT_RES,
    PKA_MAPPING_SLOT_AMOUNT,
};

/* Where an opcode claims it performs multiple operations, that is achieved by
 * using immediate or zero operands, not by any actual switching of the
 * operation being performed.
 */
enum cc3xx_pka_operation_t {
    CC3XX_PKA_OPCODE_TERMINATE = 0x0,
    CC3XX_PKA_OPCODE_ADD_INC = 0x4, /* INC is add immediate */
    CC3XX_PKA_OPCODE_SUB_DEC_NEG = 0x5, /* DEC is add immediate */
    CC3XX_PKA_OPCODE_MODADD_MODINC = 0x6,
    CC3XX_PKA_OPCODE_MODSUB_MODDEC_MODNEG = 0x7,
    CC3XX_PKA_OPCODE_AND_TST0_CLR0 = 0x8,
    CC3XX_PKA_OPCODE_OR_COPY_SET0 = 0x9,
    CC3XX_PKA_OPCODE_XOR_FLIP0_INVERT_COMPARE = 0xA,
    CC3XX_PKA_OPCODE_SHR0 = 0xC,
    CC3XX_PKA_OPCODE_SHR1 = 0xD,
    CC3XX_PKA_OPCODE_SHL0 = 0xE,
    CC3XX_PKA_OPCODE_SHL1 = 0xF,
    CC3XX_PKA_OPCODE_MULLOW = 0x10,
    CC3XX_PKA_OPCODE_MODMUL = 0x11,
    CC3XX_PKA_OPCODE_MODMULN = 0x12,
    CC3XX_PKA_OPCODE_MODEXP = 0x13,
    CC3XX_PKA_OPCODE_DIV = 0x14,
    /* Opcodes below here are not documented in the TRM. */
    CC3XX_PKA_OPCODE_MODINV = 0x15,
    CC3XX_PKA_OPCODE_MODDIV = 0x16,
    CC3XX_PKA_OPCODE_MULHIGH = 0x17U,
    CC3XX_PKA_OPCODE_MODMLAC = 0x18U,
    CC3XX_PKA_OPCODE_MODMLACNR = 0x19U,
    CC3XX_PKA_OPCODE_SEPINT = 0x1AU,
    CC3XX_PKA_OPCODE_REDUCTION = 0x1BU,
};

/* It seems strange that the state that is external is so small, while things
 * like the virtual register allocations are internal to the implementation and
 * therefore not saved in a get_state/set_state operation. In reality,
 * recalculating the sram addresses is fast, and saving them has downsides
 * related to the temporary register sram address swapping, so this is a
 * reasonable approach.
 */
static uint32_t pka_reg_am_max;
struct {
    bool in_use;
    bool is_mapped;
    uint32_t phys_reg;
    uint32_t sram_addr;
} virt_regs[CC3XX_CONFIG_PKA_MAX_VIRT_REG_AMOUNT];

cc3xx_pka_reg_id_t phys_reg_mapping[CC3XX_PKA_PHYS_REG_AMOUNT];
uint32_t phys_reg_next_mapped;

/* So we can prevent arguments in the same call taking each others' physical
 * registers.
 */
static cc3xx_pka_reg_id_t arg_mapping_slots[PKA_MAPPING_SLOT_AMOUNT];

static struct cc3xx_pka_state_t pka_state;


static inline uint32_t pka_addr_from_byte_addr(uint32_t offset)
{
    return offset / sizeof(uint32_t);
}

static inline uint32_t pad_to_pka_word_size(uint32_t byte_size)
{
    /* round up to the nearest PKA word */
    return (((byte_size + PKA_WORD_SIZE - 1) / PKA_WORD_SIZE) * PKA_WORD_SIZE);
}

static void pka_init_from_state(void)
{
    uint32_t idx;

    P_CC3XX->misc.pka_clk_enable = 1;
    P_CC3XX->pka.pka_sw_reset = 1;

    /* The TRM says that this register is a byte-size, but it is in fact a
     * bit-size.
     */
    P_CC3XX->pka.pka_l[PKA_OP_SIZE_REGISTER] = pka_state.reg_size * 8;

    assert((pka_state.reg_size & ((PKA_WORD_SIZE) - 1)) == 0);
    assert(pka_state.reg_size >= (PKA_WORD_SIZE));

    pka_reg_am_max = (PKA_SRAM_SIZE) / pka_state.reg_size;
    if (pka_reg_am_max >= CC3XX_CONFIG_PKA_MAX_VIRT_REG_AMOUNT + 2) {
        pka_reg_am_max = CC3XX_CONFIG_PKA_MAX_VIRT_REG_AMOUNT + 2;
    }

    /* We need to allocate 4 special registers (and have at least 1 left for an
     * operation).
     */
    assert(pka_reg_am_max > 4);

    /* Unmap all the physical registers */
    phys_reg_next_mapped = PKA_PHYS_REG_FIRST_MAPPABLE;

    /* Set up the first two regions as N and Np. These are special, so map them
     * now.
     */
    for (idx = 0; idx < PKA_PHYS_REG_FIRST_MAPPABLE; idx++) {
        virt_regs[idx].is_mapped = true;
        virt_regs[idx].phys_reg = idx;
        phys_reg_mapping[idx] = idx;
        P_CC3XX->pka.memory_map[idx] =
            pka_addr_from_byte_addr(pka_state.reg_size * idx);
    }

    /* Then reserve all but two regions for the general purpose registers */
    for (; idx < pka_reg_am_max - 2; idx++) {
        virt_regs[idx].sram_addr =
            pka_addr_from_byte_addr(pka_state.reg_size * idx);
    }

    P_CC3XX->pka.memory_map[PKA_PHYS_REG_TEMP_0] =
        pka_addr_from_byte_addr(pka_state.reg_size * idx);

    idx++;

    P_CC3XX->pka.memory_map[PKA_PHYS_REG_TEMP_1] =
        pka_addr_from_byte_addr(pka_state.reg_size * idx);

    /* We don't count the temporary registers in reg_am_max, since it's used for
     * verifying parameters of functions, and these should never be used as
     * parameters */
    pka_reg_am_max -= 2;
}

void cc3xx_pka_init(uint32_t size)
{
    cc3xx_pka_uninit();

    /* Minimum size is 16 bytes (128 bits), but just transparently increase it
     * if needed
     */
    if (size < 16) {
        size = 16;
    }

    /* Max size of an operation is 256 bytes (2048 bits). The actual max size is
     * 2112 bits, but 64 bits of overflow are required. */
    assert(size <= 256);

    /* Calculate the register size based on the requested operation size + the
     * size by which operations can overflow */
    pka_state.reg_size = pad_to_pka_word_size(size + PKA_MAX_OVERFLOW_SIZE);

    pka_init_from_state();
}

static void allocate_phys_reg(cc3xx_pka_reg_id_t virt_reg,
                              enum pka_arg_mapping_slots_t slot)
{
    uint32_t phys_reg;
    cc3xx_pka_reg_id_t old_virt_reg;

    assert(virt_reg <= pka_reg_am_max);

    /* Wait for outstanding operations to finish before remapping registers */
    while(!P_CC3XX->pka.pka_done) {}

    /* Check we aren't taking a reg which is being used in this function
     * call. If we are, then just skip it, since we have a large amount of
     * physical registers and only a small amount of arguments.
     */
    do {
        phys_reg = phys_reg_next_mapped++;

        if (phys_reg_next_mapped == PKA_PHYS_REG_TEMP_0) {
            phys_reg_next_mapped = PKA_PHYS_REG_FIRST_MAPPABLE;
        }

        old_virt_reg = phys_reg_mapping[phys_reg];
    } while (phys_reg == arg_mapping_slots[PKA_MAPPING_SLOT_R0] ||
             phys_reg == arg_mapping_slots[PKA_MAPPING_SLOT_R1] ||
             phys_reg == arg_mapping_slots[PKA_MAPPING_SLOT_R2] ||
             phys_reg == arg_mapping_slots[PKA_MAPPING_SLOT_RES]);
    arg_mapping_slots[slot] = phys_reg;

    assert(virt_reg != CC3XX_PKA_REG_N);
    assert(virt_reg != CC3XX_PKA_REG_NP);
    assert(old_virt_reg != CC3XX_PKA_REG_NP);
    assert(phys_reg != CC3XX_PKA_REG_N);
    assert(phys_reg != CC3XX_PKA_REG_NP);

    if (old_virt_reg != 0) {
        /* If this register was previously mapped, update the virtual register.
         * Note that no physical register except 0 can be mapped to virtual
         * address 0, and the next mappable physical register can never be 0,
         * so this check catches uninitialised physical registers only.
         * Since the physical registers here are actually hardware virtual
         * registers, it's important to actually update the SRAM address as it
         * might have been swapped.
         */
        virt_regs[old_virt_reg].sram_addr = P_CC3XX->pka.memory_map[phys_reg];
        virt_regs[old_virt_reg].is_mapped = false;
    }

    /* Since the physical registers here are actually hardware virtual
     * registers, it's important to actually update the SRAM address as it might
     * have been swapped.
     */
    P_CC3XX->pka.memory_map[phys_reg] = virt_regs[virt_reg].sram_addr;
    while(!P_CC3XX->pka.pka_done) {}

    virt_regs[virt_reg].is_mapped = true;
    virt_regs[virt_reg].phys_reg = phys_reg;
    phys_reg_mapping[phys_reg] = virt_reg;
}

static cc3xx_pka_reg_id_t allocate_virt_reg(void)
{
    cc3xx_pka_reg_id_t reg_id = 0;
    size_t idx;

    for (idx = PKA_VIRT_REG_FIRST_ALLOCATABLE; idx < pka_reg_am_max; idx++) {
        if (!virt_regs[idx].in_use) {
            reg_id = idx;
            break;
        }
    }

    assert(idx != pka_reg_am_max);
    assert(reg_id != 0);

    virt_regs[idx].in_use = true;

    cc3xx_pka_clear(reg_id);

    return reg_id;
}

cc3xx_pka_reg_id_t cc3xx_pka_allocate_reg(void)
{
    /* We allocate virtual registers to the outside world, but this is an
     * implementation detail so we don't mention this in the function name
     */
    return allocate_virt_reg();
}

void cc3xx_pka_free_reg(cc3xx_pka_reg_id_t reg_id)
{
    assert(reg_id <= pka_reg_am_max);
    assert(virt_regs[reg_id].in_use);

    virt_regs[reg_id].in_use = false;
}

static void ensure_virt_reg_is_mapped(cc3xx_pka_reg_id_t reg_id,
                                      enum pka_arg_mapping_slots_t slot)
{
    assert(reg_id <= pka_reg_am_max);

    if (virt_regs[reg_id].is_mapped) {
        arg_mapping_slots[slot] = virt_regs[reg_id].phys_reg;
    } else {
        allocate_phys_reg(reg_id, slot);
    }
}

static void pka_write_reg(cc3xx_pka_reg_id_t reg_id, const uint32_t *data,
                          size_t len, bool swap_endian)
{
    size_t idx;

    /* Check alignment */
    assert(((uintptr_t)data & (sizeof(uint32_t) - 1)) == 0);
    /* Check length */
    assert((len & (sizeof(uint32_t) - 1)) == 0);


    /* Check slot */
    assert(reg_id < pka_reg_am_max);
    assert(virt_regs[reg_id].in_use);
    assert(len <= pka_state.reg_size);


    /* Make sure we have a physical register mapped for the virtual register */
    ensure_virt_reg_is_mapped(reg_id, PKA_MAPPING_SLOT_RES);

    /* Wait for any outstanding operations to finish before performing reads or
     * writes on the PKA SRAM
     */
    while(!P_CC3XX->pka.pka_done) {}
    P_CC3XX->pka.pka_sram_addr =
        P_CC3XX->pka.memory_map[virt_regs[reg_id].phys_reg];
    while(!P_CC3XX->pka.pka_done) {}

    /* Write data */
    for (idx = 0; idx < len / sizeof(uint32_t); idx++) {
        P_CC3XX->pka.pka_sram_wdata = swap_endian ? bswap_32(data[(len / sizeof(uint32_t) - 1) - idx])
                                                  : data[idx];
        while(!P_CC3XX->pka.pka_done) {}
    }

    /* Zero the rest of the register */
    for (; idx < pka_state.reg_size / sizeof(uint32_t); idx++) {
        P_CC3XX->pka.pka_sram_wdata = 0;
        while(!P_CC3XX->pka.pka_done) {}
    }

}

void cc3xx_pka_write_reg_swap_endian(cc3xx_pka_reg_id_t reg_id, const uint32_t *data,
                                     size_t len)
{
    pka_write_reg(reg_id, (uint32_t *)data, len, true);
}

void cc3xx_pka_write_reg(cc3xx_pka_reg_id_t reg_id, const uint32_t *data, size_t len)
{
    pka_write_reg(reg_id, data, len, false);
}

static void pka_read_reg(cc3xx_pka_reg_id_t reg_id, uint32_t *data, size_t len,
                         bool swap_endian)
{
    size_t idx;

    /* Check alignment */
    assert(((uintptr_t)data & (sizeof(uint32_t) - 1)) == 0);
    /* Check length */
    assert((len & (sizeof(uint32_t) - 1)) == 0);

    /* Check slot */
    assert(reg_id < pka_reg_am_max);
    assert(virt_regs[reg_id].in_use);
    assert(len <= pka_state.reg_size);

    /* Make sure we have a physical register mapped for the virtual register */
    ensure_virt_reg_is_mapped(reg_id, PKA_MAPPING_SLOT_RES);

    /* The PKA registers can be remapped by the hardware (by swapping value
     * values of the memory_map registers), so we need to read the memory_map
     * register to find the correct address.
     */
    while(!P_CC3XX->pka.pka_done) {}
    P_CC3XX->pka.pka_sram_raddr =
        P_CC3XX->pka.memory_map[virt_regs[reg_id].phys_reg];
    while(!P_CC3XX->pka.pka_done) {}

    /* Read data */
    for (idx = 0; idx < len / sizeof(uint32_t); idx++) {
        if (swap_endian) {
            data[(len / sizeof(uint32_t) -1) - idx] = bswap_32(P_CC3XX->pka.pka_sram_rdata);
        } else {
            data[idx] = P_CC3XX->pka.pka_sram_rdata;
        }
    }
}

void cc3xx_pka_read_reg(cc3xx_pka_reg_id_t reg_id, uint32_t *data, size_t len)
{
    pka_read_reg(reg_id, data, len, false);

}

void cc3xx_pka_read_reg_swap_endian(cc3xx_pka_reg_id_t reg_id, uint32_t *data, size_t len)
{
    pka_read_reg(reg_id, (uint32_t *)data, len, true);

}

/* Calculate the Barrett Tag (https://en.wikipedia.org/wiki/Barrett_reduction)
 * to enable reduction modulo N. If this tag is not calulated, reduction
 * operations will fail. doi:10.1007/3-540-47721-7_24 is good reference.
 *
 * We are attempting to calculate 2^k / N. In the reference the value k = 2 * n
 * where n is the bit-length of N is chosen due to the max value to be reduced
 * being representable in 2 * n bits. In the previous driver, instead k = n + 64
 * (which is the PKA word size), which means the max value to be reduced must be
 * representable in n + 64 bits. It is assumed, but not certain, that this holds
 * because of how the reduction in hardware is being calculated.
 */
static inline void calc_Np(void)
{
    cc3xx_pka_reg_id_t reg_temp_0 = cc3xx_pka_allocate_reg();
    cc3xx_pka_reg_id_t reg_temp_1 = cc3xx_pka_allocate_reg();
    uint32_t N_bit_size = cc3xx_pka_get_bit_size(CC3XX_PKA_REG_N);
    uint32_t power;

    /* If N is large, we perform a special-case operation to avoid having to
     * generate 2^n, which may be large. In this case, we first divide N by
     * 2^(N_bit_size2 * PKA_WORD_BIT_SIZE) and then divide the constant 2^(3 *
     * PKA_WORD_SIZE) by the result, meaning the largest number we need to
     * synthesize in a register is 2^(3 * PKA_WORD_BIT_SIZE). This is done so
     * that if the modulus size is the maximum 2048 bits, then the largest
     * synthesized number fits into the 2112 bit register+overflow size.
     */
    if (N_bit_size > PKA_MAX_OVERFLOW_BIT_SIZE * 2) {
        power = PKA_MAX_OVERFLOW_BIT_SIZE * 3 - 1;
        cc3xx_pka_set_to_power_of_two(reg_temp_0, power);

        power = N_bit_size - 2 * PKA_MAX_OVERFLOW_BIT_SIZE;
        cc3xx_pka_set_to_power_of_two(reg_temp_1, power);

        /* Use Np reg to hold unneeded remainder, to save an allocation */
        cc3xx_pka_div(CC3XX_PKA_REG_N, reg_temp_1, reg_temp_1, CC3XX_PKA_REG_NP);

        /* Ceiling */
        cc3xx_pka_add_si(reg_temp_1, 1, reg_temp_1);
        cc3xx_pka_div(reg_temp_0, reg_temp_1, CC3XX_PKA_REG_NP, reg_temp_1);
    } else {
        /* set r0 to 2^(N_bit_size + PKA_WORD_SIZE - 1) */
        power = N_bit_size + PKA_MAX_OVERFLOW_BIT_SIZE - 1;
        cc3xx_pka_set_to_power_of_two(reg_temp_0, power);

        /* Finally, perform the division */
        cc3xx_pka_div(reg_temp_0, CC3XX_PKA_REG_N, CC3XX_PKA_REG_NP, reg_temp_1);
    }

    cc3xx_pka_free_reg(reg_temp_0);
    cc3xx_pka_free_reg(reg_temp_1);
}

void cc3xx_pka_set_modulus(const uint32_t *N, size_t N_len,
                           const uint32_t *Np, size_t Np_len)
{
    assert(N != NULL);
    assert(N_len <= pka_state.reg_size);

    virt_regs[CC3XX_PKA_REG_N].in_use = true;
    cc3xx_pka_write_reg(CC3XX_PKA_REG_N, N, N_len);

    /* This operation size must correspond exactly to the bit-size of the
     * modulus, so a bit-counting operation is performed.
     */
    P_CC3XX->pka.pka_l[PKA_OP_SIZE_N] = cc3xx_pka_get_bit_size(CC3XX_PKA_REG_N);

    virt_regs[CC3XX_PKA_REG_NP].in_use = true;
    if (Np != NULL) {
        assert(Np_len <= pka_state.reg_size);
        cc3xx_pka_write_reg(CC3XX_PKA_REG_NP, Np, Np_len);
    } else {
        calc_Np();
    }
}

void cc3xx_pka_get_state(struct cc3xx_pka_state_t *state, uint32_t save_reg_am,
                         cc3xx_pka_reg_id_t *save_reg_list,
                         uint32_t **save_reg_ptr_list,
                         const size_t *save_reg_size_list)
{
    size_t idx;
    cc3xx_pka_reg_id_t reg_id;

    memcpy(state, &pka_state, sizeof(*state));

    for (idx = 0; idx < save_reg_am; idx++) {
        reg_id = save_reg_list[idx];
        assert(reg_id < pka_reg_am_max);
        assert(virt_regs[reg_id].in_use);

        cc3xx_pka_read_reg(reg_id, save_reg_ptr_list[idx], save_reg_size_list[idx]);
    }
}

void cc3xx_pka_set_state(const struct cc3xx_pka_state_t *state,
                         uint32_t load_reg_am, cc3xx_pka_reg_id_t *load_reg_list,
                         const uint32_t **load_reg_ptr_list,
                         const size_t *load_reg_size_list)
{
    size_t idx;
    cc3xx_pka_reg_id_t reg_id;

    memcpy(&pka_state, state, sizeof(*state));

    pka_init_from_state();

    for (idx = 0; idx < load_reg_am; idx++) {
        reg_id = load_reg_list[idx];
        assert(reg_id < pka_reg_am_max);
        assert(virt_regs[reg_id].in_use);

        cc3xx_pka_write_reg(reg_id, load_reg_ptr_list[idx], load_reg_size_list[idx]);
    }
}

void cc3xx_pka_uninit(void)
{
    memset(&pka_state, 0, sizeof(pka_state));
    memset(virt_regs, 0, sizeof(virt_regs));
    memset(phys_reg_mapping, 0, sizeof(phys_reg_mapping));
    memset(arg_mapping_slots, 0, sizeof(arg_mapping_slots));

    P_CC3XX->misc.pka_clk_enable = 0;
}

static uint32_t opcode_construct(enum cc3xx_pka_operation_t op,
                                 enum pka_op_size_t size,
                                 bool r0_is_immediate, uint32_t r0,
                                 bool r1_is_immediate, uint32_t r1,
                                 bool discard_result, uint32_t res)
{
    uint32_t opcode = 0;

    /* The tag part of the opcode register is designed to be used to debug PKA
     * operations, but we don't use this functionality. For some of the opcodes
     * that aren't documented in the TRM, this is used as a third register
     * input.
     */
    /* opcode |= r3 & 0b11111; */

    /* The top bit of the output register select is a field which if set
     * prevents the operation writing the output register (or more accurately,
     * prevents the swapping of the virtual address of the output register and
     * the temporary register). The pka_status register is still set, so flags
     * such as the sign of the result can still be used.
     */
    if (!discard_result) {
        assert(res >= 0);
        assert(res < pka_reg_am_max);
        assert(virt_regs[res].in_use);
        /* Make sure we have a physical register mapped for the virtual register */
        ensure_virt_reg_is_mapped(res, PKA_MAPPING_SLOT_RES);
        opcode |= (virt_regs[res].phys_reg & 0b11111) << 6;
    } else {
        opcode |= (discard_result & 0b1) << 11;
    }

    /* The top bit of the REG_A field is a toggle between being a register ID
     * and an immediate, and the lower 5 bits give us either a 0-31 register ID,
     * a -16-15 signed immediate or a 0-31 unsigned immediate depending on the
     * operation.
     */
    if (r1_is_immediate) {
        opcode |= (r1_is_immediate & 0b1) << 17;
        opcode |= (r1 & 0b11111) << 12;
    } else {
        assert(r1 >= 0);
        assert(r1 < pka_reg_am_max);
        assert(virt_regs[r1].in_use);
        /* Make sure we have a physical register mapped for the virtual register */
        ensure_virt_reg_is_mapped(r1, PKA_MAPPING_SLOT_R1);
        opcode |= (virt_regs[r1].phys_reg & 0b11111) << 12;
    }

    /* The top bit of the REG_B field is a toggle between being a register ID
     * and an immediate, and the lower 5 bits give us either a 0-31 register ID,
     * a -16-15 signed immediate or a 0-31 unsigned immediate depending on the
     * operation.
     */
    if (r0_is_immediate) {
        opcode |= (r0_is_immediate & 0b1) << 23;
        opcode |= (r0 & 0b11111) << 18;
    } else {
        assert(r0 >= 0);
        assert(r0 <= pka_reg_am_max);
        assert(virt_regs[r0].in_use);
        /* Make sure we have a physical register mapped for the virtual register */
        ensure_virt_reg_is_mapped(r0, PKA_MAPPING_SLOT_R0);
        opcode |= (virt_regs[r0].phys_reg & 0b11111) << 18;
    }

    /* Select which of the pka_l register is used for the bit-length of the
     * operation.
     */
    opcode |= (size & 0b111) << 24;

    /* Set the actual operation */
    opcode |= (op & 0b11111) << 27;

    return opcode;
}

uint32_t cc3xx_pka_get_bit_size(cc3xx_pka_reg_id_t r0)
{
    int32_t idx;
    uint32_t word;

    ensure_virt_reg_is_mapped(r0, PKA_MAPPING_SLOT_R0);

    /* This isn't an operation that can use the PKA pipeline, so we need to wait
     * for the pipeline to be finished before reading the SRAM.
     */
    while(!P_CC3XX->pka.pka_done) {}

    for (idx = pka_state.reg_size - sizeof(uint32_t); idx >= 0;
        idx -= sizeof(uint32_t)) {
        P_CC3XX->pka.pka_sram_raddr =
            P_CC3XX->pka.memory_map[virt_regs[r0].phys_reg] +
            pka_addr_from_byte_addr(idx);
        while(!P_CC3XX->pka.pka_done) {}

        word = P_CC3XX->pka.pka_sram_rdata;

        if (word) {
            break;
        }
    }

    if (idx < 0) {
        return 0;
    } else {
        return idx * 8 + (32 - __builtin_clz(word));
    }
}

void cc3xx_pka_set_to_power_of_two(cc3xx_pka_reg_id_t r0, uint32_t power)
{
    uint32_t final_word = 1 << (power % (sizeof(uint32_t) * 8));
    uint32_t word_offset = power / (8 * sizeof(uint32_t));

    cc3xx_pka_clear(r0);

    ensure_virt_reg_is_mapped(r0, PKA_MAPPING_SLOT_R0);

    /* This isn't an operation that can use the PKA pipeline, so we need to wait
     * for the pipeline to be finished before reading the SRAM.
     */
    while(!P_CC3XX->pka.pka_done) {}

    P_CC3XX->pka.pka_sram_addr =
        P_CC3XX->pka.memory_map[virt_regs[r0].phys_reg] + word_offset;
    while(!P_CC3XX->pka.pka_done) {}

    P_CC3XX->pka.pka_sram_wdata = final_word;
    while(!P_CC3XX->pka.pka_done) {}
}

#ifdef CC3XX_CONFIG_RNG_ENABLE
cc3xx_err_t cc3xx_pka_set_to_random(cc3xx_pka_reg_id_t r0)
{
    uint32_t random_buf[pka_state.reg_size / sizeof(uint32_t)];
    cc3xx_err_t err;

    err = cc3xx_rng_get_random((uint8_t*)random_buf, sizeof(random_buf));
    if (err != CC3XX_ERR_SUCCESS) {
        return err;
    }

    cc3xx_pka_write_reg(r0, random_buf, sizeof(random_buf));

    return CC3XX_ERR_SUCCESS;
}
#endif /* CC3XX_CONFIG_RNG_ENABLE */

void cc3xx_pka_add(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_ADD_INC,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_add_si(cc3xx_pka_reg_id_t r0, int32_t imm, cc3xx_pka_reg_id_t res)
{
    assert(imm <= PKA_MAX_SIGNED_IMMEDIATE);
    assert(imm >= PKA_MIN_SIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_ADD_INC,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, imm, false, res);
}

void cc3xx_pka_sub(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SUB_DEC_NEG,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_sub_si(cc3xx_pka_reg_id_t r0, int32_t imm, cc3xx_pka_reg_id_t res)
{
    assert(imm <= PKA_MAX_SIGNED_IMMEDIATE);
    assert(imm >= PKA_MIN_SIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SUB_DEC_NEG,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, imm, false, res);
}

void cc3xx_pka_neg(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SUB_DEC_NEG,
                                           PKA_OP_SIZE_REGISTER,
                                           true, 0, false, r0, false, res);
}

void cc3xx_pka_mod_add(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(cc3xx_pka_less_than(r1, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODADD_MODINC,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_mod_add_si(cc3xx_pka_reg_id_t r0, int32_t imm, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);

    assert(imm <= PKA_MAX_SIGNED_IMMEDIATE);
    assert(imm >= PKA_MIN_SIGNED_IMMEDIATE);

    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(cc3xx_pka_greater_than_si(CC3XX_PKA_REG_N, imm));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODADD_MODINC,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, imm, false, res);
}

void cc3xx_pka_mod_sub(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(cc3xx_pka_less_than(r1, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODSUB_MODDEC_MODNEG,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_mod_sub_si(cc3xx_pka_reg_id_t r0, int32_t imm, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);

    assert(imm <= PKA_MAX_SIGNED_IMMEDIATE);
    assert(imm >= PKA_MIN_SIGNED_IMMEDIATE);

    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(cc3xx_pka_greater_than_si(CC3XX_PKA_REG_N, imm));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODSUB_MODDEC_MODNEG,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, imm, false, res);
}

void cc3xx_pka_mod_neg(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODSUB_MODDEC_MODNEG,
                                           PKA_OP_SIZE_REGISTER,
                                           true, 0, false, r0, false, res);
}

void cc3xx_pka_and(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_AND_TST0_CLR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_and_si(cc3xx_pka_reg_id_t r0, uint32_t mask, cc3xx_pka_reg_id_t res)
{
    assert(mask <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_AND_TST0_CLR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, mask, false, res);
}

bool cc3xx_pka_test_bit(cc3xx_pka_reg_id_t r0, uint32_t idx)
{
    assert((0x1 << idx) <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_AND_TST0_CLR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, 0x1 << idx, true, 0);

    /* We need the pipeline to finish before we read the status register for the
     * result.
     */
    while(!P_CC3XX->pka.pka_done) {}

    /* Return the inverted value of ALU_OUT_ZERO */
    return !(P_CC3XX->pka.pka_status & (0b1 << 12));
}

void cc3xx_pka_clear_bit(cc3xx_pka_reg_id_t r0, uint32_t idx, cc3xx_pka_reg_id_t res)
{
    /* Check that we can construct the required mask */
    assert((0x1 << idx) <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_AND_TST0_CLR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, ~(1 << idx), false, res);
}

void cc3xx_pka_clear(cc3xx_pka_reg_id_t r0)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_AND_TST0_CLR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, 0, false, r0);
}

void cc3xx_pka_or(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_OR_COPY_SET0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_or_si(cc3xx_pka_reg_id_t r0, uint32_t mask, cc3xx_pka_reg_id_t res)
{
    assert(mask <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_OR_COPY_SET0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, mask, false, res);
}

void cc3xx_pka_copy(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_OR_COPY_SET0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, 0, false, res);
}

void cc3xx_pka_set_bit(cc3xx_pka_reg_id_t r0, uint32_t idx, cc3xx_pka_reg_id_t res)
{
    assert(idx < 32);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_AND_TST0_CLR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, 1 << idx, false, res);
}

void cc3xx_pka_xor(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_XOR_FLIP0_INVERT_COMPARE,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_xor_si(cc3xx_pka_reg_id_t r0, uint32_t mask, cc3xx_pka_reg_id_t res)
{
    assert(mask <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_XOR_FLIP0_INVERT_COMPARE,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, mask, false, res);
}

void cc3xx_pka_flip_bit(cc3xx_pka_reg_id_t r0, uint32_t idx, cc3xx_pka_reg_id_t res)
{
    assert(idx < 32);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_XOR_FLIP0_INVERT_COMPARE,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, 1 << idx, false, res);
}

bool cc3xx_pka_are_equal(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_XOR_FLIP0_INVERT_COMPARE,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, true, 0);

    /* We need the pipeline to finish before we read the status register for the
     * result.
     */
    while(!P_CC3XX->pka.pka_done) {}

    /* Return ALU_OUT_ZERO */
    return P_CC3XX->pka.pka_status & (0b1 << 12);
}

bool cc3xx_pka_are_equal_si(cc3xx_pka_reg_id_t r0, int32_t imm)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_XOR_FLIP0_INVERT_COMPARE,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, imm, true, 0);

    /* We need the pipeline to finish before we read the status register for the
     * result.
     */
    while(!P_CC3XX->pka.pka_done) {}

    /* Return ALU_OUT_ZERO */
    return P_CC3XX->pka.pka_status & (0b1 << 12);
}

bool cc3xx_pka_less_than(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SUB_DEC_NEG,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, true, 0);

    /* Wait for the pipeline to be finished before reading the pka status
     * register.
     */
    while(!P_CC3XX->pka.pka_done) {}

    /* Return the value of ALU_SIGN_OUT */
    return P_CC3XX->pka.pka_status & (0b1 << 8);
}

bool cc3xx_pka_less_than_si(cc3xx_pka_reg_id_t r0, int32_t imm)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SUB_DEC_NEG,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, imm, true, 0);

    /* Wait for the pipeline to be finished before reading the pka status
     * register.
     */
    while(!P_CC3XX->pka.pka_done) {}

    /* Return the value of ALU_SIGN_OUT */
    return P_CC3XX->pka.pka_status & (0b1 << 8);
}

bool cc3xx_pka_greater_than(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1)
{
    return !cc3xx_pka_less_than(r0, r1)
        && !cc3xx_pka_are_equal(r0, r1);
}

bool cc3xx_pka_greater_than_si(cc3xx_pka_reg_id_t r0, int32_t imm)
{
    return !cc3xx_pka_less_than_si(r0, imm)
        && !cc3xx_pka_are_equal_si(r0, imm);
}

void cc3xx_pka_shift_right_fill_0_ui(cc3xx_pka_reg_id_t r0, uint32_t shift, cc3xx_pka_reg_id_t res)
{
    assert(shift <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* The shift operations shifts by 1 more than the number requested, so for
     * the sake of sensible semantics we decrease the shift number by 1.
     * Shifting by 0 is technically reasonable, but we can decrease code-size by
     * disallowing it via this assert.
     */
    assert(shift != 0);
    shift -= 1;

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SHR0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, shift, false, res);
}

void cc3xx_pka_shift_right_fill_1_ui(cc3xx_pka_reg_id_t r0, uint32_t shift, cc3xx_pka_reg_id_t res)
{
    assert(shift <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* The shift operations shifts by 1 more than the number requested, so for
     * the sake of sensible semantics we decrease the shift number by 1.
     * Shifting by 0 is technically reasonable, but we can decrease code-size by
     * disallowing it via this assert.
     */
    assert(shift != 0);
    shift -= 1;

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SHR1,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, shift, false, res);
}

void cc3xx_pka_shift_left_fill_0_ui(cc3xx_pka_reg_id_t r0, uint32_t shift, cc3xx_pka_reg_id_t res)
{
    assert(shift <= PKA_MAX_UNSIGNED_IMMEDIATE);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* The shift operations shifts by 1 more than the number requested, so for
     * the sake of sensible semantics we decrease the shift number by 1.
     * Shifting by 0 is technically reasonable, but we can decrease code-size by
     * disallowing it via this assert.
     */
    assert(shift != 0);
    shift -= 1;

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SHL0,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, shift, false, res);
}

void cc3xx_pka_shift_left_fill_1_ui(cc3xx_pka_reg_id_t r0, uint32_t shift, cc3xx_pka_reg_id_t res)
{
    assert(shift <= PKA_MAX_UNSIGNED_IMMEDIATE);
    assert(shift != 0);

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    shift -= 1;

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_SHL1,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, true, shift, false, res);
}

void cc3xx_pka_mul_low_half(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MULLOW,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_mul_high_half(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MULHIGH,
                                           PKA_OP_SIZE_REGISTER,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_div(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t quotient,
                   cc3xx_pka_reg_id_t remainder)
{
    cc3xx_pka_reg_id_t temp_r0 = cc3xx_pka_allocate_reg();
    cc3xx_pka_reg_id_t temp_r1;

    /* Since the div operation uses r0 to store the remainder, and we want to
     * avoid clobbering input registers, perform a copy first.
     */
    cc3xx_pka_copy(r0, temp_r0);

    /* If r1 is also the quotient register, this produces no result. In this
     * case, copy to a temporary register.
     */
    if (r1 == quotient) {
        temp_r1 = cc3xx_pka_allocate_reg();
        cc3xx_pka_copy(r1, temp_r1);
    } else {
        temp_r1 = r1;
    }

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_DIV,
                                           PKA_OP_SIZE_REGISTER,
                                           false, temp_r0, false, temp_r1,
                                           false, quotient);

    /* Now clobber the remainder register */
    cc3xx_pka_copy(temp_r0, remainder);
    cc3xx_pka_free_reg(temp_r0);

    if (temp_r1 != r1) {
        cc3xx_pka_free_reg(temp_r1);
    }
}

void cc3xx_pka_mod_mul(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(cc3xx_pka_less_than(r1, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* This operation uses PKA_OP_SIZE_N, instead of _REGISTER. This is not
     * because it performs reduction, since mod_add uses _REGISTER, but because
     * it does not use the ALU, but the special-purpose modular multiplier.
     */
    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODMUL,
                                           PKA_OP_SIZE_N,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_mod_exp(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t r1, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(cc3xx_pka_less_than(r1, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* This operation uses PKA_OP_SIZE_N, instead of _REGISTER. This is not
     * because it performs reduction, since mod_add uses _REGISTER, but because
     * it does not use the ALU, but the special-purpose modular multiplier.
     */
    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODEXP,
                                           PKA_OP_SIZE_N,
                                           false, r0, false, r1, false, res);
}

void cc3xx_pka_mod_exp_si(cc3xx_pka_reg_id_t r0, int32_t imm, cc3xx_pka_reg_id_t res)
{
    cc3xx_pka_reg_id_t temp_reg = cc3xx_pka_allocate_reg();

    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));
    assert(imm <= PKA_MAX_SIGNED_IMMEDIATE);
    assert(imm >= PKA_MIN_SIGNED_IMMEDIATE);

    /* This operation doesn't work with negative numbers */
    assert(imm >= 0);

    /* temp_reg starts at 0, so this is effectively a set */
    cc3xx_pka_add_si(temp_reg, imm, temp_reg);

    cc3xx_pka_mod_exp(r0, temp_reg, res);

    cc3xx_pka_free_reg(temp_reg);
}

#ifdef CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE
void cc3xx_pka_mod_inv(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t res)
{
    cc3xx_pka_reg_id_t n_minus_2 = cc3xx_pka_allocate_reg();

    /* Use the special-case Euler theorem  a^-1 = a^N-2 mod N */
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));

    cc3xx_pka_sub_si(CC3XX_PKA_REG_N, 2, n_minus_2);
    cc3xx_pka_mod_exp(r0, n_minus_2, res);

    cc3xx_pka_free_reg(n_minus_2);
}
#else
void cc3xx_pka_mod_inv(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* This operation uses PKA_OP_SIZE_N, instead of _REGISTER. This is not
     * because it performs reduction, since mod_add uses _REGISTER, but because
     * it does not use the ALU, but the special-purpose modular multiplier.
     */
    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_MODINV,
                                           PKA_OP_SIZE_N,
                                           true, 1, false, r0, false, res);
}
#endif /* CC3XX_CONFIG_DPA_MITIGATIONS_ENABLE */

void cc3xx_pka_reduce(cc3xx_pka_reg_id_t r0, cc3xx_pka_reg_id_t res)
{
    assert(virt_regs[CC3XX_PKA_REG_N].in_use);
    assert(cc3xx_pka_less_than(r0, CC3XX_PKA_REG_N));

    /* Wait for a pipeline slot to be free before submitting this operation.
     * Note that the previous operations may still be in progress at this point.
     */
    while(!P_CC3XX->pka.pka_pipe_rdy) {}

    /* This operation uses PKA_OP_SIZE_N, instead of _REGISTER. This is not
     * because it performs reduction, since mod_add uses _REGISTER, but because
     * it does not use the ALU, but the special-purpose modular multiplier.
     */
    P_CC3XX->pka.opcode = opcode_construct(CC3XX_PKA_OPCODE_REDUCTION,
                                           PKA_OP_SIZE_N,
                                           false, r0, false, 0, false, res);
}
