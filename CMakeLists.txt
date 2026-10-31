cmake_minimum_required(VERSION 3.20)
project(gb-emulator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(GB_BUILD_FRONTEND "Build the SDL2 frontend" ON)
option(GB_BUILD_TESTS "Build unit tests" ON)
option(GB_FETCH_SDL2 "Download and build SDL2 if it is not installed" ON)

# Warnings for our own targets only.
add_library(gb_warnings INTERFACE)
if(MSVC)
    target_compile_options(gb_warnings INTERFACE /W4 /permissive- /utf-8)
else()
    target_compile_options(gb_warnings INTERFACE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endif()

# ---- Core: the emulator itself, no platform dependencies ----
add_library(gb_core
    src/core/cartridge.cpp
)
target_include_directories(gb_core PUBLIC src)
target_link_libraries(gb_core PRIVATE gb_warnings)

# ---- Frontend: window, input, audio via SDL2 ----
if(GB_BUILD_FRONTEND)
    find_package(SDL2 CONFIG QUIET)
    if(NOT SDL2_FOUND)
        if(GB_FETCH_SDL2)
            message(STATUS "SDL2 not found, fetching it")
            include(FetchContent)
            set(SDL_SHARED OFF CACHE BOOL "" FORCE)
            set(SDL_STATIC ON CACHE BOOL "" FORCE)
            set(SDL_TEST OFF CACHE BOOL "" FORCE)
            FetchContent_Declare(SDL2
                URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.9/SDL2-2.30.9.tar.gz
                DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
            FetchContent_MakeAvailable(SDL2)
        else()
            message(FATAL_ERROR "SDL2 not found. Install it (e.g. libsdl2-dev) or set GB_FETCH_SDL2=ON")
        endif()
    endif()

    add_executable(gbemu src/frontend/main.cpp)
    target_link_libraries(gbemu PRIVATE gb_core gb_warnings)
    if(TARGET SDL2::SDL2main)
        target_link_libraries(gbemu PRIVATE SDL2::SDL2main)
    endif()
    if(TARGET SDL2::SDL2)
        target_link_libraries(gbemu PRIVATE SDL2::SDL2)
    else()
        target_link_libraries(gbemu PRIVATE SDL2::SDL2-static)
    endif()
endif()

# ---- Tests ----
if(GB_BUILD_TESTS)
    enable_testing()
    add_executable(gb_tests tests/test_main.cpp tests/test_cartridge.cpp)
    target_link_libraries(gb_tests PRIVATE gb_core gb_warnings)
    add_test(NAME unit COMMAND gb_tests)
endif()
