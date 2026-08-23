cmake_minimum_required(VERSION 3.10)
project(bone-yards C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
endif()

# Bone Yards Game DLL
if(BUILD_GAME_DLL)
    file(GLOB BONE_YARDS_SOURCES
        "src/bone_yards/by_main.c"
        "src/bone_yards/by_power.c"
        "src/bone_yards/by_player.c"
        "src/bone_yards/by_enemy.c"
        "src/bone_yards/by_editor.c"
        "src/bone_yards/by_hazard.c"
        "src/bone_yards/by_spawn.c"
        "src/bone_yards/by_input.c"
        "src/bone_yards/by_movement.c"
    )

    add_library(game SHARED ${BONE_YARDS_SOURCES})

    target_include_directories(game PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(game PUBLIC
        ${OPENGL_LIBRARIES}
        m
    )

    if(PLATFORM_LINUX OR PLATFORM_FREEBSD)
        target_compile_options(game PRIVATE -fPIC -fvisibility=hidden)
    elseif(PLATFORM_WINDOWS)
        target_compile_definitions(game PRIVATE WIN32 _WINDOWS)
    elseif(PLATFORM_MACOS)
        target_compile_options(game PRIVATE -fPIC)
        set_target_properties(game PROPERTIES
            SUFFIX ".dylib"
            PREFIX ""
        )
    endif()

    if(BUILD_WITH_EDITOR)
        target_compile_definitions(game PRIVATE BY_EDITOR_ENABLED=1)
    endif()

    install(TARGETS game LIBRARY DESTINATION lib)
endif()

# Standalone Executable
if(BUILD_STANDALONE)
    file(GLOB ENGINE_SOURCES
        "src/bone_yards/by_engine.c"
        "src/bone_yards/by_render.c"
        "src/bone_yards/by_main_standalone.c"
    )

    add_executable(bone-yards ${ENGINE_SOURCES})

    target_include_directories(bone-yards PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${GLFW3_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
    )

    target_link_libraries(bone-yards PRIVATE
        ${OPENGL_LIBRARIES}
        glfw
        GLEW::GLEW
        m
    )

    if(PLATFORM_LINUX OR PLATFORM_FREEBSD)
        target_compile_options(bone-yards PRIVATE -fPIC)
    elseif(PLATFORM_WINDOWS)
        target_compile_definitions(bone-yards PRIVATE WIN32 _WINDOWS)
    elseif(PLATFORM_MACOS)
        target_compile_options(bone-yards PRIVATE -fPIC)
    endif()

    install(TARGETS bone-yards RUNTIME DESTINATION bin)
endif()
