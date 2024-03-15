@echo off

echo "%CD%" | findstr "\blestack\btool" > nul
if %ERRORLEVEL% == 1 goto ble5
start btool.exe ble3
goto :end
:ble5
start btool.exe
:end