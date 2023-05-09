ielftool.exe --ihex --verbose %1.out %1.hex

%5/tools/common/oad/oad_image_tool.exe --verbose iar %2 7 -hex1 %3/Exe/%4.hex -k %5/tools/common/oad/private.pem -o %6/%7_oad