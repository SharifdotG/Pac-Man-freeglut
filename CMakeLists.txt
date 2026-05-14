cmake_minimum_required(VERSION 3.20)

project(pacman_fg VERSION 0.9.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(pacman
    src/main.cpp
    src/core/app.cpp
    src/core/clock.cpp
    src/core/game.cpp
    src/core/rng.cpp
    src/core/state.cpp
    src/gameplay/pacman.cpp
    src/gameplay/ghost.cpp
    src/gameplay/ghost_ai.cpp
    src/gameplay/modes.cpp
    src/input/input.cpp
    src/util/file.cpp
    src/render/gl_init.cpp
    src/render/primitives.cpp
    src/render/text.cpp
    src/world/level_loader.cpp
    src/world/maze.cpp
    src/world/tile.cpp
    src/gameplay/collision.cpp
    src/world/pathfind.cpp
    src/gameplay/score.cpp
    src/ui/hud.cpp
    src/audio/audio.cpp
    src/gameplay/fruit.cpp
    src/gameplay/combo.cpp
    src/gameplay/perks.cpp
)

target_include_directories(pacman PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/include
    ${CMAKE_SOURCE_DIR}/thirdparty/stb
    ${CMAKE_SOURCE_DIR}/thirdparty/miniaudio
)

target_link_directories(pacman PRIVATE
    ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/lib
)

target_link_libraries(pacman PRIVATE
    freeglut opengl32 glu32 winmm gdi32
)

add_custom_command(TARGET pacman POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/thirdparty/freeglut/bin/libfreeglut.dll
        $<TARGET_FILE_DIR:pacman>/libfreeglut.dll
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:pacman>/assets
)
