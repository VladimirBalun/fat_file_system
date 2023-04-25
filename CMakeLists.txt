cmake_minimum_required(VERSION 3.22)
project(file_system)

set(CMAKE_CXX_STANDARD 17)

add_executable(
        file_system main.cpp
        file_system/file_system.hpp
        file_system/file.hpp
        file_system/fat/fat_file.cpp
        file_system/fat/fat_file.hpp
        file_system/fat/fat_file_system.hpp
        file_system/fat/fat_memory_manager.hpp
        unit_tests.hpp
)
