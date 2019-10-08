cmake_minimum_required(VERSION 3.15)
project(DA_Project)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(DA_Project
        include/Protocol.h
        include/Utils.h
        src/da_proc.cpp
        src/Utils.cpp
        src/Protocol.cpp)
