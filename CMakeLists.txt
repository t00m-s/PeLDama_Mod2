cmake_minimum_required(VERSION 3.22)
project(PeLDama_Mod2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
include_directories(.)

add_executable(dama
        play.cpp
        verify.cpp
        player.cpp)
