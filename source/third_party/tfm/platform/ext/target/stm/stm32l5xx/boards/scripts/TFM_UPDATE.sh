#!/bin/bash -
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

echo "TFM UPDATE started"
# Absolute path to this script
SCRIPT=$(readlink -f $0)
# Absolute path this script
SCRIPTPATH=`dirname $SCRIPT`
PATH="/C/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/":$PATH
stm32programmercli="STM32_Programmer_CLI"
connect_no_reset="-c port=SWD mode=HotPlug"
connect="-c port=SWD mode=HotPlug --hardRst"
echo "Write TFM_Appli Secure"
# part ot be updated according to flash_layout.h
slot0=0xc014000
slot1=0xc038000
slot2=
slot3=
its=
sst=
scratch=
nvcounter=
boot=0x0c001000
unused=

$stm32programmercli $connect -d $SCRIPTPATH/../tfm_s_signed.bin $slot0 -v
echo "TFM_Appli Secure Written"
echo "Write TFM_Appli NonSecure"
$stm32programmercli $connect -d $SCRIPTPATH/../tfm_ns_signed.bin $slot1 -v
echo "TFM_Appli NonSecure Written"
echo "Write TFM_SBSFU_Boot"
$stm32programmercli $connect -d $SCRIPTPATH/../bl2/ext/mcuboot/mcuboot.bin $boot -v
echo "TFM_SBSFU_Boot Written"
echo "TFM_UPDATE Done"