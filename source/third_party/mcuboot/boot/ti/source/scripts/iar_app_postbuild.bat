ielftool.exe --bin "%2.out" "%2-noheader.bin"
python %1/source/third_party/mcuboot/scripts/imgtool.py sign --header-size 0x80 --align 4 --slot-size %3 --version 1.0.0 --pad-header --pad --key %1/source/third_party/mcuboot/root-ec-p256.pem "%2-noheader.bin" "%2.bin"
if "%4" == "" goto end
python %1/source/third_party/mcuboot/scripts/imgtool.py sign --header-size 0x80 --align 4 --slot-size %4 --version 1.0.0 --pad-header --pad --key %1/source/third_party/mcuboot/root-ec-p256.pem "%2-noheader.bin" "%2.p2.bin"
:end
