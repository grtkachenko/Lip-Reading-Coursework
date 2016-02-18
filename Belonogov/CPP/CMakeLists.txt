cmake_minimum_required(VERSION 3.0)
project(CPP)

include(/home/vanya/soft/lib/dlib-18.18/dlib/cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


find_package(OpenCV)

include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(CPP main.cpp SegSet.cpp pnt.cpp)
target_link_libraries(CPP dlib ${OpenCV_LIBS})


