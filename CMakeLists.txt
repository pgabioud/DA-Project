cmake_minimum_required(VERSION 3.10)
project(DA_Project)

set(CMAKE_CXX_STANDARD 11)
find_package (Threads)

include_directories(include)

add_executable(project
        include/Protocol.h
        include/Utils.h
        src/da_proc.cpp
        src/Utils.cpp
        src/Protocol.cpp)

target_link_libraries (project ${CMAKE_THREAD_LIBS_INIT})