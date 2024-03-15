REM @echo off
set repoDir=pyspinel_repo
IF not exist %repoDir% (mkdir %repoDir%)
cd %repoDir%
git clone https://github.com/TexasInstruments/ti-wisunfan-pyspinel
cd pyspinel_repo
cd ti-wisunfan-pyspinel
git checkout TI_WiSUN_STACK_01_00_06
pip install -r requirements.txt

TIMEOUT /T 10000
