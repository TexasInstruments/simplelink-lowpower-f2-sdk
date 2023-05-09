:: Assuming Pyinstaller is installed via pip
:: See more here: http://www.pyinstaller.org/

@echo off

set pwd=%cd%

set bat_dir=%~dp0
cd %bat_dir%

echo "Cleaning up build folders"
rmdir /s /q %bat_dir%\bin\windows
rmdir /s /q %bat_dir%\pyinstaller_build

echo "Generate temp build folders"
mkdir %bat_dir%\pyinstaller_build

echo "Generating binary"
cd %bat_dir%\pyinstaller_build
pyinstaller --onefile --hidden-import="ecdsa" %bat_dir%\toad_image_tool.py

mkdir ..\bin\windows
move /y %bat_dir%\pyinstaller_build\dist\toad_image_tool.exe %bat_dir%\bin\windows\toad_image_tool.exe
copy %bat_dir%\bin\windows\toad_image_tool.exe %bat_dir%

cd %bat_dir%
rmdir /s /q %bat_dir%\pyinstaller_build

cd %pwd%

pause
