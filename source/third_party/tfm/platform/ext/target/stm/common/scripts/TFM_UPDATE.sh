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

echo "TFM UPDATE started"
sn_option=""
if [ $# -eq 1 ]; then
sn_option="sn=$1"
fi
# Absolute path to this script
SCRIPT=$(readlink -f $0)
# Absolute path this script
SCRIPTPATH=`dirname $SCRIPT`
BINPATH_SPE="$SCRIPTPATH/bin"
BINPATH_BL2="$SCRIPTPATH/bin"
BINPATH_NSPE="$SCRIPTPATH/image_signing/scripts"

PATH="/C/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/":$PATH
stm32programmercli="STM32_Programmer_CLI"
# part ot be updated according to flash_layout.h
slot0=
slot1=
slot2=
slot3=
its=
sst=
scratch=
nvcounter=
boot=
nvmcnt=
prov=
unused=
encrypted=
#select external flash according to slot2 value
u5=0x70000000
l5=0x90000000
slot_s=$slot0
slot_ns=$slot1
cubedir="`which STM32_Programmer_CLI`"
cubedir="$(dirname "${cubedir}")"
#when image are encrypted, image are not installed in place (mcuboot installs the image from download slot)
if [ $encrypted == "0x1" ]; then
slot_s=$slot2
slot_ns=$slot3
fi
if [ $slot2 == $u5 ]; then
external_loader="-el $cubedir/ExternalLoader/MX25LM51245G_STM32U585I-IOT02A.stldr"
fi
if [ $slot2 == $l5 ]; then
external_loader="-el $cubedir/ExternalLoader/MX25LM51245G_STM32L562E-DK.stldr"
fi
connect_no_reset="-c port=SWD "$sn_option" mode=UR $external_loader"
connect="-c port=SWD "$sn_option" mode=UR --hardRst $external_loader"

echo "Write TFM_Appli Secure"
# part ot be updated according to flash_layout.h

$stm32programmercli $connect -d $BINPATH_SPE/tfm_s_signed.bin $slot_s -v
echo "TFM_Appli Secure Written"
echo "Write TFM_Appli NonSecure"
$stm32programmercli $connect -d $BINPATH_NSPE/tfm_ns_signed.bin $slot_ns -v
echo "TFM_Appli NonSecure Written"
echo "Write TFM_SBSFU_Boot"
$stm32programmercli $connect -d $BINPATH_BL2/bl2.bin $boot -v
echo "TFM_SBSFU_Boot Written"
echo "TFM_UPDATE Done"
