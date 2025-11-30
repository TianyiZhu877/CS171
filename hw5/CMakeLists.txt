cmake_minimum_required(VERSION 3.22)
project(hw3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set output directories to build folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/utils/include)
include_directories(SYSTEM ${CMAKE_SOURCE_DIR})

# Find OpenGL libraries (same as demo Makefile)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

# Gather all .cpp files from utils directory
file(GLOB UTILS_SOURCES "utils/*.cpp")

# Main executable target
add_executable(opengl_renderer  main.cpp ${UTILS_SOURCES})
target_link_libraries(opengl_renderer ${OPENGL_LIBRARIES} GLEW::GLEW GLUT::GLUT m)

# Demo executable target
add_executable(opengl_demo demo/opengl_demo.cpp)
target_link_libraries(opengl_demo ${OPENGL_LIBRARIES} GLEW::GLEW GLUT::GLUT m)

# Create custom targets for make commands
add_custom_target(main_target DEPENDS opengl_renderer)
add_custom_target(demo_target DEPENDS opengl_demo)

# Set default target to main
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY DEFAULT_TARGET main_target)
