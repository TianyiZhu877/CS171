cmake_minimum_required(VERSION 3.22)
project(hw6_2 LANGUAGES CXX)
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

# i_bar executable target
add_executable(i_bar i_bar.cpp ${UTILS_SOURCES})
target_link_libraries(i_bar ${OPENGL_LIBRARIES} GLEW::GLEW GLUT::GLUT m)

# bunny_frames executable target
add_executable(bunny_frames bunny_frames.cpp)
target_link_libraries(bunny_frames ${OPENGL_LIBRARIES} GLEW::GLEW GLUT::GLUT m)

# Default target builds both executables
# Running "make" or "make -j8" will build both i_bar and bunny_frames
# Running "make i_bar" will only build i_bar
# Running "make bunny_frames" will only build bunny_frames
