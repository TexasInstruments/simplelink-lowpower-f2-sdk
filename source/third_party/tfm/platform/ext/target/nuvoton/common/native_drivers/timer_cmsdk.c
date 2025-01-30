/*
 * Copyright (c) 2016-2017 ARM Limited
 * Copyright (c) 2023 Nuvoton Technology Corp.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file timer_cmsdk.c
 * \brief Generic driver for CMSDK APB Timers.
 *        The timer is a 32-bit down-counter with the following features:
 *        - optional programmable external clock source
 *        - programmable interrupt source, triggered if counter reaches 0
 *        - automatic reload if counter reaches 0
 */

#include "timer_cmsdk.h"
#include "NuMicro.h"

/** Setter bit manipulation macro */
#define SET_BIT(WORD, BIT_INDEX) ((WORD) |= (1U << (BIT_INDEX)))
/** Clearing bit manipulation macro */
#define CLR_BIT(WORD, BIT_INDEX) ((WORD) &= ~(1U << (BIT_INDEX)))
/** Getter bit manipulation macro */
#define GET_BIT(WORD, BIT_INDEX) (bool)(((WORD) & (1U << (BIT_INDEX))))

/**
 * \brief INTSTATUS/INTCLEAR register bit definitions
 *
 */
enum interrupt_reg_bits_t{
    INTERRUPT_REG_ENUM_STATUS_AND_CLEAR_INDEX = 0
};

void cmsdk_timer_init(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;            

    if (dev->data->is_initialized == 0) {

        NVIC_EnableIRQ(TMR0_IRQn);
        NVIC_EnableIRQ(TMR2_IRQn);

        TIMER_Open(tmr, TIMER_PERIODIC_MODE, 1000);
        dev->data->is_initialized = 1;
    }
}

bool cmsdk_timer_is_initialized(const struct cmsdk_timer_dev_t* dev)
{
    return dev->data->is_initialized;
}

void cmsdk_timer_enable_external_input(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_EnableCapture(tmr, TIMER_CAPTURE_COUNTER_RESET_MODE, TIMER_CAPTURE_EVENT_RISING);
}

void cmsdk_timer_disable_external_input(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_DisableCapture(tmr);
}

bool cmsdk_timer_is_external_input_enabled(const struct cmsdk_timer_dev_t* dev)
{
    return ((((TIMER_T*)dev->cfg->base)->EXTCTL & TIMER_EXTCTL_CAPEN_Msk) != 0);
}

void cmsdk_timer_set_clock_to_internal(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_DisableEventCounter(tmr);
}

void cmsdk_timer_set_clock_to_external(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_EnableEventCounter(tmr, TIMER_COUNTER_EVENT_RISING);
}

bool cmsdk_timer_is_clock_external(const struct cmsdk_timer_dev_t* dev)
{
    return ((((TIMER_T *)dev->cfg->base)->CTL & TIMER_CTL_EXTCNTEN_Msk) != 0);
}

void cmsdk_timer_enable(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_Start(tmr);
}

void cmsdk_timer_disable(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_Stop(tmr);
}

bool cmsdk_timer_is_enabled(const struct cmsdk_timer_dev_t* dev)
{
    return ((((TIMER_T *)dev->cfg->base)->CTL & TIMER_CTL_CNTEN_Msk) != 0);
}

void cmsdk_timer_enable_interrupt(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_EnableInt(tmr);
}

void cmsdk_timer_disable_interrupt(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_DisableInt(tmr);
}

bool cmsdk_timer_is_interrupt_enabled(const struct cmsdk_timer_dev_t* dev)
{
    return ((((TIMER_T *)dev->cfg->base)->CTL & TIMER_CTL_INTEN_Msk) != 0);
}

bool cmsdk_timer_is_interrupt_active(const struct cmsdk_timer_dev_t* dev)
{
    return ((((TIMER_T *)dev->cfg->base)->INTSTS & TIMER_INTSTS_TIF_Msk) != 0);
}

void cmsdk_timer_clear_interrupt(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_ClearIntFlag(tmr);
}

uint32_t cmsdk_timer_get_current_value(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    return TIMER_GetCounter(tmr);
}

void cmsdk_timer_set_reload_value(const struct cmsdk_timer_dev_t* dev,
                                uint32_t reload)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;

    /* reload value must > 1 */
    if(reload < 2)
    {
        reload = 2;
    }

    TIMER_SET_CMP_VALUE(tmr, reload);
}

void cmsdk_timer_reset(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T* tmr = (TIMER_T*)dev->cfg->base;
    TIMER_ResetCounter(tmr);
}

uint32_t cmsdk_timer_get_reload_value(const struct cmsdk_timer_dev_t* dev)
{
    return ((TIMER_T*)dev->cfg->base)->CMP;
}

uint32_t cmsdk_timer_get_elapsed_value(const struct cmsdk_timer_dev_t* dev)
{
    TIMER_T *tmr;

    tmr = (TIMER_T*)dev->cfg->base;
    return tmr->CMP - tmr->CNT;
}
