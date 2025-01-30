/**
  ******************************************************************************
  * @file    nv_counters_device.h
  * @author  MCD Application Team
  * @brief   Tick device definition
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
/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef NV_COUNTERS_DEVICE_H
#define NV_COUNTERS_DEVICE_H
#include "flash_layout.h"

#define DEVICE_NV_COUNTERS_AREA_SIZE FLASH_BL2_NVCNT_AREA_SIZE

#define DEVICE_NV_COUNTERS_AREA_OFFSET FLASH_BL2_NVCNT_AREA_OFFSET

#define DEVICE_NV_COUNTERS_SECTOR_SIZE FLASH_AREA_IMAGE_SECTOR_SIZE

#define DEVICE_NV_COUNTERS_FLASH_NAME  FLASH_DEV_NAME

#endif /* NV_COUNTERS_DEVICE_H*/
