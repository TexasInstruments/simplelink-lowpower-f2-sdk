/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.txt file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */
#ifndef SID_PROTOCOL_DCR_CONFIG_H
#define SID_PROTOCOL_DCR_CONFIG_H

#include <sid_time_types.h>
#include <rnet_errors.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SID_DCR_MIN_DUTY_CYCLE    1
#define SID_DCR_MAX_DUTY_CYCLE    100

/** @brief sid set duty cycle.
 *
 *  The API to configure duty cycle for restriction.
 *
 *  @param duty_cycle duty cycle in percentage.
 *  @return rnet error code
 */
rnet_error_t sid_dcr_set_duty_cycle(uint8_t duty_cycle);

/** @brief sid get available system time.
 *
 *  The API get the available system time of duty cycle restriction.
 *
 *  @param available pointer to store available system time.
 *  @return rnet error code
 */
rnet_error_t sid_dcr_get_available_time(struct sid_timespec *available);

/** @brief sid duty cycle check.
 *
 *  The API do the duty cycle check for restriction.
 *
 *  @param timestamp timestamp to check, NULL pointer will be treated as NOW.
 *  @return rnet error code
 */
rnet_error_t sid_dcr_duty_cycle_check(struct sid_timespec *timestamp);

/** @brief sid update tx available time.
 *
 *  The API update the tx available time,
 *  when a TX done event from PHY layer received
 *
 */
void sid_dcr_update_txdone_info(void);

/** @brief Update current packet time on air in ms.
 *
 *  The API update current packet time on air in ms to dcr structure.
 *
 *  @param time_on_air_ms time on air of a transmit packet.
 */
void sid_dcr_update_time_on_air_ms(uint32_t time_on_air_ms);

/** @brief Get current dcr setting.
 *
 *  @returns dcr setting in percentage
 */
uint8_t sid_dcr_get_duty_cycle(void);

#ifdef __cplusplus
}
#endif

#endif
