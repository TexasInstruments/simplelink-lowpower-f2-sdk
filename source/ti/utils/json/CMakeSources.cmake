cmake_minimum_required(VERSION 3.21.3)

# Fetch all .c files in source/
file(GLOB json_sources *.c)

# No need for further set() since that list is all we need