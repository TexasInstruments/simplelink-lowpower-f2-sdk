/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All rights reserved.
 *
 * AMAZON PROPRIETARY/CONFIDENTIAL
 *
 * You may not use this file except in compliance with the terms and
 * conditions set forth in the accompanying LICENSE.TXT file.
 *
 * THESE MATERIALS ARE PROVIDED ON AN "AS IS" BASIS. AMAZON SPECIFICALLY
 * DISCLAIMS, WITH RESPECT TO THESE MATERIALS, ALL WARRANTIES, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
 */

#ifndef INCLUDE_APPV1_WRAPPERS_TYPES_H_
#define INCLUDE_APPV1_WRAPPERS_TYPES_H_

#include <appv1/wrappers/generic.h>
#include <appv1/generic_ifc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HALL_CLASS_IMPL_STRUCT_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _impl_s)
#define HALL_CLASS_IMPL_TYPE_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _impl_t)
#define HALL_CLASS_STRUCT_EXT_TYPE_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _ext_ifc_s)
#define HALL_CLASS_IFC_EXT_TYPE_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _ext_ifc)
#define HALL_CLASS_STRUCT_TYPE_NAME(class_name_) struct CONCAT3(halo_hall_appv1_, class_name_, _ifc_s)
#define HALL_CLASS_IFC_TYPE_NAME(class_name_) CONCAT3(halo_hall_appv1_, class_name_, _ifc)

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* INCLUDE_APPV1_WRAPPERS_TYPES_H_ */
