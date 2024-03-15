# Windows WSL2 How To

These instructions are for building Texas Instruments F2 and F3 SimpleLink SDKs on Windows with WSL2.  These instructions are for Windows 10 or 11 running WSL2, and it is assumed the basic Ubuntu WSL2 installation has already been installed.  Please review the Ubuntu WSL2 tutorial here: https://ubuntu.com/tutorials/install-ubuntu-on-wsl2-on-windows-10#1-overview

Prerequisites:
1.  Windows 10 or 11 version that supports WSL2
2.  WSL2 with Ubuntu 20.04+
3.  Knowledge of Linux command line utilities
4.  sudo access on Linux


### Install CMake 3.21+

A CMake version greater than 3.21 is needed to build the SimpleLink SDK.  Any of the newer versions of CMake will work, and in this example, we are installing cmake-3.24.2.  This version may be found on this page(https://github.com/Kitware/CMake/releases?page=5) or at the official CMake download site (https://cmake.org/download/).

After downloading CMake, install CMake by:
sudo tar xz -f /mnt/c/Users/a0194920/Downloads/cmake-3.24.2-linux-x86_64.tar.gz -C /opt

### Install Unzip
By default, Ubuntu does not have unzip installed.  Unzip is needed in the next FreeRTOS step.  Install by:

sudo apt install unzip

### Install FreeRTOS 202212.01

Download FreeRTOS from https://www.freertos.org/a00104.html  and unzip it in a known location.  This location will be needed when setting the FreeRTOS location imports.mak.  Recommended location is $HOME/ti.

cd $HOME/ti
wget https://github.com/FreeRTOS/FreeRTOS/releases/download/202212.01/FreeRTOSv202212.01.zip
unzip FreeRTOSv202212.01.zip

### Install CCS

Download CCS 12.5+ from https://www.ti.com/tool/CCSTUDIO#downloads, un-tar the tar.gz file, and run the .run file.

1. tar zxfv CCS12.5.0.00007_linux-x64.tar.gz
2.  cd CCS12.5.0.00007_linux-x64
3. ./CCS12.5.0.00007_linux-x64/ccs_setup_12.5.0.00007.run  --mode unattended

Note that the exact versions will vary depending on the version (the 12.5.0.00007 part of the string will vary) of the downloaded CCS.  

The above sequence of steps will install CCS in $HOME/ti/CCS1250.  Again, the exact version will vary based on the CCS version download. 

### Install SysConfig

There will be a version of SysConfig in the CCS install, but the SDK requires a specific version.  The particular version requirement is due to the types of devices supported by SysConfig.  Newer devices require a newer version of SysConfig.  The safest option is to install the SDK recommended version.  

Download the 1.18 version of SysConfig from:

wget https://dr-download.ti.com/software-development/ide-configuration-compiler-or-debugger/MD-nsUM6f7Vvb/1.18.0.3266/sysconfig-1.18.0_3266-setup.run
chmod +x sysconfig-1.18.0_3266-setup.run
./sysconfig-1.18.0_3266-setup.run 

Press enter a few times to accept the license agreement.  The default installation for SysConfig will be $HOME/ti/sysconfig_1.18.0

### Install GCC

1. Download Q4 2019 version of GCC from https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2?revision=108bd959-44bd-4619-9c19-26187abf5225&rev=108bd95944bd46199c1926187abf5225&hash=E367D388F6429B67D5D6BECF691B9521. 
2.  tar jzfv gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2 -C $HOME/ti


All the build utilities are in place and follow the SDK build instructions on the main GitHub download page.  Please note the Linux path locations where CCS, GCC, SysConfig, FreeRTOS, and CMake were installed, as they will be needed when modifying imports.mak
