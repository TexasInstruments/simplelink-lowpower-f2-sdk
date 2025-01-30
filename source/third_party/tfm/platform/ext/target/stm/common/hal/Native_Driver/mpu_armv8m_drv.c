/*
 * Copyright (c) 2017-2019, Arm Limited. All rights reserved.
 * Copyright (c) 2023 Cypress Semiconductor Corporation (an Infineon
 * company) or an affiliate of Cypress Semiconductor Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "mpu_armv8m_drv.h"
#include "cmsis.h"

/*
 * FixMe:
 * This is a beta quality driver for MPU in v8M. To be finalized.
 */

enum mpu_armv8m_error_t mpu_armv8m_enable(struct mpu_armv8m_dev_t *dev,
                                          uint32_t privdef_en,
                                          uint32_t hfnmi_en)
{
  /*No error checking*/

  MPU_Type *mpu = (MPU_Type *)dev->base;

  /*
   * FixMe: Set 4 pre-defined MAIR_ATTR for memory. The attributes come
   * from default memory map, need to check if fine-tune is necessary.
   *
   * MAIR0_0: Peripheral, Device-nGnRE.
   * MAIR0_1: Code, WT RA. Same attr for Outer and Inner.
   * MAIR0_2: SRAM, WBWA RA. Same attr for Outer and Inner.
   * MAIR0_3: Non cacheable RA. Same attr for Outer and Inner.
   */
  mpu->MAIR0 = (MPU_ARMV8M_MAIR_ATTR_DEVICE_VAL << MPU_MAIR0_Attr0_Pos) |
               (MPU_ARMV8M_MAIR_ATTR_CODE_VAL << MPU_MAIR0_Attr1_Pos) |
               (MPU_ARMV8M_MAIR_ATTR_DATA_VAL << MPU_MAIR0_Attr2_Pos) |
               (MPU_ARMV8M_MAIR_ATTR_DATANOCACHE_VAL << MPU_MAIR0_Attr3_Pos);

  mpu->CTRL =
    (privdef_en ? MPU_CTRL_PRIVDEFENA_Msk : 0) |
    (hfnmi_en   ? MPU_CTRL_HFNMIENA_Msk   : 0);

  /*Ensure all configuration is written before enable*/

  mpu->CTRL |= MPU_CTRL_ENABLE_Msk;

  /* Enable MPU before next instruction */
  __DSB();
  __ISB();
  return MPU_ARMV8M_OK;
}

enum mpu_armv8m_error_t mpu_armv8m_check(struct mpu_armv8m_dev_t *dev,
                                         uint32_t privdef_en,
                                         uint32_t hfnmi_en)
{
  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_ERROR;
  uint32_t mair0;
  uint32_t ctrl;

  /*No error checking*/

  MPU_Type *mpu = (MPU_Type *)dev->base;

  /*
   * FixMe: Set 4 pre-defined MAIR_ATTR for memory. The attributes come
   * from default memory map, need to check if fine-tune is necessary.
   *
   * MAIR0_0: Peripheral, Device-nGnRE.
   * MAIR0_1: Code, WT RA. Same attr for Outer and Inner.
   * MAIR0_2: SRAM, WBWA RA. Same attr for Outer and Inner.
   * MAIR0_3: Non cacheable RA. Same attr for Outer and Inner.
   */
  mair0 =      (MPU_ARMV8M_MAIR_ATTR_DEVICE_VAL << MPU_MAIR0_Attr0_Pos) |
               (MPU_ARMV8M_MAIR_ATTR_CODE_VAL << MPU_MAIR0_Attr1_Pos) |
               (MPU_ARMV8M_MAIR_ATTR_DATA_VAL << MPU_MAIR0_Attr2_Pos) |
               (MPU_ARMV8M_MAIR_ATTR_DATANOCACHE_VAL << MPU_MAIR0_Attr3_Pos);

  ctrl =
    (privdef_en ? MPU_CTRL_PRIVDEFENA_Msk : 0) |
    (hfnmi_en   ? MPU_CTRL_HFNMIENA_Msk   : 0);

  /*Ensure all configuration is written before enable*/

  ctrl |= MPU_CTRL_ENABLE_Msk;


  if ((mpu->MAIR0 == mair0) && (mpu->CTRL == ctrl))
  {
    ret_val = MPU_ARMV8M_OK;
  }

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_disable(struct mpu_armv8m_dev_t *dev)
{
  MPU_Type *mpu = (MPU_Type *)dev->base;

  /* Reset all fields as enable does full setup */
  mpu->CTRL = 0;

  return MPU_ARMV8M_OK;
}


enum mpu_armv8m_error_t mpu_armv8m_region_enable(
  struct mpu_armv8m_dev_t *dev,
  struct mpu_armv8m_region_cfg_t *region_cfg)
{
  MPU_Type *mpu = (MPU_Type *)dev->base;

  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_OK;
  uint32_t base_cfg;
  uint32_t limit_cfg;

  /*FIXME : Add complete error checking*/
  if ((region_cfg->region_base & ~MPU_RBAR_BASE_Msk) != 0)
  {
    return MPU_ARMV8M_ERROR;
  }
  /* region_limit doesn't need to be aligned but the scatter
   * file needs to be setup to ensure that partitions do not overlap.
   */
  /* don't disable MPU */

  mpu->RNR  = region_cfg->region_nr & MPU_RNR_REGION_Msk;

  /* This zeroes the lower bits of the base address */
  base_cfg = region_cfg->region_base & MPU_RBAR_BASE_Msk;
  base_cfg |= (region_cfg->attr_sh << MPU_RBAR_SH_Pos) & MPU_RBAR_SH_Msk;
  base_cfg |= (region_cfg->attr_access << MPU_RBAR_AP_Pos) & MPU_RBAR_AP_Msk;
  base_cfg |= (region_cfg->attr_exec << MPU_RBAR_XN_Pos) & MPU_RBAR_XN_Msk;

  mpu->RBAR = base_cfg;

  /* This zeroes the lower bits of limit address but they are treated as 1 */
  limit_cfg = (region_cfg->region_limit - 1) & MPU_RLAR_LIMIT_Msk;

  limit_cfg |= (region_cfg->region_attridx << MPU_RLAR_AttrIndx_Pos) &
               MPU_RLAR_AttrIndx_Msk;

  limit_cfg |= MPU_RLAR_EN_Msk;

  mpu->RLAR = limit_cfg;

  /* Enable MPU before the next instruction */
  __DSB();
  __ISB();

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_region_enable_check(
  struct mpu_armv8m_dev_t *dev,
  struct mpu_armv8m_region_cfg_t *region_cfg)
{
  MPU_Type *mpu = (MPU_Type *)dev->base;

  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_ERROR;
  uint32_t base_cfg;
  uint32_t limit_cfg;

  /*FIXME : Add complete error checking*/
  if ((region_cfg->region_base & ~MPU_RBAR_BASE_Msk) != 0)
  {
    return MPU_ARMV8M_ERROR;
  }
  /* region_limit doesn't need to be aligned but the scatter
   * file needs to be setup to ensure that partitions do not overlap.
   */
  /* don't disable MPU */

  mpu->RNR  = region_cfg->region_nr & MPU_RNR_REGION_Msk;

  /* This zeroes the lower bits of the base address */
  base_cfg = region_cfg->region_base & MPU_RBAR_BASE_Msk;
  base_cfg |= (region_cfg->attr_sh << MPU_RBAR_SH_Pos) & MPU_RBAR_SH_Msk;
  base_cfg |= (region_cfg->attr_access << MPU_RBAR_AP_Pos) & MPU_RBAR_AP_Msk;
  base_cfg |= (region_cfg->attr_exec << MPU_RBAR_XN_Pos) & MPU_RBAR_XN_Msk;

  /* This zeroes the lower bits of limit address but they are treated as 1 */
  limit_cfg = (region_cfg->region_limit - 1) & MPU_RLAR_LIMIT_Msk;

  limit_cfg |= (region_cfg->region_attridx << MPU_RLAR_AttrIndx_Pos) &
               MPU_RLAR_AttrIndx_Msk;

  limit_cfg |= MPU_RLAR_EN_Msk;

  if ((mpu->RBAR == base_cfg) && (mpu->RLAR == limit_cfg))
  {
    ret_val = MPU_ARMV8M_OK;
  }

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_region_disable(
  struct mpu_armv8m_dev_t *dev,
  uint32_t region_nr)
{

  MPU_Type *mpu = (MPU_Type *)dev->base;

  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_OK;
  uint32_t ctrl_before;

  /*FIXME : Add complete error checking*/

  ctrl_before = mpu->CTRL;
  mpu->CTRL = 0;

  mpu->RNR  = region_nr & MPU_RNR_REGION_Msk;

  mpu->RBAR = 0;
  mpu->RLAR = 0;

  /*Restore main MPU control*/
  mpu->CTRL = ctrl_before;

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_region_disable_check(
  struct mpu_armv8m_dev_t *dev,
  uint32_t region_nr)
{

  MPU_Type *mpu = (MPU_Type *)dev->base;

  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_ERROR;

  mpu->RNR  = region_nr & MPU_RNR_REGION_Msk;

  if ((mpu->RBAR == 0) && (mpu->RLAR == 0))
  {
    ret_val = MPU_ARMV8M_OK;
  }

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_region_config_only(
  struct mpu_armv8m_dev_t *dev,
  struct mpu_armv8m_region_cfg_t *region_cfg)
{
  MPU_Type *mpu = (MPU_Type *)dev->base;

  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_OK;
  uint32_t base_cfg;
  uint32_t limit_cfg;

  /*FIXME : Add complete error checking*/
  if ((region_cfg->region_base & ~MPU_RBAR_BASE_Msk) != 0)
  {
    return MPU_ARMV8M_ERROR;
  }
  if (((region_cfg->region_limit+1) & ~MPU_RLAR_LIMIT_Msk) !=0)
  {
    return MPU_ARMV8M_ERROR;
  }
  /* region_limit needs to be setup to ensure that partitions do not overlap.
   */
  /* don't disable MPU */

  mpu->RNR  = region_cfg->region_nr & MPU_RNR_REGION_Msk;

  /* This 0s the lower bits of the base address */
  base_cfg = region_cfg->region_base & MPU_RBAR_BASE_Msk;
  base_cfg |= (region_cfg->attr_sh << MPU_RBAR_SH_Pos) & MPU_RBAR_SH_Msk;
  base_cfg |= (region_cfg->attr_access << MPU_RBAR_AP_Pos) & MPU_RBAR_AP_Msk;
  base_cfg |= (region_cfg->attr_exec << MPU_RBAR_XN_Pos) & MPU_RBAR_XN_Msk;

  mpu->RBAR = base_cfg;

  /*This 0s the lower bits of base address but they are treated as 1 */
  limit_cfg = (region_cfg->region_limit) & MPU_RLAR_LIMIT_Msk;

  limit_cfg |= (region_cfg->region_attridx << MPU_RLAR_AttrIndx_Pos) &
               MPU_RLAR_AttrIndx_Msk;

  /* Do not enable region */
  /* limit_cfg |= MPU_RLAR_EN_Msk; */

  mpu->RLAR = limit_cfg;

  /* Enable MPU before the next instruction */
  __DSB();
  __ISB();

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_region_config_only_check(
  struct mpu_armv8m_dev_t *dev,
  struct mpu_armv8m_region_cfg_t *region_cfg)
{
  MPU_Type *mpu = (MPU_Type *)dev->base;

  enum mpu_armv8m_error_t ret_val = MPU_ARMV8M_ERROR;
  uint32_t base_cfg;
  uint32_t limit_cfg;

  /*FIXME : Add complete error checking*/
  if ((region_cfg->region_base & ~MPU_RBAR_BASE_Msk) != 0)
  {
    return MPU_ARMV8M_ERROR;
  }
  /* region_limit doesn't need to be aligned but the scatter
   * file needs to be setup to ensure that partitions do not overlap.
   */
  /* don't disable MPU */

  mpu->RNR  = region_cfg->region_nr & MPU_RNR_REGION_Msk;

  /* This 0s the lower bits of the base address */
  base_cfg = region_cfg->region_base & MPU_RBAR_BASE_Msk;
  base_cfg |= (region_cfg->attr_sh << MPU_RBAR_SH_Pos) & MPU_RBAR_SH_Msk;
  base_cfg |= (region_cfg->attr_access << MPU_RBAR_AP_Pos) & MPU_RBAR_AP_Msk;
  base_cfg |= (region_cfg->attr_exec << MPU_RBAR_XN_Pos) & MPU_RBAR_XN_Msk;

  /*This 0s the lower bits of base address but they are treated as 1 */
  limit_cfg = (region_cfg->region_limit) & MPU_RLAR_LIMIT_Msk;

  limit_cfg |= (region_cfg->region_attridx << MPU_RLAR_AttrIndx_Pos) &
               MPU_RLAR_AttrIndx_Msk;

  /* Region is not enabled */
  /* limit_cfg |= MPU_RLAR_EN_Msk; */

  if ((mpu->RBAR == base_cfg) && (mpu->RLAR == limit_cfg))
  {
    ret_val = MPU_ARMV8M_OK;
  }

  return ret_val;
}

enum mpu_armv8m_error_t mpu_armv8m_clean(struct mpu_armv8m_dev_t *dev)
{
  MPU_Type *mpu = (MPU_Type *)dev->base;
  uint32_t i = (mpu->TYPE & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;

  while (i > 0)
  {
    mpu_armv8m_region_disable(dev, i - 1);
    i--;
  }

  return MPU_ARMV8M_OK;

}
