@ECHO OFF

ielftool.exe --ihex --verbose %3.out %3.hex

%1\oad_image_tool --verbose iar %2 7 -hex1 %3.hex -k %1\private.pem -o %3_oad

%4\zOTAfileGen %3_oad.bin %5 %6 %7 %8