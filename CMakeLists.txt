cmake_minimum_required(VERSION 3.21)

project(VoleSimulator
    VERSION 1.0.0
    DESCRIPTION "Vole Machine Simulator with a Qt 6 Widgets GUI front end"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Shared simulator core, built as the vole_core static library.
# gui/CMakeLists.txt reuses this target instead of adding it a second time.
add_subdirectory(source-code)

# Qt 6 Widgets GUI front end.
add_subdirectory(gui)
