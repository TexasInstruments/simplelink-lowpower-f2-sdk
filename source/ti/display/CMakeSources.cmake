cmake_minimum_required(VERSION 3.21.3)

set(SOURCES_COMMON Display.c DisplayExt.c DisplayHost.c DisplaySharp.c DisplayUart2.c lcd/SharpGrLib.c)

set(SOURCES_CC13XX_CC26XX ${SOURCES_COMMON} DisplayDogm1286.c lcd/LCDDogm1286.c lcd/LCDDogm1286_util.c)
