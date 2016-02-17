cmake_minimum_required(VERSION 3.0)
project(CPP)

include(/home/vanya/soft/lib/dlib-18.18/dlib/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(CPP ${SOURCE_FILES})
