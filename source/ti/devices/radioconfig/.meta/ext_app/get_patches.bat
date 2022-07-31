@echo off
rem *********************************************************************
rem
rem File:	get_batches.bat
rem
rem Brief:	Batch-file called by SysConfig to gather information about 
rem			currently installed patches.
rem
rem *********************************************************************

rem SysConfig communication file
set COMFILE=%1

rem Device family
set DEV_FAMILY=%2

rem Absolute path of this command file
set EXE_PATH=%~dp0

rem Absolute path of the RF patches in the SDK
set PATCH_PATH=%EXE_PATH%\..\..\..\%DEV_FAMILY%\rf_patches

rem Get patch list and store in SysConfig communication file
dir /b %PATCH_PATH%\*.h > %COMFILE%
