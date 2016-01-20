cmake_minimum_required(VERSION 3.3)
project(hw1)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set(SOURCE_FILES hello.c sigtest.c readContext.c myrestart.c test.c restart.c test.h ckpt.c)
add_executable(hw1 ${SOURCE_FILES})

