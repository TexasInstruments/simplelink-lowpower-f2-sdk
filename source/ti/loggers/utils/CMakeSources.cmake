cmake_minimum_required(VERSION 3.21.3)

# Only on C file to compile. The secure version should not be compiled into the
# library.
set(loggers_sources
    LoggerBuf.c
)