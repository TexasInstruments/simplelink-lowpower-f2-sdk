/**
 ******************************************************************************
 * @file    stsafea_service_stub.h
 * @author  MCD Application Team
 * @version V1.0.0
 * @brief   Implementation file of psa_drv_se interface.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */


#ifndef STSAFEA_SERVICE_STUB_H_
#define STSAFEA_SERVICE_STUB_H_

#include "stm32u5xx.h"

int32_t HW_IO_Init(void);
void    BSP_TimeDelay(uint32_t msDelay);
int32_t BSP_I2C_Init(void);
int32_t BSP_I2C_DeInit(void);
int32_t BSP_I2C_Send(uint16_t DevAddr, uint8_t *pData, uint16_t Length);
int32_t BSP_I2C_Recv(uint16_t DevAddr, uint8_t *pData, uint16_t Length);
#endif
