cmake_minimum_required(VERSION 3.3)
project(hw2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp master.c worker.c ExpSeriesTester.c)
add_executable(hw2 ${SOURCE_FILES})