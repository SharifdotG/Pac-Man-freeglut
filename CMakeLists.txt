cmake_minimum_required(VERSION 3.20)

project(pacman_fg
    VERSION 0.1.0
    DESCRIPTION "Pac-Man clone for CSE 426 Computer Graphics Lab — freeglut + C++17"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Single-binary layout: build/bin/pacman.exe with assets/ + freeglut.dll alongside.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# -----------------------------------------------------------------------------
# Sources — explicit lists per module (no GLOB; we want CMake to notice new files)
# -----------------------------------------------------------------------------
set(PACMAN_SOURCES
    src/main.cpp

    src/core/app.cpp
    src/core/clock.cpp
    src/core/game.cpp
    src/core/rng.cpp
    src/core/state.cpp

    src/render/camera.cpp
    src/render/gl_init.cpp
    src/render/particles.cpp
    src/render/primitives.cpp
    src/render/text.cpp

    src/audio/audio.cpp

    src/world/level_loader.cpp
    src/world/maze.cpp
    src/world/pathfind.cpp
    src/world/tile.cpp

    src/gameplay/collision.cpp
    src/gameplay/combo.cpp
    src/gameplay/fruit.cpp
    src/gameplay/ghost.cpp
    src/gameplay/ghost_ai.cpp
    src/gameplay/modes.cpp
    src/gameplay/pacman.cpp
    src/gameplay/perks.cpp
    src/gameplay/score.cpp

    src/ui/gameover.cpp
    src/ui/help.cpp
    src/ui/hud.cpp
    src/ui/menu.cpp
    src/ui/pause.cpp

    src/input/input.cpp

    src/util/file.cpp
)

add_executable(pacman ${PACMAN_SOURCES})

# -----------------------------------------------------------------------------
# Includes
# -----------------------------------------------------------------------------
target_include_directories(pacman
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/include
        ${CMAKE_SOURCE_DIR}/thirdparty/stb
        ${CMAKE_SOURCE_DIR}/thirdparty/miniaudio
)

# -----------------------------------------------------------------------------
# Linking — bundled freeglut + system OpenGL/winmm
# -----------------------------------------------------------------------------
target_link_directories(pacman
    PRIVATE
        ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/lib
)

# Define FREEGLUT_STATIC if we ever switch to libfreeglut_static.a; default is dynamic via
# libfreeglut.dll.a + libfreeglut.dll shipped next to the exe.
target_link_libraries(pacman
    PRIVATE
        freeglut
        opengl32
        glu32
        winmm   # miniaudio backend
        gdi32   # required by miniaudio + freeglut on some setups
)

# -----------------------------------------------------------------------------
# Compile flags — strict warnings on our code, defaults for thirdparty
# -----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(pacman PRIVATE
        -Wall -Wextra -Wpedantic -Wshadow -Wconversion
        -Wno-unused-parameter   # noisy in stub phase; tighten later
    )
endif()

# Define STB_IMAGE_IMPLEMENTATION + MINIAUDIO_IMPLEMENTATION only in their dedicated
# translation units (render/texture.cpp + audio/audio.cpp respectively).
target_compile_definitions(pacman
    PRIVATE
        $<$<CONFIG:Debug>:PACMAN_DEBUG=1>
)

# -----------------------------------------------------------------------------
# Post-build: copy freeglut.dll + assets/ next to the exe
# -----------------------------------------------------------------------------
add_custom_command(TARGET pacman POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/bin/libfreeglut.dll
        $<TARGET_FILE_DIR:pacman>/libfreeglut.dll
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:pacman>/assets
    COMMENT "Copying freeglut.dll and assets/ next to pacman.exe"
)

message(STATUS "pacman_fg configured. Build with:  cmake --build ${CMAKE_BINARY_DIR} -j")
