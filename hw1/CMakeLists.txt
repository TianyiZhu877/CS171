cmake_minimum_required(VERSION 3.22)
project(hw1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/utils/include)

# Gather all .cpp files from utils directory
file(GLOB UTILS_SOURCES "utils/*.cpp")

# Add executable with source files
add_executable(wireframe main.cpp ${UTILS_SOURCES})
