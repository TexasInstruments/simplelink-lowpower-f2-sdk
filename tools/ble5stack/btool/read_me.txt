===========================================
Texas Instruments BTool
===========================================
-------------------------------------------
BTool - (For Version 01.42.00 or higher)
-------------------------------------------

-------------------------------------------
Integrated Cross Platform Runtime Executable 
(Using Mono 4.8.1)
-------------------------------------------
1. Install BTool in Windows. (Note: Mono is NOT needed in Windows) 

2. Copy the installed files 
btool.exe 
btool.exe.config
btool_gatt_uuid.xml
btool_config.xml
as is, to a different OS with Mono installed.

3. Run "btool.exe" using Mono on the different OS.

Tested using Windows XP, Windows 7, Ubuntu 12.04 LTS, Ubuntu 11.10.
Should be usable under Linux, Mac OS X, BSD, Sun Solaris, Android, PS3, Xbox 360 and more!
See http://mono-project.com/What_is_Mono for more information. 

===========================================
Getting BTool Running In A OS Other Than Windows
===========================================

Install Mono 4.8.1
Follow instructions here to get latest mono via apt-get http://www.mono-project.com/docs/getting-started/install/linux/#debian-ubuntu-and-derivatives
sudo apt-get install mono-complete
sudo chown <user_name> /dev/ttyACM0 
or
sudo chown <user_name> /dev/ttyACM1

Using a terminal window, change to the directory where the files are and issue the following command:
mono btool.exe 




