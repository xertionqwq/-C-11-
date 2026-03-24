cmake_minimum_required(VERSION 3.15)
project(chapter3_dataShared)

add_executable(base3 dataShared.cpp)
add_executable(deadlock deadLock.cpp)
add_executable(lock lockSafe.cpp)