cmake_minimum_required(VERSION 3.16)
project(bone-yards-cpp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OPENGL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIRS}
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
    ${SDL2_LIBRARIES}
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
