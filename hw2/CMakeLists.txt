cmake_minimum_required(VERSION 3.22)
project(hw2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/utils/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR})

# Gather all .cpp files from utils directory
file(GLOB UTILS_SOURCES "utils/*.cpp")

# Add executable with source files
add_executable(shaded_renderer main.cpp ${UTILS_SOURCES})
