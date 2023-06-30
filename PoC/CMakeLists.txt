cmake_minimum_required(VERSION 3.25)
project(EchoACPoC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(EchoACPoC main.cpp DriverInterface.h DriverTypes.h)
