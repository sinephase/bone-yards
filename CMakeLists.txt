cmake_minimum_required(VERSION 3.16)
project(bone-yards-cpp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use vcpkg toolchain
if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Find packages with vcpkg
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED CONFIG)
find_package(glm REQUIRED CONFIG)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Source files - explicitly list all source files
set(ENGINE_SOURCES
    src/qcommon.cpp
    src/main.cpp
    src/renderer/gl_main.cpp
    src/renderer/gl_model.cpp
    src/renderer/gl_light.cpp
    src/input/in_input.cpp
    src/qcommon/cmd.cpp
    src/qcommon/cvar.cpp
    src/qcommon/math.cpp
    src/qcommon/pmove.cpp
    src/qcommon/cmodel.cpp
    src/server/sv_world.cpp
    src/game/g_main.cpp
    src/game/g_combat.cpp
    src/game/g_spawn.cpp
    src/game/g_weapon.cpp
    src/editor/editor.cpp
    src/client/cl_main.cpp
    src/ref/gl.cpp
)

# Create executable
add_executable(bone-yards ${ENGINE_SOURCES})

# Link libraries - SDL2::SDL2 must come before other system libs
target_link_libraries(bone-yards PRIVATE
    SDL2::SDL2
    OpenGL::GL
    glm::glm
)

# Platform-specific settings
if(MSVC)
    target_compile_options(bone-yards PRIVATE /W4 /wd4100)
    target_link_options(bone-yards PRIVATE /SUBSYSTEM:CONSOLE)
    set_target_properties(bone-yards PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
else()
    target_compile_options(bone-yards PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

message(STATUS "bone-yards C++ OpenGL build configured")
message(STATUS "Source files: ${ENGINE_SOURCES}")
message(STATUS "SDL2 found: ${SDL2_FOUND}")
message(STATUS "GLM found: ${glm_FOUND}")
message(STATUS "Output: ${CMAKE_CURRENT_BINARY_DIR}/bone-yards.exe")
