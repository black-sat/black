cmake_minimum_required(VERSION 3.8)

add_library(remotery STATIC remotery/lib/Remotery.c)
target_include_directories(remotery PUBLIC remotery/lib/)
target_compile_options(remotery PRIVATE -Wno-switch)
