REM "%1 = PROJ_FNAME  %2 = CONFIG_NAME %3 = TARGET_BPATH %4 = PROJ_DIR %5 = SDK_PATH"

ielftool.exe --ihex --verbose "%3.out" "%3.hex" 
REM change the directory to workspace to allow oad image tool to write file 
cd %5
dir
"%5/tools/common/oad/oad_image_tool.exe" --verbose iar "%4" 7 -hex1  "%3.hex"  -o "%2/%1"
"%5/tools/ti154stack/turbo_oad/toad_image_tool.exe" --verbose -j "%4/ti_154stack_toad_config.json" -nimg "%4/%2/%1.bin" -o "%4/%2/%1.dim"
