cmake_minimum_required(VERSION 3.21)

# Qt 6 GUI front end for the Vole machine.
#
# This directory is a complete CMake project on its own:
#     cmake -S gui -B build/gui -DCMAKE_PREFIX_PATH=C:/Qt/6.12.0/mingw_64
#     cmake --build build/gui
# It also works when the root CMakeLists.txt adds it with add_subdirectory().
project(VoleGui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt's meta-object compiler and resource compiler.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Reuse the shared simulator core. When the root project already created the
# target, this is skipped.
if(NOT TARGET vole_core)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../source-code"
                     "${CMAKE_CURRENT_BINARY_DIR}/vole_core")
endif()

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

if(WIN32)
    set(VOLE_GUI_WIN32 WIN32)
endif()

add_executable(vole_gui ${VOLE_GUI_WIN32}
    MainWindow.cpp
    MainWindow.h
    main.cpp
    resources.qrc)

target_link_libraries(vole_gui PRIVATE vole_core Qt6::Widgets)

# Copy the Qt runtime next to vole_gui.exe so the application can be started
# straight from CLion or Explorer without adding Qt's bin directory to PATH.
if(WIN32 AND TARGET Qt6::windeployqt)
    add_custom_command(TARGET vole_gui POST_BUILD
        COMMAND Qt6::windeployqt
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            "$<TARGET_FILE:vole_gui>"
        COMMENT "Deploying Qt runtime next to vole_gui"
        VERBATIM)
endif()
