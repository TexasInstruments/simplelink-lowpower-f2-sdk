#!/usr/bin/env bash
#******************************************************************************
#  * @attention
#  *
#  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
#  * All rights reserved.</center></h2>
#  *
#  * This software component is licensed by ST under BSD 3-Clause license,
#  * the "License"; You may not use this file except in compliance with the
#  * License. You may obtain a copy of the License at:
#  *                        opensource.org/licenses/BSD-3-Clause
#  *
#  ******************************************************************************
echo "regression script started"
sn_option=""
if [ $# -eq 1 ]; then
sn_option="sn=$1"
fi
PATH="/C/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/":$PATH
stm32programmercli="STM32_Programmer_CLI"
# remove write protection
connect="-c port=SWD "$sn_option" mode=UR --hardRst"
connect_no_reset="-c port=SWD "$sn_option" mode=HotPlug"
echo "Regression platforme H5"
product_state="-ob PRODUCT_STATE=0xED  TZEN=0xB4"
remove_bank1_protect="-ob SECWM1_STRT=127 SECWM1_END=0 WRPSGn1=0xffffffff" 
remove_bank2_protect="-ob SECWM2_STRT=127 SECWM2_END=0 WRPSGn2=0xffffffff" 
erase_all="-e all"
remove_hdp_protection="-ob HDP1_END=0 HDP2_END=0"
default_ob1="-ob SECBOOTADD="0x0C0000" HDP1_STRT=1 HDP1_END=0 HDP2_STRT=1 HDP2_END=0 SWAP_BANK=0 SRAM2_RST=0 SRAM2_ECC=0"
default_ob2="-ob SECWM2_STRT=0 SECWM2_END=127 SECWM1_STRT=0 SECWM1_END=127"


echo "Regression to PRODUCT_STATE 0xED and  tzen=1"
$stm32programmercli $connect_no_reset $product_state
echo "Remove bank1 protection and erase all"
$stm32programmercli $connect $remove_bank1_protect $erase_all
echo "Remove bank2 protection and erase all"
$stm32programmercli $connect $remove_bank2_protect $erase_all
echo "Remove hdp protection"
$stm32programmercli $connect_no_reset $remove_hdp_protection
echo "Set default OB 1 (dual bank, swap bank, sram2 reset, secure entry point, bank 1 full secure)"
$stm32programmercli $connect_no_reset $default_ob1
echo "Set default OB 2 (bank 2 full secure)"
$stm32programmercli $connect_no_reset $default_ob2
echo "regression script Done"