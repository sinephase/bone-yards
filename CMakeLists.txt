cmake_minimum_required(VERSION 3.16)
project(bone-yards-cpp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use vcpkg toolchain if it exists
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Find packages
find_package(OpenGL REQUIRED)

# Try to find SDL2 - with multiple methods for Windows
find_package(SDL2 CONFIG QUIET)

if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found via CONFIG, trying MODULE mode...")
    find_package(SDL2 MODULE QUIET)
endif()

if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found, trying manual search...")
    # Manual search in common locations
    find_path(SDL2_INCLUDE_DIR SDL.h
        PATHS
        C:/vcpkg/installed/x64-windows/include
        C:/SDL2/include
        /usr/include
        /usr/local/include
    )
    find_library(SDL2_LIBRARY SDL2
        PATHS
        C:/vcpkg/installed/x64-windows/lib
        C:/SDL2/lib
        /usr/lib
        /usr/local/lib
    )
    
    if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
        set(SDL2_FOUND TRUE)
        message(STATUS "SDL2 found at: ${SDL2_INCLUDE_DIR}")
    endif()
endif()

if(NOT SDL2_FOUND)
    message(FATAL_ERROR "SDL2 not found. Please install: vcpkg install sdl2:x64-windows")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
)

# Source files - only the C++ OpenGL port files
set(ENGINE_SOURCES
    src/qcommon.cpp
    src/main.cpp
    src/renderer/gl_main.cpp
    src/renderer/gl_model.cpp
    src/renderer/gl_light.cpp
    src/input/in_input.cpp
)

# Create executable
add_executable(bone-yards ${ENGINE_SOURCES})

# Link libraries
target_link_libraries(bone-yards PRIVATE
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
)

# Platform-specific settings
if(MSVC)
    target_compile_options(bone-yards PRIVATE /W4)
    set_target_properties(bone-yards PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
    target_compile_options(bone-yards PRIVATE -Wall -Wextra)
endif()

message(STATUS "bone-yards C++ OpenGL build configured")
message(STATUS "Output: ${CMAKE_CURRENT_BINARY_DIR}/bone-yards.exe")
