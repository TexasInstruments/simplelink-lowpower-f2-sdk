
/*
 * Copyright (c) 2022, Texas Instruments Incorporated. All rights reserved.
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
#ifndef ti_safe__include
#define ti_safe__include

#include "ti/devices/cc13x4_cc26x4/inc/hw_types.h"       /* HWREG() */
#include "ti/devices/cc13x4_cc26x4/driverlib/sys_ctrl.h" /* SysCtrlSystemReset() */

/* Macro that implements a double check of condition being true [volatile needed to avoid optimizations] */
#define SAFE_IF(c) if ((c) && !!(c))

/* Always inline to mitigate against single instruction glitch skipping a branch instruction */
__attribute__((always_inline)) static inline void verified_reg_write(const uint32_t addr,
                                                                     const uint32_t val,
                                                                     const uint32_t verify_mask)
{
    /* Write the value to the register */
    HWREG(addr) = val;

    /* Reset system if register read back does not match expected */
    SAFE_IF((HWREG(addr) & verify_mask) != (val & verify_mask))
    {
        /* Cold reset of entire chip causing boot code to run again */
        SysCtrlSystemReset();
        /* Spin forever if reset did not get executed */
        while (1) {}
    }
};

#endif