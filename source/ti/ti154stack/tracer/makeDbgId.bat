@echo off

set SCRIPT_DIR=%~dp0

set EWP_FILE=%1%
set ROLE1=%2%
set ROLE2=%3%

set ARGC=0
for %%x in (%*) do Set /A ARGC+=1

echo %ARGC%

rem # Check if perl is available and quit if it is not.
perl -version >nul 2>&1 && (
    echo Found Perl...
) || (
    echo Perl wasn't found! Please install Strawberry Perl.
    goto exit
)

echo perl %SCRIPT_DIR%autodbg.pl %EWP_FILE% %SCRIPT_DIR%dbgid_sys_%ROLE1%.h -na -c3 > %SCRIPT_DIR%temp.txt
start /B perl %SCRIPT_DIR%autodbg.pl %EWP_FILE% %SCRIPT_DIR%dbgid_sys_%ROLE1%.h -na -c3

if %ARGC%==3 (
   echo perl %SCRIPT_DIR%autodbg.pl %EWP_FILE% %SCRIPT_DIR%dbgid_sys_%ROLE2%.h -na -c3 >> %SCRIPT_DIR%temp.txt
   start /B perl %SCRIPT_DIR%autodbg.pl %EWP_FILE% %SCRIPT_DIR%dbgid_sys_%ROLE2%.h -na -c3
)

echo Done!

:exit
