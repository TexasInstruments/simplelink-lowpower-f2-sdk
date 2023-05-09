/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All rights reserved.
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

#ifndef SYSTEM_REPORTS_H
#define SYSTEM_REPORTS_H

#include <stdint.h>

//RING NET packet

typedef struct{
  uint8_t reg_resetpin      : 1;    //see RESETREAS
  uint8_t reg_dog           : 1;
  uint8_t reg_sreq          : 1;
  uint8_t reg_lockup        : 1;
  uint8_t reg_off           : 1;
  uint8_t report_hander     : 3;    //see HALO_SYSTEM_RESET_REPORT_HANDLER...
}s_rst_reas;

typedef struct{
  uint8_t code;
  s_rst_reas rst_reas;
  uint8_t pc[3];
}s_rst_report;

void rnet_sys_rst_report_process(void);
s_rst_report get_reset_report(void);

#endif //SYSTEM_REPORTS_H
