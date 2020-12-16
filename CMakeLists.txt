cmake_minimum_required(VERSION 3.0)
project(pacman)
set(CMAKE_C_STANDARD 11)
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
endif()

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
        target_link_libraries(sokol X11 Xi Xcursor GL asound dl m)
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
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(pacman PUBLIC --shell-file ../sokol/shell.html)
    target_link_options(pacman PUBLIC -sNO_FILESYSTEM=1 -sASSERTIONS=0 -sMALLOC=emmalloc --closure=1)
endif()

# explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(pacman PRIVATE LINKER:-dead_strip)
endif()
