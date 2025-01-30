/*
 * Copyright (c) 2019-2022 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __HOST_DEVICE_CFG_H__
#define __HOST_DEVICE_CFG_H__

/**
 * \file device_cfg.h
 * \brief
 * This is the device configuration file with only used peripherals
 * defined and configured via the secure and/or non-secure base address.
 */

#ifdef RSS_USE_HOST_UART
/* ARM UART PL011 */
#define DEFAULT_UART_CONTROL 0
#define DEFAULT_UART_BAUDRATE  115200
#define UART0_PL011_S
#define UART0_PL011_NS
#endif /* RSS_USE_HOST_UART */

/*External Intel Strata Flash Device*/
#define SPI_STRATAFLASHJ3_S

/* CFI Controller */
#define CFI_S

/* MHU */
#define MHU_AP_TO_RSS
#define MHU_RSS_TO_AP
#define MHU_SCP_TO_RSS
#define MHU_RSS_TO_SCP

#endif  /* __HOST_DEVICE_CFG_H__ */
