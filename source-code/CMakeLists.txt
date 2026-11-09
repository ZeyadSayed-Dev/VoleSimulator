cmake_minimum_required(VERSION 3.21)

# The simulator core is a small static library used by the Qt 6 GUI front end.
# It can be built on its own, or included with add_subdirectory().
project(VoleCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(vole_core STATIC
    src/ALU.cpp
    src/ControlUnit.cpp
    src/Storage.cpp
    src/Utilities.cpp
    src/VoleMachine.cpp)

target_include_directories(vole_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include")
