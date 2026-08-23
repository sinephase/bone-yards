cmake_minimum_required(VERSION 3.10)
project(bone-yards C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GAME_DLL "Build the game DLL" ON)
option(BUILD_STANDALONE "Build standalone executable" ON)
option(BUILD_WITH_EDITOR "Enable in-game editor" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PLATFORM_LINUX TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_MACOS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(PLATFORM_FREEBSD TRUE)
endif()

find_package(OpenGL REQUIRED)

# GLFW3 and GLEW for standalone build
if(BUILD_STANDALONE)
    find_package(glfw3 QUIET)
    find_package(GLEW QUIET)
    
    if(NOT glfw3_FOUND OR NOT GLEW_FOUND)
        message(STATUS "GLFW3 or GLEW not found, trying vcpkg...")
        if(PLATFORM_WINDOWS)
            set(VCPKG_ARCH x64-windows)
            if(EXISTS "C:/vcpkg/installed/${VCPKG_ARCH}")
                set(VCPKG_INSTALLED_DIR "C:/vcpkg/installed/${VCPKG_ARCH}")
                if(NOT glfw3_FOUND)
                    set(GLFW3_INCLUDE_DIR "${VCPKG_INSTALLED_DIR}/include")
                    find_library(GLFW3_LIBRARY glfw3 PATHS "${VCPKG_INSTALLED_DIR}/lib" NO_DEFAULT_PATH)
                endif()
                if(NOT GLEW_FOUND)
                    set(GLEW_INCLUDE_DIR "${VCPKG_INSTALLED_DIR}/include")
                    find_library(GLEW_LIBRARY glew32 PATHS "${VCPKG_INSTALLED_DIR}/lib" NO_DEFAULT_PATH)
                endif()
            endif()
        endif()
    endif()
endif()

# Shared library files
set(QCOMMON_SOURCES
    src/qcommon/math.c
    src/qcommon/cvar.c
    src/qcommon/cmd.c
    src/qcommon/collision.c
    src/qcommon/pmove.c
)

set(SERVER_SOURCES
    src/server/sv_world.c
    src/server/sv_physics.c
)

set(GAME_SOURCES
    src/game/g_local.c
    src/game/g_main.c
    src/game/g_spawn.c
    src/game/g_physics.c
    src/game/g_combat.c
    src/game/g_items.c
    src/game/g_weapon.c
    src/game/g_monster.c
    src/game/g_func.c
    src/game/g_power.c
)

set(RENDERER_SOURCES
    src/ref/gl.c
    src/ref/shaders.c
    src/ref/models.c
    src/ref/textures.c
    src/ref/palette.c
)

set(CLIENT_SOURCES
    src/client/cl_main.c
    src/client/cl_input.c
    src/client/hud.c
    src/client/menu.c
    src/client/console.c
    src/client/keys.c
    src/client/sound.c
    src/client/ui.c
)

set(EDITOR_SOURCES
    src/editor/editor.c
    src/editor/ed_ui.c
)

# Common include directories
include_directories(src)
include_directories(${OPENGL_INCLUDE_DIR})

if(BUILD_STANDALONE)
    include_directories(${GLFW3_INCLUDE_DIR})
    include_directories(${GLEW_INCLUDE_DIR})
endif()

# Game DLL
if(BUILD_GAME_DLL)
    add_library(gamedll SHARED
        ${QCOMMON_SOURCES}
        ${SERVER_SOURCES}
        ${GAME_SOURCES}
    )
    
    target_link_libraries(gamedll PRIVATE ${OPENGL_LIBRARIES})
    
    if(PLATFORM_WINDOWS)
        set_target_properties(gamedll PROPERTIES SUFFIX ".dll")
    elseif(PLATFORM_MACOS)
        set_target_properties(gamedll PROPERTIES SUFFIX ".dylib")
    else()
        set_target_properties(gamedll PROPERTIES SUFFIX ".so")
    endif()
endif()

# Standalone executable
if(BUILD_STANDALONE)
    add_executable(bone-yards
        ${QCOMMON_SOURCES}
        ${SERVER_SOURCES}
        ${GAME_SOURCES}
        ${RENDERER_SOURCES}
        ${CLIENT_SOURCES}
    )
    
    if(BUILD_WITH_EDITOR)
        target_sources(bone-yards PRIVATE ${EDITOR_SOURCES})
        target_compile_definitions(bone-yards PRIVATE BUILD_WITH_EDITOR=1)
    endif()
    
    target_link_libraries(bone-yards PRIVATE
        ${OPENGL_LIBRARIES}
        glfw
        GLEW::GLEW
    )
    
    # Platform-specific link libraries
    if(PLATFORM_LINUX)
        target_link_libraries(bone-yards PRIVATE m)
    elseif(PLATFORM_WINDOWS)
        target_link_libraries(bone-yards PRIVATE ws2_32)
    elseif(PLATFORM_MACOS)
        target_link_libraries(bone-yards PRIVATE "-framework Cocoa" "-framework IOKit")
    endif()
endif()

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
