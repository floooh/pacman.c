cmake_minimum_required(VERSION 3.0)
project(pacman)
set(CMAKE_C_STANDARD 11)

#=== LIBRARY: sokol
# add headers to the the file list because they are useful to have in IDEs
set(SOKOL_HEADERS
    sokol/sokol_gfx.h
    sokol/sokol_app.h
    sokol/sokol_audio.h
    sokol/sokol_time.h
    sokol/sokol_glue.h)
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    add_library(sokol STATIC sokol/sokol.m ${SOKOL_HEADERS})
    target_link_libraries(sokol
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal"
        "-framework AudioToolbox")
else()
    add_library(sokol STATIC sokol/sokol.c ${SOKOL_HEADERS})
    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_link_libraries(sokol X11 Xi Xcursor GL asound dl pthread m)
    endif()
endif()
target_include_directories(sokol INTERFACE sokol)

#=== EXECUTABLE: pacman
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_executable(pacman WIN32 pacman.c)
else()
    add_executable(pacman pacman.c)
endif()
target_link_libraries(pacman sokol)
# explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_link_options(pacman PRIVATE LINKER:-dead_strip)
endif()
